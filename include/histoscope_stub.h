/****************************************************************************
*									    *
*  histoscope_stub.h  -- dynamic interface for client histoscope routines   *
*                                                                           *
****************************************************************************/

#ifndef HISTOSCOPE_STUB_H
#define HISTOSCOPE_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

void hs_stub_initialize(const char* id_string);
int hs_stub_server_port(void);
void hs_stub_update(void);
void hs_stub_complete(void);
void hs_stub_complete_and_wait(void);
void hs_stub_histoscope(int return_immediately);
void hs_stub_histoscope_hidden(void);
void hs_stub_histo_with_config(int return_immediately, const char* config_file);
int hs_stub_num_connected_scopes(void);
void hs_stub_load_config_string(const char* cfgStr);
void hs_stub_load_config_file(const char* cfgFile);
void hs_stub_kill_histoscope(void);
int hs_stub_create_1d_hist(int uid, const char* title, const char* category, 
                      const char* x_label, const char* y_label,
                      int n_bins, float min, float max);
int hs_stub_create_2d_hist(int uid, const char* title, const char* category,
                      const char* x_label, const char* y_label,
                      const char* z_label, int x_bins, int y_bins,
                      float x_min, float x_max, float y_min, float y_max);
int hs_stub_create_3d_hist(int uid, const char* title, const char* category,
		      const char* x_label, const char* y_label,
		      const char* z_label, const char* v_label,
		      int x_bins, int y_bins, int z_bins,
		      float x_min, float x_max, float y_min,
		      float y_max, float z_min, float z_max);
int hs_stub_create_ntuple(int uid, const char* title, const char* category,
                     int n_variables, char** names);
int hs_stub_create_indicator(int uid, const char* title, const char* category,
                        float min, float max);
int hs_stub_create_control(int uid, const char* title, const char* category, 
                      float min, float max, float default_value);
int hs_stub_create_trigger(int uid, const char* title, const char* category);
int hs_stub_create_group(int uid, const char* title, const char* category,
                    int groupType, int numItems, int *itemId, int *errsDisp);
void hs_stub_fill_1d_hist(int id, float x, float weight);
void hs_stub_fill_2d_hist(int id, float x, float y, float weight);
void hs_stub_fill_3d_hist(int id, float x, float y, float z, float weight);
void hs_stub_1d_hist_set_bin(int id, int ix, float value);
void hs_stub_2d_hist_set_bin(int id, int ix, int iy, float value);
void hs_stub_3d_hist_set_bin(int id, int ix, int iy, int iz, float value);
void hs_stub_1d_hist_set_bin_errors(int id, int ix,
                               float pos, float neg, int which);
void hs_stub_2d_hist_set_bin_errors(int id, int ix, int iy,
                               float pos, float neg, int which);
void hs_stub_3d_hist_set_bin_errors(int id, int ix, int iy, int iz,
                               float pos, float neg, int which);
void hs_stub_hist_set_slice(int id, int bin0, int stride, int count, float *data);
void hs_stub_hist_set_slice_errors(int id, int bin0, int stride, int count,
                              float *err_valsP, float *err_valsM);
int hs_stub_fill_ntuple(int id, float *values);
void hs_stub_set_indicator(int id, float value);
void hs_stub_read_control(int id, float *value);
int hs_stub_check_trigger(int id);
void hs_stub_set_1d_errors(int id, float *err_valsP, float *err_valsM);
void hs_stub_set_2d_errors(int id, float *err_valsP, float *err_valsM);
void hs_stub_set_3d_errors(int id, float *err_valsP, float *err_valsM);
void hs_stub_reset(int id);
int hs_stub_save_file(const char* name);
void hs_stub_delete(int id);
void hs_stub_delete_items(int *ids, int num_ids);
void hs_stub_change_uid(int id, int newuid);
void hs_stub_change_category(int id, const char* newcategory);
void hs_stub_change_title(int id, const char* newtitle);
int hs_stub_id(int uid, const char* category);
int hs_stub_id_from_title(const char* title, const char* category);
int hs_stub_list_items(const char* title, const char*  category, int *ids, int num, 
		  int matchFlg);
