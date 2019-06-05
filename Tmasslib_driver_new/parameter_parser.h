#ifndef PARAMETER_PARSER_H_
#define PARAMETER_PARSER_H_

#include "tcl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Parameter list parsers for the hs::ntuple_so_scan command.
 * The parameters are passed via the (normally optional) last
 * argument which in our case must look like a list of lists
 * {{parameter_name1 value1} {parameter_name2 value2} ...}.
 * All these functions return either TCL_OK or TCL_ERROR except
 * "get_string_parameter" which simply returns null pointer
 * in case the parameter is not found. Note that "free" should
 * be eventually called on the "get_string_parameter" result
 * in case it is not null.
 */
char * get_string_parameter(Tcl_Interp *interp, const char *param_list,
                            const char *pname);
int get_double_parameter(Tcl_Interp *interp, const char *param_list,
                         const char *pname, double *pvalue);
int get_int_parameter(Tcl_Interp *interp, const char *param_list,
                      const char *pname, int *pvalue);
int get_boolean_parameter(Tcl_Interp *interp, const char *param_list,
                          const char *pname, int *pvalue);

#ifdef __cplusplus
}
#endif

#endif /* PARAMETER_PARSER_H_ */
