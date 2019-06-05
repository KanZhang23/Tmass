#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <limits.h>
#include <math.h>
#include "histoscope.h"
#include "histo_utils.h"
#include "fit_function.h"
#include "cernlib.h"

#define MAX_FUN_PARAMETERS 300

#ifndef USE_CERNLIB
#define USE_CERNLIB
#endif

#define checkfree(pointer) do {if (pointer) {free(pointer); pointer=0;}} while(0);
#define checknull(pointer) do {if (pointer == 0) {\
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);\
    return TCL_ERROR;\
}} while(0);

#define tcl_require_objc_option(N) do {\
  if (objc != N)\
  {\
    Tcl_AppendResult(interp, "wrong # of arguments (fit function ",\
		     Tcl_GetStringFromObj(objv[0],NULL), ", option ",\
                     Tcl_GetStringFromObj(objv[1],NULL), ")", NULL);\
    return TCL_ERROR;\
  }\
} while(0);

#define tcl_require_objc_sharedlib(N) do {\
  if (objc != N)\
  {\
    Tcl_AppendResult(interp, "wrong # of arguments for option \"",\
		     Tcl_GetStringFromObj(objv[0],NULL), "\"", NULL);\
    return TCL_ERROR;\
  }\
} while(0);

typedef struct
{
    void *handle;
    char *filename;
    int refcount;
} DLLibrary;

typedef struct
{
    double *var;
    double dmin;
    double dmax;
    int npoints;
} Scan_info;

typedef enum
{
    FITFUN_NONE = 0,
    FITFUN_SUM,
    FITFUN_PRODUCT,
    FITFUN_RATIO,
    FITFUN_COMPOSITION,
    N_FITFUN_COMBO_TYPES
} CombinationType;

static const char * CombinationTypeNames[N_FITFUN_COMBO_TYPES] = {
    "empty combination",
    "sum",
    "product",
    "ratio",
    "composition"
};

typedef struct
{
    CombinationType type;
    double c1;
    char *name1;
    int npars1;
    Minuit_fitter_info *f1;
    double c2;
    char *name2;
    int npars2;
    Minuit_fitter_info *f2;
} FunctionCombination;

static DLLibrary *libtable = 0;
static int libtable_len = 0;

static Tcl_HashTable fitter_table;
static int n_fitters = 0;

static FunctionCombination* combo_table = 0;
static int n_combos = 0;

static int auto_name_counter = 0;

static int combo_compatible(CombinationType type,
			    Minuit_fitter_info *f1,
			    Minuit_fitter_info *f2);

/* Function to close dll by number. Returns 0 on success. */
int hs_close_fitter_dll_bynumber(int n);

static Minuit_aux_function hs_combo_init;
static Minuit_c_fit_function hs_combo_fitfun;
static Minuit_aux_function hs_combo_cleanup;

/* Local functions for which we will not create tcl commands */
tcl_routine(dlopen);
tcl_routine(dlclose);
tcl_routine(dlliblist);
tcl_routine(dllibfile);
tcl_routine(fitfun_info);
tcl_routine(fitfun_configure);
tcl_routine(fitfun_cget);
tcl_routine(fitfun_copy);
tcl_routine(fitfun_scan);
tcl_routine(fitfun_eval);
tcl_routine(fitfun_combine);
tcl_routine(fitfun_integrate);
tcl_routine(fitfun_rename);

tcl_routine(function_sum)
{
    long type = FITFUN_SUM;
    return tcl_c_name(fitfun_combine) ((ClientData)type, interp, objc, objv);
}

tcl_routine(function_divide)
{
    long type = FITFUN_RATIO;
    return tcl_c_name(fitfun_combine) ((ClientData)type, interp, objc, objv);
}

tcl_routine(function_multiply)
{
    long type = FITFUN_PRODUCT;
    return tcl_c_name(fitfun_combine) ((ClientData)type, interp, objc, objv);
}

tcl_routine(function_compose)
{
    long type = FITFUN_COMPOSITION;
    return tcl_c_name(fitfun_combine) ((ClientData)type, interp, objc, objv);
}

tcl_routine(Function_owns_dll)
{
    char *fname;
    int yesno;
    Minuit_fitter_info *fitter = NULL;

    tcl_require_objc(3);
    if (Tcl_GetBooleanFromObj(interp, objv[2], &yesno) != TCL_OK)
	return TCL_ERROR;
    fname = Tcl_GetStringFromObj(objv[1], NULL);
    fitter = hs_find_fitter_function(fname);
    if (fitter == NULL)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 fname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    if (fitter->ownsdll != yesno)
    {
	fitter->ownsdll = yesno;
	if (yesno)
	    hs_incr_dll_refcount(fitter->groupnumber);
	else
	    hs_decr_dll_refcount(fitter->groupnumber);
    }
    return TCL_OK;
}

tcl_routine(function)
{
    int exists;
    char *command, *fname;
    Minuit_fitter_info *fitter = NULL;

    if (objc < 3)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    command = Tcl_GetStringFromObj(objv[2], NULL);
    fname = Tcl_GetStringFromObj(objv[1], NULL);
    fitter = hs_find_fitter_function(fname);
    exists = (fitter != NULL);
    if (strcmp(command, "exists") == 0)
    {
	tcl_require_objc(3);
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(exists));
	return TCL_OK;
    }
    if (!exists)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 fname, "\" does not exist", NULL);
	return TCL_ERROR;
    }

    /* Put "eval" check first for faster evaluation ... */
    if (strcmp(command, "eval") == 0 ||
        strcmp(command, "listeval") == 0)
    {
	return tcl_c_name(fitfun_eval) ((ClientData)fitter,
					interp, objc-1, objv+1);
    }
    else if (strcmp(command, "scan") == 0)
    {
	return tcl_c_name(fitfun_scan) ((ClientData)fitter,
					interp, objc-1, objv+1);
    }
    else if (strcmp(command, "integrate") == 0 ||
             strcmp(command, "cdf") == 0 ||
             strcmp(command, "invcdf_lin") == 0 ||
#ifdef USE_CERNLIB
	     strcmp(command, "random") == 0 ||
	     strcmp(command, "invcdf") == 0 ||
#endif
	     strcmp(command, "moments") == 0)
    {
	return tcl_c_name(fitfun_integrate) ((ClientData)fitter,
					     interp, objc-1, objv+1);
    }
    else if (strcmp(command, "copy") == 0)
    {
	return tcl_c_name(fitfun_copy) ((ClientData)fitter,
					interp, objc-1, objv+1);
    }
    else if (strcmp(command, "del") == 0 ||
	     strcmp(command, "delete") == 0)
    {
	return hs_rename_fitter_function(interp, "", fname);
    }
    else if (strcmp(command, "rename") == 0)
    {
	return tcl_c_name(fitfun_rename) ((ClientData)fitter,
					  interp, objc-1, objv+1);
    }
    else if (strcmp(command, "configure") == 0 ||
	     strcmp(command, "config") == 0)
    {
	return tcl_c_name(fitfun_configure) ((ClientData)fitter,
					     interp, objc-1, objv+1);
    }
    else if (strcmp(command, "cget") == 0)
    {
	return tcl_c_name(fitfun_cget) ((ClientData)fitter,
					interp, objc-1, objv+1);
    }
    else if (strcmp(command, "info") == 0)
    {
	return tcl_c_name(fitfun_info) ((ClientData)fitter,
					interp, objc-1, objv+1);
    }
    else
    {
	Tcl_AppendResult(interp, "Invalid option \"", command, 
			 "\" for function ", fname, ". Valid options are: ",
			 "cdf, ",
			 "cget, ",
			 "configure, ",
			 "copy, ",
			 "delete, ",
			 "eval, ",
			 "exists, ",
			 "info, ",
			 "integrate, ",
#ifdef USE_CERNLIB
                         "invcdf, ",
#endif
			 "invcdf_lin, ",
			 "moments, ",
#ifdef USE_CERNLIB
                         "random, ",
#endif
			 "rename, ",
			 "and scan.", NULL);
	return TCL_ERROR;
    }    
}

