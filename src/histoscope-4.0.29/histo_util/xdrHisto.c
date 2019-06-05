/*******************************************************************************
*									       *
* xdrHisto.c -- XDR Utility routines for the Nirvana Histoscope tool	       *
*									       *
* Copyright (c) 1993 Universities Research Association, Inc.		       *
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
* Originally part of HistoUtil.c and written April 20, 1992		       *
* Extracted from HistoUtil.c September 1, 1993 by JMK			       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modified by Joy Kyriakopulos 9/1/93 to add translation of histogram errors   *
*									       *
* Modified by P. Lebrun to support V3 files				       *
*									       *
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#ifdef VMS	/* ask rpc/types to refrain from declaring malloc */
#include "../util/VMSparam.h"
#define DONT_DECLARE_MALLOC
#else
#ifndef VXWORKS
#include <sys/param.h>
#endif /*VXWORKS*/  
#endif /*VMS*/  
#include <rpc/types.h>
#include <sys/types.h>
#include <rpc/xdr.h>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#define XDR_FILTERS_HERE 1
#include "hsTypes.h"
#include "hsFile.h"
#include "nfitParam.h"
#include "xdrHisto.h"
#include "histprotocol.h"

/*  #define COM_DEBUG */

#define HSFILE 1
#define INTERACTIVE 2
static int FileOrInteractive = HSFILE;
static int FileVersion = 1;
static int ClientVersion = 0;
static int LocDataPos = 0;
static int DontReadNfitRec = 0;	/* False */


/***** Function Prototypes for static routines *****/
static bool_t xdr_f_indicator(XDR *xdrs, hsIndicator **hsI);
static bool_t xdr_d_indicator(XDR *xdrs, hsIndicator **hsI);
static bool_t xdr_l_indicator(XDR *xdrs, hsIndicator **hsI);
static bool_t xdr_hsIndicator_allStruct(XDR *xdrs, hsIndicator **hsI);
static bool_t xdr_hsIndicator_shortStructND(XDR *xdrs, hsIndicator **hsI);
static bool_t xdr_hsIndicator_dataOnly(XDR *xdrs, hsIndicator **hsI);
static bool_t xdr_f_control(XDR *xdrs, hsControl **hsI);
static bool_t xdr_d_control(XDR *xdrs, hsControl **hsI);
static bool_t xdr_l_control(XDR *xdrs, hsControl **hsI);
static bool_t xdr_hsControl_allStruct(XDR *xdrs, hsControl **hsI);
static bool_t xdr_hsControl_shortStructND(XDR *xdrs, hsControl **hsI);
static bool_t xdr_hsControl_dataOnly(XDR *xdrs, hsControl **hsI);
static bool_t xdr_l_trigger(XDR *xdrs, hsTrigger **hsT);
static bool_t xdr_hsTrigger_allStruct(XDR *xdrs, hsTrigger **hsT);
static bool_t xdr_l_1dhist(XDR *xdrs, hs1DHist **hsH);
static bool_t xdr_hs1DHist_allStructND(XDR *xdrs, hs1DHist **hs1DH);
static bool_t xdr_l_2dhist(XDR *xdrs, hs2DHist **hsH);
static bool_t xdr_hs2DHist_allStructND(XDR *xdrs, hs2DHist **hs2DH);
static bool_t xdr_l_3dhist(XDR *xdrs, hs3DHist **hsH);
static bool_t xdr_hs3DHist_allStructND(XDR *xdrs, hs3DHist **hs3DH);
static bool_t xdr_d_1dhist(XDR *xdrs, hs1DHist **hsH);
static bool_t xdr_d_2dhist(XDR *xdrs, hs2DHist **hsH);
static bool_t xdr_d_3dhist(XDR *xdrs, hs3DHist **hsH);
static bool_t xdr_f_1dhist(XDR *xdrs, hs1DHist **hsH);
static bool_t xdr_f_2dhist(XDR *xdrs, hs2DHist **hsH);
static bool_t xdr_f_3dhist(XDR *xdrs, hs3DHist **hsH);
static bool_t xdr_hs1DHist_shortStructND(XDR *xdrs, hs1DHist **hs1DH);
static bool_t xdr_hs1DHist_dataOnly(XDR *xdrs, hs1DHist **hs1DH);
static bool_t xdr_hs2DHist_shortStructND(XDR *xdrs, hs2DHist **hs2DH);
static bool_t xdr_hs2DHist_dataOnly(XDR *xdrs, hs2DHist **hs2DH);
static bool_t xdr_hs3DHist_shortStructND(XDR *xdrs, hs3DHist **hs3DH);
static bool_t xdr_hs3DHist_dataOnly(XDR *xdrs, hs3DHist **hs3DH);
static bool_t xdr_l_ntuple(XDR *xdrs, hsNTuple **hsN);
static bool_t xdr_d_ntuple(XDR *xdrs, hsNTuple **hsN);
static bool_t xdr_f_ntuple(XDR *xdrs, hsNTuple **hsN);
static bool_t xdr_hsNTuple_allStructND(XDR *xdrs, hsNTuple **hsN);
static void gatherNtupleData(hsNTuple *nTuple, int oldN);
static bool_t xdr_hsNTuple_shortStructND(XDR *xdrs, hsNTuple **hsN);
static bool_t xdr_hsNTuple_dataOnly(XDR *xdrs, hsNTuple **hsN, int start,
					int end);
static bool_t xdr_hsNTuple_HBdataEncode(XDR *xdrs, hsNTuple **hsN, int from,
		int to);
