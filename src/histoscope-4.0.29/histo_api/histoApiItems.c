/*******************************************************************************
*									       *
* histoApiItems.c -- Application Interface routines, to access items,	       *
*			item numbers, list or properties.                      *
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
* December 93    							       *
*									       *
* Written by Mark Edel and Paul Lebrun   				       *
*									       *
*									       *
*******************************************************************************/
/*
* REQUIRED INCLUDE FILES
*/
#include <stdio.h>
#include <stdlib.h>
#ifdef VMS
#include <unixio.h>
#include <processes.h>
#include <file.h>
#include <perror.h>
#include "../util/VMSparam.h"
#else /*VMS*/
#ifndef VXWORKS
#include <sys/param.h>
#endif /*VXWORKS*/
#endif /*VMS*/
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include <math.h>
#include "histoscope.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiItems.h"

#if       !defined(TRUE) || ((TRUE) != 1)
#define   TRUE    (1)
#define   FALSE   (0)
#endif

/*  
** Functions for looking up items:
** hs_id, hs_id_from_title, hs_list_items
*/

int histo_id(int uid, const char *category)
/* 
	uid		Unique User id 
	category	Category name, must be an exact match

	Returns:	Histoscope id, or -1 if no items matched, or if 
			Category is illegal..
*/
{
	int i, j;
	hsGeneral *item, **ip;
	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_id: Error, called before hs_initialize.\n");
    	return -1;			/* hs_initialize must be called first */
    }
	
     if (!CheckValidCategory(category, "hs_id", FALSE))
     	 return -1;
     ip = HistoPtrList;
     ValidStr(category, HS_MAX_CATEGORY_LENGTH, "hs_id", "category");
     for (i=0; i< NumOfItems; i++, ip++) {
	 if (*ip == NULL) 
	    continue;		/* skip deleted and HBOOK items */
	 item = (hsGeneral *) *ip;
	 if (item->uid == uid) { /* Look first at uid, if match, */
	         		    /* Confirm with Category */
	    if ((category == NULL) && (item->category == NULL)) 
	      				return (item->id);
	    if (strcmp(category, item->category) == 0)
	    				return (item->id);
            j = strspn(category, " ");	
	    if (strcmp((category+j), item->category) == 0)
	                                return (item->id); 
	 }
     }
     return -1;
}
	
int histo_id_from_title(const char *title, const char *category)
{
/*
	title		Item title, must be an exact match
	
	category	Category name, must be an exact match
	
	Returns:	Histoscope id, or -1 if no items matched
*/
    int i, j;
    hsGeneral *item, **ip;
    char cat[HS_MAX_CATEGORY_LENGTH];
    const char *tit;
	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_id_from_title: Error, called before hs_initialize.\n");
    	return -1;			/* hs_initialize must be called first */
    }
	
     if (!CheckValidCategory(category, "hs_id_from_title", FALSE)) return -1;
     ip = HistoPtrList;
     strcpy(cat, ValidStr(category, HS_MAX_CATEGORY_LENGTH, "hs_id_from_title", 
     			"category")); 
     tit = ValidStr(title, HS_MAX_TITLE_LENGTH, "hs_id_from_title", "title"); 
	for (i=0; i< NumOfItems; i++, ip++){
	    if (*ip == NULL)
	    	continue;
	    item = (hsGeneral *) *ip;
	    if (strcmp(tit, item->title) == 0 ) { /* Compare titles */
	         		                 /* Confirm with Category */
	      if ((category == NULL) && (item->category == NULL)) 
	      				return (item->id);
	      if (strcmp(cat, item->category) == 0)
	      				return (item->id);
              j = strspn(cat, " ");	
	      if (strcmp((cat+j), item->category) == 0)
	                                return (item->id); 
	   }
	}
/* igv: remove verbose message -- the user should check the return value */
/*       fprintf(stderr,  */
/*      	   "hs_id_from_title:  Title \"%s\" not found within category \"%s\"\n", */
/*      	   tit, cat); */
     return -1;
}
	       
int  histo_list_items(const char* title, const char *category, int *ids,
                      int num, int matchFlg)
