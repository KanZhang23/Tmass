/*******************************************************************************
*									       *
* communications -- Histoscope communications routine		       	       *
*									       *
* Copyright (c) 1991, 1993 Universities Research Association, Inc.	       *
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
* May 11, 1992								       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modified by Joy Kyriakopulos 3/15/93 for VMS Port			       *
*									       *
* Modified by Paul Lebrun December 93 for V3 files       		       *
*									       *
********************************************************************************
* REQUIRED INCLUDE FILES
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>
#include <fcntl.h>
#define socket_perror perror
#define socket_read read
#define socket_write write
#define socket_errno errno
#define socket_close close
#endif /*VMS*/
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <math.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include "histoMainProgram.h"
#include "commPanel.h"
#define XDR_FILTERS_HERE 1
#include "../histo_util/hsTypes.h"
#include "../histo_util/xdrHisto.h"
#include "../histo_util/publish.h"
#include "../histo_util/histoUtil.h"
#include "../histo_util/histprotocol.h"
#include "../histo_util/hsFile.h"
#include "histoP.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "controls.h"
#include "configFile.h"
#include "communications.h"
#include "../util/DialogF.h"
/****   #define COM_DEBUG   ****/
#ifdef VMS
#define SOCK_MILLITM 250             /* # milliseconds for polling socket */
#endif /*VMS*/

/*	Initial update frequency is three seconds */

#define INIT_UPDATE_FREQ 3000

typedef struct _updateListHeader {
    struct _updateListHeader *next;
    updStruct *item;
} updateListHeader;

typedef struct _deferStruct {	     /* Update structure:     */
    int itemID;			     /*  histo item id	      */
    deferProc deferredCB;	     /*  routine to call      */
    void *dataPtr;		     /*  data for CB routine  */
} deferStruct;

typedef struct _deferListHeader {
    struct _deferListHeader *next;
    deferStruct *item;
} deferListHeader;

static Widget WaitingMsg = NULL;
#ifdef VMS
static timeb_t TimeBeforeSentUpdate = {0, 0, 0, 0};
static timeb_t TimeSetTimeOut = {0, 0, 0, 0};
#else
static struct timeval TimeBeforeSentUpdate = {0, 0};
static struct timeval TimeSetTimeOut = {0, 0};
#endif /*VMS*/
static updateListHeader *UpdateList = NULL;
static deferListHeader *DeferListHdr = NULL;
static int NumUpdateItems = 0;
static int UpdatesFinished = 1;
static int TimeOutTime = 0;
static XtIntervalId TimeOutID = 0;
static int DoAnotherUpdate = False;
static int UpdateFreqInMillis = INIT_UPDATE_FREQ;
static int ClientVer = 0;
static float ControlValue = 0.;
static int ConfigFileToLoad = 0;
static char ConfigFile[FILENAME_MAX];
static int DeferMPRedisplay = FALSE;
static int ConnectionIsCaptive = FALSE;
#ifdef COM_DEBUG
static int NumTriggersSet = 0;
static int NumTriggersRead = 0;
#endif /*COM_DEBUG*/

static int       sendToClient(int messageCode, int id);
static void      disconnectProcess(void);
static void      wkCancelButtonCB(Widget parent, caddr_t client_data,
		 	caddr_t call_data);
static void      AddUpdItemToList (updStruct *item, updateListHeader **updList);
static updStruct *GetUpdItemByID (int id, updateListHeader *updList); 
static void      DeleteUpdItemFromList (int id, updateListHeader **updList);
static void      FreeEntireUpdList(updateListHeader **updList);
static void      updateUpdItem(updateListHeader *upd);
static int       sendUpdateReqs(int numUpdateItems, msgStruct *message);
static int       sendSetCtrl(msgStruct *message);
static void      setTimer(int timeInMillis);
static void      cancelTimer(void);
static void      sendUpdatesProc(void);
static void      timerUpCB(void);
static char      *sock_erstr(int error_num);
#ifdef VMS
static void      sockTimerUpCB(void);
#endif /*VMS*/
static void      loadConfigFile(hsGeneral *hsG);
static void      addItemIdToDeferList(int itemID, deferProc deferredCB, 
		 	void *dataPtr, deferListHeader **deferListHdr);
deferStruct      *takeDeferItemByID(int id, deferListHeader **deferListHdr);
static Boolean   serviceDataWaitCB(int itemID);

int CaptiveConnect(char *setenvVar)
{
    char myHostName[] = "localhost";
    char userName[MAX_USER_NAME_LEN];
    char idString[HS_MAX_IDENT_LENGTH];
    char windowName[HS_MAX_IDENT_LENGTH];
    char startTime[TIME_FIELD_LEN];
    char pidStr[21];
    int portNum, key;
    
    sscanf(setenvVar, "%d %s %d %s \"%[^\"]\" \"%[^\"]\"",
    		   &key, pidStr, &portNum, userName, idString, startTime);
    strcpy(windowName, idString);
    strcat(windowName, " ");
    strcat(windowName, pidStr);
    strcat(windowName, " ");
    strcat(windowName, startTime);
    
    if (ConnectToClient(myHostName, portNum) != COM_OK)
    	return 0;
    SetMainPanelConnected(windowName);
    ConnectionIsCaptive = TRUE;
    return 1;
}

int ConnectToClient(char * on_host, int portNum)
{
    struct sockaddr_in server;
    struct hostent *hp;			/* for gethostbyname */
    XmString xmstr;

    /* Create socket */
    ComFD = socket(AF_INET, SOCK_STREAM, 0);  /* ARPA Internet addresses;
		   sequenced, reliable, 2-way byte stream; internet protocol */
    if (ComFD < 0) {
	socket_perror("Error opening connection in ConnectToClient");
	disconnectProcess();
	return COM_CTC_ERROR_OPEN;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);
    hp = gethostbyname(on_host);
    if (hp == 0) {
	socket_perror("Error getting host address in ConnectToClient");
	disconnectProcess();
        return COM_CTC_ERROR_HADDR;
    }
    memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
    if (connect(ComFD, (struct sockaddr *)&server, sizeof(server)) != 0) {
	socket_perror ("Error connecting to attached process in ConnectToClient");
	disconnectProcess();
	return COM_CTC_ERROR_CONN;
    }
    if (sendToClient(HISTOSCOPE_CONNECTING, 0) != COM_OK) {
    	disconnectProcess();
    	return COM_CTC_ERROR_SEND;
    }
    if ((WaitingMsg != NULL) && XtIsManaged(WaitingMsg)) {
	SET_ONE_RSRC(WaitingMsg, XmNmessageString, (xmstr=MKSTRING(
		"Waiting for response...")));
	XmStringFree ( xmstr );
    }
    return COM_OK;
}

