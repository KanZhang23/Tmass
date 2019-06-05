/*******************************************************************************
*									       *
* histprotocol.h -- interface for communications between Histo-Scope and its   *
*                   client process.  This module defines the protocol, simple  *
*		    status returns, and the structure definition and xdr       *
*		    routines for the message protocol structure.	       *
*									       *
* Copyright (c) 1992 Universities Research Association, Inc.		       *
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
* June 1, 1992								       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modified by:  JMK - 8/24/93 - Added CLIENT_WAITING, CONTROL_SET,	       *
*				CONTROL_READ, REQUEST_ERRORS, Save file	       *
*				Format B, and V2_CLIENT itemID		       *
* 		JMK -  9/1/93 - Added NO_ERRORS_AVAIL			       *
* 		JMK - 9/30/93 - Added Triggers				       *
* 		JMK -  4/8/96 - Added Client Req for Captive Histos to Exit    *
*									       *
*******************************************************************************/

#define COM_OK 0
#define COM_FAILURE 1
#define NO_CONNECTION -1

#ifndef DONT_DEFINE_MSG_STRUCT
/* Message codes... */
#define HISTOSCOPE_CONNECTING 1	     /* Histo-Scope -> Client */
#define CLIENT_CONFIRMING 2	     /*  C -> H		      */
#define CONFIRM_CODE -123456789      /*  for all messages     */
#define V1_CLIENT 0		     /*  itemID of CLIENT_CONFIRMING msg */
#define V2_CLIENT 1		     /*     "   "    "        */
#define V3_CLIENT 2		     /*     "   "    "        */
#define V4_CLIENT 3		     /*     "   "    "        */
#define ITEM_TO_LIST 3		     /*  C -> H 	      */
#define END_OF_LIST 4		     /*  C -> H		      */
#define REQUEST_UPDATES 5	     /*  H -> C		      */
#define UPDATES_FINISHED 6	     /*  C -> H		      */
#define HERE_IS_DATA 7		     /*  C -> H		      */
#define ITEM_DELETED 8		     /*  C -> H		      */
#define NEW_ITEM 9		     /*  C -> H		      */
#define NTUPLE_RESET 10		     /*  C -> H		      */
#define REQ_LAST_UPDATE 11	     /*  C -> H		      */
#define BEGIN_LIST 12		     /*  C -> H		      */
#define END_NTUPLE_DATA	13	     /*  C -> H		      */
#define NO_ERRORS_AVAIL 18	     /*  C -> H		      */
#define REQUEST_ERRORS 19	     /*  H -> C		      */
#define CONTROL_SET 20		     /*  H -> C		      */
#define CONTROL_READ 21		     /*  C -> H		      */
#define TRIGGER_SET 22		     /*  H -> C		      */
#define TRIGGER_READ 23		     /*  C -> H		      */
#define TRIGGER_RESET 24	     /*  C -> H		      */
#define LOAD_CONFIG 31		     /*  C -> H		      */
#define REQ_CAPTIVE_HISTO_EXIT 96    /*  C -> H		      */
#define CLIENT_WAITING 97	     /*  C -> H		      */
#define HISTOSCOPE_DONE 98	     /*  H -> C		      */
#define CLIENT_DONE 99		     /*  C -> H		      */
#define INHIBIT_RESET_REFRESH 150    /*  C -> H		      */
#define ALLOW_RESET_REFRESH   151    /*  C -> H		      */
#define HISTO_TASK_OK      152       /*  H -> C		      */
#define HISTO_TASK_FAILURE 153       /*  H -> C		      */

/* Message structure... */
typedef struct _msgStruct {	     /* Message structure:    */
    int code;			     /*  message code	      */
    int confirm;		     /*  confirmation code    */
    int itemID;			     /*  item id	      */
    int dataLength;		     /*  size of data to read */
} msgStruct;

/* Update list structure... */
typedef struct _updStruct {	     /* Update structure:     */
    int itemID;			     /*  item id	      */
    int type;			     /*  item type	      */
    int updateLevel;		     /*  item update level    */
    float updateLevelF;		     /*  item update level (float)    */
} updStruct;

int hs_ClientVersion(void);    /* found in HistoClient.c and communications.c */

/* XDR functions and prototypes for translating the message and update 
 * structures */

static bool_t xdr_msgStruct (XDR *xdrs, msgStruct *msg);

static bool_t xdr_msgStruct (XDR *xdrs, msgStruct *msg)
{
    return (xdr_int(xdrs, &msg->code) &&
            xdr_int(xdrs, &msg->confirm) &&
            xdr_int(xdrs, &msg->itemID) &&
            xdr_int(xdrs, &msg->dataLength));

}

static bool_t xdr_updStruct (XDR *xdrs, updStruct *upd);

static bool_t xdr_updStruct (XDR *xdrs, updStruct *upd)
{
    return ( xdr_int(xdrs, &upd->itemID) &&
             xdr_int(xdrs, &upd->type) &&
             xdr_int(xdrs, &upd->updateLevel) &&
             xdr_float(xdrs, &upd->updateLevelF) );

}
#endif /* DONT_DEFINE_MSG_STRUCT */
