#ifndef _FIT_FUNCTION_H
#define _FIT_FUNCTION_H

#include "simple_tcl_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The prototype for a fitter function written in C.
 */
#define MN_FUTIL_ARGLIST double x, double y, double z, int mode, \
                         const double *pars, int *ierr
typedef double (Minuit_c_fit_function) (MN_FUTIL_ARGLIST);
/* Input arguments:
 *   x    -- the first variable
 *   y    -- the second variable (may be ignored)
 *   z    -- the third variable (may be ignored)
 *   mode -- an external parameter which will not be changed by Minut.
 *           For example, it may specify the degree of a polynomial.
 *   pars -- array of parameters
 *
 * Output:
 *   ierr -- error status. 0 means everything is OK. The function is expected
 *           to print a reasonable error message to stdout if something goes
 *           wrong and set ierr to a non-zero value.
 *
 * Returns:
 *   The function value at given coordinates with given parameters.
 */
#define Minuit_futil(funct) double funct (MN_FUTIL_ARGLIST)


/* The prototype for the fitter function written in FORTRAN.
 */
typedef double (Minuit_f_fit_function) (double *x, double *y, double *z,
			 const int *mode, const double *pars, int *ierr);
/* The meaning of the arguments is the same as in the Minuit_c_fit_function */


/* The prototype for the gradient (over parameters) written in C */
typedef int (Minuit_c_grad_function) (double x, double y, double z,
			int mode, const double *pars, double *grad);
/* Input arguments:
 *   x    -- the first variable
 *   y    -- the second variable (may be ignored)
 *   z    -- the third variable (may be ignored)
 *   mode -- an external parameter which will not be changed by Minut.
 *           For example, it may specify the degree of a polynomial.
 *   pars -- array of parameters
 *
 * Output:
 *   grad -- array of the partial derivatives over parameters
 *
 * Returns:
 *   Error status. 0 means everything is OK.
 */


/* The prototype for the gradient (over parameters) written in FORTRAN */
typedef int (Minuit_f_grad_function) (double *x, double *y, double *z,
		    const int *mode, const double *pars, double *grad);


/* Auxiliary function for initialization and cleanup. We do not expect
 * to write such functions often, so we are going to use a FORTRAN-
 * compatible prototype everywhere. Such a function should return 0
 * in case everything is OK. The functions are called with just one
 * integer argument which, if needed, could be an index into an array
 * of structures defined elsewhere.
 */
typedef int (Minuit_aux_function) (const int *mode);


/* Struct which contains all info about a fitter function */
typedef struct
{
    int groupnumber;                /* The number of a group to which
                                       this function belongs. This number
                                       is usually just the dll number. */
    char *fullname;                 /* Mathematical function name */
    char *description;              /* Function description */
    int ndim;                       /* Number of variables (1, 2, or 3) */
    int is_combined;                /* If this is set, the function is
				       a combination of other functions.
				       In this case the mode (next) will not
				       be modifiable from Tcl */
    int mode;                       /* The value of the mode argument which
                                       will be passed to the function. This
				       value will be accessible for reading
                                       or writing from Tcl. */
    int refcount;                   /* The number of times this function has
                                       been referenced by other objects. It
                                       should not be possible to delete the
                                       function, change the mode, or rename
				       the parameters when refcount > 0 */
    int npars_min;                  /* Minimum allowed number of parameters */
    int npars_max;                  /* Maximum allowed number of parameters.
                                       When npars_min != npars_max it is
                                       assumed that the number of parameters
                                       is given by "mode". */
    char **param_names;             /* Parameter names. Must contain npars_max
				       names. */

    Minuit_aux_function* init;      /* Initializer function */
    char *init_fun_name;            /* Initializer function compiler name */

    Minuit_c_fit_function* fit_c;   /* C fit function */
    Minuit_f_fit_function* fit_f;   /* FORTRAN fit function. At least one of
                                       fit_c and fit_f must not be NULL */
    char *fit_fun_name;             /* Fit function compiler name */

    Minuit_c_grad_function* grad_c; /* C gradient function */
    Minuit_f_grad_function* grad_f; /* FORTRAN gradient function */
    char *grad_fun_name;            /* Gradient function compiler name */

    Minuit_aux_function* cleanup;   /* Cleanup function */
    char *cleanup_fun_name;         /* Cleanup function compiler name */

    int ownsdll;                    /* Tells if the function should try
				       to unload its own dll when deleted */
} Minuit_fitter_info;

/* Functions related to the memory management of the Minuit_fitter_info.
   The copy function resets the refcount to 0. */
void hs_destroy_fitter_info(Minuit_fitter_info *p);
Minuit_fitter_info * hs_copy_fitter_info(const Minuit_fitter_info *p);

/* The following function is used to define a fitter function. If successful,
   it assumes the ownership of the Minuit_fitter_info struct which therefore
   must be allocated on the heap. name is the function tag in the table of
   functions. Returns TCL_OK when everything is fine. */
int hs_add_fitter_function(Tcl_Interp *interp, const char *name,
			   Minuit_fitter_info *fitter);

/* Function to figure out the number of parameters for a fitter. 
   Use this function rather than access npars_min, npars_max,
   and mode directly. */
int hs_fitter_num_pars(Minuit_fitter_info *fitfun);

/* Function to copy fitter info. Returns TCL_OK if everything is fine. */
int hs_copy_fitter_function(Tcl_Interp *interp, const char *newname,
			    const char *oldname);

/* Function to rename fitter info. Returns TCL_OK if everything is fine. */
int hs_rename_fitter_function(Tcl_Interp *interp, const char *newname,
			      const char *oldname);

/* Function to lookup fitter info by name. Returns NULL 
 * if there is no fitter with the given name
 */
Minuit_fitter_info * hs_find_fitter_function(const char *name);

/* The following functions are used to clean up the table of functions.
 * They return 0 on success, 1 if the argument does not correspond to
 * any function, and 2 if the action would result in a removal of 
 * a function currently in use. The "really_remove" argument may
 * be set to 0 in which case the function is not actually removed,
 * but the status is returned as if the action had been performed.
 */
int hs_remove_fitter_function(const char *name, int really_remove);
int hs_remove_fitter_group(int groupnumber);

/* Usage count (will call initialization and cleanup as necessary).
 * These functions return 0 if everything is fine.
 */
int hs_incr_fitter_usage_count(Minuit_fitter_info *fitfun);
int hs_decr_fitter_usage_count(Minuit_fitter_info *fitfun);

/* Function to increment and decrement dll reference counts.
 * The decrement function will attempt to unload the dll when
 * the count reaches 0. The functions return 0 on success.
 * hs_decr_dll_refcount will call the _hs_fini function (if exported)
 * before unloading the dll.
 */
int hs_incr_dll_refcount(int n);
int hs_decr_dll_refcount(int n);

/* Find a function in a library.
 * On exit, the value of *code is set to
 *   -1   in case the function not found
 *    0   if this appears to be a C function
 *    1   if this appears to be a FORTRAN function
 */
void * hs_find_library_function(Tcl_Interp *interp, int libnumber,
				const char* funct_name, int *code);

/* Headers for the Tcl API functions */
tcl_routine(sharedlib);
tcl_routine(function);
tcl_routine(function_import);
tcl_routine(function_list);
tcl_routine(function_sum);
tcl_routine(function_divide);
tcl_routine(function_multiply);
tcl_routine(function_compose);
tcl_routine(Function_owns_dll);

#ifdef __cplusplus
}
#endif

#endif /* _FIT_FUNCTION_H */
