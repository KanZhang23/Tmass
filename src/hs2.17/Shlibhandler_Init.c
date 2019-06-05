#include "sharedlib_handler.h"

int Shlibhandler_Init(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#endif

    tcl_new_command(hs, sharedlib_handler);

    return TCL_OK;
}
