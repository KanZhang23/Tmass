#include "histoscope.h"
#include "histoscope_stub.h"

void hs_stub_initialize(const char *id_string)
{
    hs_initialize(id_string);
}

int hs_stub_server_port(void)
{
    return hs_server_port();
}

void hs_stub_update(void)
{
    hs_update();
}

void hs_stub_complete(void)
{
    hs_complete();
}

void hs_stub_complete_and_wait(void)
{
    hs_complete_and_wait();
}

void hs_stub_histoscope(int return_immediately)
{
    hs_histoscope(return_immediately);
}

void hs_stub_histoscope_hidden(void)
{
    hs_histoscope_hidden();
}

void hs_stub_histo_with_config(int return_immediately, const char *config_file)
{
    hs_histo_with_config(return_immediately, config_file);
}

int hs_stub_num_connected_scopes(void)
{
    return hs_num_connected_scopes();
}

void hs_stub_load_config_string(const char *cfgStr)
{
    hs_load_config_string(cfgStr);
}

void hs_stub_load_config_file(const char *cfgFile)
{
    hs_load_config_file(cfgFile);
}

void hs_stub_kill_histoscope(void)
{
    hs_kill_histoscope();
}

int hs_stub_create_1d_hist(int uid, const char *title, const char *category, 
			const char *x_label, const char *y_label,
			int n_bins, float min, float max)
{
    return hs_create_1d_hist(uid, title, category, 
			     x_label, y_label,
			     n_bins, min, max);
}

int hs_stub_create_2d_hist(int uid, const char *title, const char *category,
			const char *x_label, const char *y_label,
			const char *z_label, int x_bins, int y_bins, float x_min,
			float x_max, float y_min, float y_max)
{
    return hs_create_2d_hist(uid, title, category,
			     x_label, y_label, z_label,
			     x_bins, y_bins, x_min,
			     x_max, y_min, y_max);
}

int hs_stub_create_3d_hist(int uid, const char *title, const char *category,
			   const char *x_label, const char *y_label,
			   const char *z_label, const char *v_label,
			   int x_bins, int y_bins, int z_bins,
			   float x_min, float x_max, float y_min,
			   float y_max, float z_min, float z_max)
{
    return hs_create_3d_hist(uid, title, category,
			     x_label, y_label, z_label, v_label,
			     x_bins, y_bins, z_bins, x_min,
			     x_max, y_min, y_max, z_min, z_max);
}

int hs_stub_create_ntuple(int uid, const char *title, const char *category,
			 int n_variables, char **names)
{
    return hs_create_ntuple(uid, title, category,
			    n_variables, names);
}

int hs_stub_create_indicator(int uid, const char *title, const char *category,
			     float min, float max)
{
    return hs_create_indicator(uid, title, category, min, max);
}

int hs_stub_create_control(int uid, const char *title, const char *category, 
			   float min, float max, float default_value)
{
    return hs_create_control(uid, title, category,
			     min, max, default_value);
}

int hs_stub_create_trigger(int uid, const char *title, const char *category)
{
    return hs_create_trigger(uid, title, category);
}

int hs_stub_create_group(int uid, const char *title, const char *category, int groupType,
			int numItems, int *itemId, int *errsDisp)
{
    return hs_create_group(uid, title, category, groupType,
			   numItems, itemId, errsDisp);
}

void hs_stub_fill_1d_hist(int id, float x, float weight)
{
    hs_fill_1d_hist(id, x, weight);
}

void hs_stub_fill_2d_hist(int id, float x, float y, float weight)
{
    hs_fill_2d_hist(id, x, y, weight);
}

void hs_stub_fill_3d_hist(int id, float x, float y, float z, float weight)
{
    hs_fill_3d_hist(id, x, y, z, weight);
}

void hs_stub_1d_hist_set_bin(int id, int ix, float weight)
{
    hs_1d_hist_set_bin(id, ix, weight);
}

void hs_stub_2d_hist_set_bin(int id, int ix, int iy, float weight)
{
    hs_2d_hist_set_bin(id, ix, iy, weight);
}

