#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "histoscope.h"
#include "histo_utils.h"
#include "histo_tcl_api.h"
#include "cernlib.h"
#include "tl_interface.h"

static int hs_linfit_1d(Tcl_Interp *interp, int id, int key, int use_errors);
static int hs_linfit_2d(Tcl_Interp *interp, int id, int key, int use_errors);

static char *OUT_OF_MEMORY = "out of memory";

int hs_have_cernlib(void)
{
    return 1;
}

INT_FUNCT_WITH_VOID_ARG(have_cernlib)

tcl_routine(Fortran_executable)
{
    if (objc > 1)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_SetResult(interp, HS_FC_EXECUTABLE , TCL_VOLATILE);
    return TCL_OK;
}

tcl_routine(Tcl_config_file)
{
    if (objc > 1)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_SetResult(interp, HS_SYS_TCL_CONFIG_FILE , TCL_VOLATILE);
    return TCL_OK;
}

tcl_routine(Histosope_lib_dir)
{
    if (objc > 1)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_SetResult(interp, HS_HISTO_LIB_DIR , TCL_VOLATILE);
    return TCL_OK;
}

tcl_routine(1d_linear_fit)
{
    int id, suppress_zero, use_errors = 0;
  
    if (objc < 3 || objc > 4)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    verify_1d_histo(id,1);
    if (Tcl_GetBooleanFromObj(interp, objv[2], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    if (objc > 3)
	if (Tcl_GetBooleanFromObj(interp, objv[3], &use_errors) != TCL_OK)
	    return TCL_ERROR;
    return hs_linfit_1d(interp, id, !suppress_zero, use_errors);
}

tcl_routine(2d_linear_fit)
{
    int id, suppress_zero, use_errors = 0;
  
    if (objc < 3 || objc > 4)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    verify_2d_histo(id,1);
    if (Tcl_GetBooleanFromObj(interp, objv[2], &suppress_zero) != TCL_OK)
	return TCL_ERROR;
    if (objc > 3)
	if (Tcl_GetBooleanFromObj(interp, objv[3], &use_errors) != TCL_OK)
	    return TCL_ERROR;
    return hs_linfit_2d(interp, id, !suppress_zero, use_errors);
}

static int hs_linfit_1d(Tcl_Interp *interp, int id, int key, int use_errors)
{
    char stringbuf[100];
    int i, nbins, ndof = -2;
    int status = TCL_ERROR;
    float xmin, xmax, *errpos, *data = NULL;
    double x, dmin, dmax, dbins, drange, a, b, siga, sigb, corr, w, chisq;
    double s = 0.0, sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0, syy = 0.0;
    Tcl_Obj *results[7];

    nbins = hs_1d_hist_num_bins(id);
    if (nbins < 2)
    {
	sprintf(stringbuf,
		"can't fit histogram %d: number of bins is too small", id);
	Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
	return TCL_ERROR;
    }
    if ((data = (float *)malloc(2*nbins*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    errpos = data + nbins;
    dbins = (double)nbins;

    hs_1d_hist_range(id, &xmin, &xmax);
    dmin = xmin;
    dmax = xmax;
    drange = dmax - dmin;

    /* Use bin centers */
    dmin += (drange/dbins/2.0);

    /* Get the data */
    hs_1d_hist_bin_contents(id, data);

    /* Get the errors if necessary */
    if (use_errors)
    {
	if (hs_1d_hist_errors(id, errpos, NULL) == HS_NO_ERRORS)
	{
	    sprintf(stringbuf,
		    "can't fit histogram %d: it has no errors", id);
	    Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
	    goto fail;
	}

	/* Accumulate the sums */
	for (i=0; i<nbins; ++i)
	{
	    if (errpos[i] < 0.f)
	    {
		sprintf(stringbuf,
		"can't fit histogram %d: found a negative error value", id);
		Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
		goto fail;
	    }
	    else if ((key || data[i]) && errpos[i] > 0.f)
	    {
		++ndof;
		x = dmin + ((double)i/dbins)*drange;
		w = 1.0/errpos[i];
		w *= w;
		s += w;
		sx += x * w;
		sy += data[i] * w;
		sxx += x * w * x;
		sxy += data[i] * w * x;
		syy += data[i] * w * data[i];
	    }
	}
    }
    else
    {
	/* Accumulate the sums */
	for (i=0; i<nbins; ++i)
	{
	    if (key || data[i])
	    {
		++ndof;
		x = dmin + ((double)i/dbins)*drange;
		sx += x;
		sy += data[i];
		sxx += x * x;
		sxy += data[i] * x;
		syy += data[i] * (double)data[i];
	    }
	}
	s = (double)(ndof + 2);
    }

    /* Convert sums into parameter values */
    if (ndof < 0)
    {
	Tcl_SetResult(interp, "not enough data points", TCL_VOLATILE);
	goto fail;
    }
    w = s*sxx - sx*sx;
    a = (s*sxy - sx*sy)/w;
    b = (sxx*sy - sx*sxy)/w;
    if (ndof > 0)
    {
	chisq = (s*syy-sy*sy-w*a*a)/s;
	if (chisq < 0.0)
	    chisq = 0.0;
    }
    else
	chisq = 0.0;
    if (chisq > 0.0)
    {
	siga = sqrt(s/w);
	sigb = sqrt(sxx/w);
	if (!use_errors)
	{
	    w = sqrt(chisq/ndof);
	    siga *= w;
	    sigb *= w;
	}
	corr = -sx/sqrt(s*sxx);
	if (corr > 1.0)
	    corr = 1.0;
	else if (corr < -1.0)
	    corr = -1.0;
    }
    else
    {
	siga = 0.0;
	sigb = 0.0;
	corr = 0.0;
    }

    results[0] = Tcl_NewDoubleObj(a);
    results[1] = Tcl_NewDoubleObj(b);
    results[2] = Tcl_NewDoubleObj(siga);
    results[3] = Tcl_NewDoubleObj(sigb);
    results[4] = Tcl_NewDoubleObj(corr);
    results[5] = Tcl_NewDoubleObj(chisq);
    results[6] = Tcl_NewIntObj(ndof);
    Tcl_SetObjResult(interp, Tcl_NewListObj(7, results));
    status = TCL_OK;

fail:
    if (data)
	free(data);
    return status;
}

#define calculate_rho(rho,s1,s2) do {\
    if (s1 > 0.0 && s2 > 0.0)\
    {\
	rho = rho/s1/s2;\
	if (rho < -1.0) rho = -1.0;\
	else if (rho > 1.0) rho = 1.0;\
    }\
    else\
	rho = 0.0;\
} while(0);

static int hs_linfit_2d(Tcl_Interp *interp, int id, int key, int use_errors)
{
    char stringbuf[100];
    int i, j, index = 0, nbins, xbins, ybins, ndof = -3;
    int status = TCL_ERROR;
    float xmin, xmax, ymin, ymax, *errpos, *data = NULL;
    double x, y, a, b, c, s_a, s_b, s_c, r_ab, r_ac, r_bc, w, chisq;
    double dxmin, dxmax, dymin, dymax, dxbins, dybins, dxrange, dyrange;
    double s = 0.0, sx = 0.0, sy = 0.0, sz = 0.0;
    double sxx = 0.0, sxy = 0.0, syy = 0.0, szz = 0.0, sxz = 0.0, syz = 0.0;
    Tcl_Obj *results[11];

    hs_2d_hist_num_bins(id, &xbins, &ybins);
    nbins = xbins * ybins;
    if (nbins < 3)
    {
	sprintf(stringbuf,
		"can't fit histogram %d: number of bins is too small", id);
	Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
	return TCL_ERROR;
    }
    if ((data = (float *)malloc(2*nbins*sizeof(float))) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    errpos = data + nbins;
    dxbins = (double)xbins;
    dybins = (double)ybins;

    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
    dxmin = xmin;
    dxmax = xmax;
    dymin = ymin;
    dymax = ymax;
    dxrange = dxmax - dxmin;
    dyrange = dymax - dymin;
    
    /* Use bin centers */
    dxmin += (dxrange/dxbins/2.0);
    dymin += (dyrange/dybins/2.0);

    /* Get the data */
    hs_2d_hist_bin_contents(id, data);

    /* Get the errors if necessary */
    if (use_errors)
    {
	if (hs_2d_hist_errors(id, errpos, NULL) == HS_NO_ERRORS)
	{
	    sprintf(stringbuf,
		    "can't fit histogram %d: it has no errors", id);
	    Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
	    goto fail;
	}

	/* Accumulate the sums */
	for (i=0; i<xbins; ++i)
	{
	    x = dxmin + ((double)i/dxbins)*dxrange;
	    for (j=0; j<ybins; ++j, ++index)
	    {
		if (errpos[index] < 0.f)
		{
		    sprintf(stringbuf,
		    "can't fit histogram %d: found a negative error value", id);
		    Tcl_SetResult(interp, stringbuf, TCL_VOLATILE);
		    goto fail;
		}
		else if ((key || data[index]) && errpos[index] > 0.f)
		{
		    ++ndof;
		    y = dymin + ((double)j/dybins)*dyrange;
		    w = 1.0/errpos[index];
		    w *= w;
		    s += w;
		    sx += x * w;
		    sy += y * w;
		    sz += data[index] * w;
		    sxx += x * w * x;
		    sxy += y * w * x;
		    syy += y * w * y;
		    szz += data[index] * w * data[index];
		    sxz += x * w * data[index];
		    syz += y * w * data[index];
		}
	    }
	}
    }
    else
    {
	/* Accumulate the sums */
	for (i=0; i<xbins; ++i)
	{
	    x = dxmin + ((double)i/dxbins)*dxrange;
	    for (j=0; j<ybins; ++j, ++index)
	    {
		if (key || data[index])
		{
		    ++ndof;
		    y = dymin + ((double)j/dybins)*dyrange;
		    sx += x;
		    sy += y;
		    sz += data[index];
		    sxx += x * x;
		    sxy += y * x;
		    syy += y * y;
		    szz += data[index] * (double)data[index];
		    sxz += x * data[index];
		    syz += y * data[index];
		}
	    }
	}
	s = (double)(ndof + 3);
    }

    /* Convert sums into parameter values */
    if (ndof < 0)
    {
	Tcl_SetResult(interp, "not enough data points", TCL_VOLATILE);
	goto fail;
    }
    w = s*(sxx*syy-sxy*sxy) + sx*(sxy*sy-sx*syy) + sy*(sx*sxy-sxx*sy);
    a = -(sxz*sy*sy-s*sxz*syy+s*sxy*syz-sx*sy*syz-sxy*sy*sz+sx*syy*sz)/w;
    b = -(s*sxy*sxz-sx*sxz*sy+sx*sx*syz-s*sxx*syz-sx*sxy*sz+sxx*sy*sz)/w;
    c = -(sx*sxz*syy+sxx*sy*syz-sxy*(sxz*sy+sx*syz)+sxy*sxy*sz-sxx*syy*sz)/w;
    if (ndof > 0)
    {
	chisq = -(sxz*sxz*(-sy*sy + s*syy) + s*sxx*syz*syz - 
		 2.0*sxx*sy*syz*sz - sxy*sxy*sz*sz + sxx*syy*sz*sz - 
		 2.0*sxz*(s*sxy*syz - sx*sy*syz - sxy*sy*sz + sx*syy*sz) + 
		 s*sxy*sxy*szz + sxx*sy*sy*szz - s*sxx*syy*szz + 
		 2.0*sx*sxy*(syz*sz - sy*szz) + sx*sx*(-syz*syz + syy*szz))/w;
	if (chisq < 0.0)
	    chisq = 0.0;
    }
    else
	chisq = 0.0;
    if (chisq > 0.0)
    {
	s_a = (s*syy - sy*sy)/w;
	if (s_a > 0.0)
	    s_a = sqrt(s_a);
	else
	    s_a = 0.0;
	s_b = (s*sxx - sx*sx)/w;
	if (s_b > 0.0)
	    s_b = sqrt(s_b);
	else
	    s_b = 0.0;
	s_c = (sxx*syy - sxy*sxy)/w;
	if (s_c > 0.0)
	    s_c = sqrt(s_c);
	else
	    s_c = 0.0;
	r_ab = (sx*sy - s*sxy)/w;
	calculate_rho(r_ab,s_a,s_b);
	r_ac = (sxy*sy - sx*syy)/w;
	calculate_rho(r_ac,s_a,s_c);
	r_bc = (sx*sxy - sxx*sy)/w;
	calculate_rho(r_bc,s_b,s_c);
	/* Renormalize the errors */
	if (!use_errors)
	{
	    w = sqrt(chisq/ndof);
	    s_a *= w;
	    s_b *= w;
	    s_c *= w;
	    if (ndof == nbins - 3)
		r_ab = 0.0;
	}
    }
    else
    {
	s_a = 0.0;
	s_b = 0.0;
	s_c = 0.0;
	r_ab = 0.0;
	r_ac = 0.0;
	r_bc = 0.0;
    }

    results[0] = Tcl_NewDoubleObj(a);
    results[1] = Tcl_NewDoubleObj(b);
    results[2] = Tcl_NewDoubleObj(c);
    results[3] = Tcl_NewDoubleObj(s_a);
    results[4] = Tcl_NewDoubleObj(s_b);
    results[5] = Tcl_NewDoubleObj(s_c);
    results[6] = Tcl_NewDoubleObj(r_ab);
    results[7] = Tcl_NewDoubleObj(r_ac);
    results[8] = Tcl_NewDoubleObj(r_bc);
    results[9] = Tcl_NewDoubleObj(chisq);
    results[10] = Tcl_NewIntObj(ndof);
    Tcl_SetObjResult(interp, Tcl_NewListObj(11, results));
    status = TCL_OK;

fail:
    if (data)
	free(data);
    return status;
}

#define fourier_check_1d do {\
    tcl_require_objc(3);\
    verify_1d_histo(id,1);\
    verify_1d_histo(id_result,2);\
    nbins = hs_1d_hist_num_bins(id);\
    if (nbins % 2)\
    {\
	Tcl_AppendResult(interp, "histogram with id ",\
			 Tcl_GetStringFromObj(objv[1], NULL),\
			 " does not represent a set of Fourier"\
			 " coefficients (odd number of bins)", NULL);\
	return TCL_ERROR;\
    }\
    halfbins = nbins/2;\
    if (halfbins != hs_1d_hist_num_bins(id_result))\
    {\
	Tcl_AppendResult(interp, "Histogram with id ",\
			 Tcl_GetStringFromObj(objv[2], NULL),\
			 " can not be a Fourier spectral ",\
			 "density of histogram with id ",\
			 Tcl_GetStringFromObj(objv[1], NULL),\
			 ": the number of bins is wrong", NULL);\
	return TCL_ERROR;\
    }\
    data = (float *)malloc((nbins + halfbins) * sizeof(float));\
    if (data == NULL)\
    {\
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);\
	return TCL_ERROR;\
    }\
    hs_1d_hist_bin_contents(id, data);\
    result = data + nbins;\
} while(0);

tcl_routine(1d_fourier_power)
{
    int i, j, id, id_result, nbins, halfbins;
    float *data, *result;
    double p0, dhalf, power, total = 0.0;
    register double dtmp;
    Tcl_Obj *tobj;

    fourier_check_1d;

    dtmp = data[0];
    dhalf = (double)halfbins;
    /* Note the coefficient of 1/2 in the following formula.
       It is there because the first term in the Fourier series
       is usually written as 1/2*A0 rather than just A0.
       Our definition of A0 conforms to the former (use 1/2). */
    p0 = dtmp*dtmp*dhalf/2.0;
    for (i=0, j=0; i<halfbins-1; ++i)
    {
	dtmp = data[++j];
	power = dtmp*dtmp;
	dtmp = data[++j];
	power += dtmp*dtmp;
	power *= dhalf;
	result[i] = power;
	total += power;
    }
    dtmp = data[++j];
    /* Note the coefficient of 2 in the following formula.
       It is there because the average cos^2 for the highest
       frequency is 1 instead of 1/2. */
    power = dtmp*dtmp*dhalf*2.0;
    result[i] = power;
    total += power;

    hs_1d_hist_block_fill(id_result, result, NULL, NULL);
    free(data);
    if ((tobj = Tcl_NewListObj(0, NULL)) == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    Tcl_ListObjAppendElement(interp, tobj, Tcl_NewDoubleObj(p0));
    Tcl_ListObjAppendElement(interp, tobj, Tcl_NewDoubleObj(total));
    Tcl_SetObjResult(interp, tobj);
    return TCL_OK;
}

tcl_routine(1d_fourier_phase)
{
    int i, j, id, id_result, nbins, halfbins;
    float *data, *result;

    fourier_check_1d;

    for (i=0, j=1; i<halfbins-1; ++i, j+=2)
	result[i] = (float)atan2((double)data[j+1],(double)data[j]);
    result[halfbins-1] = (float)atan2(data[0],(double)data[j]);

    hs_1d_hist_block_fill(id_result, result, NULL, NULL);
    free(data);
    return TCL_OK;
}

tcl_routine(1d_fourier_conjugate)
{
    int i, id1, id2, nbins, halfbins;
    float *data;

    tcl_require_objc(3);
    verify_1d_histo(id1,1);
    verify_1d_histo(id2,2);
    nbins = hs_1d_hist_num_bins(id1);
    if (nbins % 2)
    {
	Tcl_AppendResult(interp, "histogram with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " may not represent a set of Fourier"
			 " coefficients (odd number of bins)", NULL);
	return TCL_ERROR;
    }
    if (nbins != hs_1d_hist_num_bins(id2))
    {
	Tcl_AppendResult(interp, "Histograms with ids ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " and ", Tcl_GetStringFromObj(objv[2], NULL),
			 " are not bin-compatible", NULL);
	return TCL_ERROR;
    }
    data = (float *)malloc(nbins * sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    halfbins = nbins/2;
    hs_1d_hist_bin_contents(id1, data);
    for (i=1; i<halfbins; ++i)
	data[2*i] *= -1.f;
    hs_1d_hist_block_fill(id2, data, NULL, NULL);
    free(data);
    return TCL_OK;
}

tcl_routine(1d_fourier_synthesize)
{
    /* Usage:  hs::1d_fourier_synthesize $id $p0 $id_power $id_phase */
    int i, nbins, halfbins, id, id_power, id_phase;
    double dhalf, a, p0;
    float *data, *power, *phase;

    tcl_require_objc(5);
    verify_1d_histo(id,1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &p0) != TCL_OK)
	return TCL_ERROR;
    if (p0 < 0.0)
    {
	Tcl_SetResult(interp, "Constant term power can not be negative",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    verify_1d_histo(id_power,3);
    verify_1d_histo(id_phase,4);

    nbins = hs_1d_hist_num_bins(id);
    if (nbins % 2)
    {
	Tcl_AppendResult(interp, "histogram with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " may not represent a set of Fourier"
			 " coefficients (odd number of bins)", NULL);
	return TCL_ERROR;
    }
    halfbins = nbins/2;
    if (hs_1d_hist_num_bins(id_power) != halfbins)
    {
	Tcl_AppendResult(interp, "Histogram with id ",
			 Tcl_GetStringFromObj(objv[3], NULL),
			 " can not be a Fourier power ",
			 "spectrum of histogram with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ": the number of bins is wrong", NULL);
	return TCL_ERROR;
    }
    if (hs_1d_hist_num_bins(id_phase) != halfbins)
    {
	Tcl_AppendResult(interp, "Histogram with id ",
			 Tcl_GetStringFromObj(objv[4], NULL),
			 " can not be a Fourier phase ",
			 "spectrum of histogram with id ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 ": the number of bins is wrong", NULL);
	return TCL_ERROR;
    }

    /* Fetch the data */
    data = (float *)malloc(nbins * 2 * sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    power = data + nbins;
    phase = power + halfbins;
    hs_1d_hist_bin_contents(id_power, power);
    hs_1d_hist_bin_contents(id_phase, phase);

    dhalf = (double)halfbins;
    for (i=0; i<halfbins-1; ++i)
    {
	if (power[i] < 0.f)
	{
	    Tcl_SetResult(interp, "bad power spectrum: found a negative entry",
			  TCL_STATIC);
	    free(data);
	    return TCL_ERROR;
	}
	a = sqrt(power[i]/dhalf);
	data[2*i+1] = a*cos((double)(phase[i]));
	data[2*i+2] = a*sin((double)(phase[i]));
    }
    data[nbins-1] = sqrt(power[i]/dhalf/2.0);
    if (cos((double)(phase[i])) < 0.0)
	data[nbins-1] *= -1.f;
    data[0] = sqrt(p0/dhalf*2.0);
    if (sin((double)(phase[i])) < 0.0)
	data[0] *= -1.f;

    hs_1d_hist_block_fill(id, data, NULL, NULL);
    free(data);
    return TCL_OK;
}

#define prepare_convolution do {\
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)\
	return TCL_ERROR;\
    title = Tcl_GetStringFromObj(objv[2], NULL);\
    category = Tcl_GetStringFromObj(objv[3], NULL);\
    verify_1d_histo(id1,4);\
    verify_1d_histo(id2,5);\
    nbins = hs_1d_hist_num_bins(id1);\
    if (nbins % 2)\
    {\
	Tcl_AppendResult(interp, "histogram with id ",\
			 Tcl_GetStringFromObj(objv[4], NULL),\
			 " may not represent a set of Fourier"\
			 " coefficients (odd number of bins)", NULL);\
	return TCL_ERROR;\
    }\
    if (nbins != hs_1d_hist_num_bins(id2))\
    {\
	Tcl_AppendResult(interp, "Histograms with ids ",\
			 Tcl_GetStringFromObj(objv[4], NULL),\
			 " and ", Tcl_GetStringFromObj(objv[5], NULL),\
			 " are not bin-compatible", NULL);\
	return TCL_ERROR;\
    }\
    halfbins = (float)(nbins/2);\
    result = (float *)malloc(3 * nbins * sizeof(float));\
    if (result == NULL)\
    {\
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);\
	return TCL_ERROR;\
    }\
    data1 = result + nbins;\
    data2 = data1 + nbins;\
    hs_1d_hist_bin_contents(id1, data1);\
    hs_1d_hist_bin_contents(id2, data2);\
} while(0);

tcl_routine(1d_fourier_multiply)
{
    int i, j, imax, uid, id1, id2, id_result, nbins;
    char *title, *category;
    float *data1, *data2, *result, halfbins;

    tcl_require_objc(6);
    prepare_convolution;

    /* The "halfbins" coefficient is here because of the normalization... */
    result[0] = data1[0]*data2[0]*halfbins;
    imax = nbins/2-1;
    for (i=0, j=1; i<imax; ++i, j+=2)
    {
	result[j] = (data1[j]*data2[j] - data1[j+1]*data2[j+1])*halfbins;
	result[j+1] = (data1[j]*data2[j+1] + data2[j]*data1[j+1])*halfbins;
    }
    result[nbins-1] = data1[nbins-1]*data2[nbins-1]*(float)nbins;

    id_result = hs_duplicate_axes(id1, uid, title, category);
    if (id_result <= 0)
    {
	Tcl_SetResult(interp, "failed to create a new histogram", TCL_STATIC);
	free(result);
	return TCL_ERROR;
    }
    hs_1d_hist_block_fill(id_result, result, NULL, NULL);
    free(result);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id_result));
    return TCL_OK;
}

#define division_by_zero do {\
    Tcl_SetResult(interp, "division by zero", TCL_STATIC);\
    free(result);\
    return TCL_ERROR;\
} while(0);

tcl_routine(1d_fourier_divide)
{
    int i, j, imax, uid, id1, id2, id_result, nbins, ignore0 = 0;
    char *title, *category;
    float *data1, *data2, *result, tmp, halfbins;

    tcl_objc_range(6,7);
    prepare_convolution;
    if (objc > 6)
	if (Tcl_GetBooleanFromObj(interp, objv[6], &ignore0) != TCL_OK)
	{
	    free(result);
	    return TCL_ERROR;
	}
    
    if (data2[0] == 0 || data2[nbins-1] == 0)
    {
	if (ignore0)
	    result[0] = 0.f;
	else
	    division_by_zero;
    }
    else
	result[0] = data1[0]/data2[0]/halfbins;
    imax = nbins/2-1;
    for (i=0, j=1; i<imax; ++i, j+=2)
    {
	if (data2[j] == 0 && data2[j+1] == 0)
	{
	    if (ignore0)
	    {
		result[j] = 0.f;
		result[j+1] = 0.f;
	    }
	    else
		division_by_zero;
	}
	else
	{
	    tmp = (data2[j]*data2[j] + data2[j+1]*data2[j+1])*halfbins;
	    result[j] = (data1[j]*data2[j] + data1[j+1]*data2[j+1])/tmp;
	    result[j+1] = (data1[j+1]*data2[j] - data1[j]*data2[j+1])/tmp;
	}
    }
    if (data2[nbins-1] == 0)
    {
	if (ignore0)
	    result[nbins-1] = 0.f;
	else
	    division_by_zero;
    }
    else
	result[nbins-1] = data1[nbins-1]/data2[nbins-1]/(float)nbins;

    id_result = hs_duplicate_axes(id1, uid, title, category);
    if (id_result <= 0)
    {
	Tcl_SetResult(interp, "failed to create a new histogram", TCL_STATIC);
	free(result);
	return TCL_ERROR;
    }
    hs_1d_hist_block_fill(id_result, result, NULL, NULL);
    free(result);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id_result));
    return TCL_OK;
}

tcl_routine(1d_fft)
{
    int i, id, id_transform, direction, m, nbins, size, accumulate = 0;
    float *data, *trdata = NULL;

    tcl_objc_range(4,5);
    verify_1d_histo(id,1);
    verify_1d_histo(id_transform,2);
    if (Tcl_GetBooleanFromObj(interp, objv[3], &direction) != TCL_OK)
	return TCL_ERROR;
    if (objc > 4)
	if (Tcl_GetBooleanFromObj(interp, objv[4], &accumulate) != TCL_OK)
	    return TCL_ERROR;
    nbins = hs_1d_hist_num_bins(id);
    for (m = 0, i = nbins; i > 1; i /= 2, ++m)
	if (i % 2)
	{
	    Tcl_AppendResult(interp, "Number of bins in the histogram with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a power of 2", NULL);
	    return TCL_ERROR;
	}
    if (nbins != hs_1d_hist_num_bins(id_transform))
    {
	Tcl_AppendResult(interp, "Histograms with ids ",
			 Tcl_GetStringFromObj(objv[1], NULL),
			 " and ", Tcl_GetStringFromObj(objv[2], NULL),
			 " are not bin-compatible", NULL);
	return TCL_ERROR;
    }
    size = nbins+2;
    if (accumulate)
    {
	if (id_transform == id)
	{
	    Tcl_SetResult(interp, "Can't accumulate transform coefficients: "
			  "the original and the transform histograms "
			  "have the same Histo-Scope id", TCL_STATIC);
	    return TCL_ERROR;
	}
	size += nbins;
    }
    data = (float *)malloc(size * sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    if (accumulate)
    {
	trdata = data+nbins+2;
	hs_1d_hist_bin_contents(id_transform, trdata);
    }
    if (direction)
    {
	hs_1d_hist_bin_contents(id, data);
	m = -m;
	rfft_(data, &m);
	/* The 0th and the last coefficients of the transform are going
	   to be real. All other real-valued coefficients are two times
	   larger than the real part of their complex counterparts. 
	   Also, the phases are swapped... */
	data[1] = data[0] * 2.f;
	for (i=2; i<nbins; )
	{
	    data[i++] *= 2.f;
	    data[i++] *= -2.f;
	}
	/* The very last coefficient is not mutiplied by 2 */
	if (accumulate)
	    for (i=0; i<nbins; ++i)
		trdata[i] += data[i+1];
	else
	    trdata = data+1;
    }
    else
    {
	hs_1d_hist_bin_contents(id, data+1);
	data[0] = data[1] / 2.f;
	data[1] = 0.f;
	data[nbins+1] = 0.f;
	for (i=2; i<nbins; )
	{
	    data[i++] /= 2.f;
	    data[i++] /= -2.f;
	}
	rfft_(data, &m);
	if (accumulate)
	    for (i=0; i<nbins; ++i)
		trdata[i] += data[i];
	else
	    trdata = data;
    }
    hs_1d_hist_block_fill(id_transform, trdata, NULL, NULL);
    free(data);
    return TCL_OK;
}

tcl_routine(ntuple_poly_fit)
{
    /* Usage: hs::ntuple_poly_fit hs_id xcolumn ycolumn
     *           degree_of_polynomial ?reverse_ordering?
     * Returns the list {{a_0 ... a_n} sigma} in case reverse_ordering 
     * is 0 (default) or {{a_n ... a_0} sigma} in case reverse_ordering is 1.
     */
    int i, id, xcol, ycol, deg, ideg, num_var, size, ifail = 0, ordering = 0;
    float *xdata = NULL, *ydata = NULL;
    float a[22], sigma;
    double dsigma;
    char charbuf[32];
    int status = TCL_ERROR;
    Tcl_Obj *result, *coeffs;

    if (objc < 5 || objc > 6)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    verify_ntuple(id,1);
    if (Tcl_GetIntFromObj(interp, objv[2], &xcol) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &ycol) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &deg) != TCL_OK)
	return TCL_ERROR;
    ideg = deg;
    if (objc > 5)
	if (Tcl_GetBooleanFromObj(interp, objv[5], &ordering) != TCL_OK)
	    return TCL_ERROR;

    num_var = hs_num_variables(id);
    if (xcol < 0 || xcol >= num_var || ycol < 0 || ycol >= num_var)
    {
	Tcl_SetResult(interp, "column number is out of range", 
		      TCL_STATIC);
	return TCL_ERROR;
    }

    if (deg > 20)
    {
	Tcl_SetResult(interp,
		      "degree of the polynomial must not exceed 20",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    if (deg < 0)
    {
	Tcl_SetResult(interp,
		      "degree of the polynomial can not be negative",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    size = hs_num_entries(id);
    if (size <= deg)
    {
	Tcl_SetResult(interp, "not enough data points", TCL_STATIC);
	return TCL_ERROR;
    }

    xdata = (float *)malloc(size*sizeof(float));
    ydata = (float *)malloc(size*sizeof(float));
    if (xdata == NULL || ydata == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    hs_column_contents(id, xcol, xdata);
    hs_column_contents(id, ycol, ydata);

    if (deg == 0)
	arr_stats(ydata, size, 1, a, &sigma);
    else if (deg == 1)
    {
	rlsqp1_(&size, xdata, ydata, a, a+1, &sigma, &ifail);
	dsigma = sigma;
    }
    else if (deg == 2)
    {
 	rlsqp2_(&size, xdata, ydata, a, a+1, a+2, &sigma, &ifail);
	dsigma = sigma;
    }
    else
    {
	rlsqpm_(&size, xdata, ydata, &deg, a, &dsigma, &ifail);
    }
    if (ifail != 0)
    {
	if (ifail == -1)
	{
	    Tcl_SetResult(interp,
			  "matrix of normal equations is numerically singular",
			  TCL_STATIC);
	}
	else
	{
	    /* This should not happen */
	    sprintf(charbuf, "%d", ifail);
	    Tcl_AppendResult(interp,
			     "Internal fit error (fail status ", charbuf,
			     "). This is a bug. Please report.", NULL);
	}
	goto fail;
    }

    /* Construct the result */
    result = Tcl_NewListObj(0, NULL);
    coeffs = Tcl_NewListObj(0, NULL);
    if (ordering)
	for (i=ideg; i>=0; --i)
	    Tcl_ListObjAppendElement(interp, coeffs, Tcl_NewDoubleObj((double)(a[i])));
    else
	for (i=0; i<=ideg; ++i)
	    Tcl_ListObjAppendElement(interp, coeffs, Tcl_NewDoubleObj((double)(a[i])));
    Tcl_ListObjAppendElement(interp, result, coeffs);
    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(dsigma));
    Tcl_SetObjResult(interp, result);

    status = TCL_OK;

 fail:
    if (xdata)
	free(xdata);
    if (ydata)
	free(ydata);
    return status;
}

tcl_routine(eigen_sym)
{
    int worksize, row, col, nrows, ncols, status = TCL_ERROR;
    double *data = NULL, *mem = NULL, *A, *W, *WORK;
    const int NB = 32;
    char *UPLO = "L", *JOBZ = "V";
    int N, LDA, LWORK, INFO;
    Tcl_Obj *res[2];
    int reverse_ordering = 0;

    tcl_objc_range(2,3)
    if (parse_matrix(interp, objv[1], 'c', &nrows, &ncols, &data) != TCL_OK)
	goto fail;
    if (nrows != ncols)
    {
	Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
	goto fail;
    }
    if (objc == 3)
	if (Tcl_GetBooleanFromObj(interp, objv[2], &reverse_ordering) != TCL_OK)
	    goto fail;

    /* Get the memory                          */
    LWORK = (NB+2)*nrows;
    /*           W      WORK        A          */
    worksize = nrows + LWORK + nrows*nrows;
    mem = (double *)malloc(worksize*sizeof(double));
    if (mem == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    A    = mem;
    W    = A + nrows*nrows;
    WORK = W + nrows;

    /* Symmetrize the matrix */
    for (row=0; row<nrows; ++row)
	for (col=0; col<=row; ++col)
  	    A[row+nrows*col] = 0.5*(data[row+nrows*col] + data[col+ncols*row]);

    /* Get the eigenvectors and eigenvalues */
    N = nrows;
    LDA = nrows;
    dsyev_(JOBZ, UPLO, &N, A, &LDA, W, WORK, &LWORK, &INFO, 1, 1);
    if (INFO < 0)
    {
	Tcl_SetResult(interp, "bad arguments", TCL_STATIC);
	goto fail;
    }
    else if (INFO > 0)
    {
	Tcl_SetResult(interp, "algorithm failed to converge", TCL_STATIC);
	goto fail;
    }

    if (reverse_ordering)
    {
	for (row=0; row<nrows; ++row)
	    WORK[row] = W[nrows-1-row];
	res[0] = new_list_of_doubles(nrows, WORK);
	for (row=0; row<nrows; ++row)
	    for (col=0; col<ncols; ++col)
		data[col+ncols*row] = A[col+ncols*(nrows-1-row)];
	res[1] = new_matrix_obj('c', nrows, nrows, data);
    }
    else
    {
	res[0] = new_list_of_doubles(nrows, W);
	res[1] = new_matrix_obj('c', nrows, nrows, A);
    }
    if (res[0] == NULL || res[1] == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, res));
    status = TCL_OK;

 fail:
    if (mem)
	free(mem);
    if (data)
	free(data);
    return status;
}

tcl_routine(Invert_sym_pos_matrix)
{
    double *data = NULL, *A = NULL;
    int row, col, nrows, ncols, dim, ifail = 0;
    Tcl_Obj *result;

    tcl_require_objc(2);
    if (parse_matrix(interp, objv[1], 'c', &nrows, &ncols, &data) != TCL_OK)
	goto fail;
    if (nrows != ncols)
    {
	Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
	goto fail;
    }
    dim = nrows;

    /* Symmetrize the matrix */
    A = (double *)malloc(dim*dim*sizeof(double));
    if (A == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    for (row=0; row<nrows; ++row)
	for (col=0; col<=row; ++col)
	{
  	    A[row+nrows*col] = 0.5*(data[row+nrows*col] + data[col+ncols*row]);
	    A[col+ncols*row] = A[row+nrows*col];
	}

    dsinv_(&nrows, A, &ncols, &ifail);
    if (ifail)
    {
	Tcl_SetResult(interp, "matrix is not positive definite", TCL_STATIC);
	goto fail;
    }
    result = new_matrix_obj('c', dim, dim, A);
    if (result == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }

    Tcl_SetObjResult(interp, result);
    free(A);
    free(data);
    return TCL_OK;

 fail:
    if (A)
	free(A);
    if (data)
	free(data);
    return TCL_ERROR;
}

tcl_routine(Sym_pos_linsys)
{
    double *data = NULL, *A = NULL, *B = NULL;
    int K = 1, row, col, nrows, ncols, dim, ifail = 0, status = TCL_ERROR;
    Tcl_Obj *result;

    tcl_require_objc(3);
    if (parse_matrix(interp, objv[1], 'c', &nrows, &ncols, &data) != TCL_OK)
	goto fail;
    if (nrows != ncols)
    {
	Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
	goto fail;
    }
    dim = nrows;

    /* Symmetrize the matrix */
    A = (double *)malloc(dim*dim*sizeof(double));
    if (A == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    for (row=0; row<nrows; ++row)
	for (col=0; col<=row; ++col)
	{
  	    A[row+nrows*col] = 0.5*(data[row+nrows*col] + data[col+ncols*row]);
	    A[col+ncols*row] = A[row+nrows*col];
	}

    /* Get the vector */
    if (parse_matrix(interp, objv[2], 'c', &nrows, &ncols, &B) != TCL_OK)
	goto fail;
    if (nrows != dim)
    {
	Tcl_SetResult(interp, "inconsistent matrix and "
		      "vector dimensionality", TCL_STATIC);
	goto fail;
    }
    if (ncols != K)
    {
	Tcl_SetResult(interp, "bad vector argument", TCL_STATIC);
	goto fail;
    }
    ncols = dim;

    dseqn_(&nrows, A, &ncols, &ifail, &K, B);
    result = new_list_of_doubles(dim, B);
    if (result == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }

    Tcl_SetObjResult(interp, result);
    status = TCL_OK;

 fail:
    if (B)
	free(B);
    if (A)
	free(A);
    if (data)
	free(data);
    return status;
}

int parse_matrix(Tcl_Interp *interp, Tcl_Obj *obj, char crep,
		 int *nrows, int *ncols, double **pdata)
{
    int rep, row, col, collen, rowlen, rowlen0 = -1;
    Tcl_Obj **listObjElem, **rowelem;
    double dtmp, *data = NULL;

    assert(interp && obj && nrows && ncols && pdata);
    if (crep == 'c' || crep == 'C')
	rep = 0;
    else if (crep == 'f' || crep == 'F')
	rep = 1;
    else
	assert(0);
    *nrows = 0;
    *ncols = 0;
    *pdata = NULL;

    if (Tcl_ListObjGetElements(interp, obj, &collen, &listObjElem) != TCL_OK)
	goto fail;
    if (collen < 1)
    {
	Tcl_SetResult(interp, "matrix can not be represented by an empty list",
		      TCL_STATIC);
	goto fail;
    }
    for (row=0; row<collen; ++row)
    {
	if (Tcl_ListObjGetElements(interp, listObjElem[row],
				   &rowlen, &rowelem) != TCL_OK)
	    goto fail;
	if (rowlen < 1)
	{
	    Tcl_SetResult(interp, "matrix row can not be represented "
			  "by an empty list", TCL_STATIC);
	    goto fail;
	}
	if (row == 0)
	{
	    rowlen0 = rowlen;
	    data = (double *)malloc(rowlen*collen*sizeof(double));
	    if (data == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	}
	else if (rowlen != rowlen0)
	{
	    Tcl_SetResult(interp, "matrix rows are not of equal length",
			  TCL_STATIC);
	    goto fail;
	}
	for (col=0; col<rowlen; ++col)
	{
	    if (Tcl_GetDoubleFromObj(interp, rowelem[col], &dtmp) != TCL_OK)
		goto fail;
	    if (rep)
		data[row+collen*col] = dtmp;
	    else
		data[col+rowlen*row] = dtmp;
	}
    }

    *nrows = collen;
    *ncols = rowlen;
    *pdata = data;
    return TCL_OK;

 fail:
    if (data)
	free(data);
    return TCL_ERROR;
}

Tcl_Obj *new_matrix_obj(char crep, int nrows, int ncols, double *data)
{
    int rep, row, col;
    Tcl_Obj *result, *rowlist, *value;

    assert(data);
    if (crep == 'c' || crep == 'C')
	rep = 0;
    else if (crep == 'f' || crep == 'F')
	rep = 1;
    else
	assert(0);
    result = Tcl_NewListObj(0, NULL);
    if (result == NULL)
	return NULL;
    for (row=0; row<nrows; ++row)
    {
	rowlist = Tcl_NewListObj(0, NULL);
	if (rowlist == NULL)
	    return NULL;
	for (col=0; col<ncols; ++col)
	{
	    if (rep)
		value = Tcl_NewDoubleObj(data[row+nrows*col]);
	    else
		value = Tcl_NewDoubleObj(data[col+ncols*row]);
	    if (value == NULL)
		return NULL;
	    if (Tcl_ListObjAppendElement(NULL, rowlist, value) != TCL_OK)
		return NULL;
	}
	if (Tcl_ListObjAppendElement(NULL, result, rowlist) != TCL_OK)
	    return NULL;
    }
    return result;
}

Tcl_Obj *new_list_of_doubles(int n, double *data)
{
    int i;
    Tcl_Obj *result;
    Tcl_Obj **values;

    assert(data && n >= 0);
    if (n == 0)
	return Tcl_NewListObj(0, NULL);
    values = (Tcl_Obj **)malloc(n*sizeof(Tcl_Obj *));
    if (values == NULL)
	return NULL;
    for (i=0; i<n; ++i)
    {
	values[i] = Tcl_NewDoubleObj(data[i]);
	if (values[i] == NULL)
	    goto fail;
    }
    result = Tcl_NewListObj(n, values);
    if (result == NULL)
	goto fail;
    free(values);
    return result;

 fail:
    for (n=i-1; n>=0; --n)
    {
	Tcl_IncrRefCount(values[n]);
	Tcl_DecrRefCount(values[n]);
    }
    free(values);
    return NULL;   
}

tcl_routine(Unit_matrix)
{
    int i, ndim;
    double *data = NULL;
    Tcl_Obj *res;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &ndim) != TCL_OK)
	return TCL_ERROR;
    if (ndim <= 0)
    {
	Tcl_AppendResult(interp, "expected a positive integer, got ",
			 Tcl_GetStringFromObj(objv[1], NULL), NULL);
	return TCL_ERROR;
    }
    data = (double *)calloc(ndim*ndim, sizeof(double));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    for (i=0; i<ndim; ++i)
	data[i+ndim*i] = 1.0;
    res = new_matrix_obj('c', ndim, ndim, data);
    if (res == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	free(data);
	return TCL_ERROR;
    }
    free(data);
    Tcl_SetObjResult(interp, res);
    return TCL_OK;
}

tcl_routine(Histo_matrix)
{
    /*  Usage hs::Histo_matrix id */
    int i, id, size, num_x_bins, num_y_bins;
    void *mem;
    float *data;
    double *ddata;
    Tcl_Obj *res = NULL;

    tcl_require_objc(2);
    verify_2d_histo(id, 1);
    hs_2d_hist_num_bins(id, &num_x_bins, &num_y_bins);
    size = num_x_bins*num_y_bins;
    if ((mem = malloc(size*(sizeof(float) + sizeof(double)))) == NULL)
    {
        Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
        return TCL_ERROR;
    }
    ddata = mem;
    data = (float *)(ddata + size);
    hs_2d_hist_bin_contents(id, data);
    for (i=0; i<size; ++i)
        ddata[i] = (double)data[i];
    res = new_matrix_obj('f', num_y_bins, num_x_bins, ddata);
    free(mem);
    if (res == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    else
    {
        Tcl_SetObjResult(interp, res);
        return TCL_OK;
    }
}

tcl_routine(Const_matrix)
{
    /*  Usage hs::Const_matrix nrows ncols value */
    int i, n, nrows, ncols;
    double value, *data = NULL;
    Tcl_Obj *res;

    tcl_require_objc(4);
    if (Tcl_GetIntFromObj(interp, objv[1], &nrows) != TCL_OK)
	return TCL_ERROR;
    if (nrows <= 0)
    {
	Tcl_AppendResult(interp, "expected a positive integer, got ",
			 Tcl_GetStringFromObj(objv[1], NULL), NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &ncols) != TCL_OK)
	return TCL_ERROR;
    if (ncols <= 0)
    {
	Tcl_AppendResult(interp, "expected a positive integer, got ",
			 Tcl_GetStringFromObj(objv[2], NULL), NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[3], &value) != TCL_OK)
	return TCL_ERROR;
    n = nrows*ncols;
    data = (double *)malloc(n*sizeof(double));
    if (data == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	return TCL_ERROR;
    }
    for (i=0; i<n; ++i)
	data[i] = value;
    res = new_matrix_obj('c', nrows, ncols, data);
    if (res == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	free(data);
	return TCL_ERROR;
    }
    free(data);
    Tcl_SetObjResult(interp, res);
    return TCL_OK;
}

tcl_routine(Matrix)
{
    /*  Basic matrix operations                                            */
    /*  Usage hs::Matrix A op B (binary operations)                        */
    /*        hs::Matrix A op   (unary operations)                         */
    /*        hs::Matrix A      (return a copy of A using double objects)  */

    int i, row, col, nrows1, ncols1, nrows2, ncols2, status = TCL_ERROR;
    int ifail = 0;
    double *data1 = NULL, *data2 = NULL, *result = NULL, *mem = NULL;
    double dvalue;
    char *op;
    Tcl_Obj *res = NULL;
    static char *bad_dimensions = "incompatible argument dimensions";

    /* We may want to parse matrices differently (using C or
       FORTRAN conventions) depending on operation requested */
#define unary_parse(convention) do {\
    if (parse_matrix(interp, objv[1], convention, &nrows1, &ncols1, &data1) != TCL_OK)\
	goto fail;\
} while(0);

#define binary_parse(convention) do {\
    if (parse_matrix(interp, objv[1], convention, &nrows1, &ncols1, &data1) != TCL_OK)\
	goto fail;\
    if (parse_matrix(interp, objv[3], convention, &nrows2, &ncols2, &data2) != TCL_OK)\
	goto fail;\
} while(0);

    tcl_objc_range(2,4);
    if (objc == 2)
    {
	/* Make sure the argument is correct */
	unary_parse('c');
	res = new_matrix_obj('c', nrows1, ncols1, data1);
    }
    else if (objc == 3)
    {
	/* This is a unary operation */
	op = Tcl_GetStringFromObj(objv[2],NULL);

	/* Transpose */
	if (strcasecmp(op, "t") == 0)
	{
	    unary_parse('c');
	    res = new_matrix_obj('f', ncols1, nrows1, data1);
	}

        /* Symmetrize */
        else if (strcasecmp(op, "sym") == 0)
        {
            unary_parse('c');
	    if (ncols1 != nrows1)
	    {
		Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
		goto fail;
	    }
            for (row=0; row<nrows1; ++row)
                for (col=row+1; col<ncols1; ++col)
                {
                    data1[col+row*ncols1] += data1[row+col*nrows1];
                    data1[col+row*ncols1] /= 2.0;
                    data1[row+col*nrows1] = data1[col+row*ncols1];
                }
            res = new_matrix_obj('c', nrows1, ncols1, data1);
        }

        /* Antisymmetrize */
        else if (strcasecmp(op, "asym") == 0)
        {
            unary_parse('c');
	    if (ncols1 != nrows1)
	    {
		Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
		goto fail;
	    }
            for (row=0; row<nrows1; ++row)
            {
                data1[row+row*ncols1] = 0.0;
                for (col=row+1; col<ncols1; ++col)
                {
                    data1[col+row*ncols1] -= data1[row+col*nrows1];
                    data1[col+row*ncols1] /= 2.0;
                    data1[row+col*nrows1] = -data1[col+row*ncols1];
                }
            }
            res = new_matrix_obj('c', nrows1, ncols1, data1);
        }

	/* Trace */
	else if (strcasecmp(op, "sp") == 0)
	{
	    unary_parse('c');
	    if (ncols1 != nrows1)
	    {
		Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
		goto fail;
	    }
	    dvalue = 0.0;
	    for (i=0; i<ncols1; ++i)
		dvalue += data1[i+ncols1*i];
	    res = Tcl_NewDoubleObj(dvalue);
	}

        /* Square root of the sum of all elements squared */
        else if (strcasecmp(op, "norm") == 0)
        {
	    unary_parse('c');
	    dvalue = 0.0;
	    for (row=0; row<nrows1; ++row)
                for (col=0; col<ncols1; ++col)
                {
                    double datum = data1[row*ncols1+col];
                    dvalue += datum*datum;
                }
	    res = Tcl_NewDoubleObj(sqrt(dvalue));
        }

	/* Matrix dimensionality */
	else if (strcasecmp(op, "dim") == 0)
	{
	    Tcl_Obj *answer[2];
	    unary_parse('c');
	    answer[0] = Tcl_NewIntObj(nrows1);
	    answer[1] = Tcl_NewIntObj(ncols1);
	    if (answer[0] == NULL || answer[1] == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    res = Tcl_NewListObj(2, answer);
	}

        /* Calculate determinant */
        else if (strcasecmp(op, "det") == 0)
	{
	    unary_parse('c');
	    if (ncols1 != nrows1)
	    {
		Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
		goto fail;
	    }
	    if (nrows1 == 1)
		dvalue = data1[0];
	    else if (nrows1 == 2)
		dvalue = data1[0]*data1[3] - data1[1]*data1[2];
	    else if (nrows1 == 3)
		dvalue = -(data1[2]*data1[4]*data1[6]) + data1[1]*data1[5]*data1[6] + 
		    data1[2]*data1[3]*data1[7] - data1[0]*data1[5]*data1[7] - 
		    data1[1]*data1[3]*data1[8] + data1[0]*data1[4]*data1[8];
	    else if (nrows1 == 4)
		dvalue = data1[3]*data1[6]*data1[9]*data1[12] - 
		    data1[2]*data1[7]*data1[9]*data1[12] - 
		    data1[3]*data1[5]*data1[10]*data1[12] + 
		    data1[1]*data1[7]*data1[10]*data1[12] + 
		    data1[2]*data1[5]*data1[11]*data1[12] - 
		    data1[1]*data1[6]*data1[11]*data1[12] - 
		    data1[3]*data1[6]*data1[8]*data1[13] + 
		    data1[2]*data1[7]*data1[8]*data1[13] + 
		    data1[3]*data1[4]*data1[10]*data1[13] - 
		    data1[0]*data1[7]*data1[10]*data1[13] - 
		    data1[2]*data1[4]*data1[11]*data1[13] + 
		    data1[0]*data1[6]*data1[11]*data1[13] + 
		    data1[3]*data1[5]*data1[8]*data1[14] - 
		    data1[1]*data1[7]*data1[8]*data1[14] - 
		    data1[3]*data1[4]*data1[9]*data1[14] + 
		    data1[0]*data1[7]*data1[9]*data1[14] + 
		    data1[1]*data1[4]*data1[11]*data1[14] - 
		    data1[0]*data1[5]*data1[11]*data1[14] - 
		    data1[2]*data1[5]*data1[8]*data1[15] + 
		    data1[1]*data1[6]*data1[8]*data1[15] + 
		    data1[2]*data1[4]*data1[9]*data1[15] - 
		    data1[0]*data1[6]*data1[9]*data1[15] - 
		    data1[1]*data1[4]*data1[10]*data1[15] + 
		    data1[0]*data1[5]*data1[10]*data1[15];
	    else
	    {
		int jfail = 0;
		mem = (double *)malloc(nrows1*sizeof(double));
		if (mem == NULL)
		{
		    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		    goto fail;
		}
		dfact_(&nrows1, data1, &ncols1, mem, &ifail, &dvalue, &jfail);
		switch (jfail)
		{
		case 0:
		    break;
		case 1:
		    Tcl_SetResult(interp, "determinant is too large",
				  TCL_STATIC);
		    goto fail;
		case -1:
		    Tcl_SetResult(interp, "determinant is too small",
				  TCL_STATIC);
		    goto fail;
		default:
		    assert(0);
		}
	    }
	    res = Tcl_NewDoubleObj(dvalue);
	}

	/* Invert */
	else if (strcasecmp(op, "inv") == 0)
	{
	    unary_parse('f');
	    if (ncols1 != nrows1)
	    {
		Tcl_SetResult(interp, "matrix is not square", TCL_STATIC);
		goto fail;
	    }
	    mem = (double *)malloc(ncols1*sizeof(double));
	    if (mem == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    nrows2 = nrows1;
	    dinv_(&nrows1, data1, &ncols1, mem, &ifail);
	    if (ifail)
	    {
		Tcl_SetResult(interp, "matrix is singular", TCL_STATIC);
		goto fail;
	    }
	    res = new_matrix_obj('f', nrows2, nrows2, data1);	    
	}

	/* Invalid operation */
	else
	{
	    Tcl_AppendResult(interp, "invalid unary matrix operation \"",
			     op, "\"", NULL);
	    goto fail;
	}
    }
    else if (objc == 4)
    {
	/* This is a binary operation */
	op = Tcl_GetStringFromObj(objv[2],NULL);

	/* Addition */
	if (strcmp(op, "+") == 0)
	{
	    binary_parse('c');
	    if (nrows1 != nrows2 || ncols1 != ncols2)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    result = (double *)malloc(nrows1*ncols1*sizeof(double));
	    if (result == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols1; ++col)
		    result[col+row*ncols1] = data1[col+row*ncols1] + data2[col+row*ncols1];
	    res = new_matrix_obj('c', nrows1, ncols1, result);
	}

	/* Subtraction */
	else if (strcmp(op, "-") == 0)
	{
	    binary_parse('c');
	    if (nrows1 != nrows2 || ncols1 != ncols2)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    result = (double *)malloc(nrows1*ncols1*sizeof(double));
	    if (result == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols1; ++col)
		    result[col+row*ncols1] = data1[col+row*ncols1] - data2[col+row*ncols1];
	    res = new_matrix_obj('c', nrows1, ncols1, result);
	}

	/* Matrix multiplication */
	else if (strcmp(op, ".") == 0)
	{
	    binary_parse('c');
	    if (ncols1 != nrows2)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    result = (double *)calloc(nrows1*ncols2, sizeof(double));
	    if (result == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols2; ++col)
		    for (i=0; i<nrows2; ++i)
			result[col+row*ncols2] += data1[row*ncols1+i]*data2[col+i*ncols2];
	    res = new_matrix_obj('c', nrows1, ncols2, result);
	}

	/* Transpose and multiply */
	else if (strcasecmp(op, "t.") == 0)
	{
	    if (parse_matrix(interp, objv[1], 'f', &ncols1, &nrows1, &data1) != TCL_OK)
		goto fail;
	    if (parse_matrix(interp, objv[3], 'c', &nrows2, &ncols2, &data2) != TCL_OK)
		goto fail;
	    if (ncols1 != nrows2)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    result = (double *)calloc(nrows1*ncols2, sizeof(double));
	    if (result == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols2; ++col)
		    for (i=0; i<nrows2; ++i)
			result[col+row*ncols2] += data1[row*ncols1+i]*data2[col+i*ncols2];
	    res = new_matrix_obj('c', nrows1, ncols2, result);
	}

	/* Multiply by a scalar */
	else if (strcmp(op, "*") == 0)
	{
	    unary_parse('c');
	    if (Tcl_GetDoubleFromObj(interp, objv[3], &dvalue) != TCL_OK)
		goto fail;
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols1; ++col)
		    data1[col+row*ncols1] *= dvalue;
	    res = new_matrix_obj('c', nrows1, ncols1, data1);
	}

	/* Divide by a scalar */
	else if (strcmp(op, "/") == 0)
	{
	    unary_parse('c');
	    if (Tcl_GetDoubleFromObj(interp, objv[3], &dvalue) != TCL_OK)
		goto fail;
	    if (dvalue == 0.0)
	    {
		Tcl_SetResult(interp, "division by zero", TCL_STATIC);
		goto fail;
	    }
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols1; ++col)
		    data1[col+row*ncols1] /= dvalue;
	    res = new_matrix_obj('c', nrows1, ncols1, data1);
	}

	/* Similarity */
	else if (strcasecmp(op, "sim") == 0)
	{
	    /* result = B^T A B, A is a square matrix (metric) */
	    binary_parse('c');
	    if (ncols1 != nrows2 || ncols1 != nrows1)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    mem = (double *)calloc(nrows1*ncols2, sizeof(double));
	    result = (double *)calloc(ncols2*ncols2, sizeof(double));
	    if (mem == NULL || result == NULL)
	    {
		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
		goto fail;
	    }
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols2; ++col)
		    for (i=0; i<nrows2; ++i)
			mem[col+row*ncols2] += data1[row*ncols1+i]*data2[col+i*ncols2];
	    for (row=0; row<ncols2; ++row)
		for (col=0; col<ncols2; ++col)
		    for (i=0; i<nrows1; ++i)
			result[col+row*ncols2] += data2[i*ncols2+row]*mem[col+i*ncols2];
	    res = new_matrix_obj('c', ncols2, ncols2, result);
	}

	/* Comparison */
	else if (strcmp(op, "==") == 0)
	{
	    int equal = 1;
	    binary_parse('c');
	    if (ncols1 != ncols2 || nrows1 != nrows2)
		equal = 0;
	    if (equal)
	    {
		i = 0;
		for (row=0; row<nrows1; ++row) {
		    for (col=0; col<ncols1; ++col) {
			if (data1[i] != data2[i]) {
			    equal = 0;
			    break;
			}
			++i;
		    }
		    if (!equal)
			break;
		}
	    }
	    res = Tcl_NewBooleanObj(equal);
	}

	/* Linear system */
	else if (strcasecmp(op, ".x=") == 0)
	{
	    binary_parse('f');
	    if (ncols1 != nrows1 || nrows1 != nrows2 || ncols2 != 1)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
  	    mem = (double *)malloc(ncols1*sizeof(double));
  	    if (mem == NULL)
  	    {
  		Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
  		goto fail;
  	    }

            /* Solution using CERNLIB DEQN routine */
/*  	    deqn_(&nrows1, data1, &ncols1, mem, &ifail, &ncols2, data2); */
/*  	    if (ifail) */
/*  	    { */
/*  		Tcl_SetResult(interp, "matrix is singular", TCL_STATIC); */
/*  		goto fail; */
/*  	    } */

            /* Solution using LAPACK. This solution provides
	       better recognition of singularities. */
	    dgetrf_(&nrows1, &ncols1, data1, &nrows1, (int *)mem, &ifail);
	    assert(ifail >= 0);
	    if (ifail > 0)
	    {
		Tcl_SetResult(interp, "matrix is singular", TCL_STATIC);
		goto fail;
	    }
            dgetrs_("N", &nrows1, &ncols2, data1, &nrows1, (int *)mem,
		    data2, &nrows2, &ifail, 1);
	    assert(ifail == 0);

	    /* Make up the result object */
	    res = new_matrix_obj('f', nrows2, 1, data2);
	}

	/* Subscripting */
	else if (strcmp(op, "_") == 0)
	{
	    int listlen;
	    Tcl_Obj **listObjElem;

	    unary_parse('c');
	    if (Tcl_ListObjGetElements(interp, objv[3], &listlen, &listObjElem) != TCL_OK)
		goto fail;
	    if (listlen == 0)
	    {
		Tcl_SetResult(interp, "empty subscript", TCL_STATIC);
		goto fail;
	    }
	    if (Tcl_GetIntFromObj(interp, listObjElem[0], &row) != TCL_OK)
		goto fail;
	    if (listlen == 1)
	    {
		if (nrows1 == 1 || ncols1 == 1)
		{
		    if (row < 0 || row >= nrows1*ncols1)
		    {
			Tcl_SetResult(interp, "subscript is out of range",
				      TCL_STATIC);
			goto fail;
		    }
		    res = Tcl_NewDoubleObj(data1[row]);
		}
		else
		{
		    Tcl_SetResult(interp, "missing subscript element",
				  TCL_STATIC);
		    goto fail;
		}
	    }
	    else
	    {
		if (Tcl_GetIntFromObj(interp, listObjElem[1], &col) != TCL_OK)
		    goto fail;
		if (listlen == 2)
		{
		    if (row < 0 || row >= nrows1 || col < 0 || col >= ncols1)
		    {
			Tcl_SetResult(interp, "subscript is out of range",
				      TCL_STATIC);
			goto fail;
		    }
		    res = Tcl_NewDoubleObj(data1[row*ncols1 + col]);
		}
		else
		{
		    Tcl_SetResult(interp, "too many subscript elements",
				  TCL_STATIC);
		    goto fail;
		}
	    }
	}

	/* Maximum relative difference */
	else if (strcasecmp(op, "maxreldiff") == 0)
	{
	    double reldiff;
	    binary_parse('c');
	    if (nrows1 != nrows2 || ncols1 != ncols2)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    dvalue = 0.0;
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols1; ++col)
		    if (data1[col+row*ncols1] || data2[col+row*ncols1])
		    {
			reldiff = fabs(data1[col+row*ncols1] - data2[col+row*ncols1])/
			    (fabs(data1[col+row*ncols1]) + fabs(data2[col+row*ncols1]));
			if (reldiff > dvalue)
			    dvalue = reldiff;
		    }
	    res = Tcl_NewDoubleObj(dvalue);
	}

	/* Maximum absolute difference */
	else if (strcasecmp(op, "maxabsdiff") == 0)
	{
	    double absdiff;
	    binary_parse('c');
	    if (nrows1 != nrows2 || ncols1 != ncols2)
	    {
		Tcl_SetResult(interp, bad_dimensions, TCL_STATIC);
		goto fail;
	    }
	    dvalue = 0.0;
	    for (row=0; row<nrows1; ++row)
		for (col=0; col<ncols1; ++col)
		{
		    absdiff = fabs(data1[col+row*ncols1] - data2[col+row*ncols1]);
		    if (absdiff > dvalue)
			dvalue = absdiff;
		}
	    res = Tcl_NewDoubleObj(dvalue);
	}

	/* Invalid operation */
	else
	{
	    Tcl_AppendResult(interp, "invalid binary matrix operation \"",
			     op, "\"", NULL);
	    goto fail;
	}
    }
    else
	assert(0);

    if (res == NULL)
    {
	Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	goto fail;
    }
    Tcl_SetObjResult(interp, res);
    status = TCL_OK;

 fail:
    if (result)
	free(result);
    if (mem)
	free(mem);
    if (data2)
	free(data2);
    if (data1)
	free(data1);
    return status;
}
