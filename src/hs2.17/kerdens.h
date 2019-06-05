#ifndef KERDENS_H_
#define KERDENS_H_

#include "tcl.h"

typedef double (Kernel_function_1d)(double);

/* We will only consider radially symmetric
   kernels in the 2d case. It is presumed
   that they get (x_t V x) quadratic form
   as an argument */
typedef double (Kernel_function_2d)(double);

/* Functions to look up standard kernels by name */
Kernel_function_1d *find_kernel_1d(const char *name);
Kernel_function_2d *find_kernel_2d(const char *name);

/* Functions to estimate the density. These functions 
 * return an error code:
 *
 *   0 -- success
 *   1 -- bad input parameters other than bandwidth
 *   2 -- out of memory
 *   3 -- bad bandwidth or transformation matrix
 *
 * If the code fails then the result is not changed.
 */
int estmate_kernel_density_1d(int nt_id, int points_col, int weights_col,
			      int localbw_col, Kernel_function_1d *fcn,
			      double dbw, double dmin, double dmax,
			      int nout, float *result);
int estmate_kernel_density_2d(int nt_id, int x_col, int y_col, int weights_col,
			      int sxsq_col, int sysq_col, int sxsy_col,
			      Kernel_function_2d *fcn, double mat[2][2],
			      double xmin, double xmax, int nx,
			      double ymin, double ymax, int ny, float *result);

/* Functions to append kernel names to the interp
   result (useful in building error messages) */
void append_kernel_names_1d(Tcl_Interp *interp);
void append_kernel_names_2d(Tcl_Interp *interp);

#endif /* KERDENS_H_ */