int hs_stub_uid(int id);
int hs_stub_category(int id, char* category_string);
int hs_stub_title(int id, char* title_string);
int hs_stub_type(int id);
int hs_stub_read_file(const char* filename, const char* prefix);
int hs_stub_read_file_items(const char* filename, const char* prefix,
                       const char* category, int *uids, int n_uids);
int hs_stub_save_file_items(const char* filename, const char* category,
                       int *uids, int n_uids);
void hs_stub_delete_category(const char* category);
int hs_stub_num_items(void);
void hs_stub_1d_hist_block_fill(int id, float *data, float *err, float *err_m);
void hs_stub_2d_hist_block_fill(int id, float *data, float *err, float *err_m);
void hs_stub_3d_hist_block_fill(int id, float *data, float *err, float *err_m);
int hs_stub_1d_hist_num_bins(int id);
void hs_stub_2d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins);
void hs_stub_3d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins,
			 int *num_z_bins);
void hs_stub_1d_hist_range(int id, float *min, float *max);
void hs_stub_2d_hist_range(int id, float *x_min, float *x_max, float *y_min,
	              float *y_max);
void hs_stub_3d_hist_range(int id, float *x_min, float *x_max, float *y_min,
	              float *y_max, float *z_min, float *z_max);
int hs_stub_num_entries(int id);
void hs_stub_1d_hist_bin_contents(int id, float *data);
void hs_stub_2d_hist_bin_contents(int id, float *data);
void hs_stub_3d_hist_bin_contents(int id, float *data);
int hs_stub_1d_hist_errors(int id, float *err, float *err_m);
int hs_stub_2d_hist_errors(int id, float *err, float *err_m);
int hs_stub_3d_hist_errors(int id, float *err, float *err_m);
void hs_stub_1d_hist_overflows(int id, float *underflow, float *overflow);
void hs_stub_2d_hist_overflows(int id, float overflows[3][3]);
void hs_stub_3d_hist_overflows(int id, float overflows[3][3][3]);
float hs_stub_1d_hist_x_value(int id, float x);
float hs_stub_2d_hist_xy_value(int id, float x, float y);
float hs_stub_3d_hist_xyz_value(int id, float x, float y, float z);
float hs_stub_1d_hist_bin_value(int id, int bin_num);
float hs_stub_2d_hist_bin_value(int id, int x_bin_num, int y_bin_num);
float hs_stub_3d_hist_bin_value(int id, int x_bin_num, int y_bin_num, int z_bin_num);
void hs_stub_1d_hist_minimum(int id, float *x, int *bin_num, float *value);
void hs_stub_2d_hist_minimum(int id, float *x, float *y, int *x_bin_num,
                        int *y_bin_num, float *value);
void hs_stub_3d_hist_minimum(int id, float *x, float *y, float *z, int *x_bin_num,
			int *y_bin_num, int *z_bin_num, float *value);
void hs_stub_1d_hist_maximum(int id, float *x, int *bin_num, float *value);
void hs_stub_2d_hist_maximum(int id, float *x, float *y, int *x_bin_num,
                        int *y_bin_num, float *value);
void hs_stub_3d_hist_maximum(int id, float *x, float *y, float *z, int *x_bin_num,
                        int *y_bin_num, int *z_bin_num, float *value);
void hs_stub_1d_hist_stats(int id, float *mean, float *std_dev);
void hs_stub_2d_hist_stats(int id, float *x_mean, float *y_mean,
		      float *x_std_dev, float *y_std_dev);
void hs_stub_3d_hist_stats(int id, float *x_mean, float *y_mean, float *z_mean,
		      float *x_std_dev, float *y_std_dev, float *z_std_dev);
float hs_stub_hist_integral(int id);
void hs_stub_hist_set_gauss_errors(int id);
int hs_stub_sum_histograms(int uid, const char* title, const char* category,
                      int id1, int id2, float const1, float const2);
int hs_stub_multiply_histograms(int uid, const char* title, const char* category,
                           int id1, int id2, float consta);
int  hs_stub_divide_histograms(int uid, const char* title, const char* category,
                          int id1, int id2, float consta);
int hs_stub_1d_hist_derivative(int uid, const char* title,
                          const char* category, int id);
