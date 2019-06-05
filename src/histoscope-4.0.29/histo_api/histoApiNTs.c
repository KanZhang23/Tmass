/*******************************************************************************
*									       *
* histoApiNts.c -- Application Interface routines,                             *
* to acces Ntuple properties, such # of variables, rows, colums, so forth      *
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
* January 1994 							               *
*									       *
* Written by Mark Edel, Paul Lebrun and Joy Kyriakopulos                       *
*									       *
*									       *
*******************************************************************************/
/*
* REQUIRED INCLUDE FILES
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#ifndef VXWORKS
#include <sys/errno.h>
#endif /*VXWORKS*/
#include <math.h>
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "HistoClient.h"
#include "histoApiNTs.h"

static char *copyNtupleData(char *dataPtr, hsNTuple *item);

/*
** Functions for accessing NTuple data
**
** hs_num_variables, hs_num_entries, hs_ntuple_value, hs_ntuple_contents,
** hs_row_contents, hs_column_contents
*/

int histo_num_variables(int id)
/*
   Returns the number of variables in the ntuple refered to by id.

   	id		HistoScope id, as returned by create routines.

	Return Value:	The number of variables in the ntuple, or
			-1 if the id is not valid or not an ntuple.
*/
{
    hsNTuple *hnt;
    hsGeneral *h;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_num_variables: Invalid id: %d \n",id);
        return -1;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
            "hs_num_variables: Invalid id: %d, Ntuple does not exist\n", id);
        return -1;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr, 
      	    "hs_num_variables: Item (id = %d) is not an NTuple\n", id);
        return -1;
    }
        hnt = (hsNTuple *) h;
        return hnt->nVariables;
}

int histo_variable_name(int id, int column, char *name, int bindingflag)
/*
   Gives the name of a given variable for a given NTuple refered by id.
   
   	id		HistoScope id, as returned by create routines.
   	column          The variable index  (e.g., a column number)
   			for which the user whishes to know the mnemonic name.
        name 		The mnemonic name of this variable.
        bindingflag     0 for c, else for Fortran binding.
*/
{
    hsNTuple *hnt;
    hsGeneral *h;
    int icolumn = column;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_variable_name: Invalid id: %d\n",id);
        return -1;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
           "hs_variable_name: Invalid id: %d, Ntuple does not exist\n", id);
        return -1;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr, 
            "hs_variable_name: Item (id = %d) is not an NTuple\n", id);
        return -1;
    }
    if (bindingflag != 0) icolumn--;
    hnt = (hsNTuple *) h;
    if ((icolumn < 0) || (icolumn >= hnt->nVariables)) {
        fprintf (stderr,
     "hs_variable_name: Invalid column number: %d, specified for Ntuple: %d \n",
              column, id);
        return -1;
    }
    strcpy(name, hnt->names[icolumn]);  
    return strlen(name);
}

int histo_variable_index(int id, const char *name, int bindingflag)
/*
   Returns the index (e.g. a column number in the NTuple) 
   corresponding to a given variable, refered by its mnemonic
   name, for a given NTuple refered by id. An exact match is required
   between name and one of the variable's names defined in the NTuple.
   If no match is found, returns -1. The routine does not check for 
   multiple matches, it sucessfully completes at the first occurance.
   
   	id		HistoScope id, as returned by create routines.
        name 		The name of the variable.
        bindingflag     0 for c, else for Fortran binding.
         
        Returns the index ( column) corresponding to this variable.
        
*/
{
    hsNTuple *hnt;
    hsGeneral *h;
    int i, ifort;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_variable_index: Invalid id: %d\n",id);
        return -1;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
            "hs_variable_index: Invalid id: %d, Ntuple does not exist\n", id);
        return -1;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr,
            "hs_variable_index: Item (id = %d) is not an NTuple\n", id);
        return -1;
    }
    hnt = (hsNTuple *) h;
    for (i=0; i<hnt->nVariables; i++) {
      if (strcmp(name, hnt->names[i]) == 0 ) {
        if (bindingflag == 0) return i;
        else {
         ifort = i+1;
         return ifort;
         }
        }
    }
    return -1;
}

