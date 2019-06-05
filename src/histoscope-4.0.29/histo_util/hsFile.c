/*******************************************************************************
*									       *
* hsFile.c -- HS File Access Routines for the Nirvana Histoscope tool	       *
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
* March 3, 1994								       *
*									       *
* Written by Joy Kyriakopulos				       		       *
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
#include <file.h>
#include <perror.h>
#include <mulsys/types.h>
#include <mul/errno.h>
#include "../util/VMSparam.h"
#else /*VMS*/
#ifdef VXWORKS
#include "xdr_stdio.h"
#else
#include <sys/param.h>
#endif /*VXWORKS*/
#endif /*VMS*/
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include "hsTypes.h"
#include "hsFile.h"
#define DONT_DEFINE_MSG_STRUCT	/* ask histprotocol.h not to define */
#include "histprotocol.h"
#define  XDR_FILTERS_HERE
#include "xdrHisto.h"
#define  MAX_ERROR_STRING_LEN 198
#define  V2_MAX_ITEMS_DIRECTORIED 8193
#define  V2_MAX_ITEMS_READ 1025

static void closeFile(hsFile **hsfilePtr, int numRead);
static int dirHsFileV2(hsFile **hsfilePtr, char **errorString);

/*
 *  OpenHsFile : Open a Histo-Scope format file
 *
 *	filename    - filename for opening file.
 *
 *	readOrWrite = HS_READ or HS_WRITE.
 *
 *	errorString - A string will be created if there is an error/warning
 *		      message to print.  Otherwise errorString will be
 *		      set to NULL.  Free errorString after informing user of
 *		      the error or warning.
 *
 *	Returns     - Malloc'd and initialized hsFile structure ready for
 *		      calling ReadFile(Items), WriteFile(Items), or DirFile.
 *		      Returns NULL if irrecoverable error.
 *		      Valid hsFile fields returned:
 *			   .filename
 *			   .filePtr
 *			   .xdrs
 *			   .xdrEnOrDeCode
 *			   .fileversion
 *		      In addition, for HS_READ valid hsFile fields are:
 *		           .numOfItems		( =    0, if < V3)
 *		           .locTblFile		( = NULL, if < V3)
 *		           .uidTable		( = NULL, if < V3)
 *		           .categoryTable	( = NULL, if < V3)
 */
