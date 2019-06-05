/*
 * tclAppInit.c --
 *
 *	Provides a default version of the main program and Tcl_AppInit
 *	procedure for tclsh and other Tcl-based applications (without Tk).
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 1998-1999 Scriptics Corporation.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include <stdlib.h>

#include "histoscope.h"
#include "data_integ_mtm3.h"
#include "MTM3_Init.h"

static int c_mtm3scan(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

#define tcl_require_objc(N) do {\
  if (objc != N)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

#define tcl_objc_range(N,M) do {\
  if (objc < N || objc > M)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

#define verify_ntuple(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_NTUPLE)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not an ntuple", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#undef BUILD_tcl
#undef STATIC_BUILD
#include "tcl.h"

/*
 * The following #if block allows you to change the AppInit function by using
 * a #define of TCL_LOCAL_APPINIT instead of rewriting this entire file. The
 * #if checks for that #define and uses Tcl_AppInit if it does not exist.
 */

#ifndef TCL_LOCAL_APPINIT
#define TCL_LOCAL_APPINIT Tcl_AppInit
#endif
#ifndef MODULE_SCOPE
#   define MODULE_SCOPE extern
#endif
MODULE_SCOPE int TCL_LOCAL_APPINIT(Tcl_Interp *);
MODULE_SCOPE int main(int, char **);

/*
 * The following #if block allows you to change how Tcl finds the startup
 * script, prime the library or encoding paths, fiddle with the argv, etc.,
 * without needing to rewrite Tcl_Main()
 */

#ifdef TCL_LOCAL_MAIN_HOOK
MODULE_SCOPE int TCL_LOCAL_MAIN_HOOK(int *argc, char ***argv);
#endif

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tcl_Main never returns here, so this procedure never returns
 *	either.
 *
 * Side effects:
 *	Just about anything, since from here we call arbitrary Tcl code.
 *
 *----------------------------------------------------------------------
 */

int
main(
    int argc,			/* Number of command-line arguments. */
    char *argv[])		/* Values of command-line arguments. */
{
#ifdef TCL_XT_TEST
    XtToolkitInitialize();
#endif

#ifdef TCL_LOCAL_MAIN_HOOK
    TCL_LOCAL_MAIN_HOOK(&argc, &argv);
#endif

    Tcl_Main(argc, argv, TCL_LOCAL_APPINIT);
    return 0;			/* Needed only to prevent compiler warning. */
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization. Most
 *	applications, especially those that incorporate additional packages,
 *	will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error message in
 *	the interp's result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(
    Tcl_Interp *interp)		/* Interpreter for application. */
{
    if ((Tcl_Init)(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#endif

    /*
     * Call the init procedures for included packages. Each call should look
     * like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module. (Dynamically-loadable packages
     * should have the same entry-point name.)
     */

     if (MTM3_Init(interp) == TCL_ERROR) {
         return TCL_ERROR;
     }

    /*
     * Call Tcl_CreateCommand for application-specific commands, if they
     * weren't already created by the init procedures called above.
     */
    if (Tcl_CreateObjCommand(interp,
			     "mtm3scan",
			     c_mtm3scan,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;

    /*
     * Specify a user-specific startup file to invoke if the application is
     * run interactively. Typically the startup file is "~/.apprc" where "app"
     * is the name of the application. If this line is deleted then no
     * user-specific startup file will be run under any conditions.
     */
    (Tcl_ObjSetVar2)(interp, Tcl_NewStringObj("tcl_rcFileName", -1), NULL,
	    Tcl_NewStringObj("~/.tclshrc", -1), TCL_GLOBAL_ONLY);

    return TCL_OK;
}

static int c_mtm3scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: ntuple_so_scan $ntuple_id some_string? */
    static char *OUT_OF_MEMORY = "out of memory";

    int i, parent_id, nrows, ncolumns;
    char *some_string = NULL;
    float *ntuple_data = NULL;
    int status = TCL_ERROR;

    tcl_objc_range(2, 3);
    verify_ntuple(parent_id, 1);
    if (objc > 2)
    {
        some_string = Tcl_GetStringFromObj(objv[2], NULL);
        if (some_string[0] == '\0')
            some_string = NULL;
    }

    nrows = hs_num_entries(parent_id);
    ncolumns = hs_num_variables(parent_id);
    if (nrows > 0)
    {
	ntuple_data = (float *)malloc(ncolumns*sizeof(float));
	if (ntuple_data == NULL)
	{
	    Tcl_SetResult(interp, OUT_OF_MEMORY, TCL_STATIC);
	    goto fail;
	}
        if (hs_ntuple_scan_init(interp, parent_id, some_string) != TCL_OK)
            goto fail;
        for (i=0; i<nrows; ++i)
        {
            hs_row_contents(parent_id, i, ntuple_data);
            if (hs_ntuple_scan_function(interp, ntuple_data) != TCL_OK)
                goto fail;
        }
        if (hs_ntuple_scan_conclude(interp, parent_id, some_string) != TCL_OK)
            goto fail;
    }

    status = TCL_OK;
 fail:
    if (ntuple_data)
	free(ntuple_data);
    return status;
}
