#include <stdlib.h>
#include "histo_tcl_api.h"
#include "cernlib.h"
#include "ranlux.h"

#define RANLUX_INTEGERS 25
#define RANLUX_STACK_ARRLEN 101

float hs_ranlux_common_buffer[RANDOM_BUFFER_SIZE];
int hs_ranlux_common_buffer_index = RANDOM_BUFFER_SIZE;

tcl_routine(Ranlux)
{
    int i, len;
    float *data, ftmp[RANLUX_STACK_ARRLEN];
    Tcl_Obj **results, *rtmp[RANLUX_STACK_ARRLEN];

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &len) != TCL_OK)
	return TCL_ERROR;
    if (len <= 0)
    {
	Tcl_AppendResult(interp, "expected a positive integer, got ",
			 Tcl_GetStringFromObj(objv[1],NULL), NULL);
	return TCL_ERROR;
    }
    if (len <= RANLUX_STACK_ARRLEN)
	data = ftmp;
    else
	data = (float *)malloc(len*sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    if (len <= RANLUX_STACK_ARRLEN)
	results = rtmp;
    else
	results = (Tcl_Obj **)malloc(len*sizeof(Tcl_Obj *));
    if (results == NULL)
    {
	if (data != ftmp) free(data);
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    i = len;
    ranlux_(data, &i);
    for (i=0; i<len; ++i)
    {
	if ((results[i] = Tcl_NewDoubleObj((double)data[i])) == NULL)
	{
	    for (--i; i>=0; --i)
	    {
		Tcl_IncrRefCount(results[i]);
		Tcl_DecrRefCount(results[i]);
	    }
	    if (data != ftmp) free(data);
	    if (results != rtmp) free(results);
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    if (data != ftmp) free(data);
    Tcl_SetObjResult(interp, Tcl_NewListObj(len, results));
    if (results != rtmp) free(results);
    return TCL_OK;
}

tcl_routine(Rluxgo)
{
    int listlen, lux, init, k1, k2;
    Tcl_Obj **listObjElem;

    tcl_require_objc(2);
    if (Tcl_ListObjGetElements(interp, objv[1], 
			       &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen != 4)
    {
	Tcl_AppendResult(interp, "bad starting point specifier \"",
			 Tcl_GetStringFromObj(objv[1],NULL), "\"", NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, listObjElem[0], &lux) != TCL_OK)
	return TCL_ERROR;
    if (lux < 0)
    {
	Tcl_AppendResult(interp, "bad luxury level ",
			 Tcl_GetStringFromObj(listObjElem[0],NULL), NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, listObjElem[1], &init) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, listObjElem[2], &k1) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, listObjElem[3], &k2) != TCL_OK)
	return TCL_ERROR;
    rluxgo_(&lux, &init, &k1, &k2);
    return TCL_OK;
}

tcl_routine(Rluxat)
{
    int lux, init, k1, k2;
    Tcl_Obj *result;

    tcl_require_objc(1);
    rluxat_(&lux, &init, &k1, &k2);
    result = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(lux));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(init));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(k1));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(k2));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;    
}

tcl_routine(Rluxin)
{
    int i, idata[RANLUX_INTEGERS];
    Tcl_Obj **listObjElem;

    tcl_require_objc(2);
    if (Tcl_ListObjGetElements(interp, objv[1], 
			       &i, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (i != RANLUX_INTEGERS)
    {
	Tcl_AppendResult(interp, "bad starting point specifier \"",
			 Tcl_GetStringFromObj(objv[1],NULL), "\"", NULL);
	return TCL_ERROR;
    }
    for (i=0; i<RANLUX_INTEGERS; ++i)
	if (Tcl_GetIntFromObj(interp, listObjElem[i], &idata[i]) != TCL_OK)
	    return TCL_ERROR;
    rluxin_(idata);
    return TCL_OK;
}

tcl_routine(Rluxut)
{
    int i, idata[RANLUX_INTEGERS];
    Tcl_Obj *result;

    tcl_require_objc(1);
    rluxut_(idata);
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<RANLUX_INTEGERS; ++i)
	Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(idata[i]));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;    
}

tcl_routine(Rnormx)
{
    int i, len;
    float *data, ftmp[RANLUX_STACK_ARRLEN];
    Tcl_Obj **results, *rtmp[RANLUX_STACK_ARRLEN];
    double mean, sigma;
    float fmean, fsigma;

    if (objc != 4)
    {
	Tcl_SetResult(interp, "this command needs three arguments: "
		      "mean, standard deviation, and length "
		      "of list to generate", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[1], &mean) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &sigma) != TCL_OK)
	return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &len) != TCL_OK)
	return TCL_ERROR;
    if (len <= 0)
    {
	Tcl_AppendResult(interp, "expected a positive integer, got ",
			 Tcl_GetStringFromObj(objv[3],NULL), NULL);
	return TCL_ERROR;
    }

    if (len <= RANLUX_STACK_ARRLEN)
	data = ftmp;
    else
	data = (float *)malloc(len*sizeof(float));
    if (data == NULL)
    {
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    if (len <= RANLUX_STACK_ARRLEN)
	results = rtmp;
    else
	results = (Tcl_Obj **)malloc(len*sizeof(Tcl_Obj *));
    if (results == NULL)
    {
	if (data != ftmp) free(data);
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    i = len;
    rnormx_(data, &i, ranlux_);
    fmean = (float)mean;
    fsigma = (float)sigma;
    for (i=0; i<len; ++i)
	data[i] = fmean + fsigma*data[i];
    for (i=0; i<len; ++i)
    {
	if ((results[i] = Tcl_NewDoubleObj((double)data[i])) == NULL)
	{
	    for (--i; i>=0; --i)
	    {
		Tcl_IncrRefCount(results[i]);
		Tcl_DecrRefCount(results[i]);
	    }
	    if (data != ftmp) free(data);
	    if (results != rtmp) free(results);
	    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    if (data != ftmp) free(data);
    Tcl_SetObjResult(interp, Tcl_NewListObj(len, results));
    if (results != rtmp) free(results);
    return TCL_OK;
}
