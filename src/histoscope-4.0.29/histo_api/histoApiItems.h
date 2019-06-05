/*******************************************************************************
*									       *
* histoApiItems.h -- Utility routines include file for the Nirvana Histoscope tool  *
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
* Written by Mark Edel	and P. Lebrun			       		       *
*									       *
*******************************************************************************/

/*
** Functions Prototypes 
**/

/*****  Functions for looking up items:  ****/
int  histo_id(int uid, const char *category);
int  histo_id_from_title(const char *title, const char *category);
int  histo_list_items(const char* title, const char *category,
                      int *ids, int num, int matchFlg);

/***** Functions for getting properties of all items ****/
int  histo_uid(int id);
int  histo_category(int id, char *category_string);
int  histo_title(int id, char *title_string);
int  histo_type(int id);

/*** Functions for managing data items ****/
void histo_delete_category(const char *category);
int  histo_num_items(void);
void histo_change_uid(int id, int newuid);
void histo_change_category(int id, const char *newcategory);
void histo_change_uid_and_category(int id, int newuid, const char *newcategory);
void histo_change_title(int id, const char *newtitle);