hsFile *OpenHsFile (const char *filename, int readOrWrite, char **errorString)
{
    int i;
    char *id_string;
    u_int unumt;
    hsFile *hsfile;
    
    /* Malloc and Initialize hsFile structure and errorString */
    hsfile = (hsFile *) malloc(sizeof(hsFile));
    *errorString = (char *) malloc(MAX_ERROR_STRING_LEN);
    hsfile->filename = (char *) malloc(strlen(filename)+1);
    strcpy(hsfile->filename, filename);
    hsfile->locTblFile = NULL;
    hsfile->locTblMem = NULL;
    hsfile->dataPos = NULL;
    hsfile->numOfItems = 0;
    hsfile->uidTable = NULL;
    hsfile->categoryTable = NULL;
    
    if (readOrWrite == HS_READ)			/* open for read */
    	hsfile->xdrEnOrDeCode = XDR_DECODE;
    else{					/* open for write */
    	hsfile->xdrEnOrDeCode = XDR_ENCODE;
    	hsfile->fileversion = HS_FILE_D;
    }
    
    /* Open file */
    hsfile->filePtr = fopen(hsfile->filename,
    				readOrWrite == HS_READ ? "r" : "w");
#if defined(VXWORKS) && defined(COM_DEBUG)
    printf("in OpenHsFile:  just opened %s for %s.  filePtr = %p\n", 
    	hsfile->filename, readOrWrite == HS_READ ? "read" : "write", 
    	hsfile->filePtr);
#endif /*VXWORKS && COM_DEBUG*/  
    if (hsfile->filePtr == NULL) {
    	sprintf(*errorString, "Error opening file %s \nError message: %s \n",
    	        hsfile->filename,  strerror(errno));
    	free(hsfile->filename);
    	free(hsfile);
    	return NULL;
    }
    
    /* Create the xdr stream for reading/writing */
    hsfile->xdrs = (XDR *) malloc(sizeof(XDR));
    xdrstdio_create(hsfile->xdrs, hsfile->filePtr, hsfile->xdrEnOrDeCode);
    
    /* Read/write ID string */
    if (readOrWrite == HS_READ)
    	id_string = NULL;		      /* xdr_string will do malloc */	
    else {
    	id_string = malloc(strlen(HS_ID_STRING_C)+1);
    	strcpy(id_string, HS_ID_STRING_C);
    }
    if (!xdr_string(hsfile->xdrs, &id_string, MAX_HS_ID_STRING_LEN)) {
    	sprintf(*errorString,
    	       "An error has occurred while %s the Histo-Scope File: %s\n",
    	       readOrWrite == HS_READ ? "reading" : "writing", filename);
    	if (readOrWrite == HS_READ)
    	    strcat(*errorString, 
    	    	"  File is probably not a Histo-Scope format file.\n");
    	else
    	    remove(hsfile->filename);
    	FreeHsFileStruct(&hsfile); 
    	return NULL;
    }
    
    /* Verify file version is one we can read (i.e. <= V3)	*/
    if (readOrWrite == HS_READ && strcmp(id_string, HS_ID_STRING_A) != 0
    		&& strcmp(id_string, HS_ID_STRING_B) != 0
    		&& strcmp(id_string, HS_ID_STRING_C) != 0) {
	sprintf(*errorString, 
    	    "Histo-Scope File ID different than expected, name: %s\n  id: %s\n",
   	    filename, id_string);
    	free(id_string);
    	FreeHsFileStruct(&hsfile);
    	return NULL;
    }
    
    /* Set file version					         */
    /* And we're done if open was for write or fileversion < v3  */
    if (strcmp(id_string, HS_ID_STRING_D) == 0) { 
    	hsfile->fileversion = HS_FILE_D;		/* v4 */ 
    	if (readOrWrite != HS_READ) {
    	    free(*errorString);
    	    free(id_string);
    	    *errorString = NULL;
    	    return hsfile;
        }
    }
    else if (strcmp(id_string, HS_ID_STRING_C) == 0) { 
    	hsfile->fileversion = HS_FILE_C;		/* v3 */ 
    	if (readOrWrite != HS_READ) {
    	    free(*errorString);
    	    free(id_string);
    	    *errorString = NULL;
    	    return hsfile;
        }
    }
    else {
        sprintf(*errorString, "HistoScope file %s is older version\n   %s\n", 
        	filename, "No uids will be defined for these items.");
        if (strcmp(id_string, HS_ID_STRING_B) == 0)  
    	    hsfile->fileversion = HS_FILE_B;		/* v2 */
        else 
    	    hsfile->fileversion = HS_FILE_A;		/* v1 */
    	free(id_string);
    	return hsfile;
    }
    
    /* Opened V3 or higher file for read:	*/
    /* Read the number of items in this file	*/
    free(id_string);
    if (!xdr_int(hsfile->xdrs, &hsfile->numOfItems)) {
    	sprintf (*errorString, 
    	    "Error reading header of file %s.  No data read.\n", filename);
    	FreeHsFileStruct(&hsfile);
    	return NULL;
    }

    	
#ifdef COM_DEBUG
    printf (" Number of Items in file %s: %d.\n", hsfile->filename,
    	    hsfile->numOfItems);
#endif /* COM_DEBUG */    	 
    
    /* Read the Locator table... */
    if (!xdr_array(hsfile->xdrs, (caddr_t *) &hsfile->locTblFile, &unumt, 
    	     (u_int) hsfile->numOfItems, sizeof(int), xdr_int)) {  /* xdr does malloc */
    	sprintf (*errorString, 
    	    "Error - cannot read Locator table for file %s.\n  No data read.\n",
    	     filename);
    	FreeHsFileStruct(&hsfile);
    	return NULL;
    }
    (*errorString)[0] = '\0';		/* flags whether warning msg returned */
    if (unumt != (unsigned)(hsfile->numOfItems))
    	sprintf (*errorString, 
    	    "Warning: Possible inconsistency in reading file: %s.  (%u, %d)\n",
    	     filename, unumt, hsfile->numOfItems);
    
#ifdef COM_DEBUG
    printf ("  First item position for file %s is: %d \n", filename, 
    	     *hsfile->locTblFile);
    if (unumt != hsfile->numOfItems)
    	printf (
    	    "Warning: Possible inconsistency in reading file: %s.  (%d, %d)\n",
    	     filename, unumt, hsfile->numOfItems);
#endif /* COM_DEBUG */    	 
    
    /* Read Uid Table... */
    if (!xdr_array(hsfile->xdrs, (char **)&hsfile->uidTable, &unumt, 
    	    hsfile->numOfItems, sizeof(int), xdr_int)) {   /* xdr does malloc */
    	sprintf (*errorString,  
    	    "Error - Can't Read the Uid table for file %s.\n  No data read.\n",
    	    hsfile->filename);
    	FreeHsFileStruct(&hsfile);
    	return NULL;
    }
    if (unumt != (unsigned)(hsfile->numOfItems))
    	sprintf (*errorString, 
    	    "Warning: possible inconsistency in reading file: %s.  (%u, %d)\n",
    	     filename, unumt, hsfile->numOfItems);
    
    /* Read the Category Table.  Malloc memory and make pointers null, so that
       an xdr error won't crash the application when cat strings are freed */
    hsfile->categoryTable = (char **) malloc(hsfile->numOfItems*sizeof(char *));
    for (i = 0; i < hsfile->numOfItems; ++i)
    	hsfile->categoryTable[i] = NULL;
    if (!xdr_array(hsfile->xdrs, (char **)&hsfile->categoryTable, &unumt, 
             (u_int)hsfile->numOfItems, sizeof(char *), xdr_wrapstring) ) {
             /* xdr does malloc */ 
    	sprintf (*errorString,  
    	   "Error - Reading the Category table for file %s.\n  No data read.\n",
    	    hsfile->filename);
    	FreeHsFileStruct(&hsfile);
    	return NULL;
    }
    if (unumt != (unsigned)(hsfile->numOfItems))
    	sprintf (*errorString, 
    	    "Warning: possible inconsistency in file: %s.  (%u, %d)\n",
    	     filename, unumt, hsfile->numOfItems);

    /* Done */
    if ((*errorString)[0] == '\0') {
    	free(*errorString);
    	*errorString = NULL;			/* no warning message */
    }
    return hsfile;
}

