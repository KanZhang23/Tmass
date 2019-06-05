#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "cernlib.h"
#include "minuit.h"
#include "histoscope.h"
#include "fit_config.h"
#include "fit_api.h"
#include "histo_utils.h"

#define DEFAULT_MINUIT_VERBOSITY 0
#define DEFAULT_FITTING_METHOD "ls" /* Least-squares with errors from data */

static char *Minuit_minimizers[] = {
    "migrad",   /* The first method is the default one */
    "mini",
    "simplex"
};

#define checkfree(pointer) do {if (pointer) {free(pointer); pointer=0;}} while(0);

#define checknull(pointer) do {if (pointer == 0) {\
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);\
    return TCL_ERROR;\
}} while(0);

#define check_strdup(s1,s2) do {\
    if (s2)\
    {\
	s1 = strdup(s2);\
	if (s1 == NULL)\
	    goto fail0;\
    }\
    else\
        s1 = NULL;\
} while(0);

#define check_minuit_lock do {\
    if (hs_minuit_is_locked())\
    {\
	Tcl_SetResult(interp, "Minuit is busy", TCL_STATIC);\
	return TCL_ERROR;\
    }\
} while(0);

#define check_fit_unlocked(name,op) do {\
    if (hs_minuit_is_locked())\
    {\
	assert(hs_current_fit_conf());\
	if (strcmp(name, hs_current_fit_name()) op 0)\
	{\
	    Tcl_SetResult(interp, "Minuit is busy", TCL_STATIC);\
	    return TCL_ERROR;\
	}\
    }\
} while(0);

#define tcl_require_objc_option(N) do {\
  if (objc != N)\
  {\
    Tcl_AppendResult(interp, "wrong # of arguments for option \"",\
                     Tcl_GetStringFromObj(objv[0],NULL), "\"", NULL);\
    return TCL_ERROR;\
  }\
} while(0);

/* Counter for automatic fit names */
static int auto_fit_counter = 0;

/* Local functions for which we will not create commands */
tcl_routine(Fit_subset_used_regions);
tcl_routine(hs_parse_minuit_param_options);