void hs_stub_1d_hist_set_overflows(int id, float underflow, float overflow);
void hs_stub_2d_hist_set_overflows(int id, float overflows[3][3]);
void hs_stub_3d_hist_set_overflows(int id, float overflows[3][3][3]);
int hs_stub_num_variables(int id);
int hs_stub_variable_name(int id, int column, char* name);
int hs_stub_variable_index(int id, const char* name);
float hs_stub_ntuple_value(int id, int row, int column);
void hs_stub_ntuple_contents(int id, float *data);
void hs_stub_row_contents(int id, int row, float *data);
void hs_stub_column_contents(int id, int column, float *data);
int hs_stub_merge_entries(int uid, const char* title, const char* category,
                     int id1, int id2);
void hs_stub_sum_category(const char* cat_top1, const char* cat_top2,
                     const char* prefixsum); 
void hs_stub_sum_file(const char* file, const char* cat_top, const char* prefixsum);
int hs_stub_1d_hist_labels(int id, char* xlabel, char* ylabel);
int hs_stub_2d_hist_labels(int id, char* xlabel, char* ylabel, char* zlabel);
int hs_stub_3d_hist_labels(int id, char* xlabel, char* ylabel,
		      char* zlabel, char* vlabel);
int hs_stub_copy_hist(int old_id, int new_uid, const char* newtitle, const char* newcategory);
int hs_stub_save_file_byids(const char* name, int *ids, int n_ids);
void hs_stub_reset_const(int id, float c);
int hs_stub_unary_op(int uid, const char* title, const char* category, int id,
		float (*f_user_data)(float, float),
		float (*f_user_errors)(float, float));
int hs_stub_binary_op(int uid, const char* title, const char* category,
                 int id1, int id2, 
		 float (*f_user_data)(float, float, float, float),
		 float (*f_user_errors)(float, float, float, float));
void hs_stub_change_uid_and_category(int id, int newuid, const char* newcategory);
void hs_stub_allow_item_send(int flag);
int hs_stub_socket_status(void);
int hs_stub_hist_error_status(int id);
void hs_stub_allow_reset_refresh(int id, int flag);
void hs_stub_task_completion_callback(void (*f)(int, int, int, char*, void*), void*);
int hs_stub_fill_hist_slice(int parent_id, int axis1, int bin1,
		       int axis2, int bin2, int slice_id);
int hs_stub_slice_contents(int parent_id, int axis1, int bin1, int axis2, int bin2,
		      int arrsize, float *data, float *poserr, float *negerr,
		      int *slicesize);
int hs_stub_pack_item(int id, void **mem);
int hs_stub_unpack_item(void *mem, int len, const char* prefix);

#ifdef __cplusplus
}
#endif

#ifndef HISTOSCOPE_H
#define HISTOSCOPE_H

#define FIXED_HISTOSCOPE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HS_TYPES_DEFINED
#define HS_TYPES_DEFINED

typedef enum _hsGroupType { 
    HS_INDIVIDUAL, HS_MULTI_PLOT, HS_OVERLAY_PLOT
} hsGroupType;

typedef enum _hsErrorType {
    HS_NO_ERRORS, HS_POS_ERRORS, HS_BOTH_ERRORS, HS_ITEMNOTFOUND_ERRORS
} hsErrorType;

typedef enum _hsAxisType {
    HS_AXIS_NONE = 0, HS_AXIS_X, HS_AXIS_Y, HS_AXIS_Z, N_HS_AXIS_TYPES
} hsAxisType;

typedef enum _hsItemType {
    HS_1D_HISTOGRAM = 0, HS_2D_HISTOGRAM, HS_NTUPLE, HS_INDICATOR, HS_CONTROL,
    HS_TRIGGER, HS_NONE, HS_NFIT, HS_GROUP, HS_CONFIG_STRING, HS_3D_HISTOGRAM,
    N_HS_DATA_TYPES
} hsItemType;

#endif

typedef enum _hsErrorBars {
    HS_NO_ERROR_BARS, HS_DATA_ERROR_BARS, HS_GAUSSIAN_ERROR_BARS
} hsErrorBars;

