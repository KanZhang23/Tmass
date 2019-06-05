#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <stddef.h>
#include "histoscope.h"
#include "minuit_api.h"
#include "minuit.h"
#include "fit_config.h"
#include "histo_utils.h"

#define checkfree(pointer) do {if (pointer) {free(pointer); pointer=0;}} while(0);

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

#define add_string_to_list(list,string) do {\
    if (string)\
	Tcl_ListObjAppendElement(interp, list, Tcl_NewStringObj(string, -1));\
    else\
	Tcl_ListObjAppendElement(interp, list, Tcl_NewStringObj("", -1));\
} while(0);

static int minuit_is_locked = 0;

static Tcl_HashTable fit_table;
static int n_fits = 0;

static Parameter_map_function sequential_map;
static Parameter_map_function identical_map;
static Parameter_map_function null_map;

typedef struct
{
    Parameter_map_function *fcn;
    char *name;
} Default_mappings;

static const Default_mappings known_mappings[] = {
    {sequential_map, "sequential"},
    {identical_map, "identical"},
    {null_map, "null"}
};

static int is_blank_string(const char *pc)
{
    if (pc == NULL)
	return 1;
    for (; *pc; ++pc)
	if (!isspace(*pc))
	    return 0;
    return 1;
}

void hs_destroy_fit_fitter(Fitter_info *info)
{
    if (info == NULL) return;
    checkfree(info->fitter_tag);
    checkfree(info->mapping);
    checkfree(info->best_pars);
    if (info->fitter)
	hs_decr_fitter_usage_count(info->fitter);
}

void hs_clear_minuit_param(Minuit_parameter *param)
{
    assert(param);
    checkfree(param->name);
}

int hs_copy_minuit_param_inplace(Minuit_parameter *dest,
				 const Minuit_parameter *src)
{
    assert(dest && src);
    *dest = *src;
    check_strdup(dest->name, src->name);
    return 0;

 fail0:
    hs_clear_minuit_param(dest);
    return 1;
}

Fitter_info * hs_find_fit_fitter_byname(Fit_config *config,
					const char *name)
{
    int i;

    assert(config && name);
    if (config->fitters == 0)
	return NULL;
    for (i=0; i<config->nfitters; ++i)
	if (config->fitters[i])
	    if (strcmp(config->fitters[i]->fitter_tag, name) == 0)
		return config->fitters[i];
    return NULL;
}

void hs_synch_fit_subsets(Fit_config *config)
{
    int set, i;
    Fit_subset *fitset;
    char charbuf[32];
    Tcl_Interp *interp;

    assert(config);
    interp = config->interp;
    assert(interp);
    if (config->fitsets)
    {
	for (set=0; set<config->nsets; ++set)
	{
	    fitset = config->fitsets[set];
	    if (fitset == NULL) continue;
	    checkfree(fitset->pfitters);
	    if (fitset->fitter_names)
	    {
		fitset->pfitters = (Fitter_info **)calloc(
		    fitset->nfitters, sizeof(Fitter_info *));
		if (fitset->pfitters == NULL)
		{
		    Tcl_ResetResult(interp);
		    Tcl_SetResult(interp, "out of memory", TCL_VOLATILE);
		    config->status = TCL_ERROR;
		    return;
		}
		for (i=0; i<fitset->nfitters; ++i)
		{
		    fitset->pfitters[i] = hs_find_fit_fitter_byname(
			config, fitset->fitter_names[i]);
		    if (fitset->pfitters[i] == NULL)
		    {
			sprintf(charbuf, "%d", i);
			Tcl_ResetResult(interp);
			Tcl_AppendResult(interp, "invalid fitter function \"",
					 fitset->fitter_names[i],
					 "\" in fit subset ", charbuf, NULL);
			config->status = TCL_ERROR;
			return;
		    }
		}
	    }
	}
    }
}

void hs_destroy_fit_subset(Fit_subset *subset)
{
    int i;

    if (subset == NULL)
	return;
    checkfree(subset->points);
    checkfree(subset->iweights);
    checkfree(subset->filter_string);
    if (subset->fitter_names)
    {
	for (i=0; i<subset->nfitters; ++i)
	    checkfree(subset->fitter_names[i]);
	free(subset->fitter_names);
    }
    checkfree(subset->pfitters);
    checkfree(subset->acc_method_name);
    free(subset);
}

void hs_destroy_fit_config(Fit_config *config)
{
    int i, dll, nelem;
    Tcl_Obj **listObjElem;
    Tcl_Obj *hashData;
    Tcl_HashSearch hashIter;
    Tcl_HashEntry *hashEntry;

    if (config == NULL)
	return;
    /* Make sure we do not trigger any callbacks
       in the "hs_set_current_fit" function */
    if (config->cb_complete)
    {
	Tcl_DecrRefCount(config->cb_complete);
	config->cb_complete = NULL;
    }
    if (config->cb_lostsync)
    {
	Tcl_DecrRefCount(config->cb_lostsync);
	config->cb_lostsync = NULL;
    }
    if (config->cb_delete)
    {
	Tcl_DecrRefCount(config->cb_delete);
	config->cb_delete = NULL;
    }
    if (hs_current_fit_conf() == config)
	hs_set_current_fit(NULL);

    checkfree(config->title);
    checkfree(config->description);
    checkfree(config->default_method);
    if (config->minuit_pars)
    {
	for (i=0; i<config->n_minuit_pars; ++i)
	    hs_clear_minuit_param(config->minuit_pars + i);
	free(config->minuit_pars);
    }
    checkfree(config->emat);
    checkfree(config->ematind);
    if (config->dll_list)
    {
	Tcl_ListObjGetElements(config->interp, config->dll_list,
			       &nelem, &listObjElem);
	for (i=0; i<nelem; ++i)
	{
	    Tcl_GetIntFromObj(config->interp, listObjElem[i], &dll);
	    hs_decr_dll_refcount(dll);
	}
	Tcl_DecrRefCount(config->dll_list);
    }
    if (config->fitsets)
    {
	for (i=0; i<config->nsets; ++i)
	    hs_destroy_fit_subset(config->fitsets[i]);
	free(config->fitsets);
    }
    if (config->fitters)
    {
	for (i=0; i<config->nfitters; ++i)
	    hs_destroy_fit_fitter(config->fitters[i]);
	free(config->fitters);
    }
    for (hashEntry = Tcl_FirstHashEntry(&config->tcldata,&hashIter);
	 hashEntry != NULL; hashEntry = Tcl_NextHashEntry(&hashIter))
    {
	hashData = (Tcl_Obj *)Tcl_GetHashValue(hashEntry);
	Tcl_DecrRefCount(hashData);
    }
    Tcl_DeleteHashTable(&config->tcldata);
    if (config->objv)
    {
        for (i=0; i<config->objc; ++i)
            if (config->objv[i]) {
                Tcl_DecrRefCount(config->objv[i]);
            }
        free(config->objv);
    }
    free(config);
}

