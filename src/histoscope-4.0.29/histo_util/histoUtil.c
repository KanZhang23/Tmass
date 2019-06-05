/*******************************************************************************
*									       *
* histoUtil.c -- Utility routines for the Nirvana Histoscope tool	       *
*									       *
* Copyright (c) 1992, 1993 Universities Research Association, Inc.	       *
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
* April 20, 1992							       *
*									       *
* Written by Mark Edel and Joy Kyriakopulos				       *
*									       *
* Modified by Joy Kyriakopulos 9/1/93: relocated xdr routines to xdrHisto.c    *
*	so that NPlot on VMS would not require Multinet to run		       *
*									       *
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include "hsTypes.h"
#include "histoUtil.h"
#include "xdrHisto.h"

/***** Function Prototypes for static routines *****/
static void freeIfNotNull(void *ptr);

/***** Routines for HistoLists (linked list structures pointing to items) *****/

/*
** AddItemToList - Add a histogram, nTuple, indicator, or control to a histolist
**
** 	example call: AddItemToList(item, &HistoList);
**
**	note that the list is passed by address so that the list header
**	value can be changed (the new link is added to the beginning).
*/
void AddItemToList(hsGeneral *item, histoListHeader **histoList)
{
    histoListHeader *temp;

    temp = *histoList;
    *histoList = (histoListHeader *)malloc(sizeof(histoListHeader));
    (*histoList)->next = temp;
    (*histoList)->item = item;
}

/*
**   GetItemByID - retrieve pointer to Histo item from specified histolist by
**		   supplying its id.
*/
hsGeneral *GetItemByID(int id, histoListHeader *histoList)
{
    histoListHeader *h;
    
    for (h = histoList; h != NULL; h = h->next)
	if (h->item->id == id)
	    return h->item;
    /* Item not found */
    return NULL;
}

/*
**   DeleteItemFromList - Delete a histogram, nTuple, indicator or control from
**			  a histolist AND delete the item
**
** 	example call: DeleteItemFromList(id, &HistoList);
**
**	note that the list is passed by address in case the item to be deleted
**	is the first item in the list (and the header value needs to change).
*/
void DeleteItemFromList(int id, histoListHeader **histoList)
{
    histoListHeader *temp, *toFree = NULL;

    if (*histoList == NULL) {
    	fprintf(stderr, "DeleteItemFromList - Error: list is empty.\n");
    	return;
    }
    if ((*histoList)->item->id == id) {
	toFree = *histoList;
	*histoList = toFree->next;
    } else {
	for (temp = *histoList; temp != NULL; temp = temp->next) {
	    if (temp->next == NULL)
	    	break;
	    if (temp->next->item->id == id) {
	        toFree = temp->next;
		temp->next = toFree->next;
		break;
	    }
	}
    }
    if (toFree != NULL) {
    	FreeItem(toFree->item);
    	free(toFree);
    }
    else
    	fprintf(stderr, "DeleteItemFromList - Error: item not found. id = %d\n",
    		id);
}

/*
**   DeleteItemFromListND - Delete a histogram, nTuple, indicator, or control 
**			    from a histolist WITHOUT deleting the item.
**
** 	example call:  item_ptr = DeleteItemFromListND(id, &HistoList);
**
**	note that the list is passed by address in case the item to be deleted
**	is the first item in the list (& the listheader value needs to change).
*/
hsGeneral *DeleteItemFromListND(int id, histoListHeader **histoList)
{
    histoListHeader *temp, *toFree = NULL;
    hsGeneral *item_ptr = NULL;

    if (*histoList == NULL) {
    	fprintf(stderr, "DeleteItemFromListND - Error: list is empty.\n");
    	return NULL;
    }
    if ((*histoList)->item->id == id) {
	toFree = *histoList;
	*histoList = toFree->next;
    } else {
	for (temp = *histoList; temp != NULL; temp = temp->next) {
	    if (temp->next == NULL)
	    	break;
	    if (temp->next->item->id == id) {
	        toFree = temp->next;
		temp->next = toFree->next;
		break;
	    }
	}
    }
    if (toFree != NULL) {
    	item_ptr = toFree->item;
    	free(toFree);
    }
    else
    	fprintf(stderr,"DeleteItemFromListND - Error: item not found. id = %d\n"
    		, id);
    return item_ptr;
}