float histo_ntuple_value(int id, int row, int column, int bindingflag)
/*
   Returns a value from an n-tuple, referenced by a row and column index.
   
   	id		HistoScope id, as returned by create routines.
   	row		Row (entry) number
   	column		Column (element or variable) index
        bindingflag     0 for c, else for Fortran binding.
   	
   	Return Value:	Value read from the n-tuple.  Undefined if id does
   			not exist, or refers to an item other than an n-tuple.
*/
{
    hsNTuple *hnt;
    hsGeneral *h;
    int irow = row, icolumn = column;
    int quot, rem;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_ntuple_value: Invalid id: %d\n",id);
        return -1.;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
            "hs_ntuple_value: Invalid id: %d, Ntuple does not exist\n", id);
        return -1.;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr,
            "hs_ntuple_value: Item (id = %d) is not an NTuple\n", id);
        return -1.;
    }
    
    if (bindingflag != 0) {
        irow--;
        icolumn--;
        }
        
     hnt = (hsNTuple *) h;
     if ((irow < 0) || (irow >= hnt->n)) {
        fprintf (stderr,
         "hs_ntuple_value: Invalid row number: %d, specified for Ntuple: %d \n",
           row, id);
        return -1.;
     }
     if ((icolumn < 0) || (icolumn >= hnt->nVariables)) {
         fprintf (stderr,
      "hs_ntuple_value: invalid column number: %d, specified for Ntuple: %d \n",
           column, id);
         return -1.;
     }
     if ((hnt->data != NULL) && ( irow < hnt->chunkSzData )) 
         return (hnt->data[irow * hnt->nVariables + icolumn]);
     else {  
         quot = (irow - hnt->chunkSzData) / hnt->chunkSzExt;
         rem = (irow - hnt->chunkSzData) % hnt->chunkSzExt;
         return (hnt->extensions[quot][rem * hnt->nVariables + icolumn]);
    }
}

void histo_ntuple_contents(int id, float *data, int bindingflag)
/*
   Returns all of the data in an n-tuple in the form of a two dimensional
   array of floating point values.  The array must be dimensioned to the
   number of rows (entries) by the number of columns (elements or variables)
   in the n-tuple.
   
   	id		HistoScope id, as returned by create routines.
   	data		A two dimensional array dimensioned to the number
   			of rows by the number of columns in the n-tuple.
        bindingflag     0 for c, else for Fortran binding.
*/
{
    hsNTuple *hnt;
    hsGeneral *h;
    int quot, rem, i, j, k, jt;
    float *tdh;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_ntuple_contents: Invalid id: %d\n",id);
        return;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
            "hs_ntuple_contents: Invalid id: %d, Ntuple does not exist\n", id);
        return;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr, 
            "hs_ntuple_contents: Item (id = %d) is not an NTuple\n", id);
        return;
    }
    
    hnt = (hsNTuple *) h;
    quot = (hnt->n - hnt->chunkSzData) / hnt->chunkSzExt;
    rem = (hnt->n - hnt->chunkSzData) % hnt->chunkSzExt;
    tdh = data;
    if (bindingflag == 0) {  /* c Binding, no inversion */
      if (hnt->data != NULL) {
        for (j=0; j<hnt->chunkSzData; j++) {
           for(i=0; i<hnt->nVariables; i++, tdh++)
             *tdh = hnt->data[j * hnt->nVariables + i];
          }
      }
      for (k=0; k <= quot; k++){
        if (k < quot) jt = hnt->chunkSzExt;
          else jt = rem;
        for (j=0; j<jt; j++) {
           for(i=0; i<hnt->nVariables; i++, tdh++)
             *tdh = hnt->extensions[k][j * hnt->nVariables + i];
             }
          }
        } else {/* Fortran Binding, big inversion */
          for(j=0; j< hnt->n; j++) {
            if (j < hnt->chunkSzData ) {
             for(i=0; i<hnt->nVariables; i++) 
               data[i*hnt->n +j] = hnt->data[j * hnt->nVariables +i];
             } else {
               quot = (j - hnt->chunkSzData) / hnt->chunkSzExt;
               rem = (j - hnt->chunkSzData) % hnt->chunkSzExt;
               for(i=0; i<hnt->nVariables; i++) 
                 data[i*hnt->n +j] = 
                      hnt->extensions[quot][rem * hnt->nVariables + i];
              }
           }
      } 
    return;
}