Fit_config * hs_copy_fit_config(Fit_config *config)
{
    int i, dll, nelem;
    Fit_config *result = NULL;
    Tcl_Obj **listObjElem;
    Tcl_HashSearch hashIter;
    Tcl_HashEntry *hashEntry, *newEntry;
    char *hashKey;
    Tcl_Obj *hashData;

    assert(config);
    result = (Fit_config *)malloc(sizeof(Fit_config));
    if (result == NULL)
	return NULL;

    /* Copy all simple things */
    *result = *config;

    /* Zero out all pointers to the objects this struct owns
       so that the destructor will not fail if called on an
       incompletely constructed object */
    result->title = NULL;
    result->description = NULL;
    result->default_method = NULL;
    result->minuit_pars = NULL;
    result->emat = NULL;
    result->ematind = NULL;
    result->fitsets = NULL;
    result->fitters = NULL;
    result->cb_complete = NULL;
    result->cb_lostsync = NULL;
    result->cb_delete = NULL;
    result->objv = NULL;
    Tcl_InitHashTable(&result->tcldata, TCL_STRING_KEYS);

    /* The new fit is not synchronized with Minuit */
    result->options_synched = 0;
    result->complete = 0;
    result->par_synched = 0;

    /* Start copying various things lying on the heap */
    check_strdup(result->title, config->title);
    check_strdup(result->description, config->description);
    check_strdup(result->default_method, config->default_method);

    /* copy parameters */
    if (config->minuit_pars)
    {
	result->minuit_pars = (Minuit_parameter *)calloc(
	    config->n_minuit_pars, sizeof(Minuit_parameter));
	if (result->minuit_pars == NULL)
	    goto fail0;
	for (i=0; i<config->n_minuit_pars; ++i)
	    if (hs_copy_minuit_param_inplace(result->minuit_pars+i,
					     config->minuit_pars+i))
		goto fail0;
    }

    /* copy the error matrix */
    if (config->emat)
    {
	assert(config->n_variable_pars > 0);
	i = config->n_variable_pars;
	result->emat = (double *)malloc(i * i * sizeof(double));
	result->ematind = (int *)malloc(i * sizeof(int));
	if (result->emat == NULL || result->ematind == NULL)
	    goto fail0;
	memcpy(result->emat, config->emat, i * i * sizeof(double));
	memcpy(result->ematind, config->ematind, i * sizeof(int));
    }

    /* copy fit subsets */
    if (config->fitsets)
    {
	result->fitsets = (Fit_subset **)calloc(
	    config->nsets, sizeof(Fit_subset *));
	if (result->fitsets == NULL)
	    goto fail0;
	for (i=0; i<config->nsets; ++i)
	{
	    result->fitsets[i] = hs_copy_fit_subset(config->fitsets[i]);
	    if (config->fitsets[i] != NULL && result->fitsets[i] == NULL)
		goto fail0;
	}
    }

    /* copy fitters */
    if (config->fitters)
    {
	result->fitters = (Fitter_info **)calloc(
	    config->nfitters, sizeof(Fitter_info *));
	if (result->fitters == NULL)
	    goto fail0;
	for (i=0; i<config->nfitters; ++i)
	{
	    result->fitters[i] = hs_copy_fit_fitter(config->fitters[i]);
	    if (config->fitters[i] != NULL && result->fitters[i] == NULL)
		goto fail0;
	}
    }

    /* handle dlls to unload */
    if (config->dll_list)
    {
	Tcl_ListObjGetElements(config->interp, config->dll_list,
			       &nelem, &listObjElem);
	for (i=0; i<nelem; ++i)
	{
	    Tcl_GetIntFromObj(config->interp, listObjElem[i], &dll);
	    hs_incr_dll_refcount(dll);
	}
	Tcl_IncrRefCount(config->dll_list);
    }

    /* do not copy the trace scripts, make new ones instead */
    result->cb_complete = Tcl_NewListObj(0, NULL);
    if (result->cb_complete)
	Tcl_IncrRefCount(result->cb_complete);
    else
	goto fail0;
    result->cb_delete = Tcl_NewListObj(0, NULL);
    if (result->cb_delete)
	Tcl_IncrRefCount(result->cb_delete);
    else
	goto fail0;
    result->cb_lostsync = Tcl_NewListObj(0, NULL);
    if (result->cb_lostsync)
	Tcl_IncrRefCount(result->cb_lostsync);
    else
	goto fail0;

    /* Copy the tcl data */
    for (hashEntry = Tcl_FirstHashEntry(&config->tcldata,&hashIter);
	 hashEntry != NULL; hashEntry = Tcl_NextHashEntry(&hashIter))
    {
	hashKey = Tcl_GetHashKey(&config->tcldata, hashEntry);
	hashData = (Tcl_Obj *)Tcl_GetHashValue(hashEntry);
	newEntry = Tcl_CreateHashEntry(&result->tcldata, hashKey, &i);
	if (newEntry == NULL)
	    goto fail0;
	Tcl_SetHashValue(newEntry, hashData);
	Tcl_IncrRefCount(hashData);
    }

    if (config->objv)
    {
        result->objv = (Tcl_Obj **)malloc(config->objc*sizeof(Tcl_Obj *));
        if (result->objv == NULL)
            goto fail0;
        memcpy(result->objv, config->objv, config->objc*sizeof(Tcl_Obj *));
        for (i=0; i<config->objc; ++i)
            Tcl_IncrRefCount(result->objv[i]);
    }

    return result;

 fail0:
    hs_destroy_fit_config(result);
    return NULL;
}

void hs_exe_tcl_callbacks(Tcl_Interp *interp, Tcl_Obj *callbacks,
			  const char *name, const char *type)
{
    int i, nelem, status;
    Tcl_Obj **listObjElem;
    Tcl_DString script;

    assert(interp);
    if (callbacks == NULL)
	return;

    /* Must copy callbacks locally because they
       may change themselves while executed */
    callbacks = Tcl_DuplicateObj(callbacks);
    if (callbacks == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_VOLATILE);
	Tcl_BackgroundError(interp);
	return;
    }
    Tcl_IncrRefCount(callbacks);
    if (Tcl_ListObjGetElements(interp, callbacks,
			&nelem, &listObjElem) != TCL_OK)
	Tcl_BackgroundError(interp);
    else
    {
	for (i=0; i<nelem; ++i)
	{
	    Tcl_DStringInit(&script);
	    Tcl_DStringAppend(&script, Tcl_GetStringFromObj(
		listObjElem[i],NULL), -1);
	    if (name)
		Tcl_DStringAppendElement(&script, name);
	    else
		Tcl_DStringAppendElement(&script, "");
	    if (type)
		Tcl_DStringAppendElement(&script, type);
	    else
		Tcl_DStringAppendElement(&script, "");
	    status = Tcl_EvalEx(interp, Tcl_DStringValue(&script),
				-1, TCL_EVAL_GLOBAL | TCL_EVAL_DIRECT);
	    Tcl_DStringFree(&script);
	    if (status != TCL_OK && status != TCL_RETURN)
	    {
		Tcl_BackgroundError(interp);
		break;
	    }
	}
    }
    Tcl_DecrRefCount(callbacks);
}

void hs_declare_fit_complete(Fit_config *config, int iscomplete)
{
    assert(config);
    iscomplete = !(!iscomplete);
    if (iscomplete != config->complete)
    {
	config->complete = iscomplete;
	/* Note the following assumption... Have to do this
	   because otherwise the fit tag is unknown. */
	assert(config == hs_current_fit_conf());
	if (iscomplete)
	    hs_exe_tcl_callbacks(config->interp, config->cb_complete,
				 hs_current_fit_name(), CB_FIT_COMPLETE);
	else
	    hs_exe_tcl_callbacks(config->interp, config->cb_lostsync,
				 hs_current_fit_name(), CB_FIT_LOSTSYNC);
    }
}

Fitter_info * hs_create_simple_fitter(const char *name)
{
    Fitter_info *newfitter = NULL;
    Minuit_fitter_info *fptr;
    const char *seq_map_name = "sequential";
    const char *null_map_name = "null";

    assert(name);
    fptr = hs_find_fitter_function(name);
    if (fptr == NULL)
	return NULL;
    newfitter = (Fitter_info *)calloc(1, sizeof(Fitter_info));
    if (newfitter == NULL)
	return NULL;
    check_strdup(newfitter->fitter_tag, name);
    newfitter->fitter = fptr;
    hs_incr_fitter_usage_count(fptr);
    newfitter->npars = hs_fitter_num_pars(fptr);
    if (newfitter->npars > 0)
    {
	newfitter->best_pars = (double *)calloc(newfitter->npars, sizeof(double));
	check_strdup(newfitter->mapping, seq_map_name);
	newfitter->map = hs_default_mapping_function(seq_map_name);
    }
    else
    {
	check_strdup(newfitter->mapping, null_map_name);
	newfitter->map = hs_default_mapping_function(null_map_name);
    }
    return newfitter;

 fail0:
    hs_destroy_fit_fitter(newfitter);
    return NULL;
}

Fitter_info * hs_copy_fit_fitter(const Fitter_info *src)
{
    Fitter_info *dest = NULL;
    
    if (src == NULL)
	return NULL;
    dest = (Fitter_info *)calloc(1, sizeof(Fitter_info));
    if (dest == NULL)
	return NULL;
    *dest = *src;
    hs_incr_fitter_usage_count(dest->fitter);
    check_strdup(dest->fitter_tag, src->fitter_tag);
    check_strdup(dest->mapping, src->mapping);
    if (src->best_pars)
    {
	dest->best_pars = (double *)malloc(dest->npars*sizeof(double));
	if (dest->best_pars == NULL)
	    goto fail0;
	memcpy(dest->best_pars, src->best_pars, dest->npars*sizeof(double));
    }
    return dest;

 fail0:
    hs_destroy_fit_fitter(dest);
    return NULL;
}

Fit_subset * hs_copy_fit_subset(Fit_subset *subset)
{
    int i;
    Fit_subset *result = NULL;
    
    if (subset == NULL)
	return NULL;
    result = (Fit_subset *)malloc(sizeof(Fit_subset));
    if (result == NULL)
	return NULL;
    *result = *subset;

    /* Zero out all pointers to the heap objects so that
       the destructor will be able to destroy a partially
       constructed object without problems */
    result->filter_string = NULL;
    result->fitter_names = NULL;
    result->pfitters = NULL;
    result->points = NULL;
    result->iweights = NULL;
    result->acc_method_name = NULL;
    result->nweights = 0;
    result->npoints = 0;

    /* start copying the heap objects */
    check_strdup(result->filter_string, subset->filter_string);
    check_strdup(result->acc_method_name, subset->acc_method_name);

    /* copy fitter names */
    if (subset->fitter_names)
    {
	result->fitter_names = (char **)calloc(
	    subset->nfitters, sizeof(char *));
	if (result->fitter_names == NULL)
	    goto fail0;
	for (i=0; i<subset->nfitters; ++i)
	    check_strdup(result->fitter_names[i], subset->fitter_names[i]);
    }

    return result;

 fail0:
    hs_destroy_fit_subset(result);
    return NULL;
}

Fit_config * hs_find_fit_byname(const char *name)
{
    Tcl_HashEntry *entryPtr;

    assert(name);
    if (n_fits == 0)
	return NULL;
    entryPtr = Tcl_FindHashEntry(&fit_table, name);
    if (entryPtr == NULL)
	return NULL;
    return (Fit_config *)Tcl_GetHashValue(entryPtr);
}