static int parse_config_options(
    Fit_config *config, int print_config,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int parse_fitter_options(
    Fit_config *config, Fitter_info *newfitter, int *removed,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int parse_subset_options(
    Fit_config *config, int set, int reduced_option_set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int fit_subset_plotfit(
    Fit_config *config, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int fit_subset_fitvalues(
    Fit_config *config, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int fit_subset_kstest(
    Fit_config *config, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int fit_subset_random(
    Fit_config *config, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

static int generate_fitter_cdf(
    Fit_config *config, int set, Tcl_Interp *interp, double xmin,
    double xmax, int nbins, struct simpson_bin *cdfdata);

static void report_fitplot_error(Tcl_Interp *interp, Fit_subset *fitset,
                      int fnum, double x, double y, double z, int ierr);

static int fcompare(const float *i, const float *j)
{
    if (*i < *j)
        return -1;
    else if (*i > *j)
        return 1;
    else
        return 0;
}

static int is_blank_string(const char *pc)
{
    if (pc == NULL)
	return 1;
    for (; *pc; ++pc)
	if (!isspace(*pc))
	    return 0;
    return 1;
}

static int data_point_compare_x(const DataPoint *i, const DataPoint *j)
{
    if (i->x < j->x)
	return -1;
    else if (i->x > j->x)
	return 1;
    else
	return 0;
}

tcl_routine(Fit_subset_used_regions)
{
    /* Usage: ranges {min max npoints}
     * Scans the filter function and returns the list of used
     * regions {{min1 max1 npoints1} {min2 max2 npoints2} ...}
     */
    int i, usethis, npoints, listlen, in_use;
    int point_counter = 0;
    double x=0.0, y=0.0, z=0.0, eps;
    double dmin, dmax, dpoints, drange, dstep, oldx = 0.0;
    Fit_subset *fitset;
    Tcl_Obj **listObjElem;
    Tcl_Obj *result;
    Tcl_Obj *region = NULL;

    tcl_require_objc_option(2);
    fitset = (Fit_subset *)clientData;
    assert(fitset);
    if (Tcl_ListObjGetElements(interp, objv[1], 
			       &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen != 3)
    {
	Tcl_AppendResult(interp, "bad range specifier \"",
			 Tcl_GetStringFromObj(objv[1],NULL), "\"", NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &dmin) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &dmax) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, listObjElem[2], &npoints) != TCL_OK)
	return TCL_ERROR;
    if (npoints <= 0)
    {
	Tcl_AppendResult(interp, "bad range specifier \"",
			 Tcl_GetStringFromObj(objv[1],NULL),
			 "\": the number of points must be positive", NULL);
	return TCL_ERROR;
    }

    /* Check that we deal with 1d range */
    if (fitset->ndim != 1)
    {
	Tcl_SetResult(interp, "the dataset is not 1d", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that the filter is compiled */
    if (fitset->filter == NULL)
    {
	/* This is OK as long as the filter string is a blank line */
	if (!is_blank_string(fitset->filter_string))
	{
	    Tcl_SetResult(interp, "the filter is not compiled", TCL_STATIC);
	    return TCL_ERROR;
	}
	result = Tcl_NewListObj(0, NULL);
	Tcl_ListObjAppendElement(interp, result, objv[1]);
	Tcl_SetObjResult(interp, result);
	return TCL_OK;
    }

    /* Go for it. Each bin is used only if both 
       its center and edges are not filtered. */
    dpoints = (double)npoints;
    drange = dmax - dmin;
    dstep  = drange/dpoints;
    eps    = dstep*BIN_EDGE_EPSILON;
    result = Tcl_NewListObj(0, NULL);
    in_use = 0;
    for (i=0; i<npoints; ++i)
    {
        x = dmin + ((double)i/dpoints)*drange;
        usethis   = ((fitset->filter)(x + eps, y, z) &&
                     (fitset->filter)(x + dstep/2.0, y, z) &&
                     (fitset->filter)(x + dstep - eps, y, z));
	if (in_use)
	{
	    if (usethis)
	    {
		++point_counter;
		oldx = x;
	    }
	    else
	    {
		in_use = 0;
		Tcl_ListObjAppendElement(interp, region, Tcl_NewDoubleObj(oldx));
		Tcl_ListObjAppendElement(interp, region, Tcl_NewIntObj(point_counter));
		Tcl_ListObjAppendElement(interp, result, region);
	    }
	}
	else
	{
	    if (usethis)
	    {
		in_use = 1;
		point_counter = 1;		
		region = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, region, Tcl_NewDoubleObj(x));
		oldx = x;
	    }
	}
    }
    if (in_use)
    {
	Tcl_ListObjAppendElement(interp, region, Tcl_NewDoubleObj(oldx));
	Tcl_ListObjAppendElement(interp, region, Tcl_NewIntObj(point_counter));
	Tcl_ListObjAppendElement(interp, result, region);
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

tcl_routine(Fit_reset)
{
    tcl_require_objc(1);
    check_minuit_lock;
    hs_reset_current_fit();
    return TCL_OK;
}

tcl_routine(Fit_activate)
{
    int status;
    char *name;
    
    tcl_require_objc(2);
    name = Tcl_GetStringFromObj(objv[1], NULL);
    check_fit_unlocked(name,!=);
    status = hs_set_current_fit(name);
    if (status)
    {
	if (status == 1)
	    Tcl_AppendResult(interp, "fit named \"",
			     name, "\" does not exist", NULL);
	else if (status == 2)
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	else
	    assert(0);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Fit_get_active)
{
    const char *name;

    tcl_require_objc(1);
    name = hs_current_fit_name();
    if (name)
	Tcl_SetObjResult(interp, Tcl_NewStringObj(name, -1));
    else
	Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
    return TCL_OK;
}

tcl_routine(Fit_list)
{
    tcl_require_objc(1);
    return hs_fit_list(interp);
}

tcl_routine(Fit_fcn_get)
{
    const char *name;

    tcl_require_objc(1);
    name = hs_get_current_fcn_tag();
    if (name)
	Tcl_SetObjResult(interp, Tcl_NewStringObj(name, -1));
    else
	Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
    return TCL_OK;
}

tcl_routine(Fit_fcn_set)
{
    char *name;

    tcl_require_objc(2);
    name = Tcl_GetStringFromObj(objv[1], NULL);
    if (hs_set_fit_fcn_by_tag(name) == NULL)
    {
	Tcl_AppendResult(interp, "Bad fcn tag \"", name,
			 "\". Valid tags are:\n", NULL);
	hs_show_valid_fit_fcns(interp);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Fit_create)
{
    /* Basic fit setup. Usage:
     *
     * Fit_create name ?item1? ?item2? ... ?-opt1 value1? ...
     *
     * Each item specifier item1, item2 ... is itself a list. The first
     * element of this list must be the id, and the rest are
     * switches and variable names. For histograms and ntuples with one
     * variable only it is not necessary  to provide the list of variables.
     * Example ntuple specifier: {10 -x var1 -v var2 -e var3}.
     */
    int i;
    char *name, *opt;
    Fit_config *new_fit_config = NULL;
    char stringbuf[64];

    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    name = Tcl_GetStringFromObj(objv[1], NULL);
    if (hs_find_fit_byname(name))
    {
	Tcl_AppendResult(interp, "fit named \"", name, "\" already exists", NULL);
	return TCL_ERROR;
    }
    if (name[0] == '\0')
    {
	/* Come up with an automatic name */
	do {
	    sprintf(stringbuf, "hs_fit_%d", auto_fit_counter++);
	} while(hs_find_fit_byname(stringbuf));
	name = stringbuf;
    }

    /* It is time to actually create the object ... */
    new_fit_config = (Fit_config *)calloc(1, sizeof(Fit_config));
    if (new_fit_config == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    new_fit_config->minimizer = Minuit_minimizers[0];
    new_fit_config->errdef = 1.0;
    new_fit_config->verbose = DEFAULT_MINUIT_VERBOSITY;
    new_fit_config->strategy = 1;
    new_fit_config->interp = interp;
    new_fit_config->status = TCL_OK;
    new_fit_config->dll_list = Tcl_NewListObj(0, NULL);
    Tcl_IncrRefCount(new_fit_config->dll_list);
    Tcl_InitHashTable(&new_fit_config->tcldata, TCL_STRING_KEYS);

    /* Set the default fitting method. Must do it before defining any dataset. */
    for (i=2; i<objc-1; ++i)
    {
	opt = Tcl_GetStringFromObj(objv[i], NULL);
	if (strcmp(opt, "-method") == 0)
	{
	    if (parse_config_options(
		new_fit_config, 0, interp, 2, objv + i) != TCL_OK)
		goto fail0;
	    break;
	}
    }
    if (i == objc-1)
    {
	/* "-method" option not found. Use the system default. */
	new_fit_config->default_fit_method = 
	    hs_get_accum_method_by_tag(DEFAULT_FITTING_METHOD);
	assert(new_fit_config->default_fit_method);
	new_fit_config->default_method = strdup(DEFAULT_FITTING_METHOD);
	if (new_fit_config->default_method == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    goto fail0;
	}
    }

    /* Go over the list of dataset specifiers */
    for (i=2; i<objc; ++i)
    {
	opt = Tcl_GetStringFromObj(objv[i], NULL);
	if (opt[0] == '-')
	{
	    /* This is an option */
	    if (i == objc-1)
	    {
		Tcl_AppendResult(interp, "missing value for option \"",
				 opt, "\"", NULL);
		goto fail0;
	    }
	    else if (parse_config_options(
		new_fit_config, 0, interp, 2, objv + i++) != TCL_OK)
		goto fail0;
	}
	else if (hs_add_fit_subset_tcl(
	    interp, new_fit_config, objv[i]) != TCL_OK)
	    goto fail0;
    }

    /* The lists of callbacks */
    new_fit_config->cb_complete = Tcl_NewListObj(0, NULL);
    if (new_fit_config->cb_complete)
	Tcl_IncrRefCount(new_fit_config->cb_complete);
    else
	goto fail0;
    new_fit_config->cb_delete = Tcl_NewListObj(0, NULL);
    if (new_fit_config->cb_delete)
	Tcl_IncrRefCount(new_fit_config->cb_delete);
    else
	goto fail0;
    new_fit_config->cb_lostsync = Tcl_NewListObj(0, NULL);
    if (new_fit_config->cb_lostsync)
	Tcl_IncrRefCount(new_fit_config->cb_lostsync);
    else
	goto fail0;

    /* Add the fit to the table of fits */
    if (hs_add_fit(interp, name, new_fit_config) != TCL_OK)
	goto fail0;

    Tcl_SetObjResult(interp, Tcl_NewStringObj(name, -1));
    return TCL_OK;

 fail0:
    hs_destroy_fit_config(new_fit_config);
    return TCL_ERROR;
}

tcl_routine(Fit_tcl_fcn)
{
    Fit_config *fit;
    char *fitname, *fcn;
    int dll, code;
    void *myfun;

    tcl_require_objc(4);
    fitname = Tcl_GetStringFromObj(objv[1], NULL);
    fit = hs_find_fit_byname(fitname);
    if (fit == NULL)
    {
	Tcl_AppendResult(interp, "fit named \"", fitname,
			 "\" does not exist", NULL);
	return TCL_ERROR;
    }
    fcn = Tcl_GetStringFromObj(objv[2], NULL);
    if (Tcl_GetIntFromObj(interp, objv[3], &dll) != TCL_OK)
        return TCL_ERROR;
    myfun = hs_find_library_function(interp, dll, fcn, &code);
    if (myfun == NULL)
        return TCL_ERROR;
    fit->user_tcl_fcn = (Minuit_tcl_fcn *)myfun;
    fit->clientData = NULL;
    return TCL_OK;
}

tcl_routine(Fit_exists)
{
    Fit_config *fit;

    tcl_objc_range(1,2);
    if (objc == 2)
	fit = hs_find_fit_byname(Tcl_GetStringFromObj(objv[1], NULL));
    else
	fit = hs_current_fit_conf();
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(fit != NULL));
    return TCL_OK;
}

tcl_routine(Fit_rename)
{
    char *newname, *oldname;

    tcl_require_objc(3);
    oldname = Tcl_GetStringFromObj(objv[1], NULL);
    check_fit_unlocked(oldname,==);
    newname = Tcl_GetStringFromObj(objv[2], NULL);
    return hs_rename_fit(interp, newname, oldname);
}

tcl_routine(Fit_callback)
{
    /* Usage: Fit_callback name add type script
     *        Fit_callback name delete type script
     *        Fit_callback name list type
     *        Fit_callback name clear type
     */
    int i, nelem, found;
    char *op, *ctype, *script, *s, *fitname;
    Tcl_Obj *oldlist;
    Tcl_Obj **listObjElem, **cbptr;
    Fit_config *fit;

    tcl_objc_range(4,5);
    fitname = Tcl_GetStringFromObj(objv[1], NULL);
    fit = hs_find_fit_byname(fitname);
    if (fit == NULL)
    {
	Tcl_AppendResult(interp, "fit named \"", fitname,
			 "\" does not exist", NULL);
	return TCL_ERROR;
    }
    op = Tcl_GetStringFromObj(objv[2], NULL);
    ctype = Tcl_GetStringFromObj(objv[3], NULL);
    if (strcmp(ctype, CB_FIT_DELETE) == 0)
	cbptr = &fit->cb_delete;
    else if (strcmp(ctype, CB_FIT_LOSTSYNC) == 0)
	cbptr = &fit->cb_lostsync;
    else if (strcmp(ctype, CB_FIT_COMPLETE) == 0)
	cbptr = &fit->cb_complete;
    else
    {
	Tcl_AppendResult(interp, "Invalid callback type \"", ctype,
			 "\". Valid callback types are \"",
			 CB_FIT_COMPLETE, "\", \"",
			 CB_FIT_DELETE, "\", and \"",
			 CB_FIT_LOSTSYNC, "\".", NULL);
	return TCL_ERROR;
    }
    if (strcmp(op, "list") == 0)
    {
	tcl_require_objc(4);
	Tcl_SetObjResult(interp, *cbptr);
    }
    else if (strcmp(op, "clear") == 0)
    {
	tcl_require_objc(4);
	check_fit_unlocked(fitname,==);
	Tcl_DecrRefCount(*cbptr);
	*cbptr = Tcl_NewListObj(0, NULL);
	Tcl_IncrRefCount(*cbptr);
    }
    else if (strcmp(op, "add") == 0)
    {
	tcl_require_objc(5);
	script = Tcl_GetStringFromObj(objv[4], NULL);
	if ((s = strdup(script)) == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	if (!Tcl_CommandComplete(s))
	{
	    Tcl_AppendResult(interp, "callback script \"", script,
			     "\" is not a complete tcl command", NULL);
	    return TCL_ERROR;
	}
	free(s);
	if (Tcl_IsShared(*cbptr))
	{
	    oldlist = *cbptr;
	    *cbptr = Tcl_DuplicateObj(*cbptr);
	    Tcl_IncrRefCount(*cbptr);
	    Tcl_DecrRefCount(oldlist);
	}
	Tcl_ListObjAppendElement(interp, *cbptr,
				 Tcl_NewStringObj(script,-1));
    }
    else if (strcmp(op, "del") == 0 ||
	     strcmp(op, "delete") == 0)
    {
	found = 0;
	tcl_require_objc(5);
	script = Tcl_GetStringFromObj(objv[4], NULL);
	do {
	    if (Tcl_ListObjGetElements(interp, *cbptr,
				       &nelem, &listObjElem) != TCL_OK)
		return TCL_ERROR;
	    for (i=0; i<nelem; ++i)
		if (strcmp(script, Tcl_GetStringFromObj(listObjElem[i], NULL)) == 0)
		{
		    found = 1;
		    if (Tcl_IsShared(*cbptr))
		    {
			oldlist = *cbptr;
			*cbptr = Tcl_DuplicateObj(*cbptr);
			Tcl_IncrRefCount(*cbptr);
			Tcl_DecrRefCount(oldlist);
		    }
		    Tcl_ListObjReplace(interp, *cbptr, i, 1, 0, NULL);
		    break;
		}
	} while(i < nelem);
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(found));
    }
    else
    {
	Tcl_AppendResult(interp, "Invalid option \"", op,
			 "\". Expected \"add\", \"clear\","
			 "\"delete\", or \"list\".", NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Fit_copy)
{
    char *newname = NULL, *oldname;
    char stringbuf[64];

    tcl_objc_range(2,3);
    oldname = Tcl_GetStringFromObj(objv[1], NULL);
    if (objc == 3)
    {	
	newname = Tcl_GetStringFromObj(objv[2], NULL);
	if (newname[0] == '\0')
	    newname = NULL;
    }
    if (newname == NULL)
    {
	/* Come up with the name for the copied fit */
	do {
	    sprintf(stringbuf, "hs_fit_%d", auto_fit_counter++);
	} while(hs_find_fit_byname(stringbuf));
	newname = stringbuf;
    }
    if (hs_copy_fit(interp, newname, oldname) != TCL_OK)
	return TCL_ERROR;
    Tcl_SetObjResult(interp, Tcl_NewStringObj(newname, -1));
    return TCL_OK;
}

tcl_routine(Fit_copy_data)
{
    char *fitname, *category;
    Fit_config *fit;
    Tcl_Obj *result = NULL;
    int i, uid, oldid, newid;
    char oldtitle[256];

    tcl_require_objc(3);
    fitname = Tcl_GetStringFromObj(objv[1], NULL);
    category = Tcl_GetStringFromObj(objv[2], NULL);
    fit = hs_find_fit_byname(fitname);
    if (fit == NULL)
    {
	Tcl_AppendResult(interp, "fit named \"",
			 fitname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    result = Tcl_NewListObj(0, NULL);
    uid = hs_next_category_uid(category);
    if (fit->fitsets)
        for (i=0; i<fit->nsets; ++i)
            if (fit->fitsets[i])
            {
                oldid = fit->fitsets[i]->id;
                if (hs_type(oldid) == HS_NONE)
                {
                    sprintf(oldtitle, "%d", oldid);
                    Tcl_AppendResult(interp, "Histo-Scope item with id ",
                                     oldtitle, " no longer exists", NULL);
                    goto fail;
                }
                hs_title(oldid, oldtitle);
                newid = hs_copy_hist(oldid, uid++,
                                     oldtitle, category);
                if (newid <= 0)
                {
                    sprintf(oldtitle, "%d", oldid);
                    Tcl_AppendResult(interp, "failed to copy Histo-Scope "
                                     "item with id ", oldtitle, NULL);
                    goto fail;
                }
                if (Tcl_ListObjAppendElement(interp, result,
                                             Tcl_NewIntObj(newid)) != TCL_OK)
                {
                    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
                    goto fail;
                }
                fit->fitsets[i]->id = newid;
            }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;

 fail:
    if (result)
    {
        Tcl_IncrRefCount(result);
        Tcl_DecrRefCount(result);
    }
    return TCL_ERROR;
}

tcl_routine(Fit_next_name)
{
    char stringbuf[64];

    tcl_require_objc(1);
    do {
	sprintf(stringbuf, "hs_fit_%d", auto_fit_counter++);
    } while(hs_find_fit_byname(stringbuf));
    Tcl_SetObjResult(interp, Tcl_NewStringObj(stringbuf, -1));
    return TCL_OK;
}

tcl_routine(Fit_method_list)
{
    tcl_require_objc(1);
    Tcl_SetObjResult(interp, hs_list_valid_accum_methods());
    return TCL_OK;
}

tcl_routine(Fit_lock_minuit)
{
    int lock;
    tcl_objc_range(1,2);
    if (objc == 1)
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(hs_minuit_is_locked()));
    else
    {
	if (Tcl_GetBooleanFromObj(interp, objv[1], &lock) != TCL_OK)
	    return TCL_ERROR;
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(!hs_minuit_lock(lock)));
    }
    return TCL_OK;
}

tcl_routine(Fit_info)
{
    char *fname;
    Tcl_Obj *descr;
    Fit_config *fit;

    tcl_objc_range(1,2);
    if (objc == 2)
    {
	fname = Tcl_GetStringFromObj(objv[1], NULL);
	fit = hs_find_fit_byname(fname);
	if (fit == NULL)
	{
	    Tcl_AppendResult(interp, "fit named \"",
			     fname, "\" does not exist", NULL);
	    return TCL_ERROR;
	}
    }
    else
    {
	fit = hs_current_fit_conf();
	if (fit == NULL)
	{
	    Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    if (hs_fit_description(interp, fit, &descr) != TCL_OK)
	return TCL_ERROR;
    Tcl_SetObjResult(interp, descr);
    return TCL_OK;
}

tcl_routine(Fit_subset)
{
    /* Usage: Fit_subset add subset_spec_list   (returns number)
     *        Fit_subset count
     *        Fit_subset list
     *        Fit_subset number delete
     *        Fit_subset number exists
     *        Fit_subset number info
     *        Fit_subset number stats data_or_fit quantity
     *        Fit_subset number compiledfilter
     *        Fit_subset number compiledfilter dll_number procname
     *        Fit_subset number fitvalues uid title category calculate_difference
     *        Fit_subset number plotfit result_specifier
     *        Fit_subset number ranges {min max npoints}
     *        Fit_subset number configure ?options?
     *        Fit_subset number cget option
     */
    Fit_config *fit;
    char *op, *procname, *subcommand, *confop, *meth, *cq;
    char *filter_string = NULL;
    int i, dllnumber, set, exists, ierr = 0, ifun, nelem;
    Tcl_Obj *descr, *optionList, *optionPair, *methlist;
    Tcl_Obj **listObjElem;
    Dataset_filter_function *filter = NULL;
    BasicStats *tmpstat;

    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    fit = hs_current_fit_conf();
    if (fit == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    op = Tcl_GetStringFromObj(objv[1],NULL);

    if (strcmp(op, "count") == 0)
    {
	tcl_require_objc(2);
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->nsets));
    }
    else if (strcmp(op, "list") == 0)
    {
	tcl_require_objc(2);
	descr = Tcl_NewListObj(0, NULL);
	for (set = 0; set < fit->nsets; ++set)
	    if (fit->fitsets[set])
		Tcl_ListObjAppendElement(interp, descr, Tcl_NewIntObj(set));
	Tcl_SetObjResult(interp, descr);
    }
    else if (strcmp(op, "add") == 0)
    {
	if (objc < 3 || ((objc - 3) % 2))
	{
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
			     " add : wrong # of arguments", NULL);
	    return TCL_ERROR;
	}
	check_minuit_lock;
	if (hs_add_fit_subset_tcl(interp, fit, objv[2]) != TCL_OK)
	    return TCL_ERROR;
	set = fit->nsets-1;
	if (parse_subset_options(fit, set, 1, interp,
				 objc-3, objv+3) != TCL_OK)
	{
	    hs_destroy_subset_remove_fitters(fit, set);
	    return TCL_ERROR;
	}
	hs_declare_fit_complete(fit, 0);
	Tcl_SetObjResult(interp, Tcl_NewIntObj(set));
    }
    else if (Tcl_GetIntFromObj(interp, objv[1], &set) == TCL_OK)
    {
	if (objc < 3)
	{
	    Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	    return TCL_ERROR;
	}
	subcommand = Tcl_GetStringFromObj(objv[2],NULL);
	exists = 0;
	if (set >= 0 && set < fit->nsets)
	    if (fit->fitsets[set])
	    {
		exists = 1;
		if (hs_type(fit->fitsets[set]->id) == HS_NONE)
		{
		    Tcl_AppendResult(interp, "Histo-Scope item in fit subset ",
				     Tcl_GetStringFromObj(objv[1],NULL),
				     " has been deleted", NULL);
		    return TCL_ERROR;
		}
	    }
	if (strcmp(subcommand, "exists") == 0)
	{
	    tcl_require_objc(3);
	    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(exists));
	}
	else if (!exists)
	{
	    Tcl_AppendResult(interp, "fit subset number ",
			     Tcl_GetStringFromObj(objv[1],NULL),
			     " does not exist", NULL);
	    return TCL_ERROR;
	}
	else if (strcmp(subcommand, "delete") == 0)
	{
	    tcl_require_objc(3);
	    check_minuit_lock;
	    hs_destroy_subset_remove_fitters(fit, set);
	}
	else if (strcmp(subcommand, "stats") == 0)
	{
	    tcl_require_objc(5);
	    confop = Tcl_GetStringFromObj(objv[3],NULL);
	    if (strcmp(confop, "data") == 0)
		tmpstat = &fit->fitsets[set]->data_stats;
	    else if (strcmp(confop, "fit") == 0)
		tmpstat = &fit->fitsets[set]->fit_stats;
	    else
	    {
		Tcl_AppendResult(interp, "invalid stats type \"",
				 confop, "\", expected \"data\" or \"fit\"", NULL);
		return TCL_ERROR;
	    }
	    if (fit->fitsets[set]->datapoints <= 0)
	    {
		Tcl_SetResult(interp, "No stats: the fit has "
			      "not been completed", TCL_STATIC);
		return TCL_ERROR;
	    }
	    descr = Tcl_NewListObj(0, NULL);
	    if (Tcl_ListObjGetElements(interp, objv[4], 
				       &nelem, &listObjElem) != TCL_OK)
		return TCL_ERROR;
	    for (i=0; i<nelem; ++i)
	    {
		cq = Tcl_GetStringFromObj(listObjElem[i],0);
		if (strcmp(cq, "npoints") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewIntObj((int)(tmpstat->goodpoints)));
		}
		else if (strcmp(cq, "is_pdf") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewBooleanObj(!tmpstat->has_neg_values));
		}
		else if (strcmp(cq, "sum") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->integ));
		}
		else if (strcmp(cq, "mean_x") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->mean_x));
		}
		else if (strcmp(cq, "mean_y") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->mean_y));
		}
		else if (strcmp(cq, "mean_z") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->mean_z));
		}
		else if (strcmp(cq, "s_x") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->s_x));
		}
		else if (strcmp(cq, "s_y") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->s_y));
		}
		else if (strcmp(cq, "s_z") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->s_z));
		}
		else if (strcmp(cq, "rho_xy") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->rho_xy));
		}
		else if (strcmp(cq, "rho_xz") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->rho_xz));
		}
		else if (strcmp(cq, "rho_yz") == 0)
		{
		    Tcl_ListObjAppendElement(interp, descr,
				Tcl_NewDoubleObj(tmpstat->rho_yz));
		}
		else
		{
		    Tcl_AppendResult(interp, "Invalid stats quantity \"",
				     cq, "\", expected "
				     "is_pdf, "
				     "mean_x, "
				     "mean_y, "
				     "mean_z, "
				     "npoints, "
				     "rho_xy, "
				     "rho_xz, "
				     "rho_yz, "
				     "s_x, "
				     "s_y, "
				     "s_z, "
				     "or sum.", NULL);
		    return TCL_ERROR;
		}
	    }
	    Tcl_SetObjResult(interp, descr);
	}
	else if (strcmp(subcommand, "info") == 0)
	{
	    tcl_require_objc(3);
	    if (hs_fit_subset_description(
		interp, fit->fitsets[set], &descr) != TCL_OK)
		return TCL_ERROR;
	    Tcl_SetObjResult(interp, descr);
	}
	else if (strcmp(subcommand, "config") == 0 ||
		 strcmp(subcommand, "configure") == 0)
	{
	    if (objc % 2 == 0)
	    {
		Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
		return TCL_ERROR;
	    }
	    if (objc == 3)
	    {
		/* Show all available configuration options */
		optionList = Tcl_NewListObj(0, NULL);

		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("filter", -1));
		if (fit->fitsets[set]->filter_string)
		    Tcl_ListObjAppendElement(
			interp, optionPair,
			Tcl_NewStringObj(fit->fitsets[set]->filter_string, -1));
		else
		    Tcl_ListObjAppendElement(
			interp, optionPair,
			Tcl_NewStringObj("", -1));
		Tcl_ListObjAppendElement(interp, optionList, optionPair);

		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("functions", -1));
		descr = Tcl_NewListObj(0, NULL);
		for (ifun=0; ifun < fit->fitsets[set]->nfitters; ++ifun)
		    Tcl_ListObjAppendElement(interp, descr, Tcl_NewStringObj(
			fit->fitsets[set]->fitter_names[ifun], -1));
		Tcl_ListObjAppendElement(interp, optionPair, descr);
		Tcl_ListObjAppendElement(interp, optionList, optionPair);
	
		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("method", -1));
		assert(fit->fitsets[set]->acc_method_name);
		Tcl_ListObjAppendElement(interp, optionPair,
		     Tcl_NewStringObj(fit->fitsets[set]->acc_method_name, -1));
		Tcl_ListObjAppendElement(interp, optionList, optionPair);

		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("weight", -1));
		Tcl_ListObjAppendElement(interp, optionPair,
				 Tcl_NewDoubleObj(fit->fitsets[set]->weight));
		Tcl_ListObjAppendElement(interp, optionList, optionPair);

		if (!fit->fitsets[set]->binned)
		{
		    optionPair = Tcl_NewListObj(0, NULL);
		    Tcl_ListObjAppendElement(interp, optionPair,
					     Tcl_NewStringObj("normregion", -1));
		    Tcl_ListObjAppendElement(interp, optionList, optionPair);
		    descr = Tcl_NewListObj(0, NULL);
		    Tcl_ListObjAppendElement(interp, descr,
			 Tcl_NewDoubleObj(fit->fitsets[set]->xmin));
		    Tcl_ListObjAppendElement(interp, descr,
			 Tcl_NewDoubleObj(fit->fitsets[set]->xmax));
		    Tcl_ListObjAppendElement(interp, descr,
			 Tcl_NewIntObj(fit->fitsets[set]->nx));
		    if (fit->fitsets[set]->ndim > 1)
		    {
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->ymin));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->ymax));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewIntObj(fit->fitsets[set]->ny));
		    }
		    if (fit->fitsets[set]->ndim > 2)
		    {
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->zmin));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->zmax));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewIntObj(fit->fitsets[set]->nz));
		    }
		    Tcl_ListObjAppendElement(interp, optionPair, descr);
		    Tcl_ListObjAppendElement(interp, optionList, optionPair);
		}

		Tcl_SetObjResult(interp, optionList);
	    }
	    else
	    {
		check_minuit_lock;
		if (parse_subset_options(fit, set, 0, interp,
					 objc-3, objv+3) != TCL_OK)
		    return TCL_ERROR;
	    }
	}
	else if (strcmp(subcommand, "cget") == 0)
	{
	    tcl_require_objc(4);
	    confop = Tcl_GetStringFromObj(objv[3],NULL);
	    if (confop[0] == '-') ++confop;
	    if (strcmp(confop, "weight") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(fit->fitsets[set]->weight));
	    }
	    else if (strcmp(confop, "points") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->fitsets[set]->datapoints));
	    }
	    else if (strcmp(confop, "method") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    fit->fitsets[set]->acc_method_name, -1));
	    }
	    else if (strcmp(confop, "filter") == 0)
	    {
		filter_string = fit->fitsets[set]->filter_string;
		if (filter_string)
		    Tcl_SetObjResult(interp, Tcl_NewStringObj(filter_string, -1));
		else
		    Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
	    }
	    else if (strcmp(confop, "functions") == 0)
	    {
		descr = Tcl_NewListObj(0, NULL);
		for (ifun=0; ifun < fit->fitsets[set]->nfitters; ++ifun)
		    Tcl_ListObjAppendElement(interp, descr, Tcl_NewStringObj(
			fit->fitsets[set]->fitter_names[ifun], -1));
		Tcl_SetObjResult(interp, descr);
	    }
	    else if (strcmp(confop, "id") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->fitsets[set]->id));
	    }
	    else if (strcmp(confop, "columns") == 0)
	    {
		descr = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, descr, Tcl_NewIntObj(
		    fit->fitsets[set]->colx));
		Tcl_ListObjAppendElement(interp, descr, Tcl_NewIntObj(
		    fit->fitsets[set]->coly));
		Tcl_ListObjAppendElement(interp, descr, Tcl_NewIntObj(
		    fit->fitsets[set]->colz));
		Tcl_ListObjAppendElement(interp, descr, Tcl_NewIntObj(
		    fit->fitsets[set]->colval));
		Tcl_ListObjAppendElement(interp, descr, Tcl_NewIntObj(
		    fit->fitsets[set]->colerr));
		Tcl_SetObjResult(interp, descr);
	    }
	    else if (strcmp(confop, "binned") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->fitsets[set]->binned));
	    }
	    else if (strcmp(confop, "ndof") == 0)
	    {
		if (fit->complete)
		    Tcl_SetObjResult(interp, Tcl_NewIntObj(
			fit->fitsets[set]->chisq_used_points));
		else
		    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
	    }
	    else if (strcmp(confop, "chisq") == 0)
	    {
		if (fit->complete)
		    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(
			fit->fitsets[set]->chisq));
		else
		    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(0.0));
	    }
	    else if (strcmp(confop, "normregion") == 0 ||
		     strcmp(confop, "normreg") == 0)
	    {
		descr = Tcl_NewListObj(0, NULL);
		if (!fit->fitsets[set]->binned)
		{
		    Tcl_ListObjAppendElement(interp, descr,
			 Tcl_NewDoubleObj(fit->fitsets[set]->xmin));
		    Tcl_ListObjAppendElement(interp, descr,
			 Tcl_NewDoubleObj(fit->fitsets[set]->xmax));
		    Tcl_ListObjAppendElement(interp, descr,
			 Tcl_NewIntObj(fit->fitsets[set]->nx));
		    if (fit->fitsets[set]->ndim > 1)
		    {
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->ymin));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->ymax));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewIntObj(fit->fitsets[set]->ny));
		    }
		    if (fit->fitsets[set]->ndim > 2)
		    {
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->zmin));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewDoubleObj(fit->fitsets[set]->zmax));
			Tcl_ListObjAppendElement(interp, descr,
			     Tcl_NewIntObj(fit->fitsets[set]->nz));
		    }
		}
		Tcl_SetObjResult(interp, descr);
	    }
	    else if (strcmp(confop, "ndim") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->fitsets[set]->ndim));
	    }
	    else if (strcmp(confop, "validmethods") == 0)
	    {
		descr = Tcl_NewListObj(0, NULL);
		methlist = hs_list_valid_accum_methods();
		Tcl_IncrRefCount(methlist);
		if (Tcl_ListObjGetElements(interp, methlist, 
					   &nelem, &listObjElem) != TCL_OK)
		{
		    Tcl_DecrRefCount(methlist);
		    return TCL_ERROR;
		}
		for (i=0; i<nelem; ++i)
		{
		    meth = Tcl_GetStringFromObj(listObjElem[i], NULL);
		    if (hs_check_fitting_method(interp, fit->fitsets[set], meth))
			Tcl_ListObjAppendElement(interp, descr, Tcl_NewStringObj(meth,-1));
		    else
			Tcl_ResetResult(interp);
		}
		Tcl_SetObjResult(interp, descr);
		Tcl_DecrRefCount(methlist);
	    }
	    else
	    {
		Tcl_AppendResult(interp, "Invalid cget option \"",
				 Tcl_GetStringFromObj(objv[3],NULL),
				 "\". Valid options are: "
				 "-binned, "
				 "-chisq, "
				 "-columns, "
				 "-filter, "
				 "-functions, "
				 "-id, "
				 "-method, "
				 "-ndim, "
				 "-ndof, "
				 "-normregion, "
				 "-points, "
				 "-validmethods, "
				 "and -weight.", NULL);
		return TCL_ERROR;
	    }
	}
	else if (strcmp(subcommand, "kstest") == 0)
	{
	    return fit_subset_kstest(fit, set, interp, objc-2, objv+2);
	}
	else if (strcmp(subcommand, "plotfit") == 0)
	{
	    return fit_subset_plotfit(fit, set, interp, objc-2, objv+2);
	}
	else if (strcmp(subcommand, "random") == 0)
	{
	    return fit_subset_random(fit, set, interp, objc-2, objv+2);
	}
	else if (strcmp(subcommand, "fitvalues") == 0)
	{
	    return fit_subset_fitvalues(fit, set, interp, objc-2, objv+2);
	}
	else if (strcmp(subcommand, "ranges") == 0 || 
		 strcmp(subcommand, "range") == 0)
	{
	    return tcl_c_name(Fit_subset_used_regions) (
		(ClientData)fit->fitsets[set], interp, objc-2, objv+2);
	}
	else if (strcmp(subcommand, "compiledfilter") == 0)
	{
	    if (objc != 3 && objc != 5)
	    {
		Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
				 " : wrong # of arguments", NULL);
		return TCL_ERROR;
	    }
	    if (objc == 3)
	    {
		if (fit->fitsets[set]->filter)
		    i = 1;
		else if (is_blank_string(fit->fitsets[set]->filter_string))
		    i = 1;
		else
		    i = 0;
		Tcl_SetObjResult(interp, Tcl_NewBooleanObj(i));
	    }
	    else
	    {
		check_minuit_lock;
		if (Tcl_GetIntFromObj(interp, objv[3], &dllnumber) != TCL_OK)
		    return TCL_ERROR;
		procname = Tcl_GetStringFromObj(objv[4],NULL);
		if (procname[0] == '\0')
		{
		    fit->fitsets[set]->filter = NULL;
		}
		else
		{
		    filter = (Dataset_filter_function *)
			hs_find_library_function(interp, dllnumber,
						 procname, &ierr);
		    if (ierr < 0)
			return TCL_ERROR;
		    fit->fitsets[set]->filter = filter;
		}
		hs_declare_fit_complete(fit, 0);
	    }
	}
	else
	{
	    Tcl_AppendResult(interp, "Invalid subcommand \"", subcommand,
			     "\". Valid subcommands are: "
			     "cget, "
			     "configure, "
			     "compiledfilter, "
			     "delete, "
			     "exists, "
			     "fitvalues, "
			     "info, "
			     "kstest, "
			     "plotfit, "
			     "random, "
			     "ranges, "
			     "and stats.", NULL);
	    return TCL_ERROR;
	}
    }
    else
    {
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "Invalid option \"",
			 Tcl_GetStringFromObj(objv[1],NULL),
			 "\". Expected \"add\", \"count\", \"list\", ",
			 "or a fit subset number.", NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}