/*
 *  DirHsFile : Produce a directory of a Histo-Scope format file, reading each
 *		item's structure, but not its data.
 *
 *	hsfile      - hsFile structure returned by OpenHsFile (file must have
 *		      been opened for read, HS_READ).
 *
 *	nfitFlag    - Integer set to 0 if no Nfit item is found in file.
 *		      Otherwise is set to 1.
 *
 *	errorString - A string will be created if there is an error/warning
 *		      message to print.  Otherwise errorString will be
 *		      set to NULL.  Free errorString after informing user of
 *		      the error or warning.
 *
 *	Returns     - Number of items successfully read or -1 if error.
 *
 *		      After calling DirHsFile, all hsFile fields for v3 files
 *		      are filled in, allowing a program to output a 
 *		      directory of the file in its own chosen format.
 *		      In addition, the hsFile structure can subsequently 
 *		      be used with ReadHsItemData to read an item's data. 
 *		      For v2 or earlier files, the following tables are
 *		      NULL: .locTblFile, .uidTable, .categoryTable, and
 *		      .dataPos.
 */
int DirHsFile (hsFile **hsfilePtr, int *nfitFlag, char **errorString)
{
    hsFile *hsfile = *hsfilePtr;
    int index;
    hsGeneral *item;
    enum utype u_type;

    /* Allocate errorString, to be used if error */
    *errorString = (char *) malloc(MAX_ERROR_STRING_LEN + 1);
    (*errorString)[0] = '\0';	    /* flags whether warning msg returned */
    
    /* Ensure hsfile structure was successfully opened for read */
    if (hsfile->xdrEnOrDeCode != XDR_DECODE || hsfile->xdrs == NULL || 
    	    hsfile->filename == NULL || hsfile->filePtr == NULL) {
    	fprintf(stderr, "Internal error calling DirHsFile for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling DirHsFile for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    
    /* Set file version for xdr_Item */
    xdr_SetFileVer(hsfile->fileversion);
    						
    /* Pre-Version 3 files are a special case... */
    if (hsfile->fileversion < HS_FILE_C)
    	return dirHsFileV2(hsfilePtr, errorString);

    /* more consistency checking...*/
    if (hsfile->numOfItems <= 0 || hsfile->locTblFile == NULL || 
    	    hsfile->uidTable == NULL || hsfile->categoryTable == NULL) {
    	fprintf(stderr, "Internal error calling DirHsFile for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling DirHsFile for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    
    /* Initialize caller's nfitFlag to 0			*/
    /* nfitFlag will be set to 1 if there is an nfit item in file	*/
    *nfitFlag = 0;
    
    /* Create memory for Location table and Data Position Table */
    hsfile->dataPos = (int *) malloc(hsfile->numOfItems * sizeof(int));
    hsfile->locTblMem = (hsGeneral **) malloc(hsfile->numOfItems 
    						* sizeof(hsGeneral *));
    for (index = 0; index < hsfile->numOfItems; ++index) {
    	hsfile->locTblMem[index] = NULL;
    	hsfile->dataPos[index] = 0;
    }
    						
    /* Read each item, saving item pointer to memory Location table 
       and data position to Data Position Table */
    for (index = 0; index < hsfile->numOfItems; ++index) {
	if (!xdr_setpos(hsfile->xdrs, hsfile->locTblFile[index])) {
    	    sprintf(*errorString, "Error retrieving item from file %s. \n", 
    		   hsfile->filename);
    	    FreeHsFileStruct(hsfilePtr);
    	    return 0;
	}
    	item = NULL;
    	if (!xdr_ItemDir(hsfile->xdrs, &item, &u_type) || item == NULL
    		|| item->id <= 0) {
    	    if (feof(hsfile->filePtr)) {
    		if (index < hsfile->numOfItems) {
    		    sprintf(*errorString, 
  "Warning - Number of items (%d) in file %s is inconsistent \
  with number stored in header (%d).\n", 
    			index, hsfile->filename, hsfile->numOfItems);
    		    fprintf(stderr, 
  "Warning - number of items (%d) in file %s is inconsistent \
    			with number stored in header (%d).\n", 
    			index, hsfile->filename, hsfile->numOfItems);
    		}
    		return (index <= 0 ? -1 : index);
    	    }
    	    else {
    		sprintf(*errorString, 
    		   "Error getting directory of file %s. Remainder skipped.\n", 
    		   hsfile->filename);
    		closeFile(hsfilePtr, index);
    		return (index <= 0 ? -1 : index);
    	    }
    	}
    	(hsfile->dataPos)[index] = xdr_getpos(hsfile->xdrs);  /* save data pos */
    	(hsfile->locTblMem)[index] = item;	   /* save item pointer    */
    	if (item->type == HS_NFIT)
    	    *nfitFlag = 1;
    	
#ifdef COM_DEBUG
    	printf ("Directoried item number %d, uid %d, %s\n", index, 
    		item->uid, item->title);
#endif /* COM_DEBUG */

    	if (index >= hsfile->numOfItems) {
	    /* Done (normal return for fileversion >= v3) */
	    if ((*errorString)[0] == '\0') {
    		free(*errorString);
    		*errorString = NULL;		   /* no warning message */
	    }
    	    return (hsfile->numOfItems);
    	}
    }

    /* Done */
    if ((*errorString)[0] == '\0') {
    	free(*errorString);
    	*errorString = NULL;			/* no warning message */
    }
    return hsfile->numOfItems;
}

static int dirHsFileV2(hsFile **hsfilePtr, char **errorString)
{
    hsFile *hsfile = *hsfilePtr;
    int index = 0;
    hsGeneral *item;
    enum utype u_type;

    /* Read each item, saving item pointer to memory Location table */
    hsfile->locTblMem = (hsGeneral **) malloc(V2_MAX_ITEMS_DIRECTORIED 
    						* sizeof(hsGeneral *));
    memset(hsfile->locTblMem, 0, V2_MAX_ITEMS_DIRECTORIED*sizeof(hsGeneral *));
    
    while (1) {
    	item = NULL;
    	if (!xdr_Item(hsfile->xdrs, &item, &u_type) || item == NULL
    		|| item->id <= 0) {
    	    if (feof(hsfile->filePtr)) {
		/* Done (normal return for fileversion < v3) */
		if ((*errorString)[0] == '\0') {
    		    free(*errorString);
    		    *errorString = NULL;	     /* no warning message */
		}
    		return (index <= 0 ? -1 : index);
    	    }
    	    else {
    		sprintf(*errorString, 
    		   "Error getting directory of file %s. Remainder skipped.\n", 
    		   hsfile->filename);
    		closeFile(hsfilePtr, index);
    		return (index <= 0 ? -1 : index);
    	    }
    	}
    	(hsfile->locTblMem)[index] = item;	 /* save item pointer   */
    	switch(item->type) {			 /* throw away the data */
    	    case HS_1D_HISTOGRAM:
    	    	if (((hs1DHist *)item)->bins != NULL) {
    	    	    free(((hs1DHist *)item)->bins);
    	    	    ((hs1DHist *)item)->bins = NULL;
    	    	}
    	    	break;
    	    case HS_2D_HISTOGRAM:
    	    	if (((hs2DHist *)item)->bins != NULL) {
    	    	    free(((hs2DHist *)item)->bins);
    	    	    ((hs2DHist *)item)->bins = NULL;
    	    	}
    	    	break;
    	    case HS_3D_HISTOGRAM:
    	    	if (((hs3DHist *)item)->bins != NULL) {
    	    	    free(((hs3DHist *)item)->bins);
    	    	    ((hs3DHist *)item)->bins = NULL;
    	    	}
    	    	break;
    	    case HS_NTUPLE:
    	    	if (((hsNTuple *)item)->data != NULL) {
    	    	    free(((hsNTuple *)item)->data);
    	    	    ((hsNTuple *)item)->data = NULL;	/* no data in ext's */
    	    	}
    	    	break;
    	}
    	++index;
    	
#ifdef COM_DEBUG
    printf ("Directoried item number %d, uid %d, %s\n", index, 
    	    item->uid, item->title);
#endif /* COM_DEBUG */

    }
}

/*
 *  ReadHsFile : Read an entire Histo-Scope format file
 *
 *	hsfilePtr   - address of pointer to hsFile structure as returned by
 *		      OpenHsFile.  The hs file must have been opened for
 *		      read (HS_READ).  All file information is returned
 *		      as part of the hsfile structure.  Once pertinent
 *		      information is taken from this structure, 
 *		      FreeHsFileStruct can be called to "close" the structure
 *		      and file.  However, be warned that whenever a table 
 *		      pointer is not null, FreeHsFileStruct will free the
 *		      tables they point to.  For instance, copying .locTblFile
 *		      (the pointer to the table) and then setting it to NULL
 *		      is quite OK to save time copying the tables if you know
 *		      you're only going to call FreeHsFileStruct next anyway.
 *
 *		      If an error occurs and no items were read, the hsfile
 *		      structure is freed and hsfilePtr is set to NULL.  If
 *		      an error occurs but items were read, the file and
 *		      xdr stream are closed; however the items read can be
 *		      retrieved from .locTblMem if desired.
 *
 *	errorString - A string will be created if there is an error/warning
 *		      message to print.  Otherwise errorString will be
 *		      set to NULL.  Free errorString after informing user of
 *		      the error or warning.
 *
 *	Returns     - Number of items successfully read or -1 if error.
 */
int ReadHsFile (hsFile **hsfilePtr, char **errorString)
{
    hsFile *hsfile = *hsfilePtr;
    int i, numRead = 0, numOfItems;
    hsGeneral *item;
    enum utype u_type;

    /* Allocate errorString, to be used if error */
    *errorString = (char *) malloc(MAX_ERROR_STRING_LEN);
    (*errorString)[0] = '\0';	    /* flags whether warning msg returned */
    
    /* Ensure hsfile structure was successfully opened for read */
    if (hsfile->xdrEnOrDeCode != XDR_DECODE || hsfile->xdrs == NULL || 
    	    hsfile->filename == NULL || hsfile->filePtr == NULL) {
    	fprintf(stderr, "Internal error calling ReadHsFile for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling ReadHsFile for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    
    if (hsfile->fileversion >= HS_FILE_C &&  
    	    (hsfile->numOfItems <= 0 || hsfile->locTblFile == NULL || 
    	    hsfile->uidTable == NULL || hsfile->categoryTable == NULL)) {
    	fprintf(stderr, "Internal error calling ReadHsFile for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling ReadHsFile for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    
    /* so that code works for all versions of files... */
    if (hsfile->fileversion >= HS_FILE_C) {
    	numOfItems = hsfile->numOfItems;
    }
    else {
    	numOfItems = V2_MAX_ITEMS_READ;
    }
    
    /* Set file version for xdr_Item  & allocate memory Location table */
    xdr_SetFileVer(hsfile->fileversion);
    hsfile->locTblMem = (hsGeneral **) malloc(numOfItems * sizeof(hsGeneral *));
    for (i = 0; i < numOfItems; ++i)
    	hsfile->locTblMem[i] = NULL;
    						
    /* Read each item, saving item pointer to memory Location table */
    while (1) {
    	item = NULL;
    	if (!xdr_Item(hsfile->xdrs, &item, &u_type) || item == NULL
    		|| item->id <= 0) {
    	    if (feof(hsfile->filePtr)) {
    		if (hsfile->fileversion >= HS_FILE_C 
    	    		&& numRead < hsfile->numOfItems) {
    		    sprintf(*errorString, 
  "Warning - Number of items (%d) in file %s is inconsistent \
  with number stored in header (%d).\n", 
    			numRead, hsfile->filename, hsfile->numOfItems);
    		    fprintf(stderr, 
  "Warning - number of items (%d) in file %s is inconsistent \
    			with number stored in header (%d).\n", 
    			numRead, hsfile->filename, hsfile->numOfItems);
    		}
		/* Done (normal return for fileversion < v3) */
		if ((*errorString)[0] == '\0') {
    		    free(*errorString);
    		    *errorString = NULL;	      /* no warning message */
		}
    		return (numRead <= 0 ? -1 : numRead);
    	    }
    	    else {
    		sprintf(*errorString, 
    		   "Error reading data in file %s. Remainder skipped.\n", 
    		   hsfile->filename);
    		closeFile(hsfilePtr, numRead);
    		return (numRead <= 0 ? -1 : numRead);
    	    }
    	}
    	(hsfile->locTblMem)[numRead] = item;	   /* save item pointer */
    	++numRead;
    	
#ifdef COM_DEBUG
    	printf ("Read and Stored item number %d, uid %d, %s\n", numRead, 
    		item->uid, item->title);
#endif /* COM_DEBUG */

    	if (numRead >= numOfItems) {
	    /* Done (normal return for fileversion >= v3) */
	    if (hsfile->fileversion < HS_FILE_C)
    		sprintf(*errorString, 
    		    "Restriction: Only %d items read from file.\n", numRead);
	    if ((*errorString)[0] == '\0') {
    		free(*errorString);
    		*errorString = NULL;		   /* no warning message */
	    }
    	    return (numRead);
    	}
    }
}

/*
 *  ReadHsItem : Read one item from a Histo-Scope format file
 *
 *	hsfilePtr   - address of pointer to hsFile structure as returned by
 *		      OpenHsFile.  The hs file must have been opened for
 *		      read (HS_READ).  
 *
 *		      If an error occurs the hsfile structure is freed 
 *		      and hsfilePtr is set to NULL.  
 *
 *		      Upon success, hsfile.locTblMem[index] is set to the 
 *		      item's location in memory and its address is returned.
 *
 *	index       - The index into the hsfile tables (i.e. .locTblFile, 
 *		      .categoryTable, .uidTable) of the item to read.  This
 *		      can be regarded as the hs id of the item in the file.
 *
 *	errorString - A string will be created if there is an error/warning
 *		      message to print.  Otherwise errorString will be
 *		      set to NULL.  Free errorString after informing user of
 *		      the error or warning.
 *
 *	Returns     - Pointer to the hsItem or NULL if error.
 */
hsGeneral *ReadHsItem (hsFile **hsfilePtr, int index, char **errorString)
{
    hsFile *hsfile = *hsfilePtr;
    int i;
    hsGeneral *item;
    enum utype u_type;

    /* Allocate errorString, to be used if error */
    *errorString = (char *) malloc(MAX_ERROR_STRING_LEN);
    (*errorString)[0] = '\0';	    /* flags whether warning msg returned */

    /* Ensure index is within tables */
    if (index < 0 || index >= hsfile->numOfItems) {
    	fprintf(stderr, "Internal error calling ReadHsFile for %s\n\
    	    index is out of range (%d).", hsfile->filename, index);
    	sprintf(*errorString, "Internal error calling ReadHsItem for %s\n\
    	    index is out of range (%d).", hsfile->filename, index);
    	return NULL;
    }
    
#ifdef COM_DEBUG
    /* Ensure hsfile structure was successfully opened for read */
    if (hsfile->xdrEnOrDeCode != XDR_DECODE || hsfile->xdrs == NULL || 
    	    hsfile->filename == NULL || hsfile->filePtr == NULL) {
    	fprintf(stderr, "Internal error calling ReadHsItem for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling ReadHsItem for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return NULL;
    }
    
    if (hsfile->fileversion < HS_FILE_C || (hsfile->fileversion >= HS_FILE_C &&  
    	    (hsfile->numOfItems <= 0 || hsfile->locTblFile == NULL || 
    	    hsfile->uidTable == NULL || hsfile->categoryTable == NULL))) {
    	fprintf(stderr, "Internal error calling ReadHsItem, file: %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling ReadHsItem, file: %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return NULL;
    }
#endif /*COM_DEBUG*/
   
    /* Set file version for xdr_Item  & position file */
    xdr_SetFileVer(hsfile->fileversion);
    if (xdr_setpos(hsfile->xdrs, hsfile->locTblFile[index])) {
    
        /* Read the item, saving its location to the memory Location table */
    	item = NULL;
    	if (!xdr_Item(hsfile->xdrs, &item, &u_type) || item == NULL
    		|| item->id <= 0) {
    	    sprintf(*errorString, "Error reading data item in file %s. \n", 
    	       hsfile->filename);
    	    FreeHsFileStruct(hsfilePtr);
    	    return NULL;
    	}
    	if (hsfile->locTblMem == NULL) {
    	    hsfile->locTblMem = (hsGeneral **) malloc(hsfile->numOfItems *
    	    	sizeof(hsGeneral *));
	    for (i = 0; i < hsfile->numOfItems; ++i)
    		hsfile->locTblMem[i] = NULL;
    	}
    	(hsfile->locTblMem)[index] = item;	   /* save item pointer */
    	
#ifdef COM_DEBUG
    	printf ("Read and Stored item number %d, uid %d, %s\n", index, 
    		item->uid, item->title);
#endif /* COM_DEBUG */

    	free(*errorString);
    	*errorString = NULL;			  /* no warning message */
    	return (item);				  /* normal return */
    }
    else { 					  /* error in xdr_setpos */
    	sprintf(*errorString, "Error retrieving data item from file %s. \n", 
    	       hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return NULL;
    }
}

/*
 *  WriteHsFile : Write a Histo-Scope format file
 *
 *	hsfilePtr   - address of a pointer to the hsFile structure returned by
 *		      OpenHsFile.  The hs file must have been opened for write
 *		      (HS_WRITE).  Upon return from WriteHsFile, the hsfile 
 *		      structure is freed and hsfilePtr set to NULL. 
 *
 *	locTbl      - points to a table of pointers of items to write.  This
 *		      table should NOT contain NULL members NOR triggers.
 *
 *	numOfItems  - number of items in locTbl.
 *
 *	errorString - A string will be created if there is an error/warning
 *		      message to print.  Otherwise errorString will be
 *		      set to NULL.  Free errorString after informing user of
 *		      the error or warning.
 *
 *	Returns     - Number of items successfully written or -1 if error.
 */
int WriteHsFile (hsFile **hsfilePtr, hsGeneral **locTbl, int numOfItems,
			 char **errorString)
{
    int i, locOfTblFile;
    hsFile *hsfile = *hsfilePtr;
    u_int unumt;
    enum utype u_type;

    /* Allocate errorString, to be used if error */
    *errorString = (char *) malloc(MAX_ERROR_STRING_LEN);
    (*errorString)[0] = '\0';	    /* flags whether warning msg returned */

    /* Ensure hsfile structure was opened for write */
    if (hsfile->xdrEnOrDeCode != XDR_ENCODE || hsfile->xdrs == NULL) {
    	fprintf(stderr, "Internal error calling WriteHsFile for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling WriteHsFile for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    
#ifdef COM_DEBUG
    if (hsfile->filename == NULL || hsfile->filePtr == NULL || 
    	    hsfile->locTblFile != NULL || hsfile->numOfItems != 0 || 
    	    numOfItems <= 0 || hsfile->uidTable != NULL || 
    	    hsfile->categoryTable != NULL || hsfile->locTblMem != NULL || 
    	    hsfile->dataPos != NULL) {
    	fprintf(stderr, "Internal error calling WriteHsFile for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling WriteHsFile for %s\n",
    	    hsfile->filename);
    	return -1;
    }
#endif /*COM_DEBUG*/ 
   
    /* Set file version for xdr_Item (OpenHsFile wrote this to output file */
    xdr_SetFileVer(hsfile->fileversion);
    
    /* Write number of items to file */
    hsfile->numOfItems = numOfItems;
    if (!xdr_int(hsfile->xdrs, &hsfile->numOfItems)) {
    	 remove (hsfile->filename);
    	 sprintf(*errorString, 
    	     "Error writing number of items.  File %s deleted.\n",
    	     hsfile->filename);
    	 FreeHsFileStruct(hsfilePtr);
    	 return -1;
    }
    
    /* Allocate file location table (of xdrSetPos's), the uid table, and
       the category table pointers */
    hsfile->locTblFile = (int *) malloc(numOfItems * sizeof(int));
    hsfile->uidTable = (int *) malloc(numOfItems * sizeof(int));
    hsfile->categoryTable = (char **) malloc(numOfItems * sizeof(char *));
    for (i = 0; i < numOfItems; ++i)
    	hsfile->categoryTable[i] = NULL;
    /* hsfile->locTblMem = locTbl;	uncomment if hsfile kept upon return */
     
    /* write a dummy locTblFile; it will be overwritten at a later stage */
    locOfTblFile = xdr_getpos(hsfile->xdrs);
    unumt = hsfile->numOfItems;
    memset(hsfile->locTblFile, 0, numOfItems * sizeof(int));   /* zero table */
    if (!xdr_array(hsfile->xdrs, (caddr_t *) &hsfile->locTblFile, &unumt, 
     	   (u_int) hsfile->numOfItems, sizeof(int), xdr_int)) { 
    	remove (hsfile->filename);
    	sprintf(*errorString, 
    	    "Error writing initial location table.  File %s deleted.\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
           
    /* Compose the uid and category tables and write them to the file */
    for (i = 0; i < numOfItems; ++i) {
	hsfile->uidTable[i] = locTbl[i]->uid;
	hsfile->categoryTable[i] = locTbl[i]->category;
	/* don't bother copying categories, just remember to free & zero 
	   hsfile->categoryTable before calling FreeHsFileStruct */
    }
    if (!xdr_array(hsfile->xdrs, (caddr_t *)&hsfile->uidTable, &unumt,
     	   (u_int) hsfile->numOfItems, sizeof(int), xdr_int)) { 
    	remove (hsfile->filename);
    	sprintf(*errorString, 
    	    "Error writing location table.  File %s deleted.\n",
    	    hsfile->filename);
    	free(hsfile->categoryTable);
    	hsfile->categoryTable = NULL;
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    if (!xdr_array(hsfile->xdrs, (char **)&hsfile->categoryTable, &unumt, 
             unumt, sizeof(char *), xdr_wrapstring) ) {
    	sprintf (*errorString,  
    	  "Error - Writing the Category table for file %s.\n  File deleted.\n",
    	   hsfile->filename);
#ifdef COM_DEBUG
    	for (i = 0; i < numOfItems; ++i)
    	    printf("i = %d, category = %s\n", i, hsfile->categoryTable[i]);
#endif /*COM_DEBUG*/
    	free(hsfile->categoryTable);
    	hsfile->categoryTable = NULL;
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    free(hsfile->categoryTable);		/* get rid of category table */
    hsfile->categoryTable = NULL;
    
    /* Write each item, translated by xdr, and save item position in locator
       table */
    for (i = 0; i < numOfItems; ++i) {
	switch (locTbl[i]->type) {
    	    case HS_1D_HISTOGRAM:
    	    	u_type = FILE_1DHIST;
    	    	break;
    	    case HS_2D_HISTOGRAM:
    	    	u_type = FILE_2DHIST;
    	    	break;
    	    case HS_3D_HISTOGRAM:
    	    	u_type = FILE_3DHIST;
    	    	break;
    	    case HS_NTUPLE:
    	    	u_type = FILE_NTUPLE;
    	    	break;
    	    case HS_INDICATOR:
    	    	u_type = FILE_INDICATOR;
    	    	break;
    	    case HS_CONTROL:
    	    	u_type = FILE_CONTROL;
    	    	break;
    	    case HS_GROUP:
    	    	u_type = FILE_GROUP;
    	    	break;
    	    case HS_NFIT:
    	    	u_type = FILE_NFIT;
    	    	break;
    	    case HS_TRIGGER:
    	    default:
    	    	sprintf (*errorString,  
       "Internal Error - Found unsupported item type in WriteHsFile; file: %s\n\
       id: %d, uid: %d, type: %d, title: %s\n", hsfile->filename,
    	    	    locTbl[i]->id, locTbl[i]->uid, locTbl[i]->type, 
    	    	    locTbl[i]->title);
    	    	fprintf (stderr,  
       "Internal Error - Found unsupported item type in WriteHsFile; file: %s\n\
       id: %d, uid: %d, type: %d, title: %s\n", hsfile->filename,
    	    	    locTbl[i]->id, locTbl[i]->uid, locTbl[i]->type, 
    	    	    locTbl[i]->title);
    		FreeHsFileStruct(hsfilePtr);
    		return -1;
    	}
    	/* save position of this item in the file for locator table */
    	hsfile->locTblFile[i] = xdr_getpos(hsfile->xdrs);
    	if (!xdr_Item(hsfile->xdrs, &locTbl[i], &u_type)) {
    	    sprintf (*errorString,  
    	        "Error - Writing item %d in file %s; file deleted.\n",
    	        locTbl[i]->id, hsfile->filename);
    	    remove (hsfile->filename);
    	    FreeHsFileStruct(hsfilePtr);
    	    return -1;
        }

#ifdef COM_DEBUG
    	printf (" Wrote item number %d, uid: %d at position %d \n", 
    	    locTbl[i]->id, locTbl[i]->uid, hsfile->locTblFile[i]);
#endif /* COM_DEBUG */    	 

    }	/* end of for loop */
    
    /* Now write the real locator table at file position locOfTblFile */ 
    if (!xdr_setpos(hsfile->xdrs, locOfTblFile)) { 
    	remove (hsfile->filename);
    	sprintf(*errorString, 
    	    "Error positioning file to locator table.  File %s deleted.\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    if (!xdr_array(hsfile->xdrs, (caddr_t *) &hsfile->locTblFile, &unumt, 
     	   (u_int) hsfile->numOfItems, sizeof(int), xdr_int)) { 
    	remove (hsfile->filename);
    	sprintf(*errorString, 
    	    "Error writing location table.  File %s deleted.\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    	
   /*** Done !! ****/     
    FreeHsFileStruct(hsfilePtr);
    	/* hsfile structure could be retained
    	   for later reading if take care of .xdrEnOrDeCode, .categoryTable,
    	   and .locTblMem.  See code above. */
    if ((*errorString)[0] == '\0') {
    	free(*errorString);
    	*errorString = NULL;			/* no warning message */
    }
    return numOfItems;
}

/*
 *   ReadHsItemData - Read one item's data from a Histo-Scope format file
 *
 *	hsfilePtr   - address of pointer to hsFile structure as returned by
 *		      OpenHsFile.  The hs file must have been opened for
 *		      read (HS_READ) and used with DirHsFile.  
 *
 *		      If an error occurs the hsfile structure is freed 
 *		      and hsfilePtr is set to NULL.  
 *
 *		      Upon success, the data for the item stored in 
 *		      hsfile.locTblMem[index] is read.
 *
 *	index       - The index into the hsfile tables (i.e. .locTblFile, 
 *		      .categoryTable, .uidTable) of the item to read data.
 *		      This can be regarded as the hs id of the item in the file.
 *
 *	errorString - A string will be created if there is an error/warning
 *		      message to print.  Otherwise errorString will be
 *		      set to NULL.  Free errorString after informing user of
 *		      the error or warning.
 *
 *	Returns     - -1 if error; 0 for success.
 */
int ReadHsItemData (hsFile **hsfilePtr, int index, char **errorString)
{
    hsFile *hsfile = *hsfilePtr;
    hsGeneral *item;
    enum utype u_type;

    /* Allocate errorString, to be used if error */
    *errorString = (char *) malloc(MAX_ERROR_STRING_LEN);
    (*errorString)[0] = '\0';	    /* flags whether warning msg returned */

    /* Ensure index is within tables */
    if (index < 0 || index >= hsfile->numOfItems) {
    	fprintf(stderr, "Internal error calling ReadHsFile for %s\n\
    	    index is out of range (%d).", hsfile->filename, index);
    	sprintf(*errorString, "Internal error calling ReadHsItem for %s\n\
    	    index is out of range (%d).", hsfile->filename, index);
    	return -1;
    }
    
/*#ifdef COM_DEBUG*/
    /* Ensure hsfile structure was successfully opened for read */
    if (hsfile->xdrEnOrDeCode != XDR_DECODE || hsfile->xdrs == NULL || 
    	    hsfile->filename == NULL || hsfile->filePtr == NULL) {
    	fprintf(stderr, "Internal error calling ReadHsItem for %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling ReadHsItem for %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
    
    if (hsfile->fileversion < HS_FILE_C || (hsfile->fileversion >= HS_FILE_C &&  
    	    (hsfile->numOfItems <= 0 || hsfile->locTblFile == NULL || 
    	    hsfile->uidTable == NULL || hsfile->categoryTable == NULL))) {
    	fprintf(stderr, "Internal error calling ReadHsItem, file: %s\n",
    	    hsfile->filename);
    	sprintf(*errorString, "Internal error calling ReadHsItem, file: %s\n",
    	    hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -1;
    }
/*#endif COM_DEBUG*/
   
    /* Set file version for xdr_Item  & position file */
    xdr_SetFileVer(hsfile->fileversion);
    if (xdr_setpos(hsfile->xdrs, hsfile->locTblFile[index])) {
        /* Read the item's data */
    	item = (hsfile->locTblMem)[index];
    	if (item == NULL || item->id <= 0) {
    	    sprintf(*errorString, "Error reading item's data in file %s. \n\
    	         Item pointer is null or id is <= 0\n", hsfile->filename);
    	    return -1;
    	}
    	if (!xdr_ItemData(hsfile->xdrs,&item,&u_type,hsfile->dataPos[index])) {
    	    sprintf(*errorString, "Error reading item's data in file %s. \n", 
    	       hsfile->filename);
    	    FreeHsFileStruct(hsfilePtr);
    	    return -1;
    	}
    	
#ifdef COM_DEBUG
    	printf ("Read data for item number %d, uid %d, %s\n", index, 
    		item->uid, item->title);
#endif  /* COM_DEBUG */

    	free(*errorString);
    	*errorString = NULL;			  /* no warning message */
    	return 0;				  /* success return */
    }
    else { 					  /* error in xdr_setpos */
    	sprintf(*errorString, "Error retrieving item's data from file %s. \n", 
    	       hsfile->filename);
    	FreeHsFileStruct(hsfilePtr);
    	return -3;
    }
}

/*
 *   closeFile - If the number of items read > 0, closeFile just closes the
 *		 file stream and destroys the xdr stream.  Otherwise,
 *		 FreeHsFileStruct is called, which sets hsfilePtr to NULL.
 */
static void closeFile(hsFile **hsfilePtr, int numRead)
{
    hsFile *hsfile = *hsfilePtr;

    if (numRead > 0) {
        if (hsfile->xdrs != NULL) {
            xdr_destroy(hsfile->xdrs);
            free(hsfile->xdrs);
            hsfile->xdrs = NULL;
	}
	if (hsfile->filePtr != NULL)
    	    fclose(hsfile->filePtr);
        hsfile->numOfItems = numRead;
    }
    else
        FreeHsFileStruct(hsfilePtr);
}

/*
 *   FreeHsFileStruct - Free all the parts of the hsFile structure.  Also
 *			closes a file (if open) and destroys the xdr stream.
 *			(However, if this routine is called because of an 
 *			error writing to a file, it should be called AFTER 
 *			removing the filename.)
 *
 *			**Note** Items pointed to by hsfile->locTblMem are
 *			NOT freed, even though the locTblMem table is freed.
 *
 *			hsfilePtr is set to NULL.
 */
void FreeHsFileStruct(hsFile **hsfilePtr)
{
    hsFile *hsfile = *hsfilePtr;
    int i;
    
    if (hsfile->dataPos != NULL)
        free(hsfile->dataPos);
    if (hsfile->locTblMem != NULL)
        free(hsfile->locTblMem);	/* don't free items? ? ! ? */
    if (hsfile->categoryTable != NULL && hsfile->numOfItems > 0) {
    	for (i = 0; i < hsfile->numOfItems; ++i)
            if (hsfile->categoryTable[i] != NULL)
                free(hsfile->categoryTable[i]);
    	free(hsfile->categoryTable);
    }
    if (hsfile->uidTable != NULL)
        free(hsfile->uidTable);
    if (hsfile->locTblFile != NULL)
        free(hsfile->locTblFile);
    if (hsfile->xdrs != NULL) {
        xdr_destroy(hsfile->xdrs);
        free(hsfile->xdrs);
        hsfile->xdrs = NULL;
    }
    if (hsfile->filePtr != NULL)
    	fclose(hsfile->filePtr);
    if (hsfile->filename != NULL)
        free(hsfile->filename);
    
    free(hsfile);
    *hsfilePtr = NULL;
}
