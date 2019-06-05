#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <time.h>
#include "histoscope.h"
#include "minuit_fcn.h"
#include "minuit.h"
#include "fit_config.h"
#include "mn_interface.h"

#define checkfree(pointer) do {if (pointer) {free(pointer); pointer=0;}} while(0);

typedef struct
{
    Minuit_user_function *fcn;
    char *tag;
    char *fullname;
} Fcn_description;

static const Fcn_description known_fcns[] = {
    {generic_stats_adder, "generic", "Generic Additive Statistic Accumulation"},
    {generic_user_minimizer, "user", "Generic Minuit_tcl_fcn Function"}
};

typedef struct
{
    Accumulated_stat_function *fcn;
    char *tag;
    char *fullname;
    int binning_required;
    int errors_required;
} Accumulator_description;

static const Accumulator_description known_acc[] = {
    {acc_leastsq_no_errors, "L2", "L2 distance (least squares, all weights equal)", 1, 0},
    {acc_pearsons_chisq, "chisq", "Chi-square (least squares, errors from the fit)", 1, 0},
    {acc_ext_max_likelihood, "eml", "Extended maximum likelihood", 0, 0},
    {acc_leastsq_with_errors, "ls", "Least squares, errors provided by user", 1, 1},
    {acc_max_likelihood, "ml", "Maximum likelihood", 0, 0}
};

/* Error codes for accumulator function calls */
enum {
    ACC_OK = 0,
    ACC_ZERO_POINT_ERROR,
    ACC_NEG_POINT_ERROR,
    ACC_NEG_BIN_VALUE,
    ACC_ZERO_PDF_VALUE,
    ACC_NEG_PDF_VALUE,
    N_ACC_ERROR_CODES
};

/* Error descriptions for accumulator function calls */
static const char *acc_error_description[N_ACC_ERROR_CODES] = {
    "success",
    "zero data error",
    "negative data error",
    "negative bin value",
    "zero probability density",
    "negative probability density"
};

/* Current user FCN */
static Minuit_user_function * current_fcn = 0;
static char * current_fcn_tag = 0;

static void report_function_error(Fit_config *fit_info, int set,
				  int fnum, DataPoint *point,
				  const double *minuit_pars, int ierr);
static void report_accum_error(Fit_config *fit_info, int set, DataPoint *point,
			       const double *minuit_pars, int ierr);
static void remember_function_params(Fit_config *fit_info, const double *xval);
static void remember_fit_results(Fit_config *fit_info);
static Minuit_user_function * hs_get_fit_method_by_tag(const char *fcn_tag);
static const Accumulator_description * get_accum_descr_by_tag(const char *fcn_tag);

Tcl_Obj * hs_list_valid_accum_methods(void)
{
    const int n = sizeof(known_acc)/sizeof(known_acc[0]);
/*      Tcl_Obj *names[n]; */
    Tcl_Obj *names[sizeof(known_acc)/sizeof(known_acc[0])];
    int i;

    for (i=0; i<n; ++i)
	names[i] = Tcl_NewStringObj(known_acc[i].tag, -1);
    return Tcl_NewListObj(n, names);
}

Minuit_fcn(generic_user_minimizer)
{
    if (fit_info->user_tcl_fcn)
    {
        if (*iflag == 1)
        {
            /* Fit initialization */
            assert(fit_info->interp);
            if (fit_info->status == TCL_ERROR)
            {
                Tcl_ResetResult(fit_info->interp);
                Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
                                 "error status flag is set. Please reset the fit status.", NULL);
            }
            if (fit_info->status != TCL_ERROR)
                if (!fit_info->par_synched)
                {
                    Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
                                     "Minuit parameters have not been (re)initialized", NULL);
                    fit_info->status = TCL_ERROR;
                }
            if (fit_info->status != TCL_ERROR)
                if (!fit_info->options_synched)
                {
                    Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
                                     "fitting options have not been passed to Minuit", NULL);
                    fit_info->status = TCL_ERROR;
                }
            hs_declare_fit_complete(fit_info, 0);
            memset(&fit_info->ministat, 0, sizeof(Minimization_status));
        }

        if (fit_info->status == TCL_OK)
            if ((fit_info->user_tcl_fcn)(fit_info->clientData, fit_info->interp,
                                         fit_info->objc, fit_info->objv,
                                         xval, *npar, *iflag, fval, grad) != TCL_OK)
                fit_info->status = TCL_ERROR;

        if (*iflag == 4 || *iflag == 2)
        {
            /* This is the Minuit main loop. The fit parameters
               are going to loose synchronization with Minuit. */
            fit_info->par_synched = 0;
        }
        else if (*iflag == 3)
        {
            /* End of fit */
            fit_info->n_variable_pars = *npar;
            remember_fit_results(fit_info);
            if (fit_info->status == TCL_OK)
                hs_declare_fit_complete(fit_info, 1);
        }
    }
    else
    {
        Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
                         "user fcn is undefined", NULL);
        fit_info->status = TCL_ERROR;
    }
}

