/*******************************************************************************
*									       *
* histoApiFiles.c -- Application Interface routines,                           *
*                                      to access Standard Histofiles :         *
*		hs_save_file, hs_read_file and so forth..                      *
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
* December 1993 							       *
*									       *
* Written by Paul Lebrun, Joy Kyriakopulos, and Mark Edel                      *
*									       *
*									       *
*******************************************************************************/
/*
* REQUIRED INCLUDE FILES
*/
#include <stdio.h>
#include <stdlib.h>
#ifdef VMS	/* ask rpc/types to refrain from declaring malloc */
#define DONT_DECLARE_MALLOC
#endif /*VMS*/  
#include <rpc/types.h>
#include <rpc/xdr.h>
#ifdef VMS
#include <unixio.h>
#include <processes.h>
#include <file.h>
#include <timeb.h>
#include <perror.h>
#include <climsgdef.h>  /* CLI status values */
#include <mulsys/types.h>
#include <mulsys/socket.h>
#include <mul/netdb.h>
#include <mulsys/ioctl.h>
#include <mul/errno.h>
#include "../util/VMSparam.h"
#else /*VMS*/
#ifndef VXWORKS
#include <sys/param.h>
#endif /*VXWORKS*/
#endif /*VMS*/
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include <math.h>
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "histoApiFiles.h"
#include "../histo_util/hsFile.h"
#define DONT_DEFINE_MSG_STRUCT
#include "../histo_util/histprotocol.h"
#include "HistoClient.h"
#include "histoApiItems.h"
#define  XDR_FILTERS_HERE
#include "../histo_util/xdrHisto.h"
/*****    #define COM_DEBUG   *****/

static void makeCStruct(hsGeneral **item);

/** Input function : Read existing Histograms from File **/