int hs_remove_fit_byname(const char *name)
{
    Tcl_HashEntry *entryPtr;
    Fit_config *fit;

    assert(name);
    if (n_fits == 0)
	return 1;
    entryPtr = Tcl_FindHashEntry(&fit_table, name);
    if (entryPtr == NULL)
	return 1;
    fit = (Fit_config *)Tcl_GetHashValue(entryPtr);
    if (hs_current_fit_conf() == fit)
	hs_set_current_fit(NULL);    
    hs_exe_tcl_callbacks(fit->interp, fit->cb_delete, name, CB_FIT_DELETE);
    hs_destroy_fit_config(fit);
    Tcl_DeleteHashEntry(entryPtr);
    --n_fits;
    return 0;
}

int hs_add_fit(Tcl_Interp *interp, const char *name, Fit_config *config)
{
    Tcl_HashEntry *entryPtr;
    int entry_created;

    assert(interp && name && config);
    if (name[0] == 0)
    {
	Tcl_SetResult(interp, "fit name can not be an empty string", TCL_STATIC);
	return TCL_ERROR;
    }

    if (n_fits == 0)
	Tcl_InitHashTable(&fit_table, TCL_STRING_KEYS);
    entryPtr = Tcl_CreateHashEntry(&fit_table, name, &entry_created);
    if (!entry_created)
    {
	Tcl_AppendResult(interp, "fit named \"", name, "\" already exists", NULL);
	return TCL_ERROR;
    }
    Tcl_SetHashValue(entryPtr, config);
    ++n_fits;

    return TCL_OK;
}

void hs_reset_current_fit(void)
{
    Fit_config *config;

    config = hs_current_fit_conf();
    if (config == NULL)
	return;
    config->status = TCL_OK;
    config->par_synched = 0;
    config->options_synched = 0;
    hs_declare_fit_complete(config, 0);
}

int hs_copy_fit(Tcl_Interp *interp, const char *newname,
		const char *oldname)
{
    Fit_config *oldfit, *newfit;

    assert(interp);
    assert(newname);
    assert(oldname);

    oldfit = hs_find_fit_byname(oldname);
    if (oldfit == NULL)
    {
	Tcl_AppendResult(interp, "fit named \"",
			 oldname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    
    newfit = hs_copy_fit_config(oldfit);
    if (newfit == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }

    if (hs_add_fit(interp, newname, newfit) != TCL_OK)
    {
	hs_destroy_fit_config(newfit);
	return TCL_ERROR;
    }
    return TCL_OK;
}

int hs_rename_fit(Tcl_Interp *interp, const char *newname,
		  const char *oldname)
{
    Tcl_HashEntry *entryPtr;
    Fit_config *oldfit;
    int status;

    assert(interp);
    assert(newname);
    assert(oldname);

    /* Find the relevant hash entry */
    if (n_fits == 0)
    {
	Tcl_AppendResult(interp, "fit named \"",
			 oldname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    entryPtr = Tcl_FindHashEntry(&fit_table, oldname);
    if (entryPtr == NULL)
    {
	Tcl_AppendResult(interp, "fit named \"",
			 oldname, "\" does not exist", NULL);
	return TCL_ERROR;
    }
    oldfit = (Fit_config *)Tcl_GetHashValue(entryPtr);
    if (newname[0] == '\0')
    {
	/* There should be no reason by which this fit can not be deleted */
	status = hs_remove_fit_byname(oldname);
	assert(status == 0);
	return TCL_OK;
    }
    /* Check that the fit with new name doesn't exists */
    if (hs_find_fit_byname(newname))
    {
	Tcl_AppendResult(interp, "fit named \"",
			 newname, "\" already exists", NULL);
	return TCL_ERROR;
    }
    /* Remove the old hash table entry */
    Tcl_DeleteHashEntry(entryPtr);
    --n_fits;
    /* Reinsert the function into the table */
    if (hs_add_fit(interp, newname, oldfit) != TCL_OK)
	return TCL_ERROR;
    if (oldfit == hs_current_fit_conf())
    {
	if (hs_reassign_current_fit_name(newname))
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

int hs_fit_subset_description(Tcl_Interp *interp,
			      const Fit_subset *fitset,
			      Tcl_Obj **descr)
{
    int ifun;
    Tcl_Obj *listPtr, *fitterList, *columnMapping, *region;

    assert(interp && fitset && descr);

    listPtr = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fitset->id));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fitset->ndim));
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fitset->binned));
    columnMapping = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, columnMapping, Tcl_NewIntObj(fitset->colx));
    Tcl_ListObjAppendElement(interp, columnMapping, Tcl_NewIntObj(fitset->coly));
    Tcl_ListObjAppendElement(interp, columnMapping, Tcl_NewIntObj(fitset->colz));
    Tcl_ListObjAppendElement(interp, columnMapping, Tcl_NewIntObj(fitset->colval));
    Tcl_ListObjAppendElement(interp, columnMapping, Tcl_NewIntObj(fitset->colerr));
    Tcl_ListObjAppendElement(interp, listPtr, columnMapping);
    add_string_to_list(listPtr, fitset->filter_string);
    fitterList = Tcl_NewListObj(0, NULL);
    for (ifun=0; ifun < fitset->nfitters; ++ifun)
	Tcl_ListObjAppendElement(interp, fitterList, Tcl_NewStringObj(
	    fitset->fitter_names[ifun], -1));
    Tcl_ListObjAppendElement(interp, listPtr, fitterList);
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(fitset->weight));
    add_string_to_list(listPtr, fitset->acc_method_name);
    Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(fitset->datapoints));
    region = Tcl_NewListObj(0, NULL);
    if (!fitset->binned)
	if (fitset->nx > 0)
	{
	    Tcl_ListObjAppendElement(interp, region,
		Tcl_NewDoubleObj(fitset->xmin));
	    Tcl_ListObjAppendElement(interp, region,
		Tcl_NewDoubleObj(fitset->xmax));
	    Tcl_ListObjAppendElement(interp, region,
		Tcl_NewIntObj(fitset->nx));
	    if (fitset->ndim > 1)
	    {
		Tcl_ListObjAppendElement(interp, region,
		    Tcl_NewDoubleObj(fitset->ymin));
		Tcl_ListObjAppendElement(interp, region,
		    Tcl_NewDoubleObj(fitset->ymax));
		Tcl_ListObjAppendElement(interp, region,
		    Tcl_NewIntObj(fitset->ny));
	    }
	    if (fitset->ndim > 2)
	    {
		Tcl_ListObjAppendElement(interp, region,
		    Tcl_NewDoubleObj(fitset->zmin));
		Tcl_ListObjAppendElement(interp, region,
		    Tcl_NewDoubleObj(fitset->zmax));
		Tcl_ListObjAppendElement(interp, region,
		    Tcl_NewIntObj(fitset->nz));
	    }
	}
    Tcl_ListObjAppendElement(interp, listPtr, region);

    *descr = listPtr;
    return TCL_OK;
}

int hs_fitter_info_description(Tcl_Interp *interp,
			       const Fit_config *fit,
			       const Fitter_info *info,
			       Tcl_Obj **descr)
{
    int i, ipar;
    Tcl_Obj *fitterInfo, *parList, *paramPair, *resultList;

    assert(interp && info && descr);
    fitterInfo = Tcl_NewListObj(0, NULL);
    add_string_to_list(fitterInfo, info->fitter_tag);
    add_string_to_list(fitterInfo, info->mapping);
    Tcl_ListObjAppendElement(interp, fitterInfo,
			     Tcl_NewIntObj(info->offset));
    /* Subsets */
    resultList = Tcl_NewListObj(0, NULL);
    for (i=0; i<fit->nsets; ++i)
	if (hs_fitter_exists_in_subset(fit->fitsets[i], info->fitter_tag))
	    Tcl_ListObjAppendElement(interp, resultList, Tcl_NewIntObj(i));
    Tcl_ListObjAppendElement(interp, fitterInfo, resultList);

    /* Parameters */
    parList = Tcl_NewListObj(0, NULL);
    for (ipar=0; ipar<info->npars; ++ipar)
    {
	paramPair = Tcl_NewListObj(0, NULL);
	Tcl_ListObjAppendElement(
	    interp, paramPair,
	    Tcl_NewStringObj(info->fitter->param_names[ipar], -1));
	Tcl_ListObjAppendElement(
	    interp, paramPair,
	    Tcl_NewDoubleObj(info->best_pars[ipar]));
	Tcl_ListObjAppendElement(interp, parList, paramPair);
    }
    Tcl_ListObjAppendElement(interp, fitterInfo, parList);

    *descr = fitterInfo;
    return TCL_OK;
}

