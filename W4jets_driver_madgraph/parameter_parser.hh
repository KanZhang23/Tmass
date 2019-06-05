#ifndef PARAMETER_PARSER_HH_
#define PARAMETER_PARSER_HH_

#include <string>
#include "tcl.h"

/* Parameter list parsers for the hs::ntuple_so_scan command.
 * The parameters are passed via the (normally optional) last
 * argument which in our case must look like a list of lists
 * {{parameter_name1 value1} {parameter_name2 value2} ...}.
 * All these functions return either TCL_OK or TCL_ERROR.
 */
int get_string_parameter(Tcl_Interp *interp, const char *param_list,
                         const char *pname, std::string *pvalue);
int get_double_parameter(Tcl_Interp *interp, const char *param_list,
                         const char *pname, double *pvalue);
int get_int_parameter(Tcl_Interp *interp, const char *param_list,
                      const char *pname, int *pvalue);
int get_unsigned_parameter(Tcl_Interp *interp, const char *param_list,
                           const char *pname, unsigned *pvalue);
int get_bool_parameter(Tcl_Interp *interp, const char *param_list,
                       const char *pname, bool *pvalue);

#endif /* PARAMETER_PARSER_HH_ */
