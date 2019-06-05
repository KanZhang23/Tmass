/*******************************************************************************
*									       *
* HistoClient -- Histo-Scope module called by users to communicate with the    *
*		 Histo-Scope process.  This module allows users to book, fill, *
*		 and display histograms, ntuples, and indicators.	       *
*									       *
*		 HistoClient uses XDR to translate all non-ASCII data so that  *
*		 data can be viewed by users on non-homogeneous systems.       *
*									       *
*     This module contains the routines called by the C and FORTRAN bindings   *
*     except for those routines added for Histo-Scope V3.0 in histoApi*.c.     *
*     The binding routine names start with hs_ and the HistoClient routine     *
*     names begin with histo_.  The following user-callable routines all have  *
*     histo_ equivalents in this module:				       *
*									       *
*     For all users:						               *
*									       *
*	 hs_initialize       - Initialize the Histo-Scope connection software  *
*			       and set up a potential connection to a Histo-   *
*			       Scope process.				       *
*	 hs_update	     - Updates the Histo-Scope display.		       *
*	 hs_complete	     - Closes all connections w/Histo-Scope processes. *
*	 hs_complete_and_wait- Waits until all Histo-Scopes finish scoping and *
*			       then performs an hs_complete.		       *
*	 hs_histoscope	     - Invokes Histo-Scope as a sub-process.           *
*									       *
*     For Histo-Scope item users:				               *
*									       *
*	 hs_create_1d_hist   - Books a one-dimensional histogram.	       *
*	 hs_create_2d_hist   - Books a two-dimensional histogram.	       *
*	 hs_create_ntuple    - Defines an n-tuple.  N-tuples have a specified  *
*			       number of variables and automatic storage       *
*			       allocated as they grow.			       *
*	 hs_create_indicator - Creates an indicator (a scalar value).          *
*	 hs_create_control   - Creates a control (a scalar value set by HS).   *
*	 hs_create_trigger   - Creates a trigger (an event set by HS).	       *
*	 hs_create_group     - Creates a group of histo-scope items.	       *
*	 hs_fill_1d_hist     - Adds a value to a one-dimensional histogram.    *
*	 hs_fill_2d_hist     - Adds a value to a two-dimensional histogram.    *
*	 hs_fill_ntuple      - Adds an array of real values to an n-tuple.     *
*	 hs_set_indicator    - Sets the value of an indicator.		       *
*	 hs_read_control     - Reads a control (a scalar value set by HS).     *
*	 hs_check_trigger    - Checks whether a trigger has been set by HS.    *
*	 hs_set_1d_errors    - Copies a vector of real numbers as error info   *
*	 hs_set_2d_errors    - Copies an array of real numbers as error info   *
*	 hs_reset	     - Resets all of the bins of a histogram to 0, or  *
*			       removes all of the data from an n-tuple, or     *
*			       sets an indicator to 0.			       *
*	 hs_delete	     - Deletes a histogram, n-tuple, or indicator.     *
*									       *
*     For HBOOK users:		(in HbookClient.c)		               *
*									       *
*	 hs_hbook_setup       - Sets up all HBOOK histograms and ntuples in a  *
*			        top directory for use with Histo-Scope.	       *
*	 hs_reset_hbook_setup - Call this routine if you have previously       *
*			        called hs_hbook_setup and have booked new      *
*			        histograms or ntuples, or deleted, renamed,    *
*			        rebinned, or resetted existing ones.	       *
*									       *
* Copyright (c) 1992, 1993, 1994 Universities Research Association, Inc.       *
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
* Modification History:							       *
*									       *
*	1/11/93 - JMK - Port to VMS					       *
*	7/12/93 - JMK - Change routine names from hs_ to histo_ because of VMS *
*	8/24/93 - JMK - Add control item type and errors for histograms        *
* 	12/7/93 - Paul Lebrun - added uid item & links to histoApi*.c          *
*     2002-2003 - igv - various fixes	      	                               *
*									       *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#ifdef VMS
#include <unixio.h>
#include <processes.h>
#include <file.h>
#include <perror.h>
#include <starlet.h>
#include <ssdef.h>
#include <climsgdef.h>	/* CLI status values */
#include <lnmdef.h>	/* Logical name flag definitions */
#include <descrip.h>	/* descriptor definitions */
#include <mulsys/types.h>
#include <mulsys/socket.h>
#include <mul/netdb.h>
#include <mulsys/ioctl.h>
#include <mul/errno.h>
#else /*VMS*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef VXWORKS
#include <vxWorks.h>
#include <ioLib.h>
#include <sockLib.h>
#include <in.h>
#include <timers.h>
#define socket_ioctl ioctl
#else
#include <sys/wait.h>
#include <netdb.h>
#endif /*VXWORKS*/
#define socket_perror perror
#define socket_read read
#define socket_write write
#define socket_errno errno
#define socket_close close
#endif /*VMS*/
#include <netinet/in.h>
#include <string.h>
#ifdef XDR_IBM_MALLOC_BUG
#define malloc mmalloc
#endif
#ifdef VMS	/* ask rpc/types to refrain from declaring malloc */
#define DONT_DECLARE_MALLOC
#endif /*VMS*/  
#include <rpc/types.h>
#ifdef XDR_IBM_MALLOC_BUG
#undef malloc
#define mem_alloc(bsize)	malloc(bsize)
#endif
#include <math.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#ifndef VXWORKS
#include <sys/time.h>
#endif /*VXWORKS*/
#include <rpc/xdr.h>
#define XDR_FILTERS_HERE 1
#include "../histo_util/hsTypes.h"
#include "../histo_util/xdrHisto.h"
#include "../histo_util/histoUtil.h"
#include "../histo_util/publish.h"
#include "histoApiItems.h"
#include "HistoClient.h"
#include "histoVersion.h"

#ifdef VMS
#define xdr_string hs_xdr_string
#define xdrmem_create hs_xdrmem_create
#define xdrstdio_create hs_xdrstdio_create
#define xdr_destroy hs_xdr_destroy
#define xdr_int hs_xdr_int
#define xdr_float hs_xdr_float
#define socket_close hs_socket_close
#define socket_read hs_socket_read
#define socket_write hs_socket_write
#define socket_perror hs_socket_perror
#define socket_errno hs_socket_errno()
#define socket_ioctl hs_socket_ioctl
#define getsockopt hs_getsockopt
#define setsockopt hs_setsockopt
#define accept hs_accept
#define socket hs_socket
#define bind hs_bind
#define getsockname hs_getsockname
#define listen hs_listen
#define ntohs hs_ntohs
#include "clientXDRsocket.h"
#ifdef LINKED_WITH_HBOOK
#define hnoent_ hnoent
#endif /* LINKED_WITH_HBOOK */
#endif /*VMS*/

#include "../histo_util/histprotocol.h"

#ifdef LINKED_WITH_HBOOK
#include "HbookClient.h"
#include "../histo_util/getHbookData.h"
#include "HistoClientHB.h"
extern void hnoent_(int *i1, int *i2);
#ifdef VMS
extern void hcdir(struct dsc$descriptor_s *dn, struct dsc$descriptor_s *dm);
static void hcdir_(char *dn, char *dm, int l1, int l2);
static void hcdirw_(char *dn, char *dm, int l1, int l2);
#else
extern void hcdir_(char *dn, char *dc, int l, int l2);
#define hcdirw_ hcdir_
#endif /* VMS */
#endif /* LINKED_WITH_HBOOK */

#define MAX_NUM_SCOPES 9	    /* max # of histo-scopes can connect to. 
                                       Must not exceed 32.  */
#define RUN_FAILURE -1
#define SOCKBUFSIZMIN 16384	    /* minimum buffersize for socket	      */
#define PROCESSED 0		    /* status for processing update requests  */
#define NOT_PROCESSED -1
#define PROCESSED_PARTIALLY 1
#define SIZE_HSIDLIST 250           /* space allocated for 250 hs_general 
							pointers at start..   */
							
struct updateList {		    /* Update List structure		      */
    int numItems;		    /* 	  number of items in list	      */
    int itemsProcessed;		    /* 	  number of items processed	      */
    int notFinished;		    /* 	  status of entire Update List	      */
    updStruct *updStructPtr;	    /* 	  ptr to array of item update reqs    */
    int *updStatPtr;		    /* 	  ptr to array of item update status  */
};

typedef struct _unfinMsgListHdr {   /* Unfinished message List structure      */
    struct _unfinMsgListHdr *next;  /* 	  pointer to next		      */
    char *msg;			    /* 	  pointer to message		      */
    int nBytes;			    /* 	  number of bytes in message buffer   */
    int byteOffset;		    /*    offset at which to start writing    */
    int dataId;                     /*    this is ntuple id in case the message
                                          is a data message. Needed so that
                                          the message can be removed when
                                          the ntuple is reset.                */
} unfinMsgListHdr;

typedef struct _procStat {	    /* Update processing status		      */
    int continueProcessing;
    int incItemsProcessed;
} procStat;


/* Function prototypes */
static int setUpComPort(int *portSock, int *portNum); /* creates acc. socket  */
static void acceptHCom(void);		  /* accepts pending scope conections */
static void processUpdates(int conid, int numOfItems, int bytesToRead);
					  /* processes HS requests for updates*/
static int sendItemList(int connect_id);  /* send item list to scope process  */
static int readHistoCom(int connect_id); /* reads socket for HistoScope msgs */
static void closeScope(int connect_id);	  /* closes connection to Histo-Scope */
static int sendToHS(int conid, int messageCode, int id, hsGeneral *item);
		    			  /* send message to Histo-Scope      */
static int sendHSData(int connect_id, msgStruct *message, hsGeneral *item);
		    			 /* send HS item data		      */
static int collectNtupleData(hsNTuple *nTuple);  /* collects extensions data  */
static int procUnfinUpds(void);	  /* process unfinished update requsts*/
static void freeUpdateList(int conid);    /* free update list malloc'd items  */
static procStat processUpdReq(int conid, int itemNum);
					  /* process individual update request*/
static void readNSaveReqs(int conid, int numOfItems, int bytesToRead);
static int rtnNforTuple(hsGeneral *hsG, int updateLevel);
static void freeUnfinMsgList(int connect_id);
static void addUnfinMsg(int conid, char *msg, int nBytes, int doMalloc,
		        int byteOffset, int dataId);
static void forgetUnfinMsg(int conid, int dataId);
static int needToUpdate(hsGeneral * hsG, updStruct *updStP, int conid);
static int writeUnfinMsg(int conid);
static int minInt(int a, int b);
static void reset1DHist(hs1DHist *item);
static void reset2DHist(hs2DHist *item);
static void reset3DHist(hs3DHist *item);
static void constreset1DHist(hs1DHist *item, float c);
static void constreset2DHist(hs2DHist *item, float c);
static void constreset3DHist(hs3DHist *item, float c);
static int slice2DContents(const char *name, hs2DHist *item, int axis_slice,
			   int bin_slice, int arrsize, float *data,
			   float *poserr, float *negerr, int *slicesize);
static int slice3DContents(const char *name, hs3DHist *item, int axis_slice,
			   int bin_slice, int arrsize, float *data,
			   float *poserr, float *negerr, int *slicesize);
static int stack3DContents(const char *name, hs3DHist *item, int x_bin,
                           int y_bin, int z_bin, int arrsize, float *data,
                           float *poserr, float *negerr, int *slicesize);
#ifdef LINKED_WITH_HBOOK
static int setupHbookReq(hsGeneral *hsG, updStruct *updStP);
#endif /* LINKED_WITH_HBOOK */
static void flushUpdates(void);		  /* give HS chance to req last update*/
static void waitHS(void);		  /* wait until all HS's disconnect   */
static int anyToWait(int *wFU);
static void hsComplete(void);
static void setControl(int conid, int itemID, int bytesToRead);
static void setTrigger(int conid, int itemID);
static void setHistErrsOn(int itemID, int conid);
#ifndef VXWORKS
#ifdef VMS
static int runHistoScope(char *cmd_str, int return_immediately, char *pub_str);
#else
static int runHistoScope(char *cmd_str);  /* does execl to run Histo-Scope    */
#endif /*VMS*/
#endif /*VXWORKS*/

/*  #define VERBOSE */

/* Global variables */
int InitializedAndActive = FALSE;
int NumOfItems = 0;
hsGeneral **HistoPtrList = NULL;
int AllowItemSend = 1;

/* Module global variables */
static int AccComFD = 0;
static int NumOfConnections = 0;
static int SocketBufferSize = SOCKBUFSIZMIN;
static int SocketIsBlocked = 0;
static int ComFD[MAX_NUM_SCOPES];
static int ScopeConfirmed[MAX_NUM_SCOPES];
static char PublishStr[MAX_ID_FILE_SIZE];
static int ServerPort = -1;
static struct updateList *UpdateList[MAX_NUM_SCOPES];
static unfinMsgListHdr *UnfinMsgList[MAX_NUM_SCOPES];
#ifdef COM_DEBUG
static int NumTriggersSet = 0;
static int NumTriggersRead = 0;
#endif /*COM_DEBUG*/

#ifdef LINKED_WITH_HBOOK
static int HBInitialized = FALSE;
#endif /* LINKED_WITH_HBOOK */

/* Task completion callback */
static void (*task_completion_callback)(int, int, int, char *, void *) = NULL;
static void *task_completion_callback_data = NULL;

void histo_initialize(const char *id_string)
/*
* histo_initialize() is called by the Histo-Scope bindings to initialize the
* Histo-Scope connection.  It creates a socket that Histo-Scope processes can
* connect to and advertises it in the Histo-Scope directory.  hs_initialize()
* should be called before any of the other Histo-Scope communication routines.  
*/

{
    int stat, commPortNum;
#ifndef VMS
    char ident[] = "@(#)Histo-Scope API	Version " HISTOSCOPE_VERSION "  " __DATE__;
#endif /*VMS*/

    if (InitializedAndActive)
    	return;				     /* already called and active     */

    NumOfConnections = 0;		     /* number of scoping processes   */
    NumOfItems = 0;			     /* number of histo items created */
    AccComFD = 0;			     /* accepting socket File Descr.  */
    for (stat = 0; stat < MAX_NUM_SCOPES; stat++) {
    	ComFD[stat] = NO_CONNECTION;	     /* initialize socket file desc   */
    	ScopeConfirmed[stat] = FALSE;	     /* 	   and confirm status */
    	UpdateList[stat] = NULL;	     /*		   and list headers   */
    	UnfinMsgList[stat] = NULL;	     /*		   and list headers   */
    }
    stat = setUpComPort (&AccComFD, &commPortNum); /* set up accepting socket */
    if (stat != COM_OK) {
    	fprintf(stderr,
       "hs_initialize: Cannot set up socket for communication with Histoscope");
    	InitializedAndActive = FALSE;
    	return;
    }
    ServerPort = commPortNum;

#ifndef VXWORKS
    CleanupOldIDFiles();	     /* delete any old IDFiles hanging about */
#endif /*VXWORKS*/

    strcpy(PublishStr, "\'");
    CreateIDFile(commPortNum, id_string, (char *) (PublishStr + 1));
    					     /* advertise & go public         */
    strcat(PublishStr, "\'");
#ifndef VMS
    printf("%s - Initialized -\n", ident);
#endif /*VMS*/
#ifdef COM_DEBUG
    printf("PublishStr = %s\n", PublishStr);
#endif /*COM_DEBUG*/
    InitializedAndActive = TRUE;
    xdr_SetClientVer(V4_CLIENT);
    
    signal(SIGPIPE, SIG_IGN);		     /* prevent broken pipe aborts    */
    
    HistoPtrList = (hsGeneral **) malloc(sizeof(hsGeneral *)*SIZE_HSIDLIST); 
    			/* allocate the memory for hs_General List pointer. */
}

int histo_socket_status(void)
{
    return SocketIsBlocked;
}

int histo_server_port(void)
{
    return ServerPort;
}

void histo_update(void)
/*
* hs_update() is called by the Histo-Scope client process periodically to
* update the Histo-Scope display.  hs_update() can be called as often as you
* like with no adverse impact on CPU usage.  It will only transmit the data
* for the histograms that are currently displayed by Histo-Scope, and only
* as frequently as requested by the user of the HistoScope program, regardless
* of how often you call hs_update().
*
* The Histo-Scope process sends requests for the displayed histograms, n-tuples,
* and indicators that it needs updated, but doesn't expect an answer on any
* kind of regular schedule.  The requests remain queued in the input buffer of
* the user process until the user's program calls the update routine.
*
* The update routine re-sends histograms in their entirety each time they are
* requested.  For n-tuples, the update routine sends only the new data which
* has not been sent before.  After sending all of the requested data to the
* Histo-Scope process, the update routine resets its timer and returns control
* to the user's code.
*/

{
    int i;
    
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    	
    acceptHCom();			/* accept any HistoScope connections  */
    procUnfinUpds();			/* process any unfinished updates     */
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
	readHistoCom(i);		/* read & respond to msgs on socket   */
}

void histo_complete(void)
/*
* hs_complete() is called by the Histo-Scope client process after it has no
* more need for connections to Histo-Scope processes.  The routine will close
* all connections to Histo-Scope processes.
*/

{
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    if (NumOfConnections > 0)
    	flushUpdates();			/* give HSs chance to req last update */

    hsComplete();
}

void histo_complete_and_wait(void)
/*
* hs_complete_and_wait() is called by the Histo-Scope client process after 
* data computation is finished to allow any connected Histo-Scopes to keep
* scoping as long as any Histo-Scope user wishes to look at the data.  If
* no Histo-Scopes are connected, hs_complete_and_wait just does a hs_complete.
* Note that this routine does not return to the caller until ALL Histo-Scopes
* have closed the connection with the client (or exitted).  No time-out
* period can be set.
*/

{
    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    if (NumOfConnections > 0)
    	waitHS();			/* wait until all HS's disconnect     */

    hsComplete();
}

#ifndef VXWORKS
void histo_histoscope(int return_immediately, const char *configFile, int hidden)
/*
* hs_histoscope() is called by users who want to build Histo-Scope into their
* program rather than using it separately as an inspection tool.  hs_histoscope
* invokes Histo-Scope as a sub-process.  This scope process is pre-connected
* to display the data generated in the user process.  The routine can either
* start the Histo-Scope and return immediately (return_immediately tests True),
* or it can start Histo-Scope and return after the user closes the Histo-Scope
* window (return_immediately tests False, i.e == 0).
*/
{
    char cmd[1024];
    int child_pid;
    
    if (!InitializedAndActive)
    {
    	fprintf(stderr,
    		"Error: can't start Histo-Scope before hs_initialize.\n");
    	return;			      /* hs_initialize must be called first */
    }

    memset(cmd, 0, sizeof(cmd));
#ifdef COM_DEBUG
    printf("starting Histo-Scope...\n");
#endif
#ifdef VMS
    strcpy(cmd, "HISTO_DIR:HISTO");
    child_pid = runHistoScope(cmd, return_immediately, PublishStr);
#else
    strcpy(cmd, "setenv HISTOCONNECT ");
    strcat(cmd, PublishStr);
    strcat(cmd, "; $HISTO_DIR/bin/histo");
    if (hidden)
	strcat(cmd, " -xrm \"histo.mappedWhenManaged: false\"");
    if (configFile != NULL && configFile[0] != '\0') {
    	strcat(cmd, " -config ");
    	strcat(cmd, configFile);
    }
    /* strcat(cmd, "\n"); */
    child_pid = runHistoScope(cmd);
#endif /*VMS*/
    if (child_pid == RUN_FAILURE) {
    	perror("Error starting Histoscope");
    	return;
    }
#ifndef VMS
    if (return_immediately == 0)
    	waitpid(child_pid, NULL, 0);
#endif /*VMS*/
}

void histo_task_completion_callback(void (*f)(int, int, int, char *, void *), void *cbdata)
{
    task_completion_callback = f;
    task_completion_callback_data = cbdata;
}

void histo_kill_histoscope(void)
/*
* hs_kill_histoscope will any Histo-Scope process(es) started by
*                    calling hs_histoscope.
*/

{
    int i;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
            sendToHS(i, REQ_CAPTIVE_HISTO_EXIT, 0, NULL);
}
#endif /*VXWORKS*/

int histo_create_1d_hist(int uid, const char *title, const char *category, 
                         const char *x_label, const char *y_label,
                         int n_bins, float min, float max)
/*
* histo_create_1d_hist() books a one-dimensional histogram.
*
* parameters:   uid      - User Histogram identifier
*		title    - the title for the histogram window
*		category - an optional string defining a hierarchical "location"
*		           for the histogram.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the histogram will appear in
*		           the top level category.
*		x_label, y_label - label for histogram axis (x=horiz, y=vert.)
*		n_bins   - number of bins in the histogram
*		min      - low edge of first bin
*		max      - high edge of last bin
*
* returns:      histogram id to be used for subsequent calls for this histogram.
*/

{
    hs1DHist *hist;
    hsC1DHist *histC;
    int i;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_1d_hist: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_1d_hist", FALSE))
    	return 0;
    if (n_bins <= 0) {
    	fprintf(stderr,
    	    "hs_create_1d_hist: Error - invalid number of bins.\n");
    	return 0;
    }
    if (min == max) {			/* ensure min is != max           */
    	fprintf(stderr,
    	    "hs_create_1d_hist: Error - minimum specified is = maximum.\n");
    	return 0;
    }
    /* Check that this item characterized by uid/Category has not already been 
    			created. If so, do not create a new one */
    			
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
        "hs_create_1d_hist:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Histogram not created.\n");
        return 0;
     }
    
    histC              =  (hsC1DHist *) malloc(sizeof(hsC1DHist));
    if (histC == NULL) {
    	fprintf(stderr,
    		"hs_create_1d_hist: Malloc failed. Histogram not created.\n");
    	return 0;
    }
    histC->sendErrs    = FALSE;
    histC->resetFlag   = 0;
    hist               = (hs1DHist *) histC;
    hist->title        = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH, 
    				"hs_create_1d_hist", "title"));
    if (category == NULL)
      hist->category = CopyNtrim(ValidStr(category, HS_MAX_CATEGORY_LENGTH, 
    				"hs_create_1d_hist", "category"));
    else {
	hist->category = CopyNtrim(category);
    }
    hist->type         = HS_1D_HISTOGRAM;
    ++NumOfItems;
    hist->id           = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)hist);
     					   
    hist->uid          = uid;
    hist->hbookID      = 0;
    hist->count        = 0;
    hist->nBins        = n_bins;
    hist->overflow     = 0.;
    hist->underflow    = 0.;
    hist->xScaleType   = HS_LINEAR;
    hist->xScaleBase   = 10.;
    hist->errFlg       = FALSE;
    hist->pErrs        = NULL;
    hist->mErrs        = NULL;
    hist->xLabel       = CopyString(ValidStr(x_label, HS_MAX_LABEL_LENGTH, 
    				"hs_create_1d_hist", "x_label"));
    hist->yLabel       = CopyString(ValidStr(y_label, HS_MAX_LABEL_LENGTH, 
    				"hs_create_1d_hist", "y_label"));
    if (min > max) {			 /* ensure min is <= max              */
    	fprintf(stderr,
    	    "hs_create_1d_hist: Warning - minimum specified is > maximum.\n\
                   Values switched: min = %f, max = %f.\n", max, min);
    	hist->min = max;
    	hist->max = min;;
    }
    else {
	hist->min = min;
	hist->max = max;
    }
    hist->bins = (float *) malloc(sizeof(float) * n_bins);
    if (hist->bins == NULL) {
    	free(hist->title);
    	free(hist->category);
    	free(hist->xLabel);
    	free(hist->yLabel);
    	free(hist);
    	return 0;
    }
    for (i = 0; i < n_bins; ++i)
    	hist->bins[i] = 0.f;
    SendNewItem((hsGeneral *)hist);      /* inform connected HS's of new item */
    return (hist->id);
}

int histo_create_2d_hist(int uid, const char *title, const char *category,
                         const char *x_label, const char *y_label,
                         const char *z_label, int x_bins, int y_bins, 
                         float x_min, float x_max, float y_min, float y_max)
/*
* histo_create_2d_hist() books a two-dimensional histogram.
*
* parameters:   uid      - User Histogram identifier
*		title    - the title for the histogram window
*		category - an optional string defining a hierarchical "location"
*		           for the histogram.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the histogram will appear in
*		           the top level category.
*		x_label, y_label - label for histogram axis (x=horiz, y=vert.)
*		z_label  - label for histogram z axis
*		x_bins, y_bins - number of bins in that axis of the histogram
*		x_min    - low edge of first x-axis bin
*		x_max    - high edge of last x-axis bin
*		y_min, y_max - low/high edge of y-axis bin
*
* returns:      histogram id to be used for subsequent calls for this histogram.
*/