Minuit_fcn(generic_stats_adder)
{
    int is_binned, ixbin, iybin, izbin, ipoint, ibin, jbin, fmode, aux_fvalues;
    int dt, ifun, set, defined_sets, item_type, nbins, ierr = 0;
    double x, y, z, sum, weight, dfval, accum = 0.0;
    double dxrange = 0.0, dyrange = 0.0, dzrange = 0.0;
    double dxbins = 0.0, dybins = 0.0, dzbins = 0.0;
    double halfbin_x = 0.0, halfbin_y = 0.0, halfbin_z = 0.0;
    double eps_x = 0.0;
    Fit_subset *fitset = NULL;
    DataPoint *thispoint = NULL;
    Minuit_fitter_info *fitter = NULL;
    Accumulated_stat_function *futil = NULL;
    char charbuf[32];

    /* Things related to timeout implementation */
    static time_t timeout = 0, last_time = 0;
    static int call_counter = 0, next_check = 10, call_step = 10;

    assert(fit_info);

    /* Check the calling mode */
    if (*iflag == 1)
    {
	timeout = 0;

	/* Fit initialization */
	assert(fit_info->interp);
	if (fit_info->status == TCL_ERROR)
	{
	    Tcl_ResetResult(fit_info->interp);
	    Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
			     "error status flag is set. Please reset the fit status.", NULL);
	}
	if (fit_info->status != TCL_ERROR)
	    if (!fit_info->par_synched)
	    {
		Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
				 "Minuit parameters have not been (re)initialized", NULL);
		fit_info->status = TCL_ERROR;
	    }
	if (fit_info->status != TCL_ERROR)
	    if (!fit_info->options_synched)
	    {
		Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
				 "fitting options have not been passed to Minuit", NULL);
		fit_info->status = TCL_ERROR;
	    }
	if (fit_info->status != TCL_ERROR)
	    if (!hs_is_fit_compiled(fit_info))
	    {
		Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
				 "parameter maps and/or dataset filters are not compiled", NULL);
		fit_info->status = TCL_ERROR;
	    }
	defined_sets = 0;
	for (set=0; set<fit_info->nsets; ++set)
	    if (fit_info->fitsets[set])
		++defined_sets;
	if (fit_info->status != TCL_ERROR)
	    if (defined_sets == 0)
	    {
		Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
				 "no datasets defined", NULL);
		fit_info->status = TCL_ERROR;
	    }
	/* Check that normalization regions have been defined
	   for unbinned datasets fitted by the "eml" method */
	if (fit_info->status != TCL_ERROR)
	    for (set=0; set<fit_info->nsets; ++set)
	    {
		fitset = fit_info->fitsets[set];
		if (fitset == NULL) continue;
		if (!fitset->binned && fitset->nx == 0 && 
		    fitset->acc_method == acc_ext_max_likelihood)
		{
		    sprintf(charbuf, "%d", set);
		    Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
				     "normalization region is not defined for subset ",
				     charbuf, NULL);
		    fit_info->status = TCL_ERROR;
		    break;
		}
	    }

	hs_declare_fit_complete(fit_info, 0);
	hs_synch_fit_subsets(fit_info);
	hs_init_fit_data_sets(fit_info);

	/* Reset the fit minimization status */
	memset(&fit_info->ministat, 0, sizeof(Minimization_status));

	/* Create the cache of weights needed to integrate the functions. */
	/* This memory will be freed when the fit becomes inactive. */
	if (fit_info->status != TCL_ERROR)
	{
	    for (set=0; set<fit_info->nsets; ++set)
	    {
		fitset = fit_info->fitsets[set];
		if (fitset == NULL) continue;
		checkfree(fitset->iweights);
		fitset->nweights = 0;
		is_binned = fitset->binned;
		item_type = fitset->item_type;
		futil = fitset->acc_method;
		aux_fvalues = (item_type == HS_1D_HISTOGRAM || 
			       item_type == HS_2D_HISTOGRAM ||
			       item_type == HS_3D_HISTOGRAM ||
			       (!is_binned && futil == acc_ext_max_likelihood));
		if (aux_fvalues)
		{
		    switch (fitset->ndim)
		    {
		    case 1:
			nbins = fitset->nx;
			fitset->nweights = nbins + fitset->nx + 1;
			break;
		    case 2:
			nbins = fitset->nx*fitset->ny;
			fitset->nweights = nbins +
			    (fitset->nx+1)*(fitset->ny+1);
			break;
		    case 3:
			nbins = fitset->nx*fitset->ny*fitset->nz;
			fitset->nweights = nbins +
			    (fitset->nx+1)*(fitset->ny+1)*(fitset->nz+1);
			break;
		    default:
			assert(0);
		    }

		    fitset->iweights = (DataPoint *)calloc(
			fitset->nweights, sizeof(DataPoint));
		    if (fitset->iweights == NULL)
		    {
			fitset->nweights = 0;
			Tcl_SetResult(fit_info->interp, "out of memory", TCL_VOLATILE);
			break;
		    }
		    dxrange = fitset->xmax - fitset->xmin;
		    dxbins = (double)fitset->nx;
		    halfbin_x = dxrange/dxbins/2.0;
                    eps_x     = BIN_EDGE_EPSILON*dxrange/dxbins;
		    if (fitset->ndim > 1)
		    {
			dyrange = fitset->ymax - fitset->ymin;
			dybins = (double)fitset->ny;
			halfbin_y = dyrange/dybins/2.0;
                        /* eps_y     = BIN_EDGE_EPSILON*dyrange/dybins; */
			if (fitset->ndim > 2)
			{
			    dzrange = fitset->zmax - fitset->zmin;
			    dzbins = (double)fitset->nz;
			    halfbin_z = dzrange/dzbins/2.0;
                            /* eps_z     = BIN_EDGE_EPSILON*dzrange/dzbins; */
                        }   
		    }

		    thispoint = fitset->iweights + nbins;
		    switch (fitset->ndim)
		    {
		    case 1:
			for (ixbin=0; (unsigned)ixbin<fitset->nx; ++ixbin)
			{
			    fitset->iweights[ixbin].x = fitset->xmin + 
				halfbin_x + ((double)ixbin/dxbins)*dxrange;
			    if (fitset->filter)
				if (!((fitset->filter)(fitset->iweights[ixbin].x-halfbin_x+eps_x, 0.0, 0.0) &&
                                      (fitset->filter)(fitset->iweights[ixbin].x, 0.0, 0.0) &&
                                      (fitset->filter)(fitset->iweights[ixbin].x+halfbin_x-eps_x, 0.0, 0.0)))
				    fitset->iweights[ixbin].filtered = 1;
			    if (!fitset->iweights[ixbin].filtered)
			    {
				fitset->iweights[ixbin].value = 2.0/3.0;
				thispoint[ixbin].value += 1.0/6.0;
				thispoint[ixbin+1].value += 1.0/6.0;
			    }
			}
			for (ixbin=0; (unsigned)ixbin<=fitset->nx; ++ixbin)
			{
			    thispoint[ixbin].x = fitset->xmin + 
				((double)ixbin/dxbins)*dxrange;
			    if (thispoint[ixbin].value == 0.0)
				thispoint[ixbin].filtered = 1;
			}
			break;

		    case 2:
			ipoint = 0;
			for (ixbin=0; (unsigned)ixbin<fitset->nx; ++ixbin)
			{
			    x = fitset->xmin + halfbin_x + ((double)ixbin/dxbins)*dxrange;
			    for (iybin=0; (unsigned)iybin<fitset->ny; ++iybin, ++ipoint)
			    {
				fitset->iweights[ipoint].x = x;
				fitset->iweights[ipoint].y = fitset->ymin + 
				    halfbin_y + ((double)iybin/dybins)*dyrange;
				if (fitset->filter)
				    if (!(fitset->filter)(fitset->iweights[ipoint].x, 
							  fitset->iweights[ipoint].y, 0.0))
					fitset->iweights[ipoint].filtered = 1;
				if (!fitset->iweights[ipoint].filtered)
				{
				    fitset->iweights[ipoint].value = 2.0/3.0;
				    thispoint[ixbin*(fitset->ny+1)+iybin].value += 1.0/12.0;
				    thispoint[ixbin*(fitset->ny+1)+iybin+1].value += 1.0/12.0;
				    thispoint[(ixbin+1)*(fitset->ny+1)+iybin].value += 1.0/12.0;
				    thispoint[(ixbin+1)*(fitset->ny+1)+iybin+1].value += 1.0/12.0;
				}
			    }
			}
			ipoint = 0;
			for (ixbin=0; (unsigned)ixbin<=fitset->nx; ++ixbin)
			{
			    x = fitset->xmin + ((double)ixbin/dxbins)*dxrange;
			    for (iybin=0; (unsigned)iybin<=fitset->ny; ++iybin, ++ipoint)
			    {
				thispoint[ipoint].x = x;
				thispoint[ipoint].y = fitset->ymin + 
				    ((double)iybin/dybins)*dyrange;
				if (thispoint[ipoint].value == 0.0)
				    thispoint[ipoint].filtered = 1;
			    }
			}
			break;

		    case 3:
			ipoint = 0;
			for (ixbin=0; (unsigned)ixbin<fitset->nx; ++ixbin)
			{
			    x = fitset->xmin + halfbin_x + ((double)ixbin/dxbins)*dxrange;
			    for (iybin=0; (unsigned)iybin<fitset->ny; ++iybin)
			    {
				y = fitset->ymin + halfbin_y + ((double)iybin/dybins)*dyrange;
				for (izbin=0; (unsigned)izbin<fitset->nz; ++izbin, ++ipoint)
				{
				    fitset->iweights[ipoint].x = x;
				    fitset->iweights[ipoint].y = y;
				    fitset->iweights[ipoint].z = fitset->zmin + 
					halfbin_z + ((double)izbin/dzbins)*dzrange;
				    if (fitset->filter)
					if (!(fitset->filter)(fitset->iweights[ipoint].x, 
							      fitset->iweights[ipoint].y,
							      fitset->iweights[ipoint].z))
					    fitset->iweights[ipoint].filtered = 1;
				    if (!fitset->iweights[ipoint].filtered)
				    {
					fitset->iweights[ipoint].value = 2.0/3.0;
					thispoint[ixbin*(fitset->nz+1)*(fitset->ny+1)+
						 iybin*(fitset->nz+1)+izbin].value += 1.0/24.0;
					thispoint[ixbin*(fitset->nz+1)*(fitset->ny+1)+
						 iybin*(fitset->nz+1)+izbin+1].value += 1.0/24.0;
					thispoint[ixbin*(fitset->nz+1)*(fitset->ny+1)+
						 (iybin+1)*(fitset->nz+1)+izbin].value += 1.0/24.0;
					thispoint[ixbin*(fitset->nz+1)*(fitset->ny+1)+
						 (iybin+1)*(fitset->nz+1)+izbin+1].value += 1.0/24.0;
					thispoint[(ixbin+1)*(fitset->nz+1)*(fitset->ny+1)+
						 iybin*(fitset->nz+1)+izbin].value += 1.0/24.0;
					thispoint[(ixbin+1)*(fitset->nz+1)*(fitset->ny+1)+
						 iybin*(fitset->nz+1)+izbin+1].value += 1.0/24.0;
					thispoint[(ixbin+1)*(fitset->nz+1)*(fitset->ny+1)+
						 (iybin+1)*(fitset->nz+1)+izbin].value += 1.0/24.0;
					thispoint[(ixbin+1)*(fitset->nz+1)*(fitset->ny+1)+
						 (iybin+1)*(fitset->nz+1)+izbin+1].value += 1.0/24.0;
				    }
				}
			    }
			}
			ipoint = 0;
			for (ixbin=0; (unsigned)ixbin<=fitset->nx; ++ixbin)
			{
			    x = fitset->xmin + ((double)ixbin/dxbins)*dxrange;
			    for (iybin=0; (unsigned)iybin<=fitset->ny; ++iybin)
			    {
				y = fitset->ymin + ((double)iybin/dybins)*dyrange;
				for (izbin=0; (unsigned)izbin<=fitset->nz; ++izbin, ++ipoint)
				{
				    thispoint[ipoint].x = x;
				    thispoint[ipoint].y = y;
				    thispoint[ipoint].z = fitset->zmin +
					((double)izbin/dzbins)*dzrange;
				    if (thispoint[ipoint].value == 0.0)
					thispoint[ipoint].filtered = 1;
				}
			    }
			}
			break;

		    default:
			assert(0);
		    }
		}
	    }
	}
    }
    else if (*iflag == 2)
    {
	/* Gradient expected. Skip the calculation if there was
	 * a problem with initialization or with previous calls.
	 */
	if (fit_info->status == TCL_OK)
	{
	    /* Real gradient calculation will go here at some point */
	    fit_info->status = TCL_ERROR;
	    Tcl_SetResult(fit_info->interp,
			  "Gradient calculation is not supported",
			  TCL_VOLATILE);
	    memset(grad, 0, *npar*sizeof(double));
	}
	else
	    memset(grad, 0, *npar*sizeof(double));
    }

    /* Skip the function calculation if there was a problem
     * with initialization or with previous calls.
     */
    if (fit_info->status == TCL_OK)
    {
	/* Map the function parameters */
	for (set=0; set<fit_info->nsets; ++set)
	{
	    fitset = fit_info->fitsets[set];
	    if (fitset == NULL) continue;
	    for (ifun=0; ifun<fitset->nfitters; ++ifun)
	    {
		assert(fitset->pfitters[ifun]->map);
		fitset->pfitters[ifun]->local_pars = 
		    (fitset->pfitters[ifun]->map)(
			xval, fitset->pfitters[ifun]->offset);
	    }
	}

	/* Calculate the function value */
	for (set=0; set<fit_info->nsets; ++set)
	{
	    fitset = fit_info->fitsets[set];
	    if (fitset == NULL) continue;
	    sum = 0.0;
	    fitset->datapoints = 0;
	    weight = fitset->weight;
	    futil = fitset->acc_method;
	    is_binned = fitset->binned;
	    item_type = fitset->item_type;
	    assert(item_type == HS_1D_HISTOGRAM ||
		   item_type == HS_2D_HISTOGRAM ||
		   item_type == HS_3D_HISTOGRAM ||
		   item_type == HS_NTUPLE);
	    
	    /* Obtain function values at bin centers/edges/corners (if needed) */
	    aux_fvalues = (item_type == HS_1D_HISTOGRAM || 
			   item_type == HS_2D_HISTOGRAM ||
			   item_type == HS_3D_HISTOGRAM ||
			   (!is_binned && futil == acc_ext_max_likelihood));
	    if (aux_fvalues)
	    {
		if (fitset->nweights <= 0)
		{
		    if (!is_binned && futil == acc_ext_max_likelihood)
		    {
			sprintf(charbuf, "%d", set);
			Tcl_AppendResult(fit_info->interp, "Can't perform the fit: ",
					 "normalization region is not defined for subset ",
					 charbuf, NULL);
			fit_info->status = TCL_ERROR;
			break;
		    }
		    else
			assert(0);
		}
		for (ipoint=0; ipoint<fitset->nweights; ++ipoint)
		{
		    thispoint = fitset->iweights + ipoint;
		    if (thispoint->filtered) continue;
		    thispoint->errflag = 0;
		    dfval = 0.0;
		    for (ifun=0; ifun<fitset->nfitters; ++ifun)
		    {
			fitter = fitset->pfitters[ifun]->fitter;
			if (fitter->fit_c)
			{
			    dfval += (fitter->fit_c)(thispoint->x, thispoint->y,
						     thispoint->z, fitter->mode,
						     fitset->pfitters[ifun]->local_pars, &ierr);
			}
			else
			{
			    fmode = fitter->mode;
			    x = thispoint->x;
			    y = thispoint->y;
			    z = thispoint->z;
			    dfval += (fitter->fit_f)(&x, &y, &z, &fmode,
						     fitset->pfitters[ifun]->local_pars, &ierr);
			}
			if (ierr)
			{
			    if (!fit_info->ignore_function_errors)
				report_function_error(fit_info, set, ifun,
						      thispoint, xval, ierr);
			    ierr = 0;
			    thispoint->errflag = 1;
			}
		    }
		    if (thispoint->errflag)
			thispoint->fit = 0.0;
		    else
		    {
			thispoint->fit = dfval;
			sum += dfval*thispoint->value;
		    }
		}
	    }

	    /* Cycle over bin centers */
	    for (ipoint=0; ipoint<fitset->npoints; ++ipoint)
	    {
		thispoint = fitset->points + ipoint;
		if (thispoint->filtered) continue;
		if (item_type == HS_1D_HISTOGRAM)
		{
		    /* Function values have already been estimated */
		    nbins = fitset->nx;
		    thispoint->errflag = fitset->iweights[ipoint].errflag ||
			fitset->iweights[nbins+ipoint+1].errflag ||
			fitset->iweights[nbins+ipoint].errflag;
		    if (thispoint->errflag)
			thispoint->fit = 0.0;
		    else
			thispoint->fit = (4.0*fitset->iweights[ipoint].fit +
					  fitset->iweights[nbins+ipoint+1].fit +
					  fitset->iweights[nbins+ipoint].fit)/6.0;
		}
		else if (item_type == HS_2D_HISTOGRAM)
		{
		    ixbin = ipoint / fitset->ny;
		    iybin = ipoint % fitset->ny;
		    ibin = ixbin * (fitset->ny + 1) + iybin;
		    nbins = fitset->nx*fitset->ny;
		    thispoint->errflag = fitset->iweights[ipoint].errflag ||
			fitset->iweights[nbins+ibin].errflag || 
			fitset->iweights[nbins+ibin+1].errflag ||
			fitset->iweights[nbins+ibin+fitset->ny+1].errflag || 
			fitset->iweights[nbins+ibin+fitset->ny+2].errflag;
		    if (thispoint->errflag)
			thispoint->fit = 0.0;
		    else
  			thispoint->fit = (8.0*fitset->iweights[ipoint].fit +
  					  fitset->iweights[nbins+ibin].fit+
  					  fitset->iweights[nbins+ibin+1].fit+
  					  fitset->iweights[nbins+ibin+fitset->ny+1].fit+
  					  fitset->iweights[nbins+ibin+fitset->ny+2].fit)/12.0;
		}
		else if (item_type == HS_3D_HISTOGRAM)
		{
		    ixbin = ipoint / (fitset->ny*fitset->nz);
		    iybin = (ipoint % (fitset->ny*fitset->nz)) / fitset->nz;
		    izbin = (ipoint % (fitset->ny*fitset->nz)) % fitset->nz;
		    nbins = fitset->nx*fitset->ny*fitset->nz;
		    ibin = ixbin*(fitset->nz+1)*(fitset->ny+1)+iybin*(fitset->nz+1)+izbin;
		    jbin = ibin + (fitset->nz+1)*(fitset->ny+1);
		    thispoint->errflag = fitset->iweights[ipoint].errflag ||
			fitset->iweights[nbins+ibin].errflag ||
			fitset->iweights[nbins+ibin+1].errflag ||
			fitset->iweights[nbins+ibin+fitset->nz+1].errflag ||
			fitset->iweights[nbins+ibin+fitset->nz+2].errflag ||
			fitset->iweights[nbins+jbin].errflag ||
			fitset->iweights[nbins+jbin+1].errflag ||
			fitset->iweights[nbins+jbin+fitset->nz+1].errflag ||
			fitset->iweights[nbins+jbin+fitset->nz+2].errflag;
		    if (thispoint->errflag)
			thispoint->fit = 0.0;
		    else
		    {
			thispoint->fit = (
			    16.0*fitset->iweights[ipoint].fit +
			    fitset->iweights[nbins+ibin].fit +
			    fitset->iweights[nbins+ibin+1].fit +
			    fitset->iweights[nbins+ibin+fitset->nz+1].fit +
			    fitset->iweights[nbins+ibin+fitset->nz+2].fit +
			    fitset->iweights[nbins+jbin].fit +
			    fitset->iweights[nbins+jbin+1].fit +
			    fitset->iweights[nbins+jbin+fitset->nz+1].fit +
			    fitset->iweights[nbins+jbin+fitset->nz+2].fit)/24.0;			
		    }		    
		}
		else
		{
		    thispoint->errflag = 0;
		    dfval = 0.0;
		    for (ifun=0; ifun<fitset->nfitters; ++ifun)
		    {
			fitter = fitset->pfitters[ifun]->fitter;
			if (fitter->fit_c)
			{
			    dfval += (fitter->fit_c)(thispoint->x, thispoint->y,
						     thispoint->z, fitter->mode,
						     fitset->pfitters[ifun]->local_pars, &ierr);
			}
			else
			{
			    fmode = fitter->mode;
			    x = thispoint->x;
			    y = thispoint->y;
			    z = thispoint->z;
			    dfval += (fitter->fit_f)(&x, &y, &z, &fmode,
						     fitset->pfitters[ifun]->local_pars, &ierr);
			}
			if (ierr)
			{
			    if (!fit_info->ignore_function_errors)
				report_function_error(fit_info, set, ifun,
						      thispoint, xval, ierr);
			    ierr = 0;
			    thispoint->errflag = 1;
			}
		    }
		    if (thispoint->errflag)
			thispoint->fit = 0.0;
		    else
			thispoint->fit = dfval;
		}
		if (!thispoint->errflag)
		{
		    ++fitset->datapoints;
		    accum += weight * futil(thispoint->fit, thispoint, is_binned, &ierr);
		    if (ierr)
		    {
			if (!fit_info->ignore_function_errors)
			    report_accum_error(fit_info, set,
					       thispoint, xval, ierr);
			ierr = 0;
		    }
		}
	    }

	    /* If necessary, take into account the overall normalization */
	    if (!is_binned && futil == acc_ext_max_likelihood)
	    {
		sum *= ((fitset->xmax - fitset->xmin)/(double)fitset->nx);
		if (fitset->ndim > 1)
		    sum *= ((fitset->ymax - fitset->ymin)/(double)fitset->ny);
		if (fitset->ndim > 2)
		    sum *= ((fitset->zmax - fitset->zmin)/(double)fitset->nz);
		accum += 2.0 * weight * sum;
	    }
	}
    }
    *fval = accum;

    if (*iflag == 4 || *iflag == 2)
    {
	/* This is the Minuit main loop. The fit parameters
	   are going to loose synchronization with Minuit. */
	fit_info->par_synched = 0;

	if (fit_info->status == TCL_OK && fit_info->timeout > 0)
	{
	    if (timeout == 0)
	    {
		/* Check if we need to set up a timeout */
		last_time = time(NULL);
		call_counter = 0;
		call_step = 10;
		next_check = call_counter + call_step;
		timeout = last_time + fit_info->timeout;
	    }
	    else
	    {
		/* Check if we need to react on a timeout */
		if (++call_counter == next_check)
		{
		    dt = time(NULL) - last_time;
		    last_time += dt;
		    if (dt == 0)
			call_step += (call_step/2);
		    else if (dt > 1)
		    {
			call_step -= (call_step/2);
			if (call_step < 2)
			    call_step = 2;
		    }
		    next_check = call_counter + call_step;
		    if (last_time >= timeout)
		    {
			sprintf(charbuf, "%d", (int)(fit_info->timeout + 
				(last_time - timeout)));
			Tcl_AppendResult(fit_info->interp,
					 "Fit procedure timed out after ",
					 charbuf, " seconds, ", NULL);
			sprintf(charbuf, "%d", call_counter);
			Tcl_AppendResult(fit_info->interp, charbuf,
					 " minimization steps\n", NULL);
			fit_info->status = TCL_ERROR;
			timeout = 0;
                        {
                            int maxcalls = 1;
                            mnsetnfcnmx_(&maxcalls);
                        }
		    }
		}
	    }
	}
    }
    else
    {
	/* We are not called from the Minuit main loop.
	 * Update the function parameter values (but not Minuit 
	 * parameters -- those will be resynchronized at
	 * the end of the fit).
	 */
	remember_function_params(fit_info, xval);

	/* Remove the timeout */
	timeout = 0;
    }

    if (*iflag == 3)
    {
	/* End of fit */
	fit_info->n_variable_pars = *npar;
	remember_fit_results(fit_info);
	if (fit_info->status == TCL_OK)
	    hs_declare_fit_complete(fit_info, 1);
    }

    /* Process tcl events if it appears safe to do so.
     * This must be the last operation before we return,
     * just in case we are called again from it.
     */
    if (fit_info->status == TCL_OK)
	while (hs_minuit_is_locked())
        {
            /* Do not process file events -- may be bad
               if readline callback is not installed */
	    if (Tcl_DoOneEvent(TCL_DONT_WAIT |
                               TCL_WINDOW_EVENTS |
                               TCL_TIMER_EVENTS |
                               TCL_IDLE_EVENTS) == 0)
		break;
        }
}

