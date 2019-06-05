#include "histoscope.h"
/* #include "hs.h" */
#include "histo_tcl_api.h"
#include "minuit_api.h"
#include "fit_function.h"
#include "fit_api.h"
#include "histo_utils.h"

/* int Hs_Init(Tcl_Interp *interp) */

int _hs_init(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#endif

/*  
 *  Don't call Tcl_PkgProvide here. It will be done by 
 *  "package provide" in the hs_utils.tcl file instead
 *
 *      if (Tcl_PkgProvide(interp, "hs", HS_VERSION) != TCL_OK) {
 *  	    return TCL_ERROR;
 *      }
 */

    /* Commands from the Histo-Scope C API v4.0 */
    tcl_new_command(hs, initialize);
    tcl_new_command(hs, server_port);
    tcl_new_command(hs, hs_update);
    tcl_new_command(hs, complete);
    tcl_new_command(hs, complete_and_wait);
    tcl_new_command(hs, histoscope);
    tcl_new_command(hs, histo_with_config);
    tcl_new_command(hs, num_connected_scopes);
    tcl_new_command(hs, load_config_string);
    tcl_new_command(hs, load_config_file);
    tcl_new_command(hs, kill_histoscope);
    tcl_new_command(hs, create_1d_hist);
    tcl_new_command(hs, create_2d_hist);
    tcl_new_command(hs, create_3d_hist);
    tcl_new_command(hs, create_ntuple);
    tcl_new_command(hs, create_indicator);
    tcl_new_command(hs, create_control);
    tcl_new_command(hs, create_trigger);
    tcl_new_command(hs, create_group);
    tcl_new_command(hs, fill_1d_hist);
    tcl_new_command(hs, fill_2d_hist);
    tcl_new_command(hs, fill_3d_hist);
    tcl_new_command(hs, fill_ntuple);
    tcl_new_command(hs, fill_histogram);
    tcl_new_command(hs, set_indicator);
    tcl_new_command(hs, read_control);
    tcl_new_command(hs, check_trigger);
    tcl_new_command(hs, set_1d_errors);
    tcl_new_command(hs, set_2d_errors);
    tcl_new_command(hs, set_3d_errors);
    tcl_new_command(hs, reset);
    tcl_new_command(hs, save_file);
    tcl_new_command(hs, delete);
    tcl_new_command(hs, delete_items);
    tcl_new_command(hs, change_uid);
    tcl_new_command(hs, change_category);
    tcl_new_command(hs, change_title);
    tcl_new_command(hs, id);
    tcl_new_command(hs, id_from_title);
    tcl_new_command(hs, list_items);
    tcl_new_command(hs, uid);
    tcl_new_command(hs, category);
    tcl_new_command(hs, title);
    tcl_new_command(hs, type);
    tcl_new_command(hs, read_file);
    tcl_new_command(hs, read_file_items);
    tcl_new_command(hs, save_file_items);
    tcl_new_command(hs, delete_category);
    tcl_new_command(hs, num_items);
    tcl_new_command(hs, 1d_hist_block_fill);
    tcl_new_command(hs, 2d_hist_block_fill);
    tcl_new_command(hs, 3d_hist_block_fill);
    tcl_new_command(hs, 1d_hist_num_bins);
    tcl_new_command(hs, 2d_hist_num_bins);
    tcl_new_command(hs, 3d_hist_num_bins);
    tcl_new_command(hs, 1d_hist_range);
    tcl_new_command(hs, 2d_hist_range);
    tcl_new_command(hs, 3d_hist_range);
    tcl_new_command(hs, num_entries);
    tcl_new_command(hs, 1d_hist_bin_contents);
    tcl_new_command(hs, 1d_hist_errors);
    tcl_new_command(hs, 2d_hist_bin_contents);
    tcl_new_command(hs, 2d_hist_errors);
    tcl_new_command(hs, 3d_hist_bin_contents);
    tcl_new_command(hs, 3d_hist_errors);
    tcl_new_command(hs, 1d_hist_overflows);
    tcl_new_command(hs, 2d_hist_overflows);
    tcl_new_command(hs, 3d_hist_overflows);
    tcl_new_command(hs, 1d_hist_x_value);
    tcl_new_command(hs, 2d_hist_xy_value);
    tcl_new_command(hs, 3d_hist_xyz_value);
    tcl_new_command(hs, 1d_hist_bin_value);
    tcl_new_command(hs, 2d_hist_bin_value);
    tcl_new_command(hs, 3d_hist_bin_value);
    tcl_new_command(hs, 1d_hist_minimum);
    tcl_new_command(hs, 2d_hist_minimum);
    tcl_new_command(hs, 3d_hist_minimum);
    tcl_new_command(hs, 1d_hist_maximum);
    tcl_new_command(hs, 2d_hist_maximum);
    tcl_new_command(hs, 3d_hist_maximum);
    tcl_new_command(hs, 1d_hist_stats);
    tcl_new_command(hs, 2d_hist_stats);
    tcl_new_command(hs, 3d_hist_stats);
    tcl_new_command(hs, hist_integral);
    tcl_new_command(hs, hist_set_gauss_errors);
    tcl_new_command(hs, sum_histograms);
    tcl_new_command(hs, multiply_histograms);
    tcl_new_command(hs, divide_histograms);
    tcl_new_command(hs, sum_ntuple_columns);
    tcl_new_command(hs, 1d_hist_derivative);
    tcl_new_command(hs, 1d_hist_set_overflows);
    tcl_new_command(hs, 2d_hist_set_overflows);
    tcl_new_command(hs, 3d_hist_set_overflows);
    tcl_new_command(hs, num_variables);
    tcl_new_command(hs, variable_name);
    tcl_new_command(hs, variable_index);
    tcl_new_command(hs, ntuple_value);
    tcl_new_command(hs, replace_column_contents);
    tcl_new_command(hs, ntuple_contents);
    tcl_new_command(hs, row_contents);
    tcl_new_command(hs, column_contents);
    tcl_new_command(hs, merge_entries);
    tcl_new_command(hs, sum_category);
    tcl_new_command(hs, sum_file);

    /* Additional functions from the Histo-Scope patch */
    tcl_new_command(hs, histoscope_hidden);
    tcl_new_command(hs, change_uid_and_category);
    tcl_new_command(hs, save_file_byids);
    tcl_new_command(hs, reset_const);
    tcl_new_command(hs, copy_hist);
    tcl_new_command(hs, 1d_hist_labels);
    tcl_new_command(hs, 2d_hist_labels);
    tcl_new_command(hs, 3d_hist_labels);
    tcl_new_command(hs, allow_item_send);
    tcl_new_command(hs, socket_status);
    tcl_new_command(hs, allow_reset_refresh);
    tcl_new_command(hs, fill_hist_slice);
    tcl_new_command(hs, slice_contents);
    tcl_new_command(hs, 1d_hist_set_bin);
    tcl_new_command(hs, 2d_hist_set_bin);
    tcl_new_command(hs, 3d_hist_set_bin);
    tcl_new_command(hs, 1d_hist_set_bin_errors);
    tcl_new_command(hs, 2d_hist_set_bin_errors);
    tcl_new_command(hs, 3d_hist_set_bin_errors);
    tcl_new_command(hs, hist_set_slice);
    tcl_new_command(hs, hist_set_slice_errors);
    tcl_new_command(hs, pack_item);
    tcl_new_command(hs, unpack_item);

    /* Functions from the local files */
    tcl_new_command(hs, ascii_dump);
    tcl_new_command(hs, stats_histogram);
    tcl_new_command(hs, adaptive_stats_histogram);
    tcl_new_command(hs, special_stats);
    tcl_new_command(hs, special_percentiles);

    tcl_new_command(hs, slice_2d_histogram);
    tcl_new_command(hs, project_2d_histogram);
    tcl_new_command(hs, project_3d_histogram);
    tcl_new_command(hs, slice_histogram);
    tcl_new_command(hs, fill_slice);
    tcl_new_command(hs, fill_coord_slice);
    tcl_new_command(hs, transpose_histogram);
    tcl_new_command(hs, transpose_ntuple);
    tcl_new_command(hs, 1d_hist_subrange);
    tcl_new_command(hs, 2d_hist_subrange);
    tcl_new_command(hs, 3d_hist_subrange);
    tcl_new_command(hs, 1d_hist_bin_coords);
    tcl_new_command(hs, 2d_hist_bin_coords);
    tcl_new_command(hs, 3d_hist_bin_coords);
    tcl_new_command(hs, 1d_select_bins);
    tcl_new_command(hs, 2d_select_bins);
    tcl_new_command(hs, 1d_rotate_bins);
    tcl_new_command(hs, 2d_rotate_bins);
    tcl_new_command(hs, 2d_hist_apply_weights);
    tcl_new_command(hs, 1d_hist_shape);
    tcl_new_command(hs, column_shape);
    tcl_new_command(hs, 1d_hist_percentiles);
    tcl_new_command(hs, 1d_hist_cdfvalues);
    tcl_new_command(hs, 2d_hist_percentiles);
    tcl_new_command(hs, 2d_hist_cdfvalues);
    tcl_new_command(hs, column_percentiles);
    tcl_new_command(hs, column_cdfvalues);
    tcl_new_command(hs, hist_l1_norm);
    tcl_new_command(hs, hist_l2_norm);
    tcl_new_command(hs, hist_bin_width);
    tcl_new_command(hs, hist_num_bins);
    tcl_new_command(hs, concat_histograms);
    tcl_new_command(hs, hist_set_error_range);
    tcl_new_command(hs, hist_error_status);
    tcl_new_command(hs, hist_scale_data);
    tcl_new_command(hs, hist_scale_errors);
    tcl_new_command(hs, next_category_uid);
    tcl_new_command(hs, item_properties);
    tcl_new_command(hs, find_data_mismatch);
    tcl_new_command(hs, 2d_fill_from_matrix);

    tcl_new_command(hs, duplicate_ntuple_header);
    tcl_new_command(hs, duplicate_axes);
    tcl_new_command(hs, copy_data);
    tcl_new_command(hs, swap_data_errors);

    tcl_new_command(hs, ntuple_histo_fill);
    tcl_new_command(hs, add_filled_columns);
    tcl_new_command(hs, ntuple_bit_histo);
    tcl_new_command(hs, ntuple_so_filter);
    tcl_new_command(hs, ntuple_so_scan);
    tcl_new_command(hs, column_stats);
    tcl_new_command(hs, column_edf);
    tcl_new_command(hs, sort_ntuple);
    tcl_new_command(hs, weighted_column_stats);
    tcl_new_command(hs, column_range);
    tcl_new_command(hs, ntuple_subset);
    tcl_new_command(hs, ntuple_subrange);
    tcl_new_command(hs, unique_rows);
    tcl_new_command(hs, join_entries);
    tcl_new_command(hs, pack_ntuple_row);
    tcl_new_command(hs, unpack_ntuple_row);

    tcl_new_command(hs, 1d_linear_fit);
    tcl_new_command(hs, 2d_linear_fit);

    tcl_new_command(hs, have_cernlib);

    tcl_new_command(hs, pick_random_rows);
    tcl_new_command(hs, unbinned_ks_test);
    tcl_new_command(hs, weighted_unbinned_ks_test);
    tcl_new_command(hs, ntuple_poly_fit);
    tcl_new_command(hs, eigen_sym);
    tcl_new_command(hs, uniform_random_fill);
    tcl_new_command(hs, hist_random);
    tcl_new_command(hs, poisson_random);
    tcl_new_command(hs, multinomial_random);

    tcl_new_command(hs, 1d_fft);
    tcl_new_command(hs, 1d_fourier_power);
    tcl_new_command(hs, 1d_fourier_phase);
    tcl_new_command(hs, 1d_fourier_synthesize);
    tcl_new_command(hs, 1d_fourier_multiply);
    tcl_new_command(hs, 1d_fourier_divide);
    tcl_new_command(hs, 1d_fourier_conjugate);

    tcl_new_command(hs, Ranlux);
    tcl_new_command(hs, Rluxgo);
    tcl_new_command(hs, Rluxat);
    tcl_new_command(hs, Rluxin);
    tcl_new_command(hs, Rluxut);

    tcl_new_command(hs, Rnormx);

    tcl_new_command(hs, Invert_sym_pos_matrix);
    tcl_new_command(hs, Sym_pos_linsys);
    tcl_new_command(hs, Precise_reference_chisq);

    tcl_new_command(hs, tcl_api_version);
    tcl_new_command(hs, lookup_command_info);
    tcl_new_command(hs, calc);
    tcl_new_command(hs, dir);

    /* Some internal utility functions */
    tcl_new_command(hs, Fortran_executable);
    tcl_new_command(hs, Tcl_config_file);
    tcl_new_command(hs, Histosope_lib_dir);
    tcl_new_command(hs, Basic_ntuple_count);
    tcl_new_command(hs, Basic_ntuple_project_onhisto);
    tcl_new_command(hs, Prepare_1d_scatter_plot);
    tcl_new_command(hs, Is_valid_c_identifier);
    tcl_new_command(hs, C_keyword_list);
    tcl_new_command(hs, Ntuple_covar);
    tcl_new_command(hs, List_stats);
    tcl_new_command(hs, Kernel_density_1d);
    tcl_new_command(hs, Kernel_density_2d);
    tcl_new_command(hs, Inverse_symmetric_sqrt_2d);
    tcl_new_command(hs, Matrix);
    tcl_new_command(hs, Unit_matrix);
    tcl_new_command(hs, Const_matrix);
    tcl_new_command(hs, Histo_matrix);
    tcl_new_command(hs, Periodic_gauss);
    tcl_new_command(hs, Periodic_uniform_2d);
    tcl_new_command(hs, Copy_1d_data_with_offset);
    tcl_new_command(hs, Project_ntuple_onto_1d_hist);
    tcl_new_command(hs, Apply_1d_range_masks);
    tcl_new_command(hs, Ntuple_paste);
    tcl_new_command(hs, Column_minmax);

    /* C functions in Tcl (for fitting purposes) */
    tcl_new_command(hs, sharedlib);
    tcl_new_command(hs, function);
    tcl_new_command(hs, function_import);
    tcl_new_command(hs, function_list);
    tcl_new_command(hs, function_sum);
    tcl_new_command(hs, function_divide);
    tcl_new_command(hs, function_multiply);
    tcl_new_command(hs, function_compose);
    tcl_new_command(hs, Function_owns_dll);

    /* Functions related to fitting with MINUIT */
    tcl_new_command(fit, Fit_list);
    tcl_new_command(fit, Fit_get_active);
    tcl_new_command(fit, Fit_fcn_set);
    tcl_new_command(fit, Fit_fcn_get);
    tcl_new_command(fit, Fit_next_name);
    tcl_new_command(fit, Fit_method_list);
    tcl_new_command(fit, Fit_lock_minuit);

    tcl_new_command(fit, Fit_activate);
    tcl_new_command(fit, Fit_create);
    tcl_new_command(fit, Fit_copy);
    tcl_new_command(fit, Fit_copy_data);
    tcl_new_command(fit, Fit_rename);
    tcl_new_command(fit, Fit_callback);
    tcl_new_command(fit, Fit_tcl_fcn);

    tcl_new_command(fit, Fit_exists);
    tcl_new_command(fit, Fit_info);

    tcl_new_command(fit, Fit_subset);
    tcl_new_command(fit, Fit_parameter);
    tcl_new_command(fit, Fit_function);
    tcl_new_command(fit, Fit_cget);
    tcl_new_command(fit, Fit_config);
    tcl_new_command(fit, Fit_tcldata);
    tcl_new_command(fit, Fit_append_dll);
    tcl_new_command(fit, Fit_reset);
    tcl_new_command(fit, Fit_apply_mapping);
    tcl_new_command(fit, Fit_tcl_fcn_args);
    tcl_new_command(fit, Fit_has_tcl_fcn);

    tcl_new_command(fit, Parse_minuit_pars_in_a_map);
    tcl_new_command(fit, Parse_fitter_pars_in_a_map);
    tcl_new_command(fit, Analyze_multires_ntuple);
    tcl_new_command(fit, Fit_multires_fast_cycle);

    /* Low-level MINUIT commands */
    tcl_new_command(mn, fortranfile);
    tcl_new_command(mn, mintio);
    tcl_new_command(mn, minuit);
    tcl_new_command(mn, init);
    tcl_new_command(mn, seti);
    tcl_new_command(mn, prti);
    tcl_new_command(mn, pars);
    tcl_new_command(mn, parm);
    tcl_new_command(mn, comd);
    tcl_new_command(mn, excm);
    tcl_new_command(mn, pout);
    tcl_new_command(mn, stat);
    tcl_new_command(mn, errs);
    tcl_new_command(mn, intr);
    tcl_new_command(mn, inpu);
    tcl_new_command(mn, cont);
    tcl_new_command(mn, emat);
    tcl_new_command(mn, maxcalls);
    tcl_new_command(mn, nfcn);
    tcl_new_command(mn, dcovar);

    /* Install the task completion callback */
    hs_task_completion_callback(task_completion_callback, interp);

    return TCL_OK;
}