/*
** ReadClientCom - read message from client on socket
*/
int ReadClientCom(Widget parent)
{
    char msgbuf[sizeof(msgStruct)];
    char *databuf;
    msgStruct message;
    int rval, tot_read, j, numMilliSec, milliTm;
    XDR xdrs;
    hsGeneral *hsG = NULL;
    enum utype u_type;
#ifdef VMS
    int nbio_enabled = 1;
    int nbio_disabled = 0;
    timeb_t timeAfter;
#else
    struct timeval timeAfter;
#endif /*VMS*/
    
    if (ComFD == NO_CONNECTION)	      /* ignore stuff left over after a kill */
        return 0;

#ifdef VMS
    if (socket_ioctl(ComFD, FIONBIO, &nbio_enabled) < 0) {
#else
    if (fcntl(ComFD, F_SETFL, O_NDELAY) < 0) {
#endif /*VMS*/
	disconnectProcess();
	if (WaitingMsg != NULL)
	    XtUnmanageChild(WaitingMsg);
	DialogF(DF_WARN, parent, 1,
	    "Communication error\n(%s in fcntl call: ReadClientCom)",
	    "Acknowledged", sock_erstr(socket_errno));
	return 0;
    }
    memset(msgbuf, 0, sizeof(msgbuf));
    rval = socket_read(ComFD, msgbuf, sizeof(msgbuf));
#ifdef COM_DEBUG
    printf("rval = %d (reading message)\n", rval);
#endif
    if (rval <= 0) {
	if (socket_errno == EWOULDBLOCK)
	    return 0;
	if (WaitingMsg != NULL)
	    XtUnmanageChild(WaitingMsg);
#ifdef COM_DEBUG
	socket_perror("Error reading from socket");
#endif /*COM_DEBUG*/
	disconnectProcess();
	DialogF(DF_WARN, parent, 1,
	    "Lost communication with attached process\n(%s in ReadClientCom)",
	    "Acknowledged", sock_erstr(errno));
	return 0;
    }
/* received message from client process */
#ifdef COM_DEBUG_NOT
    printf("msgbuf = ");
    { int yyy = 0;
      for (yyy = 0; yyy < 16; ++yyy)
          printf(" %x", msgbuf[yyy]);
      printf("\n");
    }
#endif
#ifdef VMS
    if(socket_ioctl(ComFD, FIONBIO, &nbio_disabled) < 0) {   /* clear flag */
	if (WaitingMsg != NULL)
	    XtUnmanageChild(WaitingMsg);
	disconnectProcess();
	DialogF(DF_WARN, parent, 1,
	    "Communication error.\n(in socket_ioctl call: ReadClientCom)",
	    "Acknowledged");
	return 0;
    }
#else /*VMS*/
    if ((j = fcntl(ComFD, F_GETFL, 0)) < 0) {
	if (WaitingMsg != NULL)
	    XtUnmanageChild(WaitingMsg);
	disconnectProcess();
	DialogF(DF_WARN, parent, 1,
	    "Communication Error.\n(%s in fcntl call: ReadClientCom)",
	    "Acknowledged", sock_erstr(errno));
	return 0;
    }
    if ((fcntl(ComFD, F_SETFL, j & ~O_NDELAY)) < 0) {	/* clear flag */
	if (WaitingMsg != NULL)
	    XtUnmanageChild(WaitingMsg);
	disconnectProcess();
	DialogF(DF_WARN, parent, 1,
	    "Communication error.\n(%s in fcntl call: ReadClientCom)",
	    "Acknowledged", sock_erstr(errno));
	return 0;
    }
#endif /*VMS*/
    if (rval != sizeof(msgbuf))  {
	if (WaitingMsg != NULL)
	    XtUnmanageChild(WaitingMsg);
	for (tot_read = rval; tot_read < (int)(sizeof(msgbuf));  ) {
	    rval = socket_read(ComFD, msgbuf+tot_read, (int)(sizeof(msgbuf))-tot_read);
#ifdef COM_DEBUG
	    printf("rval = %d\n", rval);
#endif
	    if (rval <= 0) {
		disconnectProcess();
		DialogF(DF_ERR, parent, 1,
			"Lost communication with attached process\n\
			(%s - reading message part)",
			"Acknowledged", sock_erstr(socket_errno));
		return 0;
	    }
	    tot_read += rval;
	}
    }
    xdrmem_create(&xdrs, msgbuf, sizeof(msgbuf), XDR_DECODE);
    if (!xdr_msgStruct(&xdrs, &message)) { 	/* translate message	      */
	disconnectProcess();
	DialogF(DF_WARN, parent, 1,
	    "Error translating message from attached process\n","Acknowledged");
	xdr_destroy (&xdrs);
	return 0;
    }
    xdr_destroy (&xdrs);
    if (message.confirm != CONFIRM_CODE) {	/* confirm message 	      */
#ifdef COM_DEBUG
	printf("\n***ERROR CONFIRMING MESSAGE***\n");
	printf("***msg code: %d, msg confirm code: %d, msg itemid: %d, msg datsiz: %d\n",
		message.code, message.confirm, message.itemID, message.dataLength);
    printf("***msgbuf = ");
    { int yyy = 0;
      for (yyy = 0; yyy < 16; ++yyy)
          printf(" %x", msgbuf[yyy]);
      printf("end\n");
    }
#endif
	disconnectProcess();
	DialogF(DF_WARN, parent, 1,
	    "Error confirming message from attached process\n", "Acknowledged");
	return 0;
    }
    switch (message.code) {		       /* Messages from Client:       */

	case CLIENT_CONFIRMING:
#ifdef COM_DEBUG
	    printf("received client confirming msg (V %d)\n", message.itemID+1);
#endif
	    if (WaitingMsg != NULL)
		XtUnmanageChild(WaitingMsg);
	    ClientVer = message.itemID;
	    if (ClientVer == V1_CLIENT || ClientVer == V2_CLIENT 
	    	    || ClientVer == V3_CLIENT || ClientVer == V4_CLIENT)
	    	xdr_SetClientVer(ClientVer);
	    else {
	    	int i;
	    	i = DialogF(DF_WARN, parent, 2,
		    "V4.0 Histo-Scope received connection from > V4.0 Client\n",
		    "Continue", "Disconnect");
		if (i == 2) {
		    disconnectProcess();
		    return 0;
		}
	    }
#ifdef VMS
	    ftime(&TimeBeforeSentUpdate);
#else
    	    gettimeofday(&TimeBeforeSentUpdate, NULL);
#endif /*VMS*/
    	    setTimer(UpdateFreqInMillis);
	    UpdatesFinished = 1;
	    ClientIsConnected();
	    return 1;

	case TRIGGER_READ:
#ifdef COM_DEBUG
	    ++NumTriggersRead;
	    printf("received trigger read msg. for id %d.\n", message.itemID);
	    printf("   numRead = %d, numSet = %d\n", NumTriggersRead,
	    	 NumTriggersSet);
	    if (message.dataLength != 0)
	        printf("***Warning: dataLength != 0 in trigger read msg ***\n");
#endif
	    hsG = GetMPItemByID(message.itemID);
	    if (hsG != NULL)
		AcknowledgeTrigger(message.itemID);
	    else {
		DisconnectProcess();
		DialogF(DF_WARN, parent, 1,
		       "Internal Error: can't find item for trigger read msg\n",
		       "Acknowledged");
		return 0;
	    }
	    return 1;

	case ITEM_TO_LIST:
#ifdef COM_DEBUG
	    printf("received item to list msg.  id = %d.  data coming: %d.\n",
	    	    message.itemID, message.dataLength);
#endif

	case CONTROL_READ:
#ifdef COM_DEBUG
	    if (message.code == CONTROL_READ)
	       printf("received control read msg. id = %d.  data coming: %d.\n",
	    	    message.itemID, message.dataLength);
#endif

	case NEW_ITEM:
#ifdef COM_DEBUG
	    if (message.code == NEW_ITEM)
		printf("received new item msg. id = %d, data coming: %d.\n",
	    	    message.itemID, message.dataLength);
#endif

	case HERE_IS_DATA:
#ifdef COM_DEBUG
	    if (message.code == HERE_IS_DATA)
		printf("received here is data msg. id = %d. data coming: %d.\n",
	    	    message.itemID, message.dataLength);
#endif
	    databuf = XtMalloc(message.dataLength);
	    for (tot_read = 0; tot_read < message.dataLength;  ) {
		rval = socket_read(ComFD, databuf + tot_read,
#ifndef VMS
				message.dataLength - tot_read);     /* unix */
#else
				message.dataLength - tot_read > 65535 /* vms */
				       ? 65535 : message.dataLength - tot_read);
/* Multinet bug: size cannot be > short word */
#endif /*VMS*/
#ifdef COM_DEBUG
		printf("   rval = %d,", rval);
		printf(" tot_read = %d, message.dataLength = %d\n", tot_read,
				message.dataLength);
#endif
		if (rval == -1) {
	    	    if (socket_errno == EWOULDBLOCK) {
	    	    	/* this code path should never be executed, but a */
	    	    	/* MULTINET bug did make this necessary...	  */
	    	    	/* this is drastic, but let's see if this works!  */
#ifdef COM_DEBUG
	    	    	printf("**sleeping 1 second**\n");
#endif /*COM_DEBUG*/
	    	    	sleep(1);
	    	    	continue;
	    	    } else {
#ifdef COM_DEBUG
		    	socket_perror("Error reading from socket");
#endif /*COM_DEBUG*/
			disconnectProcess();
	    		if (WaitingMsg != NULL)
			    XtUnmanageChild(WaitingMsg);
			DialogF(DF_ERR, parent, 1,
				"Lost communication with attached process\n\
				(%s - reading data/item (%d))",
				"Acknowledged", sock_erstr(socket_errno), rval);
			return 0;
		    }
		}
		else if (rval < 1 || rval > message.dataLength - tot_read){
			disconnectProcess();
	    		if (WaitingMsg != NULL)
			    XtUnmanageChild(WaitingMsg);
			DialogF(DF_ERR, parent, 1,
				"Communication error with attached process\n\
				(return value = %d) - reading data/item",
				"Acknowledged", rval);
			return 0;
		}
		tot_read += rval;
	    }
	    /* Note that if de-coding a configuration string, hsG below
	       will be NULL.  This is normal, XDR will create the string */
	    if (message.code == HERE_IS_DATA || message.code == CONTROL_READ)
	    	hsG = GetMPItemByIDnp(message.itemID);
	    xdrmem_create(&xdrs, databuf, message.dataLength, XDR_DECODE);
	    if (xdr_Item(&xdrs, &hsG, &u_type)) { 	/* translate item     */
#ifdef COM_DEBUG
/*  printf("   u_type = %d, (%s)\n", u_type, UtypeCodes[u_type]); */
#endif
	    	if (message.code == HERE_IS_DATA && u_type == DATA_CONFIG) {
	    	    loadConfigFile(hsG);
	    	    xdr_destroy (&xdrs);
	    	    XtFree(databuf);
	    	    return 1;
	    	}
	    	if (message.code == HERE_IS_DATA
	    		|| message.code == CONTROL_READ) {
	    	    hsG = GetMPItemByID(message.itemID);
	    	    if (hsG == NULL) {
	    		if (message.code != CONTROL_READ) {
			    DisconnectProcess();
			    DialogF(DF_WARN, parent, 1,
			  "Internal Error: can't find item for data received\n",
			      "Acknowledged");
			    return 0;
			}
			else
		            return 1; /* just ignore control if not yet rec'd */
	    	    }
#ifdef COM_DEBUG
	    	    if (hsG->type == HS_NTUPLE) {
	    	    	hsNTuple *nt = (hsNTuple *)hsG;
	    	    	printf("Ntuple entries = %d\n", nt->n);
	    	    }
#endif
	    	    /* Next stmt services any pending axis settings from
	    	     * configFile.c for the item */
	    	    while (serviceDataWaitCB(message.itemID))
	    	    	{}
	    	    /* This if statement gives better performance for
	    	     * huge ntuples. The refresh will happen when an
	    	     * END_NTUPLE_DATA msg is received.  This way, we
	    	     * won't have to re-draw the plot for each clump
	    	     * of Ntuple data received. Added for V4.0 */ 
	    	    if (hsG->type != HS_NTUPLE || ClientVer < V4_CLIENT)
	    	        RefreshItem(message.itemID);
	    	}
	    	else {
	            AddItemToMainPanel(hsG);	 /* ITEM_TO_LIST or NEW_ITEM */ 
	            if (message.code == NEW_ITEM)
	            	RedisplayHistogramList(False);
	        }
	    }
	    else {
		disconnectProcess();
		DialogF(DF_WARN, parent, 1,
		    "Error translating item from attached process\n",
		    "Acknowledged");
	    }
	    xdr_destroy (&xdrs);
	    XtFree(databuf);
	    return 1;

	case BEGIN_LIST:
#ifdef COM_DEBUG
	    printf("received begin list msg\n");
#endif
	    DeferMPRedisplay = TRUE;
	    return 1;

	case END_OF_LIST:
#ifdef COM_DEBUG
	    printf("received end of list msg\n");
#endif
	    RedisplayHistogramList(True);
	    DeferMPRedisplay = FALSE;
	    if (ConfigFileToLoad) {
	    	ReadConfigFile(MainPanelW, ConfigFile);
	    	ConfigFileToLoad = 0;
	    }
	    return 1;

	case END_NTUPLE_DATA:
	    /* This will be recieved only from clients >= V4_CLIENT */
#ifdef COM_DEBUG
	    printf("received end of ntuple data msg\n");
#endif
	    hsG = GetMPItemByID(message.itemID);
	    if (hsG == NULL) {
		DisconnectProcess();
		DialogF(DF_WARN, parent, 1,
	      	    "Internal Error: can't find item for message received\n",
		    "Acknowledged");
		return 0;
	    }
	    if (hsG->type == HS_NTUPLE)
	    	RefreshItem(message.itemID);
	    return 1;

        case INHIBIT_RESET_REFRESH:
#ifdef COM_DEBUG
	    printf("received inhibit reset refresh msg for id = %d\n", message.itemID);
#endif
	    hsG = GetMPItemByID(message.itemID);
	    if (hsG == NULL) {
		DisconnectProcess();
		DialogF(DF_WARN, parent, 1,
	      	    "Internal Error: can't find item for message received\n",
		    "Acknowledged");
		return 0;
	    }
	    if (hsG->type == HS_NTUPLE)
	    	((hsCNTuple *)hsG)->inhibitResetRefresh = 1;
	    return 1;

        case ALLOW_RESET_REFRESH:
#ifdef COM_DEBUG
	    printf("received allow reset refresh msg for id = %d\n", message.itemID);
#endif
	    hsG = GetMPItemByID(message.itemID);
	    if (hsG == NULL) {
		DisconnectProcess();
		DialogF(DF_WARN, parent, 1,
	      	    "Internal Error: can't find item for message received\n",
		    "Acknowledged");
		return 0;
	    }
	    if (hsG->type == HS_NTUPLE)
	    	((hsCNTuple *)hsG)->inhibitResetRefresh = 0;
	    return 1;

	case ITEM_DELETED:
#ifdef COM_DEBUG
	    printf("received item deleted msg for id = %d\n", message.itemID);
#endif
	    DeleteItemFromMainPanel(message.itemID);
	    if (!DeferMPRedisplay)
	        RedisplayHistogramList(False);
	    if (GetUpdItemByID (message.itemID, UpdateList) != NULL)
	        DeleteUpdItemFromList (message.itemID, &UpdateList);
	    return 1;

	case NTUPLE_RESET:
#ifdef COM_DEBUG
	    printf("received ntuple reset msg for id = %d\n", message.itemID);
#endif
	    hsG = GetMPItemByID(message.itemID);
	    if (hsG != NULL && hsG->type == HS_NTUPLE) {
		ResetNtuple((hsNTuple *)hsG);
		((hsCNTuple *)hsG)->needsCompleteUpdate = 0xffffffff;
		if (!((hsCNTuple *)hsG)->inhibitResetRefresh)
		    RefreshItem(message.itemID);
	    }
	    else {
		DisconnectProcess();
		DialogF(DF_WARN, parent, 1,
			"Internal Error: can't find item for reset n-tuple\n",
			"Acknowledged");
		return 0;
	    }
	    return 1;

	case TRIGGER_RESET:
#ifdef COM_DEBUG
	    printf("received trigger reset msg\n");
#endif
	    hsG = GetMPItemByID(message.itemID);
	    if (hsG != NULL && hsG->type == HS_TRIGGER) {
		ResetTrigger((hsTrigger *)hsG);
	    }
	    else {
		DisconnectProcess();
		DialogF(DF_WARN, parent, 1,
			"Internal Error: can't find item for reset triger\n",
			"Acknowledged");
		return 0;
	    }
	    return 1;

	case CLIENT_DONE:
#ifdef COM_DEBUG
	    printf("received client done msg\n");
#endif
	    disconnectProcess();
	    DialogF(DF_INF, parent, 1, "The attached process is finished.\n",
	     	"Acknowledged");
	    return 0;

	case CLIENT_WAITING:
#ifdef COM_DEBUG
	    printf("received client waiting msg\n");
#endif
	    DialogF(DF_INF, parent, 1, 
"The attached process is finished and\nwaiting for all Histo-Scopes to finish.",
	     	"Acknowledged");
	    SetAllControlsInSensitive();
	    return 1;

	case UPDATES_FINISHED:
	    UpdatesFinished = 1;
#ifdef VMS
	    ftime(&timeAfter);		/* returned in seconds */
#ifdef COM_DEBUG
	    printf("received updates finished msg\n");
	    printf("TimeBefore = (%d, %d); timeAfter = (%d, %d).\n",
	    	TimeBeforeSentUpdate.time, TimeBeforeSentUpdate.millitm,
		timeAfter.time, timeAfter.millitm);
#endif /*COM_DEBUG*/
	    /* compute number of elapsed milliseconds since update was sent */
	    numMilliSec = 1000 * (timeAfter.time - TimeBeforeSentUpdate.time)
		+ timeAfter.millitm - TimeBeforeSentUpdate.millitm;
#else /*VMS*/
	    gettimeofday(&timeAfter, NULL);
#ifdef COM_DEBUG
	    printf("received updates finished msg\n");
	    printf("TimeBefore = (%d, %d); timeAfter = (%d, %d).\n",
	    	TimeBeforeSentUpdate.tv_sec, TimeBeforeSentUpdate.tv_usec,
	    	timeAfter.tv_sec, timeAfter.tv_usec);
#endif /*COM_DEBUG*/
	    /* compute number of elapsed milliseconds since update was sent */
	    numMilliSec = 1000 * (timeAfter.tv_sec-TimeBeforeSentUpdate.tv_sec)
	    	    + (timeAfter.tv_usec - TimeBeforeSentUpdate.tv_usec) / 1000;
#endif /*VMS*/
	    if ((milliTm = UpdateFreqInMillis - numMilliSec) > 0
	    	          && DoAnotherUpdate == False)
    	    	setTimer(milliTm);
    	    else {
    	    	DoAnotherUpdate = False;
    		sendUpdatesProc();
    	    }
#ifdef COM_DEBUG
    	    printf("milliTm = %d, UpdateFreqInMillis = %d, numMilliSec = %d\n",
    	    	milliTm, UpdateFreqInMillis, numMilliSec);
#endif
	    return 1;

	case REQ_LAST_UPDATE:
	    if (UpdatesFinished)
	        sendToClient(REQUEST_UPDATES, NumUpdateItems);
	    return 1;
	    
	case REQ_CAPTIVE_HISTO_EXIT:
	    if (ConnectionIsCaptive) {
		DisconnectProcess();
    		printf("\n\nHisto-Scope:  Program Requested Exit.\n\n");
    		exit(0);
	    }
	    return 0;
	    
	default:
	    disconnectProcess();
	    DialogF(DF_WARN, parent, 1,
	    	"Error: Unrecognized message received from attached process\n",
	     	"Acknowledged");
	    return 0;

    }
}