Accumulated_stat_function * hs_get_accum_method_by_tag(const char *fcn_tag)
{
    const Accumulator_description * pacc;

    pacc = get_accum_descr_by_tag(fcn_tag);
    if (pacc == NULL)
	return NULL;
    else
	return pacc->fcn;
}

int hs_accum_if_errors_required(const char *fcn_tag)
{
    const Accumulator_description * pacc;

    pacc = get_accum_descr_by_tag(fcn_tag);
    if (pacc == NULL)
	return -1;
    else
	return pacc->errors_required;
}

int hs_accum_if_binning_required(const char *fcn_tag)
{
    const Accumulator_description * pacc;

    pacc = get_accum_descr_by_tag(fcn_tag);
    if (pacc == NULL)
	return -1;
    else
	return pacc->binning_required;
}

void hs_show_valid_fit_fcns(Tcl_Interp *interp)
{
    int i, n, len, maxlen;
    char stringbuf[32];
    char format[16];

    n = sizeof(known_fcns)/sizeof(known_fcns[0]);
    maxlen = 0;
    for (i=0; i<n; ++i)
    {
	len = strlen(known_fcns[i].tag);
	if (len > maxlen)
	    maxlen = len;
    }
    sprintf(format, "  %%%ds", maxlen);
    for (i=0; i<n; ++i)
    {
	sprintf(stringbuf, format, known_fcns[i].tag);
	Tcl_AppendResult(interp, stringbuf, " -- ",
			 known_fcns[i].fullname, NULL);
	if (i < n-1)
	    Tcl_AppendResult(interp, "\n", NULL);
    }
}

