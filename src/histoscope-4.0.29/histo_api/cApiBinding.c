/*******************************************************************************
*									       *
*    cAPIBinding.c -      C  interface for Histo-Scope client routines, API    *
*									       *
*									       *
*		     This file contains the following functions:	       *
*									       *
*	 hs_id               - Return the hs id from uid and Category	       *
*	 hs_id_from_title    - Return the hs id from title & category.         *
*	 hs_list_items       - Return the list of items from title & Category  *
*			       number of variables and automatic storage       *
* Copyright (c) 1993, 1994 Universities Research Association, Inc.	       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
*	P. Lebrun, December 13 1993 					       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef VMS
#include <sys/descrip.h>
#endif /*VMS*/
#include "histoscope.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiArytH.h"
#include "histoApiFiles.h"
#include "histoApiHists.h"
#include "histoApiItems.h"
#include "histoApiNTs.h"
#include "histoApiStatH.h"

int hs_server_port(void)
{
    return histo_server_port();
}

int hs_id(int uid, const char *category)
{
    /* this routine checks for item not found error and prints a message, so
    ** that histo_id can be used internally without printing an error if the
    ** id was not found.
    *
    *  igv: forget the message, the user of this function 
    *       should check the return value
    */
    
    int retVal;

    retVal = histo_id(uid, category);
/*      if (retVal == -1) */
/*         fprintf(stderr, "hs_id:  Uid %d not found within category \"%s\"\n", */
/*      		uid, category != NULL ? category : " ");   */
    return retVal;
}

int hs_id_from_title(const char *title, const char *category)
{
    return histo_id_from_title(title, category);
}

int hs_list_items(const char *title, const char *category, int *list, int num, 
		  int matchFlg)
{
    return histo_list_items(title, category, list, num, matchFlg);
}

int hs_uid(int id)
{
    return histo_uid(id);
}

int hs_category(int id, char * category)
{
     return histo_category(id, category);
}

int hs_title(int id, char * title)
{
     return histo_title(id, title);
}

int hs_type(int id)
{
    return histo_type(id);
}

int hs_read_file(const char *filename, const char *prefix)
{
    return histo_read_file(filename, prefix);
}
 
int hs_read_file_items(const char *filename, const char *prefix,
                       const char *category, 
                       int *uids, int n_uids)
{
    return histo_read_file_items(filename, prefix, category, uids, n_uids);
}

int hs_save_file_items(const char *filename, const char *category, 
                       int *uids, int n_uids)
{
    return histo_save_file_items(filename, category, uids, n_uids);
}

void hs_delete_category(const char *category)
{
    histo_delete_category(category);
    return;
}

void hs_change_uid(int id, int newuid)
{
    histo_change_uid(id, newuid);
    return;
}

void hs_change_category(int id, const char *newcategory)
{
    histo_change_category(id, newcategory);
    return;
}

void hs_change_title(int id, const char *newtitle)
{
    histo_change_title(id, newtitle);
    return;
}

int hs_num_items(void)
{
    return histo_num_items();
}

void hs_1d_hist_block_fill(int id, float *data, float *err, float *err_m)
{
   histo_1d_hist_block_fill(id, data, err, err_m);
   return;
}

void hs_2d_hist_block_fill(int id, float *data, float *err, float *err_m)
{
   histo_2d_hist_block_fill(id, data, err, err_m, 0);
   return;
}

void hs_3d_hist_block_fill(int id, float *data, float *err, float *err_m)
{
   histo_3d_hist_block_fill(id, data, err, err_m, 0);
   return;
}

int hs_1d_hist_num_bins(int id)
{
   return histo_1d_hist_num_bins(id);
}

void hs_2d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins)
{
   histo_2d_hist_num_bins(id, num_x_bins, num_y_bins);
   return;
}

void hs_3d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins,
			 int *num_z_bins)
{
   histo_3d_hist_num_bins(id, num_x_bins, num_y_bins, num_z_bins);
   return;
}

void hs_1d_hist_range(int id, float *min, float *max)
{  
   histo_1d_hist_range(id, min, max);
   return;
}

void hs_2d_hist_range(int id, float *x_min, float *x_max, 
		      float *y_min, float *y_max)
{
    histo_2d_hist_range(id, x_min, x_max, y_min, y_max);
}

void hs_3d_hist_range(int id, float *x_min, float *x_max, float *y_min,
		      float *y_max, float *z_min, float *z_max)
{
    histo_3d_hist_range(id, x_min, x_max, y_min, y_max, z_min, z_max);
}

void hs_1d_hist_bin_contents(int id, float *data)
{
   histo_1d_hist_bin_contents(id, data);
   return;
}

void hs_2d_hist_bin_contents(int id, float *data)
{
   histo_2d_hist_bin_contents(id, data, 0);
   return; 
}

void hs_3d_hist_bin_contents(int id, float *data)
{
   histo_3d_hist_bin_contents(id, data, 0);
   return; 
}