/*
** SendUpdates - Ask client to send updated data for items in update list
*/

static void sendUpdatesProc(void)
{
    if (UpdatesFinished) {
#ifdef VMS
	ftime(&TimeBeforeSentUpdate);
#else
	gettimeofday(&TimeBeforeSentUpdate, NULL);
#endif /*VMS*/
	if (NumUpdateItems > 0)
	    sendToClient(REQUEST_UPDATES, NumUpdateItems);
	else
            setTimer(UpdateFreqInMillis);
    }
}

/*
** setTimer - Set the timer
*/
static void setTimer(int timeInMillis)
{
    if (TimeOutID == 0) {
#ifdef VMS
	ftime(&TimeSetTimeOut);
#else
	gettimeofday(&TimeSetTimeOut, NULL);
#endif /*VMS*/
	TimeOutTime = timeInMillis;
	TimeOutID = XtAppAddTimeOut(XtWidgetToApplicationContext(MainPanelW),
    	    		    timeInMillis > 0 ? timeInMillis : 1, 
    	    		    (XtTimerCallbackProc)timerUpCB, NULL);
    }
    else {
    	cancelTimer();
    	setTimer(timeInMillis);
    }
}

/*
** cancelTimer - Cancel the timer and reset timer variables.
*/
static void cancelTimer(void)
{
    if (TimeOutID != 0) {
    	XtRemoveTimeOut(TimeOutID);
    	TimeOutTime = 0;
    	TimeOutID = 0;
#ifdef VMS
    	TimeSetTimeOut.time = 0;
    	TimeSetTimeOut.millitm = 0;
#else
    	TimeSetTimeOut.tv_sec = 0;
    	TimeSetTimeOut.tv_usec = 0;
#endif /*VMS*/
    }
}