void hs_show_valid_accum_methods(Tcl_Interp *interp)
{
    int i, n, len, maxlen;
    char stringbuf[32];
    char format[16];

    n = sizeof(known_acc)/sizeof(known_acc[0]);
    maxlen = 0;
    for (i=0; i<n; ++i)
    {
	len = strlen(known_acc[i].tag);
	if (len > maxlen)
	    maxlen = len;
    }
    sprintf(format, "  %%%ds", maxlen);
    for (i=0; i<n; ++i)
    {
	sprintf(stringbuf, format, known_acc[i].tag);
	Tcl_AppendResult(interp, stringbuf, " -- ",
			 known_acc[i].fullname, NULL);
	if (i < n-1)
	    Tcl_AppendResult(interp, "\n", NULL);
    }
}

Minuit_stat_accumulator(acc_leastsq_with_errors)
{
    double diff = 0.0;

    assert(binned);
    if (point->error > 0.0)
	diff = (fval - point->value)/point->error;
    else if (point->error == 0.0)
	*ierr = ACC_ZERO_POINT_ERROR;
    else
	*ierr = ACC_NEG_POINT_ERROR;
    return diff*diff;
}

Minuit_stat_accumulator(acc_leastsq_no_errors)
{
    double diff;

    assert(binned);
    diff = fval - point->value;
    return diff*diff;
}