int histo_read_file(const char *filename, const char *prefix)
/*
   Read all of the items from a file.  All Catagory strings will be prefixed
   by a new top level category specified in prefix.

	filename	Name of the file to read
	prefix		String to be added to the beginning of the categories
			of the items read from the file.  May be null or empty
			so that items read from file are not prefixed.  Note
			this can result in items not being read in because of
			duplicate uids within category.

	Return Value:	number of items read if everything was O.K., otherwise
			returns -1 and prints an error message.
			
	Note : This code has been mostly cloned from ReadHistoFile routine
			in module communication.c. These two pieces of 
			code better work in sync !
*/
{
    int i, numt, lenPrefix, leno, numItems;
    char *newcategory;
    hsGeneral *item;
    hsFile *hsfile;
    char *errString = NULL;

    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_read_file: Error, called before hs_initialize.\n");
    	return -1;
    }
    newcategory = (char *) malloc(HS_MAX_CATEGORY_LENGTH+1);
    if (prefix == NULL) {
        lenPrefix = 0;			/* allow null prefix, so what you */
        newcategory[0] = '\0';		/*    had is what you'll get now  */
    }
    else {
    	i = strspn(prefix, " ");      /* number of leading spaces to skip */
    	if (strlen(prefix+i) > (HS_MAX_CATEGORY_LENGTH-2)) {
    	    fprintf(stderr, 
    	        "hs_read_file: Category prefix is too long, no data read \n ");
    	    return -1;
    	}
    	if (strlen(prefix+i) == 0) {
    	    lenPrefix = 0;	  /* treat category of all spaces as null */
    	    newcategory[0] = '\0';
    	}
    	else {
	    strcpy(newcategory, (prefix+i));
/*  	    strcat(newcategory,"/"); */
	    lenPrefix = strlen(newcategory);
	}
    }
    
    /* Open the file to read */
    hsfile = OpenHsFile(filename, HS_READ, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_read_file: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL) {
    	free(newcategory);
    	return -1;			/* error on open */
    }
    
    /* Read file */
    xdr_DontReadNfitInfo(1);            /* Don't read all nfit info */
    numt = ReadHsFile (&hsfile, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_read_file: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (numt <= 0) {
    	free(newcategory);
    	return numt;			/* error on open */
    }
    
    /* Go through all the items read, and ... */
    numItems = numt;
    for (i = 0; i < numt; ++i) {
        item = (hsfile->locTblMem)[i];
        if (item->type == HS_NFIT) {	/* skip nfit items */
            FreeItem(item);	/* FreeHsFileStruct doesn't free items */
     	    (hsfile->locTblMem)[i] = NULL;
            --numItems;
     	    continue;
        }
     	/** Prefix the Category .., check that a histogram with this 
     			uid/Category does not already exist... **/
     	leno = strlen(item->category);
	if (leno > 0)
	{
	    if (lenPrefix > 0) {
	      if ((lenPrefix+2+leno) > HS_MAX_CATEGORY_LENGTH) {
		fprintf(stderr, 
    "hs_read_file: Prefixed category string too long, item (id = %d) skipped\n\
    from file %s.\n", item->id, filename);
		FreeItem(item);	/* FreeHsFileStruct doesn't free items */
		(hsfile->locTblMem)[i] = NULL;
		--numItems;
		continue;
	      }
	      newcategory[lenPrefix] = '/';
	      strcpy(newcategory+lenPrefix+1, item->category);
	    }
	    else
	      strcpy(newcategory, item->category);
	}
	else
	    newcategory[lenPrefix] = '\0';
    	 
    	if (item->uid != 0 && histo_id(item->uid, newcategory) != -1) {
     	    fprintf(stderr,
           "hs_read_file: An item with the same uid/Category already exists\n");
            fprintf(stderr, "     Item (id = %d) from file %s skipped.\n",
                item->id, filename);
            FreeItem(item);	/* FreeHsFileStruct doesn't free items */
     	    (hsfile->locTblMem)[i] = NULL;
            --numItems;
     	    continue;
    	 }
    	  
   	 free(item->category);
    	 item->category = CopyNtrim(newcategory);
    	 
         makeCStruct(&item);		/* make hsC<item> of hs<item> */ 
    	 if (item == NULL) 
    	     continue;			/* skip if error in makeCStruct */
    	       	 
	 /*	 add item to HistoPtrList... */
         ++NumOfItems;
         item->id = NumOfItems;
         AddItemToHsIdList( (hsGeneral *) item);
         if (item->type == HS_1D_HISTOGRAM ||
             item->type == HS_2D_HISTOGRAM ||
             item->type == HS_3D_HISTOGRAM ||
             item->type == HS_NTUPLE)
             SetHistResetFlag(item);
         SendNewItem(item); 		/* inform connected HS's of new item */
     	 (hsfile->locTblMem)[i] = NULL;
         
#ifdef COM_DEBUG
    	 printf ("Prefixed and saved item number %d, uid %d \n", i, item->uid);
#endif /* COM_DEBUG */

    }
    free(newcategory);
    FreeHsFileStruct(&hsfile);		/* done with file */
    return numItems;
}



    
int histo_read_file_items(const char *filename, const char *prefix,
                          const char *category, int *uids, int n_uids)
	
/*   Read items from a specific category in file filename. All Category
   strings will be prefixed by a new top level category specified in prefix.
   If items already exist in the new category with the same uid, they will be
   deleted.  The array uids specifies which items to read from the file.  uids
   may be NULL, in which case all items in the category will be read.
 
	filename	Name of the file to read
	prefix		String to be added to the beginning of the categories
			of the items read from the file
	category	Category to match, must be an exact match, but matches
				all subcategories in category specified.  Null
				or empty or spaces matches uncategorized items.
				Example: category "HS" matches "HS" and "HS/Nts"
				but not "HSNTuples" nor "hs".
	uids		Array of user id values of length n_uids specifying
			which items to read.  If NULL, read all items in
			category.
	n_uids		Number of items in uids
	
	Return Value:	Number of items read or -1 if an error occured
			An error message will also be printed.
*/
{
    int i, j, lencp, leno, lenCat, numItems = 0;
    char *newcategory;
    const char *cat;
    hsGeneral *item;
    hsFile *hsfile;
    char *errString = NULL;

    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_read_file_items: Error, called before hs_initialize.\n");
    	return -1;
    }    
    if (prefix == NULL) {
	fprintf(stderr, "hs_read_file_items: Invalid prefix: .\n");
	return -1;
    }
    cat = strspn(category, " ") + category;      
    newcategory = (char *) malloc(HS_MAX_CATEGORY_LENGTH+1);
    i = strspn(prefix, " ");      
    if (strlen(prefix+i) > (HS_MAX_CATEGORY_LENGTH-2)) {
    	fprintf(stderr, 
    	    "hs_read_file_items:  Category Prefix too long, no Data read \n ");
    	free(newcategory);
    	return -1;
    }
    strcpy(newcategory, (prefix+i));
    lencp = strlen(newcategory);