#define hs_initialize hs_stub_initialize
#define hs_server_port hs_stub_server_port
#define hs_update hs_stub_update
#define hs_complete hs_stub_complete
#define hs_complete_and_wait hs_stub_complete_and_wait
#define hs_histoscope hs_stub_histoscope
#define hs_histoscope_hidden hs_stub_histoscope_hidden
#define hs_histo_with_config hs_stub_histo_with_config
#define hs_num_connected_scopes hs_stub_num_connected_scopes
#define hs_load_config_string hs_stub_load_config_string
#define hs_load_config_file hs_stub_load_config_file
#define hs_kill_histoscope hs_stub_kill_histoscope
#define hs_create_1d_hist hs_stub_create_1d_hist
#define hs_create_2d_hist hs_stub_create_2d_hist
#define hs_create_3d_hist hs_stub_create_3d_hist
#define hs_create_ntuple hs_stub_create_ntuple
#define hs_create_indicator hs_stub_create_indicator
#define hs_create_control hs_stub_create_control
#define hs_create_trigger hs_stub_create_trigger
#define hs_create_group hs_stub_create_group
#define hs_fill_1d_hist hs_stub_fill_1d_hist
#define hs_fill_2d_hist hs_stub_fill_2d_hist
#define hs_fill_3d_hist hs_stub_fill_3d_hist
#define hs_1d_hist_set_bin hs_stub_1d_hist_set_bin
#define hs_2d_hist_set_bin hs_stub_2d_hist_set_bin
#define hs_3d_hist_set_bin hs_stub_3d_hist_set_bin
#define hs_1d_hist_set_bin_errors hs_stub_1d_hist_set_bin_errors
#define hs_2d_hist_set_bin_errors hs_stub_2d_hist_set_bin_errors
#define hs_3d_hist_set_bin_errors hs_stub_3d_hist_set_bin_errors
#define hs_hist_set_slice hs_stub_hist_set_slice
#define hs_hist_set_slice_errors hs_stub_hist_set_slice_errors
#define hs_fill_ntuple hs_stub_fill_ntuple
#define hs_set_indicator hs_stub_set_indicator
#define hs_read_control hs_stub_read_control
#define hs_check_trigger hs_stub_check_trigger
#define hs_set_1d_errors hs_stub_set_1d_errors
#define hs_set_2d_errors hs_stub_set_2d_errors
#define hs_set_3d_errors hs_stub_set_3d_errors
#define hs_reset hs_stub_reset
#define hs_save_file hs_stub_save_file
#define hs_delete hs_stub_delete
#define hs_delete_items hs_stub_delete_items
#define hs_change_uid hs_stub_change_uid
#define hs_change_category hs_stub_change_category
#define hs_change_title hs_stub_change_title
#define hs_id hs_stub_id
#define hs_id_from_title hs_stub_id_from_title
#define hs_list_items hs_stub_list_items
#define hs_uid hs_stub_uid
#define hs_category hs_stub_category
#define hs_title hs_stub_title
#define hs_type hs_stub_type
#define hs_read_file hs_stub_read_file
#define hs_read_file_items hs_stub_read_file_items
#define hs_save_file_items hs_stub_save_file_items
#define hs_delete_category hs_stub_delete_category
#define hs_num_items hs_stub_num_items
#define hs_1d_hist_block_fill hs_stub_1d_hist_block_fill
#define hs_2d_hist_block_fill hs_stub_2d_hist_block_fill
#define hs_3d_hist_block_fill hs_stub_3d_hist_block_fill
#define hs_1d_hist_num_bins hs_stub_1d_hist_num_bins
#define hs_2d_hist_num_bins hs_stub_2d_hist_num_bins
#define hs_3d_hist_num_bins hs_stub_3d_hist_num_bins
#define hs_1d_hist_range hs_stub_1d_hist_range
#define hs_2d_hist_range hs_stub_2d_hist_range
#define hs_3d_hist_range hs_stub_3d_hist_range
#define hs_num_entries hs_stub_num_entries
#define hs_1d_hist_bin_contents hs_stub_1d_hist_bin_contents
#define hs_1d_hist_errors hs_stub_1d_hist_errors
#define hs_2d_hist_bin_contents hs_stub_2d_hist_bin_contents
#define hs_2d_hist_errors hs_stub_2d_hist_errors
#define hs_3d_hist_bin_contents hs_stub_3d_hist_bin_contents
#define hs_3d_hist_errors hs_stub_3d_hist_errors
#define hs_1d_hist_overflows hs_stub_1d_hist_overflows
#define hs_2d_hist_overflows hs_stub_2d_hist_overflows
#define hs_3d_hist_overflows hs_stub_3d_hist_overflows
#define hs_1d_hist_x_value hs_stub_1d_hist_x_value
#define hs_2d_hist_xy_value hs_stub_2d_hist_xy_value
#define hs_3d_hist_xyz_value hs_stub_3d_hist_xyz_value
#define hs_1d_hist_bin_value hs_stub_1d_hist_bin_value
#define hs_2d_hist_bin_value hs_stub_2d_hist_bin_value
#define hs_3d_hist_bin_value hs_stub_3d_hist_bin_value
#define hs_1d_hist_minimum hs_stub_1d_hist_minimum
#define hs_2d_hist_minimum hs_stub_2d_hist_minimum
#define hs_3d_hist_minimum hs_stub_3d_hist_minimum
#define hs_1d_hist_maximum hs_stub_1d_hist_maximum
#define hs_2d_hist_maximum hs_stub_2d_hist_maximum
#define hs_3d_hist_maximum hs_stub_3d_hist_maximum
#define hs_1d_hist_stats hs_stub_1d_hist_stats
#define hs_2d_hist_stats hs_stub_2d_hist_stats
#define hs_3d_hist_stats hs_stub_3d_hist_stats
#define hs_hist_integral hs_stub_hist_integral
#define hs_hist_set_gauss_errors hs_stub_hist_set_gauss_errors
#define hs_sum_histograms hs_stub_sum_histograms
#define hs_multiply_histograms hs_stub_multiply_histograms
#define hs_divide_histograms hs_stub_divide_histograms
#define hs_1d_hist_derivative hs_stub_1d_hist_derivative
#define hs_1d_hist_set_overflows hs_stub_1d_hist_set_overflows
#define hs_2d_hist_set_overflows hs_stub_2d_hist_set_overflows
#define hs_3d_hist_set_overflows hs_stub_3d_hist_set_overflows
#define hs_num_variables hs_stub_num_variables
#define hs_variable_name hs_stub_variable_name
#define hs_variable_index hs_stub_variable_index
#define hs_ntuple_value hs_stub_ntuple_value
#define hs_ntuple_contents hs_stub_ntuple_contents
#define hs_row_contents hs_stub_row_contents
#define hs_column_contents hs_stub_column_contents
#define hs_merge_entries hs_stub_merge_entries
#define hs_sum_category hs_stub_sum_category
#define hs_sum_file hs_stub_sum_file
#define hs_1d_hist_labels hs_stub_1d_hist_labels
#define hs_2d_hist_labels hs_stub_2d_hist_labels
#define hs_3d_hist_labels hs_stub_3d_hist_labels
#define hs_copy_hist hs_stub_copy_hist
#define hs_save_file_byids hs_stub_save_file_byids
#define hs_reset_const hs_stub_reset_const
#define hs_unary_op hs_stub_unary_op
#define hs_binary_op hs_stub_binary_op
#define hs_change_uid_and_category hs_stub_change_uid_and_category
#define hs_allow_item_send hs_stub_allow_item_send
#define hs_socket_status hs_stub_socket_status
#define hs_hist_error_status hs_stub_hist_error_status
#define hs_allow_reset_refresh hs_stub_allow_reset_refresh
#define hs_task_completion_callback hs_stub_task_completion_callback
#define hs_fill_hist_slice hs_stub_fill_hist_slice
#define hs_slice_contents hs_stub_slice_contents
#define hs_pack_item hs_stub_pack_item
#define hs_unpack_item hs_stub_unpack_item

#ifdef __cplusplus
}
#endif

#endif  /* not HISTOSCOPE_H */

#endif  /* not HISTOSCOPE_STUB_H */