Minuit_stat_accumulator(acc_pearsons_chisq)
{
    double diff = 0.0;

    assert(binned);
    if (point->value < 0.0)
	*ierr = ACC_NEG_POINT_ERROR;
    else if (fval > 0.0)
	diff = (fval - point->value)/sqrt(fval);
    else if (fval < 0.0)
	*ierr = ACC_NEG_PDF_VALUE;
    else if (point->value > 0.0)
	*ierr = ACC_ZERO_PDF_VALUE;
    return diff*diff;
}

Minuit_stat_accumulator(acc_max_likelihood)
{
    if (binned)
    {
	if (point->value < 0.0)
	{
	    *ierr = ACC_NEG_POINT_ERROR;
	    return 0.0;
	}
	else if (fval > 0.0)
	{
	    return -2.0 * (point->value*log(fval));
	}
	else
	{
	    if (fval == 0.0)
	    {
		if (point->value > 0.0)
		    *ierr = ACC_ZERO_PDF_VALUE;
	    }
	    else
	    {
		*ierr = ACC_NEG_PDF_VALUE;
	    }
	    return (-2.0 * point->value * DBL_MIN_EXP);
	}
    }
    else
    {
	if (fval > 0.0)
	    return (-2.0 * log(fval));
	else if (fval == 0.0)
	    *ierr = ACC_ZERO_PDF_VALUE;
	else
	    *ierr = ACC_NEG_PDF_VALUE;
	return (-2.0 * DBL_MIN_EXP);
    }
}