void histo_row_contents(int id, int row, float *data, int bindingflag)
/*
   Returns the contents of a specified row (entry) of n-tuple data.  A row
   is the collection of values in a particular n-tuple entry, one value for
   each variable (A variable could also be called a column or an element).

	id		HistoScope id, as returned by create routines.
	row		Number of the row to extract.
	data		One dimensional array to receive the data.  Must
			be dimensioned to the number of variables (elements)
			in the n-tuple.
        bindingflag     0 for c, else for Fortran binding.
*/

{
    hsNTuple *hnt;
    hsGeneral *h;
    int quot, rem, i;
    int irow = row;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_row_contents: Invalid id: %d\n",id);
        return;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
           "hs_row_contents: Invalid id: %d, Ntuple does not exist\n", id);
        return;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr,
            "hs_row_contents: Item (id = %d) is not an NTuple\n", id);
        return;
    }
    
    hnt = (hsNTuple *) h;
    if (bindingflag != 0) irow--;
    if ((irow < 0) || (irow >= hnt->n)) {
        fprintf (stderr,
          "hs_row_contents: Invalid row number: %d, specified for Ntuple: %d\n",
              row, id);
        return;
    }
	
    if ((hnt->data != NULL) && ( irow < hnt->chunkSzData )) 
       for(i=0; i<hnt->nVariables; i++)
           data[i] = hnt->data[irow * hnt->nVariables + i];
     else {  
      quot = (irow - hnt->chunkSzData) / hnt->chunkSzExt;
      rem = (irow - hnt->chunkSzData) % hnt->chunkSzExt;
      for(i=0; i<hnt->nVariables; i++)
       data[i] = hnt->extensions[quot][rem * hnt->nVariables + i];
       }
      return;
}

void histo_column_contents(int id, int column, float *data, int bindingflag)
/*
   Returns the contents of a specified column of n-tuple data.  A column,
   which can also be called an element or a variable, is the set of
   values taken by a single n-tuple variable for each entry in the n-tuple.

	id		HistoScope id, as returned by create routines.
	column		The index of the variable to extract.
	data		One dimensional array to receive the data.  Must
			be dimensioned to the number of entries (rows)
			in the n-tuple.
        bindingflag     0 for c, else for Fortran binding.
*/
{
    hsNTuple *hnt;
    hsGeneral *h;
    int quot, rem,  i, it, k;
    int jcolumn = column;
    float *tdh = data;
    
    if (id <= 0) { 
        fprintf (stderr, "hs_column_contents: Invalid id: %d\n",id);
        return;
    }
      
    h = (hsGeneral *) GetItemByPtrID(id);
    if (h == NULL) {
        fprintf (stderr,
            "hs_column_contents: Invalid id: %d, Ntuple does not exist\n", id);
        return;
    }
    if (h->type != HS_NTUPLE) {
        fprintf (stderr,
            "hs_column_contents: Item (id = %d) is not an NTuple\n", id);
        return;
    }
    
    hnt = (hsNTuple *) h;
    if (bindingflag != 0) jcolumn--;
    if ((column < 0) || (jcolumn >= hnt->nVariables)) {
        fprintf (stderr,
       "hs_ntuple_value: Invalid column number: %d, specified for Ntuple: %d\n",
              column, id);
        return;
    }
    if (hnt->data != NULL) 
       for(i=0; i<hnt->chunkSzData; i++, tdh++)
           *tdh = hnt->data[i * hnt->nVariables + jcolumn];
      
    quot = (hnt->n - hnt->chunkSzData) / hnt->chunkSzExt;
    rem = (hnt->n - hnt->chunkSzData) % hnt->chunkSzExt;
    for (k=0; k <= quot; k++){
        if (k < quot) it = hnt->chunkSzExt;
          else it = rem;
        for (i=0; i<it; i++, tdh++) 
          *tdh = hnt->extensions[k][i * hnt->nVariables + jcolumn];
    }
    return;
}

