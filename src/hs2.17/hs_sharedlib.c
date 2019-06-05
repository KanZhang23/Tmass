#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <limits.h>
#include <math.h>
#include "hs_sharedlib.h"

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


static DLLibrary *libtable = 0;
static int libtable_len = 0;

/* Function to close dll by number. Returns 0 on success. */
int hs_close_fitter_dll_bynumber(int n);

/* Local functions for which we will not create tcl commands */
tcl_routine(dlopen);
tcl_routine(dlclose);
tcl_routine(dlliblist);
tcl_routine(dllibfile);

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