tcl_routine(Fit_parameter)
{
    /* Usage: Fit_parameter add name ?options?
     *        Fit_parameter list
     *        Fit_parameter apply
     *        Fit_parameter info
     *        Fit_parameter clear
     *        Fit_parameter name delete
     *        Fit_parameter name exists
     *        Fit_parameter name set ?value?
     *        Fit_parameter name configure ?options?
     *        Fit_parameter name cget options
     */
    int i, NUM, ierr = 0, parnum, exists;
    Fit_config *fit;
    char *op, *param_name, *subcommand;
    Tcl_Obj *listPtr, *pairPtr, *tmpPtr;
    Minuit_parameter param;
    Minuit_parameter *ppar = NULL;
    double STVAL, STEP, BND1, BND2;

    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    fit = hs_current_fit_conf();
    if (fit == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    op = Tcl_GetStringFromObj(objv[1],NULL);
    memset(&param, 0, sizeof(Minuit_parameter));
    param.step = 0.1;

    if (strcmp(op, "list") == 0)
    {
	tcl_require_objc(2);
	listPtr = Tcl_NewListObj(0, NULL);
	for (i=0; i<fit->n_minuit_pars; ++i)
	    Tcl_ListObjAppendElement(interp, listPtr,
			Tcl_NewStringObj(fit->minuit_pars[i].name, -1));
	Tcl_SetObjResult(interp, listPtr);
    }
    else if (strcmp(op, "clear") == 0)
    {
	tcl_require_objc(2);
	check_minuit_lock;
	for (i=fit->n_minuit_pars-1; i>=0; i--)
	    hs_delete_fit_parameter(fit, i);
    }
    else if (strcmp(op, "info") == 0)
    {
	tcl_require_objc(2);
	listPtr = Tcl_NewListObj(0, NULL);
	for (i=0; i<fit->n_minuit_pars; ++i)
	{
	    if (hs_minuit_param_description(
		interp, fit->minuit_pars+i, 1, &tmpPtr) != TCL_OK)
		return TCL_ERROR;
	    Tcl_ListObjAppendElement(interp, listPtr, tmpPtr);
	}
	Tcl_SetObjResult(interp, listPtr);
    }
    else if (strcmp(op, "apply") == 0)
    {
	tcl_require_objc(2);
	check_minuit_lock;
	if (!fit->par_synched)
	{
	    mncomd_(0, "CLEAR", &ierr, 0, 5);
	    if (ierr)
	    {
		Tcl_SetResult(interp, "Minuit command \"CLEAR\" failed",
			      TCL_STATIC);
		return TCL_ERROR;
	    }
	    for (i=0; i<fit->n_minuit_pars; ++i)
	    {
		NUM = i + 1;
		STVAL = fit->minuit_pars[i].value;
		if (fit->minuit_pars[i].fixed)
		    STEP = 0.0;
		else
		    STEP = fit->minuit_pars[i].step;
		if (fit->minuit_pars[i].has_bounds)
		{
		    BND1 = fit->minuit_pars[i].lolim;
		    BND2 = fit->minuit_pars[i].hilim;
		}
		else
		{
		    BND1 = 0.0;
		    BND2 = 0.0;
		}
		mnparm_(&NUM, fit->minuit_pars[i].name, &STVAL, &STEP,
			&BND1, &BND2, &ierr, strlen(fit->minuit_pars[i].name));
		if (ierr)
		{
		    Tcl_AppendResult(interp, "Failed to initialize Minuit parameter \"",
				     fit->minuit_pars[i].name, "\"", NULL);
		    if (i >= 100)
			Tcl_AppendResult(interp, ". Please check that your ",
					 "version of Minuit can, indeed, handle ",
					 "more than 100 parameters.", NULL);
		    return TCL_ERROR;
		}
	    }
	    fit->par_synched = 1;
	    hs_declare_fit_complete(fit, 0);
	}
    }
    else if (strcmp(op, "add") == 0)
    {
	if (objc < 3 || objc % 2 == 0)
	{
	    Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	    return TCL_ERROR;
	}
	check_minuit_lock;
	param_name = Tcl_GetStringFromObj(objv[2],NULL);
	if (strcmp(param_name, "add") == 0 ||
	    strcmp(param_name, "apply") == 0 ||
	    strcmp(param_name, "list") == 0 ||
	    strcmp(param_name, "clear") == 0 ||
	    strcmp(param_name, "info") == 0)
	{
	    Tcl_AppendResult(interp, "parameter can not be named \"",
			     param_name, "\": this word is reserved", NULL);
	    return TCL_ERROR;
	}
	for (i=0; i<fit->n_minuit_pars; ++i)
	    if (hs_minuit_par_names_cmp(param_name, fit->minuit_pars[i].name) == 0)
	    {
		Tcl_AppendResult(interp, "parameter with name similar to \"",
				 param_name, "\" already exists", NULL);
		return TCL_ERROR;
	    }
	param.name = strdup(param_name);
	checknull(param.name);
	if (tcl_c_name(hs_parse_minuit_param_options) (
	    (ClientData)(&param), interp, objc-3, objv+3) != TCL_OK)
	    goto fail0;
	if (hs_set_fit_parameter(interp, fit, &param) != TCL_OK)
	    goto fail0;
	hs_clear_minuit_param(&param);
	hs_declare_fit_complete(fit, 0);
    }
    else
    {
	/* "op" must be a parameter name */
	param_name = op;
	for (parnum=0; parnum<fit->n_minuit_pars; ++parnum)
	    if (strcmp(param_name, fit->minuit_pars[parnum].name) == 0)
		break;
	exists = (parnum < fit->n_minuit_pars);
	if (exists)
	    ppar = fit->minuit_pars + parnum;
	if (objc < 3)
	{
	    if (exists)
		Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	    else
		Tcl_AppendResult(interp, "Invalid option \"", op,
				 "\". Expected \"add\", \"apply\",",
				 " \"clear\", \"info\", \"list\", ",
				 "or a parameter name.", NULL);
	    return TCL_ERROR;
	}
	subcommand = Tcl_GetStringFromObj(objv[2],NULL);
	if (strcmp(subcommand, "exists") == 0)
	{
	    tcl_require_objc(3);
	    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(exists));
	}
	else if (!exists)
	{
	    Tcl_AppendResult(interp, "Invalid option \"", op,
			     "\". Expected \"add\", \"apply\",",
			     " \"clear\", \"info\", \"list\", ",
			     "or a parameter name.", NULL);
	    return TCL_ERROR;
	}
	else if (strcmp(subcommand, "set") == 0)
	{
	    tcl_objc_range(3,4);
	    if (objc == 3)
	    {
		if (hs_minuit_param_description(
		    interp, fit->minuit_pars+parnum, 0, &listPtr) != TCL_OK)
		    return TCL_ERROR;
		Tcl_SetObjResult(interp, listPtr);
	    }
	    else
	    {
		check_minuit_lock;
		if (hs_set_fit_parameter_tcl(interp, fit,
					     param_name, objv[3]) != TCL_OK)
		    return TCL_ERROR;
		Tcl_SetObjResult(interp, objv[3]);
	    }
	}
	else if (strcmp(subcommand, "delete") == 0)
	{
	    tcl_require_objc(3);
	    check_minuit_lock;
	    hs_delete_fit_parameter(fit, parnum);
	}
	else if (strcmp(subcommand, "configure") == 0 || 
		 strcmp(subcommand, "config") == 0)
	{
	    if (objc % 2 == 0)
	    {
		Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
		return TCL_ERROR;
	    }
	    if (objc == 3)
	    {
		/* Show all available configuration options */
		listPtr = Tcl_NewListObj(0, NULL);

		pairPtr = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, pairPtr,
					 Tcl_NewStringObj("value", -1));
		Tcl_ListObjAppendElement(interp, pairPtr,
					 Tcl_NewDoubleObj(ppar->value));
		Tcl_ListObjAppendElement(interp, listPtr, pairPtr);
		
		pairPtr = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, pairPtr,
					 Tcl_NewStringObj("state", -1));
		if (ppar->fixed)
		    Tcl_ListObjAppendElement(interp, pairPtr,
					     Tcl_NewStringObj("fixed", -1));
		else
		    Tcl_ListObjAppendElement(interp, pairPtr,
					     Tcl_NewStringObj("variable", -1));
		Tcl_ListObjAppendElement(interp, listPtr, pairPtr);
		
		pairPtr = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, pairPtr,
					 Tcl_NewStringObj("step", -1));
		Tcl_ListObjAppendElement(interp, pairPtr,
					 Tcl_NewDoubleObj(ppar->step));
		Tcl_ListObjAppendElement(interp, listPtr, pairPtr);
		
		pairPtr = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, pairPtr,
					 Tcl_NewStringObj("bounds", -1));
		tmpPtr = Tcl_NewListObj(0, NULL);
		if (ppar->has_bounds)
		{
		    Tcl_ListObjAppendElement(interp, tmpPtr,
			Tcl_NewDoubleObj(ppar->lolim));
		    Tcl_ListObjAppendElement(interp, tmpPtr,
			Tcl_NewDoubleObj(ppar->hilim));
		}
		Tcl_ListObjAppendElement(interp, pairPtr, tmpPtr);
		Tcl_ListObjAppendElement(interp, listPtr, pairPtr);

		Tcl_SetObjResult(interp, listPtr);
	    }
	    else
	    {
		check_minuit_lock;
		hs_copy_minuit_param_inplace(&param, fit->minuit_pars+parnum);
		if (tcl_c_name(hs_parse_minuit_param_options) (
		    (ClientData)(&param), interp, objc-3, objv+3) != TCL_OK)
		    goto fail0;
		if (hs_set_fit_parameter(interp, fit, &param) != TCL_OK)
		    goto fail0;
		hs_clear_minuit_param(&param);
	    }
	}
	else if (strcmp(subcommand, "cget") == 0)
	{
	    tcl_require_objc(4);
	    op = Tcl_GetStringFromObj(objv[3],NULL);
	    if (op[0] == '-') ++op;
	    if (strcmp(op, "value") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(ppar->value));
	    }
	    else if (strcmp(op, "state") == 0)
	    {
		if (ppar->fixed)
		    Tcl_SetObjResult(interp, Tcl_NewStringObj("fixed", -1));
		else
		    Tcl_SetObjResult(interp, Tcl_NewStringObj("variable", -1));
	    }
	    else if (strcmp(op, "step") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(ppar->step));
	    }
	    else if (strcmp(op, "globcc") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(ppar->globcc));
	    }
	    else if (strcmp(op, "error") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(ppar->eparab));
	    }
	    else if (strcmp(op, "epos") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(ppar->eplus));
	    }
	    else if (strcmp(op, "eneg") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewDoubleObj(ppar->eminus));
	    }
	    else if (strcmp(op, "errinfo") == 0)
	    {
		listPtr = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(ppar->eparab));
		Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(ppar->eminus));
		Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(ppar->eplus));
		Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(ppar->globcc));
		Tcl_SetObjResult(interp, listPtr);
	    }
	    else if (strcmp(op, "bounds") == 0)
	    {
		listPtr = Tcl_NewListObj(0, NULL);
		if (ppar->has_bounds)
		{
		    Tcl_ListObjAppendElement(interp, listPtr,
					     Tcl_NewDoubleObj(ppar->lolim));
		    Tcl_ListObjAppendElement(interp, listPtr,
					     Tcl_NewDoubleObj(ppar->hilim));
		}
		Tcl_SetObjResult(interp, listPtr);
	    }
	    else
	    {
		Tcl_AppendResult(interp, "Invalid cget option \"",
				 Tcl_GetStringFromObj(objv[3],NULL),
				 "\". Valid options are: "
				 "-bounds, "
				 "-globcc, "
				 "-eneg, "
				 "-epos, "
				 "-errinfo, "
				 "-error, "
				 "-state, "
				 "-step, "
				 "and -value.", NULL);
		return TCL_ERROR;
	    }
	}
	else
	{
	    Tcl_AppendResult(interp, "Invalid subcommand \"", subcommand,
			     "\". Valid subcommands are: "
			     "cget, "
			     "configure, "
			     "delete, "
			     "exists, "
			     "and set.", NULL);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;

 fail0:
    hs_clear_minuit_param(&param);
    return TCL_ERROR;
}

tcl_routine(hs_parse_minuit_param_options)
{
    /* Options: -value, -state, -step, -bounds
     * Option parsing here must be synchronized with the "configure"
     * subcommand inside the Fit_parameter code.
     */
    int pair, npairs, nelem;
    Minuit_parameter *param, *arg;
    char *op, *str;
    Tcl_Obj **listObjElem;
    Minuit_parameter local;
    
    arg = (Minuit_parameter *)clientData;
    assert(arg);
    assert(interp);
    assert(objc % 2 == 0);

    npairs = objc/2;
    if (npairs > 0)
    {
	hs_copy_minuit_param_inplace(&local, arg);
	param = &local;
	for (pair=0; pair<npairs; ++pair)
	{
	    op = Tcl_GetStringFromObj(objv[pair*2],NULL);
	    if (op[0] == '-') ++op;
	    if (strcmp(op, "value") == 0)
	    {
		if (Tcl_GetDoubleFromObj(interp, objv[pair*2+1], &param->value) != TCL_OK)
		    goto fail0;
	    }
	    else if (strcmp(op, "state") == 0)
	    {
		str = Tcl_GetStringFromObj(objv[pair*2+1], NULL);
		if (strcmp(str, "fixed") == 0)
		    param->fixed = 1;
		else if (strcmp(str, "variable") == 0)
		    param->fixed = 0;
		else
		{
		    Tcl_AppendResult(interp, "invalid parameter state \"", str,
				     "\", should be \"variable\" or \"fixed\".", NULL);
		    goto fail0;
		}
	    }
	    else if (strcmp(op, "step") == 0)
	    {
		if (Tcl_GetDoubleFromObj(interp, objv[pair*2+1], &param->step) != TCL_OK)
		    goto fail0;
		if (param->step <= 0.0)
		{
		    Tcl_SetResult(interp,
				  "parameter step size must be positive",
				  TCL_STATIC);
		    goto fail0;
		}
	    }
	    else if (strcmp(op, "bounds") == 0)
	    {
		if (Tcl_ListObjGetElements(interp, objv[pair*2+1],
					   &nelem, &listObjElem) != TCL_OK)
		    goto fail0;
		if (nelem == 0)
		    param->has_bounds = 0;
		else if (nelem == 2)
		{
		    param->has_bounds = 1;
		    if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &param->lolim) != TCL_OK)
			goto fail0;
		    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &param->hilim) != TCL_OK)
			goto fail0;
		    if (param->lolim >= param->hilim)
		    {
			Tcl_AppendResult(
			    interp, "invalid parameter bounds \"",
			    Tcl_GetStringFromObj(objv[pair*2+1],NULL),
			    "\": the lower bound should be less than the upper bound", NULL);
			goto fail0;
		    }
		}
		else
		{
		    Tcl_AppendResult(interp, "invalid parameter bounds \"",
				     Tcl_GetStringFromObj(objv[pair*2+1],NULL), "\"", NULL);
		    goto fail0;
		}
	    }
	    else
	    {
		Tcl_AppendResult(interp, "Invalid configuration option \"",
				 Tcl_GetStringFromObj(objv[pair*2],NULL),
				 "\". Valid options are: "
				 "-bounds, "
				 "-state, "
				 "-step, "
				 "and -value.", NULL);
		goto fail0;
	    }
	}
	hs_clear_minuit_param(arg);
	hs_copy_minuit_param_inplace(arg, &local);
	hs_clear_minuit_param(&local);
    }
    return TCL_OK;

 fail0:
    hs_clear_minuit_param(&local);
    return TCL_ERROR;
}