tcl_routine(function_import)
{
    /* Usage: function_import name dllnumber mathname description ndim default_mode\
       npars_min npars_max parlist init_fun fit_fun grad_fun cleanup_fun */
    Minuit_fitter_info *value = 0;
    char *name;
    int i, nelem, isfortran, npars_defined, argcnt = 1;
    Tcl_Obj **listObjElem;
    void *handle;
    char stringbuf[32];
    char **parlist;

    value = (Minuit_fitter_info *)calloc(1, sizeof(Minuit_fitter_info));
    checknull(value);

    /* Parse the arguments */
    tcl_require_objc_option(14);
    name = Tcl_GetStringFromObj(objv[argcnt++], NULL);
    if (Tcl_GetIntFromObj(interp, objv[argcnt++], &value->groupnumber) != TCL_OK)
	goto fail0;
    value->fullname = strdup(Tcl_GetStringFromObj(objv[argcnt++], NULL));
    if (value->fullname == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    value->description = strdup(Tcl_GetStringFromObj(objv[argcnt++], NULL));
    if (value->description == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    if (Tcl_GetIntFromObj(interp, objv[argcnt++], &value->ndim) != TCL_OK)
	goto fail0;
    if (Tcl_GetIntFromObj(interp, objv[argcnt++], &value->mode) != TCL_OK)
	goto fail0;
    if (Tcl_GetIntFromObj(interp, objv[argcnt++], &value->npars_min) != TCL_OK)
	goto fail0;
    if (Tcl_GetIntFromObj(interp, objv[argcnt++], &value->npars_max) != TCL_OK)
	goto fail0;
    if (value->npars_min < 0 || value->npars_min > value->npars_max)
    {
	Tcl_SetResult(interp, "wrong number of parameters", TCL_STATIC);
	goto fail0;
    }
    if (value->npars_max > MAX_FUN_PARAMETERS)
    {
	sprintf(stringbuf, "%d", MAX_FUN_PARAMETERS);
	Tcl_AppendResult(interp, "max number of parameters is too large, should be ",
			 stringbuf, " or less", NULL);
	goto fail0;
    }
    if (Tcl_ListObjGetElements(interp, objv[argcnt++], &nelem, &listObjElem) != TCL_OK)
	goto fail0;
    if (value->npars_max > 0)
    {
	parlist = (char **)calloc(value->npars_max, sizeof(char *));
	if (parlist == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    goto fail0;
	}
	value->param_names = parlist;
	if (nelem < value->npars_max)
	    npars_defined = nelem;
	else
	    npars_defined = value->npars_max;
	for (i=0; i<npars_defined; ++i)
	{
	    parlist[i] = strdup(Tcl_GetStringFromObj(listObjElem[i], NULL));
	    if (parlist[i] == NULL)
	    {
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    else if (strcmp(parlist[i], "x") == 0 ||
		     strcmp(parlist[i], "y") == 0 ||
		     strcmp(parlist[i], "z") == 0)
	    {
		Tcl_AppendResult(interp, "bad parameter name \"", parlist[i], "\"", NULL);
		goto fail0;
	    }
	    else if (!hs_is_valid_c_name(interp, parlist[i], "parameter name"))
		goto fail0;
	}
	for (i=npars_defined; i<value->npars_max; ++i)
	{
	    sprintf(stringbuf, "P%d", i);
	    parlist[i] = strdup(stringbuf);
	    if (parlist[i] == NULL)
	    {
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }	    
	}
	i = find_duplicate_name(parlist, value->npars_max);
	if (i >= 0)
	{
	    Tcl_AppendResult(interp, "duplicate parameter name \"",
			     parlist[i], "\"", NULL);
	    goto fail0;
	}
    }
    value->init_fun_name = strdup(Tcl_GetStringFromObj(objv[argcnt++], NULL));
    value->fit_fun_name = strdup(Tcl_GetStringFromObj(objv[argcnt++], NULL));
    value->grad_fun_name = strdup(Tcl_GetStringFromObj(objv[argcnt++], NULL));
    value->cleanup_fun_name = strdup(Tcl_GetStringFromObj(objv[argcnt++], NULL));
    if (value->init_fun_name == NULL || value->fit_fun_name == NULL ||
	value->grad_fun_name == NULL || value->cleanup_fun_name == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    assert(argcnt == 14);

    /* Check that the library and function arguments make sense */
    if (value->groupnumber < 0 || value->groupnumber >= libtable_len)
    {
	Tcl_SetResult(interp, "dll number is out of range", TCL_STATIC);
	goto fail0;
    }
    if (libtable[value->groupnumber].filename == NULL)
    {
	Tcl_SetResult(interp, "invalid dll number", TCL_STATIC);
	goto fail0;
    }

    if (value->init_fun_name[0])
    {
	value->init = (Minuit_aux_function *)hs_find_library_function(
	    interp, value->groupnumber, value->init_fun_name, &isfortran);
	if (isfortran < 0)
	    goto fail0;
    }

    if (value->fit_fun_name[0])
    {
	handle = hs_find_library_function(
	    interp, value->groupnumber, value->fit_fun_name, &isfortran);
	if (isfortran < 0)
	    goto fail0;
	if (isfortran)
	    value->fit_f = (Minuit_f_fit_function *)handle;
	else
	    value->fit_c = (Minuit_c_fit_function *)handle;
    }
    else
    {
	Tcl_SetResult(interp,
		      "fit function name can not be an empty string",
		      TCL_STATIC);
	goto fail0;
    }

    if (value->grad_fun_name[0])
    {
	handle = hs_find_library_function(
	    interp, value->groupnumber, value->grad_fun_name, &isfortran);
	if (isfortran < 0)
	    goto fail0;
	if (isfortran)
	    value->grad_f = (Minuit_f_grad_function *)handle;
	else
	    value->grad_c = (Minuit_c_grad_function *)handle;
    }

    if (value->cleanup_fun_name[0])
    {
	value->cleanup = (Minuit_aux_function *)hs_find_library_function(
	    interp, value->groupnumber, value->cleanup_fun_name, &isfortran);
	if (isfortran < 0)
	    goto fail0;
    }

    /* Check that the mode and the number of parameters are consistent */
    i = hs_fitter_num_pars(value);
    if (i < value->npars_min || i > value->npars_max)
    {
	Tcl_SetResult(interp,
		      "mode is inconsistent with bounds on the number of parameters",
		      TCL_STATIC);
	goto fail0;
    }

    /* Add the function */
    if (hs_add_fitter_function(interp, name, value) != TCL_OK)
	goto fail0;

    Tcl_SetObjResult(interp, Tcl_NewStringObj(name, -1));
    return TCL_OK;

 fail0:
    hs_destroy_fitter_info(value);
    return TCL_ERROR;
}

tcl_routine(fitfun_configure)
{
    char *name, *option;
    char *fullname = NULL, *description = NULL;
    char **parnames = NULL;
    Minuit_fitter_info *fun;
    int i, listobjc, iname, ival, pair, npairs, mode, oldmode, npars_defined;
    Tcl_Obj **parlist = NULL;
    Tcl_Obj *listPtr, *optList;
    char stringbuf[32];

    if (objc % 2)
    {
	Tcl_AppendResult(interp, "wrong # of arguments (fit function ",
			 Tcl_GetStringFromObj(objv[0],NULL), ", option ",
			 Tcl_GetStringFromObj(objv[1],NULL), ")", NULL);
	return TCL_ERROR;
    }
    name = Tcl_GetStringFromObj(objv[0], NULL);
    fun = (Minuit_fitter_info *)clientData;

    if (objc == 2)
    {
	/* Print out all configurable options */
	listPtr = Tcl_NewListObj(0, NULL);

	optList = Tcl_NewListObj(0, NULL);
	Tcl_ListObjAppendElement(interp, optList, Tcl_NewStringObj("name", -1));
	Tcl_ListObjAppendElement(interp, optList, Tcl_NewStringObj(fun->fullname, -1));
	Tcl_ListObjAppendElement(interp, listPtr, optList);
	
	optList = Tcl_NewListObj(0, NULL);
	Tcl_ListObjAppendElement(interp, optList, Tcl_NewStringObj("description", -1));
	Tcl_ListObjAppendElement(interp, optList, Tcl_NewStringObj(fun->description, -1));
	Tcl_ListObjAppendElement(interp, listPtr, optList);

	/* Function mode */
	if (fun->refcount == 0 && !fun->is_combined)
	{
	    optList = Tcl_NewListObj(0, NULL);
	    Tcl_ListObjAppendElement(interp, optList, Tcl_NewStringObj("mode", -1));
	    Tcl_ListObjAppendElement(interp, optList, Tcl_NewIntObj(fun->mode));
	    Tcl_ListObjAppendElement(interp, listPtr, optList);
	}

	/* Function parameters */
	if (fun->refcount == 0)
	{
	    parlist = (Tcl_Obj **)malloc(fun->npars_max * sizeof(Tcl_Obj *));
	    checknull(parlist);
	    for (i=0; i<fun->npars_max; ++i)
		parlist[i] = Tcl_NewStringObj(fun->param_names[i], -1);
	    optList = Tcl_NewListObj(0, NULL);
	    Tcl_ListObjAppendElement(interp, optList, Tcl_NewStringObj("parameters", -1));
	    Tcl_ListObjAppendElement(interp, optList, Tcl_NewListObj(fun->npars_max, parlist));
	    Tcl_ListObjAppendElement(interp, listPtr, optList);
	}

	checkfree(parlist);
	Tcl_SetObjResult(interp, listPtr);
    }
    else
    {
	mode = fun->mode;
	npairs = (objc - 2) / 2;
	for (pair = 0; pair < npairs; ++pair)
	{
	    iname = pair*2 + 2;
	    ival = iname + 1;
	    option = Tcl_GetStringFromObj(objv[iname], NULL);
	    if (option[0] == '-') ++option;
	    if (strcmp(option, "name") == 0)
	    {
		fullname = Tcl_GetStringFromObj(objv[ival], NULL);
	    }
	    else if (strcmp(option, "description") == 0)
	    {
		description = Tcl_GetStringFromObj(objv[ival], NULL);
	    }
	    else if (strcmp(option, "mode") == 0)
	    {
		if (Tcl_GetIntFromObj(interp, objv[ival], &mode) != TCL_OK)
		    return TCL_ERROR;
		if (fun->refcount)
		{
		    Tcl_AppendResult(interp, "can't configure mode for function \"",
				     name, "\": this function is in use", NULL);
		    return TCL_ERROR;
		}
		if (fun->is_combined)
		{
		    Tcl_AppendResult(interp, "can't configure mode for function \"",
				     name, "\": mode is not modifiable", NULL);

		    return TCL_ERROR;
		}
	    }
	    else if (strcmp(option, "parameters") == 0)
	    {
		if (Tcl_ListObjGetElements(interp, objv[ival], &listobjc, &parlist) != TCL_OK)
		    return TCL_ERROR;
		if (fun->refcount != 0)
		{
		    Tcl_AppendResult(interp, "can't configure parameters for function \"",
				     name, "\": this function is in use", NULL);
		    return TCL_ERROR;
		}
		if (fun->npars_max > 0)
		{
		    parnames = (char **)calloc(fun->npars_max, sizeof(char *));
		    checknull(parnames);
		    if (listobjc > fun->npars_max)
			npars_defined = fun->npars_max;
		    else
			npars_defined = listobjc;
		    for (i=0; i<npars_defined; ++i)
		    {   
			parnames[i] = strdup(Tcl_GetStringFromObj(parlist[i], NULL));
			if (parnames[i] == NULL)
			{
			    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
			    goto fail0;
			}
			else if (strcmp(parnames[i], "x") == 0 ||
				 strcmp(parnames[i], "y") == 0 ||
				 strcmp(parnames[i], "z") == 0)
			{
			    Tcl_AppendResult(interp, "bad parameter name \"", parnames[i], "\"", NULL);
			    goto fail0;
			}
			else if (!hs_is_valid_c_name(interp, parnames[i], "parameter name"))
			    goto fail0;
		    }
		    for (i=npars_defined; i<fun->npars_max; ++i)
		    {
			sprintf(stringbuf, "P%d", i);
			parnames[i] = strdup(stringbuf);
			if (parnames[i] == NULL)
			{
			    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
			    goto fail0;
			}
		    }
		    i = find_duplicate_name(parnames, fun->npars_max);
		    if (i >= 0)
		    {
			Tcl_AppendResult(interp, "duplicate parameter name \"",
					 parnames[i], "\"", NULL);
			goto fail0;
		    }
		}
	    }
	    else
	    {
		Tcl_AppendResult(interp, "invalid or read-only option \"", option, "\"", NULL);
		return TCL_ERROR;
	    }
	}
	oldmode = fun->mode;
	fun->mode = mode;
	i = hs_fitter_num_pars(fun);
	if (i < fun->npars_min || i > fun->npars_max)
	{
	    sprintf(stringbuf, "%d", mode);
	    Tcl_AppendResult(interp, "can't set mode to ", stringbuf,
			     ": this brings the number of parameters ",
			     "outside allowed range", NULL);
	    fun->mode = oldmode;
	    goto fail0;
	}
	if (fullname)
	{
	    checkfree(fun->fullname);
	    fun->fullname = strdup(fullname);
	    checknull(fun->fullname);
	}
	if (description)
	{
	    checkfree(fun->description);
	    fun->description = strdup(description);
	    checknull(fun->description);
	}
	if (parnames)
	{
	    assert(fun->param_names);
	    for (i=0; i<fun->npars_max; ++i)
		checkfree(fun->param_names[i]);
	    free(fun->param_names);
	    fun->param_names = parnames;
	}
    }
    return TCL_OK;

 fail0:
    if (parnames)
    {
	for (i=0; i<fun->npars_max; ++i)
	    checkfree(parnames[i]);
	free(parnames);
    }
    return TCL_ERROR;
}

tcl_routine(fitfun_combine)
{
    /* Usage: fit_function_sum [-result newname] c1 f1 c2 f2
     *        fit_function_multiply [-result newname] c1 f1 f2
     *        fit_function_divide [-result newname] c1 f1 f2
     *        fit_function_compose [-result newname] f1 c1 f2 c2
     */
    int i, mode;
    CombinationType type;
    char *f1 = NULL, *f2 = NULL;
    double c1 = 0.0, c2 = 0.0;
    Minuit_fitter_info *value = NULL, *pf1 = NULL, *pf2 = NULL;
    FunctionCombination *new_table;
    char *stringbuf = NULL;
    char c1_str[TCL_DOUBLE_SPACE+1], c2_str[TCL_DOUBLE_SPACE+1];
    char *result = NULL;

    if (objc < 4)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check if the name of the resulting function is specified */
    if (strcmp("-result", Tcl_GetStringFromObj(objv[1], NULL)) == 0)
    {
	result = Tcl_GetStringFromObj(objv[2], NULL);
	if (hs_find_fitter_function(result))
	{
	    Tcl_AppendResult(interp, "fit function with tag \"",
			     result, "\" already exists", NULL);
	    return TCL_ERROR;
	}
	objc -= 2;
	objv += 2;
    }
    
    type = (CombinationType)clientData;
    switch (type)
    {
    case FITFUN_SUM:
	tcl_require_objc(5);
	if (Tcl_GetDoubleFromObj(interp, objv[1], &c1) != TCL_OK)
	    return TCL_ERROR;
	f1 = Tcl_GetStringFromObj(objv[2], NULL);
	if (Tcl_GetDoubleFromObj(interp, objv[3], &c2) != TCL_OK)
	    return TCL_ERROR;
	f2 = Tcl_GetStringFromObj(objv[4], NULL);
	break;

    case FITFUN_PRODUCT:
	tcl_require_objc(4);
	if (Tcl_GetDoubleFromObj(interp, objv[1], &c1) != TCL_OK)
	    return TCL_ERROR;
	f1 = Tcl_GetStringFromObj(objv[2], NULL);
	f2 = Tcl_GetStringFromObj(objv[3], NULL);
	break;

    case FITFUN_RATIO:
	tcl_require_objc(4);
	if (Tcl_GetDoubleFromObj(interp, objv[1], &c1) != TCL_OK)
	    return TCL_ERROR;
	f1 = Tcl_GetStringFromObj(objv[2], NULL);
	f2 = Tcl_GetStringFromObj(objv[3], NULL);
	break;

    case FITFUN_COMPOSITION:
	tcl_require_objc(5);
	f1 = Tcl_GetStringFromObj(objv[1], NULL);
	if (Tcl_GetDoubleFromObj(interp, objv[2], &c1) != TCL_OK)
	    return TCL_ERROR;
	f2 = Tcl_GetStringFromObj(objv[3], NULL);
	if (Tcl_GetDoubleFromObj(interp, objv[4], &c2) != TCL_OK)
	    return TCL_ERROR;
	break;

    default:
	assert(0);
    }


    pf1 = hs_find_fitter_function(f1);
    if (pf1 == NULL)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 f1, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    pf2 = hs_find_fitter_function(f2);
    if (pf2 == NULL)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 f2, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    if (!combo_compatible(type, pf1, pf2))
    {
	Tcl_AppendResult(interp, "Can't define ",
			 CombinationTypeNames[type], " of functions \"",
			 f1, "\" and \"", f2,
			 "\": the functions are incompatible", NULL);
	return TCL_ERROR;
    }

    /* Make sure there is a place in the combo table */
    for (mode=0; mode<n_combos; ++mode)
	if (combo_table[mode].type == FITFUN_NONE)
	    break;
    if (mode == n_combos)
    {
	/* Have to increase the table size */
	new_table = (FunctionCombination *)realloc(
	    combo_table, (n_combos+1)*sizeof(FunctionCombination));
	checknull(new_table);
	combo_table = new_table;
	++n_combos;
	memset(combo_table + mode, 0, sizeof(FunctionCombination));
    }

    /* Clean up after the old entry */
    checkfree(combo_table[mode].name1);
    checkfree(combo_table[mode].name2);

    /* Fill the entry in the combo table */
    combo_table[mode].type = type;
    combo_table[mode].c1 = c1;
    combo_table[mode].name1 = strdup(f1);
    combo_table[mode].npars1 = hs_fitter_num_pars(pf1);
    combo_table[mode].f1 = pf1;
    combo_table[mode].c2 = c2;
    combo_table[mode].name2 = strdup(f2);
    combo_table[mode].npars2 = hs_fitter_num_pars(pf2);
    combo_table[mode].f2 = pf2;
    if (combo_table[mode].name1 == NULL || combo_table[mode].name2 == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }

    /* Fill the Minuit_fitter_info struct for the new fitter */
    stringbuf = (char *)malloc(strlen(f1) + strlen(f2) + 128);
    value = (Minuit_fitter_info *)calloc(1, sizeof(Minuit_fitter_info));
    if (value == NULL || stringbuf == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    value->groupnumber = -1;
    sprintf(stringbuf, "Function %s", CombinationTypeNames[type]);
    value->fullname = strdup(stringbuf);
    Tcl_PrintDouble(interp, c1, c1_str);
    Tcl_PrintDouble(interp, c2, c2_str);
    switch (type)
    {
    case FITFUN_SUM:
	sprintf(stringbuf, "%s * %s(...) + %s * %s(...)", c1_str, f1, c2_str, f2);
	break;
    case FITFUN_PRODUCT:
	sprintf(stringbuf, "%s * %s(...) * %s(...)", c1_str, f1, f2);
	break;
    case FITFUN_RATIO:
	sprintf(stringbuf, "%s * %s(...) / %s(...)", c1_str, f1, f2);
	break;
    case FITFUN_COMPOSITION:
	sprintf(stringbuf, "%s(%s * %s(...) + %s, ...)", f1, c1_str, f2, c2_str);
	break;
    default:
	assert(0);
    }
    value->description = strdup(stringbuf);
    if (value->fullname == NULL || value->description == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    value->ndim = pf2->ndim > pf1->ndim ? pf2->ndim : pf1->ndim;
    value->is_combined = 1;
    value->mode = mode;
    value->refcount = 0;
    value->npars_min = combo_table[mode].npars1 + combo_table[mode].npars2;
    value->npars_max = value->npars_min;
    if (value->npars_max > 0)
    {
	value->param_names = (char **)calloc(value->npars_max, sizeof(char *));
	if (value->param_names == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    goto fail0;
	}
	for (i=0; i<value->npars_max; ++i)
	{
	    sprintf(stringbuf, "P%d", i);
	    value->param_names[i] = strdup(stringbuf);
	    if (value->param_names[i] == NULL)
	    {
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	}
    }
    value->init = hs_combo_init;
    value->init_fun_name = strdup("hs_combo_init");
    value->fit_c = hs_combo_fitfun;
    value->fit_fun_name = strdup("hs_combo_fitfun");
    value->grad_c = NULL;
    value->grad_fun_name = strdup("");
    value->cleanup = hs_combo_cleanup;
    value->cleanup_fun_name = strdup("hs_combo_cleanup");
    if (value->init_fun_name == NULL || value->fit_fun_name == NULL ||
	value->grad_fun_name == NULL || value->cleanup_fun_name == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }

    if (result == NULL)
    {
	/* Come up with the name for the combo function */
	do {
	    sprintf(stringbuf, "hs_fit_function_%d", auto_name_counter++);
	} while(hs_find_fitter_function(stringbuf));
	result = stringbuf;
    }
    if (hs_add_fitter_function(interp, result, value) != TCL_OK)
	goto fail0;
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result, -1));
    free(stringbuf);
    return TCL_OK;

 fail0:
    combo_table[mode].type = FITFUN_NONE;
    hs_destroy_fitter_info(value);
    checkfree(stringbuf);
    return TCL_ERROR;
}

tcl_routine(fitfun_cget)
{
/*      char *name; */
    char *option;
    Minuit_fitter_info *fun;
    Tcl_Obj **parlist;
    Tcl_Obj *listPtr;
    int i, n_options, npars_used;
    char *known_options[] = {
	"dll",
	"name",
	"description",
	"ndim",
	"composite",
	"mode",
	"refcount",
	"npars",
	"npars_min",
	"npars_max",
	"parameters",
	"usedpars",
	"functions"
    };

    tcl_require_objc_option(3);
/*      name = Tcl_GetStringFromObj(objv[0], NULL); */
    fun = (Minuit_fitter_info *)clientData;

    option = Tcl_GetStringFromObj(objv[2], NULL);
    if (option[0] == '-') ++option;
    n_options = sizeof(known_options)/sizeof(known_options[0]);
    for (i=0; i<n_options; ++i)
	if (strcmp(option, known_options[i]) == 0) break;
    if (i == n_options)
    {
	Tcl_AppendResult(interp, "Bad cget option \"", option,
			 "\". Valid options are: -", known_options[0], NULL);
	for (i=1; i<n_options; ++i)
	{
	    if (i == n_options - 1)
		Tcl_AppendResult(interp, ", and -", known_options[i], ".", NULL);
	    else
		Tcl_AppendResult(interp, ", -", known_options[i], NULL);
	}
	return TCL_ERROR;
    }
    if (strcmp(option, "dll") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fun->groupnumber));
    }
    else if (strcmp(option, "name") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(fun->fullname, -1));
    }
    else if (strcmp(option, "description") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(fun->description, -1));
    }
    else if (strcmp(option, "ndim") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fun->ndim));
    }
    else if (strcmp(option, "composite") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewBooleanObj(fun->is_combined));
    }
    else if (strcmp(option, "mode") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fun->mode));
    }
    else if (strcmp(option, "refcount") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fun->refcount));
    }
    else if (strcmp(option, "npars") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(hs_fitter_num_pars(fun)));
    }
    else if (strcmp(option, "npars_min") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fun->npars_min));
    }
    else if (strcmp(option, "npars_max") == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(fun->npars_max));
    }
    else if (strcmp(option, "parameters") == 0)
    {
	parlist = (Tcl_Obj **)malloc(fun->npars_max * sizeof(Tcl_Obj *));
	if (parlist == 0)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	for (i=0; i<fun->npars_max; ++i)
	    parlist[i] = Tcl_NewStringObj(fun->param_names[i], -1);
	Tcl_SetObjResult(interp, Tcl_NewListObj(fun->npars_max, parlist));
	free(parlist);
    }
    else if (strcmp(option, "usedpars") == 0)
    {
	npars_used = hs_fitter_num_pars(fun);
	parlist = (Tcl_Obj **)malloc(npars_used * sizeof(Tcl_Obj *));
	if (parlist == 0)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	for (i=0; i<npars_used; ++i)
	    parlist[i] = Tcl_NewStringObj(fun->param_names[i], -1);
	Tcl_SetObjResult(interp, Tcl_NewListObj(npars_used, parlist));
	free(parlist);
    }
    else if (strcmp(option, "functions") == 0)
    {
	listPtr = Tcl_NewListObj(0, NULL);
	Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->init_fun_name, -1));
	Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->fit_fun_name, -1));
	Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->grad_fun_name, -1));
	Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->cleanup_fun_name, -1));
	Tcl_SetObjResult(interp, listPtr);
    }
    else
    {
	/* We can get here only if the if-else sequence is incomplete */
	assert(0);
    }
    return TCL_OK;
}