Minuit_stat_accumulator(acc_ext_max_likelihood)
{
    if (binned)
    {
	if (point->value < 0.0)
	{
	    *ierr = ACC_NEG_POINT_ERROR;
	    if (fval > 0.0)
		return 2.0*fval;
	    else
		return 0.0;
	}
	else if (fval > 0.0)
	{
	    return -2.0 * (point->value*log(fval) - fval);
	}
	else
	{
	    if (fval == 0.0)
	    {
		if (point->value > 0.0)
		    *ierr = ACC_ZERO_PDF_VALUE;
	    }
	    else
	    {
		*ierr = ACC_NEG_PDF_VALUE;
	    }
	    return (-2.0 * point->value * DBL_MIN_EXP);
	}
    }
    else
    {
	if (fval > 0.0)
	    return (-2.0 * log(fval));
	else if (fval == 0.0)
	    *ierr = ACC_ZERO_PDF_VALUE;
	else
	    *ierr = ACC_NEG_PDF_VALUE;
	return (-2.0 * DBL_MIN_EXP);
    }
}

static void report_accum_error(Fit_config *fit_info,
			       int set, DataPoint *point,
			       const double *minuit_pars, int ierr)
{
    int k;
    char stringbuf[64];
    Tcl_Interp *interp;
    Fit_subset *fitset;

    if (fit_info->status == TCL_OK)
    {
	assert(ierr < N_ACC_ERROR_CODES && ierr > 0);
	fit_info->status = TCL_ERROR;
	interp = fit_info->interp;
	assert(interp);
	fitset = fit_info->fitsets[set];
	assert(fitset);
	sprintf(stringbuf, "%d", set);
	Tcl_AppendResult(interp, "Invalid fit statistic value ",
			 "(", acc_error_description[ierr],
			 ") for dataset ", stringbuf, " at\n", NULL);
	Tcl_PrintDouble(interp, point->x, stringbuf);
	Tcl_AppendResult(interp, "x = ", stringbuf, "\n", NULL);
	if (fitset->ndim > 1)
	{
	    Tcl_PrintDouble(interp, point->y, stringbuf);
	    Tcl_AppendResult(interp, "y = ", stringbuf, "\n", NULL);
	}
	if (fitset->ndim > 2)
	{
	    Tcl_PrintDouble(interp, point->z, stringbuf);
	    Tcl_AppendResult(interp, "z = ", stringbuf, "\n", NULL);
	}
	for (k=0; k<fit_info->n_minuit_pars; ++k)
	{
	    Tcl_PrintDouble(interp, minuit_pars[k], stringbuf);
	    Tcl_AppendResult(interp, fit_info->minuit_pars[k].name,
			 " = ", stringbuf, "\n", NULL);
	}
	Tcl_AppendResult(interp, "The fitted data come from ", NULL);
	switch (hs_type(fitset->id))
	{
	case HS_1D_HISTOGRAM:
	    Tcl_AppendResult(interp, "1d histogram", NULL);
	    break;
	case HS_2D_HISTOGRAM:
	    Tcl_AppendResult(interp, "2d histogram", NULL);
	    break;
	case HS_3D_HISTOGRAM:
	    Tcl_AppendResult(interp, "3d histogram", NULL);
	    break;
	case HS_NTUPLE:
	    Tcl_AppendResult(interp, "ntuple", NULL);
	    break;
	case HS_NONE:
	    Tcl_AppendResult(interp, "deleted item", NULL);
	    break;
	default:
	    assert(0);
	}
	sprintf(stringbuf, "%d", fitset->id);
	Tcl_AppendResult(interp, " with Histo-Scope id ",
			 stringbuf, "\n", NULL);
    }
}

