#ifndef _MINUIT_FCN_H
#define _MINUIT_FCN_H

#include "tcl.h"

typedef struct _fit_config Fit_config;
typedef struct _data_point DataPoint;

#define MN_FCN_ARGLIST const int *npar, double *grad, double *fval, \
                       const double *xval, const int *iflag, \
                       Fit_config *fit_info
typedef void (Minuit_user_function) (MN_FCN_ARGLIST);
/* Input parameters: 
 *   npar     -- Number of currently variable parameters. 
 *   xval     -- vector of (constant and variable) parameters. 
 *   iflag    -- Indicates what is to be calculated:
 *               1 -- Initialization.
 *               2 -- Calculate FVAL and (optionally) GRAD. The
 *                    function is expected to calculate GRAD only
 *                    if the command SET GRAD has appeared.
 *               3 -- End of fit. Perform any final calculations,
 *                    output fitted data, etc.
 *   fit_info -- Originally, this was a pointer to the utility
 *               function, "futil". Here, this pointer is used
 *               to pass the fit configuration info.
 *
 * Output parameters:
 *   fval  -- The calculated function value. 
 *   grad  -- The (optional) vector of first derivatives.
 *   fit_info->datapoints  -- The number of data points used.
 */
#define Minuit_fcn(funct) void funct (MN_FCN_ARGLIST)

/* Typedef for user-settable fcn for various
   minimization tasks other than curve fitting */
typedef int (Minuit_tcl_fcn) (void *clientData, Tcl_Interp *interp,
                              int objc, Tcl_Obj *CONST objv[],
                              const double *xval, int npar, int iflag,
                              double *fval, double *grad);

typedef double (Accumulated_stat_function) (
    double fval, const DataPoint *point, int binned, int *ierr);
/* This function is expected to set *ierr to a meaningful
 * error code in case something goes wrong.
 */
#define Minuit_stat_accumulator(funct) double \
    funct (double fval, const DataPoint *point, int binned, int *ierr)

Minuit_user_function generic_stats_adder;
Minuit_user_function generic_user_minimizer;

void hs_show_valid_fit_fcns(Tcl_Interp *interp);
Minuit_user_function * hs_set_fit_fcn_by_tag(char *tag);
Minuit_user_function * hs_get_current_fit_fcn(void);
const char * hs_get_current_fcn_tag(void);

Accumulated_stat_function acc_leastsq_with_errors;
Accumulated_stat_function acc_leastsq_no_errors;
Accumulated_stat_function acc_max_likelihood;
Accumulated_stat_function acc_ext_max_likelihood;
Accumulated_stat_function acc_pearsons_chisq;

Accumulated_stat_function * hs_get_accum_method_by_tag(const char *tag);
int hs_accum_if_binning_required(const char *tag);
int hs_accum_if_errors_required(const char *tag);
void hs_show_valid_accum_methods(Tcl_Interp *interp);
Tcl_Obj * hs_list_valid_accum_methods(void);

#endif /* _MINUIT_FCN_H */