/*
**   FreeEntireListND - Free an entire histolist WITHOUT deleting the items
**			in it.
**
** 	example call:  FreeEntireListND(&HistoList);
**
**	note that the list is passed by address because the listheader will
**	be set to null.
*/
void FreeEntireListND(histoListHeader **histoList)
{
    histoListHeader *temp, *toFree;

    toFree = *histoList;
    *histoList = NULL;
    while (toFree != NULL) {
	temp = toFree->next;
	free(toFree);
	toFree = temp;
    }
}

/*
**   FreeEntireList - Free an entire histolist and the items in it.
**
** 	example call:  FreeEntireList(&HistoList);
**
**	note that the list is passed by address because the listheader will
**	be set to null.
*/
void FreeEntireList(histoListHeader **histoList)
{
    histoListHeader *temp, *toFree;

    toFree = *histoList;
    *histoList = NULL;
    while (toFree != NULL) {
	temp = toFree->next;
	FreeItem(toFree->item);
	free(toFree);
	toFree = temp;
    }
}

/********************** Routines for Histo items *****************************/

/*
**   FreeItem - routine for freeing Histo items
*/

void FreeItem(hsGeneral *item)
{
    int i;

    if (item == NULL)
    	return;
    freeIfNotNull(item->title);
    freeIfNotNull(item->category);
    switch (item->type) {
      case HS_1D_HISTOGRAM:
      	freeIfNotNull(((hs1DHist *)item)->xLabel);
      	freeIfNotNull(((hs1DHist *)item)->yLabel);
      	freeIfNotNull(((hs1DHist *)item)->bins);
      	freeIfNotNull(((hs1DHist *)item)->pErrs);
      	freeIfNotNull(((hs1DHist *)item)->mErrs);
	break;
      case HS_2D_HISTOGRAM:
      	freeIfNotNull(((hs2DHist *)item)->xLabel);
      	freeIfNotNull(((hs2DHist *)item)->yLabel);
      	freeIfNotNull(((hs2DHist *)item)->zLabel);
      	freeIfNotNull(((hs2DHist *)item)->bins);
      	freeIfNotNull(((hs2DHist *)item)->pErrs);
      	freeIfNotNull(((hs2DHist *)item)->mErrs);
	break;
      case HS_3D_HISTOGRAM:
      	freeIfNotNull(((hs3DHist *)item)->xLabel);
      	freeIfNotNull(((hs3DHist *)item)->yLabel);
      	freeIfNotNull(((hs3DHist *)item)->zLabel);
      	freeIfNotNull(((hs3DHist *)item)->vLabel);
      	freeIfNotNull(((hs3DHist *)item)->bins);
      	freeIfNotNull(((hs3DHist *)item)->pErrs);
      	freeIfNotNull(((hs3DHist *)item)->mErrs);
	break;
      case HS_NTUPLE:
      	if (((hsNTuple *)item)->names != NULL) {
      	    for (i = 0; i < ((hsNTuple *)item)->nVariables; i++)
      	    	freeIfNotNull(((hsNTuple *)item)->names[i]);
      	    free(((hsNTuple *)item)->names);
      	}
      	freeIfNotNull(((hsNTuple *)item)->data);
      	for (i = 0; i < 4; i++)
      	    freeIfNotNull(((hsNTuple *)item)->extensions[i]);
	break;
      case HS_NFIT:
      	freeIfNotNull(((hsNFit *)item)->expr);
      	freeIfNotNull(((hsNFit *)item)->funcPath);
      	freeIfNotNull(((hsNFit *)item)->mParam);
	break;
      case HS_GROUP:
        freeIfNotNull(((hsGroup *)item)->itemId);
        freeIfNotNull(((hsGroup *)item)->errsDisp);
	break;
      default:
        /* Controls, indicators, triggers. */
        break;
    }
    free(item);
}

