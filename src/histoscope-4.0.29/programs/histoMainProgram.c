/*******************************************************************************
*									       *
* histoMainProgram.c -- HISTOSCOPE main program and event loop		       *
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
* May 11, 1992								       *
*									       *
* Written by Joy Kyriakopulos & Mark Edel				       *
*									       *
* Modified March 10, 1993 by Joy Kyriakopulos for VMS support.		       *
*									       *
*******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#if XmVersion >= 1002
#include <Xm/PrimitiveP.h>
#endif
#include "../util/misc.h"
#include "../util/printUtils.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/publish.h"
#include "histoP.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "histoMainProgram.h"
#include "communications.h"
#include "preferences.h"
#include "configFile.h"
#include "dragPlot.h"
#include "ColorScale.h"
#include "gsComm.h"
#ifdef VMS
#include "../util/vmsUtils.h"
#endif /*VMS*/
#ifdef linux
#include <linux/types.h>
#endif /*linux*/

#define APP_NAME "histo"	/* application name for loading X resources */
#define APP_CLASS "HistoScope"	/* application class for loading X resources */

/* Function Prototypes */
static void histoMainLoop(Display *display, XtAppContext context,Widget parent);
static void registerPixmaps();
static int maxInt(int a, int b);

static char *fallbackResources[] = {
    "*.plotWidget.background: #e5e5e5e5e5e5", /* gray90 */
    "*.plotWidget.foreground: black",
    0};

/* Global Variables */
int ComFD = NO_CONNECTION;		/* File descriptor for socket
    			   		   communications w/histo client  */
