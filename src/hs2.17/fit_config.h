#ifndef _FIT_CONFIG_H
#define _FIT_CONFIG_H

#include "tcl.h"
#include "fit_function.h"
#include "minuit_fcn.h"

#define CB_FIT_DELETE "destruct"
#define CB_FIT_LOSTSYNC "lostsync"
#define CB_FIT_COMPLETE "complete"
#define BIN_EDGE_EPSILON 1.0e-5

/* Function which remaps parameters from Minuit into local definitions */
typedef const double * (Parameter_map_function) (const double *pars,
						 int offset);

/* Function which implements dataset exclusion regions */
typedef int (Dataset_filter_function) (double x, double y, double z);

struct _data_point   /* typedefed to DataPoint */
{
    int filtered;
    int errflag;
    double x;
    double y;
    double z;
    double value;
    double error;
    double fit;
};

/* The following struct must not have any pointers */
typedef struct _basic_stats
{
    size_t goodpoints;
    int has_neg_values;
    double integ;
    double mean_x;
    double mean_y;
    double mean_z;
    double s_x;
    double s_y;
    double s_z;
    double rho_xy;
    double rho_xz;
    double rho_yz;
} BasicStats;

typedef struct
{
    /* The function to use in the fit */
    char *fitter_tag;
    Minuit_fitter_info *fitter;

    /* The mapping function from global parameters
       into the parameters of this fitter */
    char *mapping;
    Parameter_map_function* map;

    /* Parameter number offset for the mapping function.
     * Useful for simple sequential mapping.
     */
    int offset;

    /* Current best values of the function parameters */
    double *best_pars;
    int npars;

    /* Just a useful temporary variable to store the result of
     * the mapping function call. It does not own any memory.
     */
    const double *local_pars;
} Fitter_info;
/* Default constructor */
Fitter_info * hs_create_simple_fitter(const char *name);
/* Copy constructor */
Fitter_info * hs_copy_fit_fitter(const Fitter_info *src);
/* Destructor */
void hs_destroy_fit_fitter(Fitter_info *info);


typedef struct
{
    int id;      /* Histo-Scope object id. Must refer
		    to either a histogram or an ntuple. */
    int ndim;    /* Dataset dimensionality (1, 2, or 3). This
		    element is needed because it is not obvious
		    for ntuples how many columns should be used. */
    int binned;  /* This tells us if the fit is binned or unbinned.
		    In the unbinned case there will be no errors
		    associated with the data points. */
    /* The following 5 variables describe ntuple column mapping.
     * They only make sense if id refers to an ntuple. Value
     * -1 means that there is no mapping.
     */
    int colx;
    int coly;
    int colz;
    int colval;
    int colerr;

    /* The filter which will be applied to all dataset points.
     * This filter allows us to implement exclusion regions.
     */
    char * filter_string;
    Dataset_filter_function * filter;

    /* The set of fitting functions. All these functions
     * will be summed in the fit. Must not have null pointers.
     */
    char **fitter_names;
    int nfitters;
    Fitter_info **pfitters; /* This is just a convenience array;
			       these pointers do not own any data */

    /* Common weight for all points in the dataset. Must not be negative. */
    double weight;

    /* The name and the pointer to the weight accumulating
     * method in the chi-square, negative log-likelihood,
     * or other minimization method.
     */
    char * acc_method_name;
    Accumulated_stat_function * acc_method;

    /* Cached data points */
    DataPoint * points;
    int npoints;

    /* The number of points used in this dataset during the most recent
       FCN call. The points must not only pass the filter but also not
       to choke the fitting function. */
    int datapoints;

    /* Cached weight assignments for the numerical integration of
       the fitting function. Will be needed for the unbinned max.
       likelihood method. */
    DataPoint * iweights;
    int nweights;

    /* Integration region limits and the number of intervals
       for function normalization using unbinned max. likelihood.
       These are also used to cache respective histogram quantities
       if id refers to a Histo-Scope histogram. All these numbers
       become meaningful only after the "hs_init_fit_data_sets"
       function call is made. */
    double xmin;
    double xmax;
    size_t nx;
    double ymin;
    double ymax;
    size_t ny;
    double zmin;
    double zmax;
    size_t nz;

    /* Cached Histo-Scope item type */
    int item_type;

    /* Chi-square calculated according to the used minimization method */
    int chisq_used_points;
    double chisq;

    /* Some basic statistics. Filled at the end of a fit. */
    BasicStats data_stats;
    BasicStats fit_stats;
} Fit_subset;
/* Copy constructor for the above struct */
Fit_subset * hs_copy_fit_subset(Fit_subset *subset);
/* Destructor */
void hs_destroy_fit_subset(Fit_subset *subset);