tcl_routine(fitfun_info)
{
/*      char *name; */
    Minuit_fitter_info *fun;
    Tcl_Obj *listPtr;
    Tcl_Obj **parlist;
    int i;

    tcl_require_objc_option(2);
/*      name = Tcl_GetStringFromObj(objv[0], NULL); */
    fun = (Minuit_fitter_info *)clientData;

    parlist = (Tcl_Obj **)malloc(fun->npars_max * sizeof(Tcl_Obj *));
    checknull(parlist);
    for (i=0; i<fun->npars_max; ++i)
	parlist[i] = Tcl_NewStringObj(fun->param_names[i], -1);

    listPtr = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fun->groupnumber));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->fullname, -1));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->description, -1));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fun->ndim));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewBooleanObj(fun->is_combined));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fun->mode));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fun->refcount));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(hs_fitter_num_pars(fun)));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fun->npars_min));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fun->npars_max));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewListObj(fun->npars_max, parlist));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->init_fun_name, -1));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->fit_fun_name, -1));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->grad_fun_name, -1));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewStringObj(fun->cleanup_fun_name, -1));

    free(parlist);
    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}

tcl_routine(fitfun_rename)
{
    char *newname, *oldname;

    tcl_require_objc_option(3);
    oldname = Tcl_GetStringFromObj(objv[0], NULL);
    newname = Tcl_GetStringFromObj(objv[2], NULL);
    return hs_rename_fitter_function(interp, newname, oldname);
}

tcl_routine(fitfun_copy)
{
    char *newname, *oldname;
    char stringbuf[64];

    oldname = Tcl_GetStringFromObj(objv[0], NULL);
    if (objc > 3)
    {
	Tcl_AppendResult(interp, "wrong # of arguments (fit function ",
			 oldname, ", option ",
			 Tcl_GetStringFromObj(objv[1],NULL), ")", NULL);
	return TCL_ERROR;
    }
    if (objc == 3)
    {	
	newname = Tcl_GetStringFromObj(objv[2], NULL);
    }
    else
    {
	/* objc is 2 by construction */
	/* Come up with the name for the copied function */
	do {
	    sprintf(stringbuf, "hs_fit_function_%d", auto_name_counter++);
	} while(hs_find_fitter_function(stringbuf));
	newname = stringbuf;
    }
    if (hs_copy_fitter_function(interp, newname, oldname) != TCL_OK)
	return TCL_ERROR;
    Tcl_SetObjResult(interp, Tcl_NewStringObj(newname, -1));
    return TCL_OK;
}

tcl_routine(function_list)
{
    Tcl_HashEntry *entryPtr;
    Tcl_HashSearch hashIterator;
    Tcl_Obj **result;
    int count = 0;
    char *pattern = "*";

    tcl_objc_range(1,2);
    if (objc > 1)
        pattern = Tcl_GetStringFromObj(objv[1], NULL);
    if (n_fitters == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewListObj(0, NULL));
	return TCL_OK;
    }

    result = (Tcl_Obj **)malloc(n_fitters * sizeof(Tcl_Obj *));
    if (result == 0)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    
    entryPtr = Tcl_FirstHashEntry(&fitter_table, &hashIterator);
    while (entryPtr)
    {
        char *fname = Tcl_GetHashKey(&fitter_table, entryPtr);
        if (Tcl_StringMatch(fname, pattern))
            result[count++] = Tcl_NewStringObj(fname, -1);
	entryPtr = Tcl_NextHashEntry(&hashIterator);
    }

    Tcl_SetObjResult(interp, Tcl_NewListObj(count, result));
    free(result);
    return TCL_OK;
}

#define setup_pointer_by_name do {\
    ppointer = NULL;\
    if (strcmp(name, "x") == 0)\
    {\
	xdef = 1;\
	ppointer = &x;\
    }\
    if (ppointer == NULL && value->ndim > 1)\
	if (strcmp(name, "y") == 0)\
	{\
            ydef = 1;\
            ppointer = &y;\
	}\
    if (ppointer == NULL && value->ndim > 2)\
	if (strcmp(name, "z") == 0)\
	{\
	    zdef = 1;\
	    ppointer = &z;\
	}\
    if (ppointer == NULL)\
	for (k=0; k<npars; ++k)\
	    if (strcmp(name, value->param_names[k]) == 0)\
	    {\
		pardef[k] = 1;\
		ppointer = pars + k;\
		break;\
	    }\
    if (ppointer == NULL)\
    {\
	Tcl_AppendResult(interp,\
			 "invalid parameter or variable name \"",\
			 name, "\"", NULL);\
	goto fail0;\
    }\
} while(0);

#define report_funct_error(fname, value, ierr) do {\
    sprintf(stringbuf, "%d", ierr);\
    Tcl_AppendResult(interp, "failed to evaluate function ",\
		     fname, " (error code ", stringbuf, ") for\n", NULL);\
    sprintf(stringbuf, "%d", value->mode);\
    Tcl_AppendResult(interp, "mode = ", stringbuf, "\n", NULL);\
    sprintf(stringbuf, "%g", x);\
    Tcl_AppendResult(interp, "x = ", stringbuf, "\n", NULL);\
    if (value->ndim > 1)\
    {\
        sprintf(stringbuf, "%g", y);\
	Tcl_AppendResult(interp, "y = ", stringbuf, "\n", NULL);\
    }\
    if (value->ndim > 2)\
    {\
        sprintf(stringbuf, "%g", z);\
        Tcl_AppendResult(interp, "z = ", stringbuf, "\n", NULL);\
    }\
    for (k=0; k<npars; ++k)\
    {\
	sprintf(stringbuf, "%g", pars[k]);\
	Tcl_AppendResult(interp, value->param_names[k],\
			 " = ", stringbuf, "\n", NULL);\
    }\
} while(0);

#define get_function_value do {\
    if (value->fit_c)\
	dfval = (value->fit_c)(x, y, z, value->mode, pars, &ierr);\
    else\
    {\
	fmode = value->mode;\
	dfval = (value->fit_f)(&x, &y, &z, &fmode, pars, &ierr);\
    }\
    if (ierr)\
    {\
	report_funct_error(funct_name, value, ierr);\
	goto fail1;\
    }\
} while(0);