/*
** timerUpCB - Timer is up.  Reset timer variables and see if there are updates
**	     to request
*/
static void timerUpCB(void)
{
    TimeOutTime = 0;
    TimeOutID = 0;
#ifdef VMS
    TimeSetTimeOut.time = 0;
    TimeSetTimeOut.millitm = 0;
#else
    TimeSetTimeOut.tv_sec = 0;
    TimeSetTimeOut.tv_usec = 0;
#endif /*VMS*/
    if (ComFD > 0)
	sendUpdatesProc();
}


/*
** hs_ClientVersion - Return version number of client
*/

int hs_ClientVersion(void)
{
    return ClientVer;
}

/*
** RequestUpdates - Add an item to the update list
*/

void RequestUpdates(int id)
{
    updStruct *updItem;
    hsGeneral *item;
    
    if (ComFD == NO_CONNECTION)
    	return;
    item = GetMPItemByID(id);
    if (item->type == HS_CONTROL)		/* controls are not updated */	
    	return;
    if (GetUpdItemByID(id, UpdateList) == NULL) {
	updItem = (updStruct *) XtMalloc(sizeof(updStruct));
	updItem->itemID = id;
	updItem->type = item->type;
	switch (updItem->type) {
    	    case HS_1D_HISTOGRAM:
    		    updItem->updateLevel = ((hs1DHist *)item)->count;
    		    updItem->updateLevelF = 0;
    		    break;
    	    case HS_2D_HISTOGRAM:
    		    updItem->updateLevel = ((hs2DHist *)item)->count;
    		    updItem->updateLevelF = 0;
    		    break;
    	    case HS_3D_HISTOGRAM:
    		    updItem->updateLevel = ((hs3DHist *)item)->count;
    		    updItem->updateLevelF = 0;
    		    break;
    	    case HS_NTUPLE:
    		    updItem->updateLevel = ((hsNTuple *)item)->n;
    		    updItem->updateLevelF = 0;
    		    break;
    	    case HS_INDICATOR:
    		    updItem->updateLevel = ((hsIndicator *)item)->valueSet;
    		    updItem->updateLevelF = ((hsIndicator *)item)->value;
    		    break;
    	    default:
    		return;
	}
	AddUpdItemToList(updItem, &UpdateList);
#ifdef COM_DEBUG
	printf("added item %d to UpdateList (%d items in list)\n", id,
    	    NumUpdateItems);
#endif
    }
    if (UpdatesFinished) {
    	cancelTimer();
    	sendUpdatesProc();
    }
    else
    	DoAnotherUpdate = True;
}

