#ifndef HISTO_TCL_API_H
#define HISTO_TCL_API_H

#include "simple_tcl_api.h"

/* Functions from the Histo-Scope C API v4.0 */
tcl_routine(initialize);
tcl_routine(server_port);
tcl_routine(hs_update);
tcl_routine(complete);
tcl_routine(complete_and_wait);
tcl_routine(histoscope);
tcl_routine(histo_with_config);
tcl_routine(num_connected_scopes);
tcl_routine(load_config_string);
tcl_routine(load_config_file);
tcl_routine(kill_histoscope);
tcl_routine(create_1d_hist);
tcl_routine(create_2d_hist);
tcl_routine(create_3d_hist);
tcl_routine(create_ntuple);
tcl_routine(create_indicator);
tcl_routine(create_control);
tcl_routine(create_trigger);
tcl_routine(create_group);
tcl_routine(fill_1d_hist);
tcl_routine(fill_2d_hist);
tcl_routine(fill_3d_hist);
tcl_routine(fill_ntuple);
tcl_routine(fill_histogram);
tcl_routine(set_indicator);
tcl_routine(read_control);
tcl_routine(check_trigger);
tcl_routine(set_1d_errors);
tcl_routine(set_2d_errors);
tcl_routine(set_3d_errors);
tcl_routine(reset);
tcl_routine(save_file);
tcl_routine(delete);
tcl_routine(delete_items);
tcl_routine(hbook_setup);
tcl_routine(reset_hbook_setup);
tcl_routine(change_uid);
tcl_routine(change_category);
tcl_routine(change_title);
tcl_routine(id);
tcl_routine(id_from_title);
tcl_routine(list_items);
tcl_routine(uid);
tcl_routine(category);
tcl_routine(title);
tcl_routine(type);
tcl_routine(read_file);
tcl_routine(read_file_items);
tcl_routine(save_file_items);
tcl_routine(delete_category);
tcl_routine(num_items);
tcl_routine(1d_hist_block_fill);
tcl_routine(2d_hist_block_fill);
tcl_routine(3d_hist_block_fill);
tcl_routine(1d_hist_num_bins);
tcl_routine(2d_hist_num_bins);
tcl_routine(3d_hist_num_bins);
tcl_routine(1d_hist_range);
tcl_routine(2d_hist_range);
tcl_routine(3d_hist_range);
tcl_routine(num_entries);
tcl_routine(1d_hist_bin_contents);
tcl_routine(1d_hist_errors);
tcl_routine(2d_hist_bin_contents);
tcl_routine(2d_hist_errors);
tcl_routine(3d_hist_bin_contents);
tcl_routine(3d_hist_errors);
tcl_routine(1d_hist_overflows);
tcl_routine(2d_hist_overflows);
tcl_routine(3d_hist_overflows);
tcl_routine(1d_hist_x_value);
tcl_routine(2d_hist_xy_value);
tcl_routine(3d_hist_xyz_value);
tcl_routine(1d_hist_bin_value);
tcl_routine(2d_hist_bin_value);
tcl_routine(3d_hist_bin_value);
tcl_routine(1d_hist_minimum);
tcl_routine(2d_hist_minimum);
tcl_routine(3d_hist_minimum);
tcl_routine(1d_hist_maximum);
tcl_routine(2d_hist_maximum);
tcl_routine(3d_hist_maximum);
tcl_routine(1d_hist_stats);
tcl_routine(2d_hist_stats);
tcl_routine(3d_hist_stats);
tcl_routine(hist_integral);
tcl_routine(hist_set_gauss_errors);
tcl_routine(sum_histograms);
tcl_routine(multiply_histograms);
tcl_routine(divide_histograms);
tcl_routine(sum_ntuple_columns);
tcl_routine(1d_hist_derivative);
tcl_routine(1d_hist_set_overflows);
tcl_routine(2d_hist_set_overflows);
tcl_routine(3d_hist_set_overflows);
tcl_routine(num_variables);
tcl_routine(variable_name);
tcl_routine(variable_index);
tcl_routine(ntuple_value);
tcl_routine(replace_column_contents);
tcl_routine(ntuple_contents);
tcl_routine(row_contents);
tcl_routine(column_contents);
tcl_routine(merge_entries);
tcl_routine(sum_category);
tcl_routine(sum_file);

/* Additional functions from the Histo-Scope patch */
tcl_routine(histoscope_hidden);
tcl_routine(change_uid_and_category);
tcl_routine(save_file_byids);
tcl_routine(reset_const);
tcl_routine(copy_hist);
tcl_routine(1d_hist_labels);
tcl_routine(2d_hist_labels);
tcl_routine(3d_hist_labels);
tcl_routine(allow_item_send);
tcl_routine(socket_status);
tcl_routine(allow_reset_refresh);
tcl_routine(fill_hist_slice);
tcl_routine(slice_contents);
tcl_routine(1d_hist_set_bin);
tcl_routine(2d_hist_set_bin);
tcl_routine(3d_hist_set_bin);
tcl_routine(1d_hist_set_bin_errors);
tcl_routine(2d_hist_set_bin_errors);
tcl_routine(3d_hist_set_bin_errors);
tcl_routine(hist_set_slice);
tcl_routine(hist_set_slice_errors);
tcl_routine(pack_item);
tcl_routine(unpack_item);

/* Functions from the local code */
tcl_routine(ascii_dump);
tcl_routine(stats_histogram);
tcl_routine(adaptive_stats_histogram);
tcl_routine(special_stats);
tcl_routine(special_percentiles);