tcl_routine(Fit_tcldata)
{
    int i, idumm, listlen;
    Fit_config *fit;
    char *action, *hashKey;
    Tcl_HashEntry *hashEntry;
    Tcl_HashSearch hashIter;
    Tcl_Obj *tclData, *hashData;
    Tcl_Obj **listelem;    

    tcl_objc_range(2, 4);
    fit = hs_current_fit_conf();
    if (fit == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    action = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcmp(action, "set") == 0)
    {
	tcl_objc_range(3, 4);
	if (objc == 3)
	{
	    if (Tcl_ListObjGetElements(interp, objv[2], &listlen, &listelem) != TCL_OK)
		return TCL_ERROR;
	    if (listlen % 2)
	    {
		Tcl_SetResult(interp, "the tcldata list must have "
			      "an even number of elements", TCL_VOLATILE);
		return TCL_ERROR;
	    }
	    for (hashEntry = Tcl_FirstHashEntry(&fit->tcldata,&hashIter);
		 hashEntry != NULL; hashEntry = Tcl_NextHashEntry(&hashIter))
	    {
		hashData = (Tcl_Obj *)Tcl_GetHashValue(hashEntry);
		Tcl_DecrRefCount(hashData);
	    }
	    Tcl_DeleteHashTable(&fit->tcldata);
	    Tcl_InitHashTable(&fit->tcldata, TCL_STRING_KEYS);
	    for (i=0; i<listlen/2; ++i)
	    {
		hashKey = Tcl_GetStringFromObj(listelem[i*2], NULL);
		hashEntry = Tcl_CreateHashEntry(&fit->tcldata, hashKey, &idumm);
		if (hashEntry == NULL)
		{
		    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		    return TCL_ERROR;
		}
		Tcl_SetHashValue(hashEntry, listelem[i*2+1]);
		Tcl_IncrRefCount(listelem[i*2+1]);
	    }
	}
	else
	{
	    hashKey = Tcl_GetStringFromObj(objv[2], NULL);
	    hashEntry = Tcl_CreateHashEntry(&fit->tcldata, hashKey, &idumm);
	    if (idumm == 0)
	    {
		/* An existing entry */
		Tcl_DecrRefCount((Tcl_Obj *)Tcl_GetHashValue(hashEntry));
	    }
	    Tcl_SetHashValue(hashEntry, objv[3]);
	    Tcl_IncrRefCount(objv[3]);
	}
    }
    else if (strcmp(action, "get") == 0)
    {
	tcl_objc_range(2, 3);
	if (objc == 2)
	{
	    tclData = Tcl_NewListObj(0, NULL);
	    for (hashEntry = Tcl_FirstHashEntry(&fit->tcldata,&hashIter);
		 hashEntry != NULL; hashEntry = Tcl_NextHashEntry(&hashIter))
	    {
		hashKey = Tcl_GetHashKey(&fit->tcldata, hashEntry);
		hashData = (Tcl_Obj *)Tcl_GetHashValue(hashEntry);
		Tcl_ListObjAppendElement(interp, tclData, Tcl_NewStringObj(hashKey,-1));
		Tcl_ListObjAppendElement(interp, tclData, hashData);
	    }
	    Tcl_SetObjResult(interp, tclData);
	}
	else
	{
	    hashKey = Tcl_GetStringFromObj(objv[2], NULL);
	    hashEntry = Tcl_FindHashEntry(&fit->tcldata, hashKey);
	    if (hashEntry == NULL)
	    {
		Tcl_AppendResult(interp, "bad tcldata index \"", hashKey, "\"", NULL);
		return TCL_ERROR;
	    }
	    hashData = (Tcl_Obj *)Tcl_GetHashValue(hashEntry);
	    Tcl_SetObjResult(interp, hashData);
	}
    }
    else if (strcmp(action, "unset") == 0)
    {
	tcl_require_objc(3);
	hashKey = Tcl_GetStringFromObj(objv[2], NULL);
	hashEntry = Tcl_FindHashEntry(&fit->tcldata, hashKey);
	if (hashEntry)
	{
	    hashData = (Tcl_Obj *)Tcl_GetHashValue(hashEntry);
	    Tcl_DecrRefCount(hashData);
	    Tcl_DeleteHashEntry(hashEntry);
	}
    }
    else if (strcmp(action, "exists") == 0)
    {
	tcl_require_objc(3);
	hashKey = Tcl_GetStringFromObj(objv[2], NULL);
	hashEntry = Tcl_FindHashEntry(&fit->tcldata, hashKey);
	if (hashEntry == NULL)
	    i = 0;
	else
	    i = 1;
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(i));
    }
    else
    {
	Tcl_AppendResult(interp, "bad subcommand \"", action,
			 "\". Valid subcommands are \"get\", "
			 "\"exists\", \"set\", and \"unset\".", NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Fit_config)
{
    Fit_config *fit;
    char command[64];
    int ierr = 0;

    fit = hs_current_fit_conf();
    if (fit == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    if (objc > 1)
    {
	check_minuit_lock;
	if (strcmp(Tcl_GetStringFromObj(objv[1],NULL), "apply") == 0)
	{
	    tcl_require_objc(2);
	    
	    /* Apply current fit options */
	    hs_minuit_quiet();

	    sprintf(command, "SET ERR %g", fit->errdef);
	    mncomd_(0, command, &ierr, 0, strlen(command));
	    assert(ierr == 0);

	    if (fit->nowarnings)
		strcpy(command, "SET NOW");
	    else
		strcpy(command, "SET WAR");
	    mncomd_(0, command, &ierr, 0, strlen(command));
	    assert(ierr == 0);	 

	    sprintf(command, "SET STR %d", fit->strategy);
	    mncomd_(0, command, &ierr, 0, strlen(command));
	    assert(ierr == 0);

	    hs_minuit_set_precision(fit->eps);

	    if (fit->has_grad)
		strcpy(command, "SET GRA 1");
	    else
		strcpy(command, "SET NOG");
	    mncomd_(0, command, &ierr, 0, strlen(command));
	    assert(ierr == 0);	    

	    hs_minuit_verbose(fit->verbose);

	    fit->options_synched = 1;
	    hs_declare_fit_complete(fit, 0);
	    return TCL_OK;
	}
    }
    return parse_config_options(fit, 1, interp, objc-1, objv+1);
}

static int parse_config_options(
    Fit_config *config, int print_config,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    int i, imax, pair, npairs, sync_lost = 0;
    char *opt, *val, *minimizer;
    Tcl_Obj *value;
    char *title = NULL, *description = NULL, *method = NULL;
    double errdef, eps;
    int timeout, verbose, nominos, status, nowarnings;
    int strategy, has_grad, err_ignore;

    assert(config);
    assert(interp);

    if (objc % 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    if (objc == 0)
    {
	if (print_config)
	{
	    /* CHANGE THIS!!! Output available options and their values. */
	    Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    else
    {
	errdef = config->errdef;
	eps = config->eps;
	verbose = config->verbose;
	nowarnings = config->nowarnings;
	strategy = config->strategy;
	has_grad = config->has_grad;
	status = config->status;
	err_ignore = config->ignore_function_errors;
	timeout = config->timeout;
	minimizer = config->minimizer;
	nominos = config->nominos;

	npairs = objc/2;
	for (pair = 0; pair<npairs; ++pair)
	{
	    opt = Tcl_GetStringFromObj(objv[pair*2], NULL);
	    if (opt[0] == '-') ++opt;
	    value = objv[pair*2+1];

	    if (strcmp(opt, "title") == 0)
	    {
		title = Tcl_GetStringFromObj(value, NULL);		    
	    }
	    else if (strcmp(opt, "minimizer") == 0)
	    {
		minimizer = Tcl_GetStringFromObj(value, NULL);
		imax = sizeof(Minuit_minimizers)/sizeof(Minuit_minimizers[0]);
		for (i=0; i<imax; ++i)
		    if (strcmp(minimizer, Minuit_minimizers[i]) == 0)
		    {
			minimizer = Minuit_minimizers[i];
			break;
		    }
		if (i == imax)
		{
		    Tcl_AppendResult(interp, "Bad Minuit minimizer \"",
				     minimizer, "\". Valid minimizers are: ",
				     Minuit_minimizers[0], NULL);
		    for (i=1; i<imax; ++i)
			Tcl_AppendResult(interp, ", ", Minuit_minimizers[i], NULL);
		    Tcl_AppendResult(interp, ".", NULL);
		    goto fail0;
		}
	    }
	    else if (strcmp(opt, "description") == 0)
	    {
		description = Tcl_GetStringFromObj(value, NULL);
	    }
	    else if (strcmp(opt, "method") == 0)
	    {
		method = Tcl_GetStringFromObj(value, NULL);
		if (hs_get_accum_method_by_tag(method) == NULL)
		{
		    Tcl_AppendResult(interp, "Bad fitting method \"", method,
				     "\". Valid fitting methods are:\n", NULL);
		    hs_show_valid_accum_methods(interp);
		    goto fail0;
		}
		if (config->default_method)
		    if (strcmp(method, config->default_method) == 0)
			method = NULL;
	    }
	    else if (strcmp(opt, "warnings") == 0 ||
		     strcmp(opt, "warning") == 0)
	    {
		if (Tcl_GetBooleanFromObj(interp, value, &nowarnings) != TCL_OK)
		    goto fail0;
		nowarnings = !nowarnings;
	    }
	    else if (strcmp(opt, "ignore") == 0)
	    {
		if (Tcl_GetBooleanFromObj(interp, value, &err_ignore) != TCL_OK)
		    goto fail0;
	    }
	    else if (strcmp(opt, "status") == 0)
	    {
		val = Tcl_GetStringFromObj(value, NULL);
		if (strcmp(val, "ok") == 0)
		    status = TCL_OK;
		else if (strcmp(val, "error") == 0)
		    status = TCL_ERROR;
		else
		{
		    Tcl_AppendResult(interp, "Bad status value \"", val,
				     "\", should be \"ok\" or \"error\"", NULL);
		    goto fail0;
		}
	    }
	    else if (strcmp(opt, "minos") == 0)
	    {
		if (Tcl_GetBooleanFromObj(interp, value, &nominos) != TCL_OK)
		    goto fail0;
		nominos = !nominos;
	    }
	    else if (strcmp(opt, "timeout") == 0)
	    {
		if (Tcl_GetIntFromObj(interp, value, &timeout) != TCL_OK)
		    goto fail0;
	    }
	    else if (strcmp(opt, "gradient") == 0)
	    {
		if (Tcl_GetBooleanFromObj(interp, value, &has_grad) != TCL_OK)
		    goto fail0;
	    }
	    else if (strcmp(opt, "errdef") == 0)
	    {
		if (Tcl_GetDoubleFromObj(interp, value, &errdef) != TCL_OK)
		    goto fail0;
		if (errdef <= 0.0)
		    Tcl_AppendResult(interp, "Bad error definition level ",
				     Tcl_GetStringFromObj(value, NULL),
				     ": must be positive", NULL);
	    }
	    else if (strcmp(opt, "precision") == 0)
	    {
		if (Tcl_GetDoubleFromObj(interp, value, &eps) != TCL_OK)
		    goto fail0;
		if (eps < 0.0)
		    Tcl_AppendResult(interp, "Bad precision level ",
				     Tcl_GetStringFromObj(value, NULL),
				     ": can not be negative", NULL);
	    }
	    else if (strcmp(opt, "strategy") == 0)
	    {
		if (Tcl_GetIntFromObj(interp, value, &strategy) != TCL_OK)
		    goto fail0;
		if (strategy < 0 || strategy > 2)
		    Tcl_AppendResult(interp, "Invalid strategy number ",
				     Tcl_GetStringFromObj(value, NULL),
				     ". Valid strategies are 0 (optimize for speed),",
				     " 1 (default), and 2 (optimize for quality).", NULL);
	    }
	    else if (strcmp(opt, "verbose") == 0)
	    {
		if (Tcl_GetIntFromObj(interp, value, &verbose) != TCL_OK)
		    goto fail0;
		if (verbose < -1 || verbose > 3)
		{
		    Tcl_AppendResult(
			interp, "Invalid verbose level ",
			Tcl_GetStringFromObj(value, NULL),
			". Valid levels are:\n",
			"  -1 : no output except from SHOW commands\n",
			"   0 : minimum output (no starting values or intermediate results)\n",
			"   1 : Minuit \"normal\" output level\n",
			"   2 : print intermediate results\n",
			"   3 : show the progress of minimization", NULL);
		    goto fail0;
		}
	    }
	    else
	    {
		Tcl_AppendResult(interp, "Invalid configuration option \"",
				 Tcl_GetStringFromObj(objv[pair*2],NULL),
				 "\". Valid options are: "
				 "-description, "
				 "-errdef, "
				 "-gradient, "
				 "-ignore, "
				 "-method, "
				 "-minimizer, "
				 "-minos, "
				 "-precision, "
				 "-status, "
				 "-strategy, "
				 "-timeout, "
				 "-title, "
				 "-verbose, "
				 "and -warnings.", NULL);
		goto fail0;
	    }
	}

	if (description)
	{
	    checkfree(config->description);
	    config->description = strdup(description);
	    checknull(config->description);
	}
	if (method)
	{
	    checkfree(config->default_method);
	    config->default_method = strdup(method);
	    checknull(config->default_method);
	    config->default_fit_method = hs_get_accum_method_by_tag(method);
	    sync_lost = 1;
	}
	if (title)
	{
	    checkfree(config->title);
	    config->title = strdup(title);
	    checknull(config->title);
	    if (config == hs_current_fit_conf())
	    {
		/* Need to update Minuit job title */
		if (title[0])
		    hs_minuit_set_title(title);
		else
		    hs_minuit_set_title(hs_default_fit_title(
			hs_current_fit_name()));
	    }
	}
	if (config->errdef != errdef)
	{
	    config->errdef = errdef;
	    sync_lost = 1;
	}
	if (config->eps != eps)
	{
	    config->eps = eps;
	    sync_lost = 1;
	}
	config->verbose = verbose;
	config->nowarnings = nowarnings;
	if (config->strategy != strategy)
	{
	    config->strategy = strategy;
	    sync_lost = 1;
	}
	if (config->has_grad != has_grad)
	{
	    config->has_grad = has_grad;
	    sync_lost = 1;
	}
	config->status = status;
	config->timeout = timeout;
	config->ignore_function_errors = err_ignore;
	config->minimizer = minimizer;
	config->nominos = nominos;

	config->options_synched = 0;
	if (sync_lost)
	    hs_declare_fit_complete(config, 0);
    }
    return TCL_OK;

 fail0:
    return TCL_ERROR;
}

tcl_routine(Fit_cget)
{
    /* This function can be used to look up various fit properties.
     *
     * Usage: Fit_cget option
     */
    int i, j;
    char *op;
    const Fit_config *fit;
    Tcl_Obj *minInfo, *ematData, *ematColumn, *ematParNames;
    double wpoints;

    tcl_require_objc(2);
    fit = hs_current_fit_conf();
    if (fit == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    op = Tcl_GetStringFromObj(objv[1],NULL);
    if (op[0] == '-') ++op;

    if (strcmp(op, "compiled") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(hs_is_fit_compiled(fit)));
    }
    else if (strcmp(op, "complete") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(fit->complete));
    }
    else if (strcmp(op, "description") == 0)
    {
	if (fit->description)
	    Tcl_SetObjResult(interp, Tcl_NewStringObj(fit->description,-1));
	else
	    Tcl_SetObjResult(interp, Tcl_NewStringObj("",-1));
    }
    else if (strcmp(op, "errdef") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewDoubleObj(fit->errdef));
    }
    else if (strcmp(op, "gradient") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(!fit->has_grad));
    }
    else if (strcmp(op, "method") == 0)
    {
	assert(fit->default_method);
	Tcl_SetObjResult(interp, Tcl_NewStringObj(fit->default_method,-1));
    }
    else if (strcmp(op, "precision") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewDoubleObj(fit->eps));
    }
    else if (strcmp(op, "strategy") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->strategy));
    }
    else if (strcmp(op, "timeout") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->timeout));
    }
    else if (strcmp(op, "minimizer") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(fit->minimizer,-1));
    }
    else if (strcmp(op, "title") == 0)
    {
	if (fit->title)
	    Tcl_SetObjResult(interp, Tcl_NewStringObj(fit->title,-1));
	else
	    Tcl_SetObjResult(interp, Tcl_NewStringObj("",-1));
    }
    else if (strcmp(op, "emat") == 0)
    {
	ematData = Tcl_NewListObj(0, NULL);
	if (fit->emat)
	{
	    assert(fit->n_variable_pars > 0);
	    for (i=0; i<fit->n_variable_pars; ++i)
	    {
		ematColumn = Tcl_NewListObj(0, NULL);
		for (j=0; j<fit->n_variable_pars; ++j)
		    Tcl_ListObjAppendElement(interp, ematColumn, Tcl_NewDoubleObj(
			fit->emat[i*fit->n_variable_pars + j]));
		Tcl_ListObjAppendElement(interp, ematData, ematColumn);
	    }
	}
	Tcl_SetObjResult(interp, ematData);
    }
    else if (strcmp(op, "epars") == 0)
    {
	ematParNames = Tcl_NewListObj(0, NULL);
	if (fit->emat)
	{   
	    for (i=0; i<fit->n_variable_pars; ++i)
		Tcl_ListObjAppendElement(interp, ematParNames, Tcl_NewStringObj(
		    fit->minuit_pars[fit->ematind[i]].name, -1));
	}
	Tcl_SetObjResult(interp, ematParNames);
    }
    else if (strcmp(op, "verbose") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fit->verbose));
    }
    else if (strcmp(op, "warnings") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(!fit->nowarnings));
    }
    else if (strcmp(op, "ignore") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(fit->ignore_function_errors));
    }
    else if (strcmp(op, "psync") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(fit->par_synched));
    }
    else if (strcmp(op, "minos") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(!fit->nominos));
    }
    else if (strcmp(op, "osync") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(fit->options_synched));
    }
    else if (strcmp(op, "dlls") == 0)
    {
	assert(fit->dll_list);
	Tcl_SetObjResult(interp, fit->dll_list);
    }
    else if (strcmp(op, "ministat") == 0)
    {
	if (fit->complete)
	{
	    if (hs_minim_status_description(
		interp, &fit->ministat, &minInfo) != TCL_OK)
		return TCL_ERROR;
	}
	else
	    minInfo = Tcl_NewListObj(0, NULL);
	Tcl_SetObjResult(interp, minInfo);
    }
    else if (strcmp(op, "wpoints") == 0)
    {
	wpoints = 0.0;
	for (i=0; i<fit->nsets; ++i)
	    if (fit->fitsets[i])
		wpoints += fit->fitsets[i]->weight*fit->fitsets[i]->datapoints;
	Tcl_SetObjResult(interp, Tcl_NewDoubleObj(wpoints));
    }
    else if (strcmp(op, "status") == 0)
    {
	if (fit->status == TCL_OK)
	    Tcl_SetObjResult(interp, Tcl_NewStringObj("ok",-1));
	else
	    Tcl_SetObjResult(interp, Tcl_NewStringObj("error",-1));
    }
    else
    {
	Tcl_AppendResult(interp, "Invalid cget option \"",
			 Tcl_GetStringFromObj(objv[1],NULL),
			 "\". Valid options are: "
			 "-compiled, "
			 "-complete, "
			 "-description, "
			 "-dlls, "
			 "-emat, "
			 "-epars, "
			 "-errdef, "
			 "-gradient, "
			 "-ignore, "
			 "-method, "
			 "-minimizer, "
			 "-ministat, "
			 "-minos, "
			 "-osync, "
			 "-precision, "
			 "-psync, "
			 "-status, "
			 "-strategy, "
			 "-timeout, "
			 "-title, "
			 "-verbose, "
			 "-warnings, "
			 "and -wpoints.", NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

tcl_routine(Fit_function)
{
    /* Usage: Fit_function add funct ?-subsets {n1 n2 ...}? ?options?
     *        Fit_function list
     *        Fit_function funct exists
     *        Fit_function funct info
     *        Fit_function funct delete
     *        Fit_function funct compiledmap
     *        Fit_function funct compiledmap dll_number procname
     *        Fit_function funct configure ?options?
     *        Fit_function funct cget option
     *
     * Each function tag in the fit must be unique.
     */
    int i, enabled_fitsets, exists, removed, dllnumber, ierr = 0;
    char *op, *fittername, *subcommand, *procname;
    Tcl_Obj *resultList, *paramPair, *optionList, *optionPair, *descr;
    Fit_config *fit;
    Fitter_info *newfitter = NULL, *oldfitter;
    Parameter_map_function *mapfun;

    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    fit = hs_current_fit_conf();
    if (fit == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    op = Tcl_GetStringFromObj(objv[1],NULL);

    if (strcmp(op, "list") == 0)
    {
	resultList = Tcl_NewListObj(0, NULL);
	if (fit->fitters)
	    for (i=0; i<fit->nfitters; ++i)
		Tcl_ListObjAppendElement(interp, resultList, Tcl_NewStringObj(
		    fit->fitters[i]->fitter_tag, -1));
	Tcl_SetObjResult(interp, resultList);
    }
    else if (strcmp(op, "add") == 0)
    {
	if (objc < 3 || objc % 2 == 0)
	{
	    Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	    return TCL_ERROR;
	}
	check_minuit_lock;
	fittername = Tcl_GetStringFromObj(objv[2],NULL);
	if (strcmp(fittername, "add") == 0 ||
	    strcmp(fittername, "list") == 0)
	{
	    Tcl_AppendResult(interp, "fit function can not be named \"",
			     fittername, "\": this word is reserved", NULL);
	    return TCL_ERROR;
	}
	if (hs_find_fit_fitter_byname(fit, fittername) != NULL)
	{
	    Tcl_AppendResult(interp, "fit function \"",
			     fittername,
			     "\" is already included in the active fit", NULL);
	    return TCL_ERROR;
	}
	if (hs_find_fitter_function(fittername) == NULL)
	{
	    Tcl_AppendResult(interp, "fit function with tag \"",
			     fittername, "\" does not exist", NULL);
	    return TCL_ERROR;
	}
	/* Check that the fit has at least one subset */
	enabled_fitsets = 0;
	for (i=0; i<fit->nsets; ++i)
	    if (fit->fitsets[i])
		++enabled_fitsets;
	if (enabled_fitsets == 0)
	{
	    Tcl_SetResult(interp,
			  "can't add a fit function: active fit has no subsets",
			  TCL_STATIC);
	    return TCL_ERROR;
	}

	newfitter = hs_create_simple_fitter(fittername);
	checknull(newfitter);

	/* Parse the options */
	if (parse_fitter_options(fit, newfitter, &removed,
				 interp, objc-3, objv+3) != TCL_OK)
	    return TCL_ERROR;
	if (removed)
	{
	    hs_destroy_fit_fitter(newfitter);
	}
	else
	{
	    /* Make sure that the fitter will be included in at least one subset */
	    for (i=0; i<fit->nsets; ++i)
		if (hs_fitter_exists_in_subset(fit->fitsets[i], fittername))
		    break;
	    if (i == fit->nsets)
	    {
		if (enabled_fitsets == 1)
		{
		    for (i=0; i<fit->nsets; ++i)
			if (fit->fitsets[i])
			{
			    if (hs_add_fitter_to_subset(fit->fitsets[i], fittername))
			    {
				Tcl_SetResult(interp, "out of memory", TCL_STATIC);
				goto fail0;
			    }
			    hs_declare_fit_complete(fit, 0);
			    break;
			}
		}
		else
		{
		    Tcl_SetResult(interp,
				  "ambiguous fit subset assignment,"
				  " please use \"-subsets\" option",
				  TCL_STATIC);
		    goto fail0;
		}
	    }
	    /* Add new fitter info to the fit */
	    if (hs_add_fitter_to_config(fit, newfitter))
	    {
		for (i=0; i<fit->nsets; ++i)
		    hs_remove_fitter_from_subset(fit->fitsets[i], fittername);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	}
	hs_declare_fit_complete(fit, 0);
    }
    else
    {
	/* "op" must be a fitter name */
	fittername = op;
	oldfitter = hs_find_fit_fitter_byname(fit, fittername);
	exists = (oldfitter != NULL);
	if (objc < 3)
	{
	    if (exists)
		Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	    else
		Tcl_AppendResult(interp, "Invalid option \"", op,
				 "\". Expected \"add\", \"list\", or a name ",
				 "of a function included in the active fit.", NULL);
	    goto fail0;
	}
	subcommand = Tcl_GetStringFromObj(objv[2],NULL);
	if (strcmp(subcommand, "exists") == 0)
	{
	    tcl_require_objc(3);
	    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(exists));
	}
	else if (!exists)
	{
	    Tcl_AppendResult(interp, "Invalid option \"", op,
			     "\". Expected \"add\", \"list\", or a name ",
			     "of a function included in the active fit.", NULL);
	    goto fail0;
	}
	else if (strcmp(subcommand, "configure") == 0 || 
		 strcmp(subcommand, "config") == 0)
	{
	    if (objc % 2 == 0)
	    {
		Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
		goto fail0;
	    }
	    if (objc == 3)
	    {
		/* Show all available configuration options */
		optionList = Tcl_NewListObj(0, NULL);

		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("mapping", -1));
		if (oldfitter->mapping)
		    Tcl_ListObjAppendElement(
			interp, optionPair,
			Tcl_NewStringObj(oldfitter->mapping, -1));
		else
		    Tcl_ListObjAppendElement(
			interp, optionPair,
			Tcl_NewStringObj("", -1));	
		Tcl_ListObjAppendElement(interp, optionList, optionPair);

		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("subsets", -1));
		resultList = Tcl_NewListObj(0, NULL);
		for (i=0; i<fit->nsets; ++i)
		    if (hs_fitter_exists_in_subset(fit->fitsets[i], fittername))
			Tcl_ListObjAppendElement(interp, resultList, Tcl_NewIntObj(i));
		Tcl_ListObjAppendElement(interp, optionPair, resultList);
		Tcl_ListObjAppendElement(interp, optionList, optionPair);

		optionPair = Tcl_NewListObj(0, NULL);
		Tcl_ListObjAppendElement(interp, optionPair,
					 Tcl_NewStringObj("params", -1));
		resultList = Tcl_NewListObj(0, NULL);
		if (oldfitter->npars > 0)
		    assert(oldfitter->best_pars);
		for (i=0; i<oldfitter->npars; ++i)
		{
		    paramPair = Tcl_NewListObj(0, NULL);
		    Tcl_ListObjAppendElement(
			interp, paramPair,
			Tcl_NewStringObj(oldfitter->fitter->param_names[i], -1));
		    Tcl_ListObjAppendElement(interp, paramPair,
					     Tcl_NewDoubleObj(oldfitter->best_pars[i]));
		    Tcl_ListObjAppendElement(interp, resultList, paramPair);
		}
		Tcl_ListObjAppendElement(interp, optionPair, resultList);
		Tcl_ListObjAppendElement(interp, optionList, optionPair);

		Tcl_SetObjResult(interp, optionList);
	    }
	    else
	    {
		check_minuit_lock;
		if (parse_fitter_options(fit, oldfitter, &removed,
					 interp, objc-3, objv+3) != TCL_OK)
		    return TCL_ERROR;
		if (removed)
		    hs_remove_fitter_from_config(fit, fittername);
		hs_declare_fit_complete(fit, 0);
	    }
	}
	else if (strcmp(subcommand, "compiledmap") == 0)
	{
	    if (objc != 3 && objc != 5)
	    {
		Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),
				 " : wrong # of arguments", NULL);
		return TCL_ERROR;
	    }
	    if (objc == 3)
	    {
		Tcl_SetObjResult(interp, Tcl_NewBooleanObj(oldfitter->map != NULL));
	    }
	    else
	    {
		if (Tcl_GetIntFromObj(interp, objv[3], &dllnumber) != TCL_OK)
		    return TCL_ERROR;
		check_minuit_lock;
		procname = Tcl_GetStringFromObj(objv[4],NULL);
		if (procname[0] == '\0')
		{
		    oldfitter->map = NULL;
		}
		else
		{
		    mapfun = NULL;
		    if (dllnumber == -1)
			mapfun = hs_default_mapping_function(procname);
		    if (mapfun == NULL)
			mapfun = (Parameter_map_function *)
			    hs_find_library_function(interp, dllnumber,
						     procname, &ierr);
		    if (ierr < 0)
			return TCL_ERROR;
		    oldfitter->map = mapfun;
		}
		hs_declare_fit_complete(fit, 0);
	    }
	}
	else if (strcmp(subcommand, "cget") == 0)
	{
	    tcl_require_objc(4);
	    op = Tcl_GetStringFromObj(objv[3],NULL);
	    if (op[0] == '-') ++op;
	    if (strcmp(op, "mapping") == 0 || 
		strcmp(op, "map") == 0)
	    {
		if (oldfitter->mapping)
		    Tcl_SetObjResult(interp, Tcl_NewStringObj(oldfitter->mapping, -1));
		else
		    Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
	    }
	    else if (strcmp(op, "subsets") == 0 ||
		     strcmp(op, "subset") == 0)
	    {
		resultList = Tcl_NewListObj(0, NULL);
		for (i=0; i<fit->nsets; ++i)
		    if (hs_fitter_exists_in_subset(fit->fitsets[i], fittername))
			Tcl_ListObjAppendElement(interp, resultList, Tcl_NewIntObj(i));
		Tcl_SetObjResult(interp, resultList);
	    }
	    else if (strcmp(op, "params") == 0 ||
		     strcmp(op, "parameters") == 0)
	    {
		resultList = Tcl_NewListObj(0, NULL);
		if (oldfitter->npars > 0)
		    assert(oldfitter->best_pars);
		for (i=0; i<oldfitter->npars; ++i)
		{
		    paramPair = Tcl_NewListObj(0, NULL);
		    Tcl_ListObjAppendElement(
			interp, paramPair,
			Tcl_NewStringObj(oldfitter->fitter->param_names[i], -1));
		    Tcl_ListObjAppendElement(interp, paramPair,
					     Tcl_NewDoubleObj(oldfitter->best_pars[i]));
		    Tcl_ListObjAppendElement(interp, resultList, paramPair);
		}
		Tcl_SetObjResult(interp, resultList);
	    }
	    else if (strcmp(op, "parnames") == 0)
	    {
		resultList = Tcl_NewListObj(0, NULL);
		for (i=0; i<oldfitter->npars; ++i)
		    Tcl_ListObjAppendElement(
			interp, resultList,
			Tcl_NewStringObj(oldfitter->fitter->param_names[i], -1));
		Tcl_SetObjResult(interp, resultList);
	    }
	    else if (strcmp(op, "parvalues") == 0)
	    {
		resultList = Tcl_NewListObj(0, NULL);
		if (oldfitter->npars > 0)
		    assert(oldfitter->best_pars);
		for (i=0; i<oldfitter->npars; ++i)
		    Tcl_ListObjAppendElement(interp, resultList,
					     Tcl_NewDoubleObj(oldfitter->best_pars[i]));
		Tcl_SetObjResult(interp, resultList);
	    }
	    else if (strcmp(op, "npars") == 0)
	    {
		Tcl_SetObjResult(interp, Tcl_NewIntObj(oldfitter->npars));
	    }
	    else
	    {
		Tcl_AppendResult(interp, "Invalid cget option \"",
				 Tcl_GetStringFromObj(objv[3],NULL),
				 "\". Valid options are: "
				 "-mapping, "
				 "-npars, "
				 "-params, "
				 "-parnames, "
				 "-parvalues, "
				 "and -subsets.", NULL);
		goto fail0;
	    }
	}
	else if (strcmp(subcommand, "delete") == 0)
	{
	    check_minuit_lock;
	    for (i=0; i<fit->nsets; ++i)
		hs_remove_fitter_from_subset(fit->fitsets[i], oldfitter->fitter_tag);
	    hs_remove_fitter_from_config(fit, fittername);
	}
	else if (strcmp(subcommand, "info") == 0)
	{
	    if (hs_fitter_info_description(interp, fit, oldfitter, &descr) != TCL_OK)
		return TCL_ERROR;
	    Tcl_SetObjResult(interp, descr);
	}
	else
	{
	    Tcl_AppendResult(interp, "Invalid subcommand \"", subcommand,
			     "\". Valid subcommands are: "
			     "cget, "
			     "configure, "
			     "compiledmap, "
			     "delete, "
			     "exists, "
			     "and info.", NULL);
	    goto fail0;
	}
    }
    return TCL_OK;

 fail0:
    hs_destroy_fit_fitter(newfitter);
    return TCL_ERROR;
}