{
    hs2DHist *hist;
    hsC2DHist *histC;
    int i, j;
    int n_bins = x_bins * y_bins;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_2d_hist: Error called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_2d_hist", FALSE))
    	return 0;

    if (x_bins <= 0) {
    	fprintf(stderr,
    	    "hs_create_2d_hist: Error - invalid number of x bins.\n");
    	return 0;
    }
    if (y_bins <= 0) {
    	fprintf(stderr,
    	    "hs_create_2d_hist: Error - invalid number of y bins.\n");
    	return 0;
    }

    if (x_min == x_max) {		/* ensure x_min is != x_max           */
    	fprintf(stderr,
    	    "hs_create_2d_hist: Error - x minimum specified is = maximum.\n");
    	return 0;
    }
    
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
        "hs_create_2d_hist:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Histogram not created.\n");
        return 0;
    }
     
    if (y_min == y_max) {		/* ensure y_min is != x_max           */
    	fprintf(stderr,
    	    "hs_create_2d_hist: Error - y minimum specified is = maximum.\n");
    	return 0;
    }
    histC              = (hsC2DHist *) malloc(sizeof(hsC2DHist));
    if (histC == NULL){
    	fprintf(stderr,
    		"hs_create_2d_hist: Malloc failed. Histogram not created.\n");
    	return 0;
    }
    histC->sendErrs    = FALSE;
    histC->resetFlag   = 0;
    hist               = (hs2DHist *) histC;
    hist->title        = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_2d_hist", "title"));
    if (category == NULL)
     hist->category = CopyNtrim(ValidStr(category, HS_MAX_CATEGORY_LENGTH,
    					"hs_create_2d_hist", "category"));
    else {
	hist->category = CopyNtrim(category);
    }
    hist->type         = HS_2D_HISTOGRAM;
    ++NumOfItems;
    hist->id           = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)hist);
    hist->uid          = uid;
    hist->hbookID      = 0;
    hist->count        = 0;
    hist->nXBins       = x_bins;
    hist->nYBins       = y_bins;
    hist->xScaleType   = HS_LINEAR;
    hist->xScaleBase   = 10.;
    hist->yScaleType   = HS_LINEAR;
    hist->yScaleBase   = 10.;
    hist->errFlg       = FALSE;
    hist->pErrs        = NULL;
    hist->mErrs        = NULL;
    hist->xLabel       = CopyString(ValidStr(x_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_2d_hist", "x_label"));
    hist->yLabel       = CopyString(ValidStr(y_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_2d_hist", "y_label"));
    hist->zLabel       = CopyString(ValidStr(z_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_2d_hist", "z_label"));
    if (x_min > x_max) {		/* ensure x_min is <= x_max           */
    	fprintf(stderr,
    	    "hs_create_2d_hist: Warning - x minimum specified is > maximum.\n\
                   Values switched: x_min = %f, x_max = %f.\n", x_max, x_min);
    	hist->xMin = x_max;
    	hist->xMax = x_min;;
    }
    else {
	hist->xMin = x_min;
	hist->xMax = x_max;
    }
    if (y_min > y_max) {		/* ensure y_min is <= y_max           */
    	fprintf(stderr,
    	    "hs_create_2d_hist: Warning - y minimum specified is > maximum.\n\
                   Values switched: y_min = %f, y_max = %f.\n", y_max, y_min);
    	hist->yMin = y_max;
    	hist->yMax = y_min;;
    }
    else {
	hist->yMin = y_min;
	hist->yMax = y_max;
    }
    hist->bins = (float *) malloc(sizeof(float) * n_bins);
    if (hist->bins == NULL) {
    	free(hist->title);
    	free(hist->category);
    	free(hist->xLabel);
    	free(hist->yLabel);
    	free(hist->zLabel);
    	free(hist);
    	return 0;
    }
    for (i = 0; i < n_bins; ++i)
    	hist->bins[i] = 0.f;
    for (i = 0; i < 3; ++i)
	for (j = 0; j < 3; ++j)
	    hist->overflow[i][j] = 0.f;
    SendNewItem((hsGeneral *)hist);      /* inform connected HS's of new item */
    return (hist->id);
}

int histo_create_3d_hist(int uid, const char *title, const char *category,
                         const char *x_label, const char *y_label,
                         const char *z_label, const char *v_label,
			 int x_bins, int y_bins, int z_bins, float x_min,
			 float x_max, float y_min, float y_max,
                         float z_min, float z_max)
{
/*
 * histo_create_3_hist() books a three-dimensional histogram.
 */
    hs3DHist *hist;
    hsC3DHist *histC;
    int i, j, k;
    int n_bins = x_bins * y_bins * z_bins;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_3d_hist: Error called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_3d_hist", FALSE))
    	return 0;

    if (x_bins <= 0) {
    	fprintf(stderr,
    	    "hs_create_3d_hist: Error - invalid number of x bins.\n");
    	return 0;
    }
    if (y_bins <= 0) {
    	fprintf(stderr,
    	    "hs_create_3d_hist: Error - invalid number of y bins.\n");
    	return 0;
    }
    if (z_bins <= 0) {
    	fprintf(stderr,
    	    "hs_create_3d_hist: Error - invalid number of z bins.\n");
    	return 0;
    }

    if (x_min == x_max) {		/* ensure x_min is != x_max           */
    	fprintf(stderr,
    	    "hs_create_3d_hist: Error - x minimum specified is = maximum.\n");
    	return 0;
    }
    
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
        "hs_create_3d_hist:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Histogram not created.\n");
        return 0;
    }
     
    if (y_min == y_max) {		/* ensure y_min is != x_max           */
    	fprintf(stderr,
    	    "hs_create_3d_hist: Error - y minimum specified is = maximum.\n");
    	return 0;
    }
    if (z_min == z_max) {		/* ensure z_min is != z_max           */
    	fprintf(stderr,
    	    "hs_create_3d_hist: Error - z minimum specified is = maximum.\n");
    	return 0;
    }
    histC              = (hsC3DHist *) malloc(sizeof(hsC3DHist));
    if (histC == NULL){
    	fprintf(stderr,
    		"hs_create_3d_hist: Malloc failed. Histogram not created.\n");
    	return 0;
    }
    histC->sendErrs    = FALSE;
    histC->resetFlag   = 0;
    hist               = (hs3DHist *) histC;
    hist->title        = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_3d_hist", "title"));
    if (category == NULL)
     hist->category = CopyNtrim(ValidStr(category, HS_MAX_CATEGORY_LENGTH,
    					"hs_create_3d_hist", "category"));
    else {
	hist->category = CopyNtrim(category);
    }
    hist->type         = HS_3D_HISTOGRAM;
    ++NumOfItems;
    hist->id           = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)hist);
    hist->uid          = uid;
    hist->hbookID      = 0;
    hist->count        = 0;
    hist->nXBins       = x_bins;
    hist->nYBins       = y_bins;
    hist->nZBins       = z_bins;
    hist->xScaleType   = HS_LINEAR;
    hist->xScaleBase   = 10.;
    hist->yScaleType   = HS_LINEAR;
    hist->yScaleBase   = 10.;
    hist->zScaleType   = HS_LINEAR;
    hist->zScaleBase   = 10.;
    hist->errFlg       = FALSE;
    hist->pErrs        = NULL;
    hist->mErrs        = NULL;
    hist->xLabel       = CopyString(ValidStr(x_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_3d_hist", "x_label"));
    hist->yLabel       = CopyString(ValidStr(y_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_3d_hist", "y_label"));
    hist->zLabel       = CopyString(ValidStr(z_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_3d_hist", "z_label"));
    hist->vLabel       = CopyString(ValidStr(v_label, HS_MAX_LABEL_LENGTH,
    					"hs_create_3d_hist", "v_label"));
    if (x_min > x_max) {		/* ensure x_min is <= x_max           */
    	fprintf(stderr,
    	    "hs_create_3d_hist: Warning - x minimum specified is > maximum.\n\
                   Values switched: x_min = %f, x_max = %f.\n", x_max, x_min);
    	hist->xMin = x_max;
    	hist->xMax = x_min;;
    }
    else {
	hist->xMin = x_min;
	hist->xMax = x_max;
    }
    if (y_min > y_max) {		/* ensure y_min is <= y_max           */
    	fprintf(stderr,
    	    "hs_create_3d_hist: Warning - y minimum specified is > maximum.\n\
                   Values switched: y_min = %f, y_max = %f.\n", y_max, y_min);
    	hist->yMin = y_max;
    	hist->yMax = y_min;;
    }
    else {
	hist->yMin = y_min;
	hist->yMax = y_max;
    }
    if (z_min > z_max) {		/* ensure z_min is <= z_max           */
    	fprintf(stderr,
    	    "hs_create_3d_hist: Warning - z minimum specified is > maximum.\n\
                   Values switched: z_min = %f, z_max = %f.\n", z_max, z_min);
    	hist->zMin = z_max;
    	hist->zMax = z_min;;
    }
    else {
	hist->zMin = z_min;
	hist->zMax = z_max;
    }
    hist->bins = (float *) malloc(sizeof(float) * n_bins);
    if (hist->bins == NULL) {
    	free(hist->title);
    	free(hist->category);
    	free(hist->xLabel);
    	free(hist->yLabel);
    	free(hist->zLabel);
    	free(hist->vLabel);
    	free(hist);
    	return 0;
    }
    for (i = 0; i < n_bins; ++i)
    	hist->bins[i] = 0.f;
    for (i = 0; i < 3; ++i)
	for (j = 0; j < 3; ++j)
	    for (k = 0; k < 3; ++k)
		hist->overflow[i][j][k] = 0.f;
    SendNewItem((hsGeneral *)hist);      /* inform connected HS's of new item */
    return (hist->id);
}

int histo_create_ntuple(int uid, const char *title, const char *category, 
			int n_variables, char **names)
/*
* histo_create_ntuple() defines an n-tuple with n_variables variables with variable
* names taken from the names array.  Storage allocation is automatic; as the
* n-tuple grows, more space will be allocated.
*
* parameters:   uid      - User Histogram identifier
*		title    - the title for the histogram window
*		category - an optional string defining a hierarchical "location"
*		           for the n-tuple.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the n-tuple will appear in
*		           the top level category.
*		n_variables - the number of variables in the n-tuple
*		names     - array of pointers to null-terminated strings for
*			    the variable names
*
* returns:      n-tuple id to be used for subsequent calls for this n-tuple.
*/

{
    hsCNTuple *nTC;
    hsNTuple *nT;
    int i;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_ntuple: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_ntuple", FALSE))
    	return 0;
    	
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
         "hs_create_ntuple:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Ntuple not created.\n");
        return 0;
    }
     
    nTC              =  (hsCNTuple *) malloc(sizeof(hsCNTuple));
    if (nTC == NULL)
    	return 0;
    nT               = &(nTC->nTuple);
    nT->title        = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_ntuple", "title"));
    if (category == NULL)		 /* if null pointer, make null string */
     nT->category = CopyNtrim(ValidStr(category, HS_MAX_CATEGORY_LENGTH,
    					"hs_create_ntuple", "title"));
    else {
	nT->category = CopyNtrim(category);
    }
    nT->type         = HS_NTUPLE;
    ++NumOfItems;
    nT->id           = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)nT);
    nT->uid          = uid;
    nT->hbookID      = 0;
    nT->nVariables   = n_variables;
    nT->n            = 0;		/* number of elements		      */
    nT->names        = (char **) malloc(sizeof(char *) * n_variables);
    if (nT->names == NULL) {
    	free(nT->title);
    	free(nT->category);
    	free(nT);
    	return 0;
    }
    for (i = 0; i < n_variables; ++i)
    	nT->names[i] = CopyNtrim(ValidStr(names[i], HS_MAX_NAME_LENGTH,
    				    "hs_create_ntuple", "ntuple name"));
    nT->chunkSzData  = 0;		/* will be zero until extensions full */
    nT->chunkSzExt   = 10;		/* starts at 10; doubled when all full*/
    nT->data         = NULL;
    for (i = 0; i < 4; ++i)
    	nT->extensions[i] = NULL; 
    SendNewItem((hsGeneral *) nT);      /* inform connected HS's of new item  */
    nTC->fromTuple = 0;
    nTC->toTuple = -1;
    nTC->inhibitResetRefresh = 0;
    nTC->needsCompleteUpdate = 0xffffffff;
    return (nT->id);
}

int histo_create_indicator(int uid, const char *title, const char *category, 
                           float min, float max)
/*
* histo_create_indicator() creates an indicator.
*
* parameters:   uid      - User Histogram identifier
*		title    - the title for the histogram window
*		category - an optional string defining a hierarchical "location"
*		           for the indicator.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the indicator will appear in
*		           the top level category.
*		min	 - minimum value allowed for the indicator.
*		max	 - maximum value allowed for the indicator.
*
* returns:    indicator id to be used for subsequent calls for this indicator.
*/
{
    hsIndicator *indicator;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_indicator: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_indicator", FALSE))
    	return 0;
    	
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
      "hs_create_indicator:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Indicator not created.\n");
     return 0;
     }
     
    indicator = (hsIndicator *) malloc(sizeof(hsIndicator));
    if (indicator == NULL)
    	return 0;
    indicator->title    = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_indicator", "title"));
    if (category == NULL)
         indicator->category = 
                     CopyNtrim(ValidStr(category,HS_MAX_CATEGORY_LENGTH,
    					"hs_create_indicator", "category"));
    else {
	indicator->category = CopyNtrim(category);
    }
    indicator->type     = HS_INDICATOR;
    ++NumOfItems;
    indicator->id       = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)indicator);
    indicator->uid      = uid;
    indicator->hbookID  = 0;
    if (min > max) {			/* ensure min is < max  */
    	fprintf(stderr,
    	    "hs_create_indicator: Warning - minimum specified is > maximum.\n\
                     Values switched: min = %f, max = %f.\n", max, min);
    	indicator->min = max;
    	indicator->max = min;
    }
    else if (min == max) {		/* ensure min is != max */
    	fprintf(stderr,
    	    "hs_create_indicator: Warning - minimum specified is = maximum.\n\
                     Old max = %f, New max = %f.\n", max, max+1.);
    	indicator->min = min;
    	indicator->max = max+1.;
    }
    else {
	indicator->min = min;
	indicator->max = max;
    }
    indicator->value = indicator->min;	 /* initialize value to min (for XDR) */
    indicator->valueSet = VALUE_NOT_SET; /* indicate value not yet set        */
    SendNewItem((hsGeneral *)indicator); /* inform connected HS's of new item */
    return (indicator->id);
}

int histo_create_control(int uid, const char *title, const char *category, 
		         float min, float max, float default_value)
/*
* histo_create_control() creates a control, which is settable by the user of
*			 Histo-Scope and readable by client process.
*
* parameters:   uid      - User Histogram identifier
*		title    - the title for the histogram window
*		category - an optional string defining a hierarchical "location"
*		           for the control.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the control will appear in
*		           the top level category.
*		min	 - minimum value allowed for the control.
*		max	 - maximum value allowed for the control.
*
* returns:    control id to be used for subsequent calls for this control.
*/
{
    hsControl *ctrl;
    hsCControl *ctrlC;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_control: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_control", FALSE))
    	return 0;
    	
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
         "hs_create_control:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Control not created.\n");
        return 0;
    }
     
    ctrlC = (hsCControl *) malloc(sizeof(hsCControl));
    if (ctrlC == NULL)
    	return 0;
    ctrl = (hsControl *) ctrlC;
    ctrl->title    = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_control", "title"));
    if (category == NULL)
       ctrl->category = CopyNtrim(ValidStr(category,HS_MAX_CATEGORY_LENGTH,
    					"hs_create_control", "category"));
    else {
	ctrl->category = CopyNtrim(category);
    }
    ctrl->type     = HS_CONTROL;
    ++NumOfItems;
    ctrl->id       = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)ctrl);
    ctrl->uid   = uid;
    ctrl->hbookID  = 0;
    if (min > max) {			/* ensure min is <= max  */
    	fprintf(stderr,
    	    "hs_create_control: Warning - minimum specified is > maximum.\n\
                     Values switched: min = %f, max = %f.\n", max, min);
    	ctrl->min = max;
    	ctrl->max = min;;
    }
    else {
	ctrl->min = min;
	ctrl->max = max;
    }
    ctrl->value = default_value;	 /* initialize value to default_value */
    ctrlC->valueSet = 0;		 /* indicate value not yet set by HS  */
    ctrlC->newValue = default_value;	 /* init. new value to default_value  */
    ctrlC->defaultValue = default_value; /* save default value for a reset    */
    SendNewItem((hsGeneral *)ctrl);	 /* inform connected HS's of new item */
    return (ctrl->id);
}


int histo_create_trigger(int uid, const char *title, const char *category)
/*
* histo_create_trigger() creates a trigger, which is an item that can register
*			 an event set by the user of Histo-Scope and be
*			 checked by the client process.
*
* parameters:   uid      - User Histogram identifier
*		title    - the title for the histogram window
*		category - an optional string defining a hierarchical "location"
*		           for the trigger.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the trigger will appear in
*		           the top level category.
*
* returns:    trigger id to be used for subsequent calls for this trigger.
*/
{
    hsTrigger *trigger;
    hsCTrigger *triggerC;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_trigger: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_trigger", FALSE))
    	return 0;
    	
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
         "hs_create_trigger:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Trigger not created.\n");
        return 0;
    }
     
    triggerC = (hsCTrigger *) malloc(sizeof(hsCTrigger));
    if (triggerC == NULL)
    	return 0;
    trigger = (hsTrigger *) triggerC;
    trigger->title    = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_trigger", "title"));
    if (category == NULL)
     trigger->category = CopyNtrim(ValidStr(category,HS_MAX_CATEGORY_LENGTH,
    					"hs_create_trigger", "category"));
    else {
     trigger->category = CopyNtrim(category);
    }
    trigger->type     = HS_TRIGGER;
    ++NumOfItems;
    trigger->id       = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)trigger);
    trigger->uid      = uid;
    trigger->hbookID  = 0;
    triggerC->numTriggered = 0;		 /* indicate no events yet set by HS  */
    SendNewItem((hsGeneral *)trigger);	 /* inform connected HS's of new item */
    return (trigger->id);
}

/*
* A group can be made out of a number of items in order to display them
* as a Multiple Plot Window, an Overlaid Plot, or simply as a convenience
* for putting plots up individually with fewer mouse clicks.  Data items
* may be put into more than one group and there is no limit on the number
* prof groups you can create.  Including an item in a group does not affect
* the display of the item individually in the main panel.  The item still
* appears in the main panel list box and can be viewed individually.  The
* category string can be used to affect how Histo-Scope items appear in
* the main panel.
*
* When a group is selected in Histo-Scope's main window and the user
* presses  the "View" Button, the groupType field tells Histo-Scope how
* to display the items in the group.  This can be overridden in the
* Histo-Scope main panel by using the "View Multiple" or "View Overlaid"
* buttons.  If groupType is HS_MULTI_PLOT, pressing the View button
* displays all the 1- and 2-d histograms in the group as one Multi-Plot
* window.  Similarly, if you would like to see all the 1d histograms in a
* group displayed over each other in one plot window, you can specify a
* groupType of HS_OVERLAY_PLOT.
*
* Specifying HS_INDIVIDUAL for groupType will suggest a default group type
* of individual.  This means that if a group is selected, and the "View"
* button is pressed on the Histo-Scope main panel, each data item in the
* group will be viewed individually.  In the case of Ntuples, the Ntuple
* panel will be displayed so that the user can choose the variables to
* plot and the type of plot to use.  Ntuples, triggers, controls, and
* indicators cannot appear as part of a Multiple Plot Window nor an
* Overlaid Plot window.  A group type of HS_INDIVIDUAL should be used for
* groups that include these data types and the "View" button should be
* pressed in the Histo-Scope main panel.
*/
int histo_create_group(int uid, const char *title, const char *category,
                       int groupType,
                       int numItems, int *itemId, int *errsDisp)
/*
* histo_create_group() creates a group, which is an item that points to
*			 other data items, thus grouping them into one
*			 addressable entity.
*
* parameters:   uid      - User identifier
*		title    - the title for the group window
*		category - an optional string defining a hierarchical "location"
*		           for the group.  Subcategories can be specified
*		           using the "/" character as in Unix file specifica-
*		           tions.  If a category or subcategory does not exist,
*		           it is created.  If the category argument is missing
*		           (either "", or NULL) the group will appear in
*		           the top level category.
*		groupType- group's suggested window type (HS_MULTI_PLOT or  
*	       		   HS_OVERLAY_PLOT or HS_INDIVIDUAL).
*		numItems - the number of items in this group
*	       		   should not be > HS_MAX_NUM_GROUP_ITEMS (=81).
*		itemId   - an array of HS item ids that compose the group
*		errsDisp - an array of values (NO_ERROR_BARS, DATA_ERROR_BARS,
*	       		   GAUSSIAN_ERROR_BARS) that corresponds to the
*	       		   itemID array that tells Histo-Scope whether/how
*	       		   to display errors for the corresponding histogram.
*
* returns:    group id to be used for subsequent calls for this group.
*/
{
    hsGroup *group;
    int i, j;
    hsGeneral *item;
    /** from plotWindows.h: **/
    enum errorBars {NO_ERROR_BARS, DATA_ERROR_BARS, GAUSSIAN_ERROR_BARS};
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_create_group: Error, called before hs_initialize.\n");
    	return 0;			/* hs_initialize must be called first */
    }
    if (!CheckValidCategory(category, "hs_create_group", FALSE))
    	return 0;
    	
    if (histo_id(uid, category) != -1) {
        fprintf(stderr,
         "hs_create_group:  An item with this uid/Category already exists.\n");
        fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
        fprintf(stderr, " Group not created.\n");
        return 0;
    }
    if (groupType != HS_MULTI_PLOT && groupType != HS_OVERLAY_PLOT
    		&& groupType != HS_INDIVIDUAL) {
    	fprintf(stderr,
            "hs_create_group:  Invalid group type.  Group not created.\n");
        return 0;
    }
    if (numItems <= 0 || numItems > HS_MAX_NUM_GROUP_ITEMS) {
    	fprintf(stderr,
            "hs_create_group:  Invalid number of items.  Group not created.\n");
        return 0;
    }
     
    group = (hsGroup *) malloc(sizeof(hsGroup));
    if (group == NULL) {
    	fprintf(stderr,
            "hs_create_group:  No more virtual memory.  Group not created.\n");
    	return 0;
    }
    group->title    = CopyNtrim(ValidStr(title, HS_MAX_TITLE_LENGTH,
    					"hs_create_group", "title"));
    if (category == NULL)
	group->category = CopyNtrim(ValidStr(category,HS_MAX_CATEGORY_LENGTH,
    					"hs_create_group", "category"));
    else
	group->category = CopyNtrim(category);
    group->type      = HS_GROUP;
    ++NumOfItems;
    group->id        = NumOfItems;
    AddItemToHsIdList( (hsGeneral *)group);
    group->uid       = uid;
    group->hbookID   = 0;
    group->groupType = groupType;
    group->itemId    = malloc(sizeof(int) * numItems);
    group->errsDisp  = malloc(sizeof(int) * numItems);
    if (group->itemId == NULL || group->errsDisp == NULL) {
    	fprintf(stderr,
            "hs_create_group:  Ran out of memory.  Group not created.\n");
        free(group);
        return 0;
    }
    
    /* Go through list of histo-scope items to add to group */
    for (i = 0, j = 0; i < numItems; ++i) {
    
    	/* Ensure histo-ids given are valid */
    	if (itemId[i] < NumOfItems && itemId[i] > 0
    		&& ( item = GetItemByPtrID(itemId[i]) ) != NULL 
    		&& item->type != HS_GROUP) {
    	    if (errsDisp[i] == NO_ERROR_BARS || errsDisp[i] == DATA_ERROR_BARS
    	    	    || errsDisp[i] == GAUSSIAN_ERROR_BARS)
    	        group->errsDisp[j] = errsDisp[i];
    	    else
    	        group->errsDisp[j] = NO_ERROR_BARS;
    	    group->itemId[j++] = itemId[i];
    	}
    	
    	/* Invalid item id.  Don't add to group. */
    	else {
    	    fprintf(stderr,
   "hs_create_group:  Id %d invalid or is a group.  Item not added to group.\n",
               itemId[i]);
    	}
    }
    
    /* Make number of items reflect true number added */
    if (j <= 0) {
    	fprintf(stderr,
            "hs_create_group:  No valid id's.  Group not created.\n");
        free(group->itemId);
        free(group->errsDisp);
        free(group);
        return 0;
    }
    group->numItems  = j;
    SendNewItem((hsGeneral *)group);	 /* inform connected HS's of new item */
    return (group->id);
}