/*      strcat(newcategory,"/"); */
    
    /* Open the file to read */
    hsfile = OpenHsFile(filename, HS_READ, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_read_file_items: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL) {
    	free(newcategory);
    	return -1;			/* error on open */
    }
    
    /* If file version is < V3, print error message & return */
    if (hsfile->fileversion <= HS_FILE_B) {
        fprintf(stderr, 
        "hs_read_file_items restriction: Cannot be used with a Version 2 file\n\
         or earlier.  Please use hs_read_file.\n");
    	free(newcategory);
    	FreeHsFileStruct(&hsfile);
        return -1;
    }

#ifdef COM_DEBUG
    printf("histo_read_file_items: looking for Category \"%s\" [%s], uids: %d", 
    	category, cat, uids == NULL ? 0 : uids[0]);
    for (j = 1; j < n_uids; ++j)
    	printf(", %d", uids[j]);
    printf("\nFile uid & category tables:\n i  Uid   Category\n\
 -  ---   --------\n");
#endif /*COM_DEBUG*/

    /* Read the category names, and make selection base of Catageory names
    	and uids.  Read and catalog the item.				  */
    xdr_DontReadNfitInfo(1);            /* Don't read all nfit info */
    lenCat = strlen(cat);
    for (i = 0; i < hsfile->numOfItems; ++i) {
#ifdef COM_DEBUG
	printf(" %d  %d   %s\n",i,hsfile->uidTable[i],hsfile->categoryTable[i]);
#endif /*COM_DEBUG*/
        if (strncmp(cat, hsfile->categoryTable[i], lenCat) == 0) {
	    if ((n_uids != 0) && uids != NULL) {
	        for (j = 0; j < n_uids; ++j) {
        	   if (hsfile->uidTable[i] == uids[j]) {
        	      /* matched category & uid */
        	      item = ReadHsItem(&hsfile, i, &errString);
        	      if (item->type != HS_NFIT) {
        		  ++numItems;
        		  break;
        	      }
        	   }
	       }
	       if (j == n_uids)
	       	   continue;		/* no uid match */
	    }
	    else {		      /* no uid list, just matched category */
        	item = ReadHsItem(&hsfile, i, &errString);
        	if (item->type != HS_NFIT) 
        	    ++numItems;
	    }
	    if (errString != NULL) {
    		fprintf(stderr, "\nhs_read_file_items: %s\n", errString);
    		free(errString);
    		errString = NULL;
    	    }
    	    if (hsfile == NULL) {
    	    	free(newcategory);
    	    	return(numItems > 0 ? numItems : -1);
    	    } 

     	/** Prefix the Category, check that a histogram with this 
     			uid/Category does not already exist... **/
     	leno = strlen(item->category);
	if (leno > 0)
	{    
	    if (lencp > 0) {
	      if ((lencp+2+leno) > HS_MAX_CATEGORY_LENGTH) {
		fprintf(stderr, 
			"hs_read_file_items:  Category String too long, skipping uid %d\n",
			item->uid);
		numItems--;
		FreeItem(item);
		continue;
	      }
	      newcategory[lencp] = '/';
	      strcpy(newcategory + lencp + 1, item->category);
	    }
	    else
	      strcpy(newcategory, item->category);
	}
    	else
	    newcategory[lencp] = '\0';

    	if (histo_id(item->uid, newcategory) != -1) {
     	    fprintf(stderr,
    "hs_read_file_items:  A histogram with this uid/Category already exists\n");
            fprintf(stderr, 
               " Item uid: %d, category: %s in file %s skipped. \n", item->uid,
               item->category, filename);
     	    numItems--;
     	    FreeItem(item);
     	    continue;
    	}
    	 
    	/* replace the item's category with prefixed category string */ 
    	free(item->category);
    	item->category = CopyNtrim(newcategory);

	/* add the client Communication hooks to the item */
        makeCStruct(&item);
    	if (item == NULL) 
    	    continue;			/* skip if error in makeCStruct */

        ++NumOfItems;
        item->id = NumOfItems;
        AddItemToHsIdList( (hsGeneral *) item);
        if (item->type == HS_1D_HISTOGRAM ||
            item->type == HS_2D_HISTOGRAM ||
            item->type == HS_3D_HISTOGRAM ||
            item->type == HS_NTUPLE)
            SetHistResetFlag(item);
        SendNewItem(item);

        }				/* end of category match test */
    }					/* end of for loop	      */
        
