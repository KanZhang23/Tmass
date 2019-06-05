/*******************************************************************************
*									       *
* histoApiStatH.h -- Include file for the Nirvana Histoscope tool 	       *
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
* Written by Mark Edel and P. Lebrun			       		       *
*									       *
*******************************************************************************/

/*
** Function Prototypes 
**/

/***** Calculations on histogram data ****/

void histo_1d_hist_minmax(int id, int mode, float *x, int *bin_num,
                                float *value, int bindingflag);
void histo_2d_hist_minmax(int id, int mode, float *x, float *y, int *x_bin_num,
                        int *y_bin_num, float *value, int bindingflag);
void histo_3d_hist_minmax(int id, int mode, float *x, float *y, float *z,
			  int *x_bin_num, int *y_bin_num, int *z_bin_num,
			  float *value, int bindingflag);
void histo_1d_hist_stats(int id, float *mean, float *std_dev);
void histo_2d_hist_stats(int id, float *x_mean, float *y_mean, float *x_std_dev,
			 float *y_std_dev);
void histo_3d_hist_stats(int id, float *x_mean, float *y_mean, float *z_mean,
			 float *x_std_dev, float *y_std_dev, float *z_std_dev);
float histo_hist_integral(int id);
void histo_hist_set_gauss_errors(int id);