/* 
   Fill a list of histoscope id numbers. The user must allocate at least num 
   integer element for array ids. These ids are returned in the order
   in which the histograms were created, or read in from file.
	
	title		Title of the histogram(s) name.  This string can be
			 	used as an exact match or as a string subset.
			 	(see matchFlg)  
	Category	Category string, must be an exact match, or, if a 
			 	trailing "..." is specified, will match
				all subcategories in category specified.  Null
				or empty or spaces matches uncategorized items.
				Example: category "HS/..." matches "HS" and
				"HS/Nts" but not "HSNTuples" nor "hs".  If just
				"..." is specified, matches all categories
				including uncategorized items.
	ids		Returns the histoscope id numbers of the matching
				items.
	num		The maximum number of items that can be returned.
	matchFlg	If == 0, specifies an exact match for title.  Otherwise
			 	will scan titles for string supplied in 'title'.
			 	Inexact matches will skip leading spaces so that
			 	supplying " " will match all titles.
	      
	Returns: 	The number of matching Title/Category strings.
		  	May be larger than num.  Returns -1 if category not 
		  	valid.
*/
{
    int i, numfound, lenCat, validCat, dotsInCat;
    hsGeneral *item;
    char *cat = NULL;
    const char* ccat = NULL, *tit;

    numfound = 0;
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_list_items: Error, called before hs_initialize.\n");
    	return numfound;		/* hs_initialize must be called first */
    }
	
    validCat = CheckValidCategory(category, "hs_list_items", TRUE);
    if (!validCat)
    	return -1;
    dotsInCat = (validCat == -1);
    
    /* accomodate NULL and "..." and skip over leading spaces */
    ccat = ValidStr(category, HS_MAX_CATEGORY_LENGTH, "hs_list_items",
    				"category");
    cat = CopyNtrim(ccat);
    lenCat = strlen(cat);
    if (dotsInCat) {
    	i = (lenCat > 4 && cat[lenCat-4] == '/') ? 4 : 3;
    	cat[lenCat-i] = '\0';
	lenCat = strlen(cat);
    }
    tit = ValidStr(title, HS_MAX_TITLE_LENGTH, "hs_list_items", "title");
    if (matchFlg != 0)
    	tit += strspn(tit, " ");	/* inexact match, skip leading spaces */

    for (i = 0; i < NumOfItems; i++) {
       if (HistoPtrList[i] == NULL)
       	   continue;			       /* skip deleted or HBOOK items */
       item = HistoPtrList[i];
       
       /* match category or, if ..., subcategories of specified category */
       if ( strcmp(cat, item->category) == 0	      /* exact category match */
            || ( dotsInCat && strncmp(cat, item->category, lenCat) == 0 && 
                          (lenCat == 0 || item->category[lenCat] == '/') ) ) {
	   /* Category matched, now confirm with title */
	   if ( (matchFlg == 0 && strcmp(title, item->title) == 0)  /* exact */
	     || (matchFlg != 0 && 				  /* inexact */
	     	      (tit[0] == '\0' || strstr(item->title, tit) != NULL)) ) {
	       if (numfound < num)
	           ids[numfound] = item->id;	 /* store id number for user */
	       numfound++;
	   }
       }
    }
    free (cat);
    return numfound;
}

int histo_uid(int idt)
/*	
	id		HistoScope id, as returned by create routines
	
	Return Value:	User id value, or 0 if id was invalid
*/
{
    hsGeneral *item;
    	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_uid: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
	
     item = (hsGeneral *) GetItemByPtrID(idt);
     if (item == NULL) return 0;
     return item->uid;
}

int histo_category(int idt, char *category_string)
/*
	id		HistoScope id, as returned by create routines
	category_string	User allocated space for returned string
			of length HS_MAX_CATEGORY_LENGTH
	
	Return Value:	length of category string or -1 if id was invalid
*/
{
    int lenc;
    hsGeneral *item;
    	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_category: Error, called before hs_initialize.\n");
    	return -1;			/* hs_initialize must be called first */
    }
    	
    item = (hsGeneral *) GetItemByPtrID(idt);
    if (item == NULL) return -1;
    category_string = strcpy(category_string, item->category);
    lenc = strlen(item->category); 
    return lenc;
}

int histo_title(int idt, char *title_string)
/*
	id		HistoScope id, as returned by create routines
	title_string 	User allocated space for returning string
			of length HS_MAX_TITLE_LENGTH
	
	Returns: 	Length of title string or -1 if id was invalid
*/