void hs_stub_3d_hist_set_bin(int id, int ix, int iy, int iz, float weight)
{
    hs_3d_hist_set_bin(id, ix, iy, iz, weight);
}

void hs_stub_1d_hist_set_bin_errors(int id, int ix,
                                    float pos, float neg, int which)
{
    hs_1d_hist_set_bin_errors(id, ix, pos, neg, which);
}

void hs_stub_2d_hist_set_bin_errors(int id, int ix, int iy,
                                    float pos, float neg, int which)
{
    hs_2d_hist_set_bin_errors(id, ix, iy, pos, neg, which);
}

void hs_stub_3d_hist_set_bin_errors(int id, int ix, int iy, int iz,
                                    float pos, float neg, int which)
{
    hs_3d_hist_set_bin_errors(id, ix, iy, iz, pos, neg, which);
}

void hs_stub_hist_set_slice(int id, int bin0, int stride,
                            int count, float *data)
{
    hs_hist_set_slice(id, bin0, stride, count, data);
}

void hs_stub_hist_set_slice_errors(int id, int bin0, int stride, int count,
                                   float *err_valsP, float *err_valsM)
{
    hs_hist_set_slice_errors(id, bin0, stride, count, err_valsP, err_valsM);
}

int hs_stub_fill_ntuple(int id, float *values)
{
    return hs_fill_ntuple(id, values);
}

void hs_stub_set_indicator(int id, float value)
{
    hs_set_indicator(id, value);
}

void hs_stub_read_control(int id, float *value)
{
    hs_read_control(id, value);
}

int hs_stub_check_trigger(int id)
{
    return hs_check_trigger(id);
}

void hs_stub_set_1d_errors(int id, float *err_valsP, float *err_valsM)
{
    hs_set_1d_errors(id, err_valsP, err_valsM);
}

void hs_stub_set_2d_errors(int id, float *err_valsP, float *err_valsM)
{
    hs_set_2d_errors(id, err_valsP, err_valsM);
}

void hs_stub_set_3d_errors(int id, float *err_valsP, float *err_valsM)
{
    hs_set_3d_errors(id, err_valsP, err_valsM);
}

void hs_stub_reset(int id)
{
    hs_reset(id);
}

int hs_stub_save_file(const char *name)
{
    return hs_save_file(name);
}

void hs_stub_delete(int id)
{
    hs_delete(id);
}

void hs_stub_delete_items(int *ids, int num_ids)
{
    hs_delete_items(ids, num_ids);
}

void hs_stub_change_uid(int id, int newuid)
{
    hs_change_uid(id, newuid);
}

void hs_stub_change_category(int id, const char *newcategory)
{
    hs_change_category(id, newcategory);
}

void hs_stub_change_title(int id, const char *newtitle)
{
    hs_change_title(id, newtitle);
}

int hs_stub_id(int uid, const char *category)
{
    return hs_id(uid, category);
}

int hs_stub_id_from_title(const char *title, const char *category)
{
    return hs_id_from_title(title, category);
}

int hs_stub_list_items(const char *title, const char * category, int *ids, int num, 
		       int matchFlg)
{
    return hs_list_items(title, category, ids, num, matchFlg);
}

int hs_stub_uid(int id)
{
    return hs_uid(id);
}

int hs_stub_category(int id, char *category_string)
{
    return hs_category(id, category_string);
}

int hs_stub_title(int id, char *title_string)
{
    return hs_title(id, title_string);
}

int hs_stub_type(int id)
{
    return hs_type(id);
}

int hs_stub_read_file(const char *filename, const char *prefix)
{
    return hs_read_file(filename, prefix);
}

int hs_stub_read_file_items(const char *filename, const char *prefix, const char *category,
	int *uids, int n_uids)
{
    return hs_read_file_items(filename, prefix, category, uids, n_uids);
}

int hs_stub_save_file_items(const char *filename, const char *category,
	int *uids, int n_uids)
{
    return hs_save_file_items(filename, category, uids, n_uids);
}