static int parse_subset_options(
    Fit_config *fit, int set, int reduced_option_set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    int i, nx, ny, nz, ifun, pair, npairs, itemtype;
    int nelem, newfun = -1, lost_sync = 0;
    double weight, xmin, xmax, ymin, ymax, zmin, zmax;
    char *confop, *fname, *filter_string = NULL, *newmethod = NULL;
    Tcl_Obj *obj;
    Accumulated_stat_function *statmeth = NULL;
    Tcl_Obj **listObjElem;
    Fitter_info *newfitter;

    assert(fit);
    assert(set >= 0 && set < fit->nsets);
    assert(fit->fitsets[set]);
    assert(interp);
    assert(objc % 2 == 0);
    itemtype = hs_type(fit->fitsets[set]->id);
    assert(itemtype == HS_1D_HISTOGRAM ||
	   itemtype == HS_2D_HISTOGRAM ||
	   itemtype == HS_3D_HISTOGRAM ||
	   itemtype == HS_NTUPLE);

    weight = fit->fitsets[set]->weight;
    npairs = objc/2;
    for (pair=0; pair<npairs; ++pair)
    {
	confop = Tcl_GetStringFromObj(objv[pair*2],NULL);
	if (confop[0] == '-') ++confop;
	obj = objv[pair*2+1];
	if (!reduced_option_set && strcmp(confop, "weight") == 0)
	{
	    if (Tcl_GetDoubleFromObj(interp, obj, &weight) != TCL_OK)
		return TCL_ERROR;
	    if (weight < 0.0)
	    {
		Tcl_SetResult(interp, "dataset weight may not be negative",
			      TCL_STATIC);
		return TCL_ERROR;
	    }
	}
	else if (!reduced_option_set && strcmp(confop, "method") == 0)
	{
	    newmethod = Tcl_GetStringFromObj(obj, NULL);
	    statmeth = hs_check_fitting_method(
		interp, fit->fitsets[set], newmethod);
	    if (statmeth == NULL)
		return TCL_ERROR;
	    if (fit->fitsets[set]->acc_method_name)
		if (strcmp(fit->fitsets[set]->acc_method_name, newmethod) == 0)
		    newmethod = NULL;
	}
	else if (strcmp(confop, "normreg") == 0 ||
		 strcmp(confop, "normregion") == 0)
	{
	    if (fit->fitsets[set]->binned)
	    {
		Tcl_SetResult(interp, "can't specify normalization region "
			      "for a binned dataset", TCL_STATIC);
		return TCL_ERROR;
	    }
	    if (Tcl_ListObjGetElements(
		interp, obj, &nelem, &listObjElem) != TCL_OK)
		return TCL_ERROR;
	    if (nelem == 0)
	    {
		fit->fitsets[set]->xmin = 0.0;
		fit->fitsets[set]->xmax = 0.0;
		fit->fitsets[set]->nx = 0;
		fit->fitsets[set]->ymin = 0.0;
		fit->fitsets[set]->ymax = 0.0;
		fit->fitsets[set]->ny = 0;
		fit->fitsets[set]->zmin = 0.0;
		fit->fitsets[set]->zmax = 0.0;
		fit->fitsets[set]->nz = 0;
	    }
	    else if (nelem == fit->fitsets[set]->ndim * 3)
	    {
		xmin = fit->fitsets[set]->xmin;
		xmax = fit->fitsets[set]->xmax;
		nx = fit->fitsets[set]->nx;
		ymin = fit->fitsets[set]->ymin;
		ymax = fit->fitsets[set]->ymax;
		ny = fit->fitsets[set]->ny;
		zmin = fit->fitsets[set]->zmin;
		zmax = fit->fitsets[set]->zmax;
		nz = fit->fitsets[set]->nz;

		if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &xmin) != TCL_OK)
		    return TCL_ERROR;
		if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &xmax) != TCL_OK)
		    return TCL_ERROR;
		if (Tcl_GetIntFromObj(interp, listObjElem[2], &nx) != TCL_OK)
		    return TCL_ERROR;
		if (xmin >= xmax || nx < 1)
		{
		    Tcl_AppendResult(interp, "invalid normalization region specification \"",
				     Tcl_GetStringFromObj(obj, NULL), "\"", NULL);
		    return TCL_ERROR;
		}
		if (fit->fitsets[set]->ndim > 1)
		{
		    if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &ymin) != TCL_OK)
			return TCL_ERROR;
		    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &ymax) != TCL_OK)
			return TCL_ERROR;
		    if (Tcl_GetIntFromObj(interp, listObjElem[2], &ny) != TCL_OK)
			return TCL_ERROR;
		    if (ymin >= ymax || ny < 1)
		    {
			Tcl_AppendResult(interp, "invalid normalization region specification \"",
					 Tcl_GetStringFromObj(obj, NULL), "\"", NULL);
			return TCL_ERROR;
		    }
		}
		if (fit->fitsets[set]->ndim > 2)
		{
		    if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &zmin) != TCL_OK)
			return TCL_ERROR;
		    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &zmax) != TCL_OK)
			return TCL_ERROR;
		    if (Tcl_GetIntFromObj(interp, listObjElem[2], &nz) != TCL_OK)
			return TCL_ERROR;
		    if (zmin >= zmax || nz < 1)
		    {
			Tcl_AppendResult(interp, "invalid normalization region specification \"",
					 Tcl_GetStringFromObj(obj, NULL), "\"", NULL);
			return TCL_ERROR;
		    }
		}

		fit->fitsets[set]->xmin = xmin;
		fit->fitsets[set]->xmax = xmax;
		fit->fitsets[set]->nx = nx;
		fit->fitsets[set]->ymin = ymin;
		fit->fitsets[set]->ymax = ymax;
		fit->fitsets[set]->ny = ny;
		fit->fitsets[set]->zmin = zmin;
		fit->fitsets[set]->zmax = zmax;
		fit->fitsets[set]->nz = nz;
	    }
	    else
	    {
		Tcl_AppendResult(interp, "invalid normalization region specification \"",
				 Tcl_GetStringFromObj(obj, NULL), "\"", NULL);
		return TCL_ERROR;
	    }
	}
	else if (strcmp(confop, "filter") == 0)
	{
	    filter_string = Tcl_GetStringFromObj(obj, NULL);
	    if (fit->fitsets[set]->filter_string)
		if (strcmp(fit->fitsets[set]->filter_string, filter_string) == 0)
		    filter_string = NULL;
	}
	else if (strcmp(confop, "function") == 0 ||
		 strcmp(confop, "functions") == 0)
	{
	    /* Add/remove functions to/from the given subset */
	    if (Tcl_ListObjGetElements(
		interp, obj, &newfun, &listObjElem) != TCL_OK)
		return TCL_ERROR;
	    /* Check that all function names are OK */
	    for (i=0; i<newfun; ++i)
	    {
		fname = Tcl_GetStringFromObj(listObjElem[i],NULL);
		if (hs_find_fitter_function(fname) == NULL)
		{
		    Tcl_AppendResult(interp, "fit function with tag \"",
				     fname, "\" does not exist", NULL);
		    return TCL_ERROR;
		}
	    }
	}
	else
	{
	    if (reduced_option_set)
	    {
		Tcl_AppendResult(interp, "Invalid option \"",
				 Tcl_GetStringFromObj(objv[pair*2],NULL),
				 "\". Valid options are: "
				 "-filter, "
				 "-functions, "
				 "and -normregion", NULL);
	    }
	    else
	    {
		Tcl_AppendResult(interp, "Invalid configure option \"",
				 Tcl_GetStringFromObj(objv[pair*2],NULL),
				 "\". Valid options are: "
				 "-filter, "
				 "-functions, "
				 "-method, "
				 "-normregion, "
				 "and -weight.", NULL);
	    }
	    return TCL_ERROR;
	}
    }

    if (npairs > 0)
    {
	if (filter_string)
	{
	    checkfree(fit->fitsets[set]->filter_string);
	    fit->fitsets[set]->filter_string = strdup(filter_string);
	    checknull(fit->fitsets[set]->filter_string);
	    fit->fitsets[set]->filter = NULL;
	    lost_sync = 1;
	}
	if (newmethod)
	{
	    checkfree(fit->fitsets[set]->acc_method_name);
	    fit->fitsets[set]->acc_method_name = strdup(newmethod);
	    checknull(fit->fitsets[set]->acc_method_name);
	    fit->fitsets[set]->acc_method = statmeth;
	    lost_sync = 1;
	}
	if (fit->fitsets[set]->weight != weight)
	{
	    fit->fitsets[set]->weight = weight;
	    lost_sync = 1;
	}
	if (newfun >= 0)
	{
	    lost_sync = 1;
	    /* Find functions to remove */
	    for (ifun=fit->fitsets[set]->nfitters-1; ifun >= 0; --ifun)
	    {
		for (i=0; i<newfun; ++i)
		    if (strcmp(fit->fitsets[set]->fitter_names[ifun],
			       Tcl_GetStringFromObj(listObjElem[i],NULL)) == 0)
			break;
		if (i == newfun)
		{
		    /* Remove this function */
		    if (hs_fit_fitter_usage_count(
			fit, fit->fitsets[set]->fitter_names[ifun]) == 1)
			hs_remove_fitter_from_config(
			    fit, fit->fitsets[set]->fitter_names[ifun]);
		    hs_remove_fitter_from_subset(
			fit->fitsets[set],
			fit->fitsets[set]->fitter_names[ifun]);
		}
	    }
	    /* Find functions to add */
	    for (i=0; i<newfun; ++i)
	    {
		fname = Tcl_GetStringFromObj(listObjElem[i],NULL);
		if (!hs_fitter_exists_in_subset(fit->fitsets[set], fname))
		{
		    if (hs_find_fit_fitter_byname(fit, fname) == NULL)
		    {
		        /* Must add fitter to the fit first */
			newfitter = hs_create_simple_fitter(fname);
			if (newfitter == NULL)
			{
			    /* Too difficult to roll things back ... */
			    fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
			    fflush(stderr);
			    abort();
			}
			if (hs_add_fitter_to_config(fit, newfitter))
			{
			    fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
			    fflush(stderr);
			    abort();
			}
		    }
		    if (hs_add_fitter_to_subset(fit->fitsets[set], fname))
		    {
			fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
			fflush(stderr);
			abort();
		    }
		}
	    }
	}
	if (lost_sync)
	    hs_declare_fit_complete(fit, 0);
    }
    
    return TCL_OK;
}

