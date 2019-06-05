/*******************************************************************************
*									       *
* xdrHisto.h -- XDR Utility routines include file for Nirvana Histoscope tool  *
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
* Originally part of HistoUtil.c and written June 10, 1992		       *
* Extracted from HistoUtil.c September 1, 1993 by JMK			       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modified by Joy Kyriakopulos to add Control item type	8/24/93		       *
* 	   by Joy Kyriakopulos to add histogram error flag 9/16/93	       *
*									       *
*******************************************************************************/


typedef struct _hsCNTuple {
    hsNTuple nTuple;
    int      fromTuple;
    int      toTuple;	    /* used in xdr_Item, how much of ntuple to write  */
    int      inhibitResetRefresh;
    unsigned needsCompleteUpdate;
} hsCNTuple;

typedef struct _hsC1DHist {
    hs1DHist  hist;
    int       sendErrs;	     /* flag: send error data with bin data?	     */
    			     /*       used in xdr_Item			     */
    int       resetFlag;     /* flag: used in HistoClient: needToUpdate	     */
} hsC1DHist;

typedef struct _hsC2DHist {
    hs2DHist  hist;
    int       sendErrs;	     /* flag: send error data with bin data?	     */
    			     /*       used in xdr_Item			     */
    int       resetFlag;     /* flag: used in HistoClient: needToUpdate	     */
} hsC2DHist;

typedef struct _hsC3DHist {
    hs3DHist  hist;
    int       sendErrs;	     /* flag: send error data with bin data?	     */
    			     /*       used in xdr_Item			     */
    int       resetFlag;     /* flag: used in HistoClient: needToUpdate	     */
} hsC3DHist;

#ifdef XDR_FILTERS_HERE		/* xdr filter routines for histo items */

enum utype {FILE_INDICATOR,     FILE_1DHIST,     FILE_2DHIST,     FILE_NTUPLE,
	    DATA_INDICATOR,     DATA_1DHIST,     DATA_2DHIST,     DATA_NTUPLE,
	    ITEMLIST_INDICATOR, ITEMLIST_1DHIST, ITEMLIST_2DHIST, ITEMLIST_NTUPLE,
	    FILE_CONTROL,    DATA_CONTROL, ITEMLIST_CONTROL, ITEMLIST_TRIGGER,
	    FILE_NFIT, FILE_GROUP, DATA_GROUP, ITEMLIST_GROUP, DATA_CONFIG,
            FILE_3DHIST, DATA_3DHIST, ITEMLIST_3DHIST, N_U_TYPES
};

#ifdef COM_DEBUG
static char *UtypeCodes[N_U_TYPES] = {"FILE_INDICATOR", "FILE_1DHIST", "FILE_2DHIST",
	     "FILE_NTUPLE", "DATA_INDICATOR",
	      "DATA_1DHIST", "DATA_2DHIST", "DATA_NTUPLE",
		"ITEMLIST_INDICATOR", "ITEMLIST_1DHIST", "ITEMLIST_2DHIST",
		"ITEMLIST_NTUPLE",
	      "FILE_CONTROL",    "DATA_CONTROL", "ITEMLIST_CONTROL",
	      "ITEMLIST_TRIGGER", "FILE_NFIT", "FILE_GROUP", "DATA_GROUP", 
	      "ITEMLIST_GROUP", "DATA_CONFIG",
	      "FILE_3DHIST", "DATA_3DHIST", "ITEMLIST_3DHIST"
};
#endif

bool_t xdr_Item(XDR *xdrs, hsGeneral **hsG, enum utype *u_type);
bool_t xdr_ItemDir(XDR *xdrs, hsGeneral **hsG, enum utype *u_type);
bool_t xdr_ItemData(XDR *xdrs, hsGeneral **hsG, enum utype *u_type, int loc);
void xdr_SetFileVer(int fileVer);
void xdr_SetClientVer(int clientVer);
void xdr_DontReadNfitInfo(int trueOrFalse);

#endif	/* XDR_FILTERS_HERE */