/*
** freeIfNotNull
*/
static void freeIfNotNull(void *ptr)
{
    if (ptr != NULL)
    	free(ptr);
}

/*
** Reference an element of an ntuple
*/
float NTRef(hsNTuple *ntuple, int var, int index)
{
    register int dataSz = ntuple->chunkSzData;
    register int extSz = ntuple->chunkSzExt;
    
    if (index < dataSz)
    	return ntuple->data[ntuple->nVariables*index + var];
    else if (index < dataSz + extSz)
    	return ntuple->extensions[0][ntuple->nVariables*(index-dataSz) + var];
    else if (index < dataSz + 2*extSz)
    	return ntuple->extensions[1]
    		[ntuple->nVariables*(index-dataSz-extSz) + var];
    else if (index < dataSz + 3*extSz)
    	return ntuple->extensions[2]
    		[ntuple->nVariables*(index-dataSz-2*extSz) + var];
    else if (index < dataSz + 4*extSz)
    	return ntuple->extensions[3]
    		[ntuple->nVariables*(index-dataSz-3*extSz) + var];
    fprintf(stderr, "Internal Error: ntuple reference out of bounds %d of %s\n",
    	    index, ntuple->title);
    return 0.;
}

/*
** Calculate the range of an ntuple
*/
void CalcNTVarRange(hsNTuple *ntuple, int var, float *minReturn,
		    float *maxReturn)
{
    int i;
    float value, min = FLT_MAX, max = -FLT_MAX;
    
    for (i=0; i<ntuple->n; i++) {
    	value = NTRef(ntuple, var, i);
    	if (value < min)
    	    min = value;
    	if (value > max)
    	    max = value;
    }
    *minReturn = min; *maxReturn = max;
}

/*
** ResetNtuple - resets an n-tuple by freeing all data and setting the number
**		 of "tuples" to 0.
*/
void ResetNtuple(hsNTuple *hsNT)
{
    int i;

    freeIfNotNull(hsNT->data);
    hsNT->data = NULL;
    for (i = 0; i < 4; i++) {
      	freeIfNotNull(hsNT->extensions[i]);
      	hsNT->extensions[i] = NULL;
    }
    hsNT->n	      =  0;
    hsNT->chunkSzData =  0;
    hsNT->chunkSzExt  = 10;
}

int ItemHasData(hsGeneral *item)
{
    switch (item->type) {
      case HS_1D_HISTOGRAM:
      	return ((hs1DHist *)item)->bins != NULL;
      case HS_2D_HISTOGRAM:
      	return ((hs2DHist *)item)->bins != NULL;
      case HS_3D_HISTOGRAM:
      	return ((hs3DHist *)item)->bins != NULL;
      case HS_NTUPLE:
      	return ((hsNTuple *)item)->data != NULL
      		|| ((hsNTuple *)item)->extensions[0] != NULL;
      case HS_INDICATOR:
      case HS_CONTROL:
      case HS_TRIGGER:
      case HS_GROUP:
      	return 1;
      default:
      	fprintf(stderr,"Internal Error: ItemHasData called with invalid item.");
      	return 0;
    }
}

