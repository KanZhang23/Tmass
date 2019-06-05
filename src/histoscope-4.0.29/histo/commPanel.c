/*******************************************************************************
*                                        				       *
* commPanel.c -- Communications panel for Nirvana Histoscope tool	       *
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
* April 24, 1992							       *
*									       *
* Written by Joy Kyriakopulos						       *
* 									       *
* Modified by Joy Kyriakopulos 3/15/93 for VMS Port			       *
*									       *
*******************************************************************************/

#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "commPanel.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "communications.h"
#include "histoMainProgram.h"
#include "../histo_util/publish.h"
#include "../util/DialogF.h"
#include "../util/help.h"
#include "../util/misc.h"

#define INITIAL_LIST_HEIGHT 8	/* number of rows in process list */
#define MAX_PASSWORD_LENGTH 20

static Widget NodeNameText;
static Widget UserNameText;
#ifdef VMS
static Widget UserPasswordText;
#endif
static Widget ProcessList;
static Widget ConnectBtn;
static Widget SearchBtn;
static Widget CancelBtn;
static Widget HelpBtn;
static Widget Form = NULL;
static Widget UsrNameLabel;
static Widget D1, D2, D3, D4;		/* Error/Warning "dialogs" */
static Boolean ConnMade = False;
static Boolean ClientIsLocal;  /* if false, client selected is on remote node */
static char MyHostName[MAXHOSTNAMELEN+1];
static char HostName[MAXHOSTNAMELEN+1];
static char NameToReturn[MAXHOSTNAMELEN+MAXPATHLEN+1];
static char *Passwd[MAX_PASSWORD_LENGTH+1]; 
static XmString *ProcsListed = NULL;
static idFileData *ListOfProcs = NULL;	/* list returned by ListIDFiles */
static int NumOfProcs;			/* number of procs " "   "      */
#ifdef VMS
static char *HelpText = "\nThe Available Processes window initially pops up \
with a list of all processes on the current system or cluster that \
Histo-Scope can \
connect to.  That is, each running process is listed that has called the \
hs_initialize routine thereby making itself known to Histo-Scope.  On \
VMS, Histo-Scope looks in the disk directory pointed to by the logical name \
HISTO_PROCESS_DIR for running process identifications.\n\n\
If you would like to connect to a process on another system or cluster, use \
the mouse \
to click on the Remote Node box and enter the nodename for the system or \
cluster alias.  Then click on the Search Node button.  \
After a pause, the processes known to Histo-Scope that are running on that \
system will be listed.  Unless you enter a remote username to use, MultiNet \
will use your local username to access the remote system.  A password typed \
will be used only when you have entered a username.\n\n\
If you get a Permission denied error, this means \
that you have not set up a MultiNet SYS$LOGIN:.RHOSTS file containing your \
current nodename and username on the remote \
system (or a .rhosts file in your home directory if you've specified a Unix \
system).  Check the spelling of the node, username, or password and \
the nodes and usernames listed in that remote \
SYS$LOGIN:.RHOSTS file.  (Refer to the MultiNet documentation for further \
information.)\
\n\nTo connect to a process, select the desired process \
with the mouse and click on the Connect button.\n\nIf you \
decide you do not want to connect to a process, clicking on the Cancel button \
will dismiss the window without performing any other action.\n\n\
Occasionally processes will be listed that are not actually running.  This can \
happen if the process has terminated without calling hs_complete.";
#else
static char *HelpText = "\nThe Available Processes window initially pops up \
with a list of all processes on the current system that Histo-Scope can \
connect to.  That is, each running process is listed that has called the \
hs_initialize routine thereby making itself known to Histo-Scope.\n\n\
If you would like to connect to a process on another system, use the mouse \
to click on the Remote Node box and enter the nodename for the system. \
Then click on the Search Node button. \
After a pause, the processes known to Histo-Scope that are running on that \
system will be listed.  (If you get a Permission denied error, this means \
that you have not set up the .rhosts file on the remote node you've entered \
or have mis-typed something.  \
Be sure your current node and username is entered into that remote .rhosts \
file.)\n\n\
Optionally, you can enter a username to use in accessing a remote system.  \
If a username is specified, its .rhosts file will be checked for access from \
your local system and username.\n\n\
To connect to a process, select the desired process \
with the mouse and click on the Connect button.\n\nIf you \
decide you do not want to connect to a process, clicking on the Cancel button \
will dismiss the window without performing any other action.\n\n\
Occasionally processes will be listed that are not actually running.  This can \
happen if the process has terminated without calling hs_complete.";
#endif /*VMS*/