tcl_routine(fitfun_scan)
{
    /* Usage: funct_name scan result_specifier {name1 value1} ... {nameN valueN} */
    char *funct_name, *name;
    Minuit_fitter_info *value;
    int i, j, k, imax, jmax, kmax, id, npars, nelem, nspec, itype, argnum, fmode;
    int ntuple_dim = 0, scandim = 0, ierr = 0;
    double x = 0.0, y = 0.0, z = 0.0, dmin, dmax;
    int xdef = 0, ydef = 0, zdef = 0;
    double *pars = NULL;
    int *pardef = NULL;
    double *ppointer, *dxp, *dyp = NULL, *dzp = NULL;
    Tcl_Obj **listObjElem, **rangeSpec;
    Scan_info scan_info[3] = {{NULL, 0.0, 0.0, 0},
			      {NULL, 0.0, 0.0, 0},
			      {NULL, 0.0, 0.0, 0}};
    float xmin, xmax, ymin, ymax, zmin, zmax, xstep, ystep, zstep;
    int npoints, nxbins, nybins, nzbins;
    float *data = NULL, *pdata;
    float ntuple_data[4];
    double dxmin, dxrange, dxpoints, dymin, dyrange, dypoints, dfval;
    double dzmin, dzrange, dzpoints;
    char stringbuf[32];

    if (objc < 3)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Parse function name */
    funct_name = Tcl_GetStringFromObj(objv[0], NULL);
    value = (Minuit_fitter_info *)clientData;

    /* Array of parameter values */
    npars = value->npars_max;
    if (npars > 0)
    {
	pars = (double *)calloc(npars, sizeof(double));
	pardef = (int *)calloc(npars, sizeof(int));
	checknull(pars && pardef);
	/* It is OK if some parameters are undefined */
	for (i=hs_fitter_num_pars(value); i<npars; ++i)
	    pardef[i] = 1;
    }

    /* Parse the result specifier. The result specifier is a list whose
     * first element is the Histo-Scope id. This id may refer to
     * 1d histogram, 2d histogram, 3d histogram, or Ntuple.
     */
    if (Tcl_ListObjGetElements(interp, objv[2], &nelem, &listObjElem) != TCL_OK)
	goto fail0;
    if (nelem < 2)
    {
	Tcl_AppendResult(interp, "invalid result specifier \"",
			 Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
	goto fail0;
    }
    if (Tcl_GetIntFromObj(interp, listObjElem[0], &id) != TCL_OK)
	goto fail0;
    itype = hs_type(id);
    switch (itype)
    {
    case HS_1D_HISTOGRAM:
	scandim = 1;
	/* The result specifier looks like this: {id name} */
	if (nelem != 2)
	{
	    Tcl_AppendResult(interp, "invalid 1d histogram result specifier \"",
			     Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
	    goto fail0;
	}
	nxbins = hs_1d_hist_num_bins(id);
	hs_1d_hist_range(id, &xmin, &xmax);

	name = Tcl_GetStringFromObj(listObjElem[1], NULL);
	setup_pointer_by_name;
	scan_info[0].var = ppointer;
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
	scandim = 2;
	/* The result specifier looks like this: {id name1 name2} */
	if (nelem != 3)
	{
	    Tcl_AppendResult(interp, "invalid 2d histogram result specifier \"",
			     Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
	    goto fail0;
	}
	hs_2d_hist_num_bins(id, &nxbins, &nybins);
	hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);

	name = Tcl_GetStringFromObj(listObjElem[1], NULL);
	setup_pointer_by_name;
	scan_info[0].var = ppointer;
	scan_info[0].npoints = nxbins;
	xstep = (xmax - xmin)/(float)nxbins;
	scan_info[0].dmin = xmin + 0.5f * xstep;
	if (nxbins == 1)
	    scan_info[0].dmax = scan_info[0].dmin;
	else
	    scan_info[0].dmax = xmax - 0.5f * xstep;

	name = Tcl_GetStringFromObj(listObjElem[2], NULL);
	setup_pointer_by_name;
	scan_info[1].var = ppointer;
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

    case HS_3D_HISTOGRAM:
	scandim = 3;
	/* The result specifier looks like this: {id name1 name2 name3} */
	if (nelem != 4)
	{
	    Tcl_AppendResult(interp, "invalid 3d histogram result specifier \"",
			     Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
	    goto fail0;
	}
	hs_3d_hist_num_bins(id, &nxbins, &nybins, &nzbins);
	hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);

	name = Tcl_GetStringFromObj(listObjElem[1], NULL);
	setup_pointer_by_name;
	scan_info[0].var = ppointer;
	scan_info[0].npoints = nxbins;
	xstep = (xmax - xmin)/(float)nxbins;
	scan_info[0].dmin = xmin + 0.5f * xstep;
	if (nxbins == 1)
	    scan_info[0].dmax = scan_info[0].dmin;
	else
	    scan_info[0].dmax = xmax - 0.5f * xstep;

	name = Tcl_GetStringFromObj(listObjElem[2], NULL);
	setup_pointer_by_name;
	scan_info[1].var = ppointer;
	scan_info[1].npoints = nybins;
	ystep = (ymax - ymin)/(float)nybins;
	scan_info[1].dmin = ymin + 0.5f * ystep;
	if (nybins == 1)
	    scan_info[1].dmax = scan_info[1].dmin;
	else
	    scan_info[1].dmax = ymax - 0.5f * ystep;

	name = Tcl_GetStringFromObj(listObjElem[3], NULL);
	setup_pointer_by_name;
	scan_info[2].var = ppointer;
	scan_info[2].npoints = nzbins;
	zstep = (zmax - zmin)/(float)nzbins;
	scan_info[2].dmin = zmin + 0.5f * zstep;
	if (nzbins == 1)
	    scan_info[2].dmax = scan_info[2].dmin;
	else
	    scan_info[2].dmax = zmax - 0.5f * zstep;

	data = (float *)malloc(nxbins * nybins * nzbins * sizeof(float));
	checknull(data);
	break;

    case HS_NTUPLE:
	/* The result specifier looks either like this: {id {name1 min1 max1 npoints1}},
	 * or like this: {id {name1 min1 max1 npoints1} {name2 min2 max2 npoints2}},
	 * or like this: {id {name1 ...} {name2 ...} {name3 ...}}.
         * The number of variables in the ntuple should be appropriate.
	 */
	if (nelem < 2 || nelem > 4)
	{
	    Tcl_AppendResult(interp, "invalid ntuple result specifier \"",
			     Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
	    goto fail0;
	}
	scandim = nelem - 1;
	ntuple_dim = hs_num_variables(id);
	if (ntuple_dim != scandim + 1)
	{
	    Tcl_AppendResult(interp, "number of ntuple variables is ",
			     "inconsistent with result specifier \"",
			     Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
		goto fail0;
	}
	/* Parse the range specifiers */
	for (i=1; i<nelem; ++i)
	{
	    if (Tcl_ListObjGetElements(interp, listObjElem[i], &nspec, &rangeSpec) != TCL_OK)
		goto fail0;
	    if (nspec != 4)
	    {
		Tcl_AppendResult(interp, "invalid ntuple result specifier \"",
				 Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
		goto fail0;
	    }
	    name = Tcl_GetStringFromObj(rangeSpec[0], NULL);
	    setup_pointer_by_name;
	    scan_info[i-1].var = ppointer;
	    if (Tcl_GetDoubleFromObj(interp, rangeSpec[1], &dmin) != TCL_OK)
		return TCL_ERROR;
	    if (Tcl_GetDoubleFromObj(interp, rangeSpec[2], &dmax) != TCL_OK)
		return TCL_ERROR;
	    if (Tcl_GetIntFromObj(interp, rangeSpec[3], &npoints) != TCL_OK)
		return TCL_ERROR;
	    if (npoints <= 0)
	    {
		Tcl_AppendResult(interp, "invalid ntuple result specifier \"",
				 Tcl_GetStringFromObj(objv[2], NULL), "\"", NULL);
		goto fail0;
	    }
	    scan_info[i-1].dmin = dmin;
	    scan_info[i-1].dmax = dmax;
	    scan_info[i-1].npoints = npoints;
	}
	if (ntuple_dim == 2)
	{
	    data = (float *)malloc(ntuple_dim*scan_info[0].npoints*sizeof(float));
	    checknull(data);
	}
	break;

    case HS_NONE:
	Tcl_AppendResult(interp, "invalid result specifier \"",
			 Tcl_GetStringFromObj(objv[2], NULL),
			 "\" : Histo-Scope item with id ",
			 Tcl_GetStringFromObj(listObjElem[0], NULL),
			 " doesn't exist", NULL);
	goto fail0;

    default:
	Tcl_AppendResult(interp, "invalid result specifier \"",
			 Tcl_GetStringFromObj(objv[2], NULL),
			 "\" : Histo-Scope item with id ",
			 Tcl_GetStringFromObj(listObjElem[0], NULL),
			 " is neither histogram nor ntuple", NULL);   
	goto fail0;
    }

    /* Parse the parameter values */
    for (argnum = 3; argnum < objc; ++argnum)
    {
	if (Tcl_ListObjGetElements(interp, objv[argnum], &nelem, &listObjElem) != TCL_OK)
	    goto fail0;
	if (nelem != 2)
	{
	    Tcl_AppendResult(interp, "invalid value specifier \"",
			     Tcl_GetStringFromObj(objv[argnum], NULL), "\"", NULL);
	    goto fail0;
	}

	/* Find the name */
	name = Tcl_GetStringFromObj(listObjElem[0], NULL);
	setup_pointer_by_name;

	/* Parse the value */
	if (Tcl_GetDoubleFromObj(interp, listObjElem[1], ppointer) != TCL_OK)
	    goto fail0;
    }

    /* Check that everything has been defined */
    if (value->ndim < 3)
	zdef = 1;
    if (value->ndim < 2)
	ydef = 1;
    if (xdef == 0)
    {
	Tcl_SetResult(interp, "x value is undefined", TCL_STATIC);
	goto fail0;
    }
    if (ydef == 0)
    {
	Tcl_SetResult(interp, "y value is undefined", TCL_STATIC);
	goto fail0;
    }
    if (zdef == 0)
    {
	Tcl_SetResult(interp, "z value is undefined", TCL_STATIC);
	goto fail0;
    }
    for (i=0; i<npars; ++i)
    {
	if (pardef[i] == 0)
	{
	    Tcl_AppendResult(interp, "parameter ", value->param_names[i],
			     " value is undefined", NULL);
	    goto fail0;
	}
    }

    /* Build the result */
    if (hs_incr_fitter_usage_count(value))
    {
	Tcl_AppendResult(interp, "failed to initialize function ",
			 funct_name, NULL);
	goto fail0;
    }
    pdata = data;
    if (ntuple_dim)
	hs_reset(id);
    dxp = scan_info[0].var;
    imax = scan_info[0].npoints;
    dxmin = scan_info[0].dmin;
    dxrange = scan_info[0].dmax - scan_info[0].dmin;
    dxpoints = (double)(scan_info[0].npoints - 1);
    if (scandim == 1)
    {
	for (i=0; i<imax; ++i)
	{
	    if (i)
		*dxp = dxmin + ((double)i / dxpoints) * dxrange;
	    else
		*dxp = dxmin;
	    get_function_value;
	    if (ntuple_dim)
		*pdata++ = *dxp;
	    *pdata++ = dfval;
	}
    }
    else
    {
	dyp = scan_info[1].var;
	jmax = scan_info[1].npoints;
	dymin = scan_info[1].dmin;
	dyrange = scan_info[1].dmax - scan_info[1].dmin;
	dypoints = (double)(scan_info[1].npoints - 1);
	if (scandim == 2)
	{
	    for (i=0; i<imax; ++i)
	    {
		/* The following "if" is needed in order to avoid
		 * division by 0 when dxpoints == 0.0
		 */
		if (i)
		    *dxp = dxmin + ((double)i / dxpoints) * dxrange;
		else
		    *dxp = dxmin;
		for (j=0; j<jmax; ++j)
		{
		    if (j)
			*dyp = dymin + ((double)j / dypoints) * dyrange;
		    else
			*dyp = dymin;
		    get_function_value;
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
			*pdata++ = dfval;
		}
	    }
	}
	else
	{
	    dzp = scan_info[2].var;
	    kmax = scan_info[2].npoints;
	    dzmin = scan_info[2].dmin;
	    dzrange = scan_info[2].dmax - scan_info[2].dmin;
	    dzpoints = (double)(scan_info[2].npoints - 1);
	    if (scandim == 3)
	    {
		for (i=0; i<imax; ++i)
		{
		    if (i)
			*dxp = dxmin + ((double)i / dxpoints) * dxrange;
		    else
			*dxp = dxmin;
		    for (j=0; j<jmax; ++j)
		    {
			if (j)
			    *dyp = dymin + ((double)j / dypoints) * dyrange;
			else
			    *dyp = dymin;
			for (k=0; k<kmax; ++k)
			{
			    if (k)
				*dzp = dzmin + ((double)k / dzpoints) * dzrange;
			    else
				*dzp = dzmin;
			    get_function_value;
			    if (ntuple_dim)
			    {
				ntuple_data[0] = *dxp;
				ntuple_data[1] = *dyp;
				ntuple_data[2] = *dzp;
				ntuple_data[3] = dfval;
				if (hs_fill_ntuple(id, ntuple_data) != id)
				{
				    Tcl_SetResult(interp, "ntuple fill failed", TCL_STATIC);
				    goto fail1;
				}
			    }
			    else
				*pdata++ = dfval;
			}
		    }
		}
	    }
	    else
		assert(0);
	}
    }
    if (itype == HS_1D_HISTOGRAM)
	hs_1d_hist_block_fill(id, data, NULL, NULL);
    else if (itype == HS_2D_HISTOGRAM)
	hs_2d_hist_block_fill(id, data, NULL, NULL);
    else if (itype == HS_3D_HISTOGRAM)
	hs_3d_hist_block_fill(id, data, NULL, NULL);
    else if (ntuple_dim && scandim == 1)
    {
	pdata = data;
	for (i=0; i<imax; ++i)
	{
	    if (hs_fill_ntuple(id, pdata) != id)
	    {
		Tcl_SetResult(interp, "ntuple fill failed", TCL_STATIC);
		goto fail1;
	    }
	    pdata += 2;
	}
    }
    if (hs_decr_fitter_usage_count(value))
    {
	Tcl_AppendResult(interp, "failed to clean up after function ",
			 funct_name, NULL);
	goto fail0;
    }

    checkfree(data);
    checkfree(pars);
    checkfree(pardef);
    return TCL_OK;

 fail1:
    hs_decr_fitter_usage_count(value);

 fail0:
    checkfree(data);
    checkfree(pars);
    checkfree(pardef);
    return TCL_ERROR;
}

#define get_y_integral_rectangle do {\
        ysum = 0.0;\
	for (j=0; j<jmax; ++j)\
	{\
	    *dyp = dymin + yhalfstep + ((double)j / dypoints) * dyrange;\
	    get_function_value;\
	    ysum += dfval;\
	}\
} while(0);

#define get_y_integral_trapezoid do {\
        ysum = 0.0;\
	*dyp = dymin;\
	get_function_value;\
	ysum += dfval;\
	for (j=1; j<jmax; ++j)\
	{\
	    *dyp = dymin + ((double)j / dypoints) * dyrange;\
	    get_function_value;\
	    ysum += 2.0*dfval;\
	}\
	*dyp = scan_info[1].dmax;\
	get_function_value;\
	ysum += dfval;\
} while(0);

#define get_z_integral_trapez do {\
    zsum = 0.0;\
    *dzp = dzmin;\
    get_function_value;\
    zsum += dfval;\
    for (k=1; k<kmax; ++k)\
    {\
	*dzp = dzmin + ((double)k / dzpoints) * dzrange;\
	get_function_value;\
	zsum += 2.0*dfval;\
    }\
    *dzp = scan_info[2].dmax;\
    get_function_value;\
    zsum += dfval;\
    zsum /= 2.0;\
} while(0);

#define get_yz_integral_trapez do {\
    ysum = 0.0;\
    *dyp = dymin;\
    get_z_integral_trapez;\
    ysum += zsum;\
    for (j=1; j<jmax; ++j)\
    {\
	*dyp = dymin + ((double)j / dypoints) * dyrange;\
	get_z_integral_trapez;\
	ysum += 2.0*zsum;\
    }\
    *dyp = scan_info[1].dmax;\
    get_z_integral_trapez;\
    ysum += zsum;\
    ysum /= 2.0;\
} while(0);

#define get_y_integral_simpson do {\
        ysum = 0.0;\
	*dyp = dymin;\
	get_function_value;\
	ysum += dfval;\
	*dyp += yhalfstep;\
	get_function_value;\
	ysum += 4.0*dfval;\
	for (j=1; j<jmax; ++j)\
	{\
	    *dyp = dymin + ((double)j / dypoints) * dyrange;\
	    get_function_value;\
	    ysum += 2.0*dfval;\
	    *dyp += yhalfstep;\
	    get_function_value;\
	    ysum += 4.0*dfval;\
	}\
	*dyp = scan_info[1].dmax;\
	get_function_value;\
	ysum += dfval;\
} while(0);

