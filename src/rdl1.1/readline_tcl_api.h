#ifndef READLINE_TCL_API_H
#define READLINE_TCL_API_H

#include "simple_tcl_api.h"
#include "rdl.h"

void init_readline_tcl_api(Tcl_Interp *interp);

tcl_routine(tcl_api_version);
tcl_routine(readline);
tcl_routine(add_completion);
tcl_routine(callback_handler_install);
tcl_routine(callback_handler_remove);
tcl_routine(callback_read_char);

tcl_routine(read_history);
tcl_routine(write_history);
tcl_routine(clear_history);
tcl_routine(history_max_length);

#endif /* not READLINE_TCL_API_H */