/*
** Histoscope tool main program
*/
int main(int argc, char **argv)
{
    XtAppContext context;
    Display *display;
    Widget mainPanel;
    char *setenvVar;
    XrmDatabase prefDB;
    int i, portNum = -1;
    int fileFormat = HISTO_FORMAT, captiveConnect = FALSE;
    char *fileName = NULL, *configFile = NULL;
    char *clientid = NULL, *rhost = NULL, *ruser = NULL;
    int blockSize = DEFAULT_HBOOK_BLOCK_SIZE;
    
    /* Initialize toolkit and create the obligatory application context */
    XtToolkitInitialize();
    context = XtCreateApplicationContext();
    
    /* Motif still has problems with releasing non-existent passive grabs */
    SuppressPassiveGrabWarnings();

    /* Set up default resources if no app-defaults file is found */
    XtAppSetFallbackResources(context, fallbackResources);
    
#ifdef VMS
    /* Convert VMS style command line to Unix style */
    ConvertVMSCommandLine((int *)&argc, &argv);
#endif /*VMS*/
    
    /* Read the preferences file and command line into a database */
    {
        unsigned uargc = argc;
        prefDB = CreateHistoPrefDB(PREF_FILE_NAME, APP_NAME, &uargc, argv);
        argc = uargc;
    }

    /* Open the Display */
    display = XtOpenDisplay(context, NULL, APP_NAME, APP_CLASS, NULL,
                            0, (int *)&argc, argv);
    if (!display) {
	fprintf(stderr,"Histo-Scope: Can't open display\n");
	exit(EXIT_FAILURE);
    }

    /* Store preferences from the command line and .nedit file  */
    RestoreHistoPrefs(prefDB, XtDatabase(display), APP_NAME, APP_CLASS);
    LoadPrintPreferences(XtDatabase(display), APP_NAME, APP_CLASS, True);

    /* Register pixmaps for use with Motif */
    registerPixmaps();

    /*
    ** Check if running as a subprocess of a client, and
    ** Create the histoscope main window
    */
    setenvVar = getenv("HISTOCONNECT");		/* publish string */
    if (setenvVar != NULL) {
#ifdef COM_DEBUG
    	printf("HISTOCONNECT = %s\n", setenvVar);
#endif
	mainPanel = CreateMainWindow(display, True);
	if (!CaptiveConnect(setenvVar)) {
	    fprintf(stderr,
	    	"Histo-Scope: Error connecting to process, Exiting...\n");
	    exit(EXIT_FAILURE);
	}
	captiveConnect = TRUE;
    } else {
        mainPanel = CreateMainWindow(display, False);
    }

    /* Figure out how many colors we can use. This should
       be done before we create any color scale. */
    setMaxColorScaleColors(guessMaxColorScaleColors(
	display, XScreenNumberOfScreen(XtScreen(mainPanel))));

    /*
    ** Read command line arguments
    */
    for (i=1; i<argc; i++) {
    	if (*argv[i] == '-') {
    	    if (!strcmp(argv[i], "-hbook")) {
 		if (fileName != NULL) {
	    	    fprintf(stderr, "histo: can't open more than one file\n");
	    	    exit(EXIT_FAILURE);
		}
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -hbook requires an argument\n");
		    exit(EXIT_FAILURE);
		}
		fileName = argv[i];
		fileFormat = HBOOK_FORMAT;
	    } else if (!strcmp(argv[i], "-blocksize")) {
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -blocksize requires an argument\n");
		    exit(EXIT_FAILURE);
		}
	    	if (sscanf(argv[i], "%d", &blockSize) != 1) {
	    	    fprintf(stderr, "histo: can't read -blocksize argument\n");
		    exit(EXIT_FAILURE);
		}
	    } else if (!strcmp(argv[i], "-config")) {
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -config requires an argument\n");
		    exit(EXIT_FAILURE);
		}
	    	configFile = argv[i];
	    } else if (!strcmp(argv[i], "-clientid")) {
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -clientid requires an argument\n");
		    exit(EXIT_FAILURE);
		}
	    	clientid = argv[i];
	    } else if (!strcmp(argv[i], "-rhost")) {
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -rhost requires an argument\n");
		    exit(EXIT_FAILURE);
		}
	    	rhost = argv[i];
	    } else if (!strcmp(argv[i], "-port")) {
		char *endptr;
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -port requires an argument\n");
		    exit(EXIT_FAILURE);
		}
	    	portNum = strtoul(argv[i], &endptr, 0);
		if (*endptr != '\0')
		{
		    fprintf(stderr,
			    "histo: expected a port number, got \"%s\"\n",
			    argv[i]);
		    exit(EXIT_FAILURE);
		}
	    } else if (!strcmp(argv[i], "-ruser")) {
		i++;
		if (i >= argc) {
		    fprintf(stderr, "histo: -ruser requires an argument\n");
		    exit(EXIT_FAILURE);
		}
	    	ruser = argv[i];
	    }
   	} else {
	    if (fileName != NULL) {
	    	fprintf(stderr, "histo: can't open more than one file\n");
	    	exit(EXIT_FAILURE);
	    }
	    if (clientid != NULL || portNum >= 0) {
	    	fprintf(stderr, "histo: can't open both a file and a client\n");
	    	exit(EXIT_FAILURE);
	    }
	    fileName = argv[i];
	    fileFormat = HISTO_FORMAT;
        }
    }

    /*
    ** If a file was specified on the command line, read it in
    */
    if (fileName != NULL) {
#ifdef VMS
	char **nameList = NULL;
	int numFiles;

	/* Use VMS's LIB$FILESCAN for fileName in argv[i] to process */
	/* wildcards and to obtain a full VMS file specification     */
	numFiles = VMSFileScan(fileName, &nameList, NULL, EXCLUDE_FNF);
	if (numFiles == 0)
	    exit(EXIT_FAILURE);
	if (numFiles != 1) {
	    fprintf(stderr, "histo: can't open more than one file\n");
	    exit(EXIT_FAILURE);
	}
	if (fileFormat == HISTO_FORMAT)
    	    OpenInitialHistoFile(nameList[0]);
	else
    	    OpenInitialHbookFile(nameList[0], blockSize);
	free(nameList[0]);
	free(nameList);
#else
	if (fileFormat == HISTO_FORMAT)
    	    OpenInitialHistoFile(fileName);
	else
    	    OpenInitialHbookFile(fileName, blockSize);
#endif /*VMS*/
    } else {
	/*
	 * Try to connect to the given port
	 */
	if (portNum >= 0 && rhost != NULL)
	{
	    if (ConnectToClient(rhost, portNum) != COM_OK) {
		fprintf(stderr, "histo: error connecting to client\n");
		exit(EXIT_FAILURE);
	    } else {
		char nameToReturn2[MAXHOSTNAMELEN+MAXPATHLEN+1];
		char cport[21];

		strcpy(nameToReturn2, rhost);
		strcat(nameToReturn2, ":");
		sprintf(cport, "%d", portNum);
		strcat(nameToReturn2, cport);
		SetMainPanelConnected(nameToReturn2);
	    }
	}
    	/*
    	** If a client id-string was specified, try to attach to client
    	*/
    	else if (clientid != NULL) {
	    int numOfProcs, connMade = FALSE;
	    idFileData *listOfProcs = NULL;	/* returned by ListIDFiles */
	    char errorMessage[MAX_LIST_ERROR_LEN]; 
	    
	    numOfProcs = ListIDFiles(rhost, &listOfProcs, errorMessage, 
	    	ruser, NULL);
    	    if (numOfProcs == -1) {
		fprintf(stderr, "histo: error finding client:\n    %s", 
		    errorMessage);
		exit(EXIT_FAILURE);
    	    }
    	    for (i = 0; i < numOfProcs; ++i) {
    	        if (strcmp(listOfProcs[i].idString, clientid) == 0)
		{
    	            if (ConnectToClient(listOfProcs[i].node, 
    	            	    listOfProcs[i].socketNum) != COM_OK) {
    	            	fprintf(stderr, "histo: error connecting to client\n");
    	            	exit(EXIT_FAILURE);
    	            } else {
    	            	char nameToReturn[MAXHOSTNAMELEN+MAXPATHLEN+1];
    	            	char pid[21];

    	            	if (rhost == NULL)
    	            	    nameToReturn[0] = '\0';
    	            	else {
    	            	    strcpy(nameToReturn, rhost);
    	            	    strcat(nameToReturn, ":");
    	            	}
    	            	strcat(nameToReturn, clientid);
    	            	strcat(nameToReturn, " ");
    	            	sprintf(pid, "%d", listOfProcs[i].pid);
			strcat(nameToReturn, pid);
			strcat(nameToReturn, " ");
			strcat(nameToReturn, listOfProcs[i].startTime);
    	            	SetMainPanelConnected(nameToReturn);
    	            	connMade = TRUE;
    	            }
		}
	    }
    	    if (connMade == FALSE) {
    		fprintf(stderr, "histo: can't find client process\n");
    		exit(EXIT_FAILURE);
    	    }
    	}
    }
    
    /* Add the required actions for our application context to allow dragging
    ** and dropping plot widgets into multiple plot windows 
    */
    AddActionsForDraggingPlots(context);
    
    /* If a configuration file was specified, load it */
    if (configFile != NULL) {
        if (clientid != NULL || captiveConnect)
            /*
            ** However if a client is connected, it first has to give us
            ** its list of histograms, so instead tell communications code
            ** to load the configuration file after it receives a list.
            */
            SetConfigFileToLoad(configFile);
        else
            ReadConfigFile(mainPanel, configFile);
    }
    
    /*
    ** Process events.
    */
    mainLoopStartsNow(context);
    histoMainLoop(display, context, mainPanel);
    return 0;
}