#define check_non_negative do {\
    if (dfval < 0.0)\
    {\
	Tcl_AppendResult(interp, "function ",\
			 funct_name, " is not a pdf: negative for\n", NULL);\
	sprintf(stringbuf, "%d", value->mode);\
	Tcl_AppendResult(interp, "mode = ", stringbuf, "\n", NULL);\
	sprintf(stringbuf, "%g", x);\
	Tcl_AppendResult(interp, "x = ", stringbuf, "\n", NULL);\
	if (value->ndim > 1)\
	{\
	    sprintf(stringbuf, "%g", y);\
	    Tcl_AppendResult(interp, "y = ", stringbuf, "\n", NULL);\
	}\
	if (value->ndim > 2)\
	{\
	    sprintf(stringbuf, "%g", z);\
	    Tcl_AppendResult(interp, "z = ", stringbuf, "\n", NULL);\
	}\
	for (k=0; k<npars; ++k)\
	{\
	    sprintf(stringbuf, "%g", pars[k]);\
	    Tcl_AppendResult(interp, value->param_names[k],\
			     " = ", stringbuf, "\n", NULL);\
	}\
        goto fail1;\
    }\
} while(0);

#define accumulate_moment_sums diff = *dxp - mean;\
    diffsq = diff*diff;\
    diffsq_w = diffsq*weights[iw++];\
    sum2 += diffsq_w;\
    sum3 += diff*diffsq_w;\
    sum4 += diffsq*diffsq_w;

tcl_routine(fitfun_integrate)
{
    /* Usages:
     *  fitfun_integrate funct_name ... {var1 min1 max1 n_intervals1} \
     *  ... {var2 min2 max2 n_intervals2} ... {name1 value1} ... {nameN valueN}
     *
     *  fitfun_moments funct_name ... {var1 min1 max1 n_intervals1} \
     *  ... {name1 value1} ... {nameN valueN}
     */
    enum {
        INTEGRATE = 0,
        MOMENTS,
        RANDOM,
	INVCDF,
        CDF
    };
    char *funct_name, *name, *op;
    Minuit_fitter_info *value;
    int i, j, k, iw, imax, jmax, kmax, npars, nelem, argnum, npoints, fmode;
    int scandim = 0, ierr = 0, iop, nrandom = 0, invcdf_linear = 0;
    double x = 0.0, y = 0.0, z = 0.0, dmin, dmax;
    double sum, sum1, sum2, sum3, sum4, ysum, zsum, xhalfstep, yhalfstep;
    int xdef = 0, ydef = 0, zdef = 0;
    double *pars = NULL, *weights = NULL;
    int *pardef = NULL;
    float *fdata = NULL;
    double *ppointer, *dxp, *dyp = NULL, *dzp = NULL;
    Tcl_Obj **listObjElem;
    Tcl_Obj *oPtr;
    Scan_info scan_info[3] = {{NULL, 0.0, 0.0, 0},
			      {NULL, 0.0, 0.0, 0},
			      {NULL, 0.0, 0.0, 0}};
    double dxmin, dxrange, dxpoints, dymin, dyrange, dypoints, dfval;
    double dzmin, dzrange, dzpoints, zhalfstep;
    double diff, diffsq, diffsq_w;
    double mean = 0.0, sigma = 0.0, skew = 0.0, kurt = 0.0;
    struct simpson_bin *regularcdf = NULL;
    char stringbuf[32];

    if (objc < 3)
    {
	Tcl_AppendResult(interp, "wrong # of arguments (fit function ",
			 Tcl_GetStringFromObj(objv[0],NULL), ", option ",
			 Tcl_GetStringFromObj(objv[1],NULL), ")", NULL);
	return TCL_ERROR;
    }
    op = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcmp(op, "integrate") == 0)
	iop = INTEGRATE;
    else if (strcmp(op, "moments") == 0)
	iop = MOMENTS;
    else if (strcmp(op, "cdf") == 0)
	iop = CDF;
    else if (strcmp(op, "invcdf_lin") == 0)
    {
	iop = INVCDF;
        invcdf_linear = 1;
    }
#ifdef USE_CERNLIB
    else if (strcmp(op, "random") == 0)
        iop = RANDOM;
    else if (strcmp(op, "invcdf") == 0)
        iop = INVCDF;
