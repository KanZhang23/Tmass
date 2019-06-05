#ifndef HISTO_UTILS_H
#define HISTO_UTILS_H

#include "tcl.h"

#define RANGE2SIG 0.7413011f

enum histo_projection_types
{
  HS_CALC_PROJ_AVE,
  HS_CALC_PROJ_RMS,
  HS_CALC_PROJ_SUM,
  HS_CALC_PROJ_MED,
  HS_CALC_PROJ_RANGE,
  HS_CALC_PROJ_MIN,
  HS_CALC_PROJ_MAX,
  HS_CALC_PROJ_COORDAVE,
  HS_CALC_PROJ_COORDRMS,
  HS_CALC_PROJ_COORDMED,
  HS_CALC_PROJ_COORDRANGE
};

struct weighted_point
{
    float x;
    float w;
};

/* Arrays of the following structure are used to model
   cumulative density functions and other integrals */
struct simpson_bin
{
    double integ;  /* the value of the integral so far, including this bin */
    double d1;     /* function first derivative in this bin                */
    double d2;     /* function second derivative in this bin               */
};

#define verify_1d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_1D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 1d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define verify_2d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_2D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 2d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define verify_3d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_3D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 3d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define verify_ntuple(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_NTUPLE)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not an ntuple", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define flip_endian(a) ( ((a) << 24) | \
                        (((a) << 8) & 0x00ff0000) | \
                        (((a) >> 8) & 0x0000ff00) | \
               ((unsigned)(a) >> 24) )

int histo_dim_from_type(int type);
int get_axis_from_obj(Tcl_Interp *interp, Tcl_Obj *obj,
		      int ndim, int strict, int *axis);
int find_duplicate_name(char **names, int count);
int dump_1d_histo(FILE *ofile, int id);
int dump_2d_histo(FILE *ofile, int id);
int dump_3d_histo(FILE *ofile, int id);
int dump_ntuple(FILE *ofile, int id);
int closestint(double f, double epsilon, int *veryclose);
int hs_find_value_index_in_sorted_array(float *arr, int n, float value);
float find_percentile(const float *coords, const float *cdf, int n, float q);
int arr_medirange(const float *array, int n, int key,
      float *qmin, float *q25, float *qmed, float *q75, float *qmax);
int arr_medirange_weighted(const struct weighted_point *array, int n,
      float *qmin, float *q25, float *qmed, float *q75, float *qmax);
int arr_stats(const float *array, int n, int key, float *mean, float *err);
int arr_stats_weighted(const struct weighted_point *array, int n,
		       float *fmean, float *ferr);
int arr_percentiles(float *array, int n, int key,
		    float *percentages, int len, float *answer);
int arr_2d_weighted_covar(const float *data, const float *weights,
			  const size_t ncols, const size_t nrows,
			  const size_t *column_list, const size_t nused,
			  float *averages, float *covar);
int hs_next_category_uid(char *category);
int project_2d_histogram(int uid, char *title, char *category, 
			 int id, int axis, int proj_type, int suppress_zero);
int project_3d_histogram(int uid, char *title, char *category,
			 int id, int axis1, int axis2, int proj_type,
			 int suppress_zero);
int slice_2d_histogram(int uid, char *title, char *category, 
                    int id, int axis, int binnum);
int hs_transpose_histogram(int uid, char *title, char *category, int id);
int get_data_slice(int id, int axis, int binnum, float **dataslice, 
		   float **poserr, float **negerr);
int hs_1d_hist_subrange(int uid, char *title, char *category,
                        int id, int bin_min, int bin_max);
int hs_2d_hist_subrange(int uid, char *title, char *category,
                        int id, int axis, int bin_min, int bin_max);
int hs_3d_hist_subrange(int uid, char *title, char *category,
                        int id, int axis, int bin_min, int bin_max);
int hs_concat_histograms(int uid, char *title, char *category,
			 int id1, int id2);
int hs_duplicate_axes(int id, int uid, char *title, char *category);
int hs_duplicate_ntuple_header(int id, int uid, char *title, char *category);
int hs_copy_data(int id_to, int id_from, float mult);
int hs_hist_num_bins(int id);
float hs_hist_bin_width(int id);
void hs_swap_data_errors(int id, int which);
void hs_hs_update(void);
int hs_have_cernlib(void);
int hs_is_valid_c_name(Tcl_Interp *interp, const char *name, const char *what);
void task_completion_callback(int connect_id, int task_number,
			      int status, char *result, void *cbdata);
int ntuple_hashtable_key_length(int nkey_columns);
int get_float_array_from_binary_or_list(
    Tcl_Interp *interp, Tcl_Obj *obj,
    float **parray, int *psize, int *pfreeItLater);

/* The following function allocates an array 
   of doubles which must be deallocated using "free".
   "mrep" should be set to 'c' or 'C' to return C array
   representation, and to 'f' or 'F' to return FORTRAN
   representation. TCL_OK is returned on success
   and TCL_ERROR on failure.
*/
int parse_matrix(Tcl_Interp *interp, Tcl_Obj *obj, char mrep,
		 int *nrows, int *ncols, double **data);
Tcl_Obj *new_matrix_obj(char mrep, int nrows, int ncols, double *data);
Tcl_Obj *new_list_of_doubles(int n, double *data);

/* In the following three functions, i0 is the value of the integral
   at the left edge of the first bin in the cdfdata array. For normal
   cdfs this should be set to 0. */
double find_cdf_value(double x, double i0, double xmin, double xmax,
		      int nbins, struct simpson_bin *cdfdata);
int find_inverse_cdf_value_linear(double fvalue, double i0,
                                  double xmin, double xmax, int nbins,
                                  struct simpson_bin *cdfdata, double *x);
int find_inverse_cdf_value(double fvalue, double i0, double xmin, double xmax,
                           int nbins, struct simpson_bin *cdfdata, double *x);

/* The following function returns 0 if there are
   two real solutions, 1 otherwise. Coefficient a is 1. */
int hs_solve_quadratic_eq(double b, double c, double *x1, double *x2);

/* Warning! The following function changes the contents 
   of the arrays (sorts them in the increasing order). */
void classic_unbinned_ks_probability(float *a1, int n1, float *a2, int n2,
				     float *ks_distance, float *ksprob);

/* Warning! The following function changes the contents 
   of the arrays (sorts them in the order of increasing x). */
void weighted_unbinned_ks_probability(
    struct weighted_point *a1, int n1, int neff1,
    struct weighted_point *a2, int n2, int neff2,
    float *ks_distance, float *ksprob);

#endif /* HISTO_UTILS_H */
