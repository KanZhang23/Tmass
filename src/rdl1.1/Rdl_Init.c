#include "rdl.h"
#include "readline_tcl_api.h"

int Rdl_Init(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.0", 0) == NULL) {
	return TCL_ERROR;
    }
#endif
/* 
 *   Don't call Tcl_PkgProvide here. It will be done by 
 *   "package provide" in the rdl_utils.tcl file instead
 *   if (Tcl_PkgProvide(interp, "rdl", RDL_VERSION) != TCL_OK) {
 *	return TCL_ERROR;
 *   }
 */
    tcl_new_command(rdl, tcl_api_version);
    tcl_new_command(rdl, readline);
    tcl_new_command(rdl, add_completion);
    tcl_new_command(rdl, callback_handler_install);
    tcl_new_command(rdl, callback_handler_remove);
    tcl_new_command(rdl, callback_read_char);

    tcl_new_command(rdl, read_history);
    tcl_new_command(rdl, write_history);
    tcl_new_command(rdl, clear_history);
    tcl_new_command(rdl, history_max_length);

    init_readline_tcl_api(interp);

    return TCL_OK;
}
