/*******************************************************************************
*									       *
* histoApiHists.h -- Include file for the Nirvana Histoscope tool 	       *
*									       *
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
* December 9, 1993							       *
*									       *
* Written by P. Lebrun and Joy Kyriakopulos		       		       *
*									       *
*******************************************************************************/

/*
** Functions Prototypes 
**/

/*** Functions for accessing histogram data ***/

int histo_num_entries(int id);
int histo_hist_error_status(int id);

void histo_1d_hist_block_fill(int id, float *data, float *err, float *err_m);
void histo_2d_hist_block_fill(int id, float *data, float *err,
                                float *err_m, int bindingflag);
void histo_3d_hist_block_fill(int id, float *data, float *err,
                                float *err_m, int bindingflag);

int  histo_1d_hist_num_bins(int id);
void histo_2d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins);
void histo_3d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins,
			    int *num_z_bins);

void histo_1d_hist_range(int id, float *min, float *max);
void histo_2d_hist_range(int id, float *x_min, float *x_max, 
			 float *y_min, float *y_max);
void histo_3d_hist_range(int id, float *x_min, float *x_max, 
			 float *y_min, float *y_max,
			 float *z_min, float *z_max);

void histo_1d_hist_bin_contents(int id, float *data);
void histo_2d_hist_bin_contents(int id, float *data, int bindingflag);
void histo_3d_hist_bin_contents(int id, float *data, int bindingflag);

int  histo_1d_hist_errors(int id, float *err, float *err_m);
int  histo_2d_hist_errors(int id, float *err, float *err_m, 
			  int bindingflag);
int  histo_3d_hist_errors(int id, float *err, float *err_m, 
			  int bindingflag);

void histo_1d_hist_overflows(int id, float *underflow, float *overflow);
void histo_2d_hist_overflows(int id, float overflows[3][3], int bindingflag);
void histo_3d_hist_overflows(int id, float overflows[3][3][3], int bindingflag);

float histo_1d_hist_x_value(int id, float x);
float histo_2d_hist_xy_value(int id, float x, float y);
float histo_3d_hist_xyz_value(int id, float x, float y, float z);

float histo_1d_hist_bin_value(int id, int bin_num, int bindingflag);
float histo_2d_hist_bin_value(int id, int x_bin_num,
                              int y_bin_num, int bindingflag);
float histo_3d_hist_bin_value(int id, int x_bin_num,
                              int y_bin_num, int z_bin_num, int bindingflag);

int histo_1d_hist_labels(int id, char *xlabel, char *ylabel);
int histo_2d_hist_labels(int id, char *xlabel, char *ylabel, char *zlabel);
int histo_3d_hist_labels(int id, char *xlabel, char *ylabel,
			 char *zlabel, char *vlabel);

void histo_1d_hist_set_overflows(int id, float underflow, float overflow);
void histo_2d_hist_set_overflows(int id, float overflows[3][3], int bindingflag);
void histo_3d_hist_set_overflows(int id, float overflows[3][3][3], int bindingflag);