int histo_merge_entries(int uid, const char *title, const char *category,
                        int id1, int id2)
/*
   Creates a new Ntuple consisting of two existing NTuple, refered by id1 
   and id2. These two NTuples must have identical number of variables.
    
	uid		User Identification for the newly created NTuple
	title, Category	It's title and category. See hs_create* routines. 
   
   	id1, id2	HistoScope ids of the input Ntuples. 
    
    	Returns the HistoScope id the newly created Ntuple, or -1 if 
    	the items refered by id1, id2 don't exist, are not NTuple or 
    	have a different number of variables. The variable names are taken
    	from id1, and are assumed to be identical to those of id2. 
*/
{
    hsNTuple *hnt1, *hnt2, *hnt3;
    hsGeneral *h1, *h2;
    int  idr;
    float *tdh;
    
    if ((id1 <= 0) || (id2 <= 0)) { 
      fprintf (stderr, "hs_merge_entries: Invalid id: %d\n",
             id1 <= 0 ? id1 : id2);
      return -1;
    }
      
    h1 = (hsGeneral *) GetItemByPtrID(id1);
    h2 = (hsGeneral *) GetItemByPtrID(id2);
    if ((h1 == NULL) || (h2 == NULL)) {
        fprintf (stderr,
            "hs_merge_entries: Invalid id: %d, Ntuple does not exist\n",
             h1 == NULL ? id1 : id2);
        return -1;
    }
    if (h1->type != HS_NTUPLE) {
        fprintf (stderr,
            "hs_merge_entries: Item (id = %d) is not an Ntuple\n", id1);
        return -1;
    }
    if (h2->type != HS_NTUPLE) {
        fprintf (stderr,
            "hs_merge_entries: Item (id = %d) is not an NTuple\n", id2);
        return -1;
    }
    
    hnt1 = (hsNTuple *) h1;
    hnt2 = (hsNTuple *) h2;
    if (hnt1->nVariables != hnt2->nVariables) {
        fprintf (stderr, 
 "hs_merge_entries: Ntuples (id's: %d & %d) have different number of entries\n",
            id1, id2);
        return -1;
    }
    
    idr = histo_create_ntuple(uid, title, category, hnt1->nVariables, 
                             hnt1->names);
    if (idr <= 0 ) {
        fprintf (stderr,
      "hs_merge_entries: Cannot create Ntuple result, uid: %d, category: %s \n", 
           uid, category);
        return -1;
    }
    
    tdh = (float *) malloc((hnt1->nVariables)*sizeof(float)*(hnt1->n+hnt2->n));
    if (tdh == NULL) {
    	fprintf (stderr, 
"hs_merge_entries: Ran out of memory.  Ntuple entries not merged (id's: %d, %d)\n",
            id1, id2);
        histo_delete(idr);
        return -1;
    }
    copyNtupleData(copyNtupleData((char *) tdh, hnt1), hnt2);
    hnt3 = (hsNTuple *) GetItemByPtrID(idr);
    hnt3->data = tdh;
    hnt3->n = hnt1->n + hnt2->n;
    hnt3->chunkSzData =  hnt1->n + hnt2->n;
    return idr;
}

/* 
 *  Copy Ntuple data into a memory area pointed to by dataPtr
 *
 *          - Assumes dataPtr was allocated large enough to hold data
 *          - Returns new dataPtr address (for subsequent calls)
 */ 
static char *copyNtupleData(char *dataPtr, hsNTuple *item)
{
    int i, dataCnt, extCnt, size = item->n;
    
    if (size == 0)
    	return dataPtr;
    if (item->data != NULL) {
    	dataCnt = size < item->chunkSzData ? size : item->chunkSzData;
    	memcpy((void *)dataPtr, (void *)item->data, item->nVariables *
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
	}
	else
	    break;
    }
    return dataPtr;
}      