typedef struct
{
    char *name;     /* Parameter name */
    double value;   /* Starting value */
    int fixed;      /* Set to 1 to fix the parameter. For MINUIT, this
		       is done by setting the parameter step to 0.0. */
    double step;    /* Starting step size or parameter uncertainty */
    int has_bounds; /* Set to 1 if the parameter is bounded, 0 otherwise. */
    double lolim;   /* Lower bound */
    double hilim;   /* Upper bound */

    /* Elements which make sense only after the fit is finished 
     */
    int minuit_num; /* Internal Minuit parameter number (when not fixed) */
    double eparab;  /* "Parabolic" error (from error matrix) */
    double eminus;  /* The negative MINOS error (unlike Minuit, this
		       is a positive number) */
    double eplus;   /* The positive MINOS error */
    double globcc;  /* The global correlation coefficient. This is a number
		       between 0 and 1 which gives the correlation between
		       the parameter and that linear combination of other
		       parameters which is most strongly correlated with it. */
} Minuit_parameter;
/* Inplace copy constructor for the above struct.
 * Returns 0 when everything is OK or 1 when out of memory.
 */
int hs_copy_minuit_param_inplace(Minuit_parameter *dest,
				 const Minuit_parameter *src);
/* Inplace destructor */
void hs_clear_minuit_param(Minuit_parameter *param);


typedef struct
{
    /* Look at the description of the Minuit function MNSTAT
     * to see the meaning of the structure elements
     */
    double fmin;
    double fedm;
    double errdef;
    int npari;
    int nparx;
    int istat;
} Minimization_status;


/* A major headache in the whole system is keeping track of synchronization
 * between Minuit and multiple fit configurations. There are three
 * flags which should be always maintained in the correct state in the
 * Fit_config structure declared below: options_synched, par_synched, 
 * and complete (the meanings of the flags are explained in the comments
 * to the struct). Any function which acts upon Fit_config structures must 
 * be aware of these flags, and set them appropriately when the structure 
 * goes out of sync with Minuit. The "complete" flag should never be set
 * or reset directly, only using the "hs_declare_fit_complete" function.
 */
struct _fit_config  /* typedefed to Fit_config */
{
    char *title;       /* A title string. Will be used in the plots and
                          in the MINUIT printouts */
    char *description; /* Some arbitrary user info */

    /* The default statistic to minimize. This
       statistic will be used with all new datasets. */
    char *default_method; 
    Accumulated_stat_function * default_fit_method;

    char *minimizer;               /* Minuit minimizer. Should be "migrad",
				      "mini", or "simplex". Default is
				      "migrad". This pointer is special:
				      the string is not owned by this
				      structure */
    int nominos;                   /* Set to a non-zero value 
				      in order NOT to run Minos */

    double errdef;                 /* The value of Minuit parameter UP
				      defining parameter uncertainties.
				      Default value is 1.0. */
    int verbose;                   /* Minuit verbose level:
				      -1 : no output except from SHOW commands
				       0 : minimum output (no starting values
                                           or intermediate results)
				       1 : Minuit "normal" output
				       2 : print intermediate results
				       3 : show the progress of minimization
				   */
    int nowarnings;                /* If set to 1, instructs Minuit to
				      suppress warning messages. */
    int strategy;                  /* Minuit strategy level. Lower values mean
				      fewer function calls, higher values mean
				      mean more reliable minimization. Allowed
				      values are 0, 1 (default), and 2. */
    double eps;                    /* Accuracy of FCN calculation. Minuit
				      sets it internally to the machine
				      accuracy which sometimes may be too
				      optimistic. */
    int has_grad;                  /* If 1, informs Minuit that it should use
				      user-defined gradient functions. */
    int options_synched;           /* Set to 1 when options are synchronized
				      with Minuit, and to 0 when something
				      changes. */