void histo_fill_1d_hist(int id, float x, float weight)
/*
* histo_fill_1d_hist() adds a value to the a one-dimensional histogram.
*
* parameters:	id       - histogram id (returned by hs_create_1d_hist)
*		x	 - axis value used to find the bin to increment
*		weight   - value added to bin
*/

{
    int bin;
    hs1DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    item = (hs1DHist *) GetItemByPtrID(id);
    if (item == NULL) {
    	fprintf(stderr, "hs_fill_1d_hist: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_1D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_fill_1d_hist: item (id = %d) not a 1-dim histogram.\n", id);
    	return;
    }
    if (x < item->min)
    	item->underflow += weight;
    else if (x >= item->max)
    	item->overflow += weight;
    else {
    	bin = ((x - item->min) * (float) item->nBins) / (item->max - item->min);
    	item->bins[bin] += weight;
    }
    ++item->count;
}

void histo_1d_hist_set_bin(int id, int ix, float value)
{
    hs1DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    item = (hs1DHist *) GetItemByPtrID(id);
    if (item == NULL) {
    	fprintf(stderr, "hs_1d_hist_set_bin: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_1D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_1d_hist_set_bin: item (id = %d) not a 1-dim histogram.\n", id);
    	return;
    }
    if (ix >= 0 && ix < item->nBins)
    {
        item->bins[ix] = value;
        ++item->count;
    }
}

void histo_1d_hist_set_bin_errors(int id, int ix, float pos, float neg, int flag)
{
    hs1DHist *item;
    int error_status_changed = 0;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    item = (hs1DHist *) GetItemByPtrID(id);
    if (item == NULL) {
    	fprintf(stderr, "hs_1d_hist_set_bin_errors: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_1D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_1d_hist_set_bin_errors: item (id = %d) not a 1-dim histogram.\n", id);
    	return;
    }
    if (ix >= 0 && ix < item->nBins)
    {
        int set_positive = 1, set_negative = 1;
        if (flag < 0)
            set_positive = 0;
        else if (flag > 0)
            set_negative = 0;
        if (set_positive)
        {
            if (item->pErrs == NULL)
            {
                item->pErrs = (float *)malloc(item->nBins * sizeof(float));
                memset(item->pErrs, 0, item->nBins * sizeof(float));
                item->errFlg = TRUE;
                error_status_changed = 1;
            }
            item->pErrs[ix] = pos;
        }
        if (set_negative)
        {
            if (item->pErrs == NULL)
            {
                fprintf(stderr,
                        "hs_1d_hist_set_bin_errors: can't set negative error "
                        "for id = %d: positive errors are not defined.\n", id);
                return;
            }
            if (item->mErrs == NULL)
            {
                item->mErrs = (float *)malloc(item->nBins * sizeof(float));
                memset(item->mErrs, 0, item->nBins * sizeof(float));
                item->errFlg = TRUE;
                error_status_changed = 1;
            }
            item->mErrs[ix] = neg;
        }
        ++item->count;
    }

    if (error_status_changed)
        SetHistResetFlag((hsGeneral *)item);
}

void histo_fill_2d_hist(int id, float x, float y, float weight)
/*
* histo_fill_2d_hist() adds a value to a two-dimensional histogram.
*
* parameters:	id       - histogram id (returned by hs_create_2d_hist)
*		x, y	 - axis values used to find the bin to increment
*		weight   - value added to bin
*/

{
    int bin_x = 1, bin_y = 1;
    hs2DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs2DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_fill_2d_hist: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_2D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_fill_2d_hist: item (id = %d) not a 2-dim histogram.\n", id);
    	return;
    }
    if (x < item->xMin)
    	bin_x = 0;
    else if (x >= item->xMax)
    	bin_x = 2;
    if (y < item->yMin)
    	bin_y = 2;		/* yes, really 2 */
    else if (y >= item->yMax)
    	bin_y = 0;		/* yes, really 0 */
    if (bin_x == 1 && bin_y == 1) {
    	bin_x = ((x - item->xMin) * (float) item->nXBins)
    					 / (item->xMax - item->xMin);
    	bin_y = ((y - item->yMin) * (float) item->nYBins)
    					 / (item->yMax - item->yMin);
    	item->bins[(bin_x)*item->nYBins + bin_y] += weight;
    }
    else 
    	item->overflow[bin_x][bin_y] += weight;
    ++item->count;
}

void histo_2d_hist_set_bin(int id, int ix, int iy, float value)
{
    hs2DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs2DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_2d_hist_set_bin: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_2D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_2d_hist_set_bin: item (id = %d) not a 2-dim histogram.\n", id);
    	return;
    }
    if (ix >= 0 && ix < item->nXBins && iy >= 0 && iy < item->nYBins)
    {
        item->bins[ix*item->nYBins + iy] = value;
        ++item->count;
    }
}

void histo_2d_hist_set_bin_errors(int id, int ix, int iy,
                                  float pos, float neg, int flag)
{
    hs2DHist *item;
    int error_status_changed = 0;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs2DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_2d_hist_set_bin_errors: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_2D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_2d_hist_set_bin_errors: item (id = %d) not a 2-dim histogram.\n", id);
    	return;
    }
    if (ix >= 0 && ix < item->nXBins && iy >= 0 && iy < item->nYBins)
    {
        int set_positive = 1, set_negative = 1;
        if (flag < 0)
            set_positive = 0;
        else if (flag > 0)
            set_negative = 0;
        if (set_positive)
        {
            if (item->pErrs == NULL)
            {
                item->pErrs = (float *)malloc(item->nXBins*item->nYBins*sizeof(float));
                memset(item->pErrs, 0, item->nXBins*item->nYBins*sizeof(float));
                item->errFlg = TRUE;
                error_status_changed = 1;
            }
            item->pErrs[ix*item->nYBins+iy] = pos;
        }
        if (set_negative)
        {
            if (item->pErrs == NULL)
            {
                fprintf(stderr,
                        "hs_2d_hist_set_bin_errors: can't set negative error "
                        "for id = %d: positive errors are not defined.\n", id);
                return;
            }
            if (item->mErrs == NULL)
            {
                item->mErrs = (float *)malloc(item->nXBins*item->nYBins*sizeof(float));
                memset(item->mErrs, 0, item->nXBins*item->nYBins*sizeof(float));
                item->errFlg = TRUE;
                error_status_changed = 1;
            }
            item->mErrs[ix*item->nYBins+iy] = neg;
        }
        ++item->count;
    }

    if (error_status_changed)
        SetHistResetFlag((hsGeneral *)item);        
}

void histo_fill_3d_hist(int id, float x, float y, float z, float weight)
/*
* histo_fill_3d_hist() adds a value to a two-dimensional histogram.
*/
{
    int bin_x = 1, bin_y = 1, bin_z = 1;
    hs3DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs3DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_fill_3d_hist: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_3D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_fill_3d_hist: item (id = %d) not a 3-dim histogram.\n", id);
    	return;
    }
    if (x < item->xMin)
    	bin_x = 0;
    else if (x >= item->xMax)
    	bin_x = 2;
    if (y < item->yMin)
    	bin_y = 0;
    else if (y >= item->yMax)
    	bin_y = 2;
    if (z < item->zMin)
    	bin_z = 0;
    else if (z >= item->zMax)
    	bin_z = 2;
    if (bin_x == 1 && bin_y == 1 && bin_z == 1) {
    	bin_x = ((x - item->xMin) * (float) item->nXBins)
    					 / (item->xMax - item->xMin);
    	bin_y = ((y - item->yMin) * (float) item->nYBins)
    					 / (item->yMax - item->yMin);
    	bin_z = ((z - item->zMin) * (float) item->nZBins)
    					 / (item->zMax - item->zMin);
    	item->bins[bin_x*item->nYBins*item->nZBins + 
		  bin_y*item->nZBins + bin_z] += weight;
    }
    else 
    	item->overflow[bin_x][bin_y][bin_z] += weight;
    ++item->count;
}

void histo_3d_hist_set_bin(int id, int ix, int iy, int iz, float value)
{
    hs3DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs3DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_3d_hist_set_bin: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_3D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_3d_hist_set_bin: item (id = %d) not a 3-dim histogram.\n", id);
    	return;
    }
    if (ix >= 0 && ix < item->nXBins && iy >= 0 && 
        iy < item->nYBins && iz >= 0 && iz < item->nZBins)
    {
        item->bins[(ix*item->nYBins+iy)*item->nZBins + iz] = value;
        ++item->count;
    }
}

void histo_3d_hist_set_bin_errors(int id, int ix, int iy, int iz,
                                  float pos, float neg, int flag)
{
    hs3DHist *item;
    int error_status_changed = 0;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs3DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_3d_hist_set_bin_errors: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_3D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_3d_hist_set_bin_errors: item (id = %d) not a 3-dim histogram.\n", id);
    	return;
    }
    if (ix >= 0 && ix < item->nXBins && iy >= 0 && 
        iy < item->nYBins && iz >= 0 && iz < item->nZBins)
    {
        int set_positive = 1, set_negative = 1;
        if (flag < 0)
            set_positive = 0;
        else if (flag > 0)
            set_negative = 0;
        if (set_positive)
        {
            if (item->pErrs == NULL)
            {
                item->pErrs = (float *)malloc(item->nXBins*item->nYBins*
                                              item->nZBins*sizeof(float));
                memset(item->pErrs, 0, item->nXBins*item->nYBins*
                                       item->nZBins*sizeof(float));
                item->errFlg = TRUE;
                error_status_changed = 1;
            }
            item->pErrs[(ix*item->nYBins+iy)*item->nZBins+iz] = pos;
        }
        if (set_negative)
        {
            if (item->pErrs == NULL)
            {
                fprintf(stderr,
                        "hs_3d_hist_set_bin_errors: can't set negative error "
                        "for id = %d: positive errors are not defined.\n", id);
                return;
            }
            if (item->mErrs == NULL)
            {
                item->mErrs = (float *)malloc(item->nXBins*item->nYBins*
                                              item->nZBins*sizeof(float));
                memset(item->mErrs, 0, item->nXBins*item->nYBins*
                                       item->nZBins*sizeof(float));
                item->errFlg = TRUE;
                error_status_changed = 1;
            }
            item->mErrs[(ix*item->nYBins+iy)*item->nZBins+iz] = neg;
        }
        ++item->count;
    }

    if (error_status_changed)
        SetHistResetFlag((hsGeneral *)item);        
}

void histo_hist_set_slice(int id, int bin0, int stride, int count, float *data)
{
    hsGeneral *item;
    float *hsData;
    int i, nbins;
    int setCount;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = GetItemByPtrID(id);
    if (item == NULL) {
    	fprintf(stderr, "hs_hist_set_slice: item (id = %d) not found.\n", id);
    	return;
    }
    switch (item->type)
    {
    case HS_1D_HISTOGRAM:
    {
        hs1DHist *h;
        h = (hs1DHist *)item;
        hsData = h->bins;
        nbins = h->nBins;
    }
    break;
    case HS_2D_HISTOGRAM:
    {
        hs2DHist *h;
        h = (hs2DHist *)item;
        hsData = h->bins;
        nbins = h->nXBins * h->nYBins;
    }
    break;
    case HS_3D_HISTOGRAM:
    {
        hs3DHist *h;
        h = (hs3DHist *)item;
        hsData = h->bins;
        nbins = h->nXBins * h->nYBins * h->nZBins;
    }
    break;
    default:
    	fprintf(stderr, "hs_hist_set_slice: item (id = %d) is not a histogram.\n", id);
        return;
    }
    if (count <= 0)
        return;
    if (bin0 < 0 || bin0 >= nbins)
        return;
    if (stride == 1)
    {
        int maxbin;
        maxbin = bin0 + count;
        if (maxbin > nbins)
            maxbin = nbins;
        setCount = maxbin-bin0;
        memcpy(hsData+bin0, data, setCount*sizeof(float));
    }
    else if (stride == -1)
    {
        int minbin;
        minbin = bin0 - count + 1;
        if (minbin < 0)
            minbin = 0;
        setCount = bin0 - minbin + 1;
        memcpy(hsData+minbin, data, setCount*sizeof(float));
    }
    else
    {
        for (setCount=0, i=bin0;
             setCount<count && i>=0 && i<nbins;
             ++setCount, i+=stride)
            hsData[i] = data[setCount];
    }
    switch (item->type)
    {
    case HS_1D_HISTOGRAM:
        ((hs1DHist *)item)->count += setCount;
        break;
    case HS_2D_HISTOGRAM:
        ((hs2DHist *)item)->count += setCount;
        break;
    case HS_3D_HISTOGRAM:
        ((hs3DHist *)item)->count += setCount;
        break;
    default:
        assert(0);
    }
}

void histo_hist_set_slice_errors(int id, int bin0, int stride, int count,
                                 float *err_valsP, float *err_valsM)
{
    hsGeneral *item;
    int i, nbins;
    int setCount;
    int error_status_changed = 0;
    float **pErrs, **mErrs;
    int *errFlg;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = GetItemByPtrID(id);
    if (item == NULL) {
    	fprintf(stderr, "hs_hist_set_slice_errors: item (id = %d) not found.\n", id);
    	return;
    }
    switch (item->type)
    {
    case HS_1D_HISTOGRAM:
    {
        hs1DHist *h;
        h = (hs1DHist *)item;
        pErrs = &h->pErrs;
        mErrs = &h->mErrs;
        nbins = h->nBins;
        errFlg = &h->errFlg;
    }
    break;
    case HS_2D_HISTOGRAM:
    {
        hs2DHist *h;
        h = (hs2DHist *)item;
        pErrs = &h->pErrs;
        mErrs = &h->mErrs;
        nbins = h->nXBins * h->nYBins;
        errFlg = &h->errFlg;
    }
    break;
    case HS_3D_HISTOGRAM:
    {
        hs3DHist *h;
        h = (hs3DHist *)item;
        pErrs = &h->pErrs;
        mErrs = &h->mErrs;
        nbins = h->nXBins * h->nYBins * h->nZBins;
        errFlg = &h->errFlg;
    }
    break;
    default:
    	fprintf(stderr, "hs_hist_set_slice_errors: item (id = %d) is not a histogram.\n", id);
        return;
    }
    if (count <= 0)
        return;
    if (bin0 < 0 || bin0 >= nbins)
        return;
    if (err_valsP == NULL && err_valsM == NULL)
        return;
    if (err_valsP)
    {
        if (*pErrs == NULL)
        {
            *pErrs = (float *)malloc(nbins*sizeof(float));
            memset(*pErrs, 0, nbins*sizeof(float));
            *errFlg = TRUE;
            error_status_changed = 1;
        }
        for (setCount=0, i=bin0;
             setCount<count && i>=0 && i<nbins;
             ++setCount, i+=stride)
            (*pErrs)[i] = err_valsP[setCount];
    }
    if (err_valsM)
    {
        if (*pErrs == NULL)
        {
            fprintf(stderr,
                    "hs_hist_set_slice_errors: can't set negative errors "
                    "for id = %d: positive errors are not defined.\n", id);
            return;
        }
        if (*mErrs == NULL)
        {
            *mErrs = (float *)malloc(nbins*sizeof(float));
            memset(*mErrs, 0, nbins*sizeof(float));
            *errFlg = TRUE;
            error_status_changed = 1;
        }
        for (setCount=0, i=bin0;
             setCount<count && i>=0 && i<nbins;
             ++setCount, i+=stride)
            (*mErrs)[i] = err_valsM[setCount];
    }
    switch (item->type)
    {
    case HS_1D_HISTOGRAM:
        ((hs1DHist *)item)->count += setCount;
        break;
    case HS_2D_HISTOGRAM:
        ((hs2DHist *)item)->count += setCount;
        break;
    case HS_3D_HISTOGRAM:
        ((hs3DHist *)item)->count += setCount;
        break;
    default:
        assert(0);
    }

    if (error_status_changed)
        SetHistResetFlag(item);
}

int histo_fill_ntuple(int id, float *values)
/*
* histo_fill_ntuple() adds an array of real values, in the order they are named
* in hs_create_ntuple, to an n-tuple.  The number of values supplied should
* be equal to the number of variables passed to hs_create_ntuple.
*
* parameters:	id       - n-tuple id (returned by hs_create_ntuple)
*		values   - pointer to an array of values to add to the n-tuple
*			   (one value per variable defined in hs_create_ntuple)
*/

{
    hsNTuple *nTuple;
    int i, quot, rem;

    if (!InitializedAndActive)
    	return 0;			/* hs_initialize must be called first */

    nTuple = (hsNTuple *) GetItemByPtrID(id); /* get item */

    if (nTuple == NULL) {
    	fprintf(stderr, "hs_fill_ntuple: item (id = %d) not found.\n", id);
    	return 0;
    }
    if (nTuple->type != HS_NTUPLE) {
    	fprintf(stderr,
    	    "hs_fill_ntuple: item (id = %d) is not an n-tuple.\n", id);
    	return 0;
    }
    quot = (nTuple->n - nTuple->chunkSzData) / nTuple->chunkSzExt;
    rem = (nTuple->n - nTuple->chunkSzData) % nTuple->chunkSzExt;
    if (quot >= 4) {
    	if (collectNtupleData(nTuple) == -1) {
    	    	fprintf(stderr,
    	    	    "hs_fill_ntuple: Memory Exhausted, item %d.\n", id);
    	    	return -1;
    	}
    	quot = 0;
    	rem = 0;
    }
    if (rem == 0) {
    	if (nTuple->extensions[quot] != NULL) {
    	    fprintf(stderr,
    	    	"hs_fill_ntuple: Internal Error, please report.\n");
    	    return 0;
    	}
    	else {
    	    nTuple->extensions[quot] = (float *) malloc(sizeof(float) *
    	  			       nTuple->nVariables * nTuple->chunkSzExt);
    	    if (nTuple->extensions[quot] == NULL) {
    	    	fprintf(stderr,
    	    	    "hs_fill_ntuple: Memory Exhausted, item %d.\n", id);
    	    	return -1;
    	    }
    	}
    }
    for (i = 0; i < nTuple->nVariables; ++i)
    	nTuple->extensions[quot][rem * nTuple->nVariables + i]
    	    = values[i];
    ++(nTuple->n);
    return id;
}

void histo_set_indicator(int id, float value)
/*
* histo_set_indicator() sets the value of an indicator.
*
* parameters:	id       - indicator id (returned by hs_create_indicator)
*		value	 - value for the indicator
*/

{
    hsIndicator *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hsIndicator *) GetItemByPtrID(id); 

    if (item == NULL) {
    	fprintf(stderr, "hs_set_indicator: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_INDICATOR) {
    	fprintf(stderr,
    	    "hs_set_indicator: item (id = %d) is not an indicator.\n", id);
    	return;
    }
    if (value < item->min)
    	item->valueSet = VALUE_UNDERFLOW;
    else  if (value > item->max)
    	item->valueSet = VALUE_OVERFLOW;
    else
	item->valueSet = VALUE_SET;
    item->value = value;
}

void histo_read_control(int id, float *value)
/*
* histo_read_control() returns the current value of a control (which might 
*		have been set by Histo-Scope).
*
* parameters:	id       - control id (input, returned by hs_create_control)
*		value	 - current value of the control (output, returned)
*/

{
    hsControl *item;
    int i;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hsControl *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_read_control: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_CONTROL) {
    	fprintf(stderr,
    	    "hs_read_control: item (id = %d) is not a control.\n", id);
    	return;
    }
    item->value = ((hsCControl *)item)->newValue;
    *value = item->value;
    if (((hsCControl *)item)->valueSet == 1) {
    	((hsCControl *)item)->valueSet = 0;   /* indicate value has been read */
    	for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	    sendToHS(i, CONTROL_READ, id, (hsGeneral *) item);
    }
}

int histo_check_trigger(int id)
/*
* histo_check_trigger() returns 1 if the trigger has been set by Histo-Scope;
*			otherwise 0 is returned.
*
* parameters:	id - trigger id (input, returned by hs_create_trigger)
*/

{
    hsTrigger *item;
    int i;

    if (!InitializedAndActive)
    	return 0;			/* hs_initialize must be called first */

    item = (hsTrigger *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_check_trigger: item (id = %d) not found.\n", id);
    	return 0;
    }
    if (item->type != HS_TRIGGER) {
    	fprintf(stderr,
    	    "hs_check_trigger: item (id = %d) is not a trigger.\n", id);
    	return 0;
    }
    if (((hsCTrigger *)item)->numTriggered == 0)
    	return 0;		 	/* indicate no events yet set by HS  */
    if (((hsCTrigger *)item)->numTriggered < 0) {
    	fprintf(stderr,
    	 "hs_check_trigger Internal Error: numTriggered for id %d reset to 0\n",
    	  id);
    	((hsCTrigger *)item)->numTriggered= 0;
    	return 0;
    }
    --((hsCTrigger *)item)->numTriggered;
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	sendToHS(i, TRIGGER_READ, id, (hsGeneral *) item);
    return 1;
}

void histo_set_1d_errors(int id, float *err_valsP, float *err_valsM)
/*
* histo_set_1d_errors() copies a vector of real numbers representing the
* errors of a 1-dimensional histogram.  The vector should have the same
* number of elements as the histogram has bins.
* 
* parameters:	id         - item id
*		err_valsP  - pointer to a vector of floating point errors (+)
*				if NULL, Positive errors are not set.
*		err_valsM  - pointer to a vector of floating point errors (-)
*				if NULL, Negative errors are not set.  Negative
*				errors should only be specified if positive
*				errors are or have been previously specified.
*			        Error values are specified as negative offsets 
*			        from the top of histogram bar (meaning they  
*			        should be positive numbers).
*
* Note:  If positive errors are supplied, but negative ones aren't, Histo-Scope
*	 assumes errors are symmetric.  Thus memory can be saved by specifying
*	 only positive errors when errors are symmetric.
*
*/

{
    hs1DHist *item;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs1DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_set_1d_errors: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_1D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_set_1d_errors: item (id = %d) not a 1-dim histogram.\n", id);
    	return;
    }

    SetHistResetFlag((hsGeneral *)item);

    /* Positive Errors */
    if (err_valsP != NULL) {
    	if (item->pErrs == NULL)
    	    item->pErrs = (float *) malloc(item->nBins * sizeof(float));
	memcpy((void *)item->pErrs, (void *)err_valsP,
		item->nBins * sizeof(float));
    	item->errFlg = TRUE;
    }
    /* Negative (minus) Errors */
    if (err_valsM != NULL) {
    	if (item->pErrs == NULL) {
    	    fprintf (stderr,
	  "hs_set_1d_errors: Set Negative errors only when positive errors \n");
            fprintf (stderr,
                "are also set. No errors defined for id = %d.\n", id);
    	    return;
    	}
    	if (item->mErrs == NULL)
    	    item->mErrs = (float *) malloc(item->nBins * sizeof(float));
	memcpy((void *)item->mErrs, (void *)err_valsM, 
		item->nBins * sizeof(float));
    	item->errFlg = TRUE;
    }
}

void histo_set_2d_errors(int id, float *err_valsP, float *err_valsM,
			 int col_maj_flag)
/*
* histo_set_2d_errors() copies an array of real numbers representing the
* errors of a 2-dimensional histogram.  The array should have the same
* number of elements in each dimension as the histogram has bins.
* 
* parameters:	id           - item id
*		err_valsP  - pointer to an array of floating point errors
*				if NULL, Positive errors are not set.
*		err_valsM  - pointer to an array of floating point errors
*				if NULL, Negative errors are not set.  Negative
*				errors should only be specified if positive
*				errors are or have been previously specified.
*			        Error values are specified as negative offsets 
*			        from the top of histogram bar (meaning they  
*			        should be positive numbers).
*		col_maj_flag - if != 0, flags that 2d array is column major
*				 (i.e. from FORTRAN)
*
* Note:  If positive errors are supplied, but negative ones aren't, Histo-Scope
*	 assumes errors are symmetric.  Thus memory can be saved by specifying
*	 only positive errors when errors are symmetric.
*
*/