#ifdef COM_DEBUG
    printf ("histo_read_file_items: Number of Items read: %d \n", numItems);
#endif /* COM_DEBUG */    	 
    
    FreeHsFileStruct(&hsfile);
    free(newcategory);
    return (numItems);
}

 
/*** Output routines : hs_save_file, hs_save_file_items.  ****/

int histo_save_file(const char *name)
/*
* histo_save_file saves all current histograms, n-tuples, and indicators in a
* Histo-Scope-format file.  The number of items written is returned, or -1
* if an error occurs.  The file named is open for writing ("w"), discarding
* previous contents, if any.
*
* parameters:	name     - pointer to a null-terminated Unix filename string
*	  
*		Returns the number of item written, -1 if an error occurs.
*/
{
    hsFile *hsfile;
    char *errString = NULL;
    int i, numItems = 0;
    hsGeneral **locTblWrite;
    
    if (!InitializedAndActive)
    	return 0;			/* hs_initialize must be called first */
    
    hsfile = OpenHsFile(name, HS_WRITE, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_save_file: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL)
    	return -1;			/* error on open */
    	
    /* Compute the real number of items and compose a list of id's to write */
    locTblWrite = (hsGeneral **) malloc(NumOfItems * sizeof(hsGeneral *));
    for (i = 0; i < NumOfItems; ++i) {
        if (HistoPtrList[i] != NULL 
        	&& HistoPtrList[i]->type != HS_TRIGGER) {
            locTblWrite[numItems] = HistoPtrList[i];
            ++numItems;
	}
    }
    if (numItems <= 0) {
    	fprintf(stderr, 
    	    "hs_save_file: No items to write to %s\n", name); 
    	remove(name);
    	FreeHsFileStruct(&hsfile);
    	free(locTblWrite);
    	return 0;
    }
    
    i = WriteHsFile (&hsfile, locTblWrite, numItems, &errString);
    free(locTblWrite);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_save_file: %s\n", errString);
    	free(errString);
    }
    return i;
}

int histo_save_file_items(const char *name, const char *category,
                          int *uids, int n_uids)
/*
* histo_save_file saves all specified histograms, n-tuples, indicators and
* controls in a Histo-Scope-format file.  The number of items written is 
* returned, or -1 if an error occurs.  The file named is open for writing 
* ("w"), discarding previous contents, if any.
*
* parameters:	

        name      -  pointer to a null-terminated Unix filename string
	category  -  Category to match, Exact matches only.  Null == Empty
			== Uncategorized.
	uids	  -  Array of user id values of length n_uids specifying
			which items to write.  If NULL, write all items in
			category.
	n_uids	  -  Number of items in uids
	
*	Returns the number of item written, or -1 if an error occurs.
*/
	