static void createCommDialog(Widget parent);
static XmString* stringTable(int count, idFileData *proclist);
static void freeStringTable(XmString *table);
static XmString *findProcs(char *);
static void reInitDialog(void);

static void connectCB(Widget w, caddr_t clientData, caddr_t callData);
static void cancelCB(Widget w, caddr_t clientData, caddr_t callData);
static void helpCB(Widget w, caddr_t clientData, caddr_t callData);
static void searchCB(Widget w, caddr_t clientData, caddr_t callData);
static void listCB(Widget w, caddr_t clientData, caddr_t callData);
static void txtFocusCB(Widget w, caddr_t clientData, caddr_t callData);
static void txtLostFocusCB(Widget w, caddr_t clientData, caddr_t callData);

/*
** Connect to a client process of user's choice
*/

void ConnectToProcess(Widget parent)
{
    struct hostent *hp;			/* for gethostbyname */

    if (ComFD > 0) {			/* Is there already a connection? */
    	if (DialogF(DF_QUES, parent, 2,
    		"Disconnect current process to\nconnect to new process?",
    		"OK", "Cancel") == 2)
    	    return;
    	else {
    	    ConnMade = False;
    	    DisconnectProcess();
    	}
    }
    if (gethostname(MyHostName, MAXHOSTNAMELEN+1) < 0) { /* get loc host name */
    	DialogF(DF_ERR, parent, 1, "Error getting host name", "Acknowledged");
    	return;
    }
    ClientIsLocal = True;		/* By default */
    if (Form == NULL) {
	hp = gethostbyname(MyHostName);
	if (hp == 0) {
	    DialogF(DF_ERR, parent, 1,
	      "Error getting official local hostname\n(%s in ConnectToProcess)",
	      "acknowledged", strerror(errno));
            return;
	}
	strcpy(MyHostName, hp->h_name);	/* get OFFICIAL local host name */
    	createCommDialog(parent);	/* Create communications dialog   */
    }
    memset(HostName, 0, MAXHOSTNAMELEN+1);
    if (ProcsListed != NULL)
    	freeStringTable(ProcsListed);
    ProcsListed = findProcs("");	/* Fill list with local processes */
    XtManageChild(Form);		/* Pop it up,                     */
}

/*
** ClientIsConnected - sets dialogs & windows into proper state after connection
*/

void ClientIsConnected(void)
{
    if (Form == NULL)
    	return;
    XtUnmanageChild(Form);
    reInitDialog();			/* Set dialog to creation state   */
    SetMainPanelConnected(NameToReturn);
}

/*
** Create the Histoscope communications dialog
*/