static void report_function_error(Fit_config *fit_info, int set,
				  int fnum, DataPoint *point,
				  const double *minuit_pars, int ierr)
{
    char stringbuf[64];
    int k, npars;
    char *fname;
    Minuit_fitter_info *value;
    const double *pars;
    Tcl_Interp *interp;
    Fit_subset *fitset;

    if (fit_info->status == TCL_OK)
    {
	fit_info->status = TCL_ERROR;
	interp = fit_info->interp;
	assert(interp);
	fitset = fit_info->fitsets[set];
	assert(fitset);
	fname = fitset->pfitters[fnum]->fitter_tag;
	value = fitset->pfitters[fnum]->fitter;
	pars = fitset->pfitters[fnum]->local_pars;
	npars = fitset->pfitters[fnum]->npars;
	sprintf(stringbuf, "%d", set);
	Tcl_AppendResult(interp, "Failed to evaluate dataset ",
			 stringbuf, " function ", fname, NULL);
	sprintf(stringbuf, "%d", ierr);
	Tcl_AppendResult(interp, " (error code ", stringbuf, ") for\n", NULL);
	sprintf(stringbuf, "%d", value->mode);
	Tcl_AppendResult(interp, "mode = ", stringbuf, "\n", NULL);
	Tcl_PrintDouble(interp, point->x, stringbuf);
	Tcl_AppendResult(interp, "x = ", stringbuf, "\n", NULL);
	if (value->ndim > 1)
	{
	    Tcl_PrintDouble(interp, point->y, stringbuf);
	    Tcl_AppendResult(interp, "y = ", stringbuf, "\n", NULL);
	}
	if (value->ndim > 2)
	{
	    Tcl_PrintDouble(interp, point->z, stringbuf);
	    Tcl_AppendResult(interp, "z = ", stringbuf, "\n", NULL);
	}
	for (k=0; k<npars; ++k)
	{
	    Tcl_PrintDouble(interp, pars[k], stringbuf);
	    Tcl_AppendResult(interp, value->param_names[k],
			 " = ", stringbuf, "\n", NULL);
	}
	Tcl_AppendResult(interp, "The fitted data come from ", NULL);
	switch (hs_type(fitset->id))
	{
	case HS_1D_HISTOGRAM:
	    Tcl_AppendResult(interp, "1d histogram", NULL);
	    break;
	case HS_2D_HISTOGRAM:
	    Tcl_AppendResult(interp, "2d histogram", NULL);
	    break;
	case HS_3D_HISTOGRAM:
	    Tcl_AppendResult(interp, "3d histogram", NULL);
	    break;
	case HS_NTUPLE:
	    Tcl_AppendResult(interp, "ntuple", NULL);
	    break;
	case HS_NONE:
	    Tcl_AppendResult(interp, "deleted item", NULL);
	    break;
	default:
	    assert(0);
	}
	sprintf(stringbuf, "%d", fitset->id);
	Tcl_AppendResult(interp, " with Histo-Scope id ",
			 stringbuf, "\n", NULL);
    }
}

static void remember_function_params(Fit_config *fit_info, const double *xval)
{
    int ifun, npars;
    
    for (ifun=0; ifun<fit_info->nfitters; ++ifun)
    {
	fit_info->fitters[ifun]->local_pars = 
	    (fit_info->fitters[ifun]->map)(
		xval, fit_info->fitters[ifun]->offset);
	npars = fit_info->fitters[ifun]->npars;
	if (fit_info->fitters[ifun]->best_pars == NULL)
	    fit_info->fitters[ifun]->best_pars = 
		(double *)malloc(npars*sizeof(double));
	if (fit_info->fitters[ifun]->best_pars == NULL)
	{
	    Tcl_ResetResult(fit_info->interp);
	    Tcl_SetResult(fit_info->interp, "out of memory", TCL_STATIC);
	    fit_info->status = TCL_ERROR;
	}
	else
	    memcpy(fit_info->fitters[ifun]->best_pars,
		   fit_info->fitters[ifun]->local_pars,
		   npars*sizeof(double));
    }
}