int hs_minuit_param_description(Tcl_Interp *interp,
				const Minuit_parameter *param,
				int includeName,
				Tcl_Obj **descr)
{
    Tcl_Obj *paramInfo;
    Tcl_Obj *bounds;
    
    assert(interp);
    assert(param);
    assert(descr);

    paramInfo = Tcl_NewListObj(0, NULL);
    if (includeName)
	add_string_to_list(paramInfo, param->name);
    Tcl_ListObjAppendElement(interp, paramInfo, Tcl_NewDoubleObj(param->value));
    if (param->fixed)
	Tcl_ListObjAppendElement(interp, paramInfo, Tcl_NewStringObj("fixed", -1));
    else
	Tcl_ListObjAppendElement(interp, paramInfo, Tcl_NewStringObj("variable", -1));
    Tcl_ListObjAppendElement(interp, paramInfo, Tcl_NewDoubleObj(param->step));
    bounds = Tcl_NewListObj(0, NULL);
    if (param->has_bounds)
    {
	Tcl_ListObjAppendElement(interp, bounds, Tcl_NewDoubleObj(param->lolim));
	Tcl_ListObjAppendElement(interp, bounds, Tcl_NewDoubleObj(param->hilim));
    }
    Tcl_ListObjAppendElement(interp, paramInfo, bounds);

    *descr = paramInfo;
    return TCL_OK;
}

int hs_minim_status_description(Tcl_Interp *interp,
				const Minimization_status *ps,
				Tcl_Obj **descr)
{
    Tcl_Obj *minInfo;

    assert(interp);
    assert(ps);
    assert(descr);

    minInfo = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, minInfo,
			     Tcl_NewDoubleObj(ps->fmin));
    Tcl_ListObjAppendElement(interp, minInfo,
			     Tcl_NewDoubleObj(ps->fedm));
    Tcl_ListObjAppendElement(interp, minInfo,
			     Tcl_NewDoubleObj(ps->errdef));
    Tcl_ListObjAppendElement(interp, minInfo,
			     Tcl_NewIntObj(ps->npari));
    Tcl_ListObjAppendElement(interp, minInfo,
			     Tcl_NewIntObj(ps->nparx));
    Tcl_ListObjAppendElement(interp, minInfo,
			     Tcl_NewIntObj(ps->istat));

    *descr = minInfo;
    return TCL_OK;
}

int hs_fit_description(Tcl_Interp *interp,
		       const Fit_config *config,
		       Tcl_Obj **descr)
{
    int i, j;
    Tcl_Obj *fitInfo, *fitParams, *paramInfo, *minInfo, *fitOptions;
    Tcl_Obj *subsetInfo, *subsetList, *fitterInfo, *fitterList;
    Tcl_Obj *ematParNames, *ematData, *ematColumn;

    assert(interp);
    assert(config);
    assert(descr);

    fitInfo = Tcl_NewListObj(0, NULL);
    add_string_to_list(fitInfo, config->title);
    add_string_to_list(fitInfo, config->description);
    add_string_to_list(fitInfo, config->default_method);
    add_string_to_list(fitInfo, config->minimizer);
    Tcl_ListObjAppendElement(interp, fitInfo, Tcl_NewBooleanObj(!config->nominos));

    fitOptions = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, fitOptions, Tcl_NewDoubleObj(config->errdef));
    Tcl_ListObjAppendElement(interp, fitOptions, Tcl_NewIntObj(config->verbose));
    Tcl_ListObjAppendElement(interp, fitOptions, Tcl_NewBooleanObj(!config->nowarnings));
    Tcl_ListObjAppendElement(interp, fitOptions, Tcl_NewIntObj(config->strategy));
    Tcl_ListObjAppendElement(interp, fitOptions, Tcl_NewDoubleObj(config->eps));
    Tcl_ListObjAppendElement(interp, fitOptions, Tcl_NewIntObj(config->has_grad));
    Tcl_ListObjAppendElement(interp, fitInfo, fitOptions);

    if (config->status == TCL_OK)
	Tcl_ListObjAppendElement(interp, fitInfo, Tcl_NewStringObj("ok",-1));
    else
	Tcl_ListObjAppendElement(interp, fitInfo, Tcl_NewStringObj("error",-1));
    Tcl_ListObjAppendElement(interp, fitInfo, Tcl_NewIntObj(config->timeout));
    Tcl_ListObjAppendElement(interp, fitInfo,
	Tcl_NewBooleanObj(config->ignore_function_errors));
    if (config->complete)
    {
	if (hs_minim_status_description(
	    interp, &config->ministat, &minInfo) != TCL_OK)
	    return TCL_ERROR;
    }
    else
	minInfo = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, fitInfo, minInfo);

    /* MINUIT parameters */
    fitParams = Tcl_NewListObj(0, NULL);
    if (config->minuit_pars)
    {
	for (i=0; i<config->n_minuit_pars; ++i)
	{
	    if (hs_minuit_param_description(
		interp, config->minuit_pars+i, 1, &paramInfo) != TCL_OK)
		return TCL_ERROR;
	    Tcl_ListObjAppendElement(interp, fitParams, paramInfo);
	}
    }
    Tcl_ListObjAppendElement(interp, fitInfo, fitParams);

    /* MINUIT error matrix info */
    ematParNames = Tcl_NewListObj(0, NULL);
    for (i=0; i<config->n_variable_pars; ++i)
	Tcl_ListObjAppendElement(interp, ematParNames, Tcl_NewStringObj(
	    config->minuit_pars[config->ematind[i]].name, -1));
    Tcl_ListObjAppendElement(interp, fitInfo, ematParNames);
    
    ematData = Tcl_NewListObj(0, NULL);
    if (config->emat)
    {
	assert(config->n_variable_pars > 0);
	for (i=0; i<config->n_variable_pars; ++i)
	{
	    ematColumn = Tcl_NewListObj(0, NULL);
	    for (j=0; j<config->n_variable_pars; ++j)
		Tcl_ListObjAppendElement(interp, ematColumn, Tcl_NewDoubleObj(
		    config->emat[i*config->n_variable_pars + j]));
	    Tcl_ListObjAppendElement(interp, ematData, ematColumn);
	}
    }
    Tcl_ListObjAppendElement(interp, fitInfo, ematData);

    /* List of associated DLLs */
    Tcl_ListObjAppendElement(interp, fitInfo, config->dll_list);

    /* fit subsets */
    subsetList = Tcl_NewListObj(0, NULL);
    if (config->fitsets)
    {
	for (i=0; i<config->nsets; ++i)
	{
	    if (config->fitsets[i])
	    {
		if (hs_fit_subset_description(
		    interp, config->fitsets[i], &subsetInfo) != TCL_OK)
		    return TCL_ERROR;
	    }
	    else
		subsetInfo = Tcl_NewListObj(0, NULL);
	    Tcl_ListObjAppendElement(interp, subsetList, subsetInfo);
	}
    }
    Tcl_ListObjAppendElement(interp, fitInfo, subsetList);

    /* fitter functions */
    fitterList = Tcl_NewListObj(0, NULL);
    if (config->fitters)
    {
	for (i=0; i<config->nfitters; ++i)
	{
	    if (config->fitters[i])
	    {
		if (hs_fitter_info_description(
		    interp, config, config->fitters[i], &fitterInfo) != TCL_OK)
		    return TCL_ERROR;
	    }
	    else
		fitterInfo = Tcl_NewListObj(0, NULL);
	    Tcl_ListObjAppendElement(interp, fitterList, fitterInfo);
	}
    }
    Tcl_ListObjAppendElement(interp, fitInfo, fitterList);

    *descr = fitInfo;
    return TCL_OK;
}

int hs_fit_list(Tcl_Interp *interp)
{
    Tcl_HashEntry *entryPtr;
    Tcl_HashSearch hashIterator;
    Tcl_Obj **result;
    int count = 0;

    if (n_fits == 0)
    {
	Tcl_SetObjResult(interp, Tcl_NewListObj(0, NULL));
	return TCL_OK;
    }

    result = (Tcl_Obj **)malloc(n_fits * sizeof(Tcl_Obj *));
    if (result == 0)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    
    entryPtr = Tcl_FirstHashEntry(&fit_table, &hashIterator);
    while (entryPtr)
    {
	result[count++] = Tcl_NewStringObj(Tcl_GetHashKey(&fit_table, entryPtr), -1);
	entryPtr = Tcl_NextHashEntry(&hashIterator);
    }

    Tcl_SetObjResult(interp, Tcl_NewListObj(count, result));
    free(result);
    return TCL_OK;
}