static void createCommDialog(Widget parent)
{
    Arg args[50];
    int ac;
    XmString s1, s2, s3;
    Widget processLabel, rmtNodeLabel;
#ifdef VMS
    Widget usrPasswordLabel;
#endif /*VMS*/

    /* Create the form onto which everything goes */
    ac = 0;
    XtSetArg (args[ac], XmNdialogTitle, (s1=MKSTRING("Available Processes")));
    		ac++;
    XtSetArg (args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);ac ++;
    XtSetArg(args[ac], XmNautoUnmanage, False); ac++;
    Form = XmCreateFormDialog(parent, "form", args, ac);
    XmStringFree(s1);
    XtAddCallback(Form, XmNhelpCallback,
                  (XtCallbackProc)  helpCB, NULL);



    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Connect"))); ac++;
    XtSetArg(args[ac], XmNshowAsDefault, True); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNrightWidget, SearchBtn); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    XtSetArg(args[ac], XmNleftOffset, 6); ac++;
    ConnectBtn = XmCreatePushButton(Form, "connectBtn", args, ac);
    XtAddCallback(ConnectBtn, XmNactivateCallback,
                  (XtCallbackProc)  connectCB, NULL);
    XmStringFree(s1);
    XtManageChild(ConnectBtn);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Search Node"))); ac++;
    XtSetArg(args[ac], XmNleftWidget, ConnectBtn); ac++;
    XtSetArg(args[ac], XmNrightWidget, CancelBtn); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftOffset, 12); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    SearchBtn = XmCreatePushButton(Form, "searchBtn", args, ac);
    XtAddCallback(SearchBtn, XmNactivateCallback,
                  (XtCallbackProc)  searchCB, NULL);
    XmStringFree(s1);
    XtManageChild(SearchBtn);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Cancel"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, SearchBtn); ac++;
    XtSetArg(args[ac], XmNrightWidget, HelpBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    XtSetArg(args[ac], XmNleftOffset, 12); ac++;
    CancelBtn = XmCreatePushButtonGadget(Form, "cancelBtn", args,ac);
    XtAddCallback(CancelBtn, XmNactivateCallback,
                  (XtCallbackProc)  cancelCB, NULL);
    XmStringFree(s1);
    XtManageChild(CancelBtn);
   
    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Help"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, CancelBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 10); ac++;
    XtSetArg(args[ac], XmNleftOffset, 12); ac++;
    XtSetArg(args[ac], XmNrightOffset, 6); ac++;
    HelpBtn = XmCreatePushButton(Form, "helpBtn", args,ac);
    XtAddCallback(HelpBtn, XmNactivateCallback,
                  (XtCallbackProc)  helpCB, NULL);
    XmStringFree(s1);
    XtManageChild(HelpBtn);

/******************************************************************/  

#ifdef VMS
    ac = 0;				/* rsh password avail only on VMS */
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Password"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNbottomWidget, ConnectBtn); ac++;
    XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 14); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    usrPasswordLabel = XmCreateLabelGadget(Form, "usrPasswordLabel", args, ac);
    XmStringFree(s1);
    XtManageChild(usrPasswordLabel);
    
    ac = 0;
    XtSetArg(args[ac], XmNrows, (short)1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftWidget, usrPasswordLabel); ac++;
    XtSetArg(args[ac], XmNbottomWidget, ConnectBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 5); ac++;
    XtSetArg(args[ac], XmNleftOffset, 12); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    XtSetArg(args[ac], XmNmaxLength, MAX_PASSWORD_LENGTH); ac++;
    UserPasswordText = XmCreateText(Form, "userPasswordText", args, ac);
    PasswordText(UserPasswordText, Passwd); /* sets Passwd whenever usr types */
    XtManageChild(UserPasswordText);
    RemapDeleteKey(UserPasswordText);
    XtAddCallback(UserPasswordText, XmNfocusCallback,
                  (XtCallbackProc)  txtFocusCB, NULL);

#endif /*VMS*/
    
    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Username"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
#ifdef VMS
    XtSetArg(args[ac], XmNbottomWidget, usrPasswordLabel); ac++;
#else
    XtSetArg(args[ac], XmNbottomWidget, ConnectBtn); ac++;
#endif /*VMS*/
    XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 14); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    UsrNameLabel = XmCreateLabelGadget(Form, "usrNameLabel", args, ac);
    XmStringFree(s1);
    XtManageChild(UsrNameLabel);
    
    ac = 0;
    XtSetArg(args[ac], XmNrows, (short)1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftWidget, UsrNameLabel); ac++;