{
    hs2DHist *item;
    int i, j;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs2DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_set_2d_errors: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_2D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_set_2d_errors: item (id = %d) not a 2-dim histogram.\n", id);
    	return;
    }

    SetHistResetFlag((hsGeneral *)item);

    /* Positive Errors */
    if (err_valsP != NULL) {
    	if (item->pErrs == NULL)
    	    item->pErrs = (float *) malloc(item->nXBins * item->nYBins
    	    					        * sizeof(float));
	if (col_maj_flag) {
	    for (i = 0; i < item->nXBins; ++i)
	    	for (j = 0; j < item->nYBins; ++j)
	    	    item->pErrs[i*item->nYBins+j] = err_valsP[j*item->nXBins+i];
	} else
	    memcpy((void *)item->pErrs, (void *)err_valsP,
	    	    item->nXBins * item->nYBins
    	    					        * sizeof(float));
    	item->errFlg = TRUE;
    }
    /* Negative (minus) Errors */
    if (err_valsM != NULL) {
    	if (item->pErrs == NULL) {
    	    fprintf (stderr,
	  "hs_set_2d_errors: Set Negative errors only when positive errors \n");
            fprintf (stderr,
                "are also set. No errors defined for id = %d.\n", id);
    	    return;
    	}
    	if (item->mErrs == NULL)
    	    item->mErrs = (float *) malloc(item->nXBins * item->nYBins
    	    					        * sizeof(float));
	if (col_maj_flag) {
	    for (i = 0; i < item->nXBins; ++i)
	    	for (j = 0; j < item->nYBins; ++j)
	    	    item->mErrs[i*item->nYBins+j] = err_valsM[j*item->nXBins+i];
	} else
	    memcpy((void *)item->mErrs, (void *)err_valsM,
	    	    item->nXBins * item->nYBins * sizeof(float));
    	item->errFlg = TRUE;
    }
}

void histo_set_3d_errors(int id, float *err_valsP, float *err_valsM,
			 int col_maj_flag)
/*
* histo_set_3d_errors() copies an array of real numbers representing the
* errors of a 3-dimensional histogram.  The array should have the same
* number of elements in each dimension as the histogram has bins.
*/

{
    hs3DHist *item;
    int i, j, k;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hs3DHist *) GetItemByPtrID(id); /* get item */

    if (item == NULL) {
    	fprintf(stderr, "hs_set_3d_errors: item (id = %d) not found.\n", id);
    	return;
    }
    if (item->type != HS_3D_HISTOGRAM) {
    	fprintf(stderr,
    	    "hs_set_3d_errors: item (id = %d) not a 3-dim histogram.\n", id);
    	return;
    }

    SetHistResetFlag((hsGeneral *)item);

    /* Positive Errors */
    if (err_valsP != NULL) {
    	if (item->pErrs == NULL)
    	    item->pErrs = (float *) malloc(item->nXBins * item->nYBins
    	    				  *item->nZBins * sizeof(float));
	if (col_maj_flag) {
	    for (i = 0; i < item->nXBins; ++i)
	    	for (j = 0; j < item->nYBins; ++j)
		    for (k = 0; k < item->nZBins; ++k)
			item->pErrs[i*item->nYBins*item->nZBins+j*item->nZBins+k] =
			    err_valsP[k*item->nXBins*item->nYBins+j*item->nXBins + i];
	} else
	    memcpy((void *)item->pErrs, (void *)err_valsP,
	    	    item->nXBins*item->nYBins*item->nZBins*sizeof(float));
    	item->errFlg = TRUE;
    }
    /* Negative (minus) Errors */
    if (err_valsM != NULL) {
    	if (item->pErrs == NULL) {
    	    fprintf (stderr,
	  "hs_set_3d_errors: Set Negative errors only when positive errors \n");
            fprintf (stderr,
                "are also set. No errors defined for id = %d.\n", id);
    	    return;
    	}
    	if (item->mErrs == NULL)
    	    item->mErrs = (float *) malloc(item->nXBins * item->nYBins
    	    				  *item->nZBins * sizeof(float));
	if (col_maj_flag) {
	    for (i = 0; i < item->nXBins; ++i)
	    	for (j = 0; j < item->nYBins; ++j)
		    for (k = 0; k < item->nZBins; ++k)
			item->mErrs[i*item->nYBins*item->nZBins+j*item->nZBins+k] = 
			    err_valsM[k*item->nXBins*item->nYBins+j*item->nXBins + i];
	} else
	    memcpy((void *)item->mErrs, (void *)err_valsM,
		   item->nXBins*item->nYBins*item->nZBins*sizeof(float));
    	item->errFlg = TRUE;
    }
}

void histo_allow_reset_refresh(int id, int flag)
{
    hsGeneral *item;
    int i;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    
    item = (hsGeneral *) GetItemByPtrID(id); /* get item*/

    if (item == NULL) {
    	fprintf(stderr, "hs_allow_reset_refresh: Error - id: %d does not exist.\n", id);
    	return;
    }
    if (item->type == HS_NTUPLE)
    {
	((hsCNTuple *)item)->inhibitResetRefresh = !flag;
	/* notify all histoscopes */
	for (i = 0; i < MAX_NUM_SCOPES; ++i) {     /* for each HistoScope */
	    if (ComFD[i] == NO_CONNECTION)
		continue;			   /* skip if no connection */
	    if (flag)
		sendToHS(i, ALLOW_RESET_REFRESH, ((hsNTuple *)item)->id, 0);
	    else
		sendToHS(i, INHIBIT_RESET_REFRESH, ((hsNTuple *)item)->id, 0);
	}
    }
    return;
}

void histo_reset(int id)
/*
* histo_reset() resets all of the bins of a histogram to 0, or removes all of
* the data from an n-tuple, or sets an indicator to 0, or sets a control back
* to its default value.
*
* parameters:	id       - item id
*
*/

{
    hsGeneral *item;
    int i;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    
    item = (hsGeneral *) GetItemByPtrID(id); /* get item*/

    if (item == NULL) {
    	fprintf(stderr, "hs_reset: Error - id: %d does not exist.\n", id);
    	return;
    }
    switch (item->type) {
	case HS_1D_HISTOGRAM:
	    reset1DHist( (hs1DHist *) item);
    	    break;
	case HS_2D_HISTOGRAM:
    	    reset2DHist( (hs2DHist *) item);
    	    break;
	case HS_3D_HISTOGRAM:
    	    reset3DHist( (hs3DHist *) item);
    	    break;
	case HS_NTUPLE:
	    ((hsCNTuple *)item)->fromTuple = 0;
	    ((hsCNTuple *)item)->toTuple = -1;
	    ResetNtuple((hsNTuple *)item);
	    /* notify all histoscopes */
	    for (i = 0; i < MAX_NUM_SCOPES; ++i) {     /* for each HistoScope */
		if (ComFD[i] == NO_CONNECTION)
		    continue;			     /* skip if no connection */
		forgetUnfinMsg(i, ((hsNTuple *)item)->id);
		sendToHS(i, NTUPLE_RESET, ((hsNTuple *)item)->id, 0);
		((hsCNTuple *)item)->needsCompleteUpdate = 0xffffffff;
	    }
    	    break;
	case HS_INDICATOR:
	    ((hsIndicator *)item)->value = ((hsIndicator *)item)->min;
	    ((hsIndicator *)item)->valueSet = VALUE_NOT_SET;
    	    break;
	case HS_CONTROL:
	    ((hsControl *)item)->value = ((hsCControl *)item)->defaultValue;
	    ((hsCControl *)item)->valueSet = 0;        /* value not set by HS */
	    ((hsCControl *)item)->newValue = ((hsCControl *)item)->defaultValue;
    	    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    		sendToHS(i, CONTROL_READ, id, (hsGeneral *) item);
    	    break;
	case HS_TRIGGER:
    	    ((hsCTrigger *)item)->numTriggered = 0;
	    /* notify all histoscopes */
	    for (i = 0; i < MAX_NUM_SCOPES; ++i) {     /* for each HistoScope */
		if (ComFD[i] == NO_CONNECTION)
		    continue;			     /* skip if no connection */
		sendToHS(i, TRIGGER_RESET, ((hsTrigger *)item)->id, 0);
	    }
    	    break;
	case HS_GROUP:
	    /* Reset all the histograms in the group */
	    for (i = 0; i < ((hsGroup *)item)->numItems; ++i) {
	        hsGeneral *subItem=GetItemByPtrID(((hsGroup *)item)->itemId[i]);
	    	if (subItem != NULL) {
	    	    if (subItem->type == HS_1D_HISTOGRAM)
	    	        reset1DHist( (hs1DHist *) subItem);
	    	    else if (subItem->type == HS_2D_HISTOGRAM)
	    	        reset2DHist( (hs2DHist *) subItem);
	    	    else if (subItem->type == HS_3D_HISTOGRAM)
	    	        reset3DHist( (hs3DHist *) subItem);
	        }
	    }
    	    break;
	default:
	    fprintf(stderr,
	    	"hs_reset: Internal Error: item was of unknown type.\n");
    }
}

static void reset1DHist(hs1DHist *item)
{
    int i;

    for (i = 0; i < (item)->nBins; ++i)
	item->bins[i] = 0.f;
    item->overflow  = 0.;
    item->underflow = 0.;
    item->count     = 0 ;
    /* set reset flag for each possible connected Scope */
    SetHistResetFlag((hsGeneral *)item);
}

static void reset2DHist(hs2DHist *item)
{
    int i, j, nbins;
    
    item->count = 0;
    /* set reset flag for each possible connected Scope */
    SetHistResetFlag((hsGeneral *)item);
    nbins = (item)->nXBins * (item)->nYBins;
    for (i = 0; i < nbins; ++i)
    	item->bins[i] = 0.f;
    for (i = 0; i < 3; ++i)
    	for (j = 0; j < 3; ++j)
    	    item->overflow[i][j] = 0.f;
}

static void reset3DHist(hs3DHist *item)
{
    int i, j, k, nbins;
    
    item->count = 0;
    /* set reset flag for each possible connected Scope */
    SetHistResetFlag((hsGeneral *)item);
    nbins = (item)->nXBins * (item)->nYBins * (item)->nZBins;
    for (i = 0; i < nbins; ++i)
    	item->bins[i] = 0.f;
    for (i = 0; i < 3; ++i)
    	for (j = 0; j < 3; ++j)
	    for (k = 0; k < 3; ++k)
		item->overflow[i][j][k] = 0.f;
}

void histo_delete(int id)
/*
* histo_delete() deletes a histogram, n-tuple, indicator, trigger, group, etc.
*
*		** Note: histo_delete does NOT delete the members of a group
*			 it only deletes the group item itself.
*
* parameters:	id       - item id
*
*/

{
    hsGeneral *item;
    int i;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */

    item = (hsGeneral *) GetItemByPtrID(id);

    if (item == NULL) {
    	fprintf(stderr, "hs_delete: Error - id: %d does not exist.\n", id);
    	return;
    }
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION && AllowItemSend != 0)
    	    sendToHS(i, ITEM_DELETED, id, item);
    DeleteItemFromPtrList(id);
    FreeItem(item);
}

void histo_delete_items(int *ids, int num_items)
/*
* histo_delete_items() deletes a  number of histograms, n-tuples, or indicators
*
* parameters:	ids       - array of item ids
* 		num_items - # of items in ids array
*
*/

{
    int i;

    if (!InitializedAndActive)
    	return;				/* hs_initialize must be called first */
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
            sendToHS(i, BEGIN_LIST, 0, NULL);
    for (i = 0; i < num_items; ++i)
	histo_delete(ids[i]);
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
            sendToHS(i, END_OF_LIST, 0, NULL);
}

void histo_change_uid(int id, int newuid)

/* Change the User Identification Number for a particular item, refered by its
   HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
	id              HistoScope ID of the modified item   
	newuid   	New UID number.
	
*/
{
    int i, j;
    hsGeneral *item;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_change_uid: Error, called before hs_initialize.\n");
    	return;			/* hs_initialize must be called first */
    }
	
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
    fprintf(stderr, "hs_change_uid: Error - id: %d does not exist.\n", id);
    	return;
    }
    
    j = histo_id(newuid, item->category);
    if (j >= 0) { 
    fprintf(stderr, "hs_change_uid: uid = %d already exists in category %s \n",
              newuid, item->category);
    fprintf(stderr, "  No change made for id = %d \n", id);
    	return;
    }
    
    item->uid = newuid;
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    sendToHS(i, ITEM_DELETED, id, item);
    SendNewItem(item);  
    return;
}
 
void histo_change_category(int id, const char *newcategory)

/* Rename the Category of particular item, refered by its HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
	id              HistoScope ID of the modified item   
	newcategory	New Category name.
	
*/
{
    int i, j;
    hsGeneral *item;
    const char *catString;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_change_category: Error, called before hs_initialize.\n");
    	return;			/* hs_initialize must be called first */
    }
	
    if (!CheckValidCategory(newcategory, "hs_change_category", FALSE)) {
    	fprintf(stderr,
    		"hs_change_category: Error, invalid category string\n");
       return;
    }
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
	fprintf(stderr, 
	    "hs_change_category: Error - id: %d does not exist.\n", id);
    	return;
    }
    
    j = histo_id(item->uid, newcategory);
    if (j >= 0) { 
	fprintf(stderr, 
    	     "hs_change_category: uid = %d already exists in category %s \n",
              item->uid, newcategory);
	fprintf(stderr, "  No change made for id = %d \n", id);
    	return;
    }
    catString = ValidStr(newcategory, HS_MAX_CATEGORY_LENGTH, 
    	"hs_change_category", "category");
    if (strlen(catString) > strlen(item->category)) {
	free(item->category);
	item->category = malloc(sizeof(char) * (strlen(catString) + 1));
    }
    strcpy(item->category, catString);
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    sendToHS(i, ITEM_DELETED, id, item);
    SendNewItem(item);  
    return;
}

void histo_change_uid_and_category(int id, int newuid, const char *newcategory)

/* Change the User Identification Number and Rename the Category of a 
   particular item, refered by its HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
	id              HistoScope ID of the modified item   
	newuid   	New UID number.
	newcategory	New Category name
*/
{
    int i, j;
    hsGeneral *item;
    const char *catString;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_change_uid_and_category: Error, called before hs_initialize.\n");
    	return;			/* hs_initialize must be called first */
    }
	
    if (!CheckValidCategory(newcategory, "hs_change_uid_and_category", FALSE)) {
    	fprintf(stderr,
    		"hs_change_uid_and_category: Error, invalid category string\n");
       return;
    }
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
	fprintf(stderr, 
	    "hs_change_uid_and_category: Error - id: %d does not exist.\n", id);
    	return;
    }
    
    j = histo_id(newuid, newcategory);
    if (j >= 0) { 
	fprintf(stderr, 
    	     "hs_change_uid_and_category: uid = %d already exists in category %s \n",
              newuid, newcategory);
	fprintf(stderr, "  No change made for id = %d \n", id);
    	return;
    }
    catString = ValidStr(newcategory, HS_MAX_CATEGORY_LENGTH, 
    	"hs_change_uid_and_category", "category");
    if (strlen(catString) > strlen(item->category)) {
	free(item->category);
	item->category = malloc(sizeof(char) * (strlen(catString) + 1));
    }
    strcpy(item->category, catString);
    item->uid = newuid;
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    sendToHS(i, ITEM_DELETED, id, item);
    SendNewItem(item);  
    return;
}

void histo_change_title(int id, const char *newtitle)

/* Rename the Title of particular item, refered by its HistoScope ID number.
   If Scope(s) is connected to a the process calling this routine,
   the list of items will get the update.
	id              HistoScope ID of the modified item   
	newtitle	New title string.
	
*/
{
    int i;
    hsGeneral *item;
    const char *catString;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_change_title: Error, called before hs_initialize.\n");
    	return;			/* hs_initialize must be called first */
    }
	
    item = (hsGeneral *) GetItemByPtrID(id);
    if (item == NULL) {
    fprintf(stderr, "hs_change_title: Error - id: %d does not exist.\n", id);
    	return;
    }
    catString = ValidStr(newtitle, HS_MAX_TITLE_LENGTH,
			 "hs_change_title", "title");
    if (strlen(catString) > strlen(item->title)) {
      free(item->title);
      item->title = malloc(sizeof(char) * (strlen(catString) + 1));
    }
    item->title = strcpy(item->title, catString);
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
      if (ComFD[i] != NO_CONNECTION)
	sendToHS(i, ITEM_DELETED, id, item);
    SendNewItem(item);  
    return;
}

int histo_num_connected_scopes(void)
{
    int i, numConnected = 0;

    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ScopeConfirmed[i])
    	    ++numConnected;
    	    
    return numConnected;
}

void histo_load_config(const char *cfgBuf, int cfgBufLen)
{
    int conid, sndStat;
    hsConfigString hsC;

    /* Build an hsConfigString structure to send the string through xdr rtns */
    hsC.type = HS_CONFIG_STRING;
    hsC.id = 0;
    hsC.title = "";
    hsC.category = "";
    hsC.uid = 0;
    hsC.hbookID = 0;
    hsC.stringLength = cfgBufLen;
    hsC.configString = (char *)cfgBuf;
    
    /* Send the config string to all connected & confirmed histos */
    for (conid = 0; conid < MAX_NUM_SCOPES; ++conid)
    	if (ScopeConfirmed[conid])
    	    sndStat = sendToHS(conid, HERE_IS_DATA, hsC.id, (hsGeneral *)&hsC);

}

/************************ End of User-Callable Routines ***********************/

int hs_ClientVersion(void)
{
    return V4_CLIENT;			/* used by xdr routines */
}

static int setUpComPort(int *portSock, int *portNum)
/*
** Setup a socket for accepting communications from a Histo-Scope process.
** This socket is for accepting requests only, the real communication is
** done with the socket created by the accept call.
*/
{
    int sock, length, sizze;
#if defined(VMS) || defined(VXWORKS)
    int nbio_enabled = 1;
#endif /*VMS || VXWORKS*/
    struct sockaddr_in server;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        socket_perror("HistoClient: Error opening stream socket");
        return COM_FAILURE;
    }
    /* Set accepts to be non-blocking */
#if defined(VMS) || defined(VXWORKS)
    if (socket_ioctl(sock, FIONBIO, &nbio_enabled) < 0) {
    	socket_perror("HistoClient: Error in ioctl");
#else
    if (fcntl(sock, F_SETFL, O_NDELAY) < 0) {
    	perror("HistoClient: Error in fcntl");
#endif /*VMS || VXWORKS*/
        return COM_FAILURE;
    }
    /* Name socket using wildcards. */
    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server.sin_port = 0;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)))
    {
        socket_perror("HistoClient: Error binding stream socket");
        return COM_FAILURE;
    }

    /* Find out assigned port number */
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *)&server, &length))
    {
        socket_perror("hs_initialize: Error getting socket name");
        return COM_FAILURE;
    }
    *portNum = ntohs(server.sin_port);
    
    /* Start accepting connection */
    if (listen(sock, MAX_NUM_SCOPES) < 0)
        socket_perror("hs_initialize: Error listening to socket");
    
    /* Ensure buffer size is at least SOCKBUFSIZMIN */
    sizze = sizeof(int);
    if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&SocketBufferSize, 
    		&sizze) < 0) {
    	socket_perror ("hs_initialize: Error in getsockopt:");
    	return COM_FAILURE;
    }
#ifdef COM_DEBUG
    printf("Socket buffer size = %d\n", SocketBufferSize);
#endif
     if (SocketBufferSize < SOCKBUFSIZMIN) {
    	SocketBufferSize = SOCKBUFSIZMIN;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&SocketBufferSize, 
		sizze) < 0) {
    	    socket_perror ("hs_initialize: Error in setsockopt:");
    	    return COM_FAILURE;
    	}
	if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&SocketBufferSize, 
		&sizze) < 0) {
    	    socket_perror ("hs_initialize: error in getsockopt");
    	    return COM_FAILURE;
	}
#ifdef COM_DEBUG
	printf("Socket buffer size changed to = %d\n", SocketBufferSize);
#endif
    } 
    
    /* Return the socket file descriptor */
    *portSock = sock;
    return COM_OK;
}

static int runHistoScope(char *cmd_str)
{
    int child_pid;
    /*
    ** Fork the Histo-Scope process and run the command
    */
    if (0 == (child_pid = fork())) {
       	/* child process */
	execl("/bin/csh", "csh", "-c", cmd_str, NULL);
	/* if we reach here, execl failed */
	fprintf(stderr, 
	    "hs_histoscope: Error starting Histo-Scope process (execl failed)");
	return RUN_FAILURE;
    }
    /* parent process */
    return child_pid;
}

/*
 * acceptHCom - accept any and all pending HistoScope connections
 */
static void acceptHCom(void)
{
    int connect_id, sockFD, found;
#if defined(VMS) || defined(VXWORKS)
    int nbio_enabled = 1;
#endif /*VMS || VXWORKS*/

    while (1) {
	if (NumOfConnections >= MAX_NUM_SCOPES)
    	    return;	    /* too many connections, don't even try to accept */
	if ((sockFD = accept(AccComFD,0,0)) <= 0)
    	    return;		    /* error; nothing pending to accept       */
	if (ComFD[NumOfConnections] == NO_CONNECTION)
	    connect_id = NumOfConnections;
	else {			    /* find vacant File Descriptor to use     */
	    found = FALSE;
    	    for (connect_id = 0; connect_id < MAX_NUM_SCOPES; ++connect_id) {
    		if (ComFD[connect_id] == NO_CONNECTION) {
    		    found = TRUE;
    	    	    break;
    	    	}
    	    }
    	    if (!found) {
    	    	fprintf(stderr,
    	    	    "hs_update: acceptHCom program bug - please report\n");
 	   	InitializedAndActive = FALSE;
    	    	return;
    	    }
	}
    	ComFD[connect_id] = sockFD;		    /* save File Descriptor   */
	++NumOfConnections;
#if defined(VMS) || defined(VXWORKS)
	if (socket_ioctl(sockFD, FIONBIO, &nbio_enabled) < 0)
    	    socket_perror("hs: Error in ioctl on socket");
#else
	if (fcntl(sockFD, F_SETFL, O_NDELAY) < 0)   /* make socket non-blockd */
    	    perror("hs: Error in fcntl on socket");
#endif /*VMS || VXWORKS*/
#ifdef COM_DEBUG
	printf("Accepted socket connection.  connect_id = %d, NumOfConnections = %d, sockFD = %d\n",
		connect_id, NumOfConnections, sockFD);
#endif
    }
}

/*
 * Send item list to a newly connected Histo-Scope process
 */
static int sendItemList(int connect_id)
{
    int i;
#ifdef LINKED_WITH_HBOOK
    hsGeneral *hsG;
#endif /* LINKED_WITH_HBOOK */

    for (i = 0; i < NumOfItems; ++i)
        if (HistoPtrList[i] != NULL)
    	    if (sendToHS(connect_id, ITEM_TO_LIST, HistoPtrList[i]->id, 
			 HistoPtrList[i]) != COM_OK)
    	        return COM_FAILURE;

#ifdef LINKED_WITH_HBOOK
    for ((hsG = GetHBItemToList(0)); hsG != NULL; hsG = GetHBItemToList(1)) {
    	if (hsG != NULL)
            if(sendToHS(connect_id, ITEM_TO_LIST, hsG->id, hsG) != COM_OK)
    	        return COM_FAILURE;
    }
#endif /* LINKED_WITH_HBOOK */

   if (sendToHS(connect_id, END_OF_LIST, 0, NULL) != COM_OK)
       return COM_FAILURE;

    for (i = 0; i < NumOfItems; ++i)
        if (HistoPtrList[i] != NULL)
	    if (HistoPtrList[i]->type == HS_NTUPLE)
		if (((hsCNTuple *)HistoPtrList[i])->inhibitResetRefresh)
		    if (sendToHS(connect_id, INHIBIT_RESET_REFRESH, 
				 HistoPtrList[i]->id, NULL) != COM_OK)
			return COM_FAILURE;
    return COM_OK;
}

void histo_allow_item_send(int i)
{
  if (i == 0)
    AllowItemSend = 0;
  else
    AllowItemSend = 1;
}

/*
 * Send new item to all connected Histo-Scope processes
 */
void SendNewItem(hsGeneral *item)
{
    int i;
    
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION && AllowItemSend != 0)
    	    if (sendToHS(i, NEW_ITEM, item->id, item) != COM_OK)
    	    	fprintf(stderr,
    	    	    "hs_create: error sending item to Histo-Scope %d.\n", i);
}

/*
 * Set control received from Histo-Scope
 */
