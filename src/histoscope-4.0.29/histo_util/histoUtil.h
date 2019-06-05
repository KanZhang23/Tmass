/*******************************************************************************
*									       *
* histoUtil.h -- Utility routines include file for the Nirvana Histoscope tool *
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
* June 10, 1992								       *
*									       *
* Written by Joy Kyriakopulos and Mark Edel				       *
*									       *
* Modified by Joy Kyriakopulos to add Control item type	8/24/93		       *
*			       to relocate xdr routines to xdrHisto.h 9/1/93   *
*									       *
*******************************************************************************/

/*
 *  HistoLists Definition (linked list structure for keeping track of Histo
 *  				items)
 */
typedef struct _histoListHeader {
    struct _histoListHeader *next;
    hsGeneral *item;
} histoListHeader;

/*
 *  Function Prototypes
 */
 
/********************** Routines for HistoLists *******************************/

void         AddItemToList (hsGeneral *item, histoListHeader **histoList);
hsGeneral   *GetItemByID (int id, histoListHeader *histoList); 
void         DeleteItemFromList (int id, histoListHeader **histoList);
hsGeneral   *DeleteItemFromListND (int id, histoListHeader **histoList);
void         FreeEntireList(histoListHeader **histoList);
void         FreeEntireListND(histoListHeader **histoList);

/********************** Routines for Histo items ******************************/

void	     CalcNTVarRange(hsNTuple *ntuple, int var, float *minReturn,
			    float *maxReturn);
float	     NTRef(hsNTuple *ntuple, int var, int index);
void         FreeItem (hsGeneral *item);
void         ResetNtuple (hsNTuple *hsNT);

hsGeneral   *CopyItem (hsGeneral *item);
hs1DHist    *Copy1DHist (hs1DHist *item);
hs2DHist    *Copy2DHist (hs2DHist *item);
hs3DHist    *Copy3DHist (hs3DHist *item);
hsNTuple    *CopyNTuple (hsNTuple *item);
hsIndicator *CopyIndicator (hsIndicator *item);
hsControl   *CopyControl (hsControl *item);
hsTrigger   *CopyTrigger (hsTrigger *item);
hsNFit	    *CopyNfit(hsNFit *item);
hsGroup     *CopyGroup(hsGroup *item);

char        *CopyString(const char *string);

int         ItemHasData(hsGeneral *item);