/*
** routines for copying Histo items
*/
hsGeneral *CopyItem(hsGeneral *item)
{
    hsGeneral *new = NULL;
    
    switch (item->type) {
      case HS_1D_HISTOGRAM:
        new = (hsGeneral *)Copy1DHist((hs1DHist *)item);
	break;
      case HS_2D_HISTOGRAM:
        new = (hsGeneral *)Copy2DHist((hs2DHist *)item);
	break;
      case HS_3D_HISTOGRAM:
        new = (hsGeneral *)Copy3DHist((hs3DHist *)item);
	break;
      case HS_NTUPLE:
        new = (hsGeneral *)CopyNTuple((hsNTuple *)item);
	break;
      case HS_INDICATOR:
        new = (hsGeneral *)CopyIndicator((hsIndicator *)item);
	break;
      case HS_CONTROL:
        new = (hsGeneral *)CopyControl((hsControl *)item);
	break;
      case HS_TRIGGER:
        new = (hsGeneral *)CopyTrigger((hsTrigger *)item);
	break;
      case HS_NFIT:
        new = (hsGeneral *)CopyNfit((hsNFit *)item);
	break;
      case HS_GROUP:
        new = (hsGeneral *)CopyGroup((hsGroup *)item);
	break;
    }
    return new;
}

hs1DHist *Copy1DHist(hs1DHist *item)
{
    hs1DHist *new;
    
    /* Allocate new memory for the copy */
    new = (hs1DHist *)malloc(sizeof(hsC1DHist));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsC1DHist));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    new->xLabel = CopyString(item->xLabel);
    new->yLabel = CopyString(item->yLabel);
    if (item->bins != NULL) {
    	new->bins = (float *)malloc(sizeof(float) * item->nBins);
    	memcpy((void *)new->bins, (void *)item->bins, sizeof(float) 
    		* item->nBins);
    }
    if (item->pErrs != NULL) {
    	new->pErrs = (float *)malloc(sizeof(float) * item->nBins);
    	memcpy((void *)new->pErrs, (void *)item->pErrs, sizeof(float) 
    		* item->nBins);
    }
    if (item->mErrs != NULL) {
    	new->mErrs = (float *)malloc(sizeof(float) * item->nBins);
    	memcpy((void *)new->mErrs, (void *)item->mErrs, sizeof(float) 
    		* item->nBins);
    }
    return new;
}    

hs2DHist *Copy2DHist(hs2DHist *item)
{
    hs2DHist *new;
    int size;
    
    /* Allocate new memory for the copy */
    new = (hs2DHist *)malloc(sizeof(hsC2DHist));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsC2DHist));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    new->xLabel = CopyString(item->xLabel);
    new->yLabel = CopyString(item->yLabel);
    new->zLabel = CopyString(item->zLabel);
    size = sizeof(float) * item->nXBins * item->nYBins;
    if (item->bins != NULL) {
    	new->bins = (float *)malloc(size);
    	memcpy((void *)new->bins, (void *)item->bins, size);
    }
    if (item->pErrs != NULL) {
    	new->pErrs = (float *)malloc(size);
    	memcpy((void *)new->pErrs, (void *)item->pErrs, size);
    }
    if (item->mErrs != NULL) {
    	new->mErrs = (float *)malloc(size);
    	memcpy((void *)new->mErrs, (void *)item->mErrs, size); 
    }
    return new;
}

hs3DHist *Copy3DHist(hs3DHist *item)
{
    hs3DHist *new;
    int size;
    
    /* Allocate new memory for the copy */
    new = (hs3DHist *)malloc(sizeof(hsC3DHist));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsC3DHist));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    new->xLabel = CopyString(item->xLabel);
    new->yLabel = CopyString(item->yLabel);
    new->zLabel = CopyString(item->zLabel);
    new->vLabel = CopyString(item->vLabel);
    size = sizeof(float) * item->nXBins * item->nYBins * item->nZBins;
    if (item->bins != NULL) {
    	new->bins = (float *)malloc(size);
    	memcpy((void *)new->bins, (void *)item->bins, size);
    }
    if (item->pErrs != NULL) {
    	new->pErrs = (float *)malloc(size);
    	memcpy((void *)new->pErrs, (void *)item->pErrs, size);
    }
    if (item->mErrs != NULL) {
    	new->mErrs = (float *)malloc(size);
    	memcpy((void *)new->mErrs, (void *)item->mErrs, size); 
    }
    return new;
}