{
    hsGeneral *item;
    hsFile *hsfile;
    char *errString = NULL;
    int i, j, k, numItems = 0;
    hsGeneral **locTblWrite;
    
    if (!InitializedAndActive)
    	return 0;			/* hs_initialize must be called first */
    
    hsfile = OpenHsFile(name, HS_WRITE, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_save_file_items: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL)
    	return -1;			/* error on open */
    	
    /* Compute the number of items and compose a list of items to write */
    if (category == NULL)
     	 category = "";
    locTblWrite = (hsGeneral **) malloc(NumOfItems * sizeof(hsGeneral *));
    for (i = 0; i < NumOfItems; ++i) {
        if (HistoPtrList[i] != NULL 
        	&& HistoPtrList[i]->type != HS_TRIGGER) {
            item = HistoPtrList[i];
	    k = strcmp(item->category, category);
	    if ((n_uids != 0) && uids != NULL && (k == 0)) {
	        for (j = 0; j < n_uids; ++j) {
        	    if (item->uid == uids[j]) {	    /* match category & uid */
        	        locTblWrite[numItems] = HistoPtrList[i];
        	        ++numItems;
        	        break;
        	    }
	        }
	    }
	    else if (k == 0) {		/* no uid list, just match category */
        	locTblWrite[numItems] = HistoPtrList[i];
        	++numItems;
	    } 
	}
    }
    if (numItems <= 0) {
    	fprintf(stderr, 
    	    "hs_save_file_items: No items matched uid/category specified\n\
   No items written to %s\n", name); 
    	remove(name);
    	FreeHsFileStruct(&hsfile);
    	free(locTblWrite);
    	return 0;
    }
    
    i = WriteHsFile (&hsfile, locTblWrite, numItems, &errString);
    free(locTblWrite);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_save_file_items: %s\n", errString);
    	free(errString);
    }
    return i;
}

static void makeCStruct(hsGeneral **item)
/*
* makeCStruct adds the client Communication hooks to an item gotten from 
* hs_read_file* routines. It first allocates the memory needed, then
* copies the existing hs<Item> part of the structure and fills in the 
* communication hooks specific to the item's type (the hsC part).
*
* This routine was written by Paul Lebrun because previously only Histo-
* scope read histo-files and did not need this extra stuff in the item's
* structure.  Note that the xdr routines accomodate the idiosyncracies
* for writing (ENCODING) histo items to files and histo-scope (ENCODING is
* still done only in the client), while this routine handles the client-side
* READing idiosyncracies.
*
* parameters:	hsGeneral pointer to point to hSGeneral
*
*/