tcl_routine(slice_2d_histogram);
tcl_routine(project_2d_histogram);
tcl_routine(project_3d_histogram);
tcl_routine(slice_histogram);
tcl_routine(fill_slice);
tcl_routine(fill_coord_slice);
tcl_routine(transpose_histogram);
tcl_routine(transpose_ntuple);
tcl_routine(1d_hist_subrange);
tcl_routine(2d_hist_subrange);
tcl_routine(3d_hist_subrange);
tcl_routine(1d_hist_bin_coords);
tcl_routine(2d_hist_bin_coords);
tcl_routine(3d_hist_bin_coords);
tcl_routine(1d_select_bins);
tcl_routine(2d_select_bins);
tcl_routine(1d_rotate_bins);
tcl_routine(2d_rotate_bins);
tcl_routine(2d_hist_apply_weights);
tcl_routine(1d_hist_shape);
tcl_routine(column_shape);
tcl_routine(1d_hist_percentiles);
tcl_routine(1d_hist_cdfvalues);
tcl_routine(2d_hist_percentiles);
tcl_routine(2d_hist_cdfvalues);
tcl_routine(column_percentiles);
tcl_routine(column_cdfvalues);
tcl_routine(hist_l1_norm);
tcl_routine(hist_l2_norm);
tcl_routine(hist_bin_width);
tcl_routine(hist_num_bins);
tcl_routine(concat_histograms);
tcl_routine(hist_set_error_range);
tcl_routine(hist_error_status);
tcl_routine(hist_scale_data);
tcl_routine(hist_scale_errors);
tcl_routine(next_category_uid);
tcl_routine(item_properties);
tcl_routine(find_data_mismatch);
tcl_routine(2d_fill_from_matrix);

tcl_routine(duplicate_ntuple_header);
tcl_routine(duplicate_axes);
tcl_routine(copy_data);
tcl_routine(swap_data_errors);

tcl_routine(ntuple_histo_fill);
tcl_routine(add_filled_columns);
tcl_routine(ntuple_bit_histo);
tcl_routine(ntuple_so_filter);
tcl_routine(ntuple_so_scan);
tcl_routine(column_stats);
tcl_routine(column_edf);
tcl_routine(sort_ntuple);
tcl_routine(weighted_column_stats);
tcl_routine(column_range);
tcl_routine(ntuple_subset);
tcl_routine(ntuple_subrange);
tcl_routine(unique_rows);
tcl_routine(join_entries);
tcl_routine(pack_ntuple_row);
tcl_routine(unpack_ntuple_row);

tcl_routine(1d_linear_fit);
tcl_routine(2d_linear_fit);

tcl_routine(have_cernlib);

tcl_routine(pick_random_rows);
tcl_routine(unbinned_ks_test);
tcl_routine(weighted_unbinned_ks_test);
tcl_routine(ntuple_poly_fit);
tcl_routine(eigen_sym);
tcl_routine(uniform_random_fill);
tcl_routine(hist_random);
tcl_routine(poisson_random);
tcl_routine(multinomial_random);

tcl_routine(1d_fft);
tcl_routine(1d_fourier_power);
tcl_routine(1d_fourier_phase);
tcl_routine(1d_fourier_synthesize);
tcl_routine(1d_fourier_multiply);
tcl_routine(1d_fourier_divide);
tcl_routine(1d_fourier_conjugate);

/* Interface to the RANLUX random number generator, CERNLIB entry V115 */
tcl_routine(Ranlux);
tcl_routine(Rluxgo);
tcl_routine(Rluxat);
tcl_routine(Rluxin);
tcl_routine(Rluxut);

/* Interface to Gaussian random number generator RNORMX,
   CERNLIB entry V120. This function uses RANLUX internally. */
tcl_routine(Rnormx);

/* Routines to invert a symmetric positive definite matrix and 
   to solve linear systems with such a matrix. Use CERNLIB entry F012. */
tcl_routine(Invert_sym_pos_matrix);
tcl_routine(Sym_pos_linsys);

tcl_routine(tcl_api_version);
tcl_routine(lookup_command_info);
tcl_routine(calc);
EXTERN int tcl_c_name(calc_proc) (ClientData clientData,
				  Tcl_Interp *interp,
				  int argc, char *argv[]);
tcl_routine(dir);

/* Internal utility functions */
tcl_routine(Fortran_executable);
tcl_routine(Tcl_config_file);
tcl_routine(Histosope_lib_dir);
tcl_routine(Basic_ntuple_count);
tcl_routine(Basic_ntuple_project_onhisto);
tcl_routine(Prepare_1d_scatter_plot);
tcl_routine(Is_valid_c_identifier);
tcl_routine(C_keyword_list);
tcl_routine(Ntuple_covar);
tcl_routine(List_stats);
tcl_routine(Kernel_density_1d);
tcl_routine(Kernel_density_2d);
tcl_routine(Inverse_symmetric_sqrt_2d);
tcl_routine(Matrix);
tcl_routine(Unit_matrix);
tcl_routine(Const_matrix);
tcl_routine(Histo_matrix);
tcl_routine(Periodic_gauss);
tcl_routine(Periodic_uniform_2d);
tcl_routine(Copy_1d_data_with_offset);
tcl_routine(Project_ntuple_onto_1d_hist);
tcl_routine(Apply_1d_range_masks);
tcl_routine(Ntuple_paste);
tcl_routine(Column_minmax);
tcl_routine(Precise_reference_chisq);

#endif /* not HISTO_TCL_API_H */
