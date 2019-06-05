#include <stdio.h>
#include <string.h>
#include "tcl.h"

static int c_nextint(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static Tcl_Interp *commandInterp = 0;

#define tcl_require_objc(N) do {\
  if (objc != N)\
  {\
      Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                       " : wrong # of arguments", NULL);\
      return TCL_ERROR;\
  }\
} while(0);

int _hs_init(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "nextint",
			     c_nextint,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    commandInterp = interp;
    return TCL_OK;
}

void _hs_fini(void)
{
    if (commandInterp)
	Tcl_DeleteCommand(commandInterp, "nextint");
}

static int c_nextint(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    char buf[64];
    static unsigned long long number = 0LL;

    tcl_require_objc(1);
    sprintf(buf, "%llu", number++);
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
    return TCL_OK;
}

/*
  Compile and load this code using the following commands:

  hs::sharedlib_compile nextint.c ./nextint.so
  set dlltoken [hs::sharedlib open ./nextint.so]

  When you are done using this command, unload it with

  hs::sharedlib close $dlltoken
*/