#ifdef VMS
    XtSetArg(args[ac], XmNbottomWidget, usrPasswordLabel); ac++;
#else
    XtSetArg(args[ac], XmNbottomWidget, ConnectBtn); ac++;
#endif /*VMS*/
    XtSetArg(args[ac], XmNbottomOffset, 5); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    UserNameText = XmCreateText(Form, "usrNameText" ,args, ac);
    XtManageChild(UserNameText);
    RemapDeleteKey(UserNameText);
    XtAddCallback(UserNameText, XmNfocusCallback,
                   (XtCallbackProc)  txtFocusCB, NULL);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Remote Node"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNbottomWidget, UsrNameLabel); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 14); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    rmtNodeLabel = XmCreateLabelGadget(Form, "rmtNodeLabel", args, ac);
    XmStringFree(s1);
    XtManageChild(rmtNodeLabel);
    
    ac = 0;
    XtSetArg(args[ac], XmNrows, (short)1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNbottomWidget, UsrNameLabel); ac++;
    XtSetArg(args[ac], XmNleftWidget, rmtNodeLabel); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 5); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    NodeNameText = XmCreateText(Form, "nodeNameText" ,args, ac);
    XtManageChild(NodeNameText);
    XtAddCallback(NodeNameText, XmNfocusCallback,
                 (XtCallbackProc) txtFocusCB, NULL);
    XtAddCallback(NodeNameText, XmNlosingFocusCallback,
                   (XtCallbackProc) txtLostFocusCB, NULL);
    RemapDeleteKey(NodeNameText);