static bool_t xdr_f_1dhist_dir(XDR *xdrs, hs1DHist **hsH);
static bool_t xdr_f_2dhist_dir(XDR *xdrs, hs2DHist **hsH);
static bool_t xdr_f_3dhist_dir(XDR *xdrs, hs3DHist **hsH);
static bool_t xdr_f_ntuple_dir(XDR *xdrs, hsNTuple **hsN);
static bool_t xdr_f_1dhist_dat(XDR *xdrs, hs1DHist **hsH);
static bool_t xdr_f_2dhist_dat(XDR *xdrs, hs2DHist **hsH);
static bool_t xdr_f_3dhist_dat(XDR *xdrs, hs3DHist **hsH);
static bool_t xdr_f_ntuple_dat(XDR *xdrs, hsNTuple **hsN);
static bool_t xdr_f_group_dat(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_f_nfit(XDR *xdrs, hsNFit **hsN);
static bool_t xdr_f_nfit_ENC(XDR *xdrs, hsNFit **hsN);
static bool_t xdr_minuitParam(XDR *xdrs, minuitParam *hsMp);
static bool_t xdr_f_group(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_f_group_dir(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_d_group(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_l_group(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_hsGroup_allStruct(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_hsGroup_shortStructND(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_group_dataOnly(XDR *xdrs, hsGroup **hsG);
static bool_t xdr_d_config(XDR *xdrs, hsConfigString **hsC);
static bool_t xdr_config_shortStructND(XDR *xdrs, hsConfigString **hsC);
static bool_t xdr_config_dataOnly(XDR *xdrs, hsConfigString **hsC);
static int xdr_ret_error(void);
static int xdr_ret_1(void);
static int version_3_C_Higher(void);
static int version_2_B_Higher(void);

#ifdef VXWORKS
#define MAXPATHLEN FILENAME_MAX
#endif /*VXWORKS*/

/*
** xdr filter routines for histo items *******************************
*/

/*
** xdr_Item - Translate histo item to/from xdr format.
**
**	   Although there are many xdr functions defined in this module,
**	   xdr_Item is the only externally callable function.  The calling
**	   routine merely calls xdr_Item for every type of histo item and
**	   operation.  Consistent with other XDR primitives (e.g. xdr_string),
**         xdr_Item creates the memory for an item if its pointer is NULL when
**         decoding.
**
**         To accomodate different versions of histo files and upwards-compat-
**         ible client-->histo connections, calling either xdr_SetFileVer or
**         xdr_SetClientVer is now necessary before calling xdr_Item.
**
**         Note also, depending on u_type and XDR_ENCODE/XDR_DECODE specified
**         in the xdr_create call, these routines do some fancy footwork
**         based on assumptions of where (client vs. histo-scope) they think
**         they're being called from.  Pertinent to this is the static routine
**         makeCStruct in histoApiFiles.c, which copies an hs<Item> structure
**         to an hsC<Item> structure after calling xdr_Item (on client-side).
**
**         Try praying for guidance before modifying these routines...
**
** Parameters:
**
**	xdrs    - address of the XDR structure used in the stream creation call
**
**	hsG     - address of the pointer to the histo item
**
**	u_type  - pointer to value defining the histo item type and operation
**                the caller wishes to perform.  Constants for this value are
**                defined in histoUtil.h and are of the form op_type, where:
**                op is one of: FILE, DATA, or ITEMLIST, and type is one of:
**                INDICATOR, CONTROL, 1DHIST, 2DHIST, or NTUPLE.  (Also see
**                list of values for struct xdr_discrim u_tag_arms below.)
**
** Example calls:
**
**    1) The following code sequence reads and translates one item from the
**       histo-format file file.hs.  Upon return from the xdr_Item call, the
**       pointer hsG will point to the newly created histo structure, e.g.
**       an indicator.  The value of op_type (obtained from the histo file)
**       will be FILE_INDICATOR.  The memory for the indicator structure
**       should be freed after it is no longer needed (i.e. free(hsG);). 
**
**	   XDR xdrs;
**	   hsGeneral *hsG = NULL;
**	   enum utype op_type;
**	   int fileversion;
**
**         xdrstdio_create(&xdrs, fopen("file.hs", "r"), XDR_DECODE);
**         [...get file version and other stuff from file ...]
**         xdr_SetFileVer(fileversion);
**	   xdr_Item(&xdrs, &hsG, &op_type);
**
**    2) The following code sequence translates (encodes) and writes to an
**       XDR memory stream the parts of a pre-existing indicator necessary
**       to list the item for Histo-Scope.
**
**         void listIndicator(hsIndicator *indicator)
**         {
**	       XDR xdrs;
**	       enum utype op_type = ITEMLIST_INDICATOR;
**             char databuf[2000];
**
**             xdrmem_create(&xdrs, databuf, sizeof(databuf), XDR_ENCODE);
**             xdr_SetClientVer(V4_CLIENT);
**	       xdr_Item(&xdrs, &((hsGeneral *)indicator), &op_type);
**             :
**             :
*/

bool_t xdr_Item(XDR *xdrs, hsGeneral **hsG, enum utype *u_type)
{
    /* routines called by xdr_union based on value of u_type */
    static struct xdr_discrim u_tag_arms[N_U_TYPES] = {
    	{FILE_INDICATOR, xdr_f_indicator},
    	{FILE_1DHIST, xdr_f_1dhist},
    	{FILE_2DHIST, xdr_f_2dhist},
    	{FILE_NTUPLE, xdr_f_ntuple},
    	{DATA_INDICATOR, xdr_d_indicator},
    	{DATA_1DHIST, xdr_d_1dhist},
    	{DATA_2DHIST, xdr_d_2dhist},
    	{DATA_NTUPLE, xdr_d_ntuple},
    	{ITEMLIST_INDICATOR, xdr_l_indicator},
    	{ITEMLIST_1DHIST, xdr_l_1dhist},
    	{ITEMLIST_2DHIST, xdr_l_2dhist},
    	{ITEMLIST_NTUPLE, xdr_l_ntuple},
    	{FILE_CONTROL, xdr_f_control},
    	{DATA_CONTROL, xdr_d_control},
    	{ITEMLIST_CONTROL, xdr_l_control},
    	{ITEMLIST_TRIGGER, xdr_l_trigger}, /* no FILE_TRIGGER or DATA_TRIGGER */
    	{FILE_NFIT, xdr_f_nfit},
    	{FILE_GROUP, xdr_f_group},
    	{DATA_GROUP, xdr_d_group},
    	{ITEMLIST_GROUP, xdr_l_group},
    	{DATA_CONFIG, xdr_d_config},
	{FILE_3DHIST, xdr_f_3dhist},
	{DATA_3DHIST, xdr_d_3dhist},
	{ITEMLIST_3DHIST, xdr_l_3dhist}
    };
    if (xdrs->x_op == XDR_FREE)
    	return 0;			   /* items are freed using FreeItem */
#ifdef VXWORKS
    return (xdr_union(xdrs, (int *)u_type, (caddr_t)hsG, u_tag_arms, NULL));
#else
    return (xdr_union(xdrs, (int *)u_type, hsG, u_tag_arms, NULL));
#endif /*VXWORKS*/
}

void xdr_SetFileVer(int fileVer)
{
    FileVersion = fileVer;
}

void xdr_SetClientVer(int clientVer)
{
    ClientVersion = clientVer;
}

void xdr_DontReadNfitInfo(int trueOrFalse)
{
    DontReadNfitRec = trueOrFalse;
}
/*
 *  xdr_ItemDir - used to get directory listing of histo-scope files
 */
bool_t xdr_ItemDir(XDR *xdrs, hsGeneral **hsG, enum utype *u_type)
{
    /* routines called by xdr_union based on value of u_type */
    static struct xdr_discrim u_tag_arms[N_U_TYPES] = {
    	{FILE_INDICATOR, xdr_f_indicator},    /* same because data in struct */
    	{FILE_1DHIST, xdr_f_1dhist_dir},
    	{FILE_2DHIST, xdr_f_2dhist_dir},
    	{FILE_NTUPLE, xdr_f_ntuple_dir},
    	{DATA_INDICATOR, xdr_ret_error},
    	{DATA_1DHIST, xdr_ret_error},
    	{DATA_2DHIST, xdr_ret_error},
    	{DATA_NTUPLE, xdr_ret_error},
    	{ITEMLIST_INDICATOR, xdr_ret_error},
    	{ITEMLIST_1DHIST, xdr_ret_error},
    	{ITEMLIST_2DHIST, xdr_ret_error},
    	{ITEMLIST_NTUPLE, xdr_ret_error},
    	{FILE_CONTROL, xdr_f_control},	       /* same because data in struct */
    	{DATA_CONTROL, xdr_ret_error},
    	{ITEMLIST_CONTROL, xdr_ret_error},
    	{ITEMLIST_TRIGGER, xdr_ret_error}, /* no FILE_TRIGGER or DATA_TRIGGER */
    	{FILE_NFIT, xdr_f_nfit},
    	{FILE_GROUP, xdr_f_group_dir},
    	{DATA_GROUP, xdr_ret_error},
    	{ITEMLIST_GROUP, xdr_ret_error},
    	{DATA_CONFIG, xdr_ret_error},
    	{FILE_3DHIST, xdr_f_3dhist_dir},
    	{DATA_3DHIST, xdr_ret_error},
    	{ITEMLIST_3DHIST, xdr_ret_error}
    };
    if (xdrs->x_op == XDR_FREE)
    	return 0;			   /* items are freed using FreeItem */
    if (xdrs->x_op == XDR_ENCODE)
    	return 0;			   /* used only for reading from file */
#ifdef VXWORKS
    return (xdr_union(xdrs, (int *)u_type, (caddr_t)hsG, u_tag_arms, NULL));
#else
    return (xdr_union(xdrs, (int *)u_type, hsG, u_tag_arms, NULL));
#endif /*VXWORKS*/
}

bool_t xdr_ItemData(XDR *xdrs, hsGeneral **hsG, enum utype *u_type, int loc)
{
    /* routines called by xdr_union based on value of u_type */
    static struct xdr_discrim u_tag_arms[N_U_TYPES] = {
    	{FILE_INDICATOR, xdr_ret_1},		/* already read */
    	{FILE_1DHIST, xdr_f_1dhist_dat},
    	{FILE_2DHIST, xdr_f_2dhist_dat},
    	{FILE_NTUPLE, xdr_f_ntuple_dat},
    	{DATA_INDICATOR, xdr_ret_error},
    	{DATA_1DHIST, xdr_ret_error},
    	{DATA_2DHIST, xdr_ret_error},
    	{DATA_NTUPLE, xdr_ret_error},
    	{ITEMLIST_INDICATOR, xdr_ret_error},
    	{ITEMLIST_1DHIST, xdr_ret_error},
    	{ITEMLIST_2DHIST, xdr_ret_error},
    	{ITEMLIST_NTUPLE, xdr_ret_error},
    	{FILE_CONTROL, xdr_ret_1},		/* already read */
    	{DATA_CONTROL, xdr_ret_error},
    	{ITEMLIST_CONTROL, xdr_ret_error},
    	{ITEMLIST_TRIGGER, xdr_ret_error},  /* no FILE_TRIGGER or DATA_TRIGGER */
    	{FILE_NFIT, xdr_ret_error},
    	{FILE_GROUP, xdr_f_group_dat},
    	{DATA_GROUP, xdr_ret_error},
    	{ITEMLIST_GROUP, xdr_ret_error},
    	{DATA_CONFIG, xdr_ret_error},
    	{FILE_3DHIST, xdr_f_3dhist_dat},
    	{DATA_3DHIST, xdr_ret_error},
    	{ITEMLIST_3DHIST, xdr_ret_error}
    };
    if (xdrs->x_op == XDR_FREE)
    	return 0;			   /* items are freed using FreeItem */
    if (xdrs->x_op == XDR_ENCODE)
    	return 0;			   /* used only for reading from file */
    LocDataPos = loc;
#ifdef VXWORKS
    return (xdr_union(xdrs, (int *)u_type, (caddr_t)hsG, u_tag_arms, NULL));
#else
    return (xdr_union(xdrs, (int *)u_type, hsG, u_tag_arms, NULL));
#endif /*VXWORKS*/
}

/*** routines called by xdr_union based on value of u_type ***/

/* FILE_INDICATOR */
static bool_t xdr_f_indicator(XDR *xdrs, hsIndicator **hsI)
{
    FileOrInteractive = HSFILE;
    return ( xdr_hsIndicator_allStruct(xdrs, hsI));
}

/* DATA_INDICATOR */
static bool_t xdr_d_indicator(XDR *xdrs, hsIndicator **hsI)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hsIndicator_shortStructND(xdrs, hsI) &&
    	     xdr_hsIndicator_dataOnly(xdrs, hsI) );
}

/* ITEMLIST_INDICATOR */
static bool_t xdr_l_indicator(XDR *xdrs, hsIndicator **hsI)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hsIndicator_allStruct(xdrs, hsI) );
}

static bool_t xdr_hsIndicator_allStruct(XDR *xdrs, hsIndicator **hsI)
{
/*  Translate all of the structure of an Indicator except its data (value) */

    if ( (xdrs->x_op == XDR_DECODE) && (*hsI == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsI = (hsIndicator *) malloc(sizeof(hsIndicator));
	 (*hsI)->title = NULL;			/* for xdr_string below */
	 (*hsI)->category = NULL;		/*  "   "          "    */
    	 (*hsI)->valueSet = VALUE_NOT_SET;	/* no value yet         */
    	 (*hsI)->uid = 0;			/* init for version < 3 */
    }
    return (xdr_int(xdrs, &(*hsI)->type) &&
            xdr_int(xdrs, &(*hsI)->id) &&
            (version_3_C_Higher() ? xdr_int(xdrs, &(*hsI)->uid) : 1) &&
            xdr_string(xdrs, &(*hsI)->title, HS_MAX_TITLE_LENGTH) &&
            xdr_string(xdrs, &(*hsI)->category, HS_MAX_CATEGORY_LENGTH) &&
            xdr_int(xdrs, &(*hsI)->hbookID) &&
            xdr_float(xdrs, &(*hsI)->min) &&
            xdr_float(xdrs, &(*hsI)->max) &&
            xdr_int(xdrs, &(*hsI)->valueSet) &&
            xdr_float(xdrs, &(*hsI)->value) );
}

static bool_t xdr_hsIndicator_shortStructND(XDR *xdrs, hsIndicator **hsI)
{
/*  Translate the part of an Indicator structure that is necessary to identify
    it and any structure members that may have changed, but not its data */

    return (xdr_int(xdrs, &(*hsI)->type) &&
            xdr_int(xdrs, &(*hsI)->id) &&
            xdr_int(xdrs, &(*hsI)->valueSet) );
}

static bool_t xdr_hsIndicator_dataOnly(XDR *xdrs, hsIndicator **hsI)
{
/*  Translate only the data of an Indicator */

    return (xdr_float(xdrs, &(*hsI)->value) );
}

/* FILE_CONTROL */
static bool_t xdr_f_control(XDR *xdrs, hsControl **hsI)
{
    FileOrInteractive = HSFILE;
    return ( xdr_hsControl_allStruct(xdrs, hsI));
}

/* DATA_CONTROL */
static bool_t xdr_d_control(XDR *xdrs, hsControl **hsI)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hsControl_shortStructND(xdrs, hsI) &&
    	     xdr_hsControl_dataOnly(xdrs, hsI) );
}

/* ITEMLIST_CONTROL */
static bool_t xdr_l_control(XDR *xdrs, hsControl **hsI)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hsControl_allStruct(xdrs, hsI) );
}