{
   hsC1DHist *hsC1D;
   hsC2DHist *hsC2D;
   hsC3DHist *hsC3D;
   hsCNTuple *hsCNT;
   hsCControl *hsCC;
   hsControl *hsC;
   hsGeneral *itmp;
   int nt, ntC, i;
   char *c1, *t1, *c2, *t2;
   
   itmp = *item;
   c2 = (char *) itmp;
   if (c2 == NULL) {
     *item = NULL;
     return;
   }
   t2 = c2;
   switch (itmp->type ) {
    	    case HS_1D_HISTOGRAM:
    	        ntC = sizeof(hsC1DHist);
    	        nt = sizeof(hs1DHist);
    	        c1 = (char *) malloc(ntC*sizeof(char));
    	        if (c1 == NULL) {
    	           *item = NULL;
    	           return;
		} 
    	        t1 = c1;
    	        for (i=0; i<nt; i++, t1++, t2++) {
    	             *t1 = *t2;
    	             }
    	        hsC1D = (hsC1DHist *) c1;
    	        hsC1D->sendErrs    = FALSE;
    	        hsC1D->resetFlag   = 0;
                *item = (hsGeneral *) c1;
    	        free(c2);
    	    	break;
    	    case HS_2D_HISTOGRAM:
    	        ntC = sizeof(hsC2DHist);
    	        nt = sizeof(hs2DHist);
    	        c1 = (char *) malloc(ntC*sizeof(char)); 
    	        if (c1 == NULL) {
    	           *item = NULL;
    	           return;
		} 
    	        t1 = c1;
    	        for (i=0; i<nt; i++, t1++, t2++) {
		    *t1 = *t2;
		}
    	        hsC2D = (hsC2DHist *) c1;
    	        hsC2D->sendErrs    = FALSE;
    	        hsC2D->resetFlag   = 0;
                *item = (hsGeneral *) c1;
    	        free(c2);
    	    	break;
    	    case HS_3D_HISTOGRAM:
    	        ntC = sizeof(hsC3DHist);
    	        nt = sizeof(hs3DHist);
    	        c1 = (char *) malloc(ntC*sizeof(char)); 
    	        if (c1 == NULL) {
		    *item = NULL;
		    return;
		} 
    	        t1 = c1;
    	        for (i=0; i<nt; i++, t1++, t2++) {
		    *t1 = *t2;
		}
    	        hsC3D = (hsC3DHist *) c1;
    	        hsC3D->sendErrs    = FALSE;
    	        hsC3D->resetFlag   = 0;
                *item = (hsGeneral *) c1;
    	        free(c2);
    	    	break;
    	    case HS_NTUPLE:
    	        ntC = sizeof(hsCNTuple);
    	        nt = sizeof(hsNTuple);
    	        c1 = (char *) malloc(ntC); 
    	        if (c1 == NULL) {
		    *item = NULL;
		    return;
		}
    	        t1 = c1;
    	        for (i=0; i<nt; i++, t1++, t2++) {
		    *t1 = *t2;
		}
    	        hsCNT = (hsCNTuple *) c1;
    	        hsCNT->fromTuple    = 0;
    	        hsCNT->toTuple = -1;
                *item = (hsGeneral *) c1;
    	        free(c2);
    	    	break;
    	    case HS_INDICATOR:
    	        break;
    	    case HS_CONTROL:
    	        ntC = sizeof(hsCControl);
    	        nt = sizeof(hsControl);
    	        c1 = (char *) malloc(ntC*sizeof(char));
    	        if (c1 == NULL) {
    	           *item = NULL;
    	           return;
    	           } 
    	        t1 = c1;
    	        for (i=0; i<nt; i++, t1++, t2++) {
    	             *t1 = *t2;
    	             }
    	        hsCC = (hsCControl *) c1;
    	        hsCC->valueSet = 0;
    	        hsC = (hsControl *) c1;
    	        hsCC->newValue = hsC->value;
    	        hsCC->defaultValue = hsC->value;
                *item = (hsGeneral *) c1;
    	        free(c2);
    	    	break;
   	    case HS_GROUP:
    	        break;
   	    default:
    	    	fprintf(stderr,
    	    	 "hs_read_file: Internal error, invalid item type in makeC \n");
    	    	 *item = NULL;
    	    	 break;
    	    }
     return;
}    



/* This code should become a part of 'histoApiFiles.c' */

int histo_save_file_byids(const char *name, int *ids, int n_ids)
/*
* histo_save_file_byids saves all specified histograms, n-tuples, indicators
* and controls in a Histo-Scope-format file.  The number of items written is 
* returned, or -1 if an error occurs.  The file named is open for writing 
* ("w"), discarding previous contents, if any.
*
* parameters:	

        name      -  pointer to a null-terminated Unix filename string
	ids	  -  Array of histoscope id values of length n_ids specifying
			which items to write.  
	n_ids	  -  Number of items in ids
	
*	Returns the number of item written, or -1 if an error occurs.
*/
	