void hs_stub_delete_category(const char *category)
{
    hs_delete_category(category);
}

int hs_stub_num_items(void)
{
    return hs_num_items();
}

void hs_stub_1d_hist_block_fill(int id, float *data, float *err, float *err_m)
{
    hs_1d_hist_block_fill(id, data, err, err_m);
}

void hs_stub_2d_hist_block_fill(int id, float *data, float *err, float *err_m)
{
    hs_2d_hist_block_fill(id, data, err, err_m);
}

void hs_stub_3d_hist_block_fill(int id, float *data, float *err, float *err_m)
{
    hs_3d_hist_block_fill(id, data, err, err_m);
}

int hs_stub_1d_hist_num_bins(int id)
{
    return hs_1d_hist_num_bins(id);
}

void hs_stub_2d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins)
{
    hs_2d_hist_num_bins(id, num_x_bins, num_y_bins);
}

void hs_stub_3d_hist_num_bins(int id, int *num_x_bins,
			      int *num_y_bins, int *num_z_bins)
{
    hs_3d_hist_num_bins(id, num_x_bins, num_y_bins, num_z_bins);
}

void hs_stub_1d_hist_range(int id, float *min, float *max)
{
    hs_1d_hist_range(id, min, max);
}

void hs_stub_2d_hist_range(int id, float *x_min, float *x_max, float *y_min,
			   float *y_max)
{
    hs_2d_hist_range(id, x_min, x_max, y_min, y_max);
}

void hs_stub_3d_hist_range(int id, float *x_min, float *x_max, float *y_min,
			   float *y_max, float *z_min, float *z_max)
{
    hs_3d_hist_range(id, x_min, x_max, y_min, y_max, z_min, z_max);
}

int hs_stub_num_entries(int id)
{
    return hs_num_entries(id);
}

void hs_stub_1d_hist_bin_contents(int id, float *data)
{
    hs_1d_hist_bin_contents(id, data);
}

int hs_stub_1d_hist_errors(int id, float *err, float *err_m)
{
    return hs_1d_hist_errors(id, err, err_m);
}

void hs_stub_2d_hist_bin_contents(int id, float *data)
{
    hs_2d_hist_bin_contents(id, data);
}

void hs_stub_3d_hist_bin_contents(int id, float *data)
{
    hs_3d_hist_bin_contents(id, data);
}

int hs_stub_2d_hist_errors(int id, float *err, float *err_m)
{
    return hs_2d_hist_errors(id, err, err_m);
}

int hs_stub_3d_hist_errors(int id, float *err, float *err_m)
{
    return hs_3d_hist_errors(id, err, err_m);
}

void hs_stub_1d_hist_overflows(int id, float *underflow, float *overflow)
{
    hs_1d_hist_overflows(id, underflow, overflow);
}

void hs_stub_2d_hist_overflows(int id, float overflows[3][3])
{
    hs_2d_hist_overflows(id, overflows);
}

void hs_stub_3d_hist_overflows(int id, float overflows[3][3][3])
{
    hs_3d_hist_overflows(id, overflows);
}

float hs_stub_1d_hist_x_value(int id, float x)
{
    return hs_1d_hist_x_value(id, x);
}

float hs_stub_2d_hist_xy_value(int id, float x, float y)
{
    return hs_2d_hist_xy_value(id, x, y);
}

float hs_stub_3d_hist_xyz_value(int id, float x, float y, float z)
{
    return hs_3d_hist_xyz_value(id, x, y, z);
}

float hs_stub_1d_hist_bin_value(int id, int bin_num)
{
    return hs_1d_hist_bin_value(id, bin_num);
}

float hs_stub_2d_hist_bin_value(int id, int x_bin_num, int y_bin_num)
{
    return hs_2d_hist_bin_value(id, x_bin_num, y_bin_num);
}

float hs_stub_3d_hist_bin_value(int id, int x_bin_num,
				int y_bin_num, int z_bin_num)
{
    return hs_3d_hist_bin_value(id, x_bin_num, y_bin_num, z_bin_num);
}