static bool_t xdr_hsControl_allStruct(XDR *xdrs, hsControl **hsI)
{
/*  Translate all of the structure of a Control except its data (value) */

    if ( (xdrs->x_op == XDR_DECODE) && (*hsI == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsI = (hsControl *) malloc(sizeof(hsControl));
	 (*hsI)->title = NULL;			/* for xdr_string below */
	 (*hsI)->category = NULL;		/*  "   "          "    */
    	 (*hsI)->uid = 0;			/* for version < 3      */
    }
    return (xdr_int(xdrs, &(*hsI)->type) &&
            xdr_int(xdrs, &(*hsI)->id) &&
            (version_3_C_Higher() ? xdr_int(xdrs, &(*hsI)->uid) : 1) &&
            xdr_string(xdrs, &(*hsI)->title, HS_MAX_TITLE_LENGTH) &&
            xdr_string(xdrs, &(*hsI)->category, HS_MAX_CATEGORY_LENGTH) &&
            xdr_int(xdrs, &(*hsI)->hbookID) &&
            xdr_float(xdrs, &(*hsI)->min) &&
            xdr_float(xdrs, &(*hsI)->max) &&
            xdr_float(xdrs, &(*hsI)->value) );
}

static bool_t xdr_hsControl_shortStructND(XDR *xdrs, hsControl **hsI)
{
/*  Translate the part of a Control structure that is necessary to identify
    it and any structure members that may have changed, but not its data */

    return (xdr_int(xdrs, &(*hsI)->type) &&
            xdr_int(xdrs, &(*hsI)->id) );
}

static bool_t xdr_hsControl_dataOnly(XDR *xdrs, hsControl **hsI)
{
/*  Translate only the data of a Control */

    return (xdr_float(xdrs, &(*hsI)->value) );
}

/* ITEMLIST_TRIGGER */
static bool_t xdr_l_trigger(XDR *xdrs, hsTrigger **hsT)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hsTrigger_allStruct(xdrs, hsT) );
}

static bool_t xdr_hsTrigger_allStruct(XDR *xdrs, hsTrigger **hsT)
{
/*  Translate all of the structure of a Trigger */

    if ( (xdrs->x_op == XDR_DECODE) && (*hsT == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsT = (hsTrigger *) malloc(sizeof(hsTrigger));
	 (*hsT)->title = NULL;			/* for xdr_string below */
	 (*hsT)->category = NULL;		/*  "   "          "    */
    	 (*hsT)->uid = 0;			/* for version < 3      */
    }
    return (xdr_int(xdrs, &(*hsT)->type) &&
            xdr_int(xdrs, &(*hsT)->id) &&
            (version_3_C_Higher() ? xdr_int(xdrs, &(*hsT)->uid) : 1) &&
            xdr_string(xdrs, &(*hsT)->title, HS_MAX_TITLE_LENGTH) &&
            xdr_string(xdrs, &(*hsT)->category, HS_MAX_CATEGORY_LENGTH) &&
            xdr_int(xdrs, &(*hsT)->hbookID) );
}

/* ITEMLIST_1DHIST */
static bool_t xdr_l_1dhist(XDR *xdrs, hs1DHist **hsH)
{
    FileOrInteractive = INTERACTIVE;
    if ( xdr_hs1DHist_allStructND(xdrs, hsH) ) {
    	if (xdrs->x_op == XDR_DECODE) {
    	    (*hsH)->count = 0;
    	    (*hsH)->overflow = 0;
    	    (*hsH)->underflow = 0;
    	}
    	return 1;
    }
    else
    	return 0;
}

static bool_t xdr_hs1DHist_allStructND(XDR *xdrs, hs1DHist **hs1DH)
{
/*  Translate all of the structure of a 1d Histogram but not its data */

    if ( (xdrs->x_op == XDR_DECODE) && (*hs1DH == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hs1DH = (hs1DHist *) calloc(1, sizeof(hs1DHist));
    }
    return (xdr_int(xdrs, &(*hs1DH)->type) &&
            xdr_int(xdrs, &(*hs1DH)->id) &&
            (version_3_C_Higher() ? xdr_int(xdrs, &(*hs1DH)->uid) : 1) &&
            xdr_string(xdrs, &(*hs1DH)->title, HS_MAX_TITLE_LENGTH) &&
            xdr_string(xdrs, &(*hs1DH)->category, HS_MAX_CATEGORY_LENGTH) &&
            xdr_int(xdrs, &(*hs1DH)->hbookID) &&
            xdr_int(xdrs, &(*hs1DH)->count) &&
            xdr_int(xdrs, &(*hs1DH)->nBins) &&
            xdr_float(xdrs, &(*hs1DH)->min) &&
            xdr_float(xdrs, &(*hs1DH)->max) &&
            xdr_float(xdrs, &(*hs1DH)->overflow) &&
            xdr_float(xdrs, &(*hs1DH)->underflow) &&
            xdr_int(xdrs, &(*hs1DH)->xScaleType) &&
            xdr_float(xdrs, &(*hs1DH)->xScaleBase) &&
            xdr_string(xdrs, &(*hs1DH)->xLabel, HS_MAX_LABEL_LENGTH) &&
            xdr_string(xdrs, &(*hs1DH)->yLabel, HS_MAX_LABEL_LENGTH) && 
            (version_2_B_Higher() ? xdr_int(xdrs, &(*hs1DH)->errFlg) : 1) );
}

/* ITEMLIST_2DHIST */
static bool_t xdr_l_2dhist(XDR *xdrs, hs2DHist **hsH)
{
    int i, j;

    FileOrInteractive = INTERACTIVE;
    if ( xdr_hs2DHist_allStructND(xdrs, hsH) ) {
    	if (xdrs->x_op == XDR_DECODE) {
    	    (*hsH)->count = 0;
	    for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j)
		    (*hsH)->overflow[i][j] = 0.;
    	}
    	return 1;
    }
    else
        return 0;
}

/* ITEMLIST_3DHIST */
static bool_t xdr_l_3dhist(XDR *xdrs, hs3DHist **hsH)
{
    int i, j, k;

    FileOrInteractive = INTERACTIVE;
    if ( xdr_hs3DHist_allStructND(xdrs, hsH) ) {
    	if (xdrs->x_op == XDR_DECODE) {
    	    (*hsH)->count = 0;
	    for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j)
		    for (k = 0; k < 3; ++k)
			(*hsH)->overflow[i][j][k] = 0.f;
    	}
    	return 1;
    }
    else
        return 0;
}