{
    hsGeneral *item;
    hsFile *hsfile;
    char *errString = NULL;
    int i, j, numItems = 0;
    hsGeneral **locTblWrite;
    
    if (!InitializedAndActive)
      return 0;			/* hs_initialize must be called first */
    
    hsfile = OpenHsFile(name, HS_WRITE, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhs_save_file_byids: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL)
      return -1;			/* error on open */

    /* Compute the number of items and compose a list of ids to write */
    locTblWrite = (hsGeneral **) malloc(NumOfItems * sizeof(hsGeneral *));
    if ((n_ids > 0) && ids != NULL) {
      for (i = 0; i < NumOfItems; ++i) {
	if (HistoPtrList[i] != NULL 
	    && HistoPtrList[i]->type != HS_TRIGGER) {
	  item = HistoPtrList[i];
	  for (j = 0; j < n_ids; ++j) {
	    if (item->id == ids[j]) {	    /* match id */
	      locTblWrite[numItems] = HistoPtrList[i];
	      ++numItems;
	      break;
	    }
	  }
	}
      }
    }
    if (numItems <= 0) {
      fprintf(stderr, 
	      "hs_save_file_byids: No items matched ids specified\n\
No items written to %s\n", name); 
      remove(name);
      FreeHsFileStruct(&hsfile);
      free(locTblWrite);
      return 0;
    }
    
    i = WriteHsFile (&hsfile, locTblWrite, numItems, &errString);
    free(locTblWrite);
    if (errString != NULL) {
      fprintf(stderr, "\nhs_save_file_byids: %s\n", errString);
      free(errString);
    }
    return i;
}


int histo_pack_item(int id, void **mem)
{
    char *databuf = NULL;
    hsGeneral *item;
    XDR xdrs1;
    int r = 0, datsiz = 0, buflen = 0;
    enum utype u_type;
    hsNTuple *nT = NULL;

    *mem = NULL;
    if (!InitializedAndActive)
    {
    	fprintf(stderr,
    		"hs_pack_item: Error, called before hs_initialize.\n");
    	return 0;
    }
    item = GetItemByPtrID(id);
    if (item == NULL) {
    	fprintf(stderr, "hs_pack_item: item (id = %d) not found.\n", id);
    	return 0;
    }
    switch (item->type) {
    case HS_1D_HISTOGRAM:
        r = 1;
        if (((hs1DHist *)item)->pErrs != NULL)
            ++r;
        if (((hs1DHist *)item)->mErrs != NULL)
            ++r;
        datsiz = sizeof(hs1DHist) + r * (sizeof(float)
	    	    	 * ((hs1DHist *)item)->nBins);
        datsiz += 2*(HS_MAX_LABEL_LENGTH + 1);
        u_type = FILE_1DHIST;
        break;
    case HS_2D_HISTOGRAM:
        r = 1;
        if (((hs2DHist *)item)->pErrs != NULL)
            ++r;
        if (((hs2DHist *)item)->mErrs != NULL)
            ++r;
        datsiz = sizeof(hs2DHist) + r * (sizeof(float) * 
                 ((hs2DHist *) item)->nXBins * ((hs2DHist *)item)->nYBins);
        datsiz += 3*(HS_MAX_LABEL_LENGTH + 1);
        u_type = FILE_2DHIST;
        break;
    case HS_3D_HISTOGRAM:
        r = 1;
        if (((hs3DHist *)item)->pErrs != NULL)
            ++r;
        if (((hs3DHist *)item)->mErrs != NULL)
            ++r;
        datsiz = sizeof(hs3DHist) + r * (
            sizeof(float) * 
            ((hs3DHist *)item)->nXBins *
            ((hs3DHist *)item)->nYBins * 
            ((hs3DHist *)item)->nZBins);
        datsiz += 4*(HS_MAX_LABEL_LENGTH + 1);
        u_type = FILE_3DHIST;
        break;
    case HS_NTUPLE:
        nT = (hsNTuple *)item;
        datsiz = sizeof(hsNTuple) + ( sizeof(float) * 
                                      nT->n * nT->nVariables );
        datsiz += nT->nVariables*(HS_MAX_NAME_LENGTH+1);
        u_type  = FILE_NTUPLE;
        break;
    case HS_INDICATOR:
        datsiz = sizeof(hsIndicator) + 6;
        u_type = FILE_INDICATOR;
        break;
    case HS_CONTROL:
        datsiz = sizeof(hsControl) + 6;
        u_type = FILE_CONTROL;
        break;
    default:
        fprintf(stderr, "Internal error: hs_pack_item can not "
                "pack item of type %d.\n", item->type);
        return 0;
    }
    datsiz += (HS_MAX_TITLE_LENGTH + 1);
    datsiz += (HS_MAX_CATEGORY_LENGTH + 1);
    databuf = (char *)malloc(datsiz);
    if (databuf == NULL)
    {
    	fprintf(stderr,
                "hs_pack_item: Ran out of memory. Item not packed.\n");
        return 0;
    }
    xdr_SetFileVer(HS_FILE_D);
    xdrmem_create(&xdrs1, databuf, datsiz, XDR_ENCODE);
    if (!xdr_Item(&xdrs1, &item, &u_type)) {
        fprintf(stderr,
                "hs_pack_item: Error in xdr translation "
                "(item %d, buffer length %d).\n", id, datsiz);
	xdr_destroy (&xdrs1);
	free(databuf);
        return 0;
    }
    buflen = xdr_getpos(&xdrs1);
    xdr_destroy(&xdrs1);
    *mem = (void *)databuf;
    return buflen;
}