static void setControl(int conid, int itemID, int bytesToRead)
{
    XDR xdrs;
    int i;
    char *buf;
    float value;
    hsControl *hsC;
    
    /* Read remainder of CONTROL_SET message.  Two reads are OK for this *
     * message because Histo-Scope wrote both parts of it as one chunk   */
    buf = malloc(bytesToRead);
    if (buf == NULL) {
    	closeScope(conid);
    	fprintf(stderr, "hs_update: malloc failed.  Closed HistoScope %d.",
    		conid);
    	return;
    }
    i = socket_read(ComFD[conid], buf, bytesToRead);
    if (i != bytesToRead) {
	fprintf(stderr,
	    "hs: Error reading control value from HistoScope %d. (%d)\n",
	    conid, i);
	closeScope(conid);
	free(buf);
	return;
    }
    
    /* Translate Control Value */
    if (bytesToRead != sizeof(float)) {
        fprintf(stderr,
	    "hs: Internal Warning, reading control value from HS %d. (%d)\n",
	    conid, bytesToRead);
	free(buf);
	return;
    }
    xdrmem_create(&xdrs, buf, bytesToRead, XDR_DECODE);
    if (!(i=xdr_float(&xdrs, &value))) {	/* translate Control Value */
	    fprintf(stderr,
	   "hs: Error xdr translating Control Value from HistoScope %d. (%d)\n",
		conid, i);
	    closeScope(conid);
	    xdr_destroy (&xdrs);
	    free(buf);
	    return;
    }
    xdr_destroy (&xdrs);
    free(buf);
    /* Set control value and flag that it hasn't been read by client yet */
    hsC = (hsControl *) GetItemByPtrID(itemID); 
    if (hsC->type != HS_CONTROL) {
    	fprintf(stderr,
    	 "hs: Internal Warning, error in Histo-Scope %d setting control (%d)\n",
    	 conid, itemID);
    	return;
    }
#ifdef COM_DEBUG
	printf( "    setting control %d: (%s) from %f to value %f\n", itemID, 
		hsC->title, hsC->value, value);
#endif
    ((hsCControl *)hsC)->newValue = value; /* save new value until read by c. */
    ((hsCControl *)hsC)->valueSet = 1;   /* flag value not read by client yet */
}


/*
 * Set trigger received from Histo-Scope
 */
static void setTrigger(int conid, int itemID)
{
    hsTrigger *hsT;
    
    /* Register the trigger event */
    hsT = (hsTrigger *) GetItemByPtrID(itemID);
    if (hsT->type != HS_TRIGGER) {
    	fprintf(stderr,
    	 "hs: Internal Warning, error in Histo-Scope %d setting trigger (%d)\n",
    	 conid, itemID);
    	return;
    }
    ++((hsCTrigger *)hsT)->numTriggered;
#ifdef COM_DEBUG
	printf( "    setting trigger %d: (%s) numTriggered = %d\n", itemID, 
		hsT->title, ((hsCTrigger *)hsT)->numTriggered);
#endif
}

/*
 * processUpdates - Reads remainder of Histo-Scope's request updates message
 * 	            for the list of items to send, and sends item data
 * 	            appropriately.
 *
 * 	            Note: numOfItems comes from the itemID field of the
 * 	                  REQUEST_UPDATES message.  The individual item
 * 	                  ID's are found in the data part of the message
 * 	                  yet to be read.
 */
static void processUpdates(int conid, int numOfItems, int bytesToRead)
{
    int i;
    procStat stat;
    
#ifdef VERBOSE
    printf("processUpdates called\n");
#endif

    /* Read remainder of REQUEST_UPDATES message and put into Update List */
    readNSaveReqs(conid, numOfItems, bytesToRead);
    if (UpdateList[conid] == NULL)
    	return;				/* error, or no updates to process */
    if (UnfinMsgList[conid] != NULL)
    {	
#ifdef VERBOSE
	printf("process unsent msgs before updates\n");
#endif
    	return;		     		/* process unsent msgs before updates */
    }

    /* Process each update request */
    while (UpdateList[conid]->itemsProcessed < numOfItems) {
	for (i = 0; i < numOfItems; ++i) {
	    if ((UpdateList[conid]->updStatPtr)[i] == PROCESSED)
	    	continue;
#ifdef VERBOSE
	    printf("Will call processUpdReq for conid %d, item %d\n", conid, i);
#endif
    	    stat = processUpdReq(conid, i);
	    if (stat.incItemsProcessed)
	    	++(UpdateList[conid]->itemsProcessed);
	    if (!stat.continueProcessing)
	    {
#ifdef VERBOSE
		printf("Updates broken\n");
#endif
	    	return;
	    }
	}   /* next item on this Histo-Scope's list */
    }	/* go back and finish any partially processed items */
    /* Finished with Update for this Histo-Scope */
    freeUpdateList(conid);
    sendToHS(conid, UPDATES_FINISHED, 0, NULL);
#ifdef VERBOSE
    printf("Updates finished\n");
#endif
}

/*
 * processUpdReq - Process individual Update Request
 */
static procStat processUpdReq(int conid, int itemNum)
{
    hsGeneral *hsG;
    int sndStat;
    updStruct *updStP;
    procStat stat;
    char saveCat[HS_MAX_CATEGORY_LENGTH+1];
    char tmpCat[HS_MAX_CATEGORY_LENGTH+3];
    char c_space[4];
    char *tt;

#ifdef VERBOSE
    printf("processUpdReq called\n");
#endif

    switch ((UpdateList[conid]->updStatPtr)[itemNum]) {
        case PROCESSED_PARTIALLY:
    	    updStP = &(UpdateList[conid]->updStructPtr[itemNum]);
            hsG = GetItemByPtrID(updStP->itemID);
#ifdef LINKED_WITH_HBOOK
    	    if (hsG == NULL)
    	    	hsG = GetHBItemByID(updStP->itemID);
#endif /* LINKED_WITH_HBOOK */
            if (hsG == NULL) {		/* item has been deleted */
         	(UpdateList[conid]->updStatPtr)[itemNum] = PROCESSED;
         	stat.continueProcessing = TRUE;
         	stat.incItemsProcessed = TRUE;
         	return (stat);
            }
            if (hsG->type != HS_NTUPLE)
            	break;
            /* partially processed n-tuple updates are processed the same
               way as unprocessed ones */
               
        case NOT_PROCESSED:
    	    /* determine that the item exists and if an update is needed */
    	    updStP = &(UpdateList[conid]->updStructPtr[itemNum]);
    	    hsG = GetItemByPtrID(updStP->itemID);
#ifdef LINKED_WITH_HBOOK
    	    if (hsG == NULL)
    	    	hsG = GetHBItemByID(updStP->itemID);
#endif /* LINKED_WITH_HBOOK */
    	    if (hsG == NULL) {
/*		fprintf(stderr,
 *       "hs_update: Ignoring request for nonexistant item from HistoScope.\n");
 */
         	(UpdateList[conid]->updStatPtr)[itemNum] = PROCESSED;
         	stat.continueProcessing = FALSE;
         	stat.incItemsProcessed = TRUE;
         	return (stat);
    	    }
#ifdef LINKED_WITH_HBOOK
	    if (hsG->hbookID != 0) {
		if (setupHbookReq(hsG, updStP) == PROCESSED) {
		    (UpdateList[conid]->updStatPtr)[itemNum] = PROCESSED;
         	    stat.continueProcessing = TRUE;
         	    stat.incItemsProcessed = TRUE;
         	    return (stat);
		}
	    }
	    else {
#endif /* LINKED_WITH_HBOOK */
		if (!needToUpdate(hsG, updStP, conid)) {
         	    (UpdateList[conid]->updStatPtr)[itemNum] = PROCESSED;
         	    stat.continueProcessing = TRUE;
         	    stat.incItemsProcessed = TRUE;
         	    return (stat);
		}  				/* skip this item	     */
#ifdef LINKED_WITH_HBOOK
	    }
#endif /* LINKED_WITH_HBOOK */
	    /* send item data to Histo-Scope process  */
	    sndStat = sendToHS(conid, HERE_IS_DATA, hsG->id, hsG);
#ifdef LINKED_WITH_HBOOK
	    if (hsG->hbookID != 0) {
		FreeHBData(hsG, 1);			  /* free HBOOK data */
		if (hsG->type == HS_NTUPLE) {
		 /*
		 ** Get the number of entries from Hbook. Note, we must 
		 ** be sure to get back to the directory where the 
		 ** the user is...   
		 */
	            memset(saveCat, 0, HS_MAX_CATEGORY_LENGTH+1);
	            hcdirw_(saveCat, "R", HS_MAX_CATEGORY_LENGTH, 1);
	            strcpy(c_space, "   ");
	            tt = tmpCat; strcpy(tmpCat,"//"); tt+=2;
	            strcpy(tt, hsG->category );
	            hcdir_(tmpCat, c_space, strlen(tmpCat),
	                             strlen(c_space));
		    hnoent_(&hsG->hbookID, &((hsNTuple *)hsG)->n); /*restore n*/
	            /* set directory back to where user had it */
	            hcdir_(saveCat,  c_space, strlen(saveCat), 4);
	         }
	    }
#endif /* LINKED_WITH_HBOOK */
	    if (sndStat != COM_OK) {	 /* if error, end update for this HS */
#ifdef VERBOSE
		printf("Communication error\n");
#endif		
         	stat.continueProcessing = FALSE;
         	stat.incItemsProcessed = FALSE;
         	return (stat);
	    }
	    if (hsG->type == HS_NTUPLE &&
	    	    ((hsCNTuple *)hsG)->toTuple < ((hsNTuple *)hsG)->n) {
		updStP->updateLevel = ((hsCNTuple *)hsG)->toTuple;
	    	(UpdateList[conid]->updStatPtr)[itemNum]
	    	    		    = PROCESSED_PARTIALLY;
         	stat.incItemsProcessed = FALSE;
	    }
	    else {
	    	(UpdateList[conid]->updStatPtr)[itemNum] = PROCESSED;
		if (hsG->type == HS_NTUPLE)
		    sendToHS(conid, END_NTUPLE_DATA, hsG->id, NULL);
	    	stat.incItemsProcessed = TRUE;
#ifdef COM_DEBUG
	    	printf("...item %d: id %d, status = PROCESSED for Histo-Scope %d\n",
	    		itemNum, hsG->id, conid);
#endif
	    }
	    if (UnfinMsgList[conid] != NULL)
	    {
#ifdef VERBOSE
		printf("UnfinMsgList[conid] != NULL\n");
#endif		
	        stat.continueProcessing = FALSE;
	    }
	    else
	    	stat.continueProcessing = TRUE;
	    return (stat);

        default:
            break;
            
    }	/* end of switch stmt */
    
    fprintf(stderr, "hs_update: Internal Error.  Calling hs_complete().\n");
    hsComplete();
    stat.continueProcessing = FALSE;
    stat.incItemsProcessed = FALSE;
    return (stat);
}

/*
 * hsComplete - Close all connections to Histo-Scope processes.
 */
static void hsComplete(void)
{
    int i;
    
    for (i = 0; i < NumOfItems; ++i)
    	if (HistoPtrList[i] != NULL)
    	    FreeItem(HistoPtrList[i]);	/* free items in list		      */ 
    free(HistoPtrList);			/*     and the list itself	      */
    HistoPtrList = NULL;		/* signify list is empty              */
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION) {
    	    sendToHS(i, CLIENT_DONE, 0, NULL);
	    closeScope(i);
	}				/* close Histo-Scope connection & mem */
    close (AccComFD);			/* close accepting socket	      */
    AccComFD = 0;
    ServerPort = -1;

    InitializedAndActive = FALSE;
    NumOfConnections = 0;		/* reset number of scoping processes  */
    NumOfItems = 0;			/* and number of histo items created  */
    RemoveIDFile();			/* remove registration in tmp direct. */
    signal(SIGPIPE, SIG_DFL);		/* set broken pipe handler to default */
    
#ifdef LINKED_WITH_HBOOK
    HBInitialized = FALSE;
    FreeEntireHBList();
#endif /* LINKED_WITH_HBOOK */
} 

/*
 * needToUpdate - Return True or False if a histo-scope item needs updating 
 *                and set from/to tuple numbers for an n-tuple.
 */
static int needToUpdate(hsGeneral * hsG, updStruct *updStP, int conid)
{
    int need_to_update;
    unsigned updateMask;

    switch (hsG->type) {
	case HS_1D_HISTOGRAM:
	    /* check both fill count and whether histogram has been reset */
	    need_to_update = ((hs1DHist *)hsG)->count != updStP->updateLevel
	    	|| (((hsC1DHist *)hsG)->resetFlag & (1 << conid));
	    ((hsC1DHist *)hsG)->resetFlag &= ~ (1 << conid);  /* clear flag */
	    return (need_to_update);
	case HS_2D_HISTOGRAM:
	    /* check both fill count and whether histogram has been reset */
	    need_to_update = ((hs2DHist *)hsG)->count != updStP->updateLevel
	    	|| (((hsC2DHist *)hsG)->resetFlag & (1 << conid));
	    ((hsC2DHist *)hsG)->resetFlag &= ~ (1 << conid);  /* clear flag */
	    return (need_to_update);
	case HS_3D_HISTOGRAM:
	    /* check both fill count and whether histogram has been reset */
	    need_to_update = ((hs3DHist *)hsG)->count != updStP->updateLevel
	    	|| (((hsC3DHist *)hsG)->resetFlag & (1 << conid));
	    ((hsC3DHist *)hsG)->resetFlag &= ~ (1 << conid);  /* clear flag */
	    return (need_to_update);
	case HS_NTUPLE:	
	    updateMask = (unsigned)1 << conid;
	    need_to_update = ((hsNTuple *)hsG)->n > updStP->updateLevel ||
		((hsNTuple *)hsG)->n > 0 && 
		(((hsCNTuple *)hsG)->needsCompleteUpdate & updateMask);
	    if (need_to_update) {
		if (((hsCNTuple *)hsG)->needsCompleteUpdate & updateMask) {
		    updStP->updateLevel = 0;
		    ((hsCNTuple *)hsG)->needsCompleteUpdate -= updateMask;
		}
		((hsCNTuple *)hsG)->fromTuple = updStP->updateLevel;
	    	((hsCNTuple *)hsG)->toTuple = rtnNforTuple(hsG,
		    			    	updStP->updateLevel);
#ifdef COM_DEBUG
		printf("    needToUpdate: fromTuple = %d, toTuple = %d.\n",
		    ((hsCNTuple *)hsG)->fromTuple, ((hsCNTuple *)hsG)->toTuple);
#endif
	    }
	    return (need_to_update);
	case HS_INDICATOR:
	    return (((hsIndicator *)hsG)->value != updStP->updateLevelF)
	    	    || (((hsIndicator *)hsG)->valueSet != updStP->updateLevel);
	case HS_CONTROL:
	case HS_TRIGGER:
	case HS_GROUP:
	    return 0;		/* Controls, triggers, groups are not updated */
	default:
	    fprintf(stderr,
	    	"hs_update: Found item of unknown type, skipped\n");
	    return (FALSE);
    }
}

/*
 * readNSaveReqs - Read and save incoming update requests
 */
static void readNSaveReqs(int conid, int numOfItems, int bytesToRead)
{
    XDR xdrs;
    char *updateBuf = NULL;
    int i, byteCount;
    extern int errno;
    
    /* Read remainder of REQUEST_UPDATES message.  Two reads are OK for this *
     * message because Histo-Scope wrote both parts of it as one chunk       */
    updateBuf = malloc(bytesToRead);
    if (updateBuf == NULL) {
    	closeScope(conid);
    	fprintf(stderr, "hs_update: malloc failed.  Closed HistoScope %d.",
    		conid);
    	return;
    }

    byteCount = 0;
    while (byteCount != bytesToRead) {
	i = socket_read(ComFD[conid], updateBuf+byteCount, bytesToRead-byteCount);
	if (i < 0) {
	    if (errno != EINTR && errno != EAGAIN) {
		fprintf(stderr,
			"hs: Error reading update message from HistoScope %d. (%d)\n",
			conid, i);
		fprintf(stderr, "bytesToRead = %d\n", bytesToRead);
		fprintf(stderr, "byteCount = %d\n", byteCount);
		fprintf(stderr, "System call status: %s\n", strerror(errno));
		closeScope(conid);
		fflush(stderr);
		return;
	    }
	} else {
	    byteCount += i;
	}
    }

    /* Create and fill UpdateList with Histo-Scope's requests */
    xdrmem_create(&xdrs, updateBuf, bytesToRead, XDR_DECODE);
    UpdateList[conid] = (struct updateList *) malloc(sizeof(struct updateList));
    if (UpdateList[conid] == NULL) {
    	free(updateBuf);
    	closeScope(conid);
	xdr_destroy (&xdrs);
    	fprintf(stderr, "hs_update: malloc failed.  Closed HistoScope %d.",
    		conid);
    	return;
    }
    UpdateList[conid]->numItems       = numOfItems;
    UpdateList[conid]->itemsProcessed = 0;
    UpdateList[conid]->notFinished    = TRUE;
    UpdateList[conid]->updStructPtr   =
    		           (updStruct *) malloc(numOfItems * sizeof(updStruct));
    UpdateList[conid]->updStatPtr     =
    		                 (int *) malloc(numOfItems * sizeof(int));
    if (UpdateList[conid]->updStructPtr == NULL 
    	    || UpdateList[conid]->updStatPtr == NULL) {
    	free(updateBuf);
    	if (UpdateList[conid]->updStructPtr != NULL) {
    	    free(UpdateList[conid]->updStructPtr);
    	}
    	free(UpdateList[conid]);
    	UpdateList[conid] = NULL;
    	closeScope(conid);
	xdr_destroy (&xdrs);
    	fprintf(stderr, "hs_update: malloc failed.  Closed HistoScope %d.",
    		conid);
    	return;
    }
    for (i = 0; i < numOfItems; ++i) {
	if (!xdr_updStruct(&xdrs, &((UpdateList[conid]->updStructPtr)[i]))) {
    						    /* translate update list */
	    fprintf(stderr,
		"hs: Error xdr translating update message from HistoScope.\n");
	    closeScope(conid);
	    xdr_destroy (&xdrs);
	    return;
	}
#ifdef COM_DEBUG
	printf( "    item %d requested (type = %d), update level = %d, (%f)\n",
		((UpdateList[conid]->updStructPtr)[i]).itemID,
		((UpdateList[conid]->updStructPtr)[i]).type,
		((UpdateList[conid]->updStructPtr)[i]).updateLevel,
		((UpdateList[conid]->updStructPtr)[i]).updateLevelF );
#endif
	(UpdateList[conid]->updStatPtr)[i] = NOT_PROCESSED;
    }
    xdr_destroy (&xdrs);
    free(updateBuf);
}

/*
 * procUnfinUpds - Process Unfinished Update Requests and Message Writes
 */
static int procUnfinUpds(void)
{
    int i, conid, anyUnfin = 0;
    procStat stat;

#ifdef VERBOSE
    printf("procUnfinUpds called\n");
#endif
    for (conid = 0; conid < MAX_NUM_SCOPES; ++conid) {
    
	/* Process each unfinished message Write for this Histo-Scope */
	while (UnfinMsgList[conid] != NULL) {
	    ++anyUnfin;			/* flag something was done */
	    if (writeUnfinMsg(conid) != COM_OK)
	    {
#ifdef VERBOSE
		printf("writeUnfinMsg failed\n");
#endif	    
	    	break;	/* skip remainder for this HS if Write not OK */
	    }
	}
	if (UnfinMsgList[conid] != NULL)
	{
#ifdef VERBOSE
	    printf("There are still unfin Writes\n");
#endif	    
	    continue;	/* skip remainder for this HS if still unfin Writes */
	}
	if (UpdateList[conid] == NULL)
    	    continue;		/* no updates to process for this Histo-Scope */

#ifdef VERBOSE
	printf("Continue processing unfinished update request\n");
#endif

	/* Process each unfinished update request for this Histo-Scope */
	for (i = 0; i < UpdateList[conid]->numItems; ++i) {
	    if ((UpdateList[conid]->updStatPtr)[i] == PROCESSED)
		continue;   /* skip item */
#ifdef COM_DEBUG
	    printf("Continuing processing for Histo-Scope %d, Upditem %d...\n",
	    	conid, i);
#endif
	    ++anyUnfin;			/* flag something was done */
	    do {
		stat = processUpdReq(conid, i);
		if (stat.incItemsProcessed) {
		    ++(UpdateList[conid]->itemsProcessed);
		    break;
		}
	    } while (stat.continueProcessing);
	}   /* next item on this Histo-Scope's list */

	/* Finished with Update for this Histo-Scope */
	if (UpdateList[conid] == NULL)
    	    continue;	       /* error, updates deleted for this Histo-Scope */
#ifdef COM_DEBUG
	printf("Histoscope %d: %d items processed.  itemsProcessed = %d\n",
		conid, i, UpdateList[conid]->itemsProcessed);
#endif
	if (UpdateList[conid]->itemsProcessed >= UpdateList[conid]->numItems) {
#ifdef COM_DEBUG
	    if (UpdateList[conid]->itemsProcessed
	    		> UpdateList[conid]->numItems)
	    	printf("*** Warning!! items processed > num of items !! ***\n");
#endif
	    freeUpdateList(conid);
	    sendToHS(conid, UPDATES_FINISHED, 0, NULL);
	    ++anyUnfin;			/* flag something was done */
	}
    }   /* next Histo-Scope */
    return (anyUnfin);		/* return whether something was accomplished */
}

/*
 * ValidStr - Validate strings supplied by user
 *
 *	      returns: pointer to valid same or new truncated string
 *
 *  Note: ** copy string returned, if needed, before calling ValidStr again **
 */
const char *ValidStr(const char *string, int max_length,
                     const char *routine, const char *strKind)
{
    static char str[HS_MAX_CATEGORY_LENGTH+1];	     /* make longest string */
    static char str1[1] = "";
    
    if (string == NULL)
    	return str1;			   /* return empty string	    */
    if ((int)strlen(string) <= max_length)
    	return string;			   /* return pointer to same string */
    fprintf(stderr,
    	"%s: Error. Specified %s string is too long, truncating\n     ->%s\n",
    	routine, strKind, string);
    memset(str, 0, HS_MAX_CATEGORY_LENGTH+1);
    return strncpy(str, string, max_length); /* return ptr to trunc. string */
}

/*
** Validate a category string.  Prints a descriptive error and returns
** 0 (false) if category is not valid.  routineName is to help identify the
** location of error to the user.  If dotDotDot is nonzero (true), this
** routine will allow category constructs ending in "...".  Otherwise this
** character combination will be flagged as an error.  If a valid ... occurs
** in the category specified, -1 will be returned, otherwise 1 is returned.
*/
int CheckValidCategory(const char *category, const char *routineName, int dotDotDot)
{
    static char validChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz1234567890/~!@#$%^&*()_+=-`\"\'\t?><,. ";
    char *strDots, *error = NULL;
    unsigned len;
    
    if (category == NULL)
    	return 1;
    len = strlen(category);
    if (len == 0U)
        return 1;
    strDots = strstr(category, "...");
    if (len >= HS_MAX_CATEGORY_LENGTH)
    	error = "is too long";
    else if (strspn(category, validChars) != len)
    	error = "contains invalid characters";
    else if (strstr(category, "//") != NULL)
    	error = "contains \"//\"";
    else if (category[0] == '/')
    	error = "contains leading slash";
    else if (category[len-1] == '/')
    	error = "contains trailing slash";
    else if ((dotDotDot == 0 && strDots != NULL) 
    	  || (dotDotDot != 0 && strDots != NULL && strDots != category + len-3))
    	error = "contains invalid \"...\"";
    	
    if (error != NULL) {
    	fprintf(stderr, "%s: Error in category %s: %s\n",
    		routineName, error, category);
    	return 0;
    } else {
    	return (strDots == NULL ? 1 : -1);
    }
}

/*
** CopyNtrim -  Copy "fromString" to a malloc'd new string,
** 		trimming off leading and trailing spaces & tabs.
**		The newly malloc'd string is returned.
** 		If fromString is NULL, NULL is returned.
*/
char *CopyNtrim(const char *fromString)
{
    char *toString;
    const char* c;
    int len, i;
    
    if (fromString == NULL)
    	return NULL;
    toString = (char *) malloc(strlen(fromString)+1);
    
    /* Find the first non-white character */
    for (c=fromString; *c == ' ' || *c == '\t'; c++);

    /* Copy the remainder of fromString to toString */
    strcpy(toString, c);
    
    /* Remove trailing spaces and tabs by converting to nulls */
    len = strlen(toString);
    if (len == 0)			/* special case for empty strings */
    	return toString;
    for (i = len-1; i >= 0; --i) {
    	if (isspace(toString[i]))
    	    toString[i] = '\0';
    	else
    	    break;
    }
    return toString;
}    

/*
 * collectNtupleData - Collect data from the data and extensions sections of
 *                     an n-tuple and put it into a new, larger, data section.
 *                     Free memory. Reset chunk sizes and pointers accordingly.
 */