void hs_stub_1d_hist_minimum(int id, float *x, int *bin_num, float *value)
{
    hs_1d_hist_minimum(id, x, bin_num, value);
}

void hs_stub_2d_hist_minimum(int id, float *x, float *y, int *x_bin_num,
			     int *y_bin_num, float *value)
{
    hs_2d_hist_minimum(id, x, y, x_bin_num, y_bin_num, value);
}

void hs_stub_3d_hist_minimum(int id, float *x, float *y, float *z,
			     int *x_bin_num, int *y_bin_num, int *z_bin_num,
			     float *value)
{
    hs_3d_hist_minimum(id, x, y, z, x_bin_num, y_bin_num, z_bin_num, value);
}

void hs_stub_1d_hist_maximum(int id, float *x, int *bin_num, float *value)
{
    hs_1d_hist_maximum(id, x, bin_num, value);
}

void hs_stub_2d_hist_maximum(int id, float *x, float *y, int *x_bin_num,
			     int *y_bin_num, float *value)
{
    hs_2d_hist_maximum(id, x, y, x_bin_num, y_bin_num, value);
}

void hs_stub_3d_hist_maximum(int id, float *x, float *y, float *z,
			     int *x_bin_num, int *y_bin_num, int *z_bin_num,
			     float *value)
{
    hs_3d_hist_maximum(id, x, y, z, x_bin_num, y_bin_num, z_bin_num, value);
}

void hs_stub_1d_hist_stats(int id, float *mean, float *std_dev)
{
    hs_1d_hist_stats(id, mean, std_dev);
}

void hs_stub_2d_hist_stats(int id, float *x_mean, float *y_mean,
			   float *x_std_dev, float *y_std_dev)
{
    hs_2d_hist_stats(id, x_mean, y_mean, x_std_dev, y_std_dev);
}

void hs_stub_3d_hist_stats(int id, float *x_mean, float *y_mean, float *z_mean,
			  float *x_std_dev, float *y_std_dev, float *z_std_dev)
{
    hs_3d_hist_stats(id, x_mean, y_mean, z_mean,
		     x_std_dev, y_std_dev, z_std_dev);
}

float hs_stub_hist_integral(int id)
{
    return hs_hist_integral(id);
}

void hs_stub_hist_set_gauss_errors(int id)
{
    hs_hist_set_gauss_errors(id);
}

int hs_stub_sum_histograms(int uid, const char *title, const char *category,
                      int id1, int id2, float const1, float const2)
{
    return hs_sum_histograms(uid, title, category,
			     id1, id2, const1, const2);
}

int hs_stub_multiply_histograms(int uid, const char *title, const char *category,
                           int id1, int id2, float consta)
{
    return hs_multiply_histograms(uid, title, category,
				  id1, id2, consta);
}

int  hs_stub_divide_histograms(int uid, const char *title, const char *category,
                          int id1, int id2, float consta)
{
    return  hs_divide_histograms(uid, title, category,
				 id1, id2, consta);
}

int hs_stub_1d_hist_derivative(int uid, const char *title, const char *category, int id)
{
    return hs_1d_hist_derivative(uid, title, category, id);
}

void hs_stub_1d_hist_set_overflows(int id, float underflow, float overflow)
{
    hs_1d_hist_set_overflows(id, underflow, overflow);
}

void hs_stub_2d_hist_set_overflows(int id, float overflows[3][3])
{
    hs_2d_hist_set_overflows(id, overflows);
}

void hs_stub_3d_hist_set_overflows(int id, float overflows[3][3][3])
{
    hs_3d_hist_set_overflows(id, overflows);
}

int hs_stub_num_variables(int id)
{
    return hs_num_variables(id);
}

int hs_stub_variable_name(int id, int column, char *name)
{
    return hs_variable_name(id, column, name);
}

int hs_stub_variable_index(int id, const char *name)
{
    return hs_variable_index(id, name);
}