#endif
    else
	assert(0);

    /* Parse function name */
    funct_name = Tcl_GetStringFromObj(objv[0], NULL);
    value = (Minuit_fitter_info *)clientData;

    /* Parse the number of random points to generate */
    if (iop == RANDOM)
    {
        if (Tcl_GetIntFromObj(interp, objv[2], &nrandom) != TCL_OK)
            return TCL_ERROR;
        if (nrandom < 0)
        {
            Tcl_AppendResult(interp, "expected a non-negative integer, got ",
                             Tcl_GetStringFromObj(objv[2],NULL), NULL);
            return TCL_ERROR;
        }
        ++objv;
        --objc;
    }
    /* Parse the set of points at which the (inverse)
     * cumulative density should be evaluated
     */
    else if (iop == CDF || iop == INVCDF)
    {
	if (Tcl_ListObjGetElements(interp, objv[2], &nrandom, &listObjElem) != TCL_OK)
            return TCL_ERROR;
	if (nrandom > 0)
	{
	    double dvalue;
	    fdata = (float *)malloc(nrandom*sizeof(float));
            if (fdata == NULL)
            {
                Tcl_SetResult(interp, "out of memory", TCL_STATIC);
                goto fail0;
            }
	    for (i=0; i<nrandom; ++i)
		if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &dvalue) != TCL_OK)
		    return TCL_ERROR;
		else if ((dvalue < 0.0 || dvalue > 1.0) && iop == INVCDF)
		{
		    Tcl_SetResult(interp, "inverse cdf argument out of [0, 1] range", TCL_STATIC);
		    goto fail0;
		}
		else
		    fdata[i] = dvalue;
	}
        ++objv;
        --objc;
    }

    /* Array of parameter values */
    npars = value->npars_max;
    if (npars > 0)
    {
	pars = (double *)calloc(npars, sizeof(double));
	pardef = (int *)calloc(npars, sizeof(int));
	checknull(pars && pardef);
	/* It is OK if some parameters are undefined */
	for (i=hs_fitter_num_pars(value); i<npars; ++i)
	    pardef[i] = 1;
    }

    /* Parse the parameter values and the ranges */
    for (argnum = 2; argnum < objc; ++argnum)
    {
	if (Tcl_ListObjGetElements(interp, objv[argnum], &nelem, &listObjElem) != TCL_OK)
	    goto fail0;
	if (nelem == 4)
	{
	    if (iop == MOMENTS)
	    {
		if (scandim >= 1)
		{
		    Tcl_AppendResult(interp, "too many integration range specifiers, ",
				     "only calculation of 1d moments is supported", NULL);
		    goto fail0;
		}
	    }
            else if (iop == RANDOM || iop == INVCDF || iop == CDF)
            {
		if (scandim >= 1)
		{
		    Tcl_AppendResult(interp, "too many integration range specifiers, ",
				     "only 1d distributions are supported", NULL);
		    goto fail0;
		}
            }
	    else if (iop == INTEGRATE)
	    {
		if (scandim >= 3)
		{
		    Tcl_AppendResult(interp, "too many integration range specifiers, ",
				     "only 1, 2, and 3d integrals are supported", NULL);
		    goto fail0;
		}
	    }
            else
                assert(0);
	    name = Tcl_GetStringFromObj(listObjElem[0], NULL);
	    setup_pointer_by_name;
	    scan_info[scandim].var = ppointer;
	    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &dmin) != TCL_OK)
		return TCL_ERROR;
	    if (Tcl_GetDoubleFromObj(interp, listObjElem[2], &dmax) != TCL_OK)
		return TCL_ERROR;
	    if (Tcl_GetIntFromObj(interp, listObjElem[3], &npoints) != TCL_OK)
		return TCL_ERROR;
	    if (npoints <= 0)
	    {
		Tcl_AppendResult(interp, "invalid integration range specifier \"",
				 Tcl_GetStringFromObj(objv[argnum], NULL), "\"", NULL);
		goto fail0;
	    }
	    scan_info[scandim].dmin = dmin;
	    scan_info[scandim].dmax = dmax;
	    scan_info[scandim].npoints = npoints;
	    ++scandim;
	}
	else if (nelem == 2)
	{
	    /* Find the name */
	    name = Tcl_GetStringFromObj(listObjElem[0], NULL);
	    setup_pointer_by_name;

	    /* Parse the value */
	    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], ppointer) != TCL_OK)
		goto fail0;
	}
	else
	{
	    Tcl_AppendResult(interp, "invalid value specifier \"",
			     Tcl_GetStringFromObj(objv[argnum], NULL), "\"", NULL);
	    goto fail0;
	}
    }

    /* Check that everything has been defined */
    if (value->ndim < 3)
	zdef = 1;
    if (value->ndim < 2)
	ydef = 1;
    if (xdef == 0)
    {
	Tcl_SetResult(interp, "x value is undefined", TCL_STATIC);
	goto fail0;
    }
    if (ydef == 0)
    {
	Tcl_SetResult(interp, "y value is undefined", TCL_STATIC);
	goto fail0;
    }
    if (zdef == 0)
    {
	Tcl_SetResult(interp, "z value is undefined", TCL_STATIC);
	goto fail0;
    }
    for (i=0; i<npars; ++i)
    {
	if (pardef[i] == 0)
	{
	    Tcl_AppendResult(interp, "parameter ", value->param_names[i],
			     " value is undefined", NULL);
	    goto fail0;
	}
    }

    /* Check that there is at least one integration range specifier */
    if (scandim == 0)
    {
	Tcl_AppendResult(interp, "no integration range specifiers provided", NULL);
	goto fail0;
    }

    /* Build the result */
    if (iop == MOMENTS || iop == RANDOM || iop == INVCDF || iop == CDF)
	assert(scandim == 1);
    if (hs_incr_fitter_usage_count(value))
    {
	Tcl_AppendResult(interp, "failed to initialize function ",
			 funct_name, NULL);
	goto fail0;
    }
    sum = 0.0;
    sum1 = 0.0;
    sum2 = 0.0;
    sum3 = 0.0;
    sum4 = 0.0;
    dxp = scan_info[0].var;
    imax = scan_info[0].npoints;
    dxmin = scan_info[0].dmin;
    dxrange = scan_info[0].dmax - scan_info[0].dmin;
    dxpoints = (double)(scan_info[0].npoints);
    xhalfstep = dxrange/dxpoints/2.0;

    if (scandim == 3)
    {
	dyp = scan_info[1].var;
	jmax = scan_info[1].npoints;
	dymin = scan_info[1].dmin;
	dyrange = scan_info[1].dmax - scan_info[1].dmin;
	dypoints = (double)(scan_info[1].npoints);
	yhalfstep = dyrange/dypoints/2.0;

	dzp = scan_info[2].var;
	kmax = scan_info[2].npoints;
	dzmin = scan_info[2].dmin;
	dzrange = scan_info[2].dmax - scan_info[2].dmin;
	dzpoints = (double)(scan_info[2].npoints);
	zhalfstep = dzrange/dzpoints/2.0;

        /* We will use weight 2/3 for the bin center and 1/24 for
           every bin vertex. We will not use ribs or surfaces.
	   This scheme is exact for all monomials of the order <= 3.
           Integrator of the fitter also uses this scheme.
           It is not possible to create a scheme exact for all monomials
           of the order <= 4 using only 4 coefficients (for center,
           vertex, rib, and surface).
	*/
	*dxp = dxmin;
	get_yz_integral_trapez;
	sum += ysum;
	for (i=1; i<imax; ++i)
	{
	    *dxp = dxmin + ((double)i / dxpoints) * dxrange;
	    get_yz_integral_trapez;
	    sum += 2.0*ysum;
	}
	*dxp = scan_info[0].dmax;
	get_yz_integral_trapez;
	sum += ysum;
	sum /= 6.0;

	for (i=0; i<imax; ++i)
	{
	    *dxp = dxmin + xhalfstep + ((double)i / dxpoints) * dxrange;
	    for (j=0; j<jmax; ++j)
	    {
		*dyp = dymin + yhalfstep + ((double)j / dypoints) * dyrange;
		for (k=0; k<kmax; ++k)
		{
		    *dzp = dzmin + zhalfstep + ((double)k / dzpoints) * dzrange;
		    get_function_value;
		    sum1 += dfval;
		}
	    }
	}
	sum += (sum1*2.0/3.0);
	sum *= (xhalfstep*yhalfstep*zhalfstep*8.0);
    }
    else if (scandim == 2)
    {
	dyp = scan_info[1].var;
	jmax = scan_info[1].npoints;
	dymin = scan_info[1].dmin;
	dyrange = scan_info[1].dmax - scan_info[1].dmin;
	dypoints = (double)(scan_info[1].npoints);
	yhalfstep = dyrange/dypoints/2.0;

	/* Integration using the single bin weights
	        |1 0 1|    1
	        |0 8 0| * --
	        |1 0 1|   12
	   This scheme is exact for all monomials of the order <= 3.
	   It is also consistent with the integration scheme used
	   by the fitter.
	*/
	*dxp = dxmin;
	get_y_integral_trapezoid;
	sum += ysum;
	for (i=1; i<imax; ++i)
	{
	    *dxp = dxmin + ((double)i / dxpoints) * dxrange;
	    get_y_integral_trapezoid;
	    sum += 2.0*ysum;
	}
	*dxp = scan_info[0].dmax;
	get_y_integral_trapezoid;
	sum += ysum;
	for (i=0; i<imax; ++i)
	{
	    *dxp = dxmin + xhalfstep + ((double)i / dxpoints) * dxrange;
	    get_y_integral_rectangle;
	    sum += 8.0*ysum;
	}
	sum *= (xhalfstep * yhalfstep / 3.0);

	/* Integration using the bin weights 
	        |1 4  1|    1
	        |4 16 4| * --
	        |1 4  1|   36
	   This is the "double Simpson" scheme. It is also exact
	   for all monomials of the order <= 3, but it requires
	   more function calls per bin than the previous scheme
	   (4 instead of 2). However, this scheme is also exact
	   for the x^2*y^2 monomial. For now, we will choose
	   a faster method...
	*/
/*  	get_y_integral_simpson; */
/*  	sum += ysum; */
/*  	*dxp += xhalfstep; */
/*  	get_y_integral_simpson; */
/*  	sum += 4.0*ysum; */
/*  	for (i=1; i<imax; ++i) */
/*  	{ */
/*  	    *dxp = dxmin + ((double)i / dxpoints) * dxrange; */
/*  	    get_y_integral_simpson; */
/*  	    sum += 2.0*ysum; */
/*  	    *dxp += xhalfstep; */
/*  	    get_y_integral_simpson; */
/*  	    sum += 4.0*ysum; */
/*  	} */
/*  	*dxp = scan_info[0].dmax; */
/*  	get_y_integral_simpson; */
/*  	sum += ysum; */
/*  	sum *= (xhalfstep * yhalfstep / 9.0); */
    }
    else if (scandim == 1)
    {
	if (iop == MOMENTS)
	{
	    weights = (double *)malloc((2*imax+1)*sizeof(double));
	    if (weights == NULL)
	    {
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail1;
	    }
	    iw = 0;

	    /* Calculate the sum and the mean */
	    *dxp = dxmin;
	    get_function_value;
	    check_non_negative;
	    weights[iw++] = dfval;
	    sum += dfval;
	    sum1 += *dxp*dfval;
	    *dxp += xhalfstep;
	    get_function_value;
	    check_non_negative;
	    weights[iw++] = 4.0*dfval;
	    sum += 4.0*dfval;
	    sum1 += *dxp*4.0*dfval;
	    for (i=1; i<imax; ++i)
	    {
		*dxp = dxmin + ((double)i / dxpoints) * dxrange;
		get_function_value;
		check_non_negative;
		weights[iw++] = 2.0*dfval;
		sum += 2.0*dfval;
		sum1 += *dxp*2.0*dfval;
		*dxp += xhalfstep;
		get_function_value;
		check_non_negative;
		weights[iw++] = 4.0*dfval;
		sum += 4.0*dfval;
		sum1 += *dxp*4.0*dfval;
	    }
	    *dxp = scan_info[0].dmax;
	    get_function_value;
	    check_non_negative;
	    weights[iw++] = dfval;
	    sum += dfval;
	    sum1 += *dxp*dfval;
	    if (sum == 0.0)
	    {
		Tcl_SetResult(interp,
			      "can't define moments: function value is always 0",
			      TCL_STATIC);
		goto fail1;
	    }
	    mean = sum1/sum;

	    /* Calculate higher moments */
	    iw = 0;
	    *dxp = dxmin;
	    accumulate_moment_sums;
	    *dxp += xhalfstep;
	    accumulate_moment_sums;
	    for (i=1; i<imax; ++i)
	    {
		*dxp = dxmin + ((double)i / dxpoints) * dxrange;
		accumulate_moment_sums;
		*dxp += xhalfstep;
		accumulate_moment_sums;
	    }
	    *dxp = scan_info[0].dmax;
	    accumulate_moment_sums;

	    sum2 /= sum;
	    sum3 /= sum;
	    sum4 /= sum;
	    sigma = sqrt(sum2);
	    skew = sum3/sigma/sigma/sigma;
	    kurt = sum4/sum2/sum2;
	    sum *= (xhalfstep / 3.0);
	}
	else if (iop == INTEGRATE)
	{
	    *dxp = dxmin;
	    get_function_value;
	    sum += dfval;
	    *dxp += xhalfstep;
	    get_function_value;
	    sum += 4.0*dfval;
	    for (i=1; i<imax; ++i)
	    {
		*dxp = dxmin + ((double)i / dxpoints) * dxrange;
		get_function_value;
		sum += 2.0*dfval;
		*dxp += xhalfstep;
		get_function_value;
		sum += 4.0*dfval;
	    }
	    *dxp = scan_info[0].dmax;
	    get_function_value;
	    sum += dfval;
	    sum *= (xhalfstep / 3.0);
	}
        else if ((iop == RANDOM || iop == INVCDF || iop == CDF) && nrandom > 0)
        {
            double dxmax, xcenter, xedge, edgeval, fmax, normd1, normd2;
            int status;

            regularcdf = (struct simpson_bin *)malloc(
                imax*sizeof(struct simpson_bin));
	    if (regularcdf == NULL)
	    {
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail1;
	    }
            dxmax = scan_info[0].dmax;
            *dxp = dxmin;
            get_function_value;
            check_non_negative;
            edgeval = dfval;
            for (i=0; i<imax; ++i)
            {
                xcenter = dxmin + ((double)i/dxpoints)*dxrange + xhalfstep;
                if (i == imax-1)
                    xedge = dxmax;
                else
                    xedge = xcenter + xhalfstep;
                *dxp = xedge;
                get_function_value;
                check_non_negative;
                fmax = dfval;
                *dxp = xcenter;
                get_function_value;
                check_non_negative;
                sum += ((edgeval + fmax + 4.0*dfval)/6.0);
                regularcdf[i].d1 = fmax - edgeval;
                regularcdf[i].d2 = fmax - 2.0*dfval + edgeval;
                regularcdf[i].integ = sum;
                edgeval = fmax;
            }
            if (sum == 0.0)
            {
		Tcl_SetResult(interp, "can't generate inverse cdf: "
                              "all function values are 0", TCL_STATIC);
		goto fail1;
            }
            normd1 = sum*xhalfstep*xhalfstep*4.0;
            normd2 = normd1*xhalfstep/2.0;
            for (i=0; i<imax; ++i)
            {
                regularcdf[i].integ /= sum;
                regularcdf[i].d1    /= normd1;
                regularcdf[i].d2    /= normd2;
            }

            /* Generate the points */
	    if (iop == RANDOM)
	    {
		fdata = (float *)malloc(nrandom*sizeof(float));
		if (fdata == NULL)
		{
		    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		    goto fail0;
		}
		i = nrandom;
#ifdef USE_CERNLIB
		ranlux_(fdata, &i);
#else
            /* Should never get here without CERNLIB */
		assert(0);
#endif
	    }

            if (iop == CDF)
            {
                for (i=0; i<nrandom; ++i)
                    fdata[i] = find_cdf_value(fdata[i], 0.0, dxmin,
                                              dxmax, imax, regularcdf);
            }
            else
            {
                for (i=0; i<nrandom; ++i)
                {
                    const float ftmp = fdata[i];
                    if (iop == RANDOM && (ftmp <= 0.f || ftmp >= 1.f))
                    {
                        status = 1;
#ifdef USE_CERNLIB
                        ranlux_(fdata+i, &status);
#endif
                        --i;
                        continue;
                    }

                    status = 0;
                    if (ftmp <= 0.f)
                        xcenter = dxmin;
                    else if (ftmp >= 1.f)
                        xcenter = dxmax;
                    else
                    {
                        if (invcdf_linear)
                            status = find_inverse_cdf_value_linear(
                                (double)ftmp, 0.0, dxmin,
                                dxmax, imax, regularcdf, &xcenter);
                        else
                        {
#ifdef USE_CERNLIB
                            status = find_inverse_cdf_value(
                                (double)ftmp, 0.0, dxmin,
                                dxmax, imax, regularcdf, &xcenter);
#else
                            status = 1;
                            xcenter = 0.0;
#endif
                        }
                    }

                    assert(status == 0);
                    fdata[i] = (float)xcenter;
                }
            }
        }
        else
            assert(0);
    }
    else
	assert(0);

    if (hs_decr_fitter_usage_count(value))
    {
	Tcl_AppendResult(interp, "failed to clean up after function ",
			 funct_name, NULL);
	goto fail0;
    }

    /* Create the result */
    if (iop == MOMENTS)
    {
	oPtr = Tcl_NewListObj(0, NULL);
	Tcl_ListObjAppendElement(interp, oPtr, Tcl_NewDoubleObj(sum));
	Tcl_ListObjAppendElement(interp, oPtr, Tcl_NewDoubleObj(mean));
	Tcl_ListObjAppendElement(interp, oPtr, Tcl_NewDoubleObj(sigma));
	Tcl_ListObjAppendElement(interp, oPtr, Tcl_NewDoubleObj(skew));
	Tcl_ListObjAppendElement(interp, oPtr, Tcl_NewDoubleObj(kurt));
	Tcl_SetObjResult(interp, oPtr);
    }
    else if (iop == INTEGRATE)
    {
	Tcl_SetObjResult(interp, Tcl_NewDoubleObj(sum));
    }
    else if (iop == RANDOM || iop == INVCDF || iop == CDF)
    {
        Tcl_Obj **result = NULL;
        if (nrandom > 0)
        {
            result = (Tcl_Obj **)malloc(nrandom*sizeof(Tcl_Obj *));
            if (result == NULL)
            {
                Tcl_SetResult(interp, "out of memory", TCL_STATIC);
                goto fail0;
            }
            for (i=0; i<nrandom; ++i)
                result[i] = Tcl_NewDoubleObj((double)(fdata[i]));
        }
        Tcl_SetObjResult(interp, Tcl_NewListObj(nrandom, result));
        checkfree(result);
    }
    else
        assert(0);

    checkfree(fdata);
    checkfree(regularcdf);
    checkfree(weights);
    checkfree(pars);
    checkfree(pardef);
    return TCL_OK;

 fail1:
    hs_decr_fitter_usage_count(value);

 fail0:
    checkfree(fdata);
    checkfree(regularcdf);
    checkfree(weights);
    checkfree(pars);
    checkfree(pardef);
    return TCL_ERROR;
}