static int collectNtupleData(hsNTuple *nTuple)
{
    float *newData;
    int i = 0, j, k;
    
    newData = (float *) malloc(nTuple->n * nTuple->nVariables * sizeof(float));
    					     /* allocate new data area       */
    if (newData == NULL)
    	return -1;
    if (nTuple->data != NULL)
	for (i = 0; i < nTuple->chunkSzData * nTuple->nVariables; ++i)
	    newData[i] = nTuple->data[i];    /* copy data into new data area */
    for (k = 0; k < 4; ++k) {		     /* copy data from each ext area */
	for (j = 0; j < nTuple->chunkSzExt * nTuple->nVariables; ++j)
	    newData[i++] = nTuple->extensions[k][j];
	free(nTuple->extensions[k]);
	nTuple->extensions[k] = NULL;
    }
    if (nTuple->data != NULL)
	free(nTuple->data);		    /* free "old" data area          */
    nTuple->data = newData;		    /* reset data pointer	     */
    nTuple->chunkSzData = nTuple->n;	    /* reset chunk sizes             */
    nTuple->chunkSzExt *= 2;		    /* increase extension chunk size */
    return 0;
}

/*
 * sendHSData - Send HistoScope a message with data on an item
 */
static int sendHSData(int connect_id, msgStruct *message, hsGeneral *item)
{
    char *databuf = NULL;
    XDR xdrs, xdrs1;
    bool_t s;
    int r = 0, datsiz = 0, dataId = 0, tot_writ;
    enum utype u_type;
    char msgbuf[sizeof(msgStruct)];
    hsNTuple *nT;

    SocketIsBlocked = 0;

    switch (message->code) {
    	case ITEM_TO_LIST:
    	case NEW_ITEM:
	    switch (item->type) {
	    	case HS_1D_HISTOGRAM:
	    	    datsiz = sizeof(hs1DHist) + HS_MAX_TITLE_LENGTH + 8
			+ HS_MAX_CATEGORY_LENGTH + 2 * (HS_MAX_LABEL_LENGTH +4);
		    u_type = ITEMLIST_1DHIST;
		    break;
	    	case HS_2D_HISTOGRAM:
	    	    datsiz = sizeof(hs2DHist) + HS_MAX_TITLE_LENGTH + 8
			+ HS_MAX_CATEGORY_LENGTH + 3 * HS_MAX_LABEL_LENGTH + 12;
		    u_type = ITEMLIST_2DHIST;
		    break;
	    	case HS_3D_HISTOGRAM:
	    	    datsiz = sizeof(hs3DHist) + HS_MAX_TITLE_LENGTH + 8
			+ HS_MAX_CATEGORY_LENGTH + 4 * HS_MAX_LABEL_LENGTH + 16;
		    u_type = ITEMLIST_3DHIST;
		    break;
	    	case HS_NTUPLE:
	    	    nT = (hsNTuple *)item;
	    	    datsiz = sizeof(hsNTuple) +  HS_MAX_TITLE_LENGTH + 8 +
	    	    	     HS_MAX_CATEGORY_LENGTH + ( (HS_MAX_NAME_LENGTH + 4)
	    	    	     * (((hsNTuple *)item)->nVariables) );
		    u_type = ITEMLIST_NTUPLE;
		    break;
	    	case HS_INDICATOR:
	    	    datsiz = sizeof(hsIndicator) + HS_MAX_TITLE_LENGTH
			+ HS_MAX_CATEGORY_LENGTH + 8;
		    u_type = ITEMLIST_INDICATOR;
		    break;
	    	case HS_CONTROL:
	    	    datsiz = sizeof(hsControl) + HS_MAX_TITLE_LENGTH
			+ HS_MAX_CATEGORY_LENGTH + 8;
		    u_type = ITEMLIST_CONTROL;
		    break;
	    	case HS_TRIGGER:
	    	    datsiz = sizeof(hsTrigger) + HS_MAX_TITLE_LENGTH
			+ HS_MAX_CATEGORY_LENGTH + 8;
		    u_type = ITEMLIST_TRIGGER;
		    break;
	    	case HS_GROUP:
	    	    datsiz = sizeof(hsGroup) + HS_MAX_TITLE_LENGTH
			+ HS_MAX_CATEGORY_LENGTH + 8 
			+ 2 * sizeof(int) * ((hsGroup *)item)->numItems;
		    u_type = ITEMLIST_GROUP;
		    break;
	    	default:
	    	    fprintf(stderr,
	       	    	"Internal error: sendHSData - bad item type: %d.\n",
	       		item->type);
	    	    return COM_FAILURE;
	    }
	    break;
    	case HERE_IS_DATA:
    	case CONTROL_READ:
	    switch (item->type) {
	    	case HS_1D_HISTOGRAM:
	    	    r = 1;
	    	    if (((hsC1DHist *)item)->sendErrs) {
	    	    	if (((hs1DHist *)item)->pErrs != NULL)
	    	    	    ++r;
	    	    	if (((hs1DHist *)item)->mErrs != NULL)
	    	    	    ++r;
	    	    }
	    	    datsiz = sizeof(hs1DHist) + r * (sizeof(float)
	    	    			 * ((hs1DHist *)item)->nBins);
		    u_type = DATA_1DHIST;
		    break;
	    	case HS_2D_HISTOGRAM:
	    	    r = 1;
	    	    if (((hsC2DHist *)item)->sendErrs) {
	    	    	if (((hs2DHist *)item)->pErrs != NULL)
	    	    	    ++r;
	    	    	if (((hs2DHist *)item)->mErrs != NULL)
	    	    	    ++r;
	    	    }
	    	    datsiz = sizeof(hs2DHist) + r * (sizeof(float) * 
	    	      ((hs2DHist *) item)->nXBins * ((hs2DHist *)item)->nYBins);
		    u_type = DATA_2DHIST;
		    break;
	    	case HS_3D_HISTOGRAM:
	    	    r = 1;
	    	    if (((hsC3DHist *)item)->sendErrs) {
	    	    	if (((hs3DHist *)item)->pErrs != NULL)
	    	    	    ++r;
	    	    	if (((hs3DHist *)item)->mErrs != NULL)
	    	    	    ++r;
	    	    }
	    	    datsiz = sizeof(hs3DHist) + r * (
			sizeof(float) * 
			((hs3DHist *) item)->nXBins *
			((hs3DHist *)item)->nYBins * 
			((hs3DHist *)item)->nZBins);
		    u_type = DATA_3DHIST;
		    break;
	    	case HS_NTUPLE:
	    	    nT = (hsNTuple *)item;
	    	    if (((hsCNTuple *)item)->toTuple 
	    	    			< ((hsCNTuple *)item)->fromTuple) {
	    		fprintf(stderr,
	       	    	    "hs: Internal error (sendHSData: to < from).\n");
	    		return COM_FAILURE;
		    } 
	    	    datsiz = sizeof(hsNTuple) + ( sizeof(float) * 
	    	    	 ( ((hsCNTuple *)item)->toTuple -  
	    	         ((hsCNTuple *)item)->fromTuple) * nT->nVariables );
		    u_type = DATA_NTUPLE;
		    dataId = nT->id;
#ifdef VERBOSE
		    printf("Will try to send %d bytes\n", datsiz);
#endif
	    	    break;
	    	case HS_INDICATOR:
	    	    datsiz = sizeof(hsIndicator) + 6;
		    u_type = DATA_INDICATOR;
		    break;
	    	case HS_CONTROL:
	    	    datsiz = sizeof(hsControl) + 6;
		    u_type = DATA_CONTROL;
		    break;
	    	case HS_CONFIG_STRING:
	    	    datsiz = ((hsConfigString *)item)->stringLength + 32;
	    	    u_type = DATA_CONFIG;
	    	    break;
	    	default:
	    	    fprintf(stderr,
	       	    	"Internal error: sendHSData - Bad item type: %d.\n",
	       		item->type);
	    	    return COM_FAILURE;
	    }

    	    break;
    	    
    	default:
#ifdef COM_DEBUG
	    printf("**** Bad Message Code ****\n");
#endif /*COM_DEBUG*/
	    return COM_FAILURE;
    }
    databuf = malloc(datsiz);
    if (databuf == NULL) {
	closeScope(connect_id);
	fprintf(stderr,
	    "hs_update: malloc failed (in sendHSData). Closed HistoScope %d.\n",
	    	connect_id);
#ifdef COM_DEBUG
        printf("data msg size = %d, data buffer size = %d, xdrs.handy = %d.\n",
	    message->dataLength, datsiz, xdrs.x_handy);
#endif
	return COM_FAILURE;
    }
    xdrmem_create(&xdrs1, databuf, datsiz, XDR_ENCODE);
    if (!xdr_Item(&xdrs1, &item, &u_type)) {
	fprintf(stderr,
	    "hs: Error in xdr translation (sendHSData).\n\
            data msg size = %d, data buffer size = %d, xdrs.handy = %d.\n",
	    message->dataLength, datsiz, xdrs.x_handy);
	xdr_destroy (&xdrs1);
	free(databuf);
	closeScope(connect_id);
	return COM_FAILURE;
    }
    message->dataLength = xdr_getpos(&xdrs1);	/* size of data msg */
#ifdef COM_DEBUG
    printf("data msg size = %d, data buffer size = %d, u_type = %d (%s),\n\
    	    confirm = %d\n",
	    message->dataLength, datsiz, u_type, UtypeCodes[u_type],
	    message->confirm);
#endif
    xdrmem_create(&xdrs, msgbuf, sizeof(msgbuf), XDR_ENCODE);
    s = xdr_msgStruct(&xdrs, message);
    if (s && UnfinMsgList[connect_id] != NULL) {
#ifdef COM_DEBUG
    	printf("\n***adding message & data as unfinished: HS %d***\n\n",
    	    connect_id);
#endif
	addUnfinMsg(connect_id, msgbuf, sizeof(msgbuf), TRUE, 0, dataId);
	addUnfinMsg(connect_id, databuf, message->dataLength, TRUE, 0, dataId);
	xdr_destroy (&xdrs1);
	xdr_destroy (&xdrs);
	free(databuf);
	return COM_OK;
    }
    if ( s  && ( (r = (socket_write(ComFD[connect_id], msgbuf, sizeof(msgbuf))))
    		 == sizeof(msgbuf) ) ) {
    	/* both translation and write succeeded for message */
#ifdef COM_DEBUG
	printf("r = %d,\n    msgbuf = ", r);
	{
	    int yyy = 0;
	    for (yyy = 0; yyy < 16; ++yyy)
		printf(" %x", msgbuf[yyy]);
	    printf("\n");
	}
#endif
	xdr_destroy (&xdrs);
	/* write data */
	for (tot_writ = 0; tot_writ < message->dataLength;  ) {
	    r = socket_write(ComFD[connect_id], databuf + tot_writ,
#ifndef VMS
				message->dataLength - tot_writ);     /* unix */
#else
				message->dataLength - tot_writ > 65535 /* vms */
				      ? 65535 : message->dataLength - tot_writ);
				/* Multinet bug: size cannot be > short word */
#endif /*VMS*/
#ifdef COM_DEBUG
	    printf("r = %d, databuf+tot_writ = %x\n", r, databuf+tot_writ);
#endif
	    if (r == -1 && socket_errno == EWOULDBLOCK) {
#ifdef COM_DEBUG
	    	printf("\n***adding data as unfinished message: HS %d***\n\n",
	    		connect_id);
#endif
#ifdef VERBOSE
		printf("Socket is now blocked\n");
#endif
		SocketIsBlocked = 1;
	    	addUnfinMsg(connect_id, databuf, message->dataLength, TRUE,
	    		tot_writ, dataId);
	    	xdr_destroy (&xdrs1);
	    	free(databuf);
	    	return COM_OK;
	    }
	    if (r <= 0) {
		xdr_destroy (&xdrs1);
  		free(databuf);
		fprintf(stderr,
		    "hs: Error writing data to Histo-Scope %d. (%d) %s\n",
		    connect_id, r, strerror(socket_errno));
		closeScope(connect_id);
		return COM_FAILURE;
	    }
	    tot_writ += r;
	}
	xdr_destroy (&xdrs1);
	free(databuf);
	return COM_OK;
    }
    /* Error translating message or writing it */
    if (s) {
    	/* Write problem */
    	if (r == -1 && socket_errno == EWOULDBLOCK) {
    	    /* Write would block */
#ifdef COM_DEBUG
    	    printf("\n***adding message & data as unfinished: HS %d***\n\n",
    	    	connect_id);
#endif
	    SocketIsBlocked = 1;
	    addUnfinMsg(connect_id, msgbuf, sizeof(msgbuf), TRUE, 0, dataId);
	    addUnfinMsg(connect_id, databuf, message->dataLength, TRUE, 0, dataId);
	    xdr_destroy (&xdrs1);
	    xdr_destroy (&xdrs);
	    free(databuf);
	    return COM_OK;
    	}
    	else if (r > 0 && r != sizeof(msgbuf)) {
	    /* only part of message was written */
#ifdef COM_DEBUG
	    printf("\n***adding part of msg as unfinished message: HS %d***\n\n",
	    		connect_id);
#endif
	    addUnfinMsg(connect_id, msgbuf, sizeof(msgbuf), TRUE, r, dataId);
#ifdef COM_DEBUG
    	    printf("\n**adding its data as unfinished: HS %d***\n\n",connect_id);
#endif
	    addUnfinMsg(connect_id, databuf, message->dataLength, TRUE, 0, dataId);
	    xdr_destroy (&xdrs1);
	    xdr_destroy (&xdrs);
	    free(databuf);
	    return COM_OK;
	}
    }
    /* Translation Error or other Write Error */
    xdr_destroy (&xdrs);
    xdr_destroy (&xdrs1);
    if (databuf != NULL)
  	free(databuf);
    fprintf(stderr,
	"hs: Error writing or translating message to Histo-Scope %d. (%d:%d)\n",
	connect_id, s, r);
    closeScope(connect_id);
    return COM_FAILURE;
}

/*
 * sendToHS - Send message to Histo-Scope
 */