static bool_t xdr_hs2DHist_allStructND(XDR *xdrs, hs2DHist **hs2DH)
{
/*  Translate all of the structure of a 2d Histogram but not its data */

    unsigned int tmp9 = 9;		/* number of overflow bins (3x3) */
    float *tmpv;

    if ( (xdrs->x_op == XDR_DECODE) && (*hs2DH == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hs2DH = (hs2DHist *) calloc(1, sizeof(hs2DHist));
    }
    tmpv = (float *)(*hs2DH)->overflow;
    return (xdr_int(xdrs, &(*hs2DH)->type) &&
        xdr_int(xdrs, &(*hs2DH)->id) &&
        (version_3_C_Higher() ? xdr_int(xdrs, &(*hs2DH)->uid) : 1) &&
        xdr_string(xdrs, &(*hs2DH)->title, HS_MAX_TITLE_LENGTH) &&
        xdr_string(xdrs, &(*hs2DH)->category, HS_MAX_CATEGORY_LENGTH) &&
        xdr_int(xdrs, &(*hs2DH)->hbookID) &&
        xdr_int(xdrs, &(*hs2DH)->count) &&
        xdr_int(xdrs, &(*hs2DH)->nXBins) &&
        xdr_int(xdrs, &(*hs2DH)->nYBins) &&
        xdr_float(xdrs, &(*hs2DH)->xMin) &&
        xdr_float(xdrs, &(*hs2DH)->xMax) &&
        xdr_float(xdrs, &(*hs2DH)->yMin) &&
        xdr_float(xdrs, &(*hs2DH)->yMax) &&
        xdr_array(xdrs, (char **)&tmpv, &tmp9, tmp9, sizeof(float), xdr_float) &&
        xdr_int(xdrs, &(*hs2DH)->xScaleType) &&
        xdr_float(xdrs, &(*hs2DH)->xScaleBase) &&
        xdr_int(xdrs, &(*hs2DH)->yScaleType) &&
        xdr_float(xdrs, &(*hs2DH)->yScaleBase) &&
        xdr_string(xdrs, &(*hs2DH)->xLabel, HS_MAX_LABEL_LENGTH) &&
        xdr_string(xdrs, &(*hs2DH)->yLabel, HS_MAX_LABEL_LENGTH) &&
        xdr_string(xdrs, &(*hs2DH)->zLabel, HS_MAX_LABEL_LENGTH) &&
        (version_2_B_Higher() ? xdr_int(xdrs, &(*hs2DH)->errFlg) : 1) );
}

static bool_t xdr_hs3DHist_allStructND(XDR *xdrs, hs3DHist **hs3DH)
{
/*  Translate all of the structure of a 3d Histogram but not its data */

    unsigned int tmp9 = 27;		/* number of overflow bins (3x3x3) */
    float *tmpv;

    if ( (xdrs->x_op == XDR_DECODE) && (*hs3DH == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hs3DH = (hs3DHist *) calloc(1, sizeof(hs3DHist));
    }
    tmpv = (float *)(*hs3DH)->overflow;
    return (xdr_int(xdrs, &(*hs3DH)->type) &&
        xdr_int(xdrs, &(*hs3DH)->id) &&
        (version_3_C_Higher() ? xdr_int(xdrs, &(*hs3DH)->uid) : 1) &&
        xdr_string(xdrs, &(*hs3DH)->title, HS_MAX_TITLE_LENGTH) &&
        xdr_string(xdrs, &(*hs3DH)->category, HS_MAX_CATEGORY_LENGTH) &&
        xdr_int(xdrs, &(*hs3DH)->hbookID) &&
        xdr_int(xdrs, &(*hs3DH)->count) &&
        xdr_int(xdrs, &(*hs3DH)->nXBins) &&
        xdr_int(xdrs, &(*hs3DH)->nYBins) &&
        xdr_int(xdrs, &(*hs3DH)->nZBins) &&
        xdr_float(xdrs, &(*hs3DH)->xMin) &&
        xdr_float(xdrs, &(*hs3DH)->xMax) &&
        xdr_float(xdrs, &(*hs3DH)->yMin) &&
        xdr_float(xdrs, &(*hs3DH)->yMax) &&
        xdr_float(xdrs, &(*hs3DH)->zMin) &&
        xdr_float(xdrs, &(*hs3DH)->zMax) &&
        xdr_array(xdrs, (char **)&tmpv, &tmp9, tmp9, sizeof(float), xdr_float) &&
        xdr_int(xdrs, &(*hs3DH)->xScaleType) &&
        xdr_float(xdrs, &(*hs3DH)->xScaleBase) &&
        xdr_int(xdrs, &(*hs3DH)->yScaleType) &&
        xdr_float(xdrs, &(*hs3DH)->yScaleBase) &&
        xdr_int(xdrs, &(*hs3DH)->zScaleType) &&
        xdr_float(xdrs, &(*hs3DH)->zScaleBase) &&
        xdr_string(xdrs, &(*hs3DH)->xLabel, HS_MAX_LABEL_LENGTH) &&
        xdr_string(xdrs, &(*hs3DH)->yLabel, HS_MAX_LABEL_LENGTH) &&
        xdr_string(xdrs, &(*hs3DH)->zLabel, HS_MAX_LABEL_LENGTH) &&
        xdr_string(xdrs, &(*hs3DH)->vLabel, HS_MAX_LABEL_LENGTH) &&
        (version_2_B_Higher() ? xdr_int(xdrs, &(*hs3DH)->errFlg) : 1) );
}

/* DATA_1DHIST */
static bool_t xdr_d_1dhist(XDR *xdrs, hs1DHist **hsH)
{

    FileOrInteractive = INTERACTIVE;
    return ( xdr_hs1DHist_shortStructND(xdrs, hsH) &&
    	     xdr_hs1DHist_dataOnly(xdrs, hsH) );
}

static bool_t xdr_hs1DHist_shortStructND(XDR *xdrs, hs1DHist **hs1DH)
{
/*  Translate the part of a 1d Histogram structure that is necessary to
    identify it and any structure members that may have changed, but not
    its data */

    return (xdr_int(xdrs, &(*hs1DH)->type) &&
            xdr_int(xdrs, &(*hs1DH)->id) &&
            xdr_int(xdrs, &(*hs1DH)->count) &&
            xdr_float(xdrs, &(*hs1DH)->overflow) &&
            xdr_float(xdrs, &(*hs1DH)->underflow) &&
            ClientVersion < V3_CLIENT ? 1 : xdr_int(xdrs, &(*hs1DH)->errFlg) );
}

static bool_t xdr_hs1DHist_dataOnly(XDR *xdrs, hs1DHist **hs1DH)
{
/*  Translate only the data (and errors) of a 1d Histogram */
    int tmp, tmp1 = 1, tmp0 = 0;

    /* translate data */
    if (!xdr_array(xdrs, (char **)&(*hs1DH)->bins, 
    		(unsigned int *)&(*hs1DH)->nBins, (*hs1DH)->nBins,
            	sizeof(float), xdr_float))	  /* encode/decode bin data */
        return 0;				/* translation error */
#ifdef COM_DEBUG
    printf("Translating 1d_hist:  ");
#endif /*COM_DEBUG*/
    if (xdrs->x_op == XDR_ENCODE) {
        if (FileOrInteractive == HSFILE) {
    	    /* always write errs for xdr file */
            if (!xdr_int(xdrs, &tmp1))
            	return 0;
    	} else {    /* FileOrInteractive == INTERACTIVE */
    	    /* check whether histo requested error data */
            if (!xdr_int(xdrs, &(*(hsC1DHist **)hs1DH)->sendErrs))
                return 0;			/* translation error */
#ifdef COM_DEBUG
            printf(
              "(ENCODE) sendErrs flag = %d, pErrs flag = %d, mErrs flag = %d\n",
            (*(hsC1DHist **)hs1DH)->sendErrs, ((*hs1DH)->pErrs != NULL ? 1 : 0),
            ((*hs1DH)->mErrs) != NULL ? 1 : 0);
#endif /*COM_DEBUG*/
    	}
        if (FileOrInteractive == HSFILE 
        	|| (*(hsC1DHist **)hs1DH)->sendErrs) {
#ifdef COM_DEBUG
            if ( (FileOrInteractive != HSFILE) 
              	  && ((*(hsC1DHist **)hs1DH)->sendErrs)  
                  && (*hs1DH)->pErrs == NULL && (*hs1DH)->mErrs == NULL )
            	fprintf(stderr,
               "\nhsUpdate Info Msg - while xlating 1dhist error data\n\
               Histo asking for errors we don\'t (yet) have\n");
#endif /*COM_DEBUG*/
            if ((*hs1DH)->pErrs == NULL) {
            	if (!xdr_int(xdrs, &tmp0))  /* no positive error data to send */
            	    return 0;			/* translation error */
            } else {
            	if (!xdr_int(xdrs, &tmp1))  /* flag we are sending perrs data */
            	    return 0;			/* translation error */
            	if (!xdr_array(xdrs, (char **)&(*hs1DH)->pErrs, 
    		        (unsigned int *)&(*hs1DH)->nBins, (*hs1DH)->nBins,
            	        sizeof(float), xdr_float) ) /* encode pos error data  */
            	    return 0;			/* translation error */
            }
            if ((*hs1DH)->mErrs == NULL)
            	return (xdr_int(xdrs, &tmp0));	/* no neg error data to send */
            else {
            	return (xdr_int(xdrs, &tmp1) &&
            		xdr_array(xdrs, (char **)&(*hs1DH)->mErrs, 
    		    	    (unsigned int *)&(*hs1DH)->nBins, (*hs1DH)->nBins,
            	    	    sizeof(float), xdr_float)); /* encod neg err data */
            }
        }
        return 1;				/* successful translation */
    }
    
    /* decode error data if source is V2 or greater */
    else if (xdrs->x_op == XDR_DECODE && version_2_B_Higher()) {
    	if (!xdr_int(xdrs, &tmp))		/* sendErrs flag */
            return 0;				/* translation error */
#ifdef COM_DEBUG
        printf("(DECODE) sendErrs flag = %d,\n", tmp);
#endif /*COM_DEBUG*/
        if (tmp) {			/* are any errors being sent? */
            if (!xdr_int(xdrs, &tmp))		/* = 1 if pErrs sent */
            	return 0;			/* translation error */
            if (tmp) {
#ifdef COM_DEBUG
            	printf("   pErrs flag = %d,", tmp);
#endif /*COM_DEBUG*/
            	if (!xdr_array(xdrs, (char **)&(*hs1DH)->pErrs, 
    		        (unsigned int *)&(*hs1DH)->nBins, (*hs1DH)->nBins,
            	        sizeof(float), xdr_float)) /* decode pos (+) err data */
            	    return 0;			/* translation error */
            }			/* translation error */
            if (!xdr_int(xdrs, &tmp))		/* = 1 if mErrs sent */
            	return 0;			/* translation error */
#ifdef COM_DEBUG
            printf(" mErrs flag = %d\n", tmp);
#endif /*COM_DEBUG*/
            if (tmp)
            	return (xdr_array(xdrs, (char **)&(*hs1DH)->mErrs, 
    		    (unsigned int *)&(*hs1DH)->nBins, (*hs1DH)->nBins,
            	    sizeof(float), xdr_float)); /* decode neg (-) err data */
        }
	return 1;				/* successful translation */
    }
    return 1;					/* successful translation */
}

/* DATA_2DHIST */
static bool_t xdr_d_2dhist(XDR *xdrs, hs2DHist **hsH)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hs2DHist_shortStructND(xdrs, hsH) &&
    	     xdr_hs2DHist_dataOnly(xdrs, hsH) );
}

/* DATA_3DHIST */
static bool_t xdr_d_3dhist(XDR *xdrs, hs3DHist **hsH)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hs3DHist_shortStructND(xdrs, hsH) &&
    	     xdr_hs3DHist_dataOnly(xdrs, hsH) );
}

static bool_t xdr_hs2DHist_shortStructND(XDR *xdrs, hs2DHist **hs2DH)
{
/*  Translate the part of a 2d Histogram structure that is necessary to
    identify it and any structure members that may have changed, but not
    its data */

    long tmp9 = 9;			   /* number of overflow bins (3x3) */
    float *tmpv = (float *) (*hs2DH)->overflow;

    return (xdr_int(xdrs, &(*hs2DH)->type) &&
            xdr_int(xdrs, &(*hs2DH)->id) &&
            xdr_int(xdrs, &(*hs2DH)->count) &&
            xdr_array(xdrs, (char **)&tmpv, (unsigned int *)&tmp9, tmp9,
            	      sizeof(float), xdr_float) &&
            ClientVersion < V3_CLIENT ? 1 : xdr_int(xdrs, &(*hs2DH)->errFlg) );
}

static bool_t xdr_hs3DHist_shortStructND(XDR *xdrs, hs3DHist **hs3DH)
{
/*  Translate the part of a 3d Histogram structure that is necessary to
    identify it and any structure members that may have changed, but not
    its data */

    long tmp9 = 27;			   /* number of overflow bins (3x3) */
    float *tmpv = (float *) (*hs3DH)->overflow;

    return (xdr_int(xdrs, &(*hs3DH)->type) &&
            xdr_int(xdrs, &(*hs3DH)->id) &&
            xdr_int(xdrs, &(*hs3DH)->count) &&
            xdr_array(xdrs, (char **)&tmpv, (unsigned int *)&tmp9, tmp9,
            	      sizeof(float), xdr_float) &&
            ClientVersion < V3_CLIENT ? 1 : xdr_int(xdrs, &(*hs3DH)->errFlg) );
}