tcl_routine(fitfun_eval)
{
    /* Usage: funct_name eval {name1 value1} ... {nameN valueN} */
    char *funct_name, *name, *op;
    Minuit_fitter_info *value;
    int i, k, npars, nelem, argnum, fmode, ierr = 0;
    double x = 0.0, y = 0.0, z = 0.0, dfval;
    int xdef = 0, ydef = 0, zdef = 0;
    double *ppointer = NULL, *pars = NULL;
    int *pardef = NULL;
    Tcl_Obj *result, **listObjElem;
    char stringbuf[32];
    int listeval = 1;

    if (objc < 3)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Parse function name */
    funct_name = Tcl_GetStringFromObj(objv[0], NULL);
    value = (Minuit_fitter_info *)clientData;

    /* Array of parameter values */
    npars = value->npars_max;
    if (npars > 0)
    {
	pars = (double *)calloc(npars, sizeof(double));
	pardef = (int *)calloc(npars, sizeof(int));
	checknull(pars && pardef);
	/* It is OK if some parameters are undefined */
	for (i=hs_fitter_num_pars(value); i<npars; ++i)
	    pardef[i] = 1;
    }

    op = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcmp(op, "eval") == 0)
        listeval = 0;

    /* Parse the parameter values */
    for (argnum = objc-1; argnum >= 2; --argnum)
    {
	if (Tcl_ListObjGetElements(interp, objv[argnum],
				   &nelem, &listObjElem) != TCL_OK)
	    goto fail0;
	if (nelem != 2)
	{
	    Tcl_AppendResult(interp, "invalid value specifier \"",
			     Tcl_GetStringFromObj(objv[argnum], NULL), "\"", NULL);
	    goto fail0;
	}

	/* Find the name */
	name = Tcl_GetStringFromObj(listObjElem[0], NULL);
	setup_pointer_by_name;

	/* Parse the value */
        if (!listeval || argnum > 2)
            if (Tcl_GetDoubleFromObj(interp, listObjElem[1], ppointer) != TCL_OK)
                goto fail0;
    }

    /* Check that everything has been defined */
    if (value->ndim < 3)
	zdef = 1;
    if (value->ndim < 2)
	ydef = 1;
    if (xdef == 0)
    {
	Tcl_SetResult(interp, "x value is undefined", TCL_STATIC);
	goto fail0;
    }
    if (ydef == 0)
    {
	Tcl_SetResult(interp, "y value is undefined", TCL_STATIC);
	goto fail0;
    }
    if (zdef == 0)
    {
	Tcl_SetResult(interp, "z value is undefined", TCL_STATIC);
	goto fail0;
    }
    for (i=0; i<npars; ++i)
    {
	if (pardef[i] == 0)
	{
	    Tcl_AppendResult(interp, "parameter ", value->param_names[i],
			     " value is undefined", NULL);
	    goto fail0;
	}
    }

    /* Build the result */
    if (hs_incr_fitter_usage_count(value))
    {
	Tcl_AppendResult(interp, "failed to initialize function ",
			 funct_name, NULL);
	goto fail0;
    }
    if (listeval)
    {
        float *arr;
        int size, freeLater;

        if (get_float_array_from_binary_or_list(
                interp, listObjElem[1],
                &arr, &size, &freeLater) != TCL_OK)
            return TCL_ERROR;
        if (freeLater)
        {
            // The argument is a list. Return a list.
            result = Tcl_NewListObj(0, NULL);
            for (i=0; i<size; ++i)
            {
                *ppointer = arr[i];
                if (value->fit_c)
                    dfval = value->fit_c(x, y, z, value->mode, pars, &ierr);
                else
                {
                    fmode = value->mode;
                    dfval = value->fit_f(&x, &y, &z, &fmode, pars, &ierr);
                }
                Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(dfval));
            }
            free(arr);
        }
        else
        {
            // The argument is a binary string. Return a binary string.
            float *res = NULL;
            if (size > 0)
            {
                res = (float *)malloc(size*sizeof(float));
                for (i=0; i<size; ++i)
                {
                    *ppointer = arr[i];
                    if (value->fit_c)
                        dfval = value->fit_c(x, y, z, value->mode, pars, &ierr);
                    else
                    {
                        fmode = value->mode;
                        dfval = value->fit_f(&x, &y, &z, &fmode, pars, &ierr);
                    }
                    res[i] = dfval;
                }
            }
            result = Tcl_NewByteArrayObj((unsigned char *)(res),
                                         size*sizeof(float));
            if (res) free(res);
        }
    }
    else
    {
        if (value->fit_c)
            dfval = value->fit_c(x, y, z, value->mode, pars, &ierr);
        else
        {
            fmode = value->mode;
            dfval = value->fit_f(&x, &y, &z, &fmode, pars, &ierr);
        }
        result = Tcl_NewDoubleObj(dfval);
    }
    if (hs_decr_fitter_usage_count(value))
    {
	Tcl_AppendResult(interp, "failed to clean up after functuon ",
			 funct_name, NULL);
	goto fail0;
    }
    if (ierr)
    {
	report_funct_error(funct_name, value, ierr);
	goto fail0;
    }

    checkfree(pars);
    checkfree(pardef);
    Tcl_SetObjResult(interp, result);
    return TCL_OK;

 fail0:
    checkfree(pars);
    checkfree(pardef);
    return TCL_ERROR;
}

int hs_add_fitter_function(Tcl_Interp *interp, const char *name,
			   Minuit_fitter_info *value)
{
    Tcl_HashEntry *entryPtr;
    int i, entry_created;
    char buf[32];

    /* Check the correctness of the input arguments */
    assert(interp && name && value);
    if (name[0] == 0)
    {
	Tcl_SetResult(interp, "function name can not be an empty string", TCL_STATIC);
	return TCL_ERROR;
    }
    if (value->ndim < 1 || value->ndim > 3)
    {
	sprintf(buf, "%d", value->ndim);
	Tcl_AppendResult(interp, "invalid function dimensionality ", buf, NULL);
	return TCL_ERROR;
    }
    if (value->npars_min < 0 || value->npars_min > value->npars_max)
    {
	Tcl_SetResult(interp, "wrong number of parameters", TCL_STATIC);
	return TCL_ERROR;
    }
    if (value->npars_max > MAX_FUN_PARAMETERS)
    {
	sprintf(buf, "%d", MAX_FUN_PARAMETERS);
	Tcl_AppendResult(interp, "max number of parameters is too large, should be ",
			 buf, " or less", NULL);
	return TCL_ERROR;
    }
    if (value->fit_c == NULL && value->fit_f == NULL)
    {
	Tcl_SetResult(interp, "fit function not specified", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Check the parameter names */
    if (value->npars_max > 0)
    {
	assert(value->param_names);
	for (i=0; i<value->npars_max; ++i)
	{
	    assert(value->param_names[i]);
	    if (value->param_names[i][0] == '\0')
	    {
		Tcl_SetResult(interp, "parameter name can not be an empty string",
			      TCL_STATIC);
		return TCL_ERROR;
	    }
	}
    }

    /* Create an entry in the table of fitters */
    if (n_fitters == 0)
	Tcl_InitHashTable(&fitter_table, TCL_STRING_KEYS);
    entryPtr = Tcl_CreateHashEntry(&fitter_table, name, &entry_created);
    if (!entry_created)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 name, "\" already exists", NULL);
	return TCL_ERROR;
    }
    Tcl_SetHashValue(entryPtr, value);
    ++n_fitters;

    return TCL_OK;
}

Minuit_fitter_info * hs_find_fitter_function(const char *name)
{
    Tcl_HashEntry *entryPtr;

    assert(name);
    if (n_fitters == 0)
	return NULL;
    entryPtr = Tcl_FindHashEntry(&fitter_table, name);
    if (entryPtr == NULL)
	return NULL;
    return (Minuit_fitter_info *)Tcl_GetHashValue(entryPtr);
}

int hs_remove_fitter_function(const char *name, int really_remove)
{
    Tcl_HashEntry *entryPtr;
    Minuit_fitter_info *fun;

    assert(name);
    if (n_fitters == 0) return 1;

    /* Find the relevant hash entry */
    entryPtr = Tcl_FindHashEntry(&fitter_table, name);
    if (entryPtr == NULL) return 1;

    /* Check if the function is in use */
    fun = (Minuit_fitter_info *)Tcl_GetHashValue(entryPtr);
    if (fun->refcount)
	 return 2;

    /* Remove the fitter */
    if (really_remove)
    {
	hs_destroy_fitter_info((Minuit_fitter_info *)Tcl_GetHashValue(entryPtr));
	Tcl_DeleteHashEntry(entryPtr);
	--n_fitters;
    }
    return 0;
}

int hs_remove_fitter_group(int groupnumber)
{
    Tcl_HashEntry *entryPtr;
    Tcl_HashSearch hashIterator;
    char **namelist;
    Minuit_fitter_info *fitter_info;
    int i, n_delete = 0;

    if (n_fitters == 0) return 1;
    namelist = (char **)calloc(n_fitters, sizeof(char *));
    if (namelist == NULL)
    {
	fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
	fflush(stderr);
	abort();
    }

    /* Create the list of entries which should be deleted. We can not
     * iterate over the table and delete entries simultaneously.
     */
    entryPtr = Tcl_FirstHashEntry(&fitter_table, &hashIterator);
    while (entryPtr)
    {
	fitter_info = (Minuit_fitter_info *)Tcl_GetHashValue(entryPtr);
	if (fitter_info->groupnumber == groupnumber && !fitter_info->ownsdll)
	    namelist[n_delete++] = Tcl_GetHashKey(&fitter_table, entryPtr);
	entryPtr = Tcl_NextHashEntry(&hashIterator);
    }
    if (n_delete == 0)
    {
	free(namelist);
	return 1;
    }

    /* Delete the entries */
    for (i=n_delete-1; i>=0; --i)
	if (hs_remove_fitter_function(namelist[i], 0))
	    return 2;
    for (i=n_delete-1; i>=0; --i)
	hs_remove_fitter_function(namelist[i], 1);

    free(namelist);
    return 0;
}

int hs_incr_fitter_usage_count(Minuit_fitter_info *fitfun)
{
    int fmode, status = 0;

    if (!fitfun->refcount)
    {
	if (fitfun->init)
	{
	    fmode = fitfun->mode;
	    status = (fitfun->init)(&fmode);
	}
    }
    if (status == 0)
	++fitfun->refcount;
    return status;
}

int hs_decr_fitter_usage_count(Minuit_fitter_info *fitfun)
{
    int fmode;

    if (!fitfun->refcount)
	return 0;
    if (fitfun->refcount-- != 1)
	return 0;
    if (fitfun->cleanup)
    {
	fmode = fitfun->mode;
	return (fitfun->cleanup)(&fmode);
    }
    else
	return 0;
}

void hs_destroy_fitter_info(Minuit_fitter_info *value)
{
    int i;

    if (value == NULL) return;

    if (value->is_combined)
    {
	assert(value->mode >= 0 && value->mode < n_combos);
	combo_table[value->mode].type = FITFUN_NONE;
    }

    checkfree(value->fullname);
    checkfree(value->description);
    if (value->param_names)
    {
	for (i=0; i<value->npars_max; ++i)
	    checkfree(value->param_names[i]);
	free(value->param_names);
    }
    checkfree(value->init_fun_name);
    checkfree(value->fit_fun_name);
    checkfree(value->grad_fun_name);
    checkfree(value->cleanup_fun_name);
    if (value->ownsdll)
	hs_decr_dll_refcount(value->groupnumber);
    free(value);
}

tcl_routine(sharedlib)
{
    char *command;
    if (objc < 2)
    {
	Tcl_SetResult(interp, "wrong # of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    command = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcmp(command, "open") == 0)
    {
	return tcl_c_name(dlopen) (clientData, interp, objc-1, objv+1);
    }
    else if (strcmp(command, "close") == 0)
    {
	return tcl_c_name(dlclose) (clientData, interp, objc-1, objv+1);
    }
    else if (strcmp(command, "list") == 0)
    {
	return tcl_c_name(dlliblist) (clientData, interp, objc-1, objv+1);
    }
    else if (strcmp(command, "name") == 0)
    {
	return tcl_c_name(dllibfile) (clientData, interp, objc-1, objv+1);
    }
    else
    {
	Tcl_AppendResult(interp, "Invalid option \"", command,
			 "\". Valid options are: ",
			 "open, ",
			 "close, ",
			 "list, ",
			 "and name.", NULL);
	return TCL_ERROR;
    }    
}

tcl_routine(dlopen)
{
    char *filename;
    void *handle;
    int (*hs_init)(Tcl_Interp *interp);
    int i, flags = RTLD_NOW;
    Tcl_DString ds;
    int export_globals = 0, argcount_ok = 1;

    if (objc < 2 || objc > 3)
        argcount_ok = 0;
    else if (objc == 3)
    {   
        if (strcmp(Tcl_GetStringFromObj(objv[1],NULL),"-export_globals") == 0)
            export_globals = 1;
        else
            argcount_ok = 0;
    }
    if (!argcount_ok)
    {
        Tcl_AppendResult(interp, "wrong # of arguments for option \"",
                         Tcl_GetStringFromObj(objv[0],NULL), "\"", NULL);
        return TCL_ERROR;
    }
    filename = Tcl_GetStringFromObj(objv[1+export_globals], NULL);
    Tcl_UtfToExternalDString(NULL, filename, -1, &ds);
    if (export_globals) flags |= RTLD_GLOBAL;
    handle = dlopen(Tcl_DStringValue(&ds), flags);
    Tcl_DStringFree(&ds);
    if (handle == NULL)
    {
	Tcl_AppendResult(interp, "failed to open file \"", filename,
			 "\": ", dlerror(), NULL);
	return TCL_ERROR;
    }

    /* Execute the _hs_init function */
    dlerror();
    hs_init = (int (*)(Tcl_Interp *))dlsym(handle, "_hs_init");
    if (dlerror() == NULL)
    {
	if (hs_init(interp) != TCL_OK)
	{
	    dlclose(handle);
	    return TCL_ERROR;
	}
	Tcl_ResetResult(interp);
    }

    /* Update the table of libraries */
    for (i=0; i<libtable_len; ++i)
    {
	if (libtable[i].filename == NULL)
	{
	    /* This is an empty slot */
	    break;
	}
    }
    if (i == libtable_len)
    {
	/* No available empty slots. Increase the table size. */
	libtable_len++;
	libtable = (DLLibrary *)realloc(libtable, libtable_len*sizeof(DLLibrary));
	if (libtable == NULL)
	{
	    fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
	    fflush(stderr);
	    abort();
	}
    }
    libtable[i].handle = handle;
    if ((libtable[i].filename = strdup(filename)) == NULL)
    {
	fprintf(stderr, "Fatal error: out of memory. Aborting.\n");
	fflush(stderr);
	abort();
    }
    libtable[i].refcount = 0;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
    return TCL_OK;
}

tcl_routine(dllibfile)
{
    int i;
    
    tcl_require_objc_sharedlib(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &i) != TCL_OK)
	return TCL_ERROR;
    if (i>=0 && i<libtable_len)
    {
	if (libtable[i].filename)
	{
	    Tcl_SetResult(interp, libtable[i].filename, TCL_STATIC);
	    return TCL_OK;
	}
    }
    Tcl_SetResult(interp, "invalid dll number", TCL_STATIC);
    return TCL_ERROR;
}

tcl_routine(dlliblist)
{
    Tcl_Obj **result;
    int i, count = 0;

    tcl_require_objc_sharedlib(1);
    if (libtable_len == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewListObj(0, NULL));
    }
    else
    {
	result = (Tcl_Obj **)malloc(libtable_len*sizeof(Tcl_Obj *));
	if (result == 0)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	for (i=0; i<libtable_len; ++i)
	    if (libtable[i].filename)
		result[count++] = Tcl_NewIntObj(i);
	Tcl_SetObjResult(interp, Tcl_NewListObj(count, result));
	free(result);
    }	    
    return TCL_OK;
}