{
    int lent;
    hsGeneral *item;
    	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_title: Error, called before hs_initialize.\n");
    	return -1;			/* hs_initialize must be called first */
    }
    
    item = (hsGeneral *) GetItemByPtrID(idt);
    if (item == NULL) return -1;
    strcpy(title_string, item->title);
    lent = strlen(item->title); 
    return lent;
	
}

int histo_type(int idt)
/*
	id		HistoScope id, as returned by create routines
	
	Return Value:	Type of data referred to by id. Valid type are 
			HS_1D_HISTOGRAM, HS_2D_HISTOGRAM, HS_NTUPLE,
			HS_INDICATOR, HS_CONTROL, HS_TRIGGER, HS_NONE.
			The last type referred to a non-existant item
*/
{
    hsGeneral *item;
    	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_type: Error, called before hs_initialize.\n");
    	return HS_NONE;		/* hs_initialize must be called first */
    }
	
     item = (hsGeneral *) GetItemByPtrID(idt);
     if (item == NULL) return HS_NONE;
     return item->type;

}
void histo_delete_category(const char *category)
/* 
   Remove and deallocate all items in the named category. Data content is 
   lost.  If the path name leads to a complete tree, all branches and
   leaves will be removed.
   
	Category	Category string, must be an exact match, or, if a 
			 	trailing "..." is specified, will match
				all subcategories in category specified.  Null
				or empty or spaces matches uncategorized items.
				Example: category "HS/..." matches "HS" and
				"HS/Nts" but not "HSNTuples" nor "hs".  If just
				"..." is specified, matches all categories
				including uncategorized items.

*/
{
	int i, lenCat, validCat, dotsInCat, numToDelete = 0;
	hsGeneral *item;
	char *cat;
        const char* ccat;
	
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_delete_category: Error, called before hs_initialize.\n");
    	return;			/* hs_initialize must be called first */
    }
	
    validCat = CheckValidCategory(category, "hs_delete_category", TRUE);
    if (!validCat || NumOfItems < 1)
    	return;
    dotsInCat = (validCat == -1);
    ccat = ValidStr(category, HS_MAX_CATEGORY_LENGTH, "hs_delete_category",
     		   "category");
    cat = CopyNtrim(ccat);
    lenCat = strlen(cat);
    if (dotsInCat) {
    	i = (lenCat > 4 && cat[lenCat-4] == '/') ? 4 : 3;
    	cat[lenCat-i] = '\0';
	lenCat = strlen(cat);
    }
    for (i=0; i< NumOfItems; i++) {
	if (HistoPtrList[i] == NULL) 
	       continue;
	item = HistoPtrList[i];
	if ( strcmp(cat, item->category) == 0	      /* exact category match */
            || ( dotsInCat && strncmp(cat, item->category, lenCat) == 0 && 
                          (lenCat == 0 || item->category[lenCat] == '/') ) ) {
	    if (numToDelete++ == 0)
	    	BeginItemList();		/* Batch deletions for Histo-Scope */
	    histo_delete(item->id);
    	}
    }
    if (numToDelete > 0)
    	EndItemList();			/* Tell HS Can update Main Panel */
    free(cat);
    return;
}

int histo_num_items(void)
/* 
   Returns the number of items defined so far
*/
{
	int i, numt;
	hsGeneral **ip;

    numt = 0;
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_num_items: Error, called before hs_initialize.\n");
    	return numt;			/* hs_initialize must be called first */
    }

     ip = HistoPtrList;
     for (i=0; i< NumOfItems; i++, ip++) {
	 if (*ip != NULL) numt++;
     }
     return numt;
}

/* 
void histo_change_category(int id, char *newcategory)
   Rename the Category of particular item, refered by its HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
    ****Implemented in HistoClient, to avoid making External too many variables.

*/

/* 
void histo_change_title(int id, char *newtitle)
   Rename the Category of particular item, refered by its HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
    ****Implemented in HistoClient, to avoid making External too many variables.

*/
    
/* 
void histo_change_uid(int id, int newuid)
   Rename the Category of particular item, refered by its HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
    ****Implemented in HistoClient, to avoid making External too many variables.

*/
    
