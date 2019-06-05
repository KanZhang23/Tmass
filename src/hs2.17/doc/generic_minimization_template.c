#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>
#include "tcl.h"
#include "histoscope.h"

#define tcl_c_name(name) (My_tcl_api_ ## name)

#define tcl_routine(name) EXTERN int tcl_c_name(name)\
                                   (ClientData clientData,\
                                    Tcl_Interp *interp,\
                                    int objc, Tcl_Obj *CONST objv[])

#define tcl_new_command(namespace, name) if ((Tcl_CreateObjCommand(interp,\
      #namespace"::"#name, tcl_c_name(name), (ClientData)NULL,\
      (Tcl_CmdDeleteProc *)NULL)) == NULL)\
      return TCL_ERROR

#define tcl_delete_command(namespace, name) do {\
    Tcl_DeleteCommand(commandInterp, #namespace"::"#name);\
} while(0);

#define tcl_require_objc(N) do {\
  if (objc != N)\
  {\
    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                     " : wrong # of arguments", NULL);\
    return TCL_ERROR;\
  }\
} while(0);

#define verify_ntuple(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	goto fail;\
    if (hs_type(id) != HS_NTUPLE)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not an ntuple", NULL);\
	goto fail;\
    }\
} while(0);

/* Variables local to this file */
static Tcl_Interp *commandInterp = 0;

/* 
 * The following tcl command can be used to pass the data
 * between something local to this file and the tcl code
 */
tcl_routine(interface)
{
    return TCL_OK;
}

/* Cleanup function will be called when the fit is deleted */
static void my_cleanup(void)
{
}

/*
 * Initialization function will be called when the fit is started
 * (i.e., when "iflag" is set to 1).
 */
static int my_init(void *clientData, Tcl_Interp *interp,
                   int objc, Tcl_Obj *CONST objv[])
{
    int i;

    printf("Init function was called with %d arguments:\n", objc);
    for (i=0; i<objc; ++i)
        printf("%d: %s\n", i, Tcl_GetStringFromObj(objv[i], NULL));
    return TCL_OK;

 fail:
    my_cleanup();
    return TCL_ERROR;
}

#ifdef __cplusplus
extern "C" {
#endif

int my_fit(void *clientData, Tcl_Interp *interp,
           int objc, Tcl_Obj *CONST objv[],
           const double *xval, int npar, int iflag,
           double *fval, double *grad)
{
    if (iflag == 1)
        if (my_init(clientData, interp, objc, objv) != TCL_OK)
            return TCL_ERROR;

    *fval = 0.0;
    return TCL_OK;
}

int _hs_init(Tcl_Interp *interp)
{
    tcl_new_command(myfit, interface);
    commandInterp = interp;
    return TCL_OK;
}

void _hs_fini(void)
{
    tcl_delete_command(myfit, interface);
    my_cleanup();
}

#ifdef __cplusplus
}
#endif