float hs_stub_ntuple_value(int id, int row, int column)
{
    return hs_ntuple_value(id, row, column);
}

void hs_stub_ntuple_contents(int id, float *data)
{
    hs_ntuple_contents(id, data);
}

void hs_stub_row_contents(int id, int row, float *data)
{
    hs_row_contents(id, row, data);
}

void hs_stub_column_contents(int id, int column, float *data)
{
    hs_column_contents(id, column, data);
}

int hs_stub_merge_entries(int uid, const char *title, const char *category,
                                           int id1, int id2)
{
    return hs_merge_entries(uid, title, category, id1, id2);
}

void hs_stub_sum_category(const char *cat_top1, const char *cat_top2, const char *prefixsum)
{
    hs_sum_category(cat_top1, cat_top2, prefixsum);
}
 
void hs_stub_sum_file(const char *file, const char *cat_top, const char *prefixsum)
{
    hs_sum_file(file, cat_top, prefixsum);
}

int hs_stub_1d_hist_labels(int id, char *xlabel, char *ylabel)
{
    return hs_1d_hist_labels(id, xlabel, ylabel);
}

int hs_stub_2d_hist_labels(int id, char *xlabel, char *ylabel, char *zlabel)
{
    return hs_2d_hist_labels(id, xlabel, ylabel, zlabel);
}

int hs_stub_3d_hist_labels(int id, char *xlabel, char *ylabel,
			   char *zlabel, char *vlabel)
{
    return hs_3d_hist_labels(id, xlabel, ylabel, zlabel, vlabel);
}

int hs_stub_copy_hist(int old_id, int new_uid, const char *newtitle, const char *newcategory)
{
    return hs_copy_hist(old_id, new_uid, newtitle, newcategory);
}

int hs_stub_save_file_byids(const char *name, int *ids, int n_ids)
{
    return hs_save_file_byids(name, ids, n_ids);
}

void hs_stub_reset_const(int id, float c)
{
    hs_reset_const(id, c);
}

int hs_stub_unary_op(int uid, const char *title, const char *category, int id,
		float (*f_user_data)(float, float),
		float (*f_user_errors)(float, float))
{
    return hs_unary_op(uid, title, category, id,
		       f_user_data, f_user_errors);
}

int hs_stub_binary_op(int uid, const char *title, const char *category, int id1, int id2, 
		 float (*f_user_data)(float, float, float, float),
		 float (*f_user_errors)(float, float, float, float))
{
    return hs_binary_op(uid, title, category, id1, id2, 
			f_user_data, f_user_errors);
}

void hs_stub_change_uid_and_category(int id, int newuid, const char *newcategory)
{
    hs_change_uid_and_category(id, newuid, newcategory);
}

void hs_stub_allow_item_send(int flag)
{
    hs_allow_item_send(flag);
}

int hs_stub_socket_status(void)
{
    return hs_socket_status();
}

int hs_stub_hist_error_status(int id)
{
    return hs_hist_error_status(id);
}

void hs_stub_allow_reset_refresh(int id, int flag)
{
    hs_allow_reset_refresh(id, flag);
}

void hs_stub_task_completion_callback(void (*f)(int, int, int, char *, void *), void *d)
{
    hs_task_completion_callback(f, d);
}

int hs_stub_fill_hist_slice(int parent_id, int axis1, int bin1,
			    int axis2, int bin2, int slice_id)
{
    return hs_fill_hist_slice(parent_id, axis1, bin1, axis2, bin2, slice_id);
}

int hs_stub_slice_contents(int parent_id, int axis1, int bin1, int axis2,
			   int bin2, int arrsize, float *data, float *poserr,
			   float *negerr, int *ncopied)
{
    return hs_slice_contents(parent_id, axis1, bin1, axis2, bin2,
			     arrsize, data, poserr, negerr, ncopied);
}

int hs_stub_pack_item(int id, void **mem)
{
    return hs_pack_item(id, mem);
}

int hs_stub_unpack_item(void *mem, int len, const char *prefix)
{
    return hs_unpack_item(mem, len, prefix);
}