static int sendToHS(int conid, int messageCode, int id, hsGeneral *item)
{
    msgStruct message;
    char msgbuf[sizeof(msgStruct)];
    XDR xdrs;
    int r, dataId = 0;

    if (ComFD[conid] == NO_CONNECTION)
	return COM_FAILURE;

    message.code = messageCode;
    message.confirm = CONFIRM_CODE;
    message.itemID = id;
    message.dataLength = 0;
    switch (messageCode) {		     /* Messages from Histo-Scope:    */
	case HERE_IS_DATA:
#ifdef COM_DEBUG
	    printf("Sending here is data msg (id = %d) to Histo-Scope %d.\n", 
	    	message.itemID, conid);
#endif
	    return (sendHSData(conid, &message, item));
	case UPDATES_FINISHED:
#ifdef COM_DEBUG
	    printf("Sending updates finished msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case ITEM_TO_LIST:
#ifdef COM_DEBUG
	    printf("Sending item to list msg (id = %d) to Histo-Scope %d.\n", 
	    	message.itemID, conid);
#endif
	    return (sendHSData(conid, &message, item)); 
	case NEW_ITEM:
#ifdef COM_DEBUG
	    printf(
	        "Sending new item (to list) msg (id = %d) to Histo-Scope %d.\n",
	    	 message.itemID, conid);
#endif
	    return (sendHSData(conid, &message, item)); 
	case BEGIN_LIST:
#ifdef COM_DEBUG
	    printf("Sending begin list msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case END_OF_LIST:
#ifdef COM_DEBUG
	    printf("Sending end of list msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case END_NTUPLE_DATA:
	    dataId = id;
#ifdef COM_DEBUG
	    printf("Sending end of ntuple data msg to Histo-Scope %d.\n",conid);
#endif
	    break;
	case ITEM_DELETED:
#ifdef COM_DEBUG
	    printf("Sending item deleted msg (id = %d) to Histo-Scope %d.\n", 
	    	message.itemID, conid);
#endif
	    break; 
	case INHIBIT_RESET_REFRESH:
#ifdef COM_DEBUG
	    printf("Sending inhibit reset refresh msg (id = %d) to Histo-Scope %d.\n",
	    	message.itemID, conid);
#endif
	    break; 
	case ALLOW_RESET_REFRESH:
#ifdef COM_DEBUG
	    printf("Sending allow reset refresh msg (id = %d) to Histo-Scope %d.\n",
	    	message.itemID, conid);
#endif
	    break; 
	case NTUPLE_RESET:
#ifdef COM_DEBUG
	    printf("Sending ntuple reset msg (id = %d) to Histo-Scope %d.\n", 
	    	message.itemID, conid);
#endif
	    break; 
	case REQ_LAST_UPDATE:
#ifdef COM_DEBUG
	    printf("Sending client confirming msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case CLIENT_CONFIRMING:
#ifdef COM_DEBUG
	    printf("Sending client confirming msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case CONTROL_READ:
#ifdef COM_DEBUG
	    printf("Sending control read msg (id = %d) to Histo-Scope %d.\n", 
	    	message.itemID, conid);
#endif
	    return (sendHSData(conid, &message, item)); 
	case TRIGGER_READ:
#ifdef COM_DEBUG
	    printf("Sending trigger read msg (id = %d) to Histo-Scope %d.\n", 
	    	message.itemID, conid);
	    ++NumTriggersRead;
	    printf("   numRead = %d, numSet = %d\n", NumTriggersRead,
	    	 NumTriggersSet);
#endif
	    break; 
	case CLIENT_DONE:
#ifdef COM_DEBUG
	    printf("Sending client done msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case CLIENT_WAITING:
#ifdef COM_DEBUG
	    printf("Sending client waiting msg to Histo-Scope %d.\n", conid);
#endif
	    break;
	case REQ_CAPTIVE_HISTO_EXIT:
#ifdef COM_DEBUG
	    printf("Sending request captive histo exit msg to Histo-Scope %d.\n"
	    	, conid);
#endif
	    break;
	default:
	    fprintf(stderr,
	       "Program bug, please report: sendToHS - bad msg code.\n");
	    hsComplete();
	    return COM_FAILURE;
    }
    xdrmem_create(&xdrs, msgbuf, sizeof(msgbuf), XDR_ENCODE);
    if (xdr_msgStruct(&xdrs, &message)) { 
    	if (UnfinMsgList[conid] != NULL) {
    	    addUnfinMsg(conid, msgbuf, sizeof(msgbuf), TRUE, 0, dataId);
	    xdr_destroy (&xdrs);
    	    return COM_OK;
    	}
    	if ((r=socket_write(ComFD[conid], msgbuf, sizeof(msgbuf)))
		== sizeof(msgbuf)) {
	    xdr_destroy (&xdrs);
	    return COM_OK;
	}
	if (r == -1 && socket_errno == EWOULDBLOCK) {
    	    /* Write would block */
	    addUnfinMsg(conid, msgbuf, sizeof(msgbuf), TRUE, 0, dataId);
	    xdr_destroy (&xdrs);
	    return COM_OK;
    	}
	if (r > 0) {
    	    /* Partial write */
	    addUnfinMsg(conid, msgbuf, sizeof(msgbuf), TRUE, r, dataId);
	    xdr_destroy (&xdrs);
	    return COM_OK;
    	}
    	else 
    	    fprintf(stderr,"hs: Error writing message to Histo-Scope %d\n",
    	    	conid);
    }
    else
        fprintf(stderr,"hs: Error translating message to Histo-Scope %d\n",
    	    	conid);
    xdr_destroy (&xdrs);
    closeScope(conid);
    return COM_FAILURE;
}

/*
 * writeUnfinMsg - write a translated message to a previously EWOULDBLOCK
 *                 Histo-Scope.
 */
static int writeUnfinMsg(int conid)
{
    int rval, amtToWrite;
    unfinMsgListHdr *h;

#ifdef VERBOSE
    printf("writeUnfinMsg called\n");
#endif

    if (ComFD[conid] == NO_CONNECTION || ComFD[conid] <= 0) {
#ifdef COM_DEBUG
	printf("In writeUnfinMsg, ComFD <=0, closing connection with HS %d\n",
		    conid);
#endif
    	closeScope(conid);
    	return (COM_FAILURE);
    }
    while (UnfinMsgList[conid] != NULL) {
    	amtToWrite = UnfinMsgList[conid]->nBytes-UnfinMsgList[conid]->byteOffset;
	SocketIsBlocked = 0;
    	rval = socket_write( ComFD[conid],
    		UnfinMsgList[conid]->msg + UnfinMsgList[conid]->byteOffset,
#ifndef VMS
		amtToWrite );				  /* unix */
#else
		amtToWrite > 65535 ? 65535 : amtToWrite); /* vms */
				/* Multinet bug: size cannot be > short word */
#endif /*VMS*/
#ifdef COM_DEBUG
    	printf(
    	    " ** Writing unfinished message to HS %d of %d bytes: rval = %d.\n",
    	    conid, amtToWrite, rval);
#endif
    	if (rval == -1 && socket_errno == EWOULDBLOCK)
	{
	    SocketIsBlocked = 1;
    	    return (COM_FAILURE);
	}
    	if (rval <=0) {
    	    fprintf(stderr,
    	     "hs_update:  Error writing unfinished message to HistoScope %d:\n",
    	     conid);
    	    socket_perror("hs_update");
    	    closeScope(conid);
    	    return (COM_FAILURE);
    	}
    	if (rval != UnfinMsgList[conid]->nBytes-UnfinMsgList[conid]->byteOffset)
    	    UnfinMsgList[conid]->byteOffset += rval;
    	else {
    	    h = UnfinMsgList[conid]->next;
	    free (UnfinMsgList[conid]->msg);
	    free (UnfinMsgList[conid]);
	    UnfinMsgList[conid] = h;
	}
    }
#ifdef VERBOSE
    printf("writeUnfinMsg succeeded\n");
#endif
    return (COM_OK);
}
/*
 * readHistoCom - Read message from Histo-Scope
 */
static int readHistoCom(int connect_id)
{
    char msgbuf[sizeof(msgStruct)];
    msgStruct message;
    int rval;
    XDR xdrs;
    int task_status = 1, taskNumber = 0;
    char *buf;

    while (1) {
	if (ComFD[connect_id] == NO_CONNECTION)
            return 0;

	memset(msgbuf, 0, sizeof(msgbuf));
	rval = socket_read(ComFD[connect_id], msgbuf, sizeof(msgbuf));
#ifdef COM_DEBUG
	if (rval != -1)
	    printf("rval = %d\n", rval);
#endif
	switch (rval) {
    	    case sizeof(msgbuf):
    		break;
    	    case -1:
    		if (socket_errno != EWOULDBLOCK) {
	    	    socket_perror("hs: Error reading message from HistoScope:");
	    	    closeScope(connect_id);
    		}
    		return 0;
	    case 0:
		fprintf(stderr,
		      "hs: Lost communication with Histo-Scope process %d.\n",
		      connect_id);
		closeScope(connect_id);
    		return 0;
    	    default:
                fprintf(stderr,
		    "hs: Error reading message from Histo-Scope %d: rval = %d.\n",
		    connect_id, rval);
                closeScope(connect_id);
                return 0;
	}
	/* received message from HistoScope process  */
	xdrmem_create(&xdrs, msgbuf, sizeof(msgbuf), XDR_DECODE);
	if (!xdr_msgStruct(&xdrs, &message)) { /* translate message	      */
	    fprintf(stderr,
	    	"hs: Error xdr translating message from HistoScope.\n");
	    closeScope(connect_id);
	    xdr_destroy (&xdrs);
	    return 0;
	}
	xdr_destroy (&xdrs);
	if (message.confirm != CONFIRM_CODE) {  /* confirm message 	      */
	    fprintf(stderr, "hs: Error confirming message from HistoScope.");
	    closeScope(connect_id);
	    return 0;
	}
	switch (message.code) {		       /* Messages from Histo-Scope:  */
	    case HISTOSCOPE_CONNECTING:
#ifdef COM_DEBUG
		printf("Rec'd connecting msg from HistoScope %d.\n",
			connect_id);
#endif
		if (message.dataLength != 0){
		    fprintf(stderr,
		     "hs: Error. Garbled message received from HistoScope %d\n",
		      connect_id);
		    closeScope(connect_id);
		    return 0;
		}
		if (sendToHS(connect_id, CLIENT_CONFIRMING, V4_CLIENT, NULL)
			!= COM_OK)
	    	    return 0;
		sendItemList(connect_id);
		ScopeConfirmed[connect_id] = TRUE;
		return HISTOSCOPE_CONNECTING;
	    case REQUEST_UPDATES:
#ifdef COM_DEBUG
		printf("Rec'd request updates msg from HistoScope %d.\n",
			connect_id);
#endif
		if (UpdateList[connect_id] != NULL) {
		    fprintf(stderr,
		     "hs_update: Error untimely update msg from HistoScope %d.",
		      connect_id);
		    closeScope(connect_id);
		    return 0;
		}
		if (message.dataLength != 0)
		 processUpdates(connect_id, message.itemID, message.dataLength);
		return REQUEST_UPDATES;
	    case REQUEST_ERRORS:
#ifdef COM_DEBUG
		printf("Rec'd request errors msg from HistoScope %d.\n",
			connect_id);
#endif
		setHistErrsOn(message.itemID, connect_id);
		return REQUEST_ERRORS;
	    case CONTROL_SET:
#ifdef COM_DEBUG
		printf("Rec'd set control msg from HistoScope %d.\n",
			connect_id);
#endif
		if (message.dataLength != 0)
		    setControl(connect_id, message.itemID, message.dataLength);
		return CONTROL_SET;
	    case TRIGGER_SET:
#ifdef COM_DEBUG
		printf("Rec'd trigger set msg from HistoScope %d.\n",
			connect_id);
		++NumTriggersSet;
#endif
		if (message.dataLength != 0){
		    fprintf(stderr,
		     "hs: Error. Garbled message received from HistoScope %d\n",
		      connect_id);
		    closeScope(connect_id);
		    return 0;
		}
		setTrigger(connect_id, message.itemID);
		return TRIGGER_SET;
	    case HISTOSCOPE_DONE:
#ifdef COM_DEBUG
		printf("Rec'd done msg from HistoScope %d.\n", connect_id);
#endif
		if (message.dataLength != 0){
		    fprintf(stderr,
		     "hs: Error. Garbled message received from HistoScope %d\n",
		      connect_id);
		    closeScope(connect_id);
		    return 0;
		}
		closeScope(connect_id);
		return HISTOSCOPE_DONE;
            case HISTO_TASK_OK:
	        task_status = 0;
            case HISTO_TASK_FAILURE:
                taskNumber = message.itemID;
                if (taskNumber <= 0)
                {
	            fprintf(stderr,
			"hs: Error reading task number from HistoScope %d.\n",
			connect_id);
                    closeScope(connect_id);
	            return 0;
                }
	        buf = 0;
	        if (message.dataLength > 0)
                {
                    buf = (char *)malloc(message.dataLength+1);
                    if (buf == NULL)
                    {
                       fprintf(stderr,
			   "hs: Error. Out of memory.\n");
                       return 0;
                    }
                    if (socket_read(ComFD[connect_id], buf, message.dataLength)
                        != message.dataLength)
                    {
	                fprintf(stderr,
			    "hs: Error reading task result from HistoScope %d.\n",
			    connect_id);
                        closeScope(connect_id);
	                free(buf);
	                return 0;
                    }
                    buf[message.dataLength] = '\0';
                }
                if (task_completion_callback)
                   task_completion_callback(connect_id, taskNumber, task_status, buf,
			                    task_completion_callback_data);
                if (buf) free(buf);
	        return message.code;
	    default:
		fprintf(stderr,
		 "hs: Error. Unrecognized message received from HistoScope.\n");
		closeScope(connect_id);
		return 0;
	}
    }
}

/*
 * setHistErrsOn - Set transmission of errors for a histogram
 */
static void setHistErrsOn(int itemID, int conid)
{
    hsGeneral *hsG;
#ifdef LINKED_WITH_HBOOK
    int j;
#endif /* LINKED_WITH_HBOOK */

    hsG = GetItemByPtrID(itemID);
#ifdef LINKED_WITH_HBOOK
    if (hsG == NULL)
	hsG = GetHBItemByID(itemID);
#endif /* LINKED_WITH_HBOOK */
    if (hsG == NULL) {		/* item has been deleted */
#ifdef COM_DEBUG
	printf("Errors requested from HistoScope %d, but item %d not found.\n",
	    conid, itemID);
#endif
    return;
    }
    switch (hsG->type) {
	case HS_1D_HISTOGRAM:
	    ((hsC1DHist *)hsG)->sendErrs = TRUE;
	    break;
	case HS_2D_HISTOGRAM:
	    ((hsC2DHist *)hsG)->sendErrs = TRUE;
	    break;
	case HS_3D_HISTOGRAM:
	    ((hsC3DHist *)hsG)->sendErrs = TRUE;
	    break;
	case HS_NTUPLE:
	case HS_INDICATOR:
	case HS_CONTROL:
	case HS_TRIGGER:
	case HS_GROUP:
	default:
	    fprintf(stderr,
	       	"hsUpdate Internal error: setHistErrsOn - Bad item type: %d.\n",
	       	hsG->type);
	    return;
    }
#ifdef LINKED_WITH_HBOOK
    if (hsG->hbookID != 0)
    	j = GetHbookData(hsG, 0, 0, TRUE);	/* get data and errors */
#endif /* LINKED_WITH_HBOOK */
    /* send item data to Histo-Scope process  */
    sendToHS(conid, HERE_IS_DATA, hsG->id, hsG);
#ifdef LINKED_WITH_HBOOK
    if (hsG->hbookID != 0)
	FreeHBData(hsG, 0);			  /* free HBOOK data */
#endif /* LINKED_WITH_HBOOK */
}

/*
 * closeScope - Close HistoScope connection and free data structures
 */
static void closeScope(int connect_id)
{
    if (ComFD[connect_id] == NO_CONNECTION)
        return;
#ifdef COM_DEBUG
    printf("Closing socket connection to Histo-Scope %d.\n",connect_id);
#endif
    socket_close(ComFD[connect_id]);
    ComFD[connect_id] = NO_CONNECTION;
    ScopeConfirmed[connect_id] = FALSE;
    --NumOfConnections;
    if (NumOfConnections < 0) {
    	fprintf(stderr, "hs: Internal Error: NumOfConnections < 0.\n");
    	NumOfConnections = 0;
    }
    /* free members of this Histo-Scope's update list */
    freeUpdateList(connect_id);
    freeUnfinMsgList(connect_id);
    return;
}

/*
 * rtnNforTuple - Return value of n for an n-tuple that is appropriate for
 *		  sending, considering SocketBufferSize and updateLevel.
 *		  i.e. send tuples from updateLevel+1 to rtnNforTuple(), incl.
 *
 *		  For convenience, 0 is returned for non-ntuple items.
 */
static int rtnNforTuple(hsGeneral *hsG, int updateLevel)
{
    hsNTuple *nT = (hsNTuple *)hsG;
    
    if (hsG->type != HS_NTUPLE)
    	return 0;
    	
    if ( ((long)nT->n - (long)updateLevel) * (long)nT->nVariables * (long)sizeof(float) 
    	  - (long)sizeof(hsNTuple) > (long)SocketBufferSize - 16)
    	return ( minInt( updateLevel + ( (SocketBufferSize-16-sizeof(hsNTuple)) 
    	 	 / sizeof(float) / nT->nVariables ), nT->n) ); 
    else
    	return (nT->n);
}

/*
 *   Give connected Histo-Scopes one minute to catch up on data and request
 *   one final update
 */
static void flushUpdates(void)
{
#if defined(VMS) || defined(VXWORKS)
    time_t timeStartedComplete, timeNow;
#else
    struct timeval timeStartedComplete, timeNow;
#endif /*VMS || VXWORKS*/

    int i, r_op;
    int waitingForUpd[MAX_NUM_SCOPES];
    
    for (i = 0; i < MAX_NUM_SCOPES; ++i) {
    	if (ComFD[i] == NO_CONNECTION)
    	    waitingForUpd[i] = 0;
    	else {
    	    waitingForUpd[i] = 1;
    	    sendToHS(i, REQ_LAST_UPDATE, 0, NULL);
    	}
    }
#if defined(VMS) || defined(VXWORKS)
    time(&timeStartedComplete);		/* returned in seconds */
    time(&timeNow);
    while(timeNow < timeStartedComplete + 60) {
#else
    gettimeofday(&timeStartedComplete, NULL);
    gettimeofday(&timeNow, NULL);
    while(timeNow.tv_sec < timeStartedComplete.tv_sec + 60) {
#endif /*VMS || VXWORKS*/
    	if (!anyToWait(waitingForUpd))
    	    break;
	procUnfinUpds();		  /* process any unfinished updates   */
	for (i = 0; i < MAX_NUM_SCOPES; ++i)
	    if ((r_op = readHistoCom(i)) == REQUEST_UPDATES 
	    		|| r_op == HISTOSCOPE_DONE)
	    	waitingForUpd[i] = 0;	 /* read & respond to msgs on socket */
#if defined(VMS) || defined(VXWORKS)
	time(&timeNow);
#else
	gettimeofday(&timeNow, NULL);
#endif /*VMS || VXWORKS*/
    }
#ifdef COM_DEBUG
#if defined(VMS) || defined(VXWORKS)
    printf("waited %d seconds to complete\n", timeNow - timeStartedComplete);
#else
    printf("waited %d seconds to complete\n", timeNow.tv_sec -
    	timeStartedComplete.tv_sec);
#endif /*VMS || VXWORKS*/
#endif /*COM_DEBUG*/
}

/*
 *   Wait and service all Histo-Scope requests until all Histo-Scopes disconnect
 */
static void waitHS(void)
{
#ifdef VXWORKS
    struct timespec sleepTime = {0, 500000000};		/* 1/2 second */
#endif /*VXWORKS*/
#if defined(VMS) || defined(VXWORKS)
    time_t timeStartedComplete, timeNow;
#else
    struct timeval timeStartedComplete, timeNow;
#endif /*VMS || VXWORKS*/

    int i, r_op, anyThingGoinOn;
    
#if defined(VMS) || defined(VXWORKS)
    time(&timeStartedComplete);		/* returned in seconds */
    time(&timeNow);
#else
    gettimeofday(&timeStartedComplete, NULL);
    gettimeofday(&timeNow, NULL);
#endif /*VMS || VXWORKS*/

    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] > 0)
    	    sendToHS(i, CLIENT_WAITING, 0, NULL);

    while(NumOfConnections > 0) {
	acceptHCom();			/* accept any HistoScope connections  */
	anyThingGoinOn = procUnfinUpds();  /* process any unfinished updates  */
	for (i = 0; i < MAX_NUM_SCOPES; ++i) {
	    r_op = readHistoCom(i);	/* read & respond to msgs on socket   */
	    anyThingGoinOn += r_op;
	    if (r_op == HISTOSCOPE_CONNECTING)
	    	sendToHS(i, CLIENT_WAITING, 0, NULL);
	}
	if (anyThingGoinOn == 0) {
#ifdef COM_DEBUG
	    printf("nothin' goin' on, taking a rest...");
#endif /*COM_DEBUG*/
#ifdef VXWORKS
	    nanosleep(&sleepTime, NULL); /* nothin' goin' on, take a rest... */
#else
	    sleep(1);			 /* nothin' goin' on, take a rest... */
#endif /*VXWORKS*/
	}
    }
#if defined(VMS) || defined(VXWORKS)
    time(&timeNow);
#else
    gettimeofday(&timeNow, NULL);
#endif /*VMS || VXWORKS*/
#ifdef COM_DEBUG
#if defined(VMS) || defined(VXWORKS)
    printf("waited %d seconds to complete\n", timeNow - timeStartedComplete);
#else
    printf("waited %d seconds to complete\n", timeNow.tv_sec -
    	timeStartedComplete.tv_sec);
#endif /*VMS || VXWORKS*/
#endif /*COM_DEBUG*/
}

/*
** anyToWait - return true/false if waiting for any Histo-Scopes
*/
static int anyToWait(int *wFU)
{
    int i;
    
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (wFU[i] || UnfinMsgList[i] != NULL || UpdateList[i] != NULL)
    	    return 1;
    return 0;
}

/*
** minInt -- return the smaller of two ints
*/
static int minInt(int a, int b)
{
    if (a <= b) return a;
    else return b;
}

/*
 * forgetUnfinMsg - remove a data message for a given item (after reset)
 */
static void forgetUnfinMsg(int conid, int dataId)
{
    unfinMsgListHdr *prev = NULL, *current, *next;

    if (dataId <= 0)
	return;
    current = UnfinMsgList[conid];
    while (current)
    {
	next = current->next;
	if (current->dataId == dataId)
	{
	    /* Remove this message */
	    free(current->msg);
	    free(current);
	    if (prev)
		prev->next = next;
	    else
		UnfinMsgList[conid] = next;
	}
	else
	    prev = current;
	current = next;
    }
}

/*
 * addUnfinMsg - add a (translated) message to the end of the UnfinMsgList of
 *               a Histo-Scope
 *
 *	Note: Routine was written to allow choice of whether to copy the data.
 *		However, since implementing the Unfinished Message List,
 *		all data should be copied, lest the data be modified or,
 *		even worse, freed and relocated somewhere else
 */
static void addUnfinMsg(int conid, char *msg, int nBytes, int doMalloc,
		 int byteOffset, int dataId)
{
    unfinMsgListHdr *temp;

#ifdef COM_DEBUG
    printf("\n*** adding unfinished message: HS %d, #bytes %d, offset %d, (%d) ***\n\n",
    	conid, nBytes, byteOffset, doMalloc);
#endif
    if (UnfinMsgList[conid] == NULL) {
    	UnfinMsgList[conid] = (unfinMsgListHdr*)malloc(sizeof(unfinMsgListHdr));
    	if (UnfinMsgList[conid] == NULL) {
    	    fprintf(stderr, "hs: malloc failed.  Closed HistoScope %d.",
    		conid);
    	    closeScope(conid);
    	    return;
    	}
    	UnfinMsgList[conid]->next = NULL;
    	if (doMalloc) {
    	    UnfinMsgList[conid]->msg = malloc(nBytes);
    	    if (UnfinMsgList[conid]->msg == NULL) {
    		fprintf(stderr, "hs: malloc failed.  Closed HistoScope %d.",
    		    conid);
    		closeScope(conid);
    		return;
    	    }
	    memcpy((void *)UnfinMsgList[conid]->msg, (void *)msg, nBytes);
	}
	else
	    UnfinMsgList[conid]->msg = msg;
    	UnfinMsgList[conid]->nBytes = nBytes;
    	UnfinMsgList[conid]->byteOffset = byteOffset;
        UnfinMsgList[conid]->dataId = dataId;
    	return;
    }
    temp = UnfinMsgList[conid];
    while (1) {
	if (temp->next == NULL) {
    	    temp->next = (unfinMsgListHdr *)malloc(sizeof(unfinMsgListHdr));
    	    if (temp->next == NULL) {
    		fprintf(stderr, "hs: malloc failed.  Closed HistoScope %d.",
    		    conid);
    		closeScope(conid);
    		return;
    	    }
    	    temp->next->next = NULL;
    	    if (doMalloc) {
    		temp->next->msg = malloc(nBytes);
    		if (temp->next->msg == NULL) {
    		    fprintf(stderr, "hs: malloc failed.  Closed HistoScope %d.",
    			conid);
    		    closeScope(conid);
    		    return;
    		}
                memcpy((void *)temp->next->msg, (void *)msg, nBytes);
    	    }
    	    else
    	        temp->next->msg = msg;
    	    temp->next->nBytes = nBytes;
    	    temp->next->byteOffset = byteOffset;
	    temp->next->dataId = dataId;
    	    return;
	}
	temp = temp->next;
    }
}

/*
 * freeUnfinMsgList - free members of a Histo-Scope UnfinMsgList
 */
static void freeUnfinMsgList(int conid)
{
    unfinMsgListHdr *temp, *toFree;

    toFree = UnfinMsgList[conid];
    UnfinMsgList[conid] = NULL;
    while (toFree != NULL) {
	temp = toFree->next;
	free(toFree->msg);
	free(toFree);
	toFree = temp;
    }
}

/*
 * freeUpdateList - free members of a Histo-Scope Update List
 */
static void freeUpdateList(int connect_id)
{
    if (UpdateList[connect_id] != NULL) {
    	if (UpdateList[connect_id]->updStructPtr != NULL)
    	    free(UpdateList[connect_id]->updStructPtr);
    	if (UpdateList[connect_id]->updStatPtr != NULL)
    	    free(UpdateList[connect_id]->updStatPtr);
    	free(UpdateList[connect_id]);
    	UpdateList[connect_id] = NULL;
    }
}

/*
 * AddItemToHsIdList - Add item pointer to hsidlist, expanding the list if
 *			necessary.  Item may be NULL (or 0) for adding
 *			null Hbook item placeholders.  Call this routine
 *			AFTER incrementing NumOfItems.
 */
void AddItemToHsIdList(hsGeneral *hist)
{
    hsGeneral **hOld;

    if( (NumOfItems - (NumOfItems/SIZE_HSIDLIST)*SIZE_HSIDLIST)
                     == 1 && (NumOfItems != 1)) {
    							 /* Expand hsidlist */
    	hOld = HistoPtrList;
    	HistoPtrList = (hsGeneral **) malloc(sizeof(hsGeneral *) *
                             ((NumOfItems/SIZE_HSIDLIST + 1)*SIZE_HSIDLIST));
        memcpy( (void *) HistoPtrList,  hOld, 
        	(size_t) ((NumOfItems-1)*sizeof(hsGeneral *)) );
    	free (hOld);
    }
    HistoPtrList[NumOfItems-1] = hist;
}

void SetHistResetFlag(hsGeneral *item)
{
    /* set reset flag for each possible connected Scope */
    if (item->type == HS_1D_HISTOGRAM)
	((hsC1DHist *)item)->resetFlag = ~ (~0 << MAX_NUM_SCOPES);
    else if (item->type == HS_2D_HISTOGRAM)
	((hsC2DHist *)item)->resetFlag = ~ (~0 << MAX_NUM_SCOPES);
    else if (item->type == HS_3D_HISTOGRAM)
	((hsC3DHist *)item)->resetFlag = ~ (~0 << MAX_NUM_SCOPES);
    else if (item->type == HS_NTUPLE)
	    ;
    else
	fprintf(stderr, 
    	     "hs: Internal Warning, error in call to SetHistResetFlag\n");
}

/* Routines needed by HbookClient.c *****************************************/

#ifdef LINKED_WITH_HBOOK

int IncNumOfItems(void)
{
    ++NumOfItems;
    AddItemToHsIdList(NULL);		/* Add null placeholder to hsid table */
    return (NumOfItems);
}

void SendHBItem(hsGeneral *item)
{
    int i;
    
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    if (sendToHS(i, ITEM_TO_LIST, item->id, item) != COM_OK)
    	    	fprintf(stderr, 
    	    	  "hs_hbook_setup: error sending item to Histo-Scope %d.\n", i);
}

void SendHBEndL(void)
{
    int i;
    
    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    if (sendToHS(i, END_OF_LIST,  0, NULL) != COM_OK)
    	    	fprintf(stderr,
    	    	"hs_hbook_setup: error sending end of list to Histo-Scope %d.\n"
    	    		, i);
}

/*
 * setupHbookReq - Setup HBOOK Update Request
 */
static int setupHbookReq(hsGeneral *hsG, updStruct *updStP)
{
    int need_to_update, j;

    /* Check whether any new data was added to the item since last update */
    if ( (need_to_update = hbneedtoupdate(hsG, updStP->updateLevel))
		      == 0 )		/* external: in getHbookData.c */
	return (PROCESSED);		/* skip, if no need to update  */
	
    /* Get the data to send to Histo-Scope */
    switch (hsG->type) {
	case HS_NTUPLE:
/*
** Check for circular buffer.
*/
	  if (need_to_update == HBOOK_NTUPLE_IS_CIRCULAR){
	    ((hsCNTuple *)hsG)->fromTuple = 0;
	    ((hsCNTuple *)hsG)->toTuple = ((hsNTuple *)hsG)->n;
	    j = GetHbookData(hsG, 0,
	    	((hsCNTuple *)hsG)->toTuple, FALSE);  /* no errors */
	   } else { 
	    ((hsNTuple *)hsG)->n = need_to_update;
	    ((hsCNTuple *)hsG)->fromTuple = updStP->updateLevel;
	    ((hsCNTuple *)hsG)->toTuple = rtnNforTuple(hsG,updStP->updateLevel);
#ifdef COM_DEBUG
	    printf("    setupHbookReq: fromTuple = %d, toTuple = %d.\n",
		 ((hsCNTuple *)hsG)->fromTuple, ((hsCNTuple *)hsG)->toTuple);
#endif
	    j = GetHbookData(hsG, updStP->updateLevel,
	    	((hsCNTuple *)hsG)->toTuple, FALSE);  /* no errors */
	    }
	    break;
	case HS_1D_HISTOGRAM:
    	    j = GetHbookData(hsG, 0, 0, ((hsC1DHist *)hsG)->sendErrs);
	    break;
	case HS_2D_HISTOGRAM:
    	    j = GetHbookData(hsG, 0, 0, ((hsC2DHist *)hsG)->sendErrs);
	    break;
	case HS_3D_HISTOGRAM:
    	    j = GetHbookData(hsG, 0, 0, ((hsC3DHist *)hsG)->sendErrs);
	    break;
	default:
    	    fprintf(stderr,
    	       "**Error in setupHbookReq  Item %d: type %d not 1d, 2d, or NT\n",
    	       hsG->id, hsG->type);
    	    return (PROCESSED);		/* skip, error		          */
    }
    if ( j != HBOOK_DATA_OK ) {
	fprintf(stderr,
	    "hs_update: HBOOK data for item %d is %s. (%d)\n", hsG->id,
	    j == HBOOK_DATA_MISSING ? "missing" :
	    j == HBOOK_DATA_TOO_BIG || HBOOK_ERRORS_TOO_BIG ? "too big" :
	    j == HBOOK_DATA_CORRUPTED ? "corrupted" : "giving errors",
	    j);
	FreeHBData(hsG, 0);		/* free HBOOK data */
	return (PROCESSED);		/* skip, error		          */
    }
    return (NOT_PROCESSED);		/* Indicate data should be sent   */
}

/*
 * IsHBInit - Is HBOOK / Histoscope Initialized?
 */
int IsHBInit(void)
{
    if (!InitializedAndActive)
     	return HS_NOT_INIT;		/* hs_initialize not yet called   */
    if (!HBInitialized)
     	return HB_NOT_INIT;		/* hs_hbook_setup not yet called  */
    return HB_INIT;			/* hs_hbook_setup has been called */
}

/*
 * InitHB - Set HBOOK Initialized
 */
void InitHB(void)
{
    HBInitialized = TRUE;
}

/*
 * DelHBListItem - Delete HBOOK item from Histo-Scope's item list
 */
void DelHBListItem(int id, hsGeneral *item)
{
    int i;

    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    sendToHS(i, ITEM_DELETED, id, item);
}

#endif	/* LINKED_WITH_HBOOK */

/*
 * BeginItemList - Begin item list with Histo-Scope
 */
void BeginItemList(void)
{
    int i;

    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    sendToHS(i, BEGIN_LIST, 0, NULL);
}

/*
 * EndList - End item list with Histo-Scope
 */
void EndItemList(void)
{
    int i;

    for (i = 0; i < MAX_NUM_SCOPES; ++i)
    	if (ComFD[i] != NO_CONNECTION)
    	    sendToHS(i, END_OF_LIST, 0, NULL);
}


/*
**   GetItemByPtrID - retrieve pointer to Histo item from specified a List
**		 by supplying its id and the pointer to List Pointer Array.
*/
hsGeneral *GetItemByPtrID(int id)
{
    if ( (id < 1) || (id > NumOfItems) ) 
    	return NULL;
    return HistoPtrList[id-1];
}

/*
**   DeleteItemFromPtrList - Delete a histogram, nTuple, indicator or 
**			control from Pointer List
**
** 	example call: DeleteItemFromPtrList(id, HistoPtrList);
**
*/
void DeleteItemFromPtrList(int id)
{
    if (HistoPtrList == NULL) {
    	fprintf(stderr, "DeleteItemFromPtrList - Error: list is empty.\n");
    	return;
    }
    HistoPtrList[id-1] = NULL; 
}
#ifdef LINKED_WITH_HBOOK
#ifdef VMS

static void hcdir_(char *dn, char *dm, int l1, int l2)
{
    struct dsc$descriptor *ds1, *ds2;

    hcdir((ds1 = NulStrToDesc(dn)), (ds2 = NulStrToDesc(dm)));
    FreeStrDesc(ds1);
    FreeStrDesc(ds2);
}

static void hcdirw_(char *dn, char *dm, int l1, int l2)
{
    struct dsc$descriptor *ds1, *ds2;

    ds2 = NulStrToDesc(dm);
    ds1 = NulStrWrtDesc(dn, l1);
    hcdir(ds1, ds2);
    FreeStrDesc(ds1);
    FreeStrDesc(ds2);
}
#endif /* VMS */
#endif /* LINKED_WITH_HBOOK */

int histo_copy_hist(int old_id, int uid, const char *title, const char *category)
{
    hsGeneral *item, *newitem;
    const char *catString;
    
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_copy_hist: Error, called before hs_initialize.\n");
    	return 0;		     /* hs_initialize must be called first */
    }
	
    item = (hsGeneral *) GetItemByPtrID(old_id);
    if (item == NULL) {
      fprintf(stderr, "hs_copy_hist: Error - id: %d does not exist.\n", old_id);
      return 0;
    }

    if (!CheckValidCategory(category, "hs_copy_hist", FALSE))
      return 0;

    /* Check that item characterized by this uid/Category has not already been 
    			created. If so, do not create a new one */		
    if (histo_id(uid, category) != -1) {
      fprintf(stderr,
	      "hs_copy_hist: An item with this uid/Category already exists.\n");
      fprintf(stderr, "  uid = %d, Category = %s, ", uid, category);
      fprintf(stderr, " Item not created.\n");
      return 0;
    }

    if ((newitem = CopyItem(item)) == NULL)
	return 0;

    catString = ValidStr(category, HS_MAX_CATEGORY_LENGTH, 
			 "hs_copy_hist", "category");
    if (strlen(catString) > strlen(newitem->category)) {
	free(newitem->category);
	newitem->category = malloc(sizeof(char) * (strlen(catString) + 1));
    }
    strcpy(newitem->category, catString);

    catString = ValidStr(title, HS_MAX_TITLE_LENGTH,
			 "hs_copy_hist", "title");
    if (strlen(catString) > strlen(newitem->title)) {
	free(newitem->title);
	newitem->title = malloc(sizeof(char) * (strlen(catString) + 1));
    }
    strcpy(newitem->title, catString);

    newitem->uid = uid;

    newitem->id           = ++NumOfItems;
    AddItemToHsIdList(newitem);
    SendNewItem(newitem);
    if (newitem->type == HS_1D_HISTOGRAM || 
	newitem->type == HS_2D_HISTOGRAM ||
	newitem->type == HS_3D_HISTOGRAM)
	SetHistResetFlag(newitem);

    return newitem->id;
}

static void constreset1DHist(hs1DHist *item, float c)
{
    int i;

    for (i = 0; i < (item)->nBins; ++i)
	item->bins[i] = c;
    if (item->pErrs)
	for (i = 0; i < (item)->nBins; ++i)
	    item->pErrs[i] = 0.f;
    if (item->mErrs)
	for (i = 0; i < (item)->nBins; ++i)
	    item->mErrs[i] = 0.f;
    item->overflow  = 0.f;
    item->underflow = 0.f;
    item->count     = 0 ;
    /* set reset flag for each possible connected Scope */
    SetHistResetFlag((hsGeneral *)item);
}

static void constreset2DHist(hs2DHist *item, float c)
{
    int i, j, nbins;

    nbins = (item)->nXBins * (item)->nYBins;

    for (i = 0; i < nbins; ++i)
	item->bins[i] = c;
    if (item->pErrs)
	for (i = 0; i < nbins; ++i)
	    item->pErrs[i] = 0.f;
    if (item->mErrs)
	for (i = 0; i < nbins; ++i)
	    item->mErrs[i] = 0.f;
    for (i = 0; i < 3; ++i)
    	for (j = 0; j < 3; ++j)
    	    item->overflow[i][j] = 0.f;
    item->count = 0;
    /* set reset flag for each possible connected Scope */
    SetHistResetFlag((hsGeneral *)item);
}

static void constreset3DHist(hs3DHist *item, float c)
{
    int i, j, k, nbins;

    nbins = (item)->nXBins * (item)->nYBins * (item)->nZBins;

    for (i = 0; i < nbins; ++i)
	item->bins[i] = c;
    if (item->pErrs)
	for (i = 0; i < nbins; ++i)
	    item->pErrs[i] = 0.f;
    if (item->mErrs)
	for (i = 0; i < nbins; ++i)
	    item->mErrs[i] = 0.f;
    for (i = 0; i < 3; ++i)
    	for (j = 0; j < 3; ++j)
	    for (k = 0; k < 3; ++k)
		item->overflow[i][j][k] = 0.f;
    item->count = 0;
    /* set reset flag for each possible connected Scope */
    SetHistResetFlag((hsGeneral *)item);
}

void histo_reset_const(int id, float c)
{
    hsGeneral *item;

    if (!InitializedAndActive)
    	return;			/* hs_initialize must be called first */
    
    item = (hsGeneral *) GetItemByPtrID(id); /* get item*/

    if (item == NULL) {
      fprintf(stderr, "hs_reset_const: Error - id: %d does not exist.\n", id);
      return;
    }
    switch (item->type) 
    {
	case HS_1D_HISTOGRAM:
	  constreset1DHist( (hs1DHist *) item, c);
	  break;
	case HS_2D_HISTOGRAM:
	  constreset2DHist( (hs2DHist *) item, c);
	  break;
	case HS_3D_HISTOGRAM:
	  constreset3DHist( (hs3DHist *) item, c);
	  break;
	default:
	  fprintf(stderr, "hs_reset_const: item is not a histogram.\n");
    }    
}

#define bin_out_of_range_error(name) do {\
    fprintf(stderr, "%s: Error - "\
	    "bin number out of range.\n", name);\
    return -4;\
} while(0);

#define check_axis_types(name) do {\
    if (axis1 < 0 || axis1 >= N_HS_AXIS_TYPES) {\
	fprintf(stderr, "%s: Error - "\
		"invalid first slice axis.\n", name);\
	return -1;\
    }\
    if (axis2 < 0 || axis2 >= N_HS_AXIS_TYPES) {\
	fprintf(stderr, "%s: Error - "\
		"invalid second slice axis.\n", name);\
	return -1;\
    }\
    if (axis1 == HS_AXIS_NONE && axis2 == HS_AXIS_NONE) {\
	fprintf(stderr, "%s: Error - "\
		"no valid slice axis specified.\n", name);\
	return -1;\
    }\
    if (axis1 == axis2) {\
	fprintf(stderr, "%s: Error - "\
		"same slice axis specified two times.\n", name);\
	return -1;\
    }\
    if (axis1 == HS_AXIS_NONE) {\
	axis_slice = axis2;\
	bin_slice = bin2;\
    } else if (axis2 == HS_AXIS_NONE) {\
	axis_slice = axis1;\
	bin_slice = bin1;\
    }\
} while(0);

#define prepare_stack3DContents_args(name) do {\
    if (bin1 < 0 || bin2 < 0)\
	bin_out_of_range_error( name );\
    switch (axis1)\
    {\
    case HS_AXIS_X:\
	x_bin = bin1;\
	break;\
    case HS_AXIS_Y:\
	y_bin = bin1;\
	break;\
    case HS_AXIS_Z:\
	z_bin = bin1;\
	break;\
    default:\
	fprintf(stderr, "%s: Error - bad slice axis "\
		"is specified for a 3d histogram.\n", name);\
	return -1;\
    }\
    switch (axis2)\
    {\
    case HS_AXIS_X:\
	x_bin = bin2;\
	break;\
    case HS_AXIS_Y:\
	y_bin = bin2;\
	break;\
    case HS_AXIS_Z:\
	z_bin = bin2;\
	break;\
    default:\
	fprintf(stderr, "%s: Error - bad slice axis "\
		"is specified for a 3d histogram.\n", name);\
	return -1;\
    }\
} while(0);