/*
** histoMainLoop -- process events and socket communication
**
** This routine is meant to replace the call to XtMainLoop so that socket
** events can be dispatched in addition to X events in an application.
**
** Example call format:
**
**   histoMainLoop (display, context, parent);
*/
#ifdef VMS
/* VMS Notes:
**
**    VAX C has no select statement.  Thus we just call XtAppMainLoop and 
**    rely on the timer set in the ConnectToClient module for polling
**    socket input from a client.
*/
static void histoMainLoop(Display *display, XtAppContext context, Widget parent) 
{
    XtAppMainLoop(context);
}
#else /*VMS*/
/* Unix Notes:
**
**   If a process is blocked on a select waiting for input from a
**   socket and the sending process closes the socket, the select
**   notes this as an exception rather than as data.  Hence, if the
**   select is not currently looking for exceptions, it waits indefinitely.
**   
**   The timeout set in select is awfully short because
**   stupid motif does nothing to wake up select if the user just
**   holds down the mouse button.
*/
static void histoMainLoop(Display *display, XtAppContext context, Widget parent) 
{
    fd_set readfds;
    int maxfds = 0;	       /* maximum descriptor # for select to check on */
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    while (TRUE) {
    
	/* Process all pending X events, regardless of whether	*/
	/* select says there are any. Some things, like holding */
	/* down a scroll bar button are not flagged by select.	*/
	while (XtAppPending(context)) {
	    XtAppProcessEvent(context, XtIMAll);
	}

	/* set file descriptor bits for:  X, accepting socket
	   connection, 2-way socket communication w/client process */
	FD_ZERO(&readfds);
	if (ComFD != NO_CONNECTION)
	    FD_SET(ComFD, &readfds);
	FD_SET(ConnectionNumber(display), &readfds);

	/* maximum descriptor # for select to check on */
	maxfds = 1 + maxInt(ComFD, ConnectionNumber(display));
			
	/* Set timeout for 50 msec, the scroll bar repeat delay 
	   which does not wake up select properly		*/
	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;

	/* Block and wait for something to happen */
	if (select(maxfds, &readfds, NULL, NULL, &timeout) == -1) {
	    /* if error return other than signal delivered, print error */
	    if (EINTR != errno)	 
		perror("Error in select(): ");
	}
	
	if (ComFD != NO_CONNECTION && FD_ISSET(ComFD, &readfds)) {
	    /* check for incoming data from client process on socket */
	    ReadClientCom(parent);
	}
    }
}
#endif /*VMS*/