/******************************************************************************/
    ac = 0;
    XtSetArg(args[ac], XmNlabelString,
    	     (s1=MKSTRING("Connect to:   (choose one)"))); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNtopOffset, 10); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    processLabel = XmCreateLabelGadget(Form, "processLabel", args, ac);
    XmStringFree(s1);
    XtManageChild(processLabel);
    
    ac = 0;
    XtSetArg(args[ac], XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
    XtSetArg(args[ac], XmNvisibleItemCount, INITIAL_LIST_HEIGHT); ac++;
    XtSetArg(args[ac], XmNselectionPolicy, XmBROWSE_SELECT); ac++;
    XtSetArg(args[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNtopWidget, processLabel); ac++;
    XtSetArg(args[ac], XmNbottomWidget, rmtNodeLabel); ac++;
    XtSetArg(args[ac], XmNtopOffset, 10); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 18); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    ProcessList = XmCreateScrolledList(Form, "processList", args, ac);
    XtManageChild(ProcessList);
    XtAddCallback(ProcessList, XmNdefaultActionCallback,
                  (XtCallbackProc)  listCB, NULL);

    SET_ONE_RSRC(Form, XmNdefaultButton, ConnectBtn);

    ac = 0;
    XtSetArg(args[ac], XmNmessageString,
    	  (s1=MKSTRING("Please select a process, and\n then press Connect")));
    	  ac++;
    XtSetArg (args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg (args[ac], XmNdialogTitle, (s2=MKSTRING("Information"))); ac++;
    XtSetArg (args[ac], XmNokLabelString, (s3=MKSTRING("Acknowledged"))); ac++;
    D1 = XmCreateInformationDialog(ConnectBtn, "d1", args, ac);
    XtUnmanageChild (XmMessageBoxGetChild (D1, XmDIALOG_HELP_BUTTON) );
    XtUnmanageChild (XmMessageBoxGetChild (D1, XmDIALOG_CANCEL_BUTTON) );
    XmStringFree(s1);
    XmStringFree(s2);
    XmStringFree(s3);

    ac = 0;
    XtSetArg(args[ac], XmNmessageString,
    	  (s1=MKSTRING("Please enter a remote node,\nand then press Search")));
    	  ac++;
    XtSetArg (args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg (args[ac], XmNdialogTitle, (s2=MKSTRING("Warning"))); ac++;
    XtSetArg (args[ac], XmNokLabelString, (s3=MKSTRING("Acknowledged"))); ac++;
    D2 = XmCreateWarningDialog(SearchBtn, "d2", args, ac);
    XtUnmanageChild (XmMessageBoxGetChild (D2, XmDIALOG_HELP_BUTTON) );
    XtUnmanageChild (XmMessageBoxGetChild (D2, XmDIALOG_CANCEL_BUTTON) );
    XmStringFree(s1);
    XmStringFree(s2);
    XmStringFree(s3);

    ac = 0;
    XtSetArg(args[ac], XmNmessageString,
    	  (s1=MKSTRING("Remote node is not currently reachable"))); ac++;
    XtSetArg (args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg (args[ac], XmNdialogTitle, (s2=MKSTRING("Error"))); ac++;
    XtSetArg (args[ac], XmNokLabelString, (s3=MKSTRING("Acknowledged"))); ac++;
    D3 = XmCreateErrorDialog(SearchBtn, "d3", args, ac);
    XtUnmanageChild (XmMessageBoxGetChild (D3, XmDIALOG_HELP_BUTTON) );
    XtUnmanageChild (XmMessageBoxGetChild (D3, XmDIALOG_CANCEL_BUTTON) );
    XmStringFree(s1);
    XmStringFree(s2);
    XmStringFree(s3);

    ac = 0;
    XtSetArg(args[ac], XmNmessageString,
    	  (s1=MKSTRING("Error connecting to chosen process"))); ac++;
    XtSetArg (args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg (args[ac], XmNdialogTitle, (s2=MKSTRING("Error"))); ac++;
    XtSetArg (args[ac], XmNokLabelString, (s3=MKSTRING("Acknowledged"))); ac++;
    D4 = XmCreateErrorDialog(SearchBtn, "d4", args, ac);
    XtUnmanageChild (XmMessageBoxGetChild (D4, XmDIALOG_HELP_BUTTON) );
    XtUnmanageChild (XmMessageBoxGetChild (D4, XmDIALOG_CANCEL_BUTTON) );
    XmStringFree(s1);
    XmStringFree(s2);
    XmStringFree(s3);

}

/*
 *  listCB - Callback routine called when user double-clicks on a process
 *	     in the process list.  This selects the process for connection.
 */
static void listCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    XKeyPressedEvent con_but_event;
    
    memset(&con_but_event, 0, sizeof(XKeyPressedEvent));
    con_but_event.type = KeyPress;
    con_but_event.serial = 1;
    con_but_event.send_event = True;
    con_but_event.display = XtDisplay(ConnectBtn);
    con_but_event.window = XtWindow(ConnectBtn);
    
#ifdef COM_DEBUG
    printf("item selected with double-click\n");
#endif
    XtCallActionProc(ConnectBtn, "ArmAndActivate", (XEvent*) &con_but_event,
    		NULL, 0);
}

/*
 *  connectCB - Connect to process user selected.
 */
static void connectCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    int *position_list = NULL;
    int position_count = 0;
    int port_num;
    Boolean need_to_free = False;
    char *proc_string, *node_name;
    char pid[21];
    
    if (!(need_to_free = XmListGetSelectedPos(ProcessList, &position_list,
    			&position_count))) {
	if (XtIsManaged(Form))		/* ignore if called twice because */
    	    XtManageChild(D1);		/*	user typed Enter key	  */
    	return;
    }
#ifdef COM_DEBUG
    printf("position count = %d, need_to_free = %d, item # = %d, port # = %d, node = %s\n",
    		position_count, need_to_free, *position_list,
    		ListOfProcs[(*position_list)-1].socketNum,
    		ListOfProcs[(*position_list)-1].node);
#endif
    XmStringGetLtoR (ProcsListed[(*position_list)-1], XmSTRING_DEFAULT_CHARSET,
		&proc_string);

#ifdef COM_DEBUG
    printf("   item selected = %s\n", proc_string);
#endif
    /* call routine to make connection to selected process,
       copy process name selected into global save string variable,   */
    if (strcmp(proc_string,"") != 0) {
	DisplayWaitMsg(w, True);
	port_num = ListOfProcs[(*position_list)-1].socketNum;
	node_name = ListOfProcs[(*position_list)-1].node;
	if (ConnectToClient(node_name[0] == '\0' ? MyHostName
		: node_name, port_num) != COM_OK) {
	    XtManageChild(D4);		/* Error connecting to client */
	    UndisplayWaitMsg();
	    free(proc_string);
	    if (need_to_free)
    		free(position_list);
	    return;
	}
	ConnMade = True;
	if (ClientIsLocal)
	    strcpy (NameToReturn, "");
	else {
	    strcpy (NameToReturn, node_name[0] == '\0' ? HostName : node_name);
	    strcat (NameToReturn, ":");
	}
	strcat(NameToReturn, ListOfProcs[(*position_list)-1].idString);
	strcat(NameToReturn, " ");
	sprintf(pid, "%d", ListOfProcs[(*position_list)-1].pid);
	strcat(NameToReturn, pid);
	strcat(NameToReturn, " ");
	strcat(NameToReturn, ListOfProcs[(*position_list)-1].startTime);
    }
    free(proc_string);
    if (need_to_free)
    	free(position_list);
}

/*  searchCB - Search node user entered for processes available for 
 *  Histoscope connection.  Display processes in process selection list.
 */
static void searchCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    char *node_name;
    struct hostent *hp;			/* for gethostbyname */
    
    node_name = XmTextGetString(NodeNameText);
    if (strlen(node_name) == 0) {
    	XtFree(node_name);
#ifndef VMS
	XmTextSetString(UserNameText, "");
#endif /*VMS*/
	if (ProcsListed != NULL)
    	    freeStringTable(ProcsListed);
    	ProcsListed = findProcs("");		/* Fill list for local node */
    	XmProcessTraversal (NodeNameText, XmTRAVERSE_CURRENT);
    				/* position keyboard focus */
    	return;
    }
    SET_ONE_RSRC(Form,  XmNdefaultButton, ConnectBtn);
    XmProcessTraversal (ConnectBtn, XmTRAVERSE_CURRENT);
    				/* position keyboard focus */
    if ((hp = gethostbyname(node_name)) == NULL) {
    	XtManageChild(D3);	/* Remote node is not currently reachable */
        XtFree(node_name);
    	XmProcessTraversal(NodeNameText, XmTRAVERSE_CURRENT);
    				/* position keyboard focus */
        return;
    }
    if (strcmp(hp->h_name, MyHostName) == 0) {	/* Did user enter local host? */
    	memset(HostName, 0, MAXHOSTNAMELEN+1);	/*	yes		      */
    	ClientIsLocal = True;
    }
    else {					/*	no		      */
    	strcpy(HostName, node_name);
    	ClientIsLocal = False;
    }
    XtFree(node_name);
    if (ProcsListed != NULL)
    	freeStringTable(ProcsListed);
    ProcsListed = findProcs(HostName);		/* Fill list for remote node */
    if (ProcsListed == NULL) {
	SET_ONE_RSRC(Form,  XmNdefaultButton, SearchBtn);
	XmProcessTraversal (NodeNameText, XmTRAVERSE_CURRENT);
    				/* position keyboard focus */
    }
}

static void cancelCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    SET_ONE_RSRC(Form,  XmNdefaultButton, ConnectBtn);
    XmProcessTraversal (ConnectBtn, XmTRAVERSE_CURRENT);
    					   /* position keyboard focus */
    XtUnmanageChild(Form);
    reInitDialog();			/* Set dialog to creation state   */
}