static void remember_fit_results(Fit_config *fit_info)
{
    int i, set, ofit, oval, nused, NUM, IVARBL, NDIM;
    char CHNAM[MINUIT_PARAM_NAME_LEN+1];
    double VAL, ERROR, BND1, BND2, EPARAB, EPLUS, EMINUS, GLOBCC;
    double diff, chisq;
    DataPoint *data;
    Minimization_status *ps;
    Minuit_parameter *param;
    Accumulated_stat_function *acc;

    /* Basic fit statistics */
    if (fit_info->status == TCL_OK)
    {
	ofit = offsetof(DataPoint, fit);
	oval = offsetof(DataPoint, value);
	for (set=0; set<fit_info->nsets; ++set)
	{
	    if (fit_info->fitsets[set] == NULL)
		continue;

	    /* Statistics for the data */
	    data = fit_info->fitsets[set]->points;
	    if (!fit_info->fitsets[set]->binned)
		for (i=0; i<fit_info->fitsets[set]->npoints; ++i)
		    data[i].value = 1.0;
	    hs_data_point_stats(data, fit_info->fitsets[set]->npoints,
				fit_info->fitsets[set]->ndim, oval,
				&fit_info->fitsets[set]->data_stats);

	    /* Check that the number of points used came out correct */
	    assert((unsigned)(fit_info->fitsets[set]->data_stats.goodpoints) == 
		   (unsigned)(fit_info->fitsets[set]->datapoints));

	    /* Statistics for the function */
	    if ((fit_info->fitsets[set]->acc_method == acc_ext_max_likelihood && 
		 !fit_info->fitsets[set]->binned) ||
		fit_info->fitsets[set]->item_type == HS_1D_HISTOGRAM ||
		fit_info->fitsets[set]->item_type == HS_2D_HISTOGRAM ||
		fit_info->fitsets[set]->item_type == HS_3D_HISTOGRAM)
	    {
		/* Combine integration weights and fit values */
		data = fit_info->fitsets[set]->iweights;
		for (i=0; i<fit_info->fitsets[set]->nweights; ++i)
		    data[i].fit *= data[i].value;
		hs_data_point_stats(data, fit_info->fitsets[set]->nweights,
				    fit_info->fitsets[set]->ndim, ofit,
				    &fit_info->fitsets[set]->fit_stats);
		/* Multiply the resulting sum by the bin size for unbinned fit */
		if (!fit_info->fitsets[set]->binned)
		{
		    fit_info->fitsets[set]->fit_stats.integ *= 
			((fit_info->fitsets[set]->xmax-fit_info->fitsets[set]->xmin)/
			 (double)fit_info->fitsets[set]->nx);
		    if (fit_info->fitsets[set]->ndim > 1)
			fit_info->fitsets[set]->fit_stats.integ *= 
			    ((fit_info->fitsets[set]->ymax-fit_info->fitsets[set]->ymin)/
			     (double)fit_info->fitsets[set]->ny);
		    if (fit_info->fitsets[set]->ndim > 2)
			fit_info->fitsets[set]->fit_stats.integ *= 
			    ((fit_info->fitsets[set]->zmax-fit_info->fitsets[set]->zmin)/
			     (double)fit_info->fitsets[set]->nz);
		}
	    }
	    else if (fit_info->fitsets[set]->binned)
	    {
		hs_data_point_stats(data, fit_info->fitsets[set]->npoints,
				    fit_info->fitsets[set]->ndim, ofit,
				    &fit_info->fitsets[set]->fit_stats);
	    }

	    /* Calculate chi-square */
	    if (fit_info->fitsets[set]->binned)
	    {
		chisq = 0.0;
		nused = 0;
		acc = fit_info->fitsets[set]->acc_method;
		data = fit_info->fitsets[set]->points;
		for (i=0; i<fit_info->fitsets[set]->npoints; ++i)
		{
		    if (data[i].filtered || data[i].errflag)
			continue;
		    diff = data[i].value - data[i].fit;
		    if (acc == acc_leastsq_with_errors)
		    {
			diff /= data[i].error;
			chisq += diff*diff;
			++nused;
		    }
		    else if (acc == acc_leastsq_no_errors)
		    {
			chisq += diff*diff;
			++nused;
		    }
		    else if (acc == acc_max_likelihood ||
			     acc == acc_pearsons_chisq ||
			     acc == acc_ext_max_likelihood)
		    {
			if (data[i].value > 0.0 && data[i].fit > 0.0)
			{
			    chisq += diff*diff/data[i].fit;
			    ++nused;
			}
		    }
		    else
		    {
			/* Unknown stats accumulator -- this should not happen */
			assert(0);
		    }
		}
		fit_info->fitsets[set]->chisq_used_points = nused;
		fit_info->fitsets[set]->chisq = chisq;
	    }
	}
    }

    /* Fit status */
    ps = &fit_info->ministat;
    mnstat_(&ps->fmin, &ps->fedm, &ps->errdef,
	    &ps->npari, &ps->nparx, &ps->istat);

    /* Fit parameters */
    for (i=0; i<fit_info->n_minuit_pars; ++i)
    {
	NUM = i+1;
	mnpout_(&NUM, CHNAM, &VAL, &ERROR, &BND1, &BND2,
		&IVARBL, MINUIT_PARAM_NAME_LEN);
	CHNAM[MINUIT_PARAM_NAME_LEN] = '\0';
	assert(IVARBL >= 0);
	param = fit_info->minuit_pars + i;
	assert(hs_minuit_par_names_cmp(param->name, CHNAM) == 0);
	param->minuit_num = IVARBL;
	param->value = VAL;
	param->fixed = (IVARBL == 0);
	if (param->fixed)
	{
	    param->eparab = 0.0;
	    param->eminus = 0.0;
	    param->eplus = 0.0;
	    param->globcc = 0.0;
	}
	else
	{
            /* Minuit has a bug: it does not return correct
             * bounds for fixed parameters. Therefore, take
             * the bounds from Minuit only if the parameter
             * is not fixed.
             */
            param->has_bounds = (BND1 != 0.0 || BND2 != 0.0);
            if (param->has_bounds)
            {
                param->lolim = BND1;
                param->hilim = BND2;
            }
	    param->step = ERROR;
	    NUM = i+1;
	    mnerrs_(&NUM, &EPLUS, &EMINUS, &EPARAB, &GLOBCC);
	    param->eparab = EPARAB;
	    param->eminus = EMINUS;
	    param->eplus = EPLUS;
	    param->globcc = GLOBCC;
	}
    }
    fit_info->par_synched = 1;

    /* Retrieve the parameter error matrix */
    checkfree(fit_info->emat);
    checkfree(fit_info->ematind);
    if (ps->istat > 0)
    {
	NDIM = fit_info->n_variable_pars;
	assert(NDIM <= fit_info->n_minuit_pars);
	if (NDIM > 0)
	{
	    fit_info->emat = (double *)malloc(NDIM * NDIM * sizeof(double));
	    fit_info->ematind = (int *)malloc(NDIM * sizeof(int));
	    if (fit_info->emat == NULL || fit_info->ematind == NULL)
	    {
		fprintf(stderr, "Fatal error: out of memory. Exiting.\n");
		exit(EXIT_FAILURE);
	    }
	    mnemat_(fit_info->emat, &NDIM);
	    NDIM = fit_info->n_variable_pars;
	    for (i=0; i<fit_info->n_minuit_pars; ++i)
	    {
		param = fit_info->minuit_pars + i;
		assert(param->minuit_num <= NDIM);
		if (param->minuit_num > 0)
		    fit_info->ematind[param->minuit_num-1] = i;
	    }
	}
    }
    else
	fit_info->n_variable_pars = 0;
}

const char * hs_get_current_fcn_tag(void)
{
    return current_fcn_tag;
}

Minuit_user_function * hs_get_current_fit_fcn(void)
{
    return current_fcn;
}

Minuit_user_function * hs_set_fit_fcn_by_tag(char *tag)
{
    Minuit_user_function * tmp;
    
    tmp = hs_get_fit_method_by_tag(tag);
    if (tmp)
    {
	checkfree(current_fcn_tag);
	current_fcn_tag = strdup(tag);
	assert(current_fcn_tag);
	current_fcn = tmp;
    }
    return tmp;
}

static Minuit_user_function * hs_get_fit_method_by_tag(const char *fcn_tag)
{
    int i, n;

    n = sizeof(known_fcns)/sizeof(known_fcns[0]);
    for (i=0; i<n; ++i)
	if (strcmp(known_fcns[i].tag, fcn_tag) == 0)
	    return known_fcns[i].fcn;
    return NULL;
}

static const Accumulator_description *
get_accum_descr_by_tag(const char *fcn_tag)
{
    int i, n;

    n = sizeof(known_acc)/sizeof(known_acc[0]);
    for (i=0; i<n; ++i)
	if (strcmp(known_acc[i].tag, fcn_tag) == 0)
	    return known_acc + i;
    return NULL;
}