static bool_t xdr_hs2DHist_dataOnly(XDR *xdrs, hs2DHist **hs2DH)
{
/*  Translate only the data (and errors) of a 2d Histogram */

    unsigned int tmp_nbins = (*hs2DH)->nXBins * (*hs2DH)->nYBins;
    int tmp, tmp1 = 1, tmp0 = 0;
    
    /* translate data */
    if (!xdr_array(xdrs, (char **)&(*hs2DH)->bins, &tmp_nbins, tmp_nbins,
            	sizeof(float), xdr_float) )	/* encode/decode bin data    */
        return 0;				/* translation error */
#ifdef COM_DEBUG
    printf("Translating 2d_hist:  ");
#endif /*COM_DEBUG*/

    /* encode error data */
    if (xdrs->x_op == XDR_ENCODE) {
    	if (FileOrInteractive == HSFILE) {
    	    /* always write errs for xdr file */
            if (!xdr_int(xdrs, &tmp1))
            	return 0;			/* translation error */
    	} else {			/* FileOrInteractive == INTERACTIVE */
    	    /* check whether histo requested error data */
            if (!xdr_int(xdrs, &(*(hsC2DHist **)hs2DH)->sendErrs))
            	return 0;		/* translation error */
#ifdef COM_DEBUG
            printf(
              "(ENCODE) sendErrs flag = %d, pErrs flag = %d, mErrs flag = %d\n",
            (*(hsC2DHist **)hs2DH)->sendErrs, ((*hs2DH)->pErrs != NULL ? 1 : 0),
            ((*hs2DH)->mErrs) != NULL ? 1 : 0);
#endif /*COM_DEBUG*/
    	}
        if (FileOrInteractive == HSFILE || (*(hsC2DHist **)hs2DH)->sendErrs) {
#ifdef COM_DEBUG
            if ( (FileOrInteractive != HSFILE) 
            		&& ((*(hsC2DHist **)hs2DH)->sendErrs) 
            		&& (*hs2DH)->pErrs == NULL && (*hs2DH)->mErrs == NULL )
            	fprintf(stderr,
               "\nhsUpdate Info Msg - while xlating 2dhist error data\n\
               Histo asking for errors we don\'t (yet) have\n");
#endif /*COM_DEBUG*/
            if ((*hs2DH)->pErrs == NULL) {
            	if (!xdr_int(xdrs, &tmp0))  /* no positive error data to send */
            	    return 0;			/* translation error */
            } else {
            	if (!xdr_int(xdrs, &tmp1))  /* flag we are sending perrs data */
            	    return 0;			/* translation error */
             	/* encode positive error data  */
            	if (!xdr_array(xdrs, (char **)&(*hs2DH)->pErrs, &tmp_nbins, 
    		        tmp_nbins, sizeof(float), xdr_float) )
            	    return 0;			/* translation error */
            }
            if ((*hs2DH)->mErrs == NULL)
            	return (xdr_int(xdrs, &tmp0));	/* no neg error data to send */
            else {
            	return (xdr_int(xdrs, &tmp1) &&
            		/* encode negative error data */
            		xdr_array(xdrs, (char **)&(*hs2DH)->mErrs, &tmp_nbins, 
    		        	  tmp_nbins, sizeof(float), xdr_float));
            }
        }
        return 1;				/* successful translation */
    }
    
    /* decode error data if source is V2 or greater */
    else if (xdrs->x_op == XDR_DECODE && version_2_B_Higher()) {
    	if (!xdr_int(xdrs, &tmp))		/* sendErrs flag */
            return 0;				/* translation error */
#ifdef COM_DEBUG
        printf("(DECODE) sendErrs flag = %d,\n", tmp);
#endif /*COM_DEBUG*/
        if (tmp) {			/* are any errors being sent? */
            if (!xdr_int(xdrs, &tmp))		/* = 1 if pErrs sent */
            	return 0;			/* translation error */
            if (tmp) {
#ifdef COM_DEBUG
            	printf("   pErrs flag = %d,", tmp);
#endif /*COM_DEBUG*/
             	/* decode positive (+) error data  */
            	if (!xdr_array(xdrs, (char **)&(*hs2DH)->pErrs, &tmp_nbins, 
    		        tmp_nbins, sizeof(float), xdr_float) )
            	    return 0;			/* translation error */
            }
            if (!xdr_int(xdrs, &tmp))		/* = 1 if mErrs sent */
            	return 0;			/* translation error */
#ifdef COM_DEBUG
            printf(" mErrs flag = %d\n", tmp);
#endif /*COM_DEBUG*/
            if (tmp)
                /* decode negative (-) error data */
            	return (xdr_array(xdrs, (char **)&(*hs2DH)->mErrs, &tmp_nbins, 
    		        tmp_nbins, sizeof(float), xdr_float));
        }
	return 1;				/* successful translation */
    }
    return 1;					/* successful translation */
}

static bool_t xdr_hs3DHist_dataOnly(XDR *xdrs, hs3DHist **hs3DH)
{
/*  Translate only the data (and errors) of a 3d Histogram */

    unsigned int tmp_nbins = (*hs3DH)->nXBins * (*hs3DH)->nYBins * (*hs3DH)->nZBins;
    int tmp, tmp1 = 1, tmp0 = 0;
    
    /* translate data */
    if (!xdr_array(xdrs, (char **)&(*hs3DH)->bins, &tmp_nbins, tmp_nbins,
            	sizeof(float), xdr_float) )	/* encode/decode bin data    */
        return 0;				/* translation error */
#ifdef COM_DEBUG
    printf("Translating 3d_hist:  ");
#endif /*COM_DEBUG*/

    /* encode error data */
    if (xdrs->x_op == XDR_ENCODE) {
    	if (FileOrInteractive == HSFILE) {
    	    /* always write errs for xdr file */
            if (!xdr_int(xdrs, &tmp1))
            	return 0;			/* translation error */
    	} else {			/* FileOrInteractive == INTERACTIVE */
    	    /* check whether histo requested error data */
            if (!xdr_int(xdrs, &(*(hsC3DHist **)hs3DH)->sendErrs))
            	return 0;		/* translation error */
#ifdef COM_DEBUG
            printf(
              "(ENCODE) sendErrs flag = %d, pErrs flag = %d, mErrs flag = %d\n",
            (*(hsC3DHist **)hs3DH)->sendErrs, ((*hs3DH)->pErrs != NULL ? 1 : 0),
            ((*hs3DH)->mErrs) != NULL ? 1 : 0);
#endif /*COM_DEBUG*/
    	}
        if (FileOrInteractive == HSFILE || (*(hsC3DHist **)hs3DH)->sendErrs) {
#ifdef COM_DEBUG
            if ( (FileOrInteractive != HSFILE) 
            		&& ((*(hsC3DHist **)hs3DH)->sendErrs) 
            		&& (*hs3DH)->pErrs == NULL && (*hs3DH)->mErrs == NULL )
            	fprintf(stderr,
               "\nhsUpdate Info Msg - while xlating 3dhist error data\n\
               Histo asking for errors we don\'t (yet) have\n");
#endif /*COM_DEBUG*/
            if ((*hs3DH)->pErrs == NULL) {
            	if (!xdr_int(xdrs, &tmp0))  /* no positive error data to send */
            	    return 0;			/* translation error */
            } else {
            	if (!xdr_int(xdrs, &tmp1))  /* flag we are sending perrs data */
            	    return 0;			/* translation error */
             	/* encode positive error data  */
            	if (!xdr_array(xdrs, (char **)&(*hs3DH)->pErrs, &tmp_nbins, 
    		        tmp_nbins, sizeof(float), xdr_float) )
            	    return 0;			/* translation error */
            }
            if ((*hs3DH)->mErrs == NULL)
            	return (xdr_int(xdrs, &tmp0));	/* no neg error data to send */
            else {
            	return (xdr_int(xdrs, &tmp1) &&
            		/* encode negative error data */
            		xdr_array(xdrs, (char **)&(*hs3DH)->mErrs, &tmp_nbins, 
    		        	  tmp_nbins, sizeof(float), xdr_float));
            }
        }
        return 1;				/* successful translation */
    }
    
    /* decode error data if source is V2 or greater */
    else if (xdrs->x_op == XDR_DECODE && version_2_B_Higher()) {
    	if (!xdr_int(xdrs, &tmp))		/* sendErrs flag */
            return 0;				/* translation error */
#ifdef COM_DEBUG
        printf("(DECODE) sendErrs flag = %d,\n", tmp);
#endif /*COM_DEBUG*/
        if (tmp) {			/* are any errors being sent? */
            if (!xdr_int(xdrs, &tmp))		/* = 1 if pErrs sent */
            	return 0;			/* translation error */
            if (tmp) {
#ifdef COM_DEBUG
            	printf("   pErrs flag = %d,", tmp);
#endif /*COM_DEBUG*/
             	/* decode positive (+) error data  */
            	if (!xdr_array(xdrs, (char **)&(*hs3DH)->pErrs, &tmp_nbins, 
    		        tmp_nbins, sizeof(float), xdr_float) )
            	    return 0;			/* translation error */
            }
            if (!xdr_int(xdrs, &tmp))		/* = 1 if mErrs sent */
            	return 0;			/* translation error */
#ifdef COM_DEBUG
            printf(" mErrs flag = %d\n", tmp);
#endif /*COM_DEBUG*/
            if (tmp)
                /* decode negative (-) error data */
            	return (xdr_array(xdrs, (char **)&(*hs3DH)->mErrs, &tmp_nbins, 
    		        tmp_nbins, sizeof(float), xdr_float));
        }
	return 1;				/* successful translation */
    }
    return 1;					/* successful translation */
}

/* FILE_1DHIST */
static bool_t xdr_f_1dhist(XDR *xdrs, hs1DHist **hsH)
{
    FileOrInteractive = HSFILE;
    return ( xdr_hs1DHist_allStructND(xdrs, hsH) &&
    	     xdr_hs1DHist_dataOnly(xdrs, hsH) ); /* flag to/from file */
}