/*
** EndUpdates - Remove an item from the update list
*/

void EndUpdates(int id)
{
    if (ComFD == NO_CONNECTION || GetUpdItemByID(id, UpdateList) == NULL)
    	return;
    DeleteUpdItemFromList(id, &UpdateList);
#ifdef COM_DEBUG
    printf("deleted item %d from UpdateList\n", id);
#endif
}

/*
** GetUpdateFreq - Get Update Frequency in Milliseconds
*/

int GetUpdateFreq(void)
{
    return(UpdateFreqInMillis);
}

/*
** SetUpdateFreq - Set Update Frequency (in Milliseconds)
*/

void SetUpdateFreq(int updFreq)
{
#ifdef VMS
    timeb_t timeNow;
#else
    struct timeval timeNow;
#endif /*VMS*/
    int numMilliSec, milliTm;

    UpdateFreqInMillis = updFreq;
    if (ComFD <= 0)
    	return;			     /* skip setting timers, if no connection */
#ifdef VMS
    ftime(&timeNow);
    if (UpdatesFinished) {
#ifdef COM_DEBUG
	printf("Setting Update Frequency to %d\n", updFreq);
	printf("TimeBefore = (%d, %d); timeNow = (%d, %d).\n",
		    TimeBeforeSentUpdate.time, TimeBeforeSentUpdate.millitm,
		    timeNow.time, timeNow.millitm);
#endif /*COM_DEBUG*/
	/* compute number of elapsed milliseconds since update was sent */
	numMilliSec = 1000 * (timeNow.time - TimeBeforeSentUpdate.time)
		+ timeNow.millitm - TimeBeforeSentUpdate.millitm;
#else /*VMS*/
    gettimeofday(&timeNow, NULL);
    if (UpdatesFinished) {
#ifdef COM_DEBUG
	printf("Setting Update Frequency to %d\n", updFreq);
	printf("TimeBefore = (%d, %d); timeNow = (%d, %d).\n",
	    	    TimeBeforeSentUpdate.tv_sec, TimeBeforeSentUpdate.tv_usec,
	    	    timeNow.tv_sec, timeNow.tv_usec);
#endif /*COM_DEBUG*/
	/* compute number of elapsed milliseconds since update was sent */
	numMilliSec = 1000 * (timeNow.tv_sec-TimeBeforeSentUpdate.tv_sec)
		+ (timeNow.tv_usec - TimeBeforeSentUpdate.tv_usec) / 1000;
#endif /*VMS*/
	if ((milliTm = UpdateFreqInMillis - numMilliSec) > 0
	     && DoAnotherUpdate == False)
    	    setTimer(milliTm);
	else {
    	    DoAnotherUpdate = False;
    	    sendUpdatesProc();
	}
#ifdef COM_DEBUG
    	printf("milliTm = %d, UpdateFreqInMillis = %d, numMilliSec = %d\n",
    	    	milliTm, UpdateFreqInMillis, numMilliSec);
#endif
    }
    else
    	DoAnotherUpdate = True;
}

/*
** Request Errors - send msg to client to Request Error 
*/

int RequestErrors(int id)
{
    hsGeneral *item;
    
    if (ComFD == NO_CONNECTION)
    	return (COM_ERR_NOCONNECT);
    if (ClientVer < V3_CLIENT)
    	return (COM_ERR_V1CLIENT);
    item = GetMPItemByID(id);
    if (item == NULL) {
    	fprintf(stderr,
    	    "Internal Error: requesting errors for non-existing item\n");
    	return (COM_ERR_ERR);
    }
    if (item->type != HS_1D_HISTOGRAM &&
	item->type != HS_2D_HISTOGRAM &&
	item->type != HS_3D_HISTOGRAM) {
    	fprintf(stderr,
    	    "Internal Error: requesting errors for non-histograms\n");
    	return (COM_ERR_ERR);
    }
    sendToClient(REQUEST_ERRORS, id);
    return (COM_OK);
}

/*
** SetControl - send msg to client to Set the value of a Control 
*/

int SetControl(int id, float value)
{
    hsGeneral *item;
    
    if (ComFD == NO_CONNECTION)
    	return (COM_ERR_NOCONNECT);
    if (ClientVer < V3_CLIENT)
    	return (COM_ERR_V1CLIENT);
    item = GetMPItemByID(id);
    if (item == NULL) {
    	fprintf(stderr,
    	    "Internal Error: setting control value for non-existing item\n");
    	return (COM_ERR_ERR);
    }
    if (item->type != HS_CONTROL) {
    	fprintf(stderr,
    	    "Internal Error: setting control value for non-control item\n");
    	return (COM_ERR_ERR);
    }
    ControlValue = value;
    sendToClient(CONTROL_SET, id);
    return (COM_OK);
}

/*
** SetTrigger - send msg to client to Set a Trigger 
*/

int SetTrigger(int id)
{
    hsGeneral *item;
    
    if (ComFD == NO_CONNECTION)
    	return (COM_ERR_NOCONNECT);
    if (ClientVer < V3_CLIENT)
    	return (COM_ERR_V1CLIENT);
    item = GetMPItemByID(id);
    if (item == NULL) {
    	fprintf(stderr,
    	    "Internal Error: setting trigger for non-existing item\n");
    	return (COM_ERR_ERR);
    }
    if (item->type != HS_TRIGGER) {
    	fprintf(stderr,
    	    "Internal Error: setting trigger for non-trigger item\n");
    	return (COM_ERR_ERR);
    }
    sendToClient(TRIGGER_SET, id);
    return (COM_OK);
}

/*
** sendToClient - Send message to Client
*/

static int sendToClient(int messageCode, int id)
{
    msgStruct message;
    char msgbuf[sizeof(msgStruct)];
    XDR xdrs;

    if (ComFD == NO_CONNECTION)
    	return NO_CONNECTION;
    message.code = messageCode;
    message.confirm = CONFIRM_CODE;
    message.itemID = id;
    message.dataLength = 0;
    switch (messageCode) {		     /* Messages from Histo-Scope:    */
	case HISTOSCOPE_CONNECTING:
#ifdef COM_DEBUG
	    printf("Sending connecting msg to client.\n");
#endif
	    break;
	case REQUEST_ERRORS:
#ifdef COM_DEBUG
	    printf("Sending request errors msg to client.\n");
#endif
	    break;
	case REQUEST_UPDATES:
#ifdef COM_DEBUG
	    printf("Sending request updates msg to client.\n");
#endif
	    return (sendUpdateReqs(id, &message));
	case CONTROL_SET:
#ifdef COM_DEBUG
	    printf("Sending set control msg to client.\n");
#endif
	    return (sendSetCtrl(&message));
	case TRIGGER_SET:
#ifdef COM_DEBUG
	    printf("Sending set trigger msg to client.\n");
	    ++NumTriggersSet;
#endif
	    break;
	case HISTOSCOPE_DONE:
#ifdef COM_DEBUG
	    printf("Sending done msg to client.\n");
#endif
	    break;
	default:
	    fprintf(stderr,
	       "Program bug, please report: sendToClient - bad msg code.\n");
	    xdr_destroy (&xdrs);
	    return COM_FAILURE;
    }
    xdrmem_create(&xdrs, msgbuf, sizeof(msgbuf), XDR_ENCODE);
    if (xdr_msgStruct(&xdrs, &message) && 
    	    (socket_write(ComFD, msgbuf, sizeof(msgbuf)) == sizeof(msgbuf))) {
	xdr_destroy (&xdrs);
	if (messageCode == HISTOSCOPE_DONE)
	    disconnectProcess();
	return COM_OK;
    }
    xdr_destroy (&xdrs);
    disconnectProcess();
    DialogF(DF_WARN, MainPanelW,1,"Error sending message to attached process\n",
    	"Acknowledged");
    return COM_FAILURE;
}

/*
** Send Update Requests
*/
static int sendUpdateReqs(int numUpdateItems, msgStruct *message)
{
    char *msgbuf;
    int dataLength, i;
    updateListHeader *upd;
    XDR xdrs;

    /* Malloc message and data buffer together, so can use one write */
    dataLength = numUpdateItems * sizeof(updStruct);
    msgbuf = (char *) malloc(sizeof(msgStruct) + dataLength);
    
    /* Translate UpdateList first and make sure of dataLength */
    xdrmem_create(&xdrs, msgbuf+sizeof(msgStruct), dataLength, XDR_ENCODE);
    upd = UpdateList;
    for (i = 0; i < numUpdateItems; ++i, upd = upd->next) {
	if (upd == NULL) {
#ifdef COM_DEBUG
	    printf("in sendUpdateReqs: NumUpdateItems incorrect (=%d).\n",
	    	numUpdateItems);
#endif
	    break;
	}
	updateUpdItem(upd);		/* get current update level for item */
	if (!xdr_updStruct(&xdrs, upd->item)) {
	    xdr_destroy (&xdrs);
	    disconnectProcess();
	    DialogF(DF_WARN, MainPanelW, 1,
		"Error translating update items to attached process\n",
		"Acknowledged");
	    free (msgbuf);
	    return COM_FAILURE;
	}
    }
#ifdef COM_DEBUG
    if (upd != NULL)
    	printf("in sendUpdateReqs: NumUpdateItems incorrect (=%d).\n",
    	    numUpdateItems);
#endif
    i = xdr_getpos(&xdrs);
    if (i != dataLength) {
#ifdef COM_DEBUG
    	printf("in sendUpdateReqs, xdr_getpos != dataLength. (%d != %d)\n", i,
    		dataLength);
#endif
    	dataLength = i;
    }
    xdr_destroy(&xdrs);
    
    /* Fill in message and translate message */
    message->dataLength = dataLength;
    xdrmem_create(&xdrs, msgbuf, sizeof(msgStruct), XDR_ENCODE);
    if (!xdr_msgStruct(&xdrs, message)) {
	xdr_destroy (&xdrs);
	disconnectProcess();
	DialogF(DF_WARN, MainPanelW, 1,
		"Error translating update message to attached process\n",
		"Acknowledged");
	free (msgbuf);
	return COM_FAILURE;
    }
    i = xdr_getpos(&xdrs);
    if (i != sizeof(msgStruct)) {
    	disconnectProcess();
    	DialogF(DF_WARN, MainPanelW, 1,
    		"Internal Error in sendUpdateReqs\n(%d != %d)\n","Acknowledged",
    		 i, sizeof(msgStruct));
	xdr_destroy (&xdrs);
	free (msgbuf);
	return COM_FAILURE;
    }
    xdr_destroy (&xdrs);
    
    /* Write out message and UpdateList (data) in one write statement */
    if (socket_write(ComFD, msgbuf, sizeof(msgStruct)+dataLength) != 
              (int)(sizeof(msgStruct))+dataLength) {
    	disconnectProcess();
    	DialogF(DF_WARN, MainPanelW, 1,
    		"Error sending update message to attached process\n",
    		"Acknowledged");
	free (msgbuf);
    	return COM_FAILURE;
    }
    UpdatesFinished = 0;
    free (msgbuf);
    return COM_OK;
}

/*
** Send Set Control Msg to Client
*/
static int sendSetCtrl(msgStruct *message)
{
    char *msgbuf;
    int dataLength, i;
    XDR xdrs;

    /* Malloc message and data buffer together, so can use one write */
    dataLength = sizeof(float);
    msgbuf = (char *) malloc(sizeof(msgStruct) + dataLength);
    
    /* Translate Control Value */
    xdrmem_create(&xdrs, msgbuf+sizeof(msgStruct), dataLength, XDR_ENCODE);
    if (!xdr_float(&xdrs, &ControlValue)) {
	xdr_destroy (&xdrs);
	disconnectProcess();
	DialogF(DF_WARN, MainPanelW, 1,
	    "Error translating control value to attached process\n",
	    "Acknowledged");
	free (msgbuf);
	return COM_FAILURE;
    }
    i = xdr_getpos(&xdrs);
    if (i != dataLength) {
#ifdef COM_DEBUG
    	printf("in sendSetCtrl, xdr_getpos != dataLength. (%d != %d)\n", i,
    		dataLength);
#endif
    	dataLength = i;
    }
    xdr_destroy(&xdrs);
    
    /* Fill in message and translate message */
    message->dataLength = dataLength;
    xdrmem_create(&xdrs, msgbuf, sizeof(msgStruct), XDR_ENCODE);
    if (!xdr_msgStruct(&xdrs, message)) {
	xdr_destroy (&xdrs);
	disconnectProcess();
	DialogF(DF_WARN, MainPanelW, 1,
		"Error translating set control message to attached process\n",
		"Acknowledged");
	free (msgbuf);
	return COM_FAILURE;
    }
    i = xdr_getpos(&xdrs);
    if (i != sizeof(msgStruct)) {
    	disconnectProcess();
    	DialogF(DF_WARN, MainPanelW, 1,
    		"Internal Error in sendSetCtrl\n(%d != %d)\n","Acknowledged",
    		 i, sizeof(msgStruct));
	xdr_destroy (&xdrs);
	free (msgbuf);
	return COM_FAILURE;
    }
    xdr_destroy (&xdrs);
    
    /* Write out message and Control Value (data) in one write statement */
#ifdef COM_DEBUG
    	printf("\tcontrol value -> %f, dataLength = %d, sizeof(float) = %d, i = %d\n",
    		ControlValue, dataLength, sizeof(float), i);
#endif
    if (socket_write(ComFD, msgbuf, sizeof(msgStruct)+dataLength) != 
                  (int)(sizeof(msgStruct))+dataLength) {
    	disconnectProcess();
    	DialogF(DF_WARN, MainPanelW, 1,
    		"Error sending set control message to attached process\n",
    		"Acknowledged");
	free (msgbuf);
    	return COM_FAILURE;
    }
    free (msgbuf);
    return COM_OK;
}

/*
** updateUpdItem - Update the update level for an update list item with its
**                 histogram count, n-tuple n, or indicator value.
*/

static void updateUpdItem(updateListHeader *upd)
{
    updStruct *updItem;
    hsGeneral *item;
    
    updItem = upd->item;
    item = GetMPItemByID(updItem->itemID);
    switch (item->type) {
    	case HS_1D_HISTOGRAM:
    		updItem->updateLevel = ((hs1DHist *)item)->count;
    		updItem->updateLevelF = 0;
    		break;
    	case HS_2D_HISTOGRAM:
    		updItem->updateLevel = ((hs2DHist *)item)->count;
    		updItem->updateLevelF = 0;
    		break;
    	case HS_3D_HISTOGRAM:
    		updItem->updateLevel = ((hs3DHist *)item)->count;
    		updItem->updateLevelF = 0;
    		break;
    	case HS_NTUPLE:
    		updItem->updateLevel = ((hsNTuple *)item)->n;
    		updItem->updateLevelF = 0;
    		break;
    	case HS_INDICATOR:
    		updItem->updateLevel = ((hsIndicator *)item)->valueSet;
    		updItem->updateLevelF = ((hsIndicator *)item)->value;
    		break;
    	default:
    	    	return;
    }
}

/*
** Disconnect client process
*/
void DisconnectProcess(void)
{
    if (ComFD > 0)
    	sendToClient(HISTOSCOPE_DONE, 0);
    disconnectProcess();
}

static void disconnectProcess(void)
{
    if (ComFD > 0)
    	socket_close (ComFD);		/* disconnect client process */
    ComFD = NO_CONNECTION;
    CloseMainPanel();
    FreeEntireUpdList(&UpdateList);
}
/*
** sock_erstr -  System-specific routine for printing socket errors
*/
static char *sock_erstr(int error_num)
{

#ifdef VMS
#ifdef COM_DEBUG
    socket_perror("Socket error");
#endif /*COM_DEBUG*/
    return vms_errno_string();
#else
#ifdef COM_DEBUG
    perror("Socket error");
#endif /*COM_DEBUG*/
    return strerror(error_num);
#endif /*VMS*/
}

/*
** DisplayWaitMsg
*/
void DisplayWaitMsg(Widget parent, Boolean cancelBtn)
{
    static Widget wk_cancel_but;

    XmString xmstr, xmstr1;
    Arg args[12];
    int argcnt;
    
    if (WaitingMsg == NULL) {     /* Put up waiting for connect msg */
	argcnt = 0;
	XtSetArg(args[argcnt], XmNmessageString, (xmstr=
		XmStringCreateLtoR("Waiting for connection...",
		XmSTRING_DEFAULT_CHARSET))); argcnt++;
	XtSetArg (args[argcnt], XmNdialogTitle, (xmstr1 =
		XmStringCreateLtoR("Working...",
		XmSTRING_DEFAULT_CHARSET))); argcnt++;
	WaitingMsg = XmCreateWorkingDialog (parent, "WaitingMsg", args,
		argcnt);
	XmStringFree ( xmstr );
	XmStringFree ( xmstr1 );
	XtUnmanageChild(XmMessageBoxGetChild (WaitingMsg,
		XmDIALOG_HELP_BUTTON) );
	XtUnmanageChild(XmMessageBoxGetChild (WaitingMsg,
		XmDIALOG_OK_BUTTON) );
	wk_cancel_but = XmMessageBoxGetChild (WaitingMsg,
		XmDIALOG_CANCEL_BUTTON);
	XtAddCallback(wk_cancel_but, XmNactivateCallback,
		(XtCallbackProc) wkCancelButtonCB, (caddr_t)0);
	argcnt = 0;
	XtSetArg(args[argcnt], XmNlabelString, (xmstr =
		XmStringCreateLtoR("Cancel Connect",XmSTRING_DEFAULT_CHARSET)));
		argcnt++;
	XtSetValues (wk_cancel_but, args, argcnt);
	XmStringFree ( xmstr );
    }
    else
	SET_ONE_RSRC(WaitingMsg, XmNmessageString, (xmstr = MKSTRING(
		"Waiting for connection...")));
    if (cancelBtn)
    	XtManageChild(wk_cancel_but);
    else
    	XtUnmanageChild(wk_cancel_but);
/*    XSynchronize(XtDisplay(WaitingMsg), 1);*/	/* synchronize X */
    XtManageChild(WaitingMsg);
/*    XSynchronize(XtDisplay(WaitingMsg), 0);*/	/* disable synchronize X */
    XSync(XtDisplay(WaitingMsg), 0);
}

void UndisplayWaitMsg(void)
{
    if (WaitingMsg != NULL)
	XtUnmanageChild(WaitingMsg);
    WaitingMsg = NULL;
}

static void wkCancelButtonCB(Widget parent, caddr_t client_data,
		caddr_t call_data)
{
    if (ComFD != NO_CONNECTION)
    	DisconnectProcess();
    XtUnmanageChild(WaitingMsg);
}

/*
** ReadHistoFile
**
** Opens a histo-format file and reads all the items in the file.  The number
** of items read is returned: if zero, an error occurred and/or no items
** were read in correctly;  if negative, the absolute value is the number of
** items correctly read before an error occurred.
*/
int ReadHistoFile(char *file)
{

    int i, numItems;
    hsGeneral *item;
    hsFile *hsfile;
    char *errString = NULL;

    /* Open the file to read */
    hsfile = OpenHsFile(file, HS_READ, &errString);
    if (errString != NULL) {
    	DialogF(hsfile == NULL ? DF_ERR : DF_INF, MainPanelW, 1, errString, 
    		"Acknowledged");
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL)
    	return 0;			/* error on open */
    
    /* Read file */
    xdr_DontReadNfitInfo(1);            /* Don't read all nfit info */
    numItems = ReadHsFile (&hsfile, &errString);
    if (errString != NULL) {
    	DialogF(hsfile == NULL ? DF_ERR : DF_INF, MainPanelW, 1, errString, 
    		"Acknowledged");
    	free(errString);
    	errString = NULL;
    }
    if (numItems <= 0) {
    	return 0;			/* error reading file */
    }
    
    /* Go through all the items read, and add to main panel */
    for (i = 0; i < numItems; ++i) {
        item = (hsfile->locTblMem)[i];
    	if (item->type == HS_NFIT)
    	    FreeItem(item);		/* skip nfit items */
    	else
    	    AddItemToMainPanel(item);
     	(hsfile->locTblMem)[i] = NULL;	/* just in case ! */
        
#ifdef COM_DEBUG
    	 printf ("Read item number %d, uid %d \n", i, item->uid);
#endif /* COM_DEBUG */

    }
    FreeHsFileStruct(&hsfile);		/* done with file */
    return numItems;
}

/*
** Sets things up so that if a client is connected from the mainProgram i.e. 
** from the histo command line, instead of via the communications panel,
** AND the user wants to load a configuration file, this configuration file
** will be loaded AFTER we receive the list of items from the client.
*/
void SetConfigFileToLoad(char *cfgFileToLoad)
{
    ConfigFileToLoad = 1;
    strcpy(ConfigFile, cfgFileToLoad);
}

/*
** AddUpdItemToList - Add an update item to the Update List
**
** 	example call: AddUpdItemToList(updItem, &UpdateList);
**
**	note that the list is passed by address so that the list header
**	value can be changed (the new link is added to the beginning).
*/
static void AddUpdItemToList (updStruct *item, updateListHeader **updList)
{
    updateListHeader *temp;

    temp = *updList;
    *updList = (updateListHeader *)malloc(sizeof(updateListHeader));
    (*updList)->next = temp;
    (*updList)->item = item;
    ++NumUpdateItems;
}

/*
**   GetUpdItemByID - retrieve pointer to Update item from specified update
**		      list by supplying its id.
*/
static updStruct *GetUpdItemByID (int id, updateListHeader *updList) 
{
    updateListHeader *h;
    
    for (h = updList; h != NULL; h = h->next)
	if (h->item->itemID == id)
	    return h->item;
    /* Item not found */
    return NULL;
}

/*
**   DeleteUpdItemFromList - Delete an update item from the update list.
**
** 	example call: DeleteUpdItemFromList(id, &UpdateList);
**
**	note that the list is passed by address in case the item to be deleted
**	is the first item in the list (and the header value needs to change).
*/
static void DeleteUpdItemFromList (int id, updateListHeader **updList)
{
    updateListHeader *temp, *toFree = NULL;

    if (*updList == NULL) {
    	fprintf(stderr, "DeleteUpdItemFromList - Error: list is empty.\n");
    	return;
    }
    if ((*updList)->item->itemID == id) {
	toFree = *updList;
	*updList = toFree->next;
	--NumUpdateItems;
    } else {
	for (temp = *updList; temp != NULL; temp = temp->next) {
	    if (temp->next == NULL)
	    	break;
	    if (temp->next->item->itemID == id) {
	        toFree = temp->next;
		temp->next = toFree->next;
		--NumUpdateItems;
		break;
	    }
	}
    }
    if (toFree != NULL) {
    	free(toFree->item);
    	free(toFree);
    }
    else
    	fprintf(stderr, 
    	    "DeleteUpdItemFromList - Error: item not found. id = %d\n", id);
}

/*
**   FreeEntireUpdList - Free an entire update list and the update items in it.
**
** 	example call:  FreeEntireUpdList(&UpdateList);
**
**	note that the list is passed by address because the listheader will
**	be set to null.
*/
static void FreeEntireUpdList(updateListHeader **updList)
{
    updateListHeader *temp, *toFree;

    toFree = *updList;
    *updList = NULL;
    while (toFree != NULL) {
	temp = toFree->next;
	free(toFree->item);
	free(toFree);
	toFree = temp;
    }
    NumUpdateItems = 0;
}

static void loadConfigFile(hsGeneral *hsG)
{
    hsConfigString *hsC = (hsConfigString *)hsG;
    char *buffer;
    
    buffer = (char *)malloc(hsC->stringLength + 1);
    if (buffer == NULL)
    {
	fprintf(stderr, "Can't load config string: out of memory\n");
	fflush(stderr);
	return;
    }
    memcpy(buffer, hsC->configString, hsC->stringLength);
    buffer[hsC->stringLength] = '\0';
    ParseConfigBuffer(MainPanelW, buffer);
    free(buffer);
    free(hsC->configString);
    free(hsC);
}

/*
** Setting the scaling for a plot whose data we don't yet have, is not always
** possible.  The widgets need at least some data as a frame of reference
** to make the limits meaningful, and until they have that, they ignore
** scale information until they have some data to apply it to.  The routine
** that calls ScheduleDataArrivedCallback collects the data and specifies the
** routine for serviceDataWaitCB to call when data from an attached process
** finally arrives.  
*/
void ScheduleDataArrivedCallback(int itemID, deferProc deferredCB, 
	void *dataPtr) 
{
    addItemIdToDeferList(itemID, deferredCB, dataPtr, &DeferListHdr);
}

static Boolean serviceDataWaitCB(int itemID)
{
    deferStruct *dfr;
    
    dfr = takeDeferItemByID(itemID, &DeferListHdr);
    if (dfr == NULL)
    	return False;
    (dfr->deferredCB)(dfr->dataPtr);
    free(dfr);
    return True;
}

/*
** addItemIdToDeferList - Create & add a node to a deferlist for this itemID
**
** 	example call: 
**
**           addItemIdToDeferList(itemID, deferredCB, dataPtr, &deferList);
**
**	note that the deferList is passed by address so that the list header
**	value can be changed (the new link is added to the beginning).
*/
static void addItemIdToDeferList(int itemID, deferProc deferredCB, 
	void *dataPtr, deferListHeader **deferListHdr)
{
    deferListHeader *temp;
    deferStruct *dfr;

    temp               = *deferListHdr;
    *deferListHdr      = (deferListHeader *) malloc(sizeof(deferListHeader));
    dfr                = (deferStruct *) malloc(sizeof(deferStruct));
    (*deferListHdr)->next = temp;
    (*deferListHdr)->item = dfr;
    dfr->itemID        = itemID;
    dfr->deferredCB    = deferredCB;
    dfr->dataPtr       = dataPtr;
}

/*
**   takeDeferItemByID - get a defer item from specified deferList by supplying
**		         its id and take (i.e.delete) it from the list.
*/
deferStruct *takeDeferItemByID(int id, deferListHeader **deferListHdr)
{
    deferListHeader *h, *toFree;
    deferStruct *toRtn;
    
    if (*deferListHdr == NULL)
    	return NULL;
    if ((*deferListHdr)->item->itemID == id) {
	toRtn         = (*deferListHdr)->item;
	toFree        = *deferListHdr;
	*deferListHdr = (*deferListHdr)->next;
	free(toFree);
	return toRtn;
    } else
	for (h = *deferListHdr; h != NULL; h = h->next) {
	    if (h->next == NULL)
	    	return NULL;
	    if (h->next->item->itemID == id) {
		toFree  = h->next;
		h->next = toFree->next;
		toRtn   = toFree->item;
		free(toFree);
		return toRtn;
	    }
    	}
    	
    /* Item not found */
    return NULL;
}

int ReportTaskCompletion(int taskNumber, int status,
			 const char *result)
{
    msgStruct message;
    int len, exitStatus = COM_FAILURE;
    char *msgbuf;
    XDR xdrs;

    if (taskNumber <= 0)
	return COM_OK;
    if (ComFD == NO_CONNECTION)
    	return NO_CONNECTION;
    if (status)
	message.code = HISTO_TASK_FAILURE;
    else
	message.code = HISTO_TASK_OK;
    message.confirm = CONFIRM_CODE;
    message.itemID = taskNumber;
    if (result)
	len = strlen(result);
    else
	len = 0;
    message.dataLength = len;
    msgbuf = (char *)malloc(sizeof(msgStruct) + len);
    if (msgbuf == NULL)
    {
	fprintf(stderr, "Error in ReportTaskCompletion: out of memory!\n");
	return COM_FAILURE;
    }
    xdrmem_create(&xdrs, msgbuf, sizeof(msgStruct), XDR_ENCODE);
    if (!xdr_msgStruct(&xdrs, &message))
	goto fail0;
    if (len > 0)
	strncpy(msgbuf+sizeof(msgStruct), result, len);

    /* The following assumes that the result size
       is small, and can fit into one packet... */
    if (socket_write(ComFD, msgbuf, sizeof(msgStruct)+len) !=
	(int)(sizeof(msgStruct))+len)
    {
	DialogF(DF_WARN, MainPanelW, 1, "Error sending task report "
		"to attached process\n", "Acknowledged");
	goto fail0;
    }

    exitStatus = COM_OK;
 fail0:
    xdr_destroy(&xdrs);
    free(msgbuf);
    return exitStatus;
}