    Minuit_parameter *minuit_pars; /* Minuit parameters              */
    int n_minuit_pars;             /* The total number of parameters */
    int par_synched;               /* Flag which is set when parameter
				      values are given to MINUIT and
				      reset when parameter set changes */

    int n_variable_pars;           /* Number of variable parameters
				      according to MINUIT */
    double *emat;                  /* Parameter error matrix, of the size
				      n_variable_pars x n_variable_pars */
    int *ematind;                  /* Parameter numbers for which the error
				      matrix is defined (this is an array
				      with length n_variable_pars) */

    Tcl_Obj *dll_list;     /* A bunch of dll numbers. These dlls contain
			      various mapping functions which are no longer
			      needed when the fit is deleted, so these
			      dlls will be unloaded. */

    Fit_subset **fitsets;  /* Associated datasets. Pointers are allowed
			      to be NULL in which case they are skipped. */
    int nsets;

    Fitter_info **fitters; /* Associated fitting functions */
    int nfitters;

    /* Elements related to error handling */
    int ignore_function_errors;   /* If this is not 0, the errors reported
                                     by functions will be ignored */
    int status;                   /* Will be either TCL_OK or TCL_ERROR */
    Tcl_Interp *interp;           /* Interpreter to use for error reporting.
                                     Check its result in case status is equal
				     TCL_ERROR. */
    int complete;                 /* Set when MINUIT completes the fit. Reset
				     when the fit is redefined in any way. */
    Tcl_Obj *cb_complete;         /* A list of tcl scripts which will be
				     executed whenever the "complete" value
				     changes to 1 */
    Tcl_Obj *cb_lostsync;         /* A list of tcl scripts which will be
				     executed whenever the "complete" value
				     changes to 0 */
    Tcl_Obj *cb_delete;           /* A list of tcl scripts which will be
				     executed just before the fit structure
				     is destroyed */
    Minimization_status ministat; /* Minimization status. Defined only
				     when "complete" is set to 1. */
    int timeout;                  /* The number of seconds after which
				     the fit procedure times out. 0 or
				     negative values mean that timeout
				     is disabled */

    /* The following element is used to hold tcl-only data */
    Tcl_HashTable tcldata;

    /* A bunch of arguments to use with simple user-defined fcns */
    Minuit_tcl_fcn *user_tcl_fcn;
    void *clientData;
    int objc;
    Tcl_Obj **objv;
};
/* Copy constructor for the above struct */
Fit_config * hs_copy_fit_config(Fit_config *config);
/* Destructor */
void hs_destroy_fit_config(Fit_config *config);

/* Functions for augmenting structures */
/* The following function changes an existing parameter or adds
   a new one. It leaves an error message in interp and returns 
   TCL_ERROR in case something goes wrong. */
int hs_set_fit_parameter(Tcl_Interp *interp,
			 Fit_config *fit_config,
			 const Minuit_parameter *param);
void hs_delete_fit_parameter(Fit_config *fit_config, int parnum);

/* Functions related to fit naming */
int hs_fit_list(Tcl_Interp *interp);
Fit_config * hs_find_fit_byname(const char *name);
/* The following function returns 1 if name doesn't exist */
int hs_remove_fit_byname(const char *name); 
/* The following function returns TCL_ERROR and leaves
   an error message in interp if name already exists */
int hs_add_fit(Tcl_Interp *interp, const char *name, Fit_config *config);

/* Current fit lookup functions needed by the MINUIT API */
Fit_config * hs_current_fit_conf(void);
const char * hs_current_fit_name(void);
/* The following sets current fit by name or unsets it if name
   is an empty string or NULL. Returns 0 if everything is OK. */
int hs_set_current_fit(const char *name);
/* The following function should be called only when 
   the current fit is renamed and nowhere else */
int hs_reassign_current_fit_name(const char *name);

/* The following function (re)initializes basic fit settings before
   user can actually (re)run Minuit on the current fit. This call
   should be followed by parameter and option synchronization. */
void hs_reset_current_fit(void);
int hs_copy_fit(Tcl_Interp *interp, const char *newname,
		const char *oldname);
/* In the following function, renaming of the "current" fit
   does not change the fact that the fit is "current" -- that is,
   being "current" is a property of the fit object, not the name. */
int hs_rename_fit(Tcl_Interp *interp, const char *newname,
		  const char *oldname);