/* FILE_2DHIST */
static bool_t xdr_f_2dhist(XDR *xdrs, hs2DHist **hsH)
{
    FileOrInteractive = HSFILE;
    return ( xdr_hs2DHist_allStructND(xdrs, hsH) &&
    	     xdr_hs2DHist_dataOnly(xdrs, hsH) ); /* flag to/from file */
}

/* FILE_3DHIST */
static bool_t xdr_f_3dhist(XDR *xdrs, hs3DHist **hsH)
{
    FileOrInteractive = HSFILE;
    return ( xdr_hs3DHist_allStructND(xdrs, hsH) &&
    	     xdr_hs3DHist_dataOnly(xdrs, hsH) ); /* flag to/from file */
}

/* ITEMLIST_NTUPLE */
static bool_t xdr_l_ntuple(XDR *xdrs, hsNTuple **hsN)
{
    FileOrInteractive = INTERACTIVE;
    if ( !xdr_hsNTuple_allStructND(xdrs, hsN) )
        return 0;
    if (xdrs->x_op == XDR_DECODE) {
	(*hsN)-> n = 0;			/* fix up value, no data yet */
    }
    return 1;
}

static bool_t xdr_hsNTuple_allStructND(XDR *xdrs, hsNTuple **hsN)
{
/*  Translate the part of an n-tuple structure that is necessary to
    identify it and any structure members that may have changed, but not
    its data */

    if ( (xdrs->x_op == XDR_DECODE) && (*hsN == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsN = (hsNTuple *) malloc(sizeof(hsCNTuple));
	 (*hsN)-> title          = NULL;	/* for xdr_string below */
	 (*hsN)-> category       = NULL;	/*  "   "          "    */
    	 (*hsN)-> names          = NULL;	/*  "  xdr_array   "    */
    	 (*hsN)-> data           = NULL;	/* no data              */
    	 (*hsN)-> extensions [0] = NULL;
    	 (*hsN)-> extensions [1] = NULL;
    	 (*hsN)-> extensions [2] = NULL;
    	 (*hsN)-> extensions [3] = NULL;
    	 (*hsN)-> chunkSzData    = 0;
    	 (*hsN)-> chunkSzExt     = 10;
    	 (*hsN)-> uid            = 0;	  /* init'ze for v2 or v3 file/client */
         ((hsCNTuple *)(*hsN))->fromTuple = 0;
	 ((hsCNTuple *)(*hsN))->toTuple = -1;
	 ((hsCNTuple *)(*hsN))->inhibitResetRefresh = 0;
	 ((hsCNTuple *)(*hsN))->needsCompleteUpdate = 0xffffffff;
    }
    return ( xdr_int(xdrs, &(*hsN)->type) &&
             xdr_int(xdrs, &(*hsN)->id) &&
             (version_3_C_Higher() ? xdr_int(xdrs, &(*hsN)->uid) : 1) &&
             xdr_string(xdrs, &(*hsN)->title, HS_MAX_TITLE_LENGTH) &&
             xdr_string(xdrs, &(*hsN)->category, HS_MAX_CATEGORY_LENGTH) &&
             xdr_int(xdrs, &(*hsN)->hbookID) &&
             xdr_int(xdrs, &(*hsN)->n) &&
             xdr_int(xdrs, &(*hsN)->nVariables) &&
             xdr_array(xdrs, (char **)&(*hsN)->names,
                (unsigned int *)&(*hsN)->nVariables, UINT_MAX, sizeof(char *),
            	xdr_wrapstring) );
}

/* DATA_NTUPLE (incremental encode/decode of n-tuple data) */
static bool_t xdr_d_ntuple(XDR *xdrs, hsNTuple **hsN)
{
    int from, to;
    
    FileOrInteractive = INTERACTIVE;
    if (xdrs->x_op == XDR_ENCODE) {
	from = ((hsCNTuple *) (*hsN))->fromTuple;
        to   = ((hsCNTuple *) (*hsN))->toTuple;
        if (from > to) {
            fprintf(stderr, "hs_update: Internal Error calling xdr_d_ntuple\n");
            return 0;			/* error from > to */
        }
    }
    if ( xdr_hsNTuple_shortStructND(xdrs, hsN) &&
    	 xdr_int(xdrs, &from) &&
    	 xdr_int(xdrs, &to) ) {
    	if (xdrs->x_op == XDR_DECODE)
    	    (*hsN)->n = to;		/* fix up n to reflect data received */
    	if (xdrs->x_op == XDR_ENCODE && (*hsN)->hbookID != 0)
    	    return (xdr_hsNTuple_HBdataEncode(xdrs, hsN, from, to));
    	else
    	    return (xdr_hsNTuple_dataOnly(xdrs, hsN, from, to));
    }
    else
    	return 0;		   /* a translation error occurred somewhere */
}

static bool_t xdr_hsNTuple_shortStructND(XDR *xdrs, hsNTuple **hsN)
{
/*  Translate the part of an n-tuple structure that is necessary to
    identify it and any structure members that may have changed, but not
    its data. */

    return ( xdr_int(xdrs, &(*hsN)->type) &&
             xdr_int(xdrs, &(*hsN)->id) &&
             xdr_int(xdrs, &(*hsN)->n) );
}

/*
 * xdr_hsNTuple_dataOnly
 *
 * N-tuple data storage consists of a data area with its own chunk-size and
 * up to 4 extension areas all using one extension chunk size.
 *
 * This routine takes care of all memory allocation on the decode side of xdr
 * translation.  If decoding, the routine checks whether there is already data
 * stored in the n-tuple.  If no data yet exists, all data will go into the
 * data area and the data chunk size is set to the number of elements.
 * If adding to existing data in the n-tuple, the routine determines whether
 * the current chunk-sizes are adequate, and if not calls gatherNtupleData()
 * to gather all existing n-tuple data into the data area and set things so
 * in-coming data will also go into the data area.
 *
 * Once all this is done, the encode and decode paths are the same:
 *
 * The routine merely counts from the starting n-tuple number to the ending
 * n-tuple in order to determine how many times to call xdr_float and does
 * a little footwork to get the data into or out of the correct memory area.
 *
 **** Note: When decoding, it is assumed that end == n during memory allocation.
 */
static bool_t xdr_hsNTuple_dataOnly(XDR *xdrs, hsNTuple **hsN, int start,
					int end)
{
    int i, j, k, l, quot, rem;

    if (xdrs->x_op == XDR_DECODE) {
    	if (end != (*hsN)->n) {
    	    fprintf(stderr, "Internal error in xdr_hsNTuple_dataOnly.\n");
	    return 0;
    	}
	if ((*hsN)->data == NULL && (*hsN)->extensions[0] == NULL) {
	    /** n-tuple has no data in it yet **/
	    if (start != 0 || (*hsN)->chunkSzData != 0) {
	    	fprintf(stderr,"xdr_hsNTuple_dataOnly: Internal Error.\n");
	    	return 0;
	    }
    	    /* allocate memory in data area for all incoming data */
	    (*hsN)->data = (float *) malloc(sizeof(float) * (*hsN)->n
    	  					          * (*hsN)->nVariables);
    	    (*hsN)->chunkSzData = (*hsN)->n;
	}
	else {		/** we're adding to data already in the n-tuple	**/
    	/* see if chunk-sizes are adequate for the expected data  */
	    quot = ((*hsN)->n - (*hsN)->chunkSzData) / (*hsN)->chunkSzExt;
	    rem = ((*hsN)->n - (*hsN)->chunkSzData) % (*hsN)->chunkSzExt;
	    k = (quot < 4) ? quot : ((quot==4 && rem==0) ? 3 : -1);
	    if (k == -1)	/* no, not adequate */
    		gatherNtupleData(*hsN, start);  /* gather up existing data
    					           and adjust chunk sizes */
	    else {	/* yes, just allocate more extension space if needed */
	    	for (i = 0; i <= k; ++i)
    		    if ((*hsN)->extensions[i] == NULL)
    	                /* allocate memory for expected data */
    		        (*hsN)->extensions[i] = (float *) malloc(sizeof(float)
    	  			     * (*hsN)->nVariables * (*hsN)->chunkSzExt);
	    }
	}
    }	/* end of DECODE-only code */
    /* ENCODE/DECODE n-tuple data:
	j counts from the starting n-tuple (where to start getting/putting data)
	up to the ending n-tuple number (last tuple to get/put), exclusive.
	Do some footwork to get the data into/out-of the correct memory area. */
    for (j = start; (j < end) && (j < (*hsN)->chunkSzData); ++j) {
    	    /* j = n-tuple number encoding/decoding, also data area subscript */
	for (i = 0; i < (*hsN)->nVariables; ++i) /* for ea. variable in tuple */
    	    if (!xdr_float(xdrs, &((*hsN)->data[j * (*hsN)->nVariables + i])))
    	    	return 0;		/* xdr translation error */
    }
    if (j == (*hsN)->chunkSzData) {
    	l = 0;
    	k = 0;
    }
    else {
    	l = (j - (*hsN)->chunkSzData) / (*hsN)->chunkSzExt;
    	k = (j - (*hsN)->chunkSzData) % (*hsN)->chunkSzExt;
    }
    for (l = l; l < 4; ++l) {  /* l = extension area number (subscript) */
        for (j=j, k=k; (j < end) && (k < (*hsN)->chunkSzExt); ++j, ++k) {
    	    /* j = n-tuple number encoding/decoding, k = tuple w/in ext. area */
	    for (i = 0; i < (*hsN)->nVariables; ++i) /* i = variable number   */
	    	/* for ea. variable in tuple */
    		if (!xdr_float(xdrs,
    			      &((*hsN)->extensions[l][k*(*hsN)->nVariables+i])))
    	    	    return 0;		/* xdr translation error */
	}
	k = 0;
    }
    return 1;
}

/*
 * xdr_hsNTuple_HBdataEncode - this routine encodes ntuple data for HBOOK items
 */
static bool_t xdr_hsNTuple_HBdataEncode(XDR *xdrs, hsNTuple **hsN, int from,
		int to)
{
    int i, j;

    if ((*hsN)->hbookID == 0 || (*hsN)->chunkSzData != to - from) {
    	fprintf(stderr,
    		"hs_update: Internal Error encoding HBOOK ntuple data\n");
    	return 0;
    }
    for (j = 0; j < (*hsN)->chunkSzData; ++j) {
	for (i = 0; i < (*hsN)->nVariables; ++i) /* for ea. variable in tuple */
    	    if (!xdr_float(xdrs, &((*hsN)->data[j * (*hsN)->nVariables + i])))
    		return 0;		/* xdr translation error */
    }
    return 1;
}
		
/* FILE_NTUPLE */
static bool_t xdr_f_ntuple(XDR *xdrs, hsNTuple **hsN)
{
    FileOrInteractive = HSFILE;
    return ( xdr_hsNTuple_allStructND(xdrs, hsN) &&
	     xdr_hsNTuple_dataOnly(xdrs, hsN, 0, (*hsN)->n) );    /* all data */
}

/*
 * gatherNtupleData - Gather data from the data and extensions sections of
 *                    an n-tuple and put it into a new, larger, data section.
 *                    Free memory. Reset chunk sizes and pointers accordingly.
 *		      This routine is called when we KNOW the chunk sizes
 *		      are too small to fit incoming data.  However we do need
 *		      to check whether the extensions are being used as of
 *		      yet; if not, just adjusting the extension chunk size
 *		      will do the trick.
 *
 *		      This routine is used for the DECODE side of DATA_NTUPLE.
 */
static void gatherNtupleData(hsNTuple *nTuple, int oldN)
{
    float *newData;
    int i = 0, j, k;
    
    if (nTuple->extensions[0] == NULL) {	/* are extensions used yet? */
	/* no, just reset extensions chunk size */
    	nTuple->chunkSzExt = (nTuple->n - nTuple->chunkSzData + 1) / 2;
    	/* allocate memory for expected data */
    	nTuple->extensions[0] = (float *) malloc(sizeof(float)
    	  			     * nTuple->nVariables * nTuple->chunkSzExt);
    	nTuple->extensions[1] = (float *) malloc(sizeof(float)
    	  			     * nTuple->nVariables * nTuple->chunkSzExt);
    	return;
    }
    
    newData = (float *) malloc(nTuple->n * nTuple->nVariables * sizeof(float));
    
    for (i = 0; (i < nTuple->chunkSzData * nTuple->nVariables)
    			&& (i < oldN * nTuple->nVariables); ++i)
	newData[i] = nTuple->data[i];
    for (k = 0; k < 4; ++k) {
	if (nTuple->extensions == NULL || i >= oldN * nTuple->nVariables)
	    break;
	for (j = 0; j < nTuple->chunkSzExt * nTuple->nVariables; ++j) {
	    if (i >= oldN * nTuple->nVariables)
	    	continue;
	    newData[i++] = nTuple->extensions[k][j];
	}
	free(nTuple->extensions[k]);
	nTuple->extensions[k] = NULL;
    }
    for (k = k; k < 4; ++k)
    	if (nTuple->extensions[k] != NULL) {
 	    free(nTuple->extensions[k]);
	    nTuple->extensions[k] = NULL;
    	}
    if (nTuple->data != NULL)
    	free(nTuple->data);
    nTuple->data = newData;
    nTuple->chunkSzData = nTuple->n;
    if (nTuple->chunkSzExt * 2 >= nTuple->n - oldN)
	nTuple->chunkSzExt *= 2;	    /* increase extension chunk size */
    else
	nTuple->chunkSzExt = nTuple->n - oldN;
}

static bool_t xdr_f_nfit_ENC(XDR *xdrs, hsNFit **hsN)
{
    int endPos = 0, endPosPos = 0;
    int i, nscovar;
    minuitParam *partmp;

    if ( xdr_int(xdrs, &(*hsN)->type) &&
         xdr_int(xdrs, &(*hsN)->id) &&
         xdr_int(xdrs, &(*hsN)->uid) &&
         xdr_string(xdrs, &(*hsN)->title, HS_MAX_TITLE_LENGTH) &&
         xdr_string(xdrs, &(*hsN)->category, HS_MAX_CATEGORY_LENGTH) &&
         xdr_int(xdrs, &(*hsN)->hbookID) &&
         xdr_int(xdrs, &(*hsN)->itemVersion) ) {
       endPosPos = xdr_getpos(xdrs);      
       if (!( xdr_int(xdrs, &endPos) &&
            xdr_int(xdrs,&(*hsN)->calculationMode) &&
            xdr_double(xdrs,&(*hsN)->lowLimit) &&
            xdr_double(xdrs,&(*hsN)->upLimit) &&
            xdr_string(xdrs, &(*hsN)->expr, UINT_MAX) &&
            xdr_string(xdrs, &(*hsN)->funcPath, MAXPATHLEN) &&
            xdr_int(xdrs, &(*hsN)->nParams))) return 0;
       /* Do not use xdr_array, to be able to handle more simply the 
       	memory allocation upon decode */
       partmp = (minuitParam *)(*hsN)->mParam; 	     
       for (i=0;  i<(*hsN)->nParams; i++, partmp++) {
            if (!xdr_minuitParam(xdrs, partmp)) return 0;
       }          
       if (!xdr_int(xdrs, &(*hsN)->istatCovariance)) return 0;
       if (((*hsN)->istatCovariance == 0) ||
          ((*hsN)->covarianceMatrix == NULL)) { 
                endPos = xdr_getpos(xdrs);
                if ( xdr_setpos(xdrs, endPosPos) &&
                   xdr_int(xdrs, &endPos) &&
                   xdr_setpos(xdrs, endPos) )
                  return(xdr_ret_1());
       }          
       nscovar = (*hsN)->nParams * (*hsN)->nParams;                
       if (xdr_array(xdrs, (char **)&(*hsN)->covarianceMatrix,
                    (unsigned int *)&nscovar, nscovar,
                    sizeof(double), xdr_double)) {
                endPos = xdr_getpos(xdrs);
                if ( xdr_setpos(xdrs, endPosPos) &&
                   xdr_int(xdrs, &endPos) &&
                   xdr_setpos(xdrs, endPos) )
                  return(xdr_ret_1());
          }         
    }
    return(xdr_ret_error());
}

static bool_t xdr_f_nfit(XDR *xdrs, hsNFit **hsN)
{
    unsigned int nscovar;
    int i;
    char *etmp = NULL;
    minuitParam *partmp;
    
    if ( (xdrs->x_op == XDR_DECODE) && (*hsN == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsN = (hsNFit *) malloc(sizeof(hsNFit));
	 (*hsN)-> title          = NULL;	/* for xdr_string below */
	 (*hsN)-> category       = NULL;	/*  "   "          "    */
	 (*hsN)-> expr           = NULL;	/*  "   "          "    */
	 (*hsN)-> funcPath       = NULL;	/*  "   "          "    */
	 (*hsN)-> mParam         = NULL;	/*  "   "          "    */
	 (*hsN)-> covarianceMatrix = NULL;      /*  "  xdr_array   "    */
    	 (*hsN)-> uid            = 0;	  /* init'ze for v2 or v3 file/client */
    }	 
    if ((xdrs->x_op == XDR_ENCODE) )
    	return (xdr_f_nfit_ENC(xdrs, hsN));
    	
    	 /* Also create memory for these items, so that we can free it 
    	 	with normal ( e.g. non xdr_free) calls later on. Assume 
    	 	we can allocate without leaving dangling pointers */
    	 	
    (*hsN)->title = (char *) malloc(sizeof(char) * (HS_MAX_TITLE_LENGTH +1));
    (*hsN)->category = 
                 (char *) malloc(sizeof(char) * (HS_MAX_CATEGORY_LENGTH +1));
    
    /* We don't do the expression, as we have no reasonable maximum length
    	We'll do the freeing correctly once we know the length */
    	 
    (*hsN)->funcPath = 	(char *) malloc(sizeof(char) * (MAXPATHLEN + 1));
      	 	
    	
    if    (!(xdr_int(xdrs, &(*hsN)->type) &&
             xdr_int(xdrs, &(*hsN)->id) &&
             xdr_int(xdrs, &(*hsN)->uid) &&
             xdr_string(xdrs, &(*hsN)->title, HS_MAX_TITLE_LENGTH) &&
             xdr_string(xdrs, &(*hsN)->category, HS_MAX_CATEGORY_LENGTH) &&
             xdr_int(xdrs, &(*hsN)->hbookID) &&
             xdr_int(xdrs, &(*hsN)->itemVersion) &&
             xdr_int(xdrs, &(*hsN)->endPos))) return 0;
    if (DontReadNfitRec != 0 ) 
         return (xdr_setpos(xdrs, (*hsN)->endPos)); 
    else if  (!( xdr_int(xdrs,&(*hsN)->calculationMode) &&
                 xdr_double(xdrs,&(*hsN)->lowLimit) &&
                 xdr_double(xdrs,&(*hsN)->upLimit) &&
                 xdr_string(xdrs, &etmp, UINT_MAX) &&
        	 xdr_string(xdrs, &(*hsN)->funcPath, MAXPATHLEN) &&
        	 xdr_int(xdrs, &(*hsN)->nParams))) return 0;
    if (strlen(etmp) > 1) { 	/* require 1 meaningful char */  	 
        (*hsN)->expr = (char *) malloc(sizeof(char) * (strlen(etmp) + 1));
        strcpy((*hsN)->expr,etmp);
    }    
    xdr_free(xdr_string, &etmp); /* clean xdr_free */
    partmp = (minuitParam *) malloc(sizeof(minuitParam) * (*hsN)->nParams);
    (*hsN)->mParam = partmp;
    for (i=0;  i<(*hsN)->nParams; i++, partmp++) {
            if (!xdr_minuitParam(xdrs, partmp)) return 0;
     }            
         	 
    if (!xdr_int(xdrs, &(*hsN)->istatCovariance)) return 0;
    if ((*hsN)->istatCovariance == 0) {
        (*hsN)->covarianceMatrix = NULL;
        return 1;
    }
    nscovar = (*hsN)->nParams * (*hsN)->nParams;
    (*hsN)->covarianceMatrix = (double *) malloc(nscovar * sizeof(double));
    if (!(xdr_array(xdrs, (char **)&(*hsN)->covarianceMatrix,
                    (unsigned int *)&nscovar, nscovar,
                    sizeof(double), xdr_double))) return 0;
    return 1;               
}

static bool_t xdr_minuitParam(XDR *xdrs, minuitParam *hsMp)
{
   unsigned int i;

   if (	     xdr_char(xdrs, &(hsMp->active)) &&
             xdr_char(xdrs, &(hsMp->nameBlank)) &&
             xdr_char(xdrs, &(hsMp->valueBlank)) &&
             xdr_char(xdrs, &(hsMp->initValueBlank)) &&
             xdr_char(xdrs, &(hsMp->lowLimBlank)) &&
             xdr_char(xdrs, &(hsMp->upLimBlank)) &&
             xdr_char(xdrs, &(hsMp->stepBlank)) &&
             xdr_char(xdrs, &(hsMp->fixed)) ) {
       for (i = 0; i < 11; ++i)
           if (!xdr_char(xdrs, &(hsMp->name[i])))
             	return (xdr_ret_error() );
       if (  xdr_double(xdrs, &(hsMp->value)) &&
             xdr_double(xdrs, &(hsMp->initValue)) &&
             xdr_double(xdrs, &(hsMp->lowLim)) &&
             xdr_double(xdrs, &(hsMp->upLim)) &&
             xdr_double(xdrs, &(hsMp->step)) &&
             xdr_double(xdrs, &(hsMp->parabolicError)) &&
             xdr_double(xdrs, &(hsMp->minosError[0])) &&
             xdr_double(xdrs, &(hsMp->minosError[1])) &&
             xdr_double(xdrs, &(hsMp->globcc)) )
           return (xdr_ret_1());
       else
           return (xdr_ret_error());
   }
   else 
       return (xdr_ret_error());
}

static int version_3_C_Higher(void)
{
    return ( FileOrInteractive == HSFILE
    	     ? (FileVersion >= HS_FILE_C ? 1 : 0)	/* HSFILE      */
             : (ClientVersion >= V3_CLIENT ? 1 : 0) );	/* INTERACTIVE */
}

static int version_2_B_Higher(void)
{
    return ( FileOrInteractive == HSFILE
    	     ? (FileVersion >= HS_FILE_B ? 1 : 0)	/* HSFILE      */
             : (ClientVersion >= V2_CLIENT ? 1 : 0) );	/* INTERACTIVE */
}

static bool_t xdr_f_1dhist_dir(XDR *xdrs, hs1DHist **hsH)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    return ( xdr_hs1DHist_allStructND(xdrs, hsH)  );
}

static bool_t xdr_f_2dhist_dir(XDR *xdrs, hs2DHist **hsH)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    return ( xdr_hs2DHist_allStructND(xdrs, hsH)  );
}