int hs_1d_hist_errors(int id, float *err, float *err_m)
{
     return histo_1d_hist_errors(id, err, err_m);
}

int hs_2d_hist_errors(int id, float *err, float *err_m)
{
    return histo_2d_hist_errors(id, err, err_m, 0);
}

int hs_3d_hist_errors(int id, float *err, float *err_m)
{
    return histo_3d_hist_errors(id, err, err_m, 0);
}

void hs_1d_hist_overflows(int id, float *underflow, float *overflow)
{
     histo_1d_hist_overflows(id, underflow, overflow);
}

void hs_2d_hist_overflows(int id, float overflows[3][3])
{
     histo_2d_hist_overflows(id, overflows, 0);
}

void hs_3d_hist_overflows(int id, float overflows[3][3][3])
{
     histo_3d_hist_overflows(id, overflows, 0);
}

void hs_1d_hist_set_overflows(int id, float underflow, float overflow)
{
     histo_1d_hist_set_overflows(id, underflow, overflow);
}

void hs_2d_hist_set_overflows(int id, float overflows[3][3])
{
     histo_2d_hist_set_overflows(id, overflows, 0);
}

void hs_3d_hist_set_overflows(int id, float overflows[3][3][3])
{
     histo_3d_hist_set_overflows(id, overflows, 0);
}

int hs_num_entries(int id)
{
    return histo_num_entries(id);
}

float hs_1d_hist_x_value(int id, float x)
{
    return histo_1d_hist_x_value(id, x);
}

float hs_2d_hist_xy_value(int id, float x, float y)
{
    return histo_2d_hist_xy_value(id, x, y);
}

float hs_3d_hist_xyz_value(int id, float x, float y, float z)
{
    return histo_3d_hist_xyz_value(id, x, y, z);
}

float hs_1d_hist_bin_value(int id, int bin_num)
{
    return histo_1d_hist_bin_value(id, bin_num, 0);
}

float hs_2d_hist_bin_value(int id, int x_bin_num, int y_bin_num)
{
    return histo_2d_hist_bin_value(id, x_bin_num, y_bin_num, 0);
}   

float hs_3d_hist_bin_value(int id, int x_bin_num, int y_bin_num, int z_bin_num)
{
    return histo_3d_hist_bin_value(id, x_bin_num, y_bin_num, z_bin_num, 0);
}

void hs_1d_hist_minimum(int id, float *x, int *bin_num, float *value)
{
    histo_1d_hist_minmax(id, 0, x, bin_num, value, 0);
    return;
}

void hs_2d_hist_minimum(int id, float *x, float *y, int *x_bin_num,
			int *y_bin_num, float *value)
{
    histo_2d_hist_minmax(id, 0, x, y, x_bin_num, y_bin_num, value, 0);
    return;
}

void hs_3d_hist_minimum(int id, float *x, float *y, float *z, int *x_bin_num,
			int *y_bin_num, int *z_bin_num, float *value)
{
    histo_3d_hist_minmax(id, 0, x, y, z, x_bin_num,
			 y_bin_num, z_bin_num, value, 0);
    return;
}

void hs_1d_hist_maximum(int id, float *x, int *bin_num, float *value)
{
    histo_1d_hist_minmax(id, 1, x, bin_num, value, 0);
    return;
}

void hs_2d_hist_maximum(int id, float *x, float *y, int *x_bin_num,
			int *y_bin_num, float *value)
{
    histo_2d_hist_minmax(id, 1, x, y, x_bin_num, y_bin_num, value, 0);
    return;
}

void hs_3d_hist_maximum(int id, float *x, float *y, float *z, int *x_bin_num,
			int *y_bin_num, int *z_bin_num, float *value)
{
    histo_3d_hist_minmax(id, 1, x, y, z, x_bin_num,
			 y_bin_num, z_bin_num, value, 0);
    return;
}

void hs_1d_hist_stats(int id, float *mean, float *std_dev)
{
    histo_1d_hist_stats(id, mean, std_dev);
    return;
}

void hs_2d_hist_stats(int id, float *x_mean, float *y_mean, 
		      float *x_std_dev, float *y_std_dev)
{
     histo_2d_hist_stats(id, x_mean, y_mean, x_std_dev, y_std_dev);
     return;
}
    
void hs_3d_hist_stats(int id, float *x_mean, float *y_mean, float *z_mean,
		      float *x_std_dev, float *y_std_dev, float *z_std_dev)
{
     histo_3d_hist_stats(id, x_mean, y_mean, z_mean, x_std_dev,
			 y_std_dev, z_std_dev);
     return;
}
    
float hs_hist_integral(int id)
{
     return histo_hist_integral(id);
}
    
void hs_hist_set_gauss_errors(int id)
{
     histo_hist_set_gauss_errors(id);
}

int hs_sum_histograms(int uid, const char *title, const char *category,
                         int id1, int id2, float const1, float const2)
{
    return histo_sum_histograms(uid, title, category,
                         id1, id2, const1, const2);
}