int histo_unpack_item(void *mem, int len, const char *i_prefix)
{
    XDR xdrs1;
    hsGeneral *item = NULL;
    enum utype u_type;
    int lenpref = 0;
    const char *prefix = NULL;

    if (!InitializedAndActive)
    {
    	fprintf(stderr,
    		"hs_unpack_item: Error, called before hs_initialize.\n");
    	return -1;
    }
    if (mem == NULL)
    {
    	fprintf(stderr, "hs_unpack_item: NULL buffer "
                "pointer. Item not created.\n");
        return -1;
    }
    if (len <= 0)
    {
    	fprintf(stderr, "hs_unpack_item: buffer "
                "size is not positive. Item not created.\n");
        return -1;
    }
    xdr_SetFileVer(HS_FILE_D);
    xdrmem_create(&xdrs1, (char *)mem, len, XDR_DECODE);
    if (!xdr_Item(&xdrs1, &item, &u_type) || item == NULL
        || item->id <= 0) {
    	fprintf(stderr, "hs_unpack_item: Error in xdr translation.\n");
        xdr_destroy(&xdrs1);
        return -1;
    }
    xdr_destroy(&xdrs1);

    if (i_prefix)
        prefix = i_prefix + strspn(i_prefix, " ");
    if (prefix)
        lenpref = strlen(prefix);
    if (lenpref > 0)
    {
        /* We will need to change the category */
        char newcategory[HS_MAX_CATEGORY_LENGTH+1];
        int leno = strlen(item->category);
        if (leno + lenpref + 2 > HS_MAX_CATEGORY_LENGTH)
        {
            fprintf(stderr, "hs_unpack_item: Prefixed category string is "
                    "too long, will not unpack uid %d.\n", item->uid);
            FreeItem(item);
            return -1;
        }
        strcpy(newcategory, prefix);
        if (leno > 0)
        {
            newcategory[lenpref] = '/';
            strcpy(newcategory + lenpref + 1, item->category);
        }
        free(item->category);
        item->category = CopyNtrim(newcategory);
    }

    if (histo_id(item->uid, item->category) != -1) {
        fprintf(stderr,
                "hs_unpack_item: an item with this uid/Category already exists\n");
        fprintf(stderr, 
                "Item with uid %d and category \"%s\" is not unpacked.\n",
                item->uid, item->category);
        FreeItem(item);
        return -1;
    }

    /* add the client Communication hooks to the item */
    makeCStruct(&item);
    if (item == NULL)
    {
        fprintf(stderr, "hs_unpack_item: failed to add communication hooks.\n");
        return -1;
    }

    ++NumOfItems;
    item->id = NumOfItems;
    AddItemToHsIdList( (hsGeneral *) item);
    if (item->type == HS_1D_HISTOGRAM ||
        item->type == HS_2D_HISTOGRAM ||
        item->type == HS_3D_HISTOGRAM ||
        item->type == HS_NTUPLE)
        SetHistResetFlag(item);
    SendNewItem(item);

    return item->id;
}