hsNTuple *CopyNTuple(hsNTuple *item)
{
    hsNTuple *new;
    int i, size, dataCnt, extCnt;
    char *dataPtr;
    
    /* Allocate new memory for the copy */
    new = (hsNTuple *)malloc(sizeof(hsCNTuple));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsCNTuple));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    if (item->names != NULL) {
    	new->names = (char **)malloc(item->nVariables * sizeof(char *));
    	for (i = 0; i < item->nVariables; i++)
    	    new->names[i] = CopyString(item->names[i]);
    }
    /* Copy the NTuple data */
    size = item->n;
    if (size == 0)
    	return new;
    new->data = (float *) malloc(size * item->nVariables * sizeof(float));
    dataPtr = (char *) new->data;
    if (item->data != NULL) {
    	dataCnt = size < item->chunkSzData ? size : item->chunkSzData;
    	memcpy((void *)new->data, (void *)item->data, item->nVariables *
    		dataCnt * sizeof(float));
    	size -= dataCnt;
    	dataPtr += dataCnt * sizeof(float) * item->nVariables;
    }
    for (i = 0; i < 4; ++i) {
	if (item->extensions[i] != NULL) {
    	    extCnt = size < item->chunkSzExt ? size : item->chunkSzExt;
    	    memcpy((void *)dataPtr, (void *)item->extensions[i], 
    	    	    item->nVariables * extCnt * sizeof(float));
    	    size -= extCnt;
    	    dataPtr += extCnt * sizeof(float) * item->nVariables;
    	    new->extensions[i] = NULL;
	}
	else
	    break;
    }
    new->chunkSzData = new->n;
    return new;
}

hsIndicator *CopyIndicator(hsIndicator *item)
{
    hsIndicator *new;
    
    /* Allocate new memory for the copy */
    new = (hsIndicator *)malloc(sizeof(hsIndicator));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsIndicator));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    return new;
}

hsControl *CopyControl(hsControl *item)
{
    hsControl *new;
    
    /* Allocate new memory for the copy */
    new = (hsControl *)malloc(sizeof(hsControl));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsControl));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    return new;
}

hsTrigger *CopyTrigger(hsTrigger *item)
{
    hsTrigger *new;
    
    /* Allocate new memory for the copy */
    new = (hsTrigger *)malloc(sizeof(hsTrigger));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsTrigger));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    return new;
}

hsGroup *CopyGroup(hsGroup *item)
{
    hsGroup *new;
    
    /* Allocate new memory for the copy */
    new = (hsGroup *)malloc(sizeof(hsGroup));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsGroup));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    new->itemId = (int *) malloc(sizeof(int) * new->numItems);
    new->errsDisp = (int *) malloc(sizeof(int) * new->numItems);
    memcpy((void *)new->itemId, (void *)item->itemId, 
    				sizeof(int) * new->numItems);
    memcpy((void *)new->errsDisp, (void *)item->errsDisp, 
    				sizeof(int) * new->numItems);
    return new;
}

hsNFit *CopyNfit(hsNFit *item)
{
    hsNFit *new;
    
    /* Allocate new memory for the copy */
    new = (hsNFit *)malloc(sizeof(hsNFit));
    /* Fill in the fields, and copy the strings & data from the original */
    memcpy((void *)new, (void *)item, sizeof(hsNFit));
    new->title = CopyString(item->title);
    new->category = CopyString(item->category);
    new->expr = CopyString(item->expr);
    new->funcPath = CopyString(item->funcPath);
    if (item->mParam != NULL)
    	memcpy((void *)new->mParam, item->funcPath, 
    		sizeof(double)*item->nParams);
    return new;
}

char *CopyString(const char *string)
{
    if (string != NULL)
    	return strcpy((char *)malloc(strlen(string)+1), string);
    else
    	return NULL;	
}