static int parse_fitter_options(
    Fit_config *config, Fitter_info *newfitter, int *removed,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    /* Options: -mapping, -subsets, -parameters.
     * Option parsing here must be synchronized with the "configure"
     * subcommand inside the Fit_function code.
     */
    int i, j, pair, npairs, nelem, two, parsed, set;
    int last_subset_pair = -1;
    char *op, *parname;
    Tcl_Obj **listObjElem;
    Tcl_Obj **pairObj;
    double *pvalues = NULL;
    char *newmapping = NULL;

    assert(config);
    assert(newfitter);
    assert(interp);
    assert(objc % 2 == 0);

    npairs = objc/2;
    *removed = 0;
    for (pair=0; pair<npairs; ++pair)
    {
	op = Tcl_GetStringFromObj(objv[pair*2],NULL);
	if (op[0] == '-') ++op;
	if (strcmp(op, "mapping") == 0 || 
	    strcmp(op, "map") == 0)
	{
	    newmapping = Tcl_GetStringFromObj(objv[pair*2+1],NULL);
	}
	else if (strcmp(op, "subsets") == 0 ||
		 strcmp(op, "subset") == 0)
	{
	    last_subset_pair = pair;
	}
	else if (strcmp(op, "params") == 0 ||
		 strcmp(op, "parameters") == 0)
	{
	    if (Tcl_ListObjGetElements(interp, objv[pair*2+1],
				       &nelem, &listObjElem) != TCL_OK)
		goto fail0;
	    if (nelem > 0)
	    {
		if (pvalues == NULL)
		{
		    pvalues = (double *)malloc(newfitter->npars * sizeof(double));
		    if (pvalues == NULL)
		    {
			Tcl_SetResult(interp, "out if memory", TCL_STATIC);
			goto fail0;
		    }
		    assert(newfitter->best_pars);
		    for (i=0; i<newfitter->npars; ++i)
			pvalues[i] = newfitter->best_pars[i];
		}
		/* Try to see if this is a single parameter specification */
		parsed = 0;
		if (nelem == 2)
		{
		    parname = Tcl_GetStringFromObj(listObjElem[0],NULL);
		    for (j=0; j<newfitter->npars; ++j)
			if (strcmp(newfitter->fitter->param_names[j],parname) == 0)
			{
			    if (Tcl_GetDoubleFromObj(
				interp, listObjElem[1], pvalues+j) != TCL_OK)
				return TCL_ERROR;
			    parsed = 1;
			    break;			    
			}
		}
		/* Parameters are specified as a list of name-value pairs */
		if (!parsed)
		{
		    for (i=0; i<nelem; ++i)
		    {
			if (Tcl_ListObjGetElements(interp, listObjElem[i],
						   &two, &pairObj) != TCL_OK)
			    goto fail0;
			if (two != 2)
			{
			    Tcl_AppendResult(interp, "bad parameter specifier \"",
					     Tcl_GetStringFromObj(listObjElem[i],NULL),
					     "\"", NULL);
			    goto fail0;
			}
			parname = Tcl_GetStringFromObj(pairObj[0],NULL);
			for (j=0; j<newfitter->npars; ++j)
			{
			    if (strcmp(newfitter->fitter->param_names[j],parname) == 0)
			    {
				if (Tcl_GetDoubleFromObj(
				    interp, pairObj[1], pvalues+j) != TCL_OK)
				    return TCL_ERROR;
				break;
			    }
			}
			if (j == newfitter->npars)
			{
			    Tcl_AppendResult(interp, "invalid parameter name \"",
					     parname, "\" for fit function \"",
					     newfitter->fitter_tag, "\"", NULL);
			    goto fail0;
			}
		    }
		}
	    }
	}
	else
	{
	    Tcl_AppendResult(interp, "Invalid configuration option \"",
			     Tcl_GetStringFromObj(objv[pair*2],NULL),
			     "\". Valid options are: "
			     "-mapping, "
			     "-subsets, "
			     "and -params.", NULL);
	    goto fail0;
	}
    }
    if (last_subset_pair >= 0)
    {
	pair = last_subset_pair;
	if (Tcl_ListObjGetElements(interp, objv[pair*2+1],
				   &nelem, &listObjElem) != TCL_OK)
	    goto fail0;

	/* Check that the subset list is parseable */
	for (i=0; i<nelem; ++i)
	{
	    if (Tcl_GetIntFromObj(interp, listObjElem[i], &set) != TCL_OK)
		goto fail0;
	    if (set < 0 || set >= config->nsets)
	    {
		Tcl_AppendResult(interp,
				 Tcl_GetStringFromObj(listObjElem[i],NULL),
				 " is not a valid subset number", NULL);
		goto fail0;
	    }
	    if (config->fitsets[set] == NULL)
	    {
		Tcl_AppendResult(interp,
				 Tcl_GetStringFromObj(listObjElem[i],NULL),
				 " is not a valid subset number", NULL);
		goto fail0;
	    }
	}

	/* Remove the function from all subsets */
	for (i=0; i<config->nsets; ++i)
	    hs_remove_fitter_from_subset(config->fitsets[i], newfitter->fitter_tag);
	*removed = 1;

	/* Add the function to selected subsets */
	for (i=0; i<nelem; ++i)
	{
	    Tcl_GetIntFromObj(interp, listObjElem[i], &set);
	    if (hs_add_fitter_to_subset(config->fitsets[set], newfitter->fitter_tag))
	    {
		fprintf(stderr, "Fatal error: out of memory, unable to recover. Exiting.\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	    }
	    *removed = 0;
	}

	/* Raise the flag that the fit is not complete now */
	hs_declare_fit_complete(config, 0);
    }
    if (pvalues)
    {
	for (i=0; i<newfitter->npars; ++i)
	    newfitter->best_pars[i] = pvalues[i];
	free(pvalues);
	hs_declare_fit_complete(config, 0);
    }
    if (newmapping)
    {
	checkfree(newfitter->mapping);
	newfitter->mapping = strdup(newmapping);
	checknull(newfitter->mapping);    
	newfitter->map = hs_default_mapping_function(newmapping);
	hs_declare_fit_complete(config, 0);
    }
    return TCL_OK;

 fail0:
    if (pvalues)
	free(pvalues);
    return TCL_ERROR;
}

#define calculate_sum_of_functions(x,y,z) do {\
    dfval = 0.0;\
    pointerror = 0;\
    for (ifun=0; ifun<fitset->nfitters; ++ifun)\
    {\
	fitter = fitset->pfitters[ifun]->fitter;\
	if (fitter->fit_c)\
	{\
	    dfval += (fitter->fit_c)(x, y, z, fitter->mode,\
				     fitset->pfitters[ifun]->best_pars, &ierr);\
	}\
	else\
	{\
	    fmode = fitter->mode;\
	    dfval += (fitter->fit_f)(&x, &y, &z, &fmode,\
				     fitset->pfitters[ifun]->best_pars, &ierr);\
	}\
	if (ierr)\
	{\
            pointerror = ierr;\
            ierr = 0;\
	    if (!fit->ignore_function_errors)\
            {\
		report_fitplot_error(interp, fitset, ifun, x, y, z, pointerror);\
		goto fail1;\
            }\
	}\
    }\
    if (pointerror)\
	dfval = 0.0;\
} while(0);

static int fit_subset_plotfit(
    Fit_config *fit, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: plot result_specifier. result_specifier is just an id for
     * a histogram or it must look like this for ntuple: {id {min1 max1 npoints1}}
     * in 1d case or {id {min1 max1 npoints1} {min2 max2 npoints2}} in 2d case.
     */
    typedef struct
    {
	double *var;
	double dmin;
	double dmax;
	int npoints;
    } Scan_info_s;

    int i, j, imax, pointerror, jmax, nspec, id, scandim = 0, ntuple_dim = 0;
    int ifun, fmode, itype, nelem, npoints, nxbins, nybins, ierr = 0;
    Fit_subset *fitset;
    Tcl_Obj **listObjElem, **rangeSpec;
    double x = 0.0, y = 0.0, z = 0.0, dmin, dmax;
/*  Some compilers object to the line below because x and y are not constant */
/*      Scan_info_s scan_info[2] = {{&x, 0.0, 0.0, 0}, {&y, 0.0, 0.0, 0}};   */
    Scan_info_s scan_info[2] = {{NULL, 0.0, 0.0, 0}, {NULL, 0.0, 0.0, 0}};
    char *result_specifier;
    float *data = NULL;
    double *dxp, *dyp;
    float xmin, xmax, ymin, ymax, xstep, ystep;
    float ntuple_data[3];
    double dxmin, dxrange, dxpoints, dymin, dyrange, dypoints, dfval;
    Minuit_fitter_info *fitter;

    scan_info[0].var = &x;
    scan_info[1].var = &y;

    tcl_require_objc_option(2);
    assert(fit);
    assert(set >= 0 && set < fit->nsets);
    fitset = fit->fitsets[set]; 
    assert(fitset);

    if (fitset->ndim > 2)
    {
	Tcl_SetResult(interp, "can't plot n-dimensional fits yet for n > 2",
		      TCL_STATIC);
	return TCL_ERROR;
    }
    if (fit->status != TCL_OK)
    {
	Tcl_SetResult(interp, "fit status is \"error\"", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that the data has been copied */
    if (fitset->npoints == 0)
    {
	hs_init_fit_data_sets(fit);
	if (fit->status != TCL_OK)
	{
	    if (fit->interp != interp)
		Tcl_SetResult(interp, (char *)Tcl_GetStringResult(fit->interp),
                              TCL_VOLATILE);
	    return TCL_ERROR;
	}
    }

    /* Make sure that the subset function pointers are synchronized */
    hs_synch_fit_subsets(fit);
    if (fit->status != TCL_OK)
    {
	if (fit->interp != interp)
	    Tcl_SetResult(interp, (char *)Tcl_GetStringResult(fit->interp),
                          TCL_VOLATILE);
	return TCL_ERROR;
    }

    /* Parse the result specifier */
    result_specifier = Tcl_GetStringFromObj(objv[1], NULL);
    if (Tcl_ListObjGetElements(interp, objv[1], &nelem, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (nelem == 0)
    {
	Tcl_SetResult(interp, "empty result specifier", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, listObjElem[0], &id) != TCL_OK)
	return TCL_ERROR;
    itype = hs_type(id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	if (nelem != 1)
	{
	    Tcl_AppendResult(interp, "invalid 1d histogram result specifier \"",
			     result_specifier, "\"", NULL);
	    return TCL_ERROR;
	}
	scandim = 1;
	nxbins = hs_1d_hist_num_bins(id);
	hs_1d_hist_range(id, &xmin, &xmax);
	scan_info[0].npoints = nxbins;
	xstep = (xmax - xmin)/(float)nxbins;
	scan_info[0].dmin = xmin + 0.5f * xstep;
	if (nxbins == 1)
	    scan_info[0].dmax = scan_info[0].dmin;
	else
	    scan_info[0].dmax = xmax - 0.5f * xstep;
	data = (float *)malloc(nxbins * sizeof(float));
	checknull(data);
	break;

    case HS_2D_HISTOGRAM:
	if (nelem != 1)
	{
	    Tcl_AppendResult(interp, "invalid 2d histogram result specifier \"",
			     result_specifier, "\"", NULL);
	    return TCL_ERROR;
	}
	scandim = 2;
	hs_2d_hist_num_bins(id, &nxbins, &nybins);
	hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
	scan_info[0].npoints = nxbins;
	xstep = (xmax - xmin)/(float)nxbins;
	scan_info[0].dmin = xmin + 0.5f * xstep;
	if (nxbins == 1)
	    scan_info[0].dmax = scan_info[0].dmin;
	else
	    scan_info[0].dmax = xmax - 0.5f * xstep;
	scan_info[1].npoints = nybins;
	ystep = (ymax - ymin)/(float)nybins;
	scan_info[1].dmin = ymin + 0.5f * ystep;
	if (nybins == 1)
	    scan_info[1].dmax = scan_info[1].dmin;
	else
	    scan_info[1].dmax = ymax - 0.5f * ystep;
	data = (float *)malloc(nxbins * nybins * sizeof(float));
	checknull(data);
	break;

    case HS_NTUPLE:
	if (nelem < 2 || nelem > 3)
	{
	    Tcl_AppendResult(interp, "invalid ntuple result specifier \"",
			     result_specifier, "\"", NULL);
	    return TCL_ERROR;
	}
	scandim = nelem - 1;
	ntuple_dim = hs_num_variables(id);
	if (ntuple_dim != scandim + 1)
	{
	    Tcl_AppendResult(interp, "number of ntuple variables is ",
			     "inconsistent with result specifier \"",
			     result_specifier, "\"", NULL);
		return TCL_ERROR;
	}
	/* Parse the range specifiers */
	for (i=1; i<nelem; ++i)
	{
	    if (Tcl_ListObjGetElements(interp, listObjElem[i], &nspec, &rangeSpec) != TCL_OK)
		return TCL_ERROR;
	    if (nspec != 3)
	    {
		Tcl_AppendResult(interp, "invalid ntuple result specifier \"",
				 result_specifier, "\"", NULL);
		return TCL_ERROR;
	    }
	    if (Tcl_GetDoubleFromObj(interp, rangeSpec[0], &dmin) != TCL_OK)
		return TCL_ERROR;
	    if (Tcl_GetDoubleFromObj(interp, rangeSpec[1], &dmax) != TCL_OK)
		return TCL_ERROR;
	    if (Tcl_GetIntFromObj(interp, rangeSpec[2], &npoints) != TCL_OK)
		return TCL_ERROR;
	    if (npoints <= 0)
	    {
		Tcl_AppendResult(interp, "invalid ntuple result specifier \"",
				 result_specifier, "\"", NULL);
		return TCL_ERROR;
	    }
	    scan_info[i-1].dmin = dmin;
	    scan_info[i-1].dmax = dmax;
	    scan_info[i-1].npoints = npoints;
	}
	break;

    case HS_NONE:
	Tcl_AppendResult(interp, "invalid result specifier \"",
			 result_specifier,
			 "\" : Histo-Scope item with id ",
			 Tcl_GetStringFromObj(listObjElem[0], NULL),
			 " doesn't exist", NULL);
	return TCL_ERROR;

    default:
	Tcl_AppendResult(interp, "invalid result specifier \"",
			 result_specifier,
			 "\" : Histo-Scope item with id ",
			 Tcl_GetStringFromObj(listObjElem[0], NULL),
			 " is neither histogram nor ntuple", NULL);   	
	return TCL_ERROR;
    }
    if (scandim != fitset->ndim)
    {
	Tcl_AppendResult(interp,
			 "the dimensionality of the result specifier \"",
			 result_specifier, "\" is inconsistent with ",
			 "the dimensionality of the fit data", NULL);
	goto fail1;
    }

    /* Build the result */
    assert(scandim >= 1 && scandim <= 2);
    hs_reset(id);
    dxp = scan_info[0].var;
    imax = scan_info[0].npoints;
    dxmin = scan_info[0].dmin;
    dxrange = scan_info[0].dmax - scan_info[0].dmin;
    dxpoints = (double)(scan_info[0].npoints);
    if (scandim == 2)
    {
	dyp = scan_info[1].var;
	jmax = scan_info[1].npoints;
	dymin = scan_info[1].dmin;
	dyrange = scan_info[1].dmax - scan_info[1].dmin;
	dypoints = (double)(scan_info[1].npoints);
	for (i=0; i<imax; ++i)
	{
	    *dxp = dxmin + ((double)i / dxpoints) * dxrange;
	    for (j=0; j<jmax; ++j)
	    {
		*dyp = dymin + ((double)j / dypoints) * dyrange;
		calculate_sum_of_functions(x,y,z);
		if (ntuple_dim)
		{
		    ntuple_data[0] = *dxp;
		    ntuple_data[1] = *dyp;
		    ntuple_data[2] = dfval;
		    if (hs_fill_ntuple(id, ntuple_data) != id)
		    {
			Tcl_SetResult(interp, "ntuple fill failed", TCL_STATIC);
			goto fail1;
		    }
		}
		else
		    data[i*jmax + j] = dfval;
	    }
	}
    }
    else
    {
	for (i=0; i<imax; ++i)
	{
	    *dxp = dxmin + ((double)i / dxpoints) * dxrange;
	    calculate_sum_of_functions(x,y,z);
	    if (ntuple_dim)
	    {
		ntuple_data[0] = *dxp;
		ntuple_data[1] = dfval;
		if (hs_fill_ntuple(id, ntuple_data) != id)
		{
		    Tcl_SetResult(interp, "ntuple fill failed", TCL_STATIC);
		    goto fail1;
		}
	    }
	    else
		data[i] = dfval;
	}
    }
    if (itype == HS_1D_HISTOGRAM)
	hs_1d_hist_block_fill(id, data, NULL, NULL);
    else if (itype == HS_2D_HISTOGRAM)
	hs_2d_hist_block_fill(id, data, NULL, NULL);
    else if (itype == HS_NTUPLE)
	;
    else
	assert(0);

    if (data)
	free(data);
    return TCL_OK;

 fail1:
    if (data)
	free(data);
    return TCL_ERROR;
}

#define check_non_negative(what,x,val) do {\
    if (val < 0.0)\
    {\
	Tcl_PrintDouble(interp, x, stringbuf);\
	Tcl_AppendResult(interp, "KS test failed: "\
			 "negative ", what, " value at x = ",\
			 stringbuf, NULL);\
	goto fail1;\
    }\
} while(0);

#define determine_bin_width do {\
    if (itype == HS_NTUPLE)\
    {\
	if (i == 0)\
	    binwidth = points[1].x - points[0].x;\
	else if (i == fitset->npoints - 1)\
	    binwidth = points[i].x - points[i-1].x;\
	else\
	    binwidth = (points[i+1].x - points[i-1].x)/2.0;\
    }\
} while(0);

static int fit_subset_kstest(
    Fit_config *fit, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: kstest npoints
     *   npoints   Tells the number of points used to make the dataset.
     *             Set it to 0 (or omit) if the dataset was just a normal
     *             histogram or ntuple, and it is possible to figure out
     *             the number of points automatically.
     */
    int i, itype, argpoints = 0;
    Fit_subset *fitset;
    DataPoint *points = NULL;
    struct simpson_bin *regularcdf = NULL;
    double kslevel, pointsum = 0.0, maxdistance = 0.0;
    char stringbuf[32];
    Tcl_Obj *result[3];

    if (objc > 2)
    {
	Tcl_AppendResult(interp, "wrong # of arguments for option \"",
			 Tcl_GetStringFromObj(objv[0],NULL), "\"", NULL);
	return TCL_ERROR;
    }
    assert(fit);
    assert(set >= 0 && set < fit->nsets);
    fitset = fit->fitsets[set]; 
    assert(fitset);
    if (objc > 1)
	if (Tcl_GetIntFromObj(interp, objv[1], &argpoints) != TCL_OK)
	    return TCL_ERROR;
    if (argpoints < 0)
    {
	Tcl_AppendResult(interp, "expected a non-negative integer, got ",
			 Tcl_GetStringFromObj(objv[1],NULL), NULL);
	return TCL_ERROR;
    }

    /* Check fit status */
    if (fit->status != TCL_OK)
    {
	Tcl_SetResult(interp, "fit status is \"error\"", TCL_STATIC);
	return TCL_ERROR;
    }
    if (!fit->complete)
    {
	Tcl_SetResult(interp, "the fit is not complete", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check the dataset dimensionality */
    if (fitset->ndim != 1)
    {
	Tcl_SetResult(interp, "KS test can be performed for 1d datasets only",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that the data has been copied */
    assert(fitset->npoints > 0);

    /* Check that the dataset element still exists */
    itype = hs_type(fitset->id);
    if (itype == HS_NONE)
    {
	sprintf(stringbuf, "%d", fitset->id);
	Tcl_AppendResult(interp, "the dataset item with Histo-Scope id ",
			 stringbuf, " has already been deleted", NULL);
	return TCL_ERROR;
    }
    else if (itype == HS_NTUPLE)
    {
	/* Sort the points in the order of increasing x */
	points = (DataPoint *)malloc(fitset->npoints*sizeof(DataPoint));
	checknull(points);
	memcpy(points, fitset->points, fitset->npoints*sizeof(DataPoint));
	qsort(points, fitset->npoints, sizeof(DataPoint),
	      (int (*)(const void *, const void *))data_point_compare_x);
    }
    else if (itype == HS_1D_HISTOGRAM)
    {
	assert(fitset->binned);
	points = fitset->points;
    }
    else
	assert(0);

    if (fitset->binned)
    {
	/* We have it easy. All fit values have already been calculated. */
	double fitsum = 0.0, fsum = 0.0, psum = 0.0;
	double diff, binwidth;

	binwidth = (fitset->xmax - fitset->xmin)/(double)(fitset->nx);
	for (i=0; i<fitset->npoints; ++i)
	{
	    if (points[i].filtered || points[i].errflag)
		continue;
	    check_non_negative("data", points[i].x, points[i].value);
	    check_non_negative("fit", points[i].x, points[i].fit);
	    determine_bin_width;
	    fitsum += (points[i].fit*binwidth);
	    pointsum += (points[i].value*binwidth);
	}
	if (fitsum == 0.0)
	{
	    Tcl_SetResult(interp, "KS test failed: "
			  "all fit values are 0", TCL_STATIC);
	    goto fail1;
	}
	if (pointsum == 0.0)
	{
	    Tcl_SetResult(interp, "KS test failed: "
			  "all data values are 0", TCL_STATIC);
	    goto fail1;
	}
	for (i=0; i<fitset->npoints; ++i)
	{
	    if (points[i].filtered || points[i].errflag)
		continue;
	    determine_bin_width;
	    fsum += (points[i].fit*binwidth);
	    psum += (points[i].value*binwidth);
	    diff = fabs(fsum/fitsum - psum/pointsum);
	    if (diff > maxdistance)
		maxdistance = diff;
	}
    }
    else
    {
	/* This is not easy... First, we need
	   to tabulate the fit function CDF. */
        double x, psum = 0.0, cdfv, diff;
	if (fitset->nx < 1)
	{
	    sprintf(stringbuf, "%d", set);
	    Tcl_AppendResult(interp,  "KS test failed: "
			     "normalization region is not "
			     "defined for subset ", stringbuf, NULL);
	    goto fail1;
	}
        regularcdf = (struct simpson_bin *)malloc(
            fitset->nx*sizeof(struct simpson_bin));
        if (regularcdf == NULL)
        {
            Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    goto fail1;
        }
        if (generate_fitter_cdf(fit, set, interp, fitset->xmin, fitset->xmax,
                                fitset->nx, regularcdf) != TCL_OK)
            goto fail1;

	/* Now, cycle over the data points */
	for (i=0; i<fitset->npoints; ++i)
	{
	    if (points[i].filtered || points[i].errflag)
		continue;
	    if (points[i].x <= fitset->xmin || points[i].x >= fitset->xmax)
		continue;
	    pointsum += 1.0;
	}
	if (pointsum == 0.0)
	{
	    Tcl_SetResult(interp, "KS test failed: "
			  "no useful data points", TCL_STATIC);
	    goto fail1;
	}
	for (i=0; i<fitset->npoints; ++i)
	{
	    if (points[i].filtered || points[i].errflag)
		continue;
	    x = points[i].x;
	    if (x <= fitset->xmin || x >= fitset->xmax)
		continue;
	    psum += 1.0;
            cdfv = find_cdf_value(x, 0.0, fitset->xmin, fitset->xmax,
                                  fitset->nx, regularcdf);
	    diff = fabs(cdfv - psum/pointsum);
	    if (diff > maxdistance)
		maxdistance = diff;
	}
    }
    if (argpoints == 0)
    {
	argpoints = closestint(pointsum, 0.0, 0);
	if (argpoints < 1)
	{
	    Tcl_SetResult(interp, "KS test failed: can't determine "
			  "the number of data points", TCL_STATIC);
	    goto fail1;
	}
    }

    /* Figure out the p-value */
    {
        float ftmp;
	ftmp = (float)(maxdistance * sqrt((double)argpoints));
	kslevel = probkl_(&ftmp);
    }

    if (points == fitset->points) points = NULL;
    checkfree(regularcdf);
    checkfree(points);
    result[0] = Tcl_NewDoubleObj(kslevel);
    result[1] = Tcl_NewDoubleObj(maxdistance);
    result[2] = Tcl_NewIntObj(argpoints);
    Tcl_SetObjResult(interp, Tcl_NewListObj(3, result));
    return TCL_OK;

 fail1:
    if (points == fitset->points) points = NULL;
    checkfree(regularcdf);
    checkfree(points);
    return TCL_ERROR;
}

static int fit_subset_random(
    Fit_config *fit, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: random npoints id?
     *   npoints   is the number of random points to generate
     *
     *   id        is an optional parameter specifying histogram
     *             or ntuple id where the random numbers will
     *             be saved. Set it to 0 (or omit) to return
     *             the numbers in a list of doubles.
     */
    int i, status, itype = HS_NONE, npoints, id_fill = 0;
    Fit_subset *fitset;
    struct simpson_bin *regularcdf = NULL;
    char stringbuf[32];
    Tcl_Obj **result = NULL;
    float *data = NULL;
    double x;

    if (objc < 2 || objc > 3)
    {
	Tcl_AppendResult(interp, "wrong # of arguments for option \"",
			 Tcl_GetStringFromObj(objv[0],NULL), "\"", NULL);
	return TCL_ERROR;
    }
    assert(fit);
    assert(set >= 0 && set < fit->nsets);
    fitset = fit->fitsets[set]; 
    assert(fitset);
    if (Tcl_GetIntFromObj(interp, objv[1], &npoints) != TCL_OK)
        return TCL_ERROR;
    if (npoints < 0)
    {
	Tcl_AppendResult(interp, "expected a non-negative integer, got ",
			 Tcl_GetStringFromObj(objv[1],NULL), NULL);
	return TCL_ERROR;
    }
    if (objc > 2)
	if (Tcl_GetIntFromObj(interp, objv[2], &id_fill) != TCL_OK)
	    return TCL_ERROR;
    if (id_fill < 0)
    {
	Tcl_AppendResult(interp, "expected a non-negative integer, got ",
			 Tcl_GetStringFromObj(objv[2],NULL), NULL);
	return TCL_ERROR;
    }
    if (id_fill > 0)
    {
        itype = hs_type(id_fill);
        switch (itype)
        {
        case HS_1D_HISTOGRAM:
            break;
        case HS_NTUPLE:
            if (hs_num_variables(id_fill) > 1)
            {
                Tcl_SetResult(interp, "can't fill ntuple with "
                              "more than one variable", TCL_STATIC);
                return TCL_ERROR;
            }
            break;
        case HS_NONE:
            Tcl_AppendResult(interp, "item with id ",
                             Tcl_GetStringFromObj(objv[2],NULL),
                             " does not exist", NULL);
            return TCL_ERROR;
        default:
            Tcl_AppendResult(interp, "item with id ",
                             Tcl_GetStringFromObj(objv[2],NULL),
                             " is not a 1d histogram or ntuple", NULL);
            return TCL_ERROR;
        }
    }

    /* Check fit status */
    if (fit->status != TCL_OK)
    {
	Tcl_SetResult(interp, "fit status is \"error\"", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that the data have been copied. This also
       sets up integration ranges for histogram fits. */
    if (fitset->npoints == 0)
    {
	hs_init_fit_data_sets(fit);
	if (fit->status != TCL_OK)
	{
	    if (fit->interp != interp)
		Tcl_SetResult(interp, (char *)Tcl_GetStringResult(fit->interp),
                              TCL_VOLATILE);
	    return TCL_ERROR;
	}
    }

    /* Check the dataset dimensionality */
    if (fitset->ndim != 1)
    {
	Tcl_SetResult(interp, "random number generation is "
                      "supported only for 1d fits", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Make sure that the subset function pointers are synchronized */
    hs_synch_fit_subsets(fit);
    if (fit->status != TCL_OK)
    {
	if (fit->interp != interp)
	    Tcl_SetResult(interp, (char *)Tcl_GetStringResult(fit->interp),
                          TCL_VOLATILE);
	return TCL_ERROR;
    }

    /* Generate the fit function CDF */
    if (fitset->nx < 1)
    {
        sprintf(stringbuf, "%d", set);
        Tcl_AppendResult(interp, "normalization region is not "
                         "defined for subset ", stringbuf, NULL);
        return TCL_ERROR;
    }

    /* Check the number of points */
    if (npoints == 0)
        return TCL_OK;

    regularcdf = (struct simpson_bin *)malloc(
        fitset->nx*sizeof(struct simpson_bin));
    checknull(regularcdf);
    if (generate_fitter_cdf(fit, set, interp, fitset->xmin, fitset->xmax,
                            fitset->nx, regularcdf) != TCL_OK)
        goto fail;

    /* Generate the points */
    data = (float *)malloc(npoints*sizeof(float));
    if (data == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_STATIC);
        goto fail;
    }
    i = npoints;
    ranlux_(data, &i);
    for (i=0; i<npoints; ++i)
    {
	register float ftmp;
	ftmp = data[i];
	if (ftmp <= 0.f || ftmp >= 1.f)
	{
	    status = 1;
	    ranlux_(data+i, &status);
	    --i;
	    continue;
	}
        status = find_inverse_cdf_value(
            (double)ftmp, 0.0, fitset->xmin,
            fitset->xmax, fitset->nx, regularcdf, &x);
        assert(status == 0);
        data[i] = (float)x;
    }

    /* Output the result */
    if (id_fill == 0)
    {
        result = (Tcl_Obj **)malloc(npoints*sizeof(Tcl_Obj *));
        if (result == NULL)
        {
            Tcl_SetResult(interp, "out of memory", TCL_STATIC);
            goto fail;
        }
        for (i=0; i<npoints; ++i)
            result[i] = Tcl_NewDoubleObj((double)(data[i]));
        Tcl_SetObjResult(interp, Tcl_NewListObj(npoints, result));
    }
    else
    {
        if (itype == HS_NTUPLE)
        {
            for (i=0; i<npoints; ++i)
                if (hs_fill_ntuple(id_fill, data+i) != id_fill)
                {
                    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
                    goto fail;
                }
        }
        else if (itype == HS_1D_HISTOGRAM)
        {
            for (i=0; i<npoints; ++i)
                hs_fill_1d_hist(id_fill, data[i], 1.f);
        }
        else
            assert(0);
    }
    checkfree(data);
    checkfree(regularcdf);
    checkfree(result);
    return TCL_OK;

 fail:
    checkfree(data);
    checkfree(regularcdf);
    checkfree(result);
    return TCL_ERROR;
}

static int fit_subset_fitvalues(
    Fit_config *fit, int set,
    Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: fitvalues uid title category calculate_difference
     * This command calculates the fitted values at the coordinates
     * of the data points. "calculate_difference" is a boolean variable
     * which specifies if we should calculate the difference between
     * the data and the fit or just calculate the fit. The command will
     * create the same kind of structure as the one used to represent
     * the data: an Ntuple or a histogram. interp result is set to
     * the Histo-Scope id of the result.
     */
    int i, ifun, nvars, len, uid, itype, calculate_difference;
    int fmode, pointerror, ival, id_result, ierr = 0;
    char *title, *category;
    char stringbuf[32];
    char *result_varnames[4] = {0, 0, 0, 0};
    char x_name[256];
    char y_name[256];
    char z_name[256];
    char val_name[256];
    Minuit_fitter_info *fitter;
    Fit_subset *fitset;
    float *data = NULL;
    double dfval;
    char *fit_label = "Fit value";

    tcl_require_objc_option(5);
    assert(fit);
    assert(set >= 0 && set < fit->nsets);
    fitset = fit->fitsets[set]; 
    assert(fitset);
    if (Tcl_GetIntFromObj(interp, objv[1], &uid) != TCL_OK)
	return TCL_ERROR;
    title = Tcl_GetStringFromObj(objv[2], NULL);
    category = Tcl_GetStringFromObj(objv[3], NULL);
    if (Tcl_GetBooleanFromObj(interp, objv[4], &calculate_difference) != TCL_OK)
	return TCL_ERROR;
    if (fit->status != TCL_OK)
    {
	Tcl_SetResult(interp, "fit status is \"error\"", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that the data has been copied */
    if (fitset->npoints == 0)
    {
	hs_init_fit_data_sets(fit);
	if (fit->status != TCL_OK)
	{
	    if (fit->interp != interp)
		Tcl_SetResult(interp, (char *)Tcl_GetStringResult(fit->interp),
                              TCL_VOLATILE);
	    return TCL_ERROR;
	}
    }

    /* Make sure that the subset function pointers are synchronized */
    hs_synch_fit_subsets(fit);
    if (fit->status != TCL_OK)
    {
	if (fit->interp != interp)
	    Tcl_SetResult(interp, (char *)Tcl_GetStringResult(fit->interp),
                          TCL_VOLATILE);
	return TCL_ERROR;
    }

    /* Check that we can calculate the difference if necessary */
    if (calculate_difference && !fitset->binned)
    {
	Tcl_SetResult(interp, "can't calculate the differences: the fit was unbinned",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check that the dataset element still exists */
    itype = hs_type(fitset->id);
    if (itype == HS_NONE)
    {
	sprintf(stringbuf, "%d", fitset->id);
	Tcl_AppendResult(interp, "the dataset item with Histo-Scope id ",
			 stringbuf, " has already been deleted", NULL);
	return TCL_ERROR;
    }

    /* Create the result object */
    if (itype == HS_NTUPLE)
    {
	nvars = 0;
	if (fitset->colx >= 0)
	{
	    len = hs_variable_name(fitset->id, fitset->colx, x_name);
	    assert(len < 256);
	    x_name[len] = '\0';
	    result_varnames[nvars++] = x_name;
	}
	if (fitset->coly >= 0)
	{
	    len = hs_variable_name(fitset->id, fitset->coly, y_name);
	    assert(len < 256);
	    y_name[len] = '\0';
	    result_varnames[nvars++] = y_name;
	}
	if (fitset->colz >= 0)
	{
	    len = hs_variable_name(fitset->id, fitset->colz, z_name);
	    assert(len < 256);
	    z_name[len] = '\0';
	    result_varnames[nvars++] = z_name;
	}
	if (fitset->colval >= 0)
	{
	    len = hs_variable_name(fitset->id, fitset->colval, val_name);
	    assert(len < 256);
	    val_name[len] = '\0';
	    result_varnames[nvars++] = val_name;
	} else {
	    result_varnames[nvars++] = fit_label;
	}
	assert(nvars == fitset->ndim + 1);
	id_result = hs_create_ntuple(uid, title, category,
				     nvars, result_varnames);
	if (id_result <= 0)
	{
	    Tcl_SetResult(interp, "failed to create the result ntuple",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	data = (float *)malloc(nvars * sizeof(float));
	if (data == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	ival = nvars-1;
	for (i=0; i<fitset->npoints; ++i)
	{
	    calculate_sum_of_functions(fitset->points[i].x,
		  fitset->points[i].y, fitset->points[i].z);
	    data[0] = (float)(fitset->points[i].x);
	    if (fitset->ndim > 1)
		data[1] = (float)(fitset->points[i].y);
	    if (fitset->ndim > 2)
		data[2] = (float)(fitset->points[i].z);
	    if (calculate_difference && !pointerror)
		data[ival] = (float)(fitset->points[i].value - dfval);
	    else
		data[ival] = (float)dfval;
	    hs_fill_ntuple(id_result, data);
	}
    }
    else
    {
	id_result = hs_duplicate_axes(fitset->id, uid, title, category);
	if (id_result <= 0)
	{
	    Tcl_SetResult(interp, "failed to create the result histogram",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	data = (float *)malloc(fitset->npoints * sizeof(float));
	if (data == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	for (i=0; i<fitset->npoints; ++i)
	{
	    calculate_sum_of_functions(fitset->points[i].x,
		fitset->points[i].y, fitset->points[i].z);
	    if (calculate_difference && !pointerror)
		data[i] = (float)(fitset->points[i].value - dfval);
	    else
		data[i] = (float)dfval;
	}
	if (itype == HS_1D_HISTOGRAM)
	    hs_1d_hist_block_fill(id_result, data, NULL, NULL);
	else if (itype == HS_2D_HISTOGRAM)
	    hs_2d_hist_block_fill(id_result, data, NULL, NULL);
	else if (itype == HS_3D_HISTOGRAM)
	    hs_3d_hist_block_fill(id_result, data, NULL, NULL);
	else
	    assert(0);
    }

    if (data)
	free(data);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(id_result));
    return TCL_OK;

 fail1:
    if (data)
	free(data);
    return TCL_ERROR;
}

static void report_fitplot_error(
    Tcl_Interp *interp, Fit_subset *fitset, int ifun,
    double x, double y, double z, int ierr)
{
    char stringbuf[32];
    int k, npars;
    char *fname;
    Minuit_fitter_info *value;
    const double *pars;

    fname = fitset->pfitters[ifun]->fitter_tag;
    value = fitset->pfitters[ifun]->fitter;
    pars = fitset->pfitters[ifun]->best_pars;
    npars = hs_fitter_num_pars(value);
    sprintf(stringbuf, "%d", ierr);
    Tcl_AppendResult(interp, "failed to evaluate function ",
		     fname, " (error code ", stringbuf, ") for\n", NULL);
    sprintf(stringbuf, "%d", value->mode);
    Tcl_AppendResult(interp, "mode = ", stringbuf, "\n", NULL);
    Tcl_PrintDouble(interp, x, stringbuf);
    Tcl_AppendResult(interp, "x = ", stringbuf, "\n", NULL);
    if (value->ndim > 1)
    {
	Tcl_PrintDouble(interp, y, stringbuf);
	Tcl_AppendResult(interp, "y = ", stringbuf, "\n", NULL);
    }
    if (value->ndim > 2)
    {
	Tcl_PrintDouble(interp, z, stringbuf);
	Tcl_AppendResult(interp, "z = ", stringbuf, "\n", NULL);
    }
    for (k=0; k<npars; ++k)
    {
	Tcl_PrintDouble(interp, pars[k], stringbuf);
	Tcl_AppendResult(interp, value->param_names[k],
			 " = ", stringbuf, "\n", NULL);
    }
    Tcl_AppendResult(interp, "The fitted data belongs to ", NULL);
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
	Tcl_AppendResult(interp, "deleted Histo-Scope item", NULL);
	break;
    default:
	assert(0);
    }
    sprintf(stringbuf, "%d", fitset->id);
    Tcl_AppendResult(interp, " with id ", stringbuf, "\n", NULL);
}

tcl_routine(Fit_has_tcl_fcn)
{
    int hasit;
    Fit_config *config;
    config = hs_current_fit_conf();
    if (config == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    if (config->user_tcl_fcn)
        hasit = 1;
    else
        hasit = 0;
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(hasit));
    return TCL_OK;
}

tcl_routine(Fit_tcl_fcn_args)
{
    int i;
    Fit_config *config;

    config = hs_current_fit_conf();
    if (config == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    if (config->objv)
    {
        for (i=0; i<config->objc; ++i)
            if (config->objv[i]) {
                Tcl_DecrRefCount(config->objv[i]);
            }
        free(config->objv);
        config->objv = NULL;
        config->objc = 0;
    }
    if (objc > 1)
    {
        ++objv;
        --objc;
        config->objv = (Tcl_Obj **)malloc(objc*sizeof(Tcl_Obj *));
        checknull(config->objv);
        memcpy(config->objv, objv, objc*sizeof(Tcl_Obj *));
        for (i=0; i<objc; ++i)
            Tcl_IncrRefCount(objv[i]);
        config->objc = objc;
    }
    return TCL_OK;
}

tcl_routine(Fit_apply_mapping)
{
    int i, npars;
    Fit_config *fit_config;
    static double *xval = NULL;
    static int nxval = 0;

    tcl_require_objc(1);
    fit_config = hs_current_fit_conf();
    if (fit_config == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    check_minuit_lock;

    /* Check whether all parameter maps are compiled */
    for (i = 0; i < fit_config->nfitters; ++i)
    {
	assert(fit_config->fitters[i]);
	if (fit_config->fitters[i]->map == NULL)
	{
	    Tcl_SetResult(interp,
			  "the parameter maps are not compiled",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	if (fit_config->fitters[i]->best_pars == NULL)
	    fit_config->fitters[i]->best_pars = 
		(double *)malloc(fit_config->fitters[i]->npars*sizeof(double));
	if (fit_config->fitters[i]->best_pars == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
    }

    /* Get the fit parameter values into a single array */
    npars = fit_config->n_minuit_pars;
    if (npars != nxval && npars > 0)
    {
	nxval = npars;
	xval = (double *)realloc(xval, nxval*sizeof(double));
	if (xval == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    for (i = 0; i < npars; ++i)
	xval[i] = fit_config->minuit_pars[i].value;

    /* Apply the mappings */
    for (i = 0; i < fit_config->nfitters; ++i)
    {
	fit_config->fitters[i]->local_pars =
	    (fit_config->fitters[i]->map)(
		xval, fit_config->fitters[i]->offset);
	memcpy(fit_config->fitters[i]->best_pars,
	       fit_config->fitters[i]->local_pars,
	       fit_config->fitters[i]->npars*sizeof(double));
    }

    return TCL_OK;
}

tcl_routine(Fit_append_dll)
{
    int dll;
    Fit_config *fit_config;
    Tcl_Obj *oldlist;

    tcl_require_objc(2);
    fit_config = hs_current_fit_conf();
    if (fit_config == NULL)
    {
	Tcl_SetResult(interp, "No active fit", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[1], &dll) != TCL_OK)
	return TCL_ERROR;
    if (hs_incr_dll_refcount(dll))
    {
	Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			 " is not a valid dll number", NULL);
	return TCL_ERROR;
    }
    if (Tcl_IsShared(fit_config->dll_list))
    {
	oldlist = fit_config->dll_list;
	fit_config->dll_list = Tcl_DuplicateObj(fit_config->dll_list);
	Tcl_IncrRefCount(fit_config->dll_list);
	Tcl_DecrRefCount(oldlist);
    }
    Tcl_ListObjAppendElement(interp, fit_config->dll_list, objv[1]);
    return TCL_OK;
}

tcl_routine(Parse_minuit_pars_in_a_map)
{
    /* Usage: command_name text minuit_pars c_array_name */
    int i, npars, parameter_num;
    unsigned long num;
    char *text, *c_array_name, *pc, *endptr, *start_variable, *start_text;
    char **minuit_pars = NULL;
    char thischar;
    Tcl_Obj **listObjElem;
    char stringbuf[32], array_name[128];
    Tcl_DString out;

    tcl_require_objc(4);
    text = Tcl_GetStringFromObj(objv[1], NULL);
    if (Tcl_ListObjGetElements(interp, objv[2], &npars, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (npars > 0)
    {
	minuit_pars = (char **)calloc(npars, sizeof(char *));
	checknull(minuit_pars);
	for (i=0; i<npars; ++i)
	    minuit_pars[i] = Tcl_GetStringFromObj(listObjElem[i], NULL);
    }
    c_array_name = Tcl_GetStringFromObj(objv[3], NULL);
    memset(array_name, 0, sizeof(array_name));
    strncpy(array_name, c_array_name, sizeof(array_name)-1);

    Tcl_DStringInit(&out);
    start_variable = NULL;
    start_text = text;
    pc = text;
    do {
	if (start_variable && start_text)
	    assert(0);
	else if (start_variable)
	{
	    /* Search for a non-alnum character and not '_' */
	    if ((!isalnum(*pc) && *pc != '_') || *pc == '\0')
	    {
		start_text = pc;
		if (*pc == '%')
		{
		    /* This is just '%' escaped by '%' */
		    ;
		}
		else
		{
		    /* Check if this is a correct parameter name */
		    if (pc - start_variable == 1)
		    {
			sprintf(stringbuf, "%%%c", (int)(*pc));
			Tcl_AppendResult(interp, "bad parameter specifier \"",
					 stringbuf, "\"", NULL);
			goto fail0;
		    }
		    parameter_num = -1;
		    thischar = *pc;
		    *pc = '\0';
		    for (i=0; i<npars && parameter_num < 0; ++i)
			if (strcmp(start_variable+1, minuit_pars[i]) == 0)
			    parameter_num = i;
		    if (parameter_num < 0)
		    {
			/* Check if we can treat the string as an integer */
			num = strtoul(start_variable+1, &endptr, 10);
			if (*endptr == '\0')
			{
			    if (num >= (unsigned long)npars)
			    {
				Tcl_AppendResult(interp, "bad parameter specifier \"",
						 start_variable, "\": parameter number ",
						 "is out of range", NULL);
				*pc = thischar;
				goto fail0;
			    }
			    else
				parameter_num = num;
			}
		    }
		    if (parameter_num < 0)
		    {
			Tcl_AppendResult(interp, "bad parameter specifier \"",
					 start_variable, "\"", NULL);
			*pc = thischar;
			goto fail0;
		    }
		    *pc = thischar;
		    Tcl_DStringAppend(&out, array_name, -1);
		    Tcl_DStringAppend(&out, "[", -1);
		    sprintf(stringbuf, "%d", parameter_num);
		    Tcl_DStringAppend(&out, stringbuf, -1);
		    Tcl_DStringAppend(&out, "]", -1);
		}
		start_variable = NULL;
	    }
	}
	else if (start_text)
	{
	    /* Search for % sign */
	    if (*pc == '%' || *pc == '\0')
	    {
		start_variable = pc;
		Tcl_DStringAppend(&out, start_text, pc - start_text);
		start_text = NULL;
	    }
	}
	else
	    assert(0);
    } while (*pc++);

    Tcl_DStringResult(interp, &out);
    checkfree(minuit_pars);
    Tcl_DStringFree(&out);
    return TCL_OK;

 fail0:
    checkfree(minuit_pars);
    Tcl_DStringFree(&out);
    return TCL_ERROR;
}

tcl_routine(Parse_fitter_pars_in_a_map)
{
    /* Usage: command_name text fitter_pars c_array_name */
    int i, npars, parameter_num, state;
    char *text, *c_array_name, *pc, *start_variable, *start_text;
    char **fitter_pars = NULL;
    char thischar, oldchar;
    Tcl_Obj **listObjElem;
    char stringbuf[32], array_name[128];
    Tcl_DString out;
    /* States for the parser */
    enum {
	IN_TEXT = 0,
	IN_VARIABLE,
	IN_CHAR,
	IN_COMMENT,
	IN_STRING
    };

    tcl_require_objc(4);
    text = Tcl_GetStringFromObj(objv[1], NULL);
    if (Tcl_ListObjGetElements(interp, objv[2], &npars, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (npars > 0)
    {
	fitter_pars = (char **)calloc(npars, sizeof(char *));
	checknull(fitter_pars);
	for (i=0; i<npars; ++i)
	    fitter_pars[i] = Tcl_GetStringFromObj(listObjElem[i], NULL);
    }
    c_array_name = Tcl_GetStringFromObj(objv[3], NULL);
    memset(array_name, 0, sizeof(array_name));
    strncpy(array_name, c_array_name, sizeof(array_name)-1);

    Tcl_DStringInit(&out);
    pc = text;
    start_text = text;
    start_variable = NULL;

    /* Text parser is a state machine */
    oldchar = '\0';
    state = IN_TEXT;
    do {
	thischar = *pc;
	/* Describe state transitions */
	switch (state)
	{
	case IN_TEXT:
	    /* Look for the start of a comment */
	    if (thischar == '*' && oldchar == '/')
		state = IN_COMMENT;
	    /* Look for the start of a char */
	    else if (thischar == '\'')
		state = IN_CHAR;
	    /* Look for the start of a string */
	    else if (thischar == '"')
		state = IN_STRING;
	    else
	    {
		/* Search for a letter which doesn't start after dot or digit */
		if (((isalpha(thischar) || thischar == '_') && 
		     oldchar != '.' && !isdigit(oldchar)) || thischar == '\0')
		{
		    start_variable = pc;
		    Tcl_DStringAppend(&out, start_text, pc - start_text);
		    start_text = NULL;
		    state = IN_VARIABLE;
		}
	    }
	    break;

	case IN_VARIABLE:
	    /* Search for a non-alnum character and not '_' */
	    if ((!isalnum(thischar) && thischar != '_') || thischar == '\0')
	    {
		start_text = pc;
		/* Check if this is a correct parameter name */
		parameter_num = -1;
		*pc = '\0';
		for (i=0; i<npars && parameter_num < 0; ++i)
		    if (strcmp(start_variable, fitter_pars[i]) == 0)
			parameter_num = i;
		*pc = thischar;
		if (parameter_num >= 0)
		{
		    /* Perform substitution */
		    Tcl_DStringAppend(&out, array_name, -1);
		    Tcl_DStringAppend(&out, "[", -1);
		    sprintf(stringbuf, "%d", parameter_num);
		    Tcl_DStringAppend(&out, stringbuf, -1);
		    Tcl_DStringAppend(&out, "]", -1);
		}
		else
		{
		    /* Pass the stuff as it is */
		    Tcl_DStringAppend(&out, start_variable, pc - start_variable);
		}
		start_variable = NULL;

		/* Check what is going to be the next state */
		if (thischar == '*' && oldchar == '/')
		    state = IN_COMMENT;
		/* Look for the start of a char */
		else if (thischar == '\'')
		    state = IN_CHAR;
		/* Look for the start of a string */
		else if (thischar == '"')
		    state = IN_STRING;
		else
		    state = IN_TEXT;
	    }
	    break;

	case IN_CHAR:
	    /* Look for the end of char */
	    if (thischar == '\\' && oldchar == '\\')
		thischar = 255;
	    if (thischar == '\'' && oldchar != '\\')
		state = IN_TEXT;
	    break;

	case IN_COMMENT:
	    /* Look for the end of comment */
	    if (thischar == '/' && oldchar == '*')
		state = IN_TEXT;
	    break;
	    
	case IN_STRING:
	    /* Look for the end of string -- an unescaped quote. */
	    if (thischar == '\\' && oldchar == '\\')
		thischar = 255;
	    if (thischar == '"' && oldchar != '\\')
		state = IN_TEXT;
	    break;
	    
	default:
	    assert(0);
	}
	oldchar = thischar;
    } while (*pc++);

    if (state != IN_VARIABLE && state != IN_TEXT)
    {
	if (start_text)
	    Tcl_DStringAppend(&out, start_text, pc - start_text - 1);
	else if (start_variable)
	    Tcl_DStringAppend(&out, start_variable, pc - start_variable - 1);
	else
	    assert(0);
    }

    Tcl_DStringResult(interp, &out);
    checkfree(fitter_pars);
    Tcl_DStringFree(&out);
    return TCL_OK;
}

#define multires_cleanup do {\
    checkfree(old_smoothing_curves);\
    checkfree(mem);\
    n_oldcurves = 0;\
    n_oldbins = 0;\
    m = -1;\
    binwidth = 0.0;\
} while(0);

tcl_routine(Fit_multires_fast_cycle)
{
    static int *old_smoothing_curves = NULL;
    static int n_oldcurves = 0, n_oldbins = 0, m = -1;
    static void *mem = NULL;
    static double binwidth = 0.0;
    int i, icurve, id, listlen, id_fft, need_cleanup;
    Tcl_Obj **listObjElem;
    float **data;
    float *fft, *work, *data1, *data2;
    Tcl_Obj *result;
    double sum;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &id_fft) != TCL_OK)
        return TCL_ERROR;
    if (id_fft == 0)
    {
        multires_cleanup;
        return TCL_OK;
    }
    if (hs_type(id_fft) != HS_1D_HISTOGRAM)
    {
	if (hs_type(id_fft) == HS_NONE)
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a valid Histo-Scope id", NULL);
	else
	    Tcl_AppendResult(interp, "item with id ",
			     Tcl_GetStringFromObj(objv[1], NULL),
			     " is not a 1d histogram", NULL);
	return TCL_ERROR;
    }
    if (Tcl_ListObjGetElements(interp, objv[2],
			       &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 0)
    {
        multires_cleanup;
        return TCL_OK;
    }
    need_cleanup = (listlen != n_oldcurves || n_oldbins <= 0);
    if (!need_cleanup)
    {
        for (i=0; i<listlen; ++i)
        {
            if (Tcl_GetIntFromObj(interp, listObjElem[i], &id) != TCL_OK)
                return TCL_ERROR;
            if (id != old_smoothing_curves[i])
            {
                need_cleanup = 1;
                break;
            }
        }
    }
    if (need_cleanup)
    {
        multires_cleanup;
        old_smoothing_curves = (int *)malloc(listlen*sizeof(int));
        if (old_smoothing_curves == NULL)
        {
            multires_cleanup;
            Tcl_SetResult(interp, "out of memory", TCL_STATIC);
            return TCL_ERROR;
        }
        for (i=0; i<listlen; ++i)
        {
            if (Tcl_GetIntFromObj(interp, listObjElem[i], &id) != TCL_OK)
            {
                multires_cleanup;
                return TCL_ERROR;
            }
            if (hs_type(id) != HS_1D_HISTOGRAM)
            {
                multires_cleanup;
                Tcl_SetResult(interp, "bad convolution "
                              "histogram id", TCL_STATIC);
                return TCL_ERROR;                
            }
            old_smoothing_curves[i] = id;
        }
        n_oldcurves = listlen;
        n_oldbins = hs_1d_hist_num_bins(id_fft);
        binwidth = (double)hs_hist_bin_width(id_fft);
        for (m = 0, i = n_oldbins; i > 1; i /= 2, ++m)
            if (i % 2)
            {
                multires_cleanup;
                Tcl_AppendResult(interp, "Number of bins in the histogram with id ",
                                 Tcl_GetStringFromObj(objv[1], NULL),
                                 " is not a power of 2", NULL);
                return TCL_ERROR;
            }
        for (i=0; i<listlen; ++i)
            if (hs_1d_hist_num_bins(old_smoothing_curves[i]) != n_oldbins)
            {
                multires_cleanup;
                Tcl_SetResult(interp, "convolution histograms "
                              "have incompatible binning", TCL_STATIC);
                return TCL_ERROR;
            }
        mem = malloc(n_oldbins*n_oldcurves*sizeof(float) +
                     n_oldbins*sizeof(float) +
                     (n_oldbins+2)*sizeof(float) +
                     n_oldcurves*sizeof(float *));
        if (mem == NULL)
        {
            multires_cleanup;
            Tcl_SetResult(interp, "out of memory", TCL_STATIC);
            return TCL_ERROR;
        }
    }
    fft  = (float *)mem + n_oldbins*n_oldcurves;
    work = fft + n_oldbins;
    data = (float **)(work + n_oldbins + 2);
    if (need_cleanup)
        for (i=0; i<n_oldcurves; ++i)
        {
            data[i] = (float *)mem + n_oldbins*i;
            hs_1d_hist_bin_contents(old_smoothing_curves[i], data[i]);
        }
    hs_1d_hist_bin_contents(id_fft, fft);

    /* At this point we have extracted all the data */
    result = Tcl_NewListObj(0, NULL);
    data1 = fft;
    for (icurve=0; icurve<n_oldcurves; ++icurve)
    {
        data2 = data[icurve];
        /* Fourier multiplication */
        {
            float halfbins;
            int j, imax;
            halfbins = (float)(n_oldbins/2);
            ++work;
            work[0] = data1[0]*data2[0]*halfbins;
            imax = n_oldbins/2-1;
            for (i=0, j=1; i<imax; ++i, j+=2)
            {
                work[j] = (data1[j]*data2[j] - data1[j+1]*data2[j+1])*halfbins;
                work[j+1] = (data1[j]*data2[j+1] + data2[j]*data1[j+1])*halfbins;
            }
            work[n_oldbins-1] = data1[n_oldbins-1]*data2[n_oldbins-1]*(float)n_oldbins;
        }

        /* Inverse FFT */
        --work;
        work[0] = work[1] / 2.f;
        work[1] = 0.f;
        work[n_oldbins+1] = 0.f;
        for (i=2; i<n_oldbins; )
        {
            work[i++] /= 2.f;
            work[i++] /= -2.f;
        }
        i = m;
        rfft_(work, &i);

        /* Get the norm */
        sum = 0.0;
        for (i=0; i<n_oldbins; ++i)
            sum += fabs((double)(work[i]));
        sum *= binwidth;
        i = Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(sum));
        assert(i == TCL_OK);
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

tcl_routine(Analyze_multires_ntuple)
{
    int id, row, col, nrows, ncols, index, maxind, maxcol;
    void *mem;
    float *data, *transpose;
    int *maxindices;
    double count;
    Tcl_Obj *result[2];

    tcl_require_objc(2);
    verify_ntuple(id,1);
    nrows = hs_num_entries(id);
    ncols = hs_num_variables(id);
    if (nrows == 0)
    {
        Tcl_SetResult(interp, "the ntuple is empty", TCL_STATIC);
        return TCL_ERROR;
    }
    if (nrows == 1)
    {
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(0.0));
        return TCL_OK;
    }

    /* Allocate and distribute the memory */
    mem = malloc(2*nrows*ncols*sizeof(float) + 
                 nrows*sizeof(int));
    checknull(mem);
    data = (float *)mem;
    transpose = data + nrows*ncols;
    maxindices = (int *)(transpose + nrows*ncols);

    hs_ntuple_contents(id, data);
    for (row=0; row<nrows; ++row)
        for (col=0; col<ncols; ++col)
            transpose[col*nrows+row] = data[row*ncols+col];
    for (col=0; col<ncols; ++col)
        qsort(transpose + col*nrows, nrows, sizeof(float),
              (int (*)(const void *, const void *))fcompare);

    /* Maximum index for the first row */
    row = 0;
    maxind = -1;
    maxcol = -1;
    for (col=0; col<ncols; ++col)
    {
        index = hs_find_value_index_in_sorted_array(
            transpose + col*nrows, nrows, data[row*ncols+col]);
        if (index > maxind)
        {
            maxind = index;
            maxcol = col;
        }
    }
    maxindices[row] = maxind;

    /* Maximum index for all other rows */
    for (row=1; row<nrows; ++row)
    {
        maxind = 0;
        for (col=0; col<ncols; ++col)
        {
            index = hs_find_value_index_in_sorted_array(
                transpose + col*nrows, nrows, data[row*ncols+col]);
            if (index > maxind)
                maxind = index;
        }
        maxindices[row] = maxind;
    }

    /* Find out how many points have larger maximum index */
    count = 0.0;
    index = maxindices[0];
    for (row=1; row<nrows; ++row)
    {
        if (maxindices[row] > index)
            count += 1.0;
        else if (maxindices[row] == index)
            count += 0.5;
    }

    free(mem);
    result[0] = Tcl_NewDoubleObj(count/(double)(nrows-1));
    result[1] = Tcl_NewIntObj(maxcol);
    Tcl_SetObjResult(interp, Tcl_NewListObj(2, result));
    return TCL_OK;
}

static int generate_fitter_cdf(
    Fit_config *fit, int set, Tcl_Interp *interp, double xmn,
    double xmx, int nbins, struct simpson_bin *cdfdata)
{
    Fit_subset *fitset;
    Minuit_fitter_info *fitter;
    double y = 0.0, z = 0.0, fitsum = 0.0, xleft, xright, xcenter;
    double dpoints, dstep, eps, drange, dfval, normd1, normd2;
    double fleft, fright = 0.0, fcenter;
    int i, pointerror, ifun, fmode;
    int edge_defined = 0, ierr = 0, use_this_bin = 1;
    char stringbuf[32];

    assert(fit);
    assert(set >= 0 && set < fit->nsets);
    fitset = fit->fitsets[set]; 
    assert(fitset);

    dpoints = (double)nbins;
    drange = xmx - xmn;
    dstep = drange/dpoints;
    eps   = dstep*BIN_EDGE_EPSILON;
    for (i=0; i<nbins; ++i)
    {
        xleft = xmn + ((double)i/dpoints)*drange;
        xcenter = xleft + dstep/2.0;
        if (i == nbins - 1)
            xright = xmx;
        else
            xright = xleft + dstep;
        if (fitset->filter)
            use_this_bin = ((fitset->filter)(xleft + eps, y, z) &&
                            (fitset->filter)(xcenter, y, z) &&
                            (fitset->filter)(xright - eps, y, z));
        if (use_this_bin)
        {
            if (edge_defined)
                fleft = fright;
            else
            {
                calculate_sum_of_functions(xleft,y,z);
                check_non_negative("fit",xleft,dfval);
                fleft = dfval;
            }
            calculate_sum_of_functions(xcenter,y,z);
            check_non_negative("fit",xcenter,dfval);
            fcenter = dfval;
            calculate_sum_of_functions(xright,y,z);
            check_non_negative("fit",xright,dfval);
            fright = dfval;
            edge_defined = 1;
            fitsum += ((fleft + fright + 4.0*fcenter)/6.0);
            cdfdata[i].d1 = fright - fleft;
            cdfdata[i].d2 = fright - 2.0*fcenter + fleft;
        }
        else
        {
            cdfdata[i].d1 = 0.0;
            cdfdata[i].d2 = 0.0;
            edge_defined = 0;
        }
        cdfdata[i].integ = fitsum;
    }
    if (fitsum == 0.0)
    {
        Tcl_SetResult(interp, "failed to generate cumulative density: "
                      "all fit values are 0", TCL_STATIC);
        goto fail1;
    }
    normd1 = fitsum*dstep*dstep;
    normd2 = normd1*dstep/4.0;
    for (i=0; i<nbins; ++i)
    {
        cdfdata[i].integ /= fitsum;
        cdfdata[i].d1    /= normd1;
        cdfdata[i].d2    /= normd2;
    }
    return TCL_OK;

    /* Don't remove the label below, it is used in
       the "calculate_sum_of_functions" macro */
 fail1:
    return TCL_ERROR;
}
