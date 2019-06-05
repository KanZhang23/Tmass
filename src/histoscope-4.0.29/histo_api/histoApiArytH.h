/*******************************************************************************
*									       *
* histoApiArytH.h -- Include file for the Nirvana Histoscope tool	       *
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
** Function Prototypes 
**/

/***** Calculations on histogram data ****/

int  histo_sum_histograms(int uid, const char *title, const char *category,
                         int id1, int id2, float const1, float const2);
int  histo_multdiv_histograms(int uid, const char *title, const char *category,
                              int oper, int id1, int id2, float const1);
void histo_sum_category(const char *cat_top1, const char *cat_top2,
                        const char *prefixsum); 
void histo_sum_file(const char *file, const char *cat_top,
                    const char *prefixsum);
int  histo_1d_hist_derivative(int uid, const char *title, const char *category, 
                              int id);
int histo_binary_op(int uid, const char *title, const char *category,
                    int id1, int id2, 
		    float (*f_user_data)(float, float, float, float),
		    float (*f_user_errors)(float, float, float, float));
int histo_unary_op(int uid, const char *title, const char *category, int id,
		   float (*f_user_data)(float, float),
		   float (*f_user_errors)(float, float));