int hs_set_fit_parameter(Tcl_Interp *interp,
			 Fit_config *fit,
			 const Minuit_parameter *param)
{
    int parnum, status;
    Minuit_parameter *newpars = NULL;

    for (parnum=0; parnum<fit->n_minuit_pars; ++parnum)
	if (strcmp(param->name, fit->minuit_pars[parnum].name) == 0)
	    break;
    if (parnum == fit->n_minuit_pars)
    {
	/* This is a new parameter */
	if (!hs_is_valid_c_name(interp, param->name, "parameter name"))
	    return TCL_ERROR;
	newpars = (Minuit_parameter *)calloc((fit->n_minuit_pars+1),
					     sizeof(Minuit_parameter));
	if (newpars == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	if (fit->minuit_pars)
	{
	    memcpy(newpars, fit->minuit_pars,
		   fit->n_minuit_pars*sizeof(Minuit_parameter));
	    free(fit->minuit_pars);
	}
	fit->minuit_pars = newpars;
	fit->n_minuit_pars++;
	fit->par_synched = 0;
    }
    hs_clear_minuit_param(fit->minuit_pars + parnum);
    status = hs_copy_minuit_param_inplace(fit->minuit_pars+parnum, param);
    assert(status == 0);
    fit->par_synched = 0;
    hs_declare_fit_complete(fit, 0);
    return TCL_OK;
}

int hs_add_fitter_to_config(Fit_config *fit, Fitter_info *fitter)
{
    Fitter_info **newfitters;
    int i;

    assert(fit);
    assert(fitter);

    if (hs_find_fit_fitter_byname(fit, fitter->fitter_tag) != NULL)
	return 0;
    newfitters = (Fitter_info **)malloc((fit->nfitters+1) * sizeof(Fitter_info *));
    if (newfitters == NULL)
	return 1;
    for (i=0; i<fit->nfitters; ++i)
	newfitters[i] = fit->fitters[i];
    newfitters[fit->nfitters] = fitter;
    if (fit->fitters) free(fit->fitters);
    fit->fitters = newfitters;
    if (fit->nfitters > 0)
	fitter->offset = fit->fitters[fit->nfitters-1]->offset +
	    fit->fitters[fit->nfitters-1]->npars;
    fit->nfitters++;
    hs_declare_fit_complete(fit, 0);
    return 0;
}

int hs_add_fitter_to_subset(Fit_subset *subset, const char *name)
{
    int i;
    char **newnames;
    
    assert(subset);
    assert(name);

    /* Check if this function is already in */
    if (subset->fitter_names)
	for (i=0; i<subset->nfitters; ++i)
	    if (strcmp(name, subset->fitter_names[i]) == 0)
		return 0;
    newnames = (char **)calloc(subset->nfitters+1, sizeof(char *));
    if (newnames == NULL)
	return 1;
    if (subset->fitter_names)
	for (i=0; i<subset->nfitters; ++i)
	    newnames[i] = subset->fitter_names[i];
    newnames[subset->nfitters] = strdup(name);
    if (newnames[subset->nfitters] == NULL)
    {
	free(newnames);
	return 1;
    }
    checkfree(subset->fitter_names);
    subset->fitter_names = newnames;
    subset->nfitters++;
    checkfree(subset->pfitters);
    return 0;
}

int hs_fit_fitter_usage_count(Fit_config *config,
			      const char *name)
{
    int i, usage_count = 0;
    
    assert(config && name);
    for (i=0; i<config->nsets; ++i)
	usage_count += hs_fitter_exists_in_subset(config->fitsets[i], name);
    return usage_count;
}

int hs_fitter_exists_in_subset(Fit_subset *subset,
			       const char *name)
{
    int i;
    
    if (subset == NULL)
	return 0;
    assert(name);
    if (subset->fitter_names)
	for (i=0; i<subset->nfitters; ++i)
	    if (strcmp(name, subset->fitter_names[i]) == 0)
		return 1;
    return 0;
}

void hs_remove_fitter_from_config(Fit_config *config,
				  const char *name)
{
    int i, offset;

    assert(config && name);
    if (config->fitters)
    {
	for (i=0; i<config->nfitters; ++i)
	    if (strcmp(config->fitters[i]->fitter_tag, name) == 0)
	    {
		hs_destroy_fit_fitter(config->fitters[i]);
		if (i+1 < config->nfitters)
		    memmove(config->fitters+i, config->fitters+i+1,
			    (config->nfitters-i-1)*sizeof(Fitter_info *));
		config->nfitters--;
		break;
	    }
	if (i < config->nfitters)
	{
	    if (config->nfitters == 0)
	    {
		checkfree(config->fitters);
	    }
	    else
	    {
		/* Recalculate the parameter offsets */
		offset = 0;
		for (i=0; i<config->nfitters; ++i)
		{
		    config->fitters[i]->offset = offset;
		    offset += config->fitters[i]->npars;
		}
	    }
	    hs_declare_fit_complete(config, 0);
	}
    }
}

void hs_remove_fitter_from_subset(Fit_subset *subset,
				  const char *name)
{
    int i;
    
    if (subset == NULL)
	return;
    assert(name);
    if (subset->fitter_names)
    {
	for (i=0; i<subset->nfitters; ++i)
	{
	    if (strcmp(name, subset->fitter_names[i]) == 0)
	    {
		checkfree(subset->fitter_names[i]);
		if (i+1 < subset->nfitters)
		    memmove(subset->fitter_names + i,
			    subset->fitter_names + i + 1,
			    (subset->nfitters - i - 1)*sizeof(Fitter_info *));
		subset->nfitters--;
		if (subset->nfitters == 0)
		    checkfree(subset->fitter_names);
		checkfree(subset->pfitters);
		return;
	    }
	}
    }
}

void hs_delete_fit_parameter(Fit_config *fit, int parnum)
{
    assert(fit);
    if (parnum < 0 || parnum >= fit->n_minuit_pars)
	return;
    hs_clear_minuit_param(fit->minuit_pars+parnum);
    memmove(fit->minuit_pars+parnum, fit->minuit_pars+parnum+1,
	    (fit->n_minuit_pars-parnum-1)*sizeof(Minuit_parameter));
    fit->n_minuit_pars--;
    fit->par_synched = 0;
    hs_declare_fit_complete(fit, 0);
}

int hs_set_fit_parameter_tcl(Tcl_Interp *interp,
			     Fit_config *fit_config,
			     char *name, Tcl_Obj *obj)
{
    int i, listlen, nelem, parnum = -1;
    Tcl_Obj **paramElems, **listObjElem;
    Minuit_parameter param;
    char *str;

    assert(interp);
    assert(fit_config);
    assert(obj);
    memset(&param, 0, sizeof(Minuit_parameter));

    if (Tcl_ListObjGetElements(interp, obj,
			       &listlen, &paramElems) != TCL_OK)
	goto fail0;

    /* Check if parameter already exists */
    for (i=0; i<fit_config->n_minuit_pars; ++i)
	if (strcmp(fit_config->minuit_pars[i].name, name) == 0)
	{
	    parnum = i;
	    break;
	}
    if (parnum >= 0)
    {
	if (listlen != 1 && listlen != 4)
	{
	    Tcl_AppendResult(interp, "bad parameter description \"",
			     Tcl_GetStringFromObj(obj, NULL),
			     "\": must be either a double "
			     "value or a four-element list "
			     "{value state step bounds}", NULL);
	    goto fail0;
	}
	hs_copy_minuit_param_inplace(&param, fit_config->minuit_pars + parnum);
    }
    else
    {
	if (listlen != 4)
	{
	    Tcl_AppendResult(interp, "bad new parameter description \"",
			     Tcl_GetStringFromObj(obj, NULL),
			     "\": must be a four-element list "
			     "{value state step bounds}", NULL);
	    goto fail0;
	}
	check_strdup(param.name, name);
    }

    /* Get the new value */
    if (Tcl_GetDoubleFromObj(interp, paramElems[0], &param.value) != TCL_OK)
	goto fail0;

    if (listlen == 4)
    {
	str = Tcl_GetStringFromObj(paramElems[1], NULL);
	if (strcmp(str, "fixed") == 0)
	    param.fixed = 1;
	else if (strcmp(str, "variable") == 0)
	    param.fixed = 0;
	else
	{
	    Tcl_AppendResult(interp, "invalid parameter state \"", str,
			     "\", should be \"variable\" or \"fixed\".", NULL);
	    goto fail0;
	}

	if (Tcl_GetDoubleFromObj(interp, paramElems[2], &param.step) != TCL_OK)
	    goto fail0;
	if (param.step <= 0.0)
	{
	    Tcl_SetResult(interp,
			  "parameter step size must be positive",
			  TCL_STATIC);
	    goto fail0;
	}

	if (Tcl_ListObjGetElements(interp, paramElems[3],
				   &nelem, &listObjElem) != TCL_OK)
	    goto fail0;
	if (nelem == 0)
	    param.has_bounds = 0;
	else if (nelem == 2)
	{
	    param.has_bounds = 1;
	    if (Tcl_GetDoubleFromObj(interp, listObjElem[0], &param.lolim) != TCL_OK)
		goto fail0;
	    if (Tcl_GetDoubleFromObj(interp, listObjElem[1], &param.hilim) != TCL_OK)
		goto fail0;
	    if (param.lolim >= param.hilim)
	    {
		Tcl_AppendResult(
		    interp, "invalid parameter bounds \"",
		    Tcl_GetStringFromObj(paramElems[3],NULL),
		    "\": the lower bound should be less than the upper bound", NULL);
		goto fail0;
	    }
	}
	else
	{
	    Tcl_AppendResult(interp, "invalid parameter bounds \"",
			     Tcl_GetStringFromObj(paramElems[3],NULL), "\"", NULL);
	    goto fail0;
	}
    }
    
    i = hs_set_fit_parameter(interp, fit_config, &param);
    hs_clear_minuit_param(&param);
    return i;

 fail0:
    hs_clear_minuit_param(&param);
    return TCL_ERROR;
}

int hs_add_fit_subset_tcl(Tcl_Interp *interp,
			  Fit_config *fit_config,
			  Tcl_Obj *obj)
{
    int i, id, listlen, ivar, pair, npairs, item_type;
    int *pcol;
    char *sw, *varname, *method;
    Tcl_Obj **datasetElems;
    Fit_subset *newset = NULL;
    Fit_subset **fitsets;

    assert(interp && fit_config && obj);
    if (Tcl_ListObjGetElements(interp, obj,
	       &listlen, &datasetElems) != TCL_OK)
	return TCL_ERROR;
    if (listlen > 0)
    {
	if (listlen % 2 != 1)
	{
	    Tcl_AppendResult(interp, "bad dataset specification \"",
			     Tcl_GetStringFromObj(obj,NULL),
			     "\": wrong number of elements", NULL);
	    goto fail0;
	}

	/* Check that dataset specifies a valid Histo-Scope item */
	if (Tcl_GetIntFromObj(interp, datasetElems[0], &id) != TCL_OK)
	    return TCL_ERROR;
	item_type = hs_type(id);
	if (item_type != HS_1D_HISTOGRAM &&
	    item_type != HS_2D_HISTOGRAM &&
	    item_type != HS_3D_HISTOGRAM &&
	    item_type != HS_NTUPLE)
	{
	    if (item_type == HS_NONE)
		Tcl_AppendResult(interp, Tcl_GetStringFromObj(datasetElems[0], NULL),
				 " is not a valid Histo-Scope id", NULL);
	    else
		Tcl_AppendResult(interp, "Histo-Scope item with id ",
				 Tcl_GetStringFromObj(datasetElems[0], NULL),
				 " is neither histogram nor ntuple", NULL);
	    goto fail0;
	}

	/* Create a new Fit_subset and fill out default settings */
	newset = (Fit_subset *)calloc(1, sizeof(Fit_subset));
	if (newset == NULL)
	{
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
	newset->id = id;
	newset->colx = -1;
	newset->coly = -1;
	newset->colz = -1;
	newset->colval = -1;
	newset->colerr = -1;
	newset->weight = 1.0;
	assert(fit_config->default_method);
	check_strdup(newset->acc_method_name, fit_config->default_method);
	assert(fit_config->default_fit_method);
	newset->acc_method = fit_config->default_fit_method;

	/* Fill out the settings based on item type */
	switch (item_type)
	{
	case HS_1D_HISTOGRAM:
	    newset->ndim = 1;
	    newset->binned = 1;
	    break;
	    
	case HS_2D_HISTOGRAM:
	    newset->ndim = 2;
	    newset->binned = 1;
	    break;
	    
	case HS_3D_HISTOGRAM:
	    newset->ndim = 3;
	    newset->binned = 1;
	    break;
	    
	case HS_NTUPLE:
	    /* May be binned or unbinned, and dimensionality
	       is unknown at this point */
	    if (hs_num_variables(id) == 1)
		newset->colx = 0;
	    break;
	}

	/* Parse the options */
	npairs = (listlen - 1) / 2;
	for (pair=0; pair<npairs; ++pair)
	{
	    sw = Tcl_GetStringFromObj(datasetElems[2*pair+1], NULL);
	    if (sw[0] == '-') ++sw;
	    if (strcmp(sw, "weight") == 0)
	    {
		if (Tcl_GetDoubleFromObj(interp, datasetElems[2*pair+2],
					 &newset->weight) != TCL_OK)
		    goto fail0;
		if (newset->weight < 0.0)
		{
		    Tcl_AppendResult(interp, "bad dataset specification \"",
				     Tcl_GetStringFromObj(obj,NULL),
				     "\": weight can not be negative", NULL);
		    goto fail0;
		}
	    }
	    else if (strcmp(sw, "method") == 0)
	    {
		method = Tcl_GetStringFromObj(datasetElems[2*pair+2], NULL);
		newset->acc_method = hs_get_accum_method_by_tag(method);
		if (newset->acc_method == NULL)
		{
		    Tcl_AppendResult(interp, "Bad dataset specification \"",
				     Tcl_GetStringFromObj(obj,NULL),
				     "\": invalid fitting method.",
				     " Valid fitting methods are:\n", NULL);
		    hs_show_valid_accum_methods(interp);
		    goto fail0;
		}
		checkfree(newset->acc_method_name);
		check_strdup(newset->acc_method_name, method);
	    }
	    else if (item_type == HS_NTUPLE)
	    {
		/* This may be an ntuple column specifier */
		if (strcmp(sw, "x") == 0)
		    pcol = &newset->colx;
		else if (strcmp(sw, "y") == 0)
		    pcol = &newset->coly;
		else if (strcmp(sw, "z") == 0)
		    pcol = &newset->colz;
		else if (strcmp(sw, "v") == 0)
		    pcol = &newset->colval;
		else if (strcmp(sw, "e") == 0)
		    pcol = &newset->colerr;
		else
		{
		    Tcl_AppendResult(interp, "bad ntuple dataset specification \"",
				Tcl_GetStringFromObj(obj,NULL), "\": invalid switch \"",
				Tcl_GetStringFromObj(datasetElems[2*pair+1], NULL),
				"\"", NULL);
		    goto fail0;
		}
		varname = Tcl_GetStringFromObj(datasetElems[2*pair+2], NULL);
		ivar = hs_variable_index(id, varname);
		if (ivar < 0)
		{
		    Tcl_AppendResult(interp, "\"", varname,
				     "\" is not a variable of ntuple with id ",
				     Tcl_GetStringFromObj(datasetElems[0], NULL), NULL);
		    goto fail0;
		}
		*pcol = ivar;
	    }
	    else
	    {
		Tcl_AppendResult(interp, "bad histogram dataset specification \"",
				 Tcl_GetStringFromObj(obj,NULL), "\": invalid switch \"",
				 Tcl_GetStringFromObj(datasetElems[2*pair+1], NULL),
				 "\"", NULL);
		goto fail0;
	    }
	}

	/* Check which columns have been defined in an ntuple */
	if (item_type == HS_NTUPLE)
	{
	    if (newset->colx < 0)
	    {
		Tcl_AppendResult(interp, "bad ntuple dataset specification \"",
				 Tcl_GetStringFromObj(obj,NULL),
				 "\": column mapped to x axis is not specified", NULL);
		goto fail0;
	    }
	    newset->ndim = 1;
	    if (newset->coly >= 0)
		newset->ndim++;
	    if (newset->colz >= 0)
	    {
		newset->ndim++;
		if (newset->coly < 0)
		{
		    Tcl_AppendResult(interp, "bad ntuple dataset specification \"",
				     Tcl_GetStringFromObj(obj,NULL),
				     "\": can't specify z column ",
				     "without specifying y column", NULL);
		    goto fail0;
		}
	    }
	    if (newset->colval < 0)
	    {
		newset->binned = 0;
		if (newset->colerr >= 0)
		{
		    Tcl_AppendResult(interp, "bad ntuple dataset specification \"",
				     Tcl_GetStringFromObj(obj,NULL),
				     "\": can't specify error column ",
				     "without specifying value column", NULL);
		    goto fail0;
		}
	    }
	    else
		newset->binned = 1;
	}

	/* A few checks about the fitting method */
	if (hs_check_fitting_method(interp, newset, newset->acc_method_name) == NULL)
	    goto fail0;
    }

    /* Add the new set to the fitter */
    fitsets = (Fit_subset **)malloc((fit_config->nsets+1)*sizeof(Fit_subset *));
    if (fitsets == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	goto fail0;
    }
    for (i=0; i<fit_config->nsets; ++i)
	fitsets[i] = fit_config->fitsets[i];
    fitsets[fit_config->nsets] = newset;
    ++fit_config->nsets;
    if (fit_config->fitsets)
	free(fit_config->fitsets);
    fit_config->fitsets = fitsets;
    hs_declare_fit_complete(fit_config, 0);
    return TCL_OK;

 fail0:
    hs_destroy_fit_subset(newset);
    return TCL_ERROR;
}

Parameter_map_function * hs_default_mapping_function(const char *mapping)
{
    int i, nfcns;
    
    nfcns = sizeof(known_mappings)/sizeof(known_mappings[0]);
    for (i=0; i<nfcns; ++i)
	if (strcmp(known_mappings[i].name, mapping) == 0)
	    return known_mappings[i].fcn;
    return NULL;
}

void hs_destroy_subset_remove_fitters(Fit_config *config, int set)
{
    int i;
    Fit_subset *subset;

    assert(config);
    if (set < 0 || set >= config->nsets) return;
    subset = config->fitsets[set];
    if (subset == NULL) return;

    /* Remove fitters */
    for (i=0; i<subset->nfitters; ++i)
	if (hs_fit_fitter_usage_count(config, subset->fitter_names[i]) == 1)
	    hs_remove_fitter_from_config(config, subset->fitter_names[i]);

    hs_destroy_fit_subset(subset);
    config->fitsets[set] = NULL;
    hs_declare_fit_complete(config, 0);
}

void hs_minuit_quiet()
{
    hs_minuit_verbose(-1);
}

void hs_minuit_set_title(const char *title)
{
    unsigned len;

    assert(title);
    len = strlen(title);
    if (len > 50)
	len = 50;
    mnseti_(title, len);
}

void hs_minuit_verbose(int level)
{
    char command[64];
    int ierr = 0;

    if (level < -1)
	level = -1;
    if (level > 3)
	level = 3;
    sprintf(command, "SET PRI %d", level);
    mncomd_(0, command, &ierr, 0, strlen(command));
    assert(ierr == 0);
}

void hs_minuit_set_precision(double eps)
{
    int ierr = 0;
    char command[64];
    const double min_precision = 4.0*DBL_EPSILON;

    if (eps < min_precision)
	eps = min_precision;
    sprintf(command, "SET EPS %g", eps);
    mncomd_(0, command, &ierr, 0, strlen(command));
    assert(ierr == 0);
}

int hs_is_fit_compiled(const Fit_config *fit)
{
    int i, set;

    assert(fit);
    /* Check whether all subset filters are compiled */
    for (set = 0; set < fit->nsets; ++set)
    {
	if (fit->fitsets[set] == NULL) continue;
	if (fit->fitsets[set]->filter == NULL)
	    /* This is normal if the string representation is a blank line */
	    if (!is_blank_string(fit->fitsets[set]->filter_string))
		return 0;
    }
    /* Check whether all function mappings are compiled */
    for (i = 0; i < fit->nfitters; ++i)
    {
	assert(fit->fitters[i]);
	if (fit->fitters[i]->map == NULL)
	    return 0;
    }
    return 1;
}

void hs_init_fit_data_sets(Fit_config *fit_info)
{
    int set;
    Fit_subset *fitset;
    int id, nbins, xbins, ybins, zbins, ixbin, iybin, izbin;
    int ibin, errtype, nvars;
    Tcl_Interp *interp;
    char stringbuf[32];
    float *data = NULL, *poserr = NULL;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    double x, y, dxmin, dxmax, dymin, dymax, dzmin, dzmax, eps_x;
    double dxbins, dybins, dzbins, dxrange, dyrange, dzrange, halfbin_x;

    assert(fit_info);
    assert(fit_info->interp);

    interp = fit_info->interp;
    for (set=0; set<fit_info->nsets; ++set)
    {
	fitset = fit_info->fitsets[set];
	if (fitset == NULL) continue;

	/* Reset various cache variables */
	checkfree(fitset->points);
	fitset->npoints = 0;
	fitset->datapoints = 0;
	fitset->chisq_used_points = 0;
	fitset->chisq = 0.0;
	memset(&fitset->data_stats, 0, sizeof(fitset->data_stats));
	memset(&fitset->fit_stats, 0, sizeof(fitset->fit_stats));

	/* Do different things depending on the item type */
	id = fitset->id;
	fitset->item_type = hs_type(id);
	switch (fitset->item_type)
	{
	case HS_1D_HISTOGRAM:
	    nbins = hs_1d_hist_num_bins(id);
	    data = (float *)malloc(nbins * sizeof(float));
	    poserr = (float *)malloc(nbins * sizeof(float));
	    if (data == NULL || poserr == NULL)
	    {
		Tcl_ResetResult(interp);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    fitset->points = (DataPoint *)calloc(nbins, sizeof(DataPoint));
	    if (fitset->points == NULL)
	    {
		Tcl_ResetResult(interp);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    fitset->npoints = nbins;
	    dxbins = (double)nbins;
	    hs_1d_hist_range(id, &xmin, &xmax);
	    dxmin = xmin;
	    dxmax = xmax;
	    dxrange = dxmax - dxmin;
	    fitset->nx = nbins;
	    fitset->xmin = dxmin;
	    fitset->xmax = dxmax;
            halfbin_x = dxrange/dxbins/2.0;
            eps_x = BIN_EDGE_EPSILON*dxrange/dxbins;

	    /* Use bin centers for x coordinates */
	    dxmin += halfbin_x;

	    hs_1d_hist_bin_contents(id, data);
	    errtype = hs_1d_hist_errors(id, poserr, NULL);
	    for (ixbin = 0; ixbin < nbins; ++ixbin)
	    {
		fitset->points[ixbin].x = dxmin + 
		    (((double)ixbin)/dxbins)*dxrange;
		fitset->points[ixbin].value = data[ixbin];
		if (errtype != HS_NO_ERRORS)
		    fitset->points[ixbin].error = poserr[ixbin];
		if (fitset->filter)
		    if (!((fitset->filter)(fitset->points[ixbin].x-halfbin_x+eps_x, 0.0, 0.0) &&
                          (fitset->filter)(fitset->points[ixbin].x, 0.0, 0.0) &&
                          (fitset->filter)(fitset->points[ixbin].x+halfbin_x-eps_x, 0.0, 0.0)))
			fitset->points[ixbin].filtered = 1;
	    }
	    break;

	case HS_2D_HISTOGRAM:
	    hs_2d_hist_num_bins(id, &xbins, &ybins);
	    nbins = xbins*ybins;
	    data = (float *)malloc(nbins * sizeof(float));
	    poserr = (float *)malloc(nbins * sizeof(float));
	    if (data == NULL || poserr == NULL)
	    {
		Tcl_ResetResult(interp);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    fitset->points = (DataPoint *)calloc(nbins, sizeof(DataPoint));
	    if (fitset->points == NULL)
	    {
		Tcl_ResetResult(interp);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    fitset->npoints = nbins;
	    dxbins = (double)xbins;
	    dybins = (double)ybins;
	    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
	    dxmin = xmin;
	    dxmax = xmax;
	    dymin = ymin;
	    dymax = ymax;
	    dxrange = dxmax - dxmin;
	    dyrange = dymax - dymin;
	    fitset->nx = xbins;
	    fitset->ny = ybins;
	    fitset->xmin = dxmin;
	    fitset->xmax = dxmax;
	    fitset->ymin = dymin;
	    fitset->ymax = dymax;

	    /* Use bin centers for x and y coordinates */
	    dxmin += dxrange/dxbins/2.0;
	    dymin += dyrange/dybins/2.0;

	    hs_2d_hist_bin_contents(id, data);
	    errtype = hs_2d_hist_errors(id, poserr, NULL);
	    ibin = 0;
	    for (ixbin = 0; ixbin < xbins; ++ixbin)
	    {
		x = dxmin + (((double)ixbin)/dxbins)*dxrange;
		for (iybin = 0; iybin < ybins; ++iybin, ++ibin)
		{
		    fitset->points[ibin].x = x;
		    fitset->points[ibin].y = dymin + 
			(((double)iybin)/dybins)*dyrange;
		    fitset->points[ibin].value = data[ibin];
		    if (errtype != HS_NO_ERRORS)
			fitset->points[ibin].error = poserr[ibin];
		    if (fitset->filter)
			if (!(fitset->filter)(fitset->points[ibin].x,
					      fitset->points[ibin].y, 0.0))
			    fitset->points[ibin].filtered = 1;
		}
	    }
	    break;

	case HS_3D_HISTOGRAM:
	    hs_3d_hist_num_bins(id, &xbins, &ybins, &zbins);
	    nbins = xbins*ybins*zbins;
	    data = (float *)malloc(nbins * sizeof(float));
	    poserr = (float *)malloc(nbins * sizeof(float));
	    if (data == NULL || poserr == NULL)
	    {
		Tcl_ResetResult(interp);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    fitset->points = (DataPoint *)calloc(nbins, sizeof(DataPoint));
	    if (fitset->points == NULL)
	    {
		Tcl_ResetResult(interp);
		Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		goto fail0;
	    }
	    fitset->npoints = nbins;
	    dxbins = (double)xbins;
	    dybins = (double)ybins;
	    dzbins = (double)zbins;
	    hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	    dxmin = xmin;
	    dxmax = xmax;
	    dymin = ymin;
	    dymax = ymax;
	    dzmin = zmin;
	    dzmax = zmax;
	    dxrange = dxmax - dxmin;
	    dyrange = dymax - dymin;
	    dzrange = dzmax - dzmin;
	    fitset->nx = xbins;
	    fitset->ny = ybins;
	    fitset->nz = zbins;
	    fitset->xmin = dxmin;
	    fitset->xmax = dxmax;
	    fitset->ymin = dymin;
	    fitset->ymax = dymax;
	    fitset->zmin = dzmin;
	    fitset->zmax = dzmax;

	    /* Use bin centers for coordinates */
	    dxmin += dxrange/dxbins/2.0;
	    dymin += dyrange/dybins/2.0;
	    dzmin += dzrange/dzbins/2.0;

	    hs_3d_hist_bin_contents(id, data);
	    errtype = hs_3d_hist_errors(id, poserr, NULL);
	    ibin = 0;
	    for (ixbin = 0; ixbin < xbins; ++ixbin)
	    {
		x = dxmin + (((double)ixbin)/dxbins)*dxrange;
		for (iybin = 0; iybin < ybins; ++iybin)
		{
		    y = dymin + (((double)iybin)/dybins)*dyrange;
		    for (izbin = 0; izbin < zbins; ++izbin, ++ibin)
		    {
			fitset->points[ibin].x = x;
			fitset->points[ibin].y = y;
			fitset->points[ibin].z = dzmin + 
			(((double)izbin)/dzbins)*dzrange;
			fitset->points[ibin].value = data[ibin];
			if (errtype != HS_NO_ERRORS)
			    fitset->points[ibin].error = poserr[ibin];
			if (fitset->filter)
			    if (!(fitset->filter)(fitset->points[ibin].x,
						  fitset->points[ibin].y,
						  fitset->points[ibin].z))
				fitset->points[ibin].filtered = 1;
		    }
		}
	    }
	    break;

	case HS_NTUPLE:
	    nbins = hs_num_entries(id);
	    if (nbins > 0)
	    {
		nvars = hs_num_variables(id);
		data = (float *)malloc(nbins * sizeof(float));
		if (data == NULL)
		{
		    Tcl_ResetResult(interp);
		    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		    goto fail0;
		}
		fitset->points = (DataPoint *)calloc(nbins, sizeof(DataPoint));
		if (fitset->points == NULL)
		{
		    Tcl_ResetResult(interp);
		    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
		    goto fail0;
		}
		fitset->npoints = nbins;		
		if (fitset->colx >= 0)
		{
		    assert(fitset->colx < nvars);
		    hs_column_contents(id, fitset->colx, data);
		    for (ibin = 0; ibin < nbins; ++ibin)
			fitset->points[ibin].x = data[ibin];
		}
		if (fitset->coly >= 0)
		{
		    assert(fitset->coly < nvars);
		    hs_column_contents(id, fitset->coly, data);
		    for (ibin = 0; ibin < nbins; ++ibin)
			fitset->points[ibin].y = data[ibin];
		}
		if (fitset->colz >= 0)
		{
		    assert(fitset->colz < nvars);
		    hs_column_contents(id, fitset->colz, data);
		    for (ibin = 0; ibin < nbins; ++ibin)
			fitset->points[ibin].z = data[ibin];
		}
		if (fitset->colval >= 0)
		{
		    assert(fitset->colval < nvars);
		    hs_column_contents(id, fitset->colval, data);
		    for (ibin = 0; ibin < nbins; ++ibin)
			fitset->points[ibin].value = data[ibin];
		}
		if (fitset->colerr >= 0)
		{
		    assert(fitset->colerr < nvars);
		    hs_column_contents(id, fitset->colerr, data);
		    for (ibin = 0; ibin < nbins; ++ibin)
			fitset->points[ibin].error = data[ibin];
		}
		if (fitset->filter)
		    for (ibin = 0; ibin < nbins; ++ibin)
			if (!(fitset->filter)(fitset->points[ibin].x,
					      fitset->points[ibin].y,
					      fitset->points[ibin].z))
			    fitset->points[ibin].filtered = 1;
	    }
	    break;

	case HS_NONE:
	    if (fit_info->status == TCL_OK)
	    {
		sprintf(stringbuf, "%d", id);
		Tcl_AppendResult(interp, "Histo-Scope item with id ",
				 stringbuf, " has been deleted", NULL);
	    }
	    goto fail0;

	default:
	    /* This should never happen */
	    assert(0);
	}
	checkfree(poserr);
	checkfree(data);
    }
    return;

 fail0:
    if (poserr)
	free(poserr);
    if (data)
	free(data);
    fit_info->status = TCL_ERROR;
}

int hs_minuit_par_names_cmp(const char *p1, const char *p2)
{
    int len;
    char name1[MINUIT_PARAM_NAME_LEN], name2[MINUIT_PARAM_NAME_LEN];

    assert(p1 && p2);
    memset(name1, ' ', MINUIT_PARAM_NAME_LEN);
    memset(name2, ' ', MINUIT_PARAM_NAME_LEN);

    len = strlen(p1);
    if (len > MINUIT_PARAM_NAME_LEN) len = MINUIT_PARAM_NAME_LEN;
    memcpy(name1, p1, len);
    
    len = strlen(p2);
    if (len > MINUIT_PARAM_NAME_LEN) len = MINUIT_PARAM_NAME_LEN;
    memcpy(name2, p2, len);
    
    return strncmp(name1, name2, MINUIT_PARAM_NAME_LEN);
}

Accumulated_stat_function * hs_check_fitting_method(
    Tcl_Interp *interp, Fit_subset *fitset, const char *newmethod)
{
    Accumulated_stat_function *statmeth;
    int itemtype;

    assert(interp);
    assert(fitset);
    assert(newmethod);
    itemtype = hs_type(fitset->id);
    assert(itemtype == HS_1D_HISTOGRAM ||
	   itemtype == HS_2D_HISTOGRAM ||
	   itemtype == HS_3D_HISTOGRAM ||
	   itemtype == HS_NTUPLE);
    
    statmeth = hs_get_accum_method_by_tag(newmethod);
    if (statmeth == NULL)
    {
	Tcl_AppendResult(interp, "Invalid fitting method \"",
			 newmethod, "\". Valid fitting methods are:\n", NULL);
	hs_show_valid_accum_methods(interp);
	return NULL;
    }
    /* Check that this method can be used on this dataset */
    if (hs_accum_if_errors_required(newmethod))
    {
	if (itemtype == HS_NTUPLE)
	{
	    if (fitset->colerr < 0)
	    {
		Tcl_AppendResult(interp, "can't use fitting method \"",
				 newmethod, "\": error column is not defined", NULL);
		return NULL;
	    }
	}
	else
	{
	    if (hs_hist_error_status(fitset->id) == HS_NO_ERRORS)
	    {
		Tcl_AppendResult(interp, "can't use fitting method \"",
				 newmethod, "\": histogram has no errors", NULL);
		return NULL;
	    }
	}
    }
    if (!fitset->binned)
    {
	if (hs_accum_if_binning_required(newmethod))
	{
	    Tcl_AppendResult(interp, "can't use fitting method \"",
			     newmethod, "\": data is not binned", NULL);
	    return NULL;
	}
    }
    return statmeth;
}

char * hs_default_fit_title(const char *name)
{
    static char *default_name = NULL;
    checkfree(default_name);
    default_name = (char *)malloc((strlen(name) + 10) * sizeof(char));
    assert(default_name);
    strcpy(default_name, "Fit ");
    strcat(default_name, name);
    return default_name;
}

#define calculate_rho(rho,s1,s2) do {\
    if (s1 > 0.0 && s2 > 0.0)\
    {\
	rho = rho/stats->integ/s1/s2;\
	if (rho < -1.0) rho = -1.0;\
	else if (rho > 1.0) rho = 1.0;\
    }\
    else\
	rho = 0.0;\
} while(0);

void hs_data_point_stats(const DataPoint *data, size_t npoints,
			 size_t ndim, size_t offset, BasicStats *stats)
{
    size_t i, goodpoints, o1, o2;
    double value, dx, dy, dz;

    /* Check the arguments */
    if (npoints > 0)
	assert(data);
    assert(stats);
    assert(ndim > 0 && ndim < 4);
    o1 = offsetof(DataPoint, value);
    o2 = offsetof(DataPoint, fit);
    assert(offset == o1 || offset == o2);

    memset(stats, 0, sizeof(BasicStats));
    if (npoints == 0)
	return;

    /* First pass -- just get the means */
    goodpoints = 0;
    for (i=0; i<npoints; ++i)
    {
	if (data[i].filtered || data[i].errflag)
	    continue;
	++goodpoints;
	value = *((double *)((char *)(data + i) + offset));
	if (value < 0.0)
	    stats->has_neg_values = 1;
	stats->integ += value;
	stats->mean_x += value*data[i].x;
	if (ndim > 1)
	{
	    stats->mean_y += value*data[i].y;
	    if (ndim > 2)
		stats->mean_z += value*data[i].z;
	}
    }
    stats->goodpoints = goodpoints;
    if (goodpoints == 0 || stats->integ == 0.0 || stats->has_neg_values)
    {
	stats->mean_x = 0.0;
	stats->mean_y = 0.0;
	stats->mean_z = 0.0;
	return;
    }
    stats->mean_x /= stats->integ;
    stats->mean_y /= stats->integ;
    stats->mean_z /= stats->integ;
    if (goodpoints == 1)
	return;

    /* Second pass -- get the standard deviations */
    for (i=0; i<npoints; ++i)
    {
	if (data[i].filtered || data[i].errflag)
	    continue;
	value = *((double *)((char *)(data + i) + offset));
	dx = data[i].x - stats->mean_x;
	stats->s_x += value*dx*dx;
	if (ndim > 1)
	{
	    dy = data[i].y - stats->mean_y;
	    stats->s_y += value*dy*dy;
	    stats->rho_xy += value*dx*dy;
	    if (ndim > 2)
	    {
		dz = data[i].z - stats->mean_z;
		stats->s_z += value*dz*dz;
		stats->rho_xz += value*dx*dz;
		stats->rho_yz += value*dy*dz;
	    }
	}
    }
    stats->s_x = sqrt(stats->s_x/stats->integ);
    stats->s_y = sqrt(stats->s_y/stats->integ);
    stats->s_z = sqrt(stats->s_z/stats->integ);
    calculate_rho(stats->rho_xy,stats->s_x,stats->s_y);
    calculate_rho(stats->rho_xz,stats->s_x,stats->s_z);
    calculate_rho(stats->rho_yz,stats->s_y,stats->s_z);
}

int hs_minuit_is_locked(void)
{
    return minuit_is_locked;
}

int hs_minuit_lock(int lock)
{
    if (lock && !minuit_is_locked)
    {
	minuit_is_locked = 1;
	return 0;
    }
    if (!lock && minuit_is_locked)
    {
	minuit_is_locked = 0;
	return 0;
    }
    return 1;
}

static const double * sequential_map(const double *pars, int offset)
{
    return (pars + offset);
}

static const double * null_map(const double *pars, int offset)
{
    return NULL;
}

static const double * identical_map(const double *pars, int offset)
{
    return pars;
}