static bool_t xdr_f_3dhist_dir(XDR *xdrs, hs3DHist **hsH)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    return ( xdr_hs3DHist_allStructND(xdrs, hsH)  );
}

static bool_t xdr_f_ntuple_dir(XDR *xdrs, hsNTuple **hsN)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    return ( xdr_hsNTuple_allStructND(xdrs, hsN)  );
}

static bool_t xdr_f_1dhist_dat(XDR *xdrs, hs1DHist **hsH)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    if (xdr_setpos(xdrs, LocDataPos))
	return (xdr_hs1DHist_dataOnly(xdrs, hsH) );
    else
	return (xdr_ret_error() );
}

static bool_t xdr_f_2dhist_dat(XDR *xdrs, hs2DHist **hsH)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    if (xdr_setpos(xdrs, LocDataPos))
	return ( xdr_hs2DHist_dataOnly(xdrs, hsH) );
    else
	return (xdr_ret_error() );
}

static bool_t xdr_f_3dhist_dat(XDR *xdrs, hs3DHist **hsH)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    if (xdr_setpos(xdrs, LocDataPos))
	return ( xdr_hs3DHist_dataOnly(xdrs, hsH) );
    else
	return (xdr_ret_error() );
}

static bool_t xdr_f_ntuple_dat(XDR *xdrs, hsNTuple **hsN)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    if (xdr_setpos(xdrs, LocDataPos))
	return ( xdr_hsNTuple_dataOnly(xdrs, hsN, 0, (*hsN)->n) ); /*all data*/
    else
	return (xdr_ret_error() );
}