#define match_slice_errors_to_parent do {\
    if (slicesize != slice_bins)\
    {\
	fprintf(stderr, "hs_fill_hist_slice: Error - "\
		"slice with id %d is not bin-compatible "\
		"with parent histogram %d.\n", slice_id, parent_id);\
	return -2;\
    }\
    if (!input_bins_ok)\
        bin_out_of_range_error("hs_fill_hist_slice");\
    if (!parent->errFlg || parent->pErrs == NULL) {\
	if (slice->errFlg)\
	{\
	    if (slice->pErrs)\
		memset(slice->pErrs, 0, slice_bins*sizeof(float));\
	    if (slice->mErrs)\
		memset(slice->mErrs, 0, slice_bins*sizeof(float));\
	}\
    } else {\
	slice->errFlg = 1;\
	if (slice->pErrs == NULL)\
	    slice->pErrs = (float *)malloc(slice_bins*sizeof(float));\
	if (parent->mErrs)\
	    if (slice->mErrs == NULL)\
		slice->mErrs = (float *)malloc(slice_bins*sizeof(float));\
    }\
} while(0);

#define check_slice_status do {\
    assert(status >= 0);\
    assert(nfilled == slice_bins);\
    slice->count = slice_bins;\
} while(0);

int histo_fill_hist_slice(int parent_id, int axis1, int bin1,
			  int axis2, int bin2, int slice_id)
{
    hsGeneral *parent_item, *slice_item;
    int axis_slice = -1, bin_slice, slicesize, slice_bins;
    int status, nfilled, input_bins_ok;

    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_fill_hist_slice: Error called before hs_initialize.\n");
    	return -1;		   /* hs_initialize must be called first */
    }
    parent_item = (hsGeneral *)GetItemByPtrID(parent_id);
    if (parent_item == NULL) {
	fprintf(stderr, "hs_fill_hist_slice: Error - id: "
		"%d does not exist.\n", parent_id);
	return -1;
    }
    slice_item = (hsGeneral *)GetItemByPtrID(slice_id);
    if (slice_item == NULL) {
	fprintf(stderr, "hs_fill_hist_slice: Error - id: "
		"%d does not exist.\n", slice_id);
	return -1;
    }

    /* Check axis types */
    check_axis_types("hs_fill_hist_slice");

    /* Branch depending on item types */
    if (parent_item->type == HS_2D_HISTOGRAM && 
	slice_item->type == HS_1D_HISTOGRAM)
    {
	hs2DHist *parent = (hs2DHist *)parent_item;
	hs1DHist *slice  = (hs1DHist *)slice_item;
	slice_bins = slice->nBins;
	if (axis_slice < 0)
	{
	    fprintf(stderr, "hs_fill_hist_slice: Error - "
		    "more than one slice axis specified "
		    "for a 2d histogram slice.\n");
	    return -1;
	}
	if (axis_slice == HS_AXIS_X) {
	    slicesize = parent->nYBins;
	    input_bins_ok = (bin_slice >= 0 && bin_slice < parent->nXBins);
	} else if (axis_slice == HS_AXIS_Y) {
	    slicesize = parent->nXBins;
	    input_bins_ok = (bin_slice >= 0 && bin_slice < parent->nYBins);
	} else {
	    fprintf(stderr, "hs_fill_hist_slice: Error - "
		    "bad slice axis is specified for a 2d histogram.\n");
	    return -1;
	}
	match_slice_errors_to_parent;
	status = slice2DContents(
	    "hs_fill_hist_slice", parent, axis_slice,
	    bin_slice, slice_bins, slice->bins, slice->pErrs,
	    slice->mErrs, &nfilled);
	check_slice_status;
	slice->overflow = 0.f;
	slice->underflow = 0.f;
    }
    else if (parent_item->type == HS_3D_HISTOGRAM &&
	     slice_item->type == HS_1D_HISTOGRAM)
    {
	int x_bin = -1, y_bin = -1, z_bin = -1;
	hs3DHist *parent = (hs3DHist *)parent_item;
	hs1DHist *slice  = (hs1DHist *)slice_item;
	slice_bins = slice->nBins;
	if (axis_slice >= 0)
	{
	    fprintf(stderr, "hs_fill_hist_slice: Error - "
		    "only one slice axis specified for "
		    "a 1d slice of a 3d histogram.\n");
	    return -1;
	}
	prepare_stack3DContents_args("hs_fill_hist_slice");
	if (x_bin < 0 && y_bin >= 0 && z_bin >= 0) {
	    slicesize = parent->nXBins;
	    input_bins_ok = (y_bin < parent->nYBins && z_bin < parent->nZBins);
	} else if (x_bin >= 0 && y_bin < 0 && z_bin >= 0) {
	    slicesize = parent->nYBins;
	    input_bins_ok = (x_bin < parent->nXBins && z_bin < parent->nZBins);
	} else if (x_bin >= 0 && y_bin >= 0 && z_bin < 0) {
	    slicesize = parent->nZBins;
	    input_bins_ok = (x_bin < parent->nXBins && y_bin < parent->nYBins);
	} else {
	    assert(0);
	}
	match_slice_errors_to_parent;
	status = stack3DContents(
	    "hs_fill_hist_slice", parent, x_bin, y_bin,
	    z_bin, slice_bins, slice->bins, slice->pErrs,
	    slice->mErrs, &nfilled);
	check_slice_status;
	slice->overflow = 0.f;
	slice->underflow = 0.f;
    }
    else if (parent_item->type == HS_3D_HISTOGRAM &&
	     slice_item->type == HS_2D_HISTOGRAM)
    {
	hs3DHist *parent = (hs3DHist *)parent_item;
	hs2DHist *slice  = (hs2DHist *)slice_item;
	slice_bins = slice->nXBins * slice->nYBins;
	if (axis_slice < 0)
	{
	    fprintf(stderr, "hs_fill_hist_slice: Error - "
		    "more than one slice axis specified for "
		    "a 2d slice of a 3d histogram.\n");
	    return -1;
	}
	if (axis_slice == HS_AXIS_X) {
	    slicesize = parent->nYBins*parent->nZBins;
	    input_bins_ok = (bin_slice >= 0 && bin_slice < parent->nXBins);
	} else if (axis_slice == HS_AXIS_Y) {
	    slicesize = parent->nXBins*parent->nZBins;
	    input_bins_ok = (bin_slice >= 0 && bin_slice < parent->nYBins);
	} else if (axis_slice == HS_AXIS_Z) {
	    slicesize = parent->nXBins*parent->nYBins;
	    input_bins_ok = (bin_slice >= 0 && bin_slice < parent->nZBins);
	} else {
	    fprintf(stderr, "hs_fill_hist_slice: Error - "
		    "bad slice axis is specified for a 3d histogram.\n");
	    return -1;
	}
	match_slice_errors_to_parent;
	status = slice3DContents(
	    "hs_fill_hist_slice", parent, axis_slice,
	    bin_slice, slice_bins, slice->bins,
	    slice->pErrs, slice->mErrs, &nfilled);
	check_slice_status;
	{
	    int i, j;
	    for (i=0; i<3; i++) {
		for (j=0; j<3; j++) {
		    slice->overflow[i][j] = 0.f;
		}
	    }
	}
    }
    else
    {
	fprintf(stderr, "hs_fill_hist_slice: Error - "
		"item with id %d can not be a slice "
		"of item with id %d.\n", slice_id, parent_id);
	return -3;
    }

    /* Success */
    SetHistResetFlag(slice_item);
    return 0;
}

int histo_slice_contents(int id, int axis1, int bin1, int axis2, int bin2,
			 int arrsize, float *data, float *poserr,
			 float *negerr, int *slicesize)
{
    hsGeneral *item;
    int axis_slice = -1, bin_slice;

    *slicesize = 0;
    if (!InitializedAndActive) {
    	fprintf(stderr,
    		"hs_slice_contents: Error called before hs_initialize.\n");
    	return -1;		   /* hs_initialize must be called first */
    }

    /* Check that parent is a 2d or 3d histogram */
    item = (hsGeneral *) GetItemByPtrID(id); /* get item*/
    if (item == NULL) {
	fprintf(stderr, "hs_slice_contents: Error - id: %d does not exist.\n", id);
	return -1;
    }
    if (item->type != HS_2D_HISTOGRAM && 
	item->type != HS_3D_HISTOGRAM)
    {
	fprintf(stderr, "hs_slice_contents: Error - "
		"item with id %d can not be sliced.\n", id);
	return -1;
    }

    /* Check axis types */
    check_axis_types("hs_slice_contents");

    /* Call different functions depending on 
       the parent type and slice axes specified */
    if (item->type == HS_2D_HISTOGRAM)
    {
	/* Check that only one slice axis is specified for a 2d histogram */
	if (axis_slice < 0)
	{
	    fprintf(stderr, "hs_slice_contents: Error - "
		    "more than one slice axis specified "
		    "for a 2d histogram slice.\n");
	    return -1;
	}
	return slice2DContents("hs_slice_contents", (hs2DHist *)item,
			       axis_slice, bin_slice, arrsize, data,
			       poserr, negerr, slicesize);
    }
    else if (item->type == HS_3D_HISTOGRAM)
    {
	if (axis_slice < 0)
	{
	    int x_bin = -1, y_bin = -1, z_bin = -1;
	    prepare_stack3DContents_args("hs_slice_contents");
	    return stack3DContents("hs_slice_contents", (hs3DHist *)item,
				   x_bin, y_bin, z_bin, arrsize, data,
				   poserr, negerr, slicesize);
	}
	else
	    return slice3DContents("hs_slice_contents", (hs3DHist *)item,
				   axis_slice, bin_slice, arrsize, data,
				   poserr, negerr, slicesize);
    }
    else
	assert(0);
}

#define float_copy(cp_to, cp_from, cp_stride, cp_n) do {\
    register float *from = (cp_from);\
    register float *to   = (cp_to);\
    register int icycle  = (cp_n);\
    const    int stride  = (cp_stride);\
    for (; icycle > 0; --icycle) {\
	*to++ = *from;\
	from += stride;\
    }\
} while (0);

#define array_size_error(name) do {\
    fprintf(stderr, "%s: Error - "\
	    "array is too small to hold the data.\n", name);\
    return -1;\
} while(0);

static int slice2DContents(const char *name, hs2DHist *item, int axis_slice,
			   int bin_slice, int arrsize, float *data,
			   float *poserr, float *negerr, int *slicesize)
{
    *slicesize = 0;
    if (axis_slice == HS_AXIS_X)
    {
	if (bin_slice < 0 || bin_slice >= item->nXBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nYBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    memcpy(data, item->bins+item->nYBins*bin_slice,
		   *slicesize*sizeof(float));
	if (poserr && item->pErrs && item->errFlg)
	    memcpy(poserr, item->pErrs+item->nYBins*bin_slice,
		   *slicesize*sizeof(float));
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    memcpy(negerr, item->mErrs+item->nYBins*bin_slice,
		   *slicesize*sizeof(float));
    }
    else if (axis_slice == HS_AXIS_Y)
    {
	if (bin_slice < 0 || bin_slice >= item->nYBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nXBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    float_copy(data, item->bins + bin_slice, item->nYBins, *slicesize);
	if (poserr && item->pErrs && item->errFlg)
	    float_copy(poserr, item->pErrs + bin_slice, item->nYBins, *slicesize);
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    float_copy(negerr, item->mErrs + bin_slice, item->nYBins, *slicesize);
    }
    else
    {
	fprintf(stderr, "hs_slice_contents: Error - "
		"bad slice axis is specified for a 2d histogram.\n");
	return -1;
    }
    if (item->errFlg == 0 || item->pErrs == 0)
	return HS_NO_ERRORS;
    if (item->mErrs == 0)
	return HS_POS_ERRORS;
    return HS_BOTH_ERRORS;
}

static int slice3DContents(const char *name, hs3DHist *item, int axis_slice,
			   int bin_slice, int arrsize, float *data,
			   float *poserr, float *negerr, int *slicesize)
{
    int i;

    *slicesize = 0;
    if (axis_slice == HS_AXIS_X)
    {
	if (bin_slice < 0 || bin_slice >= item->nXBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nYBins*item->nZBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    memcpy(data, item->bins + *slicesize*bin_slice,
		   *slicesize*sizeof(float));
	if (poserr && item->pErrs && item->errFlg)
	    memcpy(poserr, item->pErrs + *slicesize*bin_slice,
		   *slicesize*sizeof(float));
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    memcpy(negerr, item->mErrs + *slicesize*bin_slice,
		   *slicesize*sizeof(float));
    }
    else if (axis_slice == HS_AXIS_Y)
    {
	if (bin_slice < 0 || bin_slice >= item->nYBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nXBins*item->nZBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    for (i=0; i<item->nXBins; ++i)
		memcpy(data + i*item->nZBins,
		       item->bins + (i*item->nYBins+bin_slice)*item->nZBins,
		       item->nZBins*sizeof(float));
	if (poserr && item->pErrs && item->errFlg)
	    for (i=0; i<item->nXBins; ++i)
		memcpy(poserr + i*item->nZBins,
		       item->pErrs + (i*item->nYBins+bin_slice)*item->nZBins,
		       item->nZBins*sizeof(float));
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    for (i=0; i<item->nXBins; ++i)
		memcpy(negerr + i*item->nZBins,
		       item->mErrs + (i*item->nYBins+bin_slice)*item->nZBins,
		       item->nZBins*sizeof(float));    
    }
    else if (axis_slice == HS_AXIS_Z)
    {
	if (bin_slice < 0 || bin_slice >= item->nZBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nXBins*item->nYBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    for (i=0; i<item->nXBins; ++i)
		float_copy(data+i*item->nYBins,
			   item->bins+i*item->nYBins*item->nZBins+bin_slice,
			   item->nZBins, item->nYBins);
	if (poserr && item->pErrs && item->errFlg)
	    for (i=0; i<item->nXBins; ++i)
		float_copy(poserr+i*item->nYBins,
			   item->pErrs+i*item->nYBins*item->nZBins+bin_slice,
			   item->nZBins, item->nYBins);
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    for (i=0; i<item->nXBins; ++i)
		float_copy(negerr+i*item->nYBins,
			   item->mErrs+i*item->nYBins*item->nZBins+bin_slice,
			   item->nZBins, item->nYBins);
    }
    else
    {
	fprintf(stderr, "hs_slice_contents: Error - "
		"bad slice axis is specified for a 3d histogram.\n");
	return -1;
    }
    if (item->errFlg == 0 || item->pErrs == 0)
	return HS_NO_ERRORS;
    if (item->mErrs == 0)
	return HS_POS_ERRORS;
    return HS_BOTH_ERRORS;
}

static int stack3DContents(const char *name, hs3DHist *item, int x_bin,
			   int y_bin, int z_bin, int arrsize, float *data,
			   float *poserr, float *negerr, int *slicesize)
{
    *slicesize = 0;

    /* Positive bin numbers specify the stack position within
       the 3d histogram. Check that there is one and only one
       negative bin number. */
    {
	int nneg = 0;
	if (x_bin < 0) ++nneg;
	if (y_bin < 0) ++nneg;
	if (z_bin < 0) ++nneg;
	assert(nneg == 1);
    }

    if (x_bin < 0)
    {
	if (y_bin >= item->nYBins || z_bin >= item->nZBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nXBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    float_copy(data, item->bins + y_bin*item->nZBins + z_bin,
		       item->nYBins*item->nZBins, item->nXBins);
	if (poserr && item->pErrs && item->errFlg)
	    float_copy(poserr, item->pErrs + y_bin*item->nZBins + z_bin,
		       item->nYBins*item->nZBins, item->nXBins);
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    float_copy(negerr, item->mErrs + y_bin*item->nZBins + z_bin,
		       item->nYBins*item->nZBins, item->nXBins);
    }
    else if (y_bin < 0)
    {
	if (x_bin >= item->nXBins || z_bin >= item->nZBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nYBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    float_copy(data, item->bins + x_bin*item->nYBins*item->nZBins + z_bin,
		       item->nZBins, item->nYBins);
	if (poserr && item->pErrs && item->errFlg)
	    float_copy(poserr, item->pErrs + x_bin*item->nYBins*item->nZBins + z_bin,
		       item->nZBins, item->nYBins);
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    float_copy(negerr, item->mErrs + x_bin*item->nYBins*item->nZBins + z_bin,
		       item->nZBins, item->nYBins);
    }
    else if (z_bin < 0)
    {
	if (x_bin >= item->nXBins || y_bin >= item->nYBins)
	    bin_out_of_range_error(name);
	*slicesize = item->nZBins;
	if (arrsize < *slicesize)
	    array_size_error(name);
	if (data)
	    memcpy(data,
		   item->bins + (x_bin*item->nYBins+y_bin)*item->nZBins,
		   *slicesize*sizeof(float));
	if (poserr && item->pErrs && item->errFlg)
	    memcpy(poserr,
		   item->pErrs + (x_bin*item->nYBins+y_bin)*item->nZBins,
		   *slicesize*sizeof(float));
	if (negerr && item->pErrs && item->errFlg && item->mErrs)
	    memcpy(negerr,
		   item->mErrs + (x_bin*item->nYBins+y_bin)*item->nZBins,
		   *slicesize*sizeof(float));
    }
    else
	assert(0);

    if (item->errFlg == 0 || item->pErrs == 0)
	return HS_NO_ERRORS;
    if (item->mErrs == 0)
	return HS_POS_ERRORS;
    return HS_BOTH_ERRORS;
}