int hs_multiply_histograms(int uid, const char *title, const char *category,
                         int id1, int id2, float consta)
{
    return histo_multdiv_histograms(uid, title, category,
                        0, id1, id2, consta);
}
int hs_divide_histograms(int uid, const char *title, const char *category,
                         int id1, int id2, float consta)
{
    return histo_multdiv_histograms(uid, title, category,
                        1, id1, id2, consta);
}

int hs_num_variables(int id)
{
    return histo_num_variables(id);
}

int hs_variable_name(int id, int column, char *name)
{
     return histo_variable_name(id, column, name, 0);
}

int hs_variable_index(int id, const char *name)
{
     return histo_variable_index(id, name, 0);
}

float hs_ntuple_value(int id, int row, int column)
{
     return histo_ntuple_value(id, row, column, 0);
}

void hs_ntuple_contents(int id, float *data)
{
     histo_ntuple_contents(id, data, 0);
     return;
}

void hs_row_contents(int id, int row, float *data)
{
     histo_row_contents(id, row, data, 0);
     return;
}

void hs_column_contents(int id, int column, float *data)
{
     histo_column_contents(id, column, data, 0);
     return;
}

int hs_merge_entries(int uid, const char *title, const char *category, int id1, int id2)
{
    return histo_merge_entries(uid, title, category, id1, id2);
}

void hs_sum_category(const char *cat_top1, const char *cat_top2, const char *prefixsum)
{
     histo_sum_category(cat_top1, cat_top2, prefixsum);
     return;
}

void hs_sum_file(const char *file, const char *cat_top, const char *prefixsum)
{
     histo_sum_file(file, cat_top, prefixsum);
     return;
} 

int  hs_1d_hist_derivative(int uid, const char *title, const char *category, int id)
{
     return histo_1d_hist_derivative(uid, title, category, id);
}  





/* This code should become a part of cApiBinding.c */

int hs_1d_hist_labels(int id, char *xlabel, char *ylabel)
{
  return histo_1d_hist_labels(id, xlabel, ylabel);
}

int hs_2d_hist_labels(int id, char *xlabel, char *ylabel, char *zlabel)
{
  return histo_2d_hist_labels(id, xlabel, ylabel, zlabel);
}

int hs_3d_hist_labels(int id, char *xlabel, char *ylabel,
		      char *zlabel, char *vlabel)
{
  return histo_3d_hist_labels(id, xlabel, ylabel, zlabel, vlabel);
}

int hs_copy_hist(int old_id, int new_uid, const char *newtitle, const char *newcategory)
{
  return histo_copy_hist(old_id, new_uid, newtitle, newcategory);
}

int hs_save_file_byids(const char *name, int *ids, int n_ids)
{
  return histo_save_file_byids(name, ids, n_ids);
}

void hs_reset_const(int id, float c)
{
  histo_reset_const(id, c);
}

int hs_binary_op(int uid, const char *title, const char *category, int id1, int id2, 
		 float (*f_user_data)(float, float, float, float),
		 float (*f_user_errors)(float, float, float, float))
{
  return histo_binary_op(uid, title, category, id1, id2, 
			 f_user_data, f_user_errors);
}

int hs_unary_op(int uid, const char *title, const char *category, int id,
		float (*f_user_data)(float, float),
		float (*f_user_errors)(float, float))
{
  return histo_unary_op(uid, title, category, id,
			f_user_data, f_user_errors);  
}

void hs_change_uid_and_category(int id, int newuid, const char *newcategory)
{
  histo_change_uid_and_category(id, newuid, newcategory);
}

void hs_allow_item_send(int flag)
{
  histo_allow_item_send(flag);
}

int hs_socket_status(void)
{
  return histo_socket_status();
}

int hs_hist_error_status(int id)
{
  return histo_hist_error_status(id);
}

void hs_allow_reset_refresh(int id, int flag)
{
  histo_allow_reset_refresh(id, flag);
}

void hs_task_completion_callback(void (*f)(int, int, int, char *, void *), void *d)
{
  histo_task_completion_callback(f, d);
}

int hs_fill_hist_slice(int parent_id, int axis1, int bin1,
		       int axis2, int bin2, int slice_id)
{
    return histo_fill_hist_slice(parent_id, axis1, bin1, axis2, bin2, slice_id);
}

int hs_slice_contents(int parent_id, int axis1, int bin1, int axis2, 
		      int bin2, int arrsize, float *data, float *poserr,
		      float *negerr, int *ncopied)
{
    return histo_slice_contents(parent_id, axis1, bin1, axis2, bin2,
				arrsize, data, poserr, negerr, ncopied);
}

int hs_pack_item(int id, void **mem)
{
    return histo_pack_item(id, mem);
}

int hs_unpack_item(void *mem, int len, const char *prefix)
{
    return histo_unpack_item(mem, len, prefix);
}