/*
** Register bitmap images for later use in Motif widgets.  So far the only
** bitmaps required by HistoScope are the patterns used to create dashed
** outlines around plot windows with data errors, and invalidated plot windows.
*/
static void registerPixmaps()
{
    /* XmInstall image does not make a local copy of images, allocate static */
    static XImage errImage, invImage;
    static unsigned char errorBits[] = {
    	0x1f, 0xf0, 0x3f, 0xe0, 0x7f, 0xc0, 0xff, 0x80, 0xff, 0x01, 0xfe, 0x03,
	0xfc, 0x07, 0xf8, 0x0f, 0xf0, 0x1f, 0xe0, 0x3f, 0xc0, 0x7f, 0x80, 0xff,
	0x01, 0xff, 0x03, 0xfe, 0x07, 0xfc, 0x0f, 0xf8};
    static unsigned char invalidBits[] = {
	0x33, 0x33, 0x66, 0x66, 0xcc, 0xcc, 0x99, 0x99, 0x33, 0x33, 0x66, 0x66,
	0xcc, 0xcc, 0x99, 0x99, 0x33, 0x33, 0x66, 0x66, 0xcc, 0xcc, 0x99, 0x99,
	0x33, 0x33, 0x66, 0x66, 0xcc, 0xcc, 0x99, 0x99};
    
    memset(&errImage, 0, sizeof(XImage));
    errImage.width = 16;
    errImage.height = 16;
    errImage.data = (char *)errorBits;
    errImage.depth = 1;
    errImage.xoffset = 0;
    errImage.format = XYBitmap;
    errImage.byte_order = LSBFirst;
    errImage.bitmap_unit = 8;
    errImage.bitmap_bit_order = MSBFirst;
    errImage.bitmap_pad = 8;
    errImage.bytes_per_line = 2;
    
    XmInstallImage(&errImage, "errorWindowShadow");
    memcpy(&invImage, &errImage, sizeof(XImage));
    invImage.data = (char *)invalidBits;
    XmInstallImage(&invImage, "invalidWindowShadow");
}

/*
** maxInt -- return the larger of two ints
*/
static int maxInt(int a, int b)
{
    if (a >= b) return a;
    else return b;
}