static void helpCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    CreateHelpDialog(HelpBtn, "Connecting to a Process", HelpText);
}

static void txtFocusCB(Widget w, caddr_t clientData, caddr_t callData)
{
    SET_ONE_RSRC(Form,  XmNdefaultButton, SearchBtn);
#ifndef VMS
	XtSetSensitive(UserNameText, True);
	XtSetSensitive(UsrNameLabel, True);
#endif /*VMS*/
}

static void txtLostFocusCB(Widget w, caddr_t clientData, caddr_t callData)
{
    char *nodeName;

    nodeName = XmTextGetString(NodeNameText);
    if (strlen(nodeName) == 0) {
	SET_ONE_RSRC(Form,  XmNdefaultButton, ConnectBtn);
#ifndef VMS
	XtSetSensitive(UserNameText, False);
	XtSetSensitive(UsrNameLabel, False);
#endif /*VMS*/
    }
    XtFree(nodeName);
}

static void reInitDialog(void)
{
    XmTextSetString(NodeNameText, "");
    XmTextSetString(UserNameText, "");
#ifdef VMS
    XmTextSetString(UserPasswordText, "");
#endif /*VMS*/
    Passwd[0] = '\0';
    XmListDeselectAllItems(ProcessList);
    /* Set the initial focus of the dialog back to the connect button	*/
    SET_ONE_RSRC(Form, XmNdefaultButton, ConnectBtn)
}