int hs_incr_dll_refcount(int i)
{
    if (i>=0 && i<libtable_len)
    {
	if (libtable[i].filename)
	{
	    ++libtable[i].refcount;
	    return 0;
	}
	else
	    return 1;
    }
    return 1;
}

int hs_decr_dll_refcount(int i)
{
    if (i>=0 && i<libtable_len)
    {
	if (libtable[i].filename)
	{
	    if (libtable[i].refcount > 0)
		--libtable[i].refcount;
	    if (libtable[i].refcount == 0)
		return hs_close_fitter_dll_bynumber(i);
	}
	else
	    return 1;
    }
    return 1;
}

tcl_routine(dlclose)
{
    int i;

    tcl_require_objc_sharedlib(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &i) != TCL_OK)
	return TCL_ERROR;
    switch (hs_close_fitter_dll_bynumber(i))
    {
    case 0:
	return TCL_OK;
    case 1:
	Tcl_AppendResult(interp, "invalid dll number ",
			 Tcl_GetStringFromObj(objv[1],NULL), NULL);
	break;
    case 2:
	Tcl_AppendResult(interp, "failed to close dll ",
			 Tcl_GetStringFromObj(objv[1],NULL),
			 ": some objects are in use", NULL);
	break;
    default:
	assert(0);
    }
    return TCL_ERROR;
}

int hs_close_fitter_dll_bynumber(int i)
{
    if (i>=0 && i<libtable_len)
    {
	if (libtable[i].filename)
	{
	    switch (hs_remove_fitter_group(i))
	    {
	    case 0:
	    case 1:
		free(libtable[i].filename);
		libtable[i].filename = NULL;

		/* Execute the _hs_fini function */
		{
		    void (*hs_fini)(void);
		    dlerror();
		    hs_fini = (void (*)(void))dlsym(libtable[i].handle, "_hs_fini");
		    if (dlerror() == NULL)
			hs_fini();
		}

		dlclose(libtable[i].handle);
		libtable[i].handle = NULL;
		return 0;
	    case 2:
		return 2;
	    default:
		assert(0);
	    }
	}
    }
    return 1;
}

int hs_copy_fitter_function(Tcl_Interp *interp,
			    const char *newname,
			    const char *oldname)
{
    Minuit_fitter_info *oldfun, *newfun;

    assert(interp && newname && oldname);

    /* Find the relevant hash entry */
    oldfun = hs_find_fitter_function(oldname);
    if (oldfun == NULL)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 oldname, "\" does not exist", NULL);
	return TCL_ERROR;
    }

    /* Create a copy */
    newfun = hs_copy_fitter_info(oldfun);
    if (newfun == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }

    /* Insert the copy into the table of functions */
    if (hs_add_fitter_function(interp, newname, newfun) != TCL_OK)
    {
	hs_destroy_fitter_info(newfun);
	return TCL_ERROR;
    }
    return TCL_OK;
}

int hs_rename_fitter_function(Tcl_Interp *interp,
			      const char *newname,
			      const char *oldname)
{
    Tcl_HashEntry *entryPtr;
    Minuit_fitter_info *oldfun;

    assert(interp && newname && oldname);

    /* Find the relevant hash entry */
    if (n_fitters == 0)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 oldname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    entryPtr = Tcl_FindHashEntry(&fitter_table, oldname);
    if (entryPtr == NULL)
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 oldname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    oldfun = (Minuit_fitter_info *)Tcl_GetHashValue(entryPtr);
    if (oldfun->refcount > 0)
    {
	Tcl_AppendResult(interp, "can't rename function \"",
			 oldname, "\": this function is in use", NULL);
	return TCL_ERROR;
    }
    if (newname[0] == '\0')
    {
	/* There should be no reason by which this function can not be deleted */
	assert(hs_remove_fitter_function(oldname, 1) == 0);
	return TCL_OK;
    }
    /* Check that the function with new name doesn't exists */
    if (hs_find_fitter_function(newname))
    {
	Tcl_AppendResult(interp, "fit function with tag \"",
			 newname, "\" already exists", NULL);
	return TCL_ERROR;
    }
    /* Remove the old hash table entry */
    Tcl_DeleteHashEntry(entryPtr);
    --n_fitters;
    /* Reinsert the function into the table */
    return hs_add_fitter_function(interp, newname, oldfun);
}

void * hs_find_library_function(Tcl_Interp *interp, int libindex,
				const char* procname, int *code)
{
    /* Lookup the functions. First, search for the exact name match. 
     * If the exact match not found then convert to lower case, add 
     * the underscore, and search again -- this must be a FORTRAN function.
     * On exit, the value of *code is set to
     *   -1   in case the function not found
     *    0   if this appears to be a C function
     *    1   if this appears to be a FORTRAN function
     */
    void *location;
    char *fortname = 0;
    int i, len, isfortran;
    char charbuf[32];

    assert(procname);
    *code = -1;
    if (libindex < 0 || libindex >= libtable_len)
    {
	Tcl_SetResult(interp, "invalid dll number", TCL_STATIC);
	return NULL;
    }
    if (libtable[libindex].handle == NULL)
    {
	Tcl_SetResult(interp, "invalid dll number", TCL_STATIC);
	return NULL;
    }

    /* Figure out if this is a fortran name */
    len = strlen(procname);
    fortname = (char *)malloc((len + 2) * sizeof(char));
    if (fortname == NULL)
	return NULL;
    for (i=0; i<len; ++i)
	fortname[i] = tolower(procname[i]);
    if (procname[len-1] != '_')
	fortname[i++] = '_';
    fortname[i++] = '\0';
    if (strcmp(procname, fortname) == 0)
	isfortran = 1;
    else
	isfortran = 0;

    /* Look up the original name. Before doing that, however,
     * we should clear the dl error message buffer.
     */
    dlerror();
    location = dlsym(libtable[libindex].handle, procname);
    if (dlerror() == NULL)
    {
	*code = isfortran;
	free(fortname);
	return location;
    }

    /* Try to find the fortran name */
    if (!isfortran)
    {
	location = dlsym(libtable[libindex].handle, fortname);
	if (dlerror() == NULL)
	{
	    *code = 1;
	    free(fortname);
	    return location;
	}
    }
    
    free(fortname);
    sprintf(charbuf, "%d", libindex);
    Tcl_AppendResult(interp, "function \"", procname,
		     "\" not found in library # ", charbuf, NULL);
    return NULL;
}

int hs_fitter_num_pars(Minuit_fitter_info *fitfun)
{
    assert(fitfun);
    if (fitfun->npars_min == fitfun->npars_max)
	return fitfun->npars_min;
    else
	return fitfun->mode;
}

Minuit_fitter_info * hs_copy_fitter_info(const Minuit_fitter_info *p)
{
    int i;
    Minuit_fitter_info *value = 0;

    assert(p);
    value = (Minuit_fitter_info *)malloc(sizeof(Minuit_fitter_info));
    if (value == NULL)
    {
	fprintf(stderr, "Error in hs_copy_fitter_info: out of memory\n");
	return NULL;
    }
    *value = *p;
    value->refcount = 0;

    value->fullname = strdup(p->fullname);
    value->description = strdup(p->description);
    value->init_fun_name = strdup(p->init_fun_name);
    value->fit_fun_name = strdup(p->fit_fun_name);
    value->grad_fun_name = strdup(p->grad_fun_name);
    value->cleanup_fun_name = strdup(p->cleanup_fun_name);
    if (value->fullname == NULL || value->description == NULL || 
	value->init_fun_name == NULL || value->fit_fun_name == NULL || 
	value->grad_fun_name == NULL || value->cleanup_fun_name == NULL)
    {
	fprintf(stderr, "Error in hs_copy_fitter_info: out of memory\n");
	goto fail0;
    }

    if (p->npars_max > 0)
    {
	assert(p->param_names);
	value->param_names = (char **)calloc(p->npars_max, sizeof(char *));
	if (value->param_names == NULL)
	{
	    fprintf(stderr, "Error in hs_copy_fitter_info: out of memory\n");
	    goto fail0;
	}
	for (i=0; i<p->npars_max; ++i)
	    if (p->param_names[i])
	    {
		value->param_names[i] = strdup(p->param_names[i]);
		if (value->param_names[i] == NULL)
		{
		    fprintf(stderr, "Error in hs_copy_fitter_info: out of memory\n");
		    goto fail0;
		}
	    }
    }

    if (p->ownsdll)
	hs_incr_dll_refcount(p->groupnumber);

    return value;

 fail0:
    hs_destroy_fitter_info(value);
    checkfree(value);
    return NULL;
}

static Minuit_futil(hs_combo_fitfun)
{
    int ierr1 = 0, ierr2 = 0, fmode;
    double val1, val2, result;
    FunctionCombination* combo;

    assert(mode >= 0 && mode < n_combos);
    combo = combo_table + mode;
    assert(combo->type != FITFUN_NONE);

    if (combo->f2->fit_c)
	val2 = (combo->f2->fit_c)(x, y, z, combo->f2->mode, pars + combo->npars1, &ierr2);
    else
    {
	/* Protect the mode argument from change */
	fmode = combo->f2->mode;
	val2 = (combo->f2->fit_f)(&x, &y, &z, &fmode, pars + combo->npars1, &ierr2);
    }
    if (ierr2)
    {
	*ierr = ierr2;
	return 0.0;
    }
    if (combo->type == FITFUN_COMPOSITION)
	x = combo->c1 * val2 + combo->c2;
    if (combo->f1->fit_c)
	val1 = (combo->f1->fit_c)(x, y, z, combo->f1->mode, pars, &ierr1);
    else
    {
	/* Protect the mode argument from change */
	fmode = combo->f1->mode;
	val1 = (combo->f1->fit_f)(&x, &y, &z, &fmode, pars, &ierr1);
    }
    if (ierr1)
    {
	*ierr = ierr1;
	return 0.0;
    }

    switch(combo->type)
    {
    case FITFUN_SUM:
	result = combo->c1 * val1 + combo->c2 * val2;
	break;

    case FITFUN_PRODUCT:
	result = combo->c1 * val1 * val2;
	break;

    case FITFUN_RATIO:
	if (val2 == 0)
	{
	    fprintf(stderr, "division by zero\n");
	    *ierr = 1;
	    return 0.0;
	}
	result = combo->c1 * val1 / val2;
	break;

    case FITFUN_COMPOSITION:
	result = val1;
	break;

    default:
	assert(0);
    }
    return result;
}

static int hs_combo_init(const int *mode)
{
    int status = 0;
    FunctionCombination* combo;

    assert(*mode >= 0 && *mode < n_combos);
    combo = combo_table + *mode;
    assert(combo->type != FITFUN_NONE);

    assert(combo->name1);
    assert(combo->name1[0]);
    if ((combo->f1 = hs_find_fitter_function(combo->name1)) == NULL)
    {
	fprintf(stderr,
		"Can't initialize function combination: component \"%s\" doesn't exist\n",
		combo->name1);
	return 1;
    }
    if (hs_fitter_num_pars(combo->f1) != combo->npars1)
    {
	fprintf(stderr, "Can't initialize function combination: the number of parameters for component\n");
	fprintf(stderr, "\"%s\" has been modified\n", combo->name1);
	return 1;
    }

    assert(combo->name2);
    assert(combo->name2[0]);
    if ((combo->f2 = hs_find_fitter_function(combo->name2)) == NULL)
    {
	fprintf(stderr,
		"Can't initialize function combination: component \"%s\" doesn't exist\n",
		combo->name2);
	return 1;
    }
    if (hs_fitter_num_pars(combo->f2) != combo->npars2)
    {
	fprintf(stderr, "Can't initialize function combination: the number of parameters for component\n");
	fprintf(stderr, "\"%s\" has been modified\n", combo->name2);
	return 1;
    }

    if (!combo_compatible(combo->type, combo->f1, combo->f2))
    {
	fprintf(stderr, "Can't initialize function combination: ");
	fprintf(stderr, "%s of functions \"%s\" and \"%s\" can not be defined\n",
		CombinationTypeNames[combo->type], combo->name1, combo->name2);
    }

    if (combo->f1)
	if (hs_incr_fitter_usage_count(combo->f1))
	{
	    fprintf(stderr, "Failed to initialize function component \"%s\"\n", combo->name1);
	    status = 1;
	}
    if (combo->f2)
	if (hs_incr_fitter_usage_count(combo->f2))
	{
	    fprintf(stderr, "Failed to initialize function component \"%s\"\n", combo->name2);
	    hs_decr_fitter_usage_count(combo->f1);
	    status = 1;
	}
    return status;
}

static int hs_combo_cleanup(const int *mode)
{
    int status = 0;
    FunctionCombination* combo;

    assert(*mode >= 0 && *mode < n_combos);
    combo = combo_table + *mode;
    assert(combo->type != FITFUN_NONE);

    if (combo->f1)
	if (hs_decr_fitter_usage_count(combo->f1))
	    status = 1;
    if (combo->f2)
	if (hs_decr_fitter_usage_count(combo->f2))
	    status = 1;
    return status;
}

static int combo_compatible(CombinationType type,
			    Minuit_fitter_info *f1,
			    Minuit_fitter_info *f2)
{
    if (f1 == NULL || f2 == NULL)
	return 0;
    switch (type)
    {
    case FITFUN_NONE:
	break;
    case FITFUN_SUM:
    case FITFUN_PRODUCT:
    case FITFUN_RATIO:
	return 1;
    case FITFUN_COMPOSITION:
	if (f1->ndim == 1)
	    return 1;
	break;
    default:
	assert(0);
    }
    return 0;
}
