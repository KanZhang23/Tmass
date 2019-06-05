/*******************************************************************************
*									       *
* histoApiFiles.h -- Include file for the Nirvana Histoscope tool, Api Files   *
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

int histo_read_file(const char *filename, const char *prefix);
int histo_read_file_items(const char *filename, const char *prefix,
                          const char *category, int *uids, int n_uids);
int histo_save_file(const char *filename);
int histo_save_file_items(const char *filename, const char *category, 
                          int *uids, int n_uids);
int histo_save_file_byids(const char *name, int *ids, int n_ids);
int histo_pack_item(int id, void **mem);
int histo_unpack_item(void *mem, int len, const char *prefix);