static XmString *findProcs(char * node)
{    
    XmString *st1;
    Arg args[6];
    char errorMessage[MAX_LIST_ERROR_LEN]; 
    char *username;
    Boolean waitdisplayed = False;

    username = XmTextGetString(UserNameText);

    DisplayWaitMsg(XtIsManaged(SearchBtn) ? SearchBtn : MainPanelW, False);
    waitdisplayed = True;

    if (ListOfProcs != NULL) {
	free(ListOfProcs);
	ListOfProcs = NULL;
    }

    NumOfProcs = ListIDFiles(node, &ListOfProcs, errorMessage, username,
		(char *)Passwd);
    XtFree(username);
    if (waitdisplayed)
    	UndisplayWaitMsg();
    if (NumOfProcs == -1) {
    	DialogF(DF_ERR, SearchBtn, 1, "Error listing files:\n%s",
    		"Acknowledged", errorMessage);
    	XmListDeleteAllItems(ProcessList);
    	return (NULL);
    }
#ifdef COM_DEBUG
    printf("%d histoscope id files found in /tmp directory\n", NumOfProcs);
#endif
    XtSetArg(args[0], XmNitems, (st1=stringTable(NumOfProcs, ListOfProcs)));
    XtSetArg(args[1], XmNitemCount, NumOfProcs);
    XtSetValues(ProcessList, args, 2);
    XmListSelectPos(ProcessList, 1, False);   /* select 1st item (default) */
    return (st1);
}

static XmString* stringTable(int count, idFileData *proclist)
{
    XmString		*array;
    int			i;
    char		procString[/* pid */ 20 + MAX_USER_NAME_LEN +
				HS_MAX_IDENT_LENGTH + TIME_FIELD_LEN +
				MAX_NODE_LEN + 5]; /* (), 2 spaces, NUL at end*/
    char		pid[21];

    array = (XmString*)XtMalloc((count+1) * sizeof(XmString));
    for(i = 0;  i < count; i++ ) {
	strcpy(procString, proclist[i].idString);
	strcat(procString, " (");
	strcat(procString, proclist[i].userName);
	strcat(procString, ") ");
	sprintf(pid, "%d", proclist[i].pid);
	strcat(procString, pid);
	strcat(procString, " ");
	strcat(procString, proclist[i].startTime);
	strcat(procString, " <");
	strcat(procString, proclist[i].node);
	strcat(procString, ">");
	array[i] = XmStringCreateLtoR(procString, XmSTRING_DEFAULT_CHARSET);
    }
    array[i] = (XmString)0;
    return(array);
}

static void freeStringTable(XmString *table)
{
    int i;

    for( i = 0; table[i] != 0 ; i++) {
	XmStringFree(table[i]);
    }
    XtFree((char *) table);
}