static bool_t xdr_f_group(XDR *xdrs, hsGroup **hsG)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    return ( xdr_hsGroup_allStruct(xdrs, hsG) );
}

static bool_t xdr_f_group_dir(XDR *xdrs, hsGroup **hsG)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    return ( xdr_hsGroup_shortStructND(xdrs, hsG)  );
}

static bool_t xdr_d_group(XDR *xdrs, hsGroup **hsG)
{
   return 0;
}

static bool_t xdr_l_group(XDR *xdrs, hsGroup **hsG)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_hsGroup_allStruct(xdrs, hsG) );
}

static bool_t xdr_hsGroup_allStruct(XDR *xdrs, hsGroup **hsG)
{
/*  Translate the entire part of an group structure  */

    if ( (xdrs->x_op == XDR_DECODE) && (*hsG == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsG = (hsGroup *) malloc(sizeof(hsGroup));
	 (*hsG)-> title          = NULL;	/* for xdr_string below */
	 (*hsG)-> category       = NULL;	/*  "   "          "    */
    	 (*hsG)-> itemId         = NULL;	/*  "  xdr_array   "    */
    	 (*hsG)-> errsDisp       = NULL;	/* no data              */
    	 (*hsG)-> uid            = 0;	  /* init'ze for v2 or v3 file/client */
    }
    return ( xdr_int(xdrs, &(*hsG)->type) &&
             xdr_int(xdrs, &(*hsG)->id) &&
             xdr_int(xdrs, &(*hsG)->uid) &&
             xdr_string(xdrs, &(*hsG)->title, HS_MAX_TITLE_LENGTH) &&
             xdr_string(xdrs, &(*hsG)->category, HS_MAX_CATEGORY_LENGTH) &&
             xdr_int(xdrs, &(*hsG)->hbookID) &&
             xdr_int(xdrs, &(*hsG)->groupType) &&
             xdr_int(xdrs, &(*hsG)->numItems) &&
             xdr_array(xdrs, (char **)&(*hsG)->itemId,
                (unsigned int *)&(*hsG)->numItems, UINT_MAX, sizeof(int),
            	xdr_int) &&
             xdr_array(xdrs, (char **)&(*hsG)->errsDisp,
                (unsigned int *)&(*hsG)->numItems, UINT_MAX, sizeof(int),
            	xdr_int) );
    
}

static bool_t xdr_hsGroup_shortStructND(XDR *xdrs, hsGroup **hsG)
{
    if ( (xdrs->x_op == XDR_DECODE) && (*hsG == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsG = (hsGroup *) malloc(sizeof(hsGroup));
	 (*hsG)-> title          = NULL;	/* for xdr_string below */
	 (*hsG)-> category       = NULL;	/*  "   "          "    */
    	 (*hsG)-> itemId         = NULL;	/*  "  xdr_array   "    */
    	 (*hsG)-> errsDisp       = NULL;	/* no data              */
    	 (*hsG)-> uid            = 0;	  /* init'ze for v2 or v3 file/client */
    }
    return ( xdr_int(xdrs, &(*hsG)->type) &&
             xdr_int(xdrs, &(*hsG)->id) &&
             xdr_int(xdrs, &(*hsG)->uid) &&
             xdr_string(xdrs, &(*hsG)->title, HS_MAX_TITLE_LENGTH) &&
             xdr_string(xdrs, &(*hsG)->category, HS_MAX_CATEGORY_LENGTH) &&
             xdr_int(xdrs, &(*hsG)->hbookID) &&
             xdr_int(xdrs, &(*hsG)->groupType) &&
             xdr_int(xdrs, &(*hsG)->numItems) );
}

static bool_t xdr_f_group_dat(XDR *xdrs, hsGroup **hsG)
{
    FileOrInteractive = HSFILE;			 /* flag to/from file */
    if (xdr_setpos(xdrs, LocDataPos))
	return (xdr_group_dataOnly(xdrs, hsG) );
    else
	return (xdr_ret_error() );
}

static bool_t xdr_group_dataOnly(XDR *xdrs, hsGroup **hsG)
{
    if (xdrs->x_op == XDR_DECODE) {
       return (xdr_array(xdrs, (char **)&(*hsG)->itemId,
			 (unsigned int *)&(*hsG)->numItems, UINT_MAX,
			  sizeof(int), xdr_int) &&
	       xdr_array(xdrs, (char **)&(*hsG)->errsDisp,
			 (unsigned int *)&(*hsG)->numItems, UINT_MAX, 
			 sizeof(int), xdr_int ));
    }
    else
	return (xdr_ret_error() );
}

/* DATA_CONFIG */
static bool_t xdr_d_config(XDR *xdrs, hsConfigString **hsC)
{
    FileOrInteractive = INTERACTIVE;
    return ( xdr_config_shortStructND(xdrs, hsC) &&
    	     xdr_config_dataOnly(xdrs, hsC) );
}

static bool_t xdr_config_shortStructND(XDR *xdrs, hsConfigString **hsC)
{
/*  Translate the part of a config string structure that is necessary to
    identify it and any structure members that may have changed, but not
    its data */

    if ( (xdrs->x_op == XDR_DECODE) && (*hsC == NULL) ) {
         /* create memory for the item if its pointer is NULL when decoding */
	 *hsC = (hsConfigString *) malloc(sizeof(hsConfigString));
	 (*hsC)-> title          = NULL;
	 (*hsC)-> category       = NULL;
	 (*hsC)-> configString   = NULL;
    	 (*hsC)-> id             = 0;
    	 (*hsC)-> uid            = 0;
    	 (*hsC)-> hbookID        = 0;
    }

    return (xdr_int(xdrs, &(*hsC)->type) &&
            xdr_int(xdrs, &(*hsC)->stringLength) );
}

static bool_t xdr_config_dataOnly(XDR *xdrs, hsConfigString **hsC)
{
    return (xdr_string(xdrs, &(*hsC)->configString, (*hsC)->stringLength));
}

static int xdr_ret_error(void)
{
    return 0;
}

static int xdr_ret_1(void)
{
    return 1;
}
