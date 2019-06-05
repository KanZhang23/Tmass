/*******************************************************************************
*									       *
* histoApiNTs.h -- Include file for the Nirvana Histoscope tool		       *
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

/*** Functions for accessing ntuple data ****/

int  histo_num_variables(int id);
int histo_variable_name(int id, int column, char *name, int bindingflag);
int  histo_variable_index(int id, const char *name, int bindingflag);
float histo_ntuple_value(int id, int row, int column, int bindingflag);
void histo_ntuple_contents(int id, float *data, int bindingflag);
void histo_row_contents(int id, int row, float *data, int bindingflag);
void histo_column_contents(int id, int column, float *data, int bindingflag);
int  histo_merge_entries(int uid, const char *title, const char *category,
                         int id1, int id2);