/* Function to check whether all subset filters and
   function mappings are compiled. Returns 1 if yes, 0 if no. */
int hs_is_fit_compiled(const Fit_config *fit);

/* Function to initialize fit subset data */
void hs_init_fit_data_sets(Fit_config *fit_info);

/* Functions for getting object descriptions in tcl */
int hs_fit_subset_description(Tcl_Interp *interp,
			      const Fit_subset *fitset,
			      Tcl_Obj **descr);
int hs_fitter_info_description(Tcl_Interp *interp,
			       const Fit_config *config,
			       const Fitter_info *info,
			       Tcl_Obj **descr);
int hs_minuit_param_description(Tcl_Interp *interp,
				const Minuit_parameter *param,
				int includeName,
				Tcl_Obj **descr);
int hs_fit_description(Tcl_Interp *interp,
		       const Fit_config *config,
		       Tcl_Obj **descr);
int hs_minim_status_description(Tcl_Interp *interp,
				const Minimization_status *ps,
				Tcl_Obj **descr);

/* Functions for building objects from tcl lists */
int hs_add_fit_subset_tcl(Tcl_Interp *interp,
			  Fit_config *fit_config,
			  Tcl_Obj *obj);
int hs_set_fit_parameter_tcl(Tcl_Interp *interp,
			     Fit_config *fit_config,
			     char *name, Tcl_Obj *obj);

/* Function to figure out function pointers from function names
 * in the subsets. When something fails, it sets config->status
 * to TCL_ERROR and reports the failure in config->interp. This
 * function should be called in the FCN init stage.
 */
void hs_synch_fit_subsets(Fit_config *config);

/* Other function related to fitter synchronization */
Fitter_info * hs_find_fit_fitter_byname(Fit_config *config,
					const char *name);
int hs_fitter_exists_in_subset(Fit_subset *subset,
			       const char *name);
int hs_fit_fitter_usage_count(Fit_config *config,
			      const char *name);

/* The next function returns 0 on success and
   1 if we have the out-of-memory condition. subset
   pointer must not be NULL. */
int hs_add_fitter_to_subset(Fit_subset *subset, const char *name);
void hs_remove_fitter_from_subset(Fit_subset *subset, const char *name);

/* The next function returns 0 on success and 1 if we have
   the out-of-memory condition. Fit_config assumes the ownership
   of the struct pointed to by pfitter. */
int hs_add_fitter_to_config(Fit_config *config, Fitter_info *pfitter);
void hs_remove_fitter_from_config(Fit_config *config, const char *name);

/* The following function destroys a subset and removes
   fitters which do not map into any subset from the Fit_config */
void hs_destroy_subset_remove_fitters(Fit_config *config, int set);

/* A function to check if the current mapping 
   corresponds to some default mapping */
Parameter_map_function * hs_default_mapping_function(const char *mapping);

/* Some functions which simplify Minuit interface */
void hs_minuit_quiet();
void hs_minuit_verbose(int level);
void hs_minuit_set_precision(double eps);
void hs_minuit_set_title(const char *title);

/* A function to compare Minuit parameter names in a FORTRAN-like manner */
int hs_minuit_par_names_cmp(const char *p1, const char *p2);

/* Functions which checks if the given fitting method can be used on
   a given dataset. Returns a pointer to the accumulator function if yes,
   NULL if not. In the latter case an error message is left in interp. */
Accumulated_stat_function * hs_check_fitting_method(
    Tcl_Interp *interp, Fit_subset *fitset, const char *newmethod);

/* The following function should be used to 
   indicate that the fit is complete */
void hs_declare_fit_complete(Fit_config *config, int iscomplete);

/* Function to generate a fit title */
char * hs_default_fit_title(const char *fit_name);

/* Function to execute fit or other callbacks */
void hs_exe_tcl_callbacks(Tcl_Interp *interp, Tcl_Obj *callbacks,
			   const char *name, const char *type);

/* Function to calculate basic data and fit statistics */
void hs_data_point_stats(const DataPoint *data, size_t npoints,
	 		 size_t ndim, size_t offset, BasicStats *stats);

/* Function to grab/release Minuit. Returns 0 on success. */
int hs_minuit_lock(int lock);

/* Function to check Minuit lock status */
int hs_minuit_is_locked(void);

#endif /* _FIT_CONFIG_H */
