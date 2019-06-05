/*******************************************************************************
*									       *
* mainPanel.c -- Main panel for Nirvana Histoscope tool			       *
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
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* April 20, 1992							       *
*									       *
* Written by Mark Edel							       *
*									       *
* Modifications:							       *
*	JMK - 6/28/93 - Fixed memory leaks found by Purify		       *
*	JMK - 10/19/93 - Added disabling controls when client is waiting       *
*			 for Histo-Scope to finish.			       *
*									       *
*******************************************************************************/
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/FileSB.h>
#include <Xm/TextF.h>
#include <Xm/DialogS.h>
#include "../util/DialogF.h"
#include "../util/getfiles.h"
#include "../util/misc.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "../histo_util/getHbookData.h"
#include "../histo_util/readHbookList.h"
#include "histoP.h"
#include "plotWindows.h"
#include "configFile.h"
#include "mainPanel.h"
#include "mainMenu.h"
#include "multPlot.h"
#include "commPanel.h"
#include "ntuplePanel.h"
#include "communications.h"
#include "controls.h"
#include "interpret.h"
#include "variablePanel.h"
#include "gsComm.h"
#include "globalgc.h"

#define INITIAL_LIST_HEIGHT 17	/* number of rows in the category list */
#define OPT_MENU_PAD 4		/* added to align other widgets with option
				   menus to compensate for space on left */

enum panelState {NO_FILE_OPEN, HBOOK_FILE_OPEN, HISTO_FILE_OPEN, CONNECTED};

/* Global Variables */
Widget MainPanelW = 0;
GC globalGC = 0;

/* Module Global Variables */
static Widget MainContentsW = 0, CategoryListW = 0, HistogramListW = 0;
static Widget DataTypeBtnW = 0, ViewBtnW = 0, HbookBtnW = 0, HsidBtnW = 0;
static Widget CategoryMenuW = 0, CategoryOptMenuW = 0, OpenBtnW = 0;
static Widget ViewOvrlBtnW = 0, ViewMultBtnW = 0;
static char HbookFilename[MAXPATHLEN];
static int HbookBlockSize = DEFAULT_HBOOK_BLOCK_SIZE;
static int HbookInitialized = False;
static char HistoFilename[MAXPATHLEN];
static int PanelState = NO_FILE_OPEN;
static int ClientWaiting = 0;
static histoListHeader *HistoList = NULL;
static histoListHeader *ClosedHistoList = NULL;
static int ClosedIDCount = INT_MAX;
static int *ListedIDs;
static char CurrentCategory[HS_MAX_CATEGORY_LENGTH] = "   Uncategorized   ";
static char TopCategory[HS_MAX_CATEGORY_LENGTH] = "   Uncategorized   ";

/* Function Prototypes */
static void createContents(Widget parent);
static void closeCB(Widget w, caddr_t clientData, caddr_t callData);
static void categoryListCB(Widget w, caddr_t clientData, caddr_t callData); 
static void categoryMenuCB(Widget w, int catNum, caddr_t callData); 
static void histoListCB(Widget w, caddr_t clientData, caddr_t callData);
static void chkHistoListCB(Widget w, caddr_t clientData, caddr_t callData); 
static void hbookBtnCB(Widget w, caddr_t clientData, caddr_t callData);
static void hsidBtnCB(Widget w, caddr_t clientData, caddr_t callData);
static void dataTypeBtnCB(Widget w, caddr_t clientData, caddr_t callData);
static void openCB(Widget w, caddr_t clientData, caddr_t callData);
static void viewCB(Widget w, caddr_t clientData, caddr_t callData);
static void viewOvrlCB(Widget w, caddr_t clientData, caddr_t callData);
static void viewMultCB(Widget w, caddr_t clientData, caddr_t callData);
static void redisplayItems(void);
static void redisplaySubcategories(void);
static void redisplayCategoryMenu(void);
static void setMainPanelState(int state, char * title);
static char *nextCategory(char *toString, char *fromString);
static int subCategory(char *category, char *categories, char *subCategory);
static int itemInCategory(hsGeneral *item, char *category);
static void setListItems(Widget w, XmString *strings, int nStrings);
static int getHbookFilename(char *filename, int *blockSize);
static void resetTopCategory(void);
static multWindow *putUpMultiPlot(Widget parent, char *title, int numItems, 
	int *ids, int *errsDisp);

/*
** Create the Histoscope main window
*/
Widget CreateMainWindow(Display *display, int captiveMode)
{
    Widget appShell, menuBar;
    Arg al[30];
    int ac;

    /* Create an toplevel shell to hold the window */
    ac = 0;
    XtSetArg(al[ac], XmNtitle, "Histoscope"); ac++;
    XtSetArg(al[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(al[ac], XmNiconName, "Histoscope"); ac++;
    appShell = XtAppCreateShell("histo", "Histoscope",
		applicationShellWidgetClass, display, al, ac);

    /* Create a main window holding a menu bar and a form with the rest of
       the window contents. */
    MainPanelW = XmCreateMainWindow(appShell, "main", NULL, 0);
    XtManageChild(MainPanelW);
    menuBar = CreateMainMenuBar(MainPanelW, captiveMode);
    XtManageChild(menuBar);
    createContents(MainPanelW);
    setMainPanelState(NO_FILE_OPEN, "HistoScope");

    /* realize all of the widgets in the new window */
    XtRealizeWidget(appShell);

    /* Create a simple global GC */
    globalGC = XCreateGC(XtDisplay(MainPanelW),
			 RootWindowOfScreen(XtScreen(MainPanelW)), 0, NULL);

    /* set up closeCB to be called when the user selects close from the
       window frame menu */
    AddMotifCloseCallback(appShell,(XtCallbackProc) closeCB, NULL);

    /* Add a special handler for messages from ghostscript */
    addToCommWidgets(MainPanelW);
    addToCommWidgets(MainContentsW);
    addToCommWidgets(CategoryListW);
    addToCommWidgets(HistogramListW);
    addToCommWidgets(CategoryMenuW);
    addToCommWidgets(OpenBtnW);

    return MainPanelW;
}

/*
** AddItemToMainPanel, DeleteItemFromMainPanel
**
** Add or remove a histogram, nTuple, or indicator to(from) the main panel list.
** These do not redisplay the list in the panel, call RedisplayHistogramList
** after completing changes to the internal list to change the displayed list.
*/
void AddItemToMainPanel(hsGeneral *item)
{
    AddItemToList(item, &HistoList);
}
void DeleteItemFromMainPanel(int id)
{
    DeleteItemFromList(id, &HistoList);
    InvalidateItem(id);
    InvalidateNTuple(id);
    CloseVariablePanelsWithItem(id);
    RemoveNTupleExtension(id);
}

/*
** Redisplay the histogram list after changes have been made to
** CurrentCategory or HistoList
*/
void RedisplayHistogramList(int resetCurrentCategory)
{
    resetTopCategory();
    if (resetCurrentCategory)
    	strcpy(CurrentCategory, TopCategory);
    redisplayItems();
    redisplaySubcategories();
    redisplayCategoryMenu();
}

/*
** Return an item displayed, or previously displayed
** on the Main Panel given its ID
*/
hsGeneral *GetMPItemByID(int id)
{
    hsGeneral *tmp;
    
    tmp = GetItemByID(id, HistoList);
    if (tmp == NULL)
    	tmp = GetItemByID(id, ClosedHistoList);
    if (tmp == NULL)
    	fprintf(stderr,
    	   "Internal Error: GetMPItemByID could not find item, id = %d\n", id);
    return (tmp);
}

/*
** Return an item displayed, or previously displayed on the Main Panel 
** given its ID - BUT don't print an error if it's not found, just return NULL.
*/
hsGeneral *GetMPItemByIDnp(int id)
{
    hsGeneral *tmp;
    
    tmp = GetItemByID(id, HistoList);
    if (tmp == NULL)
    	tmp = GetItemByID(id, ClosedHistoList);
    return (tmp);
}

/*
** Find a data item from the current file, by category and UID.
** If not found, return NULL instead of pointer to item.
*/
hsGeneral *GetMPItemByUID(char *category, int uid)
{
    histoListHeader *h;
    
    for (h = HistoList; h != NULL; h = h->next) {
	if (h->item->uid == uid  && !strcasecmp(category, h->item->category))
	    return h->item;
    }
    return NULL; /* Item not found */
}

/*
** Find a data item from the current file, by category and item title.
** If not found, return NULL instead of pointer to item.
*/
hsGeneral *GetMPItemByName(char *category, char *name)
{
    histoListHeader *h;
    
    for (h = HistoList; h != NULL; h = h->next) {
	if (!strcmp(name, h->item->title) &&
		!strcasecmp(category, h->item->category))
	    return h->item;
    }
    return NULL; /* Item not found */
}

/*
** Find an HBOOK data item from the current file, by category and hbookID.
** If not found with a complete category, ignore the top category for matching.
** If not found, return NULL instead of pointer to item.
*/
hsGeneral *GetMPItemByHbookID(char *category, int hbookID)
{
    histoListHeader *h;
    
    for (h = HistoList; h != NULL; h = h->next) {
	if (!strcmp(category, h->item->category) && h->item->hbookID == hbookID)
	    return h->item;
    }
    if (PanelState == HBOOK_FILE_OPEN) {
	char *subCat = strchr(category, '/');
	char *subCitem;
	if (subCat != NULL)
	    subCat++;
	for (h = HistoList; h != NULL; h = h->next) {
	    subCitem = strchr(h->item->category, '/');
	    if (subCitem != NULL)
	    	subCitem++;
	    if (h->item->hbookID == hbookID &&  
	    		(subCitem == subCat || !strcmp(subCat, subCitem)))
		return h->item;
	}
    }
    return NULL; /* Item not found */
}

/*
** OpenNewFile
**
** Prompts the user for a filename, and opens an
** HBOOK_FORMAT or HISTO_FORMAT file
*/
void OpenNewFile(int format)
{
    int resp;
   
    if (PanelState == HBOOK_FILE_OPEN || PanelState == HISTO_FILE_OPEN) {
    	resp = DialogF(DF_INF, MainPanelW, 2,
    		"Opening a new file will close\nthe currently open file",
    			"Close", "Cancel");
    	if (resp == 2)
    	    return;
    	CloseMainPanel();
    } else if (PanelState == CONNECTED) {
    	resp = DialogF(DF_INF, MainPanelW, 2,
    		"Opening a file will disconnect\nthe attached process",
    			"Disconnect", "Cancel");
    	if (resp == 2)
    	    return;
    	DisconnectProcess();
    }
    if (format == HBOOK_FORMAT) {
	if (getHbookFilename(HbookFilename, &HbookBlockSize)) {
	    if (!OpenInitialHbookFile(HbookFilename, HbookBlockSize))
		DialogF(DF_ERR, MainPanelW, 1,
	    	    	"Error opening HBOOK file", "Acknowledged");
    	}
    } else {
    	while (True) {
    	    resp = GetExistingFilename(MainPanelW, "HistoScope File to open:",
    	    	    HistoFilename);
    	    if (resp == GFN_OK) {
    		if (OpenInitialHistoFile(HistoFilename))
    		    break;
    	    } else /* resp == GFN_CANCEL */ { 
    	    	break;
    	    }
    	}
    }
}

/*
** OpenInitialHistoFile
**
** Opens the named histoscope format file in an EMPTY main panel.  For
** file names specified on the command line.
*/
int OpenInitialHistoFile(char *filename)
{
    if (ReadHistoFile(filename) == 0)
    	return False;
    
    setMainPanelState(HISTO_FILE_OPEN, filename);
    RedisplayHistogramList(True);
    strcpy(HistoFilename, filename);
    return True;
}

/*
** OpenInitialHbookFile
**
** Opens the named hbook format file in an EMPTY main panel.  For
** file names specified on the command line.
*/
int OpenInitialHbookFile(char *filename, int blockSize)
{
    int numRead, i;
    hsGeneral **itemTable;

    if (!HbookInitialized) {
    	HbookInitialized = True;
    }
    
    if (ReadHbookList(filename, blockSize, &numRead, &itemTable) == 0)
	return False;

    for (i = 0; i < numRead; ++i)
    	AddItemToMainPanel(itemTable[i]);
    free(itemTable);
    setMainPanelState(HBOOK_FILE_OPEN, filename);
    RedisplayHistogramList(True);
    strcpy(HbookFilename, filename);
    HbookBlockSize = blockSize;
    return True;
}

/*
** RereadFile
**
** Close and re-open the file that is currently open
*/
void RereadFile()
{
    int resp, oldPanelState;
    int numRead, i;
    hsGeneral **itemTable;
    
    if (!(PanelState == HBOOK_FILE_OPEN || PanelState == HISTO_FILE_OPEN))
    	return;
    
    /* Warn the user that operation is irreversible */
    resp = DialogF(DF_WARN, MainPanelW, 2,
	    "Re-reading the current file will\nclose all of the plot windows",
    	    "Re-read", "Cancel");
    if (resp == 2)
    	return;
    	
    /* Close all windows and close the current file */
    oldPanelState = PanelState;
    
    /* Mini-plots in Multi-Plot windows MUST be closed first !!!
       This is because each mini-plot is also on the WindowInfo list,
       and we don't want to delete/destroy them twice!  CloseAllMultWindows
       closes all multiple-plot windows and removes each mini-plot from
       the WindowInfo list */
    CloseAllMultWindows();
    CloseAllPlotWindows();
    CloseAllNTupleWindows();
    CloseMainPanel();
    
    /* re-open the current file and warn the user if it doesn't work */
    if (oldPanelState == HBOOK_FILE_OPEN) {
	if (ReadHbookList(HbookFilename, HbookBlockSize, &numRead, 
	    	    &itemTable) != 0) {
    	    for (i = 0; i < numRead; ++i)
    		    AddItemToMainPanel(itemTable[i]);
    	    free(itemTable);
    	    setMainPanelState(HBOOK_FILE_OPEN, HbookFilename);
	} else {
	    DialogF(DF_ERR, MainPanelW, 1, "Error re-opening %s",
	    	    "Acknowledged", HbookFilename);
	    return;
	}
    } else { /* oldPanelState == HISTO_FILE_OPEN) */
    	if (ReadHistoFile(HistoFilename) != 0)
    	    setMainPanelState(HISTO_FILE_OPEN, HistoFilename);
    	else
    	    return;
    }
    RedisplayHistogramList(True);
}

/*
** OpenNewConnection
**
** Asks user about closing open file or connection and initiates connection
*/
void OpenNewConnection(void)
{
    int resp;
   
    if (PanelState == HBOOK_FILE_OPEN || PanelState == HISTO_FILE_OPEN) {
    	resp = DialogF(DF_INF, MainPanelW, 2,
    		"Connecting to a process will close\nthe currently open file",
    			"Close", "Cancel");
    	if (resp == 2)
    	    return;
    	CloseMainPanel();
    } else if (PanelState == CONNECTED) {
    	resp = DialogF(DF_INF, MainPanelW, 2,
    	  "Connecting to a new process will\ndisconnect the attached process",
    			"Disconnect", "Cancel");
    	if (resp == 2)
    	    return;
    	DisconnectProcess();
    }
    ConnectToProcess(MainPanelW);
}

/*
** Routine to be called by communication software when the connection is
** established (ConnectToProcess only begins the chain of events which
** set up the connection, the connection is completed later by communication
** routines dispatched from the main loop.  When the link is in place,
** this routine is called to get the panel ready to display its list.)
*/
void SetMainPanelConnected(char *title)
{
    /* set the main panel sensitive and ready to accept items */
    setMainPanelState(CONNECTED, title);
    
    /* wipe out the "no file or process connected" message */
    setListItems(HistogramListW, NULL, 0);
}

/*
** Close the currently open file or connection without asking and without
** closing any open windows
*/
void CloseFileOrConnection(void)
{
    if (PanelState == HBOOK_FILE_OPEN || PanelState == HISTO_FILE_OPEN) {
    	CloseMainPanel();
    } else if (PanelState == CONNECTED) {
    	DisconnectProcess();
    }
}

void CloseMainPanel(void)
{
    histoListHeader *h, *temp;

    CurrentCategory[0] = '\0';
    TopCategory[0] = '\0';
    
    /* "garbage collect" the ClosedHistoList, removing items which are no
       longer displayed */
    h = ClosedHistoList;
    ClosedHistoList = NULL;
    while (h != NULL) {
    	if (ItemIsDisplayed(h->item->id) || NtupleIsDisplayed(h->item->id)) {
    	    temp = ClosedHistoList;
    	    ClosedHistoList = h;
    	    h = h->next;
    	    ClosedHistoList->next = temp;
    	} else {
    	    DeleteItemFromList(h->item->id, &h);
    	}
    }
    	    
    /* move all displayed and non-empty items to the ClosedHistoList, delete
       all empty or undisplayed items, and renumber the items moved to the
       ClosedHistoList backwards from INT_MAX */
    while (HistoList != NULL) {
/*      Commented out by igv -- this may cause a problem when */
/*      a displayed item was about to request the data        */
/*      	if (ItemHasData(HistoList->item) && */
    	if ((ItemIsDisplayed(HistoList->item->id) ||
    		     NtupleIsDisplayed(HistoList->item->id))) {
    	    if (HistoList->item->type == HS_CONTROL ||
    	    	    HistoList->item->type == HS_TRIGGER)
    	    	SetControlInsensitive(HistoList->item);
    	    temp = ClosedHistoList;
    	    ClosedHistoList = HistoList;
    	    HistoList = ClosedHistoList->next;
    	    ClosedHistoList->next = temp;
    	    ChangeWindowItemID(ClosedHistoList->item->id, ClosedIDCount);
    	    ChangeNtuplePanelItemID(ClosedHistoList->item->id, ClosedIDCount);
	    ChangeNTupleExtensionItemID(ClosedHistoList->item->id,
	    	    ClosedIDCount);
	    ChangeVariablePanelItemIDs(ClosedHistoList->item->id,
	    	    ClosedIDCount);
    	    ClosedHistoList->item->id = ClosedIDCount--;
    	} else {
    	    DeleteItemFromMainPanel(HistoList->item->id);
    	}
    }
    
    setMainPanelState(NO_FILE_OPEN, "HistoScope");
    RedisplayHistogramList(True);
}

/*
** Set all controls in the Main Panel insensitive
*/
void SetAllControlsInSensitive(void)
{
    histoListHeader *h;
    
    h = HistoList;
    while (h != NULL) {
    	if (h->item->type == HS_CONTROL || h->item->type == HS_TRIGGER)
    	    SetControlInsensitive(h->item);
    	h = h->next;
    }
    ClientWaiting = 1;
}

/*
** Supplies the current category string and a list of histogram IDs for the
** category.  Returns the number of IDs in histIDs array.  Implemented to
** support multiple panel plots of a category.  *histIds should be freed by
** the calling routine.
*/
int GetCurrentCategoryHists(char *currentCategory, int **histIDs)
{
    histoListHeader *h;
    int numIds = 0, numItems = 0;
    
    strcpy(currentCategory, CurrentCategory);
    h = HistoList;
    
    /* First count number of histograms in category */
    while (h != NULL) {
    	if ((h->item->type == HS_1D_HISTOGRAM
	     || h->item->type == HS_2D_HISTOGRAM
	     || h->item->type == HS_3D_HISTOGRAM)
    	        && itemInCategory(h->item, CurrentCategory))
    	    numItems++;
    	h = h->next;
    }
    
    /* Then save their IDs in a malloc'd array */
    *histIDs = (int *)XtMalloc(sizeof(int) * numItems);
    h = HistoList;
    while (h != NULL) {
    	if ((h->item->type == HS_1D_HISTOGRAM
    	     || h->item->type == HS_2D_HISTOGRAM
    	     || h->item->type == HS_3D_HISTOGRAM)
	    && itemInCategory(h->item, CurrentCategory))
    	    (*histIDs)[numIds++] = h->item->id;
    	h = h->next;
    }
    if (numItems != numIds)
        fprintf(stderr, "Error in GetCurrentCategoryHists: "
		"numIds = %d, numItems = %d\n", numIds, numItems);
    return numIds;
}

/*
** Create the contents area of the Histoscope main window
*/
static void createContents(Widget parent)
{
    Arg args[50];
    int ac;
    XmString s1, *st1;
    Widget categoryForm, categoryLabel, subCatLabel;
    Widget histogramLabel, fakeBtn;

    /* Create the form onto which everything goes */
    ac = 0;
    MainContentsW = XmCreateForm(parent, "form", args, ac);

    /* Create a form to hold the first column of information (category list) */
    ac = 0;
    XtSetArg(args[ac], XmNmarginHeight, 0); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftOffset, 6); ac++;
    XtSetArg(args[ac], XmNtopOffset, 10); ac++;
    categoryForm = XmCreateForm(MainContentsW, "categoryForm", args, ac);
    XtManageChild(categoryForm);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Current Category"))); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftOffset, OPT_MENU_PAD); ac++;
    categoryLabel = XmCreateLabelGadget(categoryForm, "categoryLabel", args,ac);
    XmStringFree(s1);
    XtManageChild(categoryLabel);
 
    ac = 0;
    CategoryMenuW = XmCreatePulldownMenu(categoryForm, "category", args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNspacing, 0); ac++;
    XtSetArg(args[ac], XmNmarginWidth, 0); ac++;
    XtSetArg(args[ac], XmNresizeWidth, False); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNtopWidget, categoryLabel); ac++;
    XtSetArg(args[ac], XmNsubMenuId, CategoryMenuW); ac++;
    CategoryOptMenuW = XmCreateOptionMenu(categoryForm, "categoryMenu",
    					  args, ac);
    XtManageChild(CategoryOptMenuW);
 
    /* Option menus have several serious problems, particularly in the version
       of Motif on SGI. Two separate methods are used to keep the option
       menu at a constant width.  For the current SGI version, we create a
       fake, zero height, menu item to keep the option menu widget from
       narrowing when the menu items are changed.  The string in the category
       option menu (and of course the font) determines the width for both the
       category menu and for the sub-category list. */
    fakeBtn = XtVaCreateManagedWidget("fakeCategory", xmPushButtonWidgetClass,
    	    CategoryMenuW,
    	    XmNlabelString, s1=XmStringCreateSimple(CurrentCategory),
    	    XmNshadowThickness, 0,
    	    XmNmarginHeight, 0,
    	    XmNsensitive, False,
    	    XmNrecomputeSize, False,
    	    XmNheight, 0 , NULL);
    XmStringFree(s1);
    XtVaSetValues(fakeBtn, XmNlabelString, s1=XmStringCreateSimple(""),
    	    XmNheight, 0, NULL);
    /* On systems other than the SGI, just displaying the "   Uncategorized   "
       initial category item and setting XmNrecomputeSize to false is enough */
    redisplayCategoryMenu();
    XmStringFree(s1);				/* JMK - 6/28/93	*/
    XtVaSetValues(XmOptionButtonGadget(CategoryOptMenuW),
    	    XmNrecomputeSize, False, NULL);
    
    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Sub Categories"))); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNtopWidget, CategoryOptMenuW); ac++;
    XtSetArg(args[ac], XmNleftOffset, OPT_MENU_PAD); ac++;
    subCatLabel = XmCreateLabelGadget(categoryForm, "subCatLabel", args,ac);
    XmStringFree(s1);
    XtManageChild(subCatLabel);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("Open"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 4); ac++;
    XtSetArg(args[ac], XmNleftOffset, 6 + OPT_MENU_PAD); ac++;
    OpenBtnW = XmCreatePushButton(categoryForm, "openBtn", args,ac);
    XtAddCallback(OpenBtnW, XmNactivateCallback,
                  (XtCallbackProc)  openCB, NULL);
    XmStringFree(s1);
    XtManageChild(OpenBtnW);
 
    ac = 0;
    XtSetArg(args[ac], XmNitems, (st1=StringTable(" "))); ac++;
    XtSetArg(args[ac], XmNitemCount, 1); ac++;
    XtSetArg(args[ac], XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
    XtSetArg(args[ac], XmNvisibleItemCount, INITIAL_LIST_HEIGHT); ac++;
    XtSetArg(args[ac], XmNselectionPolicy, XmBROWSE_SELECT); ac++;
    XtSetArg(args[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNtopWidget, subCatLabel); ac++;
    XtSetArg(args[ac], XmNbottomWidget, OpenBtnW); ac++;
    XtSetArg(args[ac], XmNtopOffset, 0); ac++;
    XtSetArg(args[ac], XmNleftOffset, 3 + OPT_MENU_PAD); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 4); ac++;
    CategoryListW = XmCreateScrolledList(categoryForm, "categoryList", args,ac);
    FreeStringTable(st1);
    XtAddCallback(CategoryListW, XmNdefaultActionCallback,
                  (XtCallbackProc)  categoryListCB,NULL);
    XtManageChild(CategoryListW);
 
    ac = 0;
    XtSetArg(args[ac], XmNlabelString,
    	     (s1=MKSTRING("Histogram/Ntuple/Indicator/Control"))); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, categoryForm); ac++;
    XtSetArg(args[ac], XmNtopOffset, 10); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    histogramLabel = XmCreateLabelGadget(MainContentsW, "histogramLabel",
    					 args, ac);
    XmStringFree(s1);
    XtManageChild(histogramLabel);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("View"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, categoryForm); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 4); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 6); ac++;
    ViewBtnW = XmCreatePushButton(MainContentsW, "viewBtn", args,ac);
    XtAddCallback(ViewBtnW, XmNactivateCallback,
                  (XtCallbackProc)  viewCB, NULL);
    XmStringFree(s1);
    XtManageChild(ViewBtnW);
 
    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("View Overlaid"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, ViewBtnW); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 4); ac++;
    XtSetArg(args[ac], XmNleftOffset, 6); ac++;
    XtSetArg(args[ac], XmNrightOffset, 6); ac++;
    ViewOvrlBtnW = XmCreatePushButton(MainContentsW, "viewOvrlBtn", args,ac);
    XtAddCallback(ViewOvrlBtnW, XmNactivateCallback,
                  (XtCallbackProc)  viewOvrlCB, NULL);
    XmStringFree(s1);
    XtManageChild(ViewOvrlBtnW);
 
    ac = 0;
    XtSetArg(args[ac], XmNlabelString, (s1=MKSTRING("View Multiple"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, ViewOvrlBtnW); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 4); ac++;
    XtSetArg(args[ac], XmNleftOffset, 6); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    ViewMultBtnW = XmCreatePushButton(MainContentsW, "viewMultBtn", args,ac);
    XtAddCallback(ViewMultBtnW, XmNactivateCallback,
                  (XtCallbackProc)  viewMultCB, NULL);
    XmStringFree(s1);
    XtManageChild(ViewMultBtnW);
 
    ac = 0;
    XtSetArg(args[ac], XmNlabelString,
    	     (s1=MKSTRING("Show Data Type"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, categoryForm); ac++;
    XtSetArg(args[ac], XmNbottomWidget, ViewBtnW); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 2); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    DataTypeBtnW = XmCreateToggleButtonGadget(MainContentsW, "dataTypeBtn", args, ac);
    XtAddCallback(DataTypeBtnW, XmNvalueChangedCallback,
                  (XtCallbackProc)dataTypeBtnCB, NULL);
    XmStringFree(s1);
    XtManageChild(DataTypeBtnW);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString,
    	     (s1=MKSTRING("Show Histo-Scope Ids"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, categoryForm); ac++;
    XtSetArg(args[ac], XmNbottomWidget, DataTypeBtnW); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    XtSetArg(args[ac], XmNtopOffset, 2); ac++;
    XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    HsidBtnW = XmCreateToggleButtonGadget(MainContentsW, "hsidBtn", args, ac);
    XtAddCallback(HsidBtnW, XmNvalueChangedCallback,
                  (XtCallbackProc)hsidBtnCB, NULL);
    XmStringFree(s1);
    XtManageChild(HsidBtnW);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString,
    	     (s1=MKSTRING("Show User Id or HBOOK ID Numbers"))); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftWidget, categoryForm); ac++;
    XtSetArg(args[ac], XmNbottomWidget, HsidBtnW); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    XtSetArg(args[ac], XmNtopOffset, 2); ac++;
    XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    HbookBtnW = XmCreateToggleButtonGadget(MainContentsW, "hbookBtn", args, ac);
    XtAddCallback(HbookBtnW, XmNvalueChangedCallback,
                  (XtCallbackProc)hbookBtnCB, NULL);
    XmStringFree(s1);
    XtManageChild(HbookBtnW);

    ac = 0;
    XtSetArg(args[ac], XmNitems,
    	     (st1=StringTable("(No file or process open)"))); ac++;
    XtSetArg(args[ac], XmNitemCount, 1); ac++;
    XtSetArg(args[ac], XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
    XtSetArg(args[ac], XmNvisibleItemCount, INITIAL_LIST_HEIGHT); ac++;
    XtSetArg(args[ac], XmNselectionPolicy, XmEXTENDED_SELECT); ac++;
    XtSetArg(args[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNleftWidget, categoryForm); ac++;
    XtSetArg(args[ac], XmNtopWidget, histogramLabel); ac++;
    XtSetArg(args[ac], XmNbottomWidget, HbookBtnW); ac++;
    XtSetArg(args[ac], XmNtopOffset, 0); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 4); ac++;
    XtSetArg(args[ac], XmNleftOffset, 10); ac++;
    XtSetArg(args[ac], XmNrightOffset, 10); ac++;
    HistogramListW = XmCreateScrolledList(MainContentsW, "histogramList", 
    					  args, ac);
    XtAddCallback(HistogramListW, XmNdefaultActionCallback,
                  (XtCallbackProc)  histoListCB, NULL);
    XtAddCallback(HistogramListW, XmNextendedSelectionCallback,
                  (XtCallbackProc)  chkHistoListCB, NULL);
    FreeStringTable(st1);
    XtManageChild(HistogramListW);
    
    /* The panel is set insensitive (dimmed) after being created, rather
       than being created that way, because option menus stick permanently
       insensitive if they are created with an insensitive parent! */
    XtSetSensitive(MainContentsW, False);

    XtManageChild(MainContentsW);
}

/*
** Callback procedures for the widgets on the Histoscope main panel
*/
static void closeCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    DisconnectProcess();
    exit(0);
}

static void histoListCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    SimulateButtonPress(ViewBtnW);
}

/*
 * chkHistoListCB - This callback routine is called when the user selects or
 *		    unselects an item (in extended select mode) in the histogram
 *		    list. The routine checks all selections, the data type of
 *		    the item selected, and will disable the View Multiple or
 *		    View Overlaid buttons if all selected items are not
 *		    histograms or enable the buttons if all are histograms.
 *		    2D Histograms canNOT be overlaid.
 */
static void chkHistoListCB(Widget w, caddr_t clientData, caddr_t callD) 
{
    XmListCallbackStruct *callData = (XmListCallbackStruct *)callD;
    int *posList = NULL;
    int i, posCount = 0, num1DHists = 0, num2DHists = 0;
    int num3DHists = 0, numGrps = 0;
    hsGeneral *item;
    
/*
 *    printf("in chkHistoListCB: reason = %d, item_position = %d\n\
 *	selected_item_count = %d, ", callData->reason, callData->item_position, 
 *   	callData->selected_item_count);
 *   if (callData->selected_item_count > 0) {
 *   	printf("selected_item_positions = ");
 *   	for (i = 0; i < callData->selected_item_count; ++i) {
 *   	    printf("%d   ", callData->selected_item_positions[i]);
 *   	}
 *   }
 *   printf("\n");
 */

    /* Easy case:  if no items selected all buttons insensitive */
    if (callData->selected_item_count == 0) {
	XtSetSensitive(ViewBtnW, False);
	XtSetSensitive(ViewOvrlBtnW, False);
	XtSetSensitive(ViewMultBtnW, False);
	return;
    }

    /* get the items from the histogram list corresponding to the selected
       list position from the histogram list widget */
    if (!XmListGetSelectedPos(HistogramListW, &posList, &posCount)) {
	return;
    }
   for (i = 0; i < posCount; ++i) {
	item = GetItemByID(ListedIDs[posList[i] - 1], HistoList);
	if (item == NULL) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error: Could not find item",
		    "Acknowledged");
	    XtFree((char *)posList);
    	    return;
	}

	/* If an item other than a histogram or group is selected, grey
	   out View Overlaid and View Multiple buttons, but View button
	   should be sensitive */
	if (item->type == HS_GROUP)
	    ++numGrps;
	else if (item->type == HS_3D_HISTOGRAM)
	    ++num3DHists;
	else if (item->type == HS_2D_HISTOGRAM)
	    ++num2DHists;
	else if (item->type == HS_1D_HISTOGRAM)
	    ++num1DHists;
	else {
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, True);
    	    XtFree((char *)posList);
    	    return;
    	}
    }
    
    /* All histograms and/or groups */
    if (num2DHists + num1DHists > 1 || numGrps > 0)
	XtSetSensitive(ViewMultBtnW, True);
    else
	XtSetSensitive(ViewMultBtnW, False);
    if (num2DHists > 0 || (num1DHists < 2 && numGrps == 0))
	XtSetSensitive(ViewOvrlBtnW, False);
    else
    	XtSetSensitive(ViewOvrlBtnW, True);
    XtSetSensitive(ViewBtnW, True);
    XtFree((char *)posList);
}

static void categoryListCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    SimulateButtonPress(OpenBtnW);
}

static void categoryMenuCB(Widget w, int catNum, caddr_t callData) 
{
    char *cats, cat[HS_MAX_CATEGORY_LENGTH];
    int i;
    
    /* Move upwards in the category hierarchy by truncating the
       CurrentCategory string with a null after catNum categories */
    cats = CurrentCategory;
    if (catNum == 0) {
    	*CurrentCategory = '\0';
    } else {
	for (i=1; i<=catNum; i++) {
    	    cats = nextCategory(cat, cats);
    	    if (cats == NULL) {
    		/* safety valve in case CurrentCategory has changed between
    		   callback setup and invocation.  Shouldn't happen often */
    		return;
    	    }
	}
	*cats = '\0';
    }
    RedisplayHistogramList(False);
}

static void hbookBtnCB(Widget w, caddr_t clientData, caddr_t callData)
{
    if (XmToggleButtonGetState(w))
	XmToggleButtonSetState(HsidBtnW, False, False);
    redisplayItems();
}

static void hsidBtnCB(Widget w, caddr_t clientData, caddr_t callData)
{
    if (XmToggleButtonGetState(w))
	XmToggleButtonSetState(HbookBtnW, False, False);
    redisplayItems();
}

static void dataTypeBtnCB(Widget w, caddr_t clientData, caddr_t callData)
{
    redisplayItems();
}

static void openCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    int nSelectedItems;
    XmString *selectedItems;
    char *category, *lastSlash;
    
    /* Make sure a category is selected */
    GET_ONE_RSRC(CategoryListW, XmNselectedItemCount, &nSelectedItems)
    if (nSelectedItems == 0) {
	DialogF(DF_WARN, w, 1, "Please select a\ncategory to open",
		"Acknowledged");
	return;
    }
    
    /* Get the selected item (list is set to select only one item) */
    GET_ONE_RSRC(CategoryListW, XmNselectedItems, &selectedItems)
    XmStringGetLtoR(*selectedItems, XmSTRING_DEFAULT_CHARSET, &category);
    
    /* Change the current category to the category selected */
    if (!strncmp(category, "<-- ", 4)) {
    	/* move up in the category hierarchy */
    	lastSlash = strrchr(CurrentCategory, '/');
    	if (lastSlash == NULL)
    	    *CurrentCategory = '\0';
    	else
    	    *lastSlash = '\0';
    } else {
	/* move down the category hierarchy */
	if (strcmp(CurrentCategory, ""))
	    strcat(CurrentCategory, "/");
	strcat(CurrentCategory, category);
    }
    RedisplayHistogramList(False);
    XtFree(category);				/* JMK - 6/28/93 */
}

static void viewCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    int *posList = NULL;
    int i, posCount = 0;
    hsGeneral *item;
    
    /* get the item from the histogram list corresponding to the selected
       list position from the histogram list widget */
    if (!XmListGetSelectedPos(HistogramListW, &posList, &posCount)) {
	DialogF(DF_INF, w, 1,
	   "Please select a histogram,\nn-tuple, indicator, or control to view",
	    "Acknowledged");
    	return;
    }
    if (posCount > 25) {
    	if (DialogF(DF_QUES, w, 2,
     "You have selected %d items.\nAre you sure you want\nto display %d items?",
     	    "Yes", "No", posCount, posCount) == 2)
     	    return;
    }
    for (i = 0; i < posCount; ++i) {
	item = GetItemByID(ListedIDs[posList[i] - 1], HistoList);
	if (item == NULL) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error: could not find item",
		    "Acknowledged");
	    XtFree((char *)posList);
    	    return;
	}

	/* Put up the window to view the item */
	if (!LoadItemData(w, item))
    	    continue;
	ViewItem(XtDisplay(w), item, NULL, NULL, False, NULL, NULL);
    }
    XtFree((char *)posList);
    XmListDeselectAllItems(HistogramListW);
    XtSetSensitive(ViewBtnW, False);
    XtSetSensitive(ViewMultBtnW, False);
    XtSetSensitive(ViewOvrlBtnW, False);

}

static void viewOvrlCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    int *posList = NULL;
    int i, k, numOverlaid, posCount = 0, numItems = 0;
    hsGeneral *item = NULL;
    Boolean moreThanOnePlot = False;
    windowInfo *wInfo = NULL;
    
    /* Get the item(s) from the histogram list corresponding to the selected
       list position from the histogram list widget */
    if (!XmListGetSelectedPos(HistogramListW, &posList, &posCount)) {
	DialogF(DF_INF, w, 1,
	 "Please select two or more histograms\nto view in an overlay window",
	    "Acknowledged");
	XtFree((char *)posList);
    	return;
    }

    /* First count the number of 1-d histograms (including those in groups) */
    for (i = 0, numItems = 0; i < posCount; ++i) {
	item = GetItemByID(ListedIDs[posList[i] - 1], HistoList);
	/* printf("    getting item %d (%x)\n", ListedIDs[posList[i] - 1], item);*/
	if (item == NULL) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error: Could not find item.",
		    "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
	}
	if (item->type == HS_GROUP) {
	    hsGroup *group = (hsGroup *)item;
	    hsGeneral *grItem;
	    int j;
	    for (j = 0; j < group->numItems; ++j) {
	    	grItem = GetItemByID(group->itemId[j], HistoList);
		/* printf("    getting item %d (%x)\n", group->itemId[j], item); */
	    	if (grItem == NULL)
	    	    continue;		/* ignore deleted items in group list */
	    	if (grItem->type == HS_1D_HISTOGRAM)
	    	    ++numItems; 
	    }
	}
	else if (item->type == HS_1D_HISTOGRAM)
	    ++numItems;
    }
    /* printf("in viewOvrlCB:  numItems (1d-hists) = %d\n", numItems); */
    if (numItems > MAX_OVERLAID) {
    	if (DialogF(DF_INF, w, 2,
	    "You have selected %d 1-d histograms to overlay.\n\n\
There will be a maximum of %d histograms in an\noverlaid plot window.",
     	    "Continue", "Cancel", numItems, MAX_OVERLAID) == 2) {
	    XtFree((char *)posList);
     	    return;
     	} 
    }
    if (numItems <= 0) {
    	DialogF(DF_INF, w, 1,
	    "Only 1-d histograms in a group are displayed in an\n\
overlaid plot window.  No 1-d histograms in group.",
	    "Acknowledged");
    	XtFree((char *)posList);
    	return;
    }
    
    /* First, create one 1d-histogram window, and then add the rest of
     *     the histograms as overlays.
     * Up to MAX_OVERLAID plots are put into an overlay plot.
     * Anything other than 1-d histograms are put up individually. 
     *     Ntuples pop up the Ntuple Panel to specify variables plotted.
     */
    for (i = 0, k = 0, numOverlaid = 0; i < posCount && k < numItems; ++i) {
	item = GetItemByID(ListedIDs[posList[i] - 1], HistoList);
	/* printf("    getting item %d (%x)\n", ListedIDs[posList[i] - 1], item);*/
	if (item == NULL) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error: could not find item.",
		    "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
	}
	/* Individual items must be histograms */
	if (item->type != HS_1D_HISTOGRAM && item->type != HS_GROUP) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error (View Overlaid): Item not 1-d histogram",
		    "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
	}
	if (item->type == HS_GROUP) {
	    int j;
	    hsGroup *group = (hsGroup *) item;
	    hsGeneral *grItem;
	    for (j = 0; j < group->numItems; ++j) {
	    	grItem = GetItemByID(group->itemId[j], HistoList);
		/* printf(
		 "    getting group item %d (%x)\n", group->itemId[j], item); */
	    	if (grItem == NULL)
	    	    continue;		/* ignore deleted items in group list */
	        if (grItem->type != HS_1D_HISTOGRAM
	        	&& grItem->type != HS_GROUP) {
		    ViewItem(XtDisplay(w), grItem, NULL, NULL, False, NULL, NULL);
		    continue;
		}	/* Histograms in groups */
	    	if (wInfo == NULL) {
	    	    wInfo = ViewItem(XtDisplay(w), grItem, NULL, NULL, False, NULL, NULL);
	    	    ++numOverlaid;
	    	}
	    	else {
	    	    /* printf("calling OverlayPlot\n"); */
	    	    OverlayPlot(wInfo, grItem, HIST1D, NULL, NULL, 
            	    		((hs1DHist *) grItem)->nBins, 0, NO_ERROR_BARS,
            	    		NULL, 0, NULL, NULL, NULL);
	    	    if (++numOverlaid >= MAX_OVERLAID) {
	    	    	numOverlaid = 0;
	    	    	wInfo = NULL;
	    	    }
            	}
	    }
	} else {	/* Individual 1-d histogram */
	    if (wInfo == NULL) {
	    	if (!LoadItemData(MainPanelW, item))
    			continue;    /* indiv. histograms may be HBOOK items */
	    	wInfo = ViewItem(XtDisplay(w), item, NULL, NULL, False, NULL, NULL);
	    	++numOverlaid;
	    }
	    else {
		if (!LoadItemData(MainPanelW, item))
    			continue;    /* indiv. histograms may be HBOOK items */
		/* printf("calling OverlayPlot\n"); */
		OverlayPlot(wInfo, item, HIST1D, NULL, NULL, 
            	    	    ((hs1DHist *) item)->nBins, 0, NO_ERROR_BARS,
            	    	    NULL, 0, NULL, NULL, NULL);
	    	if (++numOverlaid >= MAX_OVERLAID) {
	    	    numOverlaid = 0;
	    	    wInfo = NULL;
	    	    moreThanOnePlot = True;
	    	}
            }
    	}
    }
    
    /* If only one group was selected to overlay, give window title of group */
    if (posCount == 1 && !moreThanOnePlot && wInfo != NULL) {
    	XtVaSetValues(wInfo->shell, XmNtitle, item->title, NULL);
    }
    
    XtFree((char *)posList);
    XmListDeselectAllItems(HistogramListW);
    XtSetSensitive(ViewOvrlBtnW, False);
    XtSetSensitive(ViewMultBtnW, False);
    XtSetSensitive(ViewBtnW, False);
    UpdateWindowsMenu();
}

/*
 * viewMultCB - Put up *ONE* Multi-Plot window of all selected items.
 *		When entering this routine, we know (and assume) that all
 *		non-group items are histograms.  However, we still have to
 *		check for groups that contain items that are not histograms.
 *		This routine tries to accomodate this by popping up these
 *		items as separate windows, for the convenience of the user.
 *		In this case, we pop up an informative dialog telling the user.
 */
static void viewMultCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    int *posList = NULL, doWarn = 0;
    int i, k, posCount = 0, numItems = 0;
    hsGeneral *item = NULL;
    int *itemIds, *errsDisp;
    char *winTitle; 
    
    /* Get the item(s) from the histogram list corresponding to the selected
       list position from the histogram list widget */
    if (!XmListGetSelectedPos(HistogramListW, &posList, &posCount)) {
	DialogF(DF_INF, w, 1,
	 "Please select two or more histograms\nto view in a multi-plot window",
	    "Acknowledged");
	XtFree((char *)posList);
    	return;
    }

    /* First count the number of histograms (including those in groups) */
    for (i = 0, numItems = 0; i < posCount; ++i) {
	item = GetItemByID(ListedIDs[posList[i] - 1], HistoList);
/*	printf("    getting item %d (%x)\n", ListedIDs[posList[i] - 1], item);*/
	if (item == NULL) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error: Could not find item.",
		    "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
	}
	if (item->type == HS_GROUP) {
	    hsGroup *group = (hsGroup *)item;
	    hsGeneral *grItem;
	    int j;
	    for (j = 0; j < group->numItems; ++j) {
	    	grItem = GetItemByID(group->itemId[j], HistoList);
/*	printf("    getting item %d (%x)\n", group->itemId[j], item); */
	    	if (grItem == NULL)
	    	    continue;		/* ignore deleted items in group list */
	    	if (grItem->type == HS_1D_HISTOGRAM || 
		    grItem->type == HS_2D_HISTOGRAM ||
		    grItem->type == HS_3D_HISTOGRAM)
	    	    ++numItems; 
	    }
	}
	else if (item->type == HS_1D_HISTOGRAM ||
		 item->type == HS_2D_HISTOGRAM ||
		 item->type == HS_3D_HISTOGRAM)
	    ++numItems;
    }
/*     printf("in viewMultCB:  numItems (histograms) = %d\n", numItems); */
    if (numItems > HS_MAX_NUM_GROUP_ITEMS) {
    	if (DialogF(DF_QUES, w, 2,
	    "You have selected %d items.\nAre you sure you want to\ndisplay %d \
items in your\nmulti-plot window?",
     	    "Yes", "No", numItems, numItems) == 2) {
	    XtFree((char *)posList);
     	    return;
     	}
    }
    if (numItems <= 0) {
    	DialogF(DF_INF, w, 1,
	    "Only histograms in a group are displayed\nNo histograms in group.",
		    "Acknowledged");
    	return;
    }
    
    /* Create a list of item ids and pop them up in a multiplot window.
     * Can only put histograms (1-d & 2-d) into a multiple plot window 
     * because Ntuples need the Ntuple Panel to specify variables plotted.
     */
    itemIds = (int *) XtMalloc (numItems * sizeof(int));
    errsDisp = (int *) XtMalloc (numItems * sizeof(int));
    for (i = 0, k = 0; i < posCount && k < numItems; ++i) {
	item = GetItemByID(ListedIDs[posList[i] - 1], HistoList);
/*	printf("    getting item %d (%x)\n", ListedIDs[posList[i] - 1], item);*/
	if (item == NULL) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error: could not find item.",
		    "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
	}
	/* Individual items must be histograms */
	if (item->type != HS_1D_HISTOGRAM && item->type != HS_2D_HISTOGRAM && 
	    item->type != HS_3D_HISTOGRAM && item->type != HS_GROUP) {
	    DialogF(DF_ERR, w, 1,
		    "Internal Error (View Multiple): Item not histogram",
		    "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
	}
	if (item->type == HS_GROUP) {
	    int j;
	    hsGroup *group = (hsGroup *) item;
	    hsGeneral *grItem;
	    for (j = 0; j < group->numItems; ++j) {
	    	grItem = GetItemByID(group->itemId[j], HistoList);
/* printf("    getting group item %d (%x)\n", group->itemId[j], item); */
	    	if (grItem == NULL)
	    	    continue;		/* ignore deleted items in group list */
	        if (grItem->type != HS_1D_HISTOGRAM && 
		    grItem->type != HS_2D_HISTOGRAM && 
		    grItem->type != HS_3D_HISTOGRAM && 
		    grItem->type != HS_GROUP) {
		    doWarn = 1;
		    ViewItem(XtDisplay(w), grItem, NULL, NULL, False, NULL, NULL);
		    continue;
		}	/* Histograms in groups */
	    	itemIds[k] = group->itemId[j];
	    	errsDisp[k++] = group->errsDisp[j];
	    }
	} else {	/* Individual 1-d and 2-d histograms */
	    itemIds[k] = ListedIDs[posList[i] - 1];
	    errsDisp[k++] = NO_ERROR_BARS;
    	}
    }
    if (posCount > 1) {			/* More than 1 item selected by user */
	winTitle = (char *) XtMalloc (12 + strlen(item->category));
	strcpy(winTitle, "Items from ");
	strcat(winTitle, item->category);
    } else {				/* 1 Group must have been selected */
	winTitle = (char *) XtMalloc (strlen(item->title) + 1);
	strcpy(winTitle, item->title);
    }
    if (k != numItems) {
    	DialogF(DF_ERR, w, 1,
		    "Internal Error: inconsistency.", "Acknowledged");
	    XtFree((char *)posList);
	    XmListDeselectAllItems(HistogramListW);
	    XtSetSensitive(ViewOvrlBtnW, False);
	    XtSetSensitive(ViewMultBtnW, False);
	    XtSetSensitive(ViewBtnW, False);
    	    return;
    }
    putUpMultiPlot(MainPanelW, winTitle, numItems, itemIds, errsDisp);
    if (doWarn > 0)
    	DialogF(DF_INF, w, 1,
    "Multi-Plot contains histograms only.\nOther items displayed individually.",
		"Acknowledged");
    XtFree((char *)winTitle);
    XtFree((char *)posList);
    XmListDeselectAllItems(HistogramListW);
    XtSetSensitive(ViewOvrlBtnW, False);
    XtSetSensitive(ViewMultBtnW, False);
    XtSetSensitive(ViewBtnW, False);
}

int LoadItemData(Widget parent, hsGeneral *item)
{
    char *message;
    int stat;

    /* If the data is comming from an hbook file, attach the data to the
       item here on the first view request.  If the scope is connected
       to a user process, the data is requested instead by the plot
       window creation routines */
    if (PanelState == HBOOK_FILE_OPEN && !ItemHasData(item)) {
    	stat = GetHbookDataF(HbookFilename, HbookBlockSize, item);
    	if (stat == HBOOK_DATA_MISSING)
    	    DialogF(DF_INF, parent, 1, "Item \"%s\" is empty", "Acknowledged",
    	    	    item->title);
    	else if (stat != HBOOK_DATA_OK) {
    	    if (stat == HBOOK_DATA_TOO_BIG)
    	    	message = "not enough memory";
    	    else if (stat == HBOOK_DATA_CORRUPTED)
    	    	message = "data is corrupted";
    	    else if (stat == HBOOK_ERRORS_TOO_BIG)
    	    	message = "not enough memory for errors";
    	    else message = "";
    	    DialogF(DF_WARN, parent, 1,
    	    	    "Could not read item %s from\nfile %s: %s",
    	    	    "Acknowledged", item->title, HbookFilename, message);
    	    if (stat != HBOOK_ERRORS_TOO_BIG)
    	    	return False;
    	}
    }
    return True;
}

windowInfo *ViewItem(Display *display, hsGeneral *item, char *winID,
	char *geometry, int headingType, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo)
{
/**
 ** Please note that this routine is now recursive if a group is being
 ** displayed (i.e. ViewItem calls itself!).  Therefore be careful of changes, 
 ** especially use of static variables.
 */
    windowInfo *wInfo = NULL;
    Widget appShell = NULL;

    /* for those items for which there can be only one copy, check if one
       is already displayed.  If so, raise it and don't create a new window */
     if (item->type == HS_CONTROL || item->type == HS_INDICATOR ||
     	    item->type == HS_TRIGGER) {
	if ((wInfo = FirstDisplayedItem(item->id)) != NULL) {
    	    XMapRaised(display, XtWindow(wInfo->shell));
    	    return wInfo;
    	}
    }
    
    /* If item is a group, and its default group type is multi-plot window,
       create a multi-plot window with all the histograms in the group. 
       Display non-histograms individually. */
    if (item->type == HS_GROUP) {
    	hsGroup *group = (hsGroup *)item;
    	hsGeneral *grItem;
	if (group->groupType == HS_MULTI_PLOT) {
    	    int i, numHists = 0, errsDisp[HS_MAX_NUM_GROUP_ITEMS];
    	    int histIDs[HS_MAX_NUM_GROUP_ITEMS];
    	    /* Count number of histograms and make list of histograms only */
    	    for (i = 0; i < group->numItems && i < HS_MAX_NUM_GROUP_ITEMS;++i) {
    	    	grItem = GetItemByID(group->itemId[i], HistoList);
    	    	if (grItem != NULL && (grItem->type == HS_1D_HISTOGRAM
				       || grItem->type == HS_2D_HISTOGRAM
				       || grItem->type == HS_3D_HISTOGRAM)) {
    	    	    errsDisp[numHists] = group->errsDisp[i];
    	    	    histIDs[numHists++] = group->itemId[i];
    	    	}
    	    	else if (grItem != NULL)
		    ViewItem(display, grItem, winID, NULL, False, csInfo, confInfo);
    	    }
    	    
    	    /* Put up the Multi-Plot */
    	    if (numHists > 0)
    	        putUpMultiPlot(MainPanelW, item->title, numHists, histIDs,
    	    	    errsDisp);
            wInfo = NULL;
        }
        
        /* Put up group items individually */
        else if (group->groupType == HS_INDIVIDUAL) {
	    int i;
	    for (i = 0; i < group->numItems; ++i) {
		grItem = GetItemByID(group->itemId[i], HistoList);
		if (grItem != NULL) {
		    /* Put up the window to view the item */
		    ViewItem(display, grItem, winID, NULL, False, csInfo, confInfo);
		}
	    }
            wInfo = NULL;
        }
        
        /* Overlay 1d Histograms in Group */
        else if (group->groupType == HS_OVERLAY_PLOT) {
            int i, j;
	    for (i = 0; i < group->numItems; ++i) {
		grItem = GetItemByID(group->itemId[i], HistoList);
		if (grItem != NULL && grItem->type == HS_1D_HISTOGRAM) {
		    /* Put up the window to view the item */
		    appShell = XtVaAppCreateShell ("plotShell", "PlotShell",
    			    applicationShellWidgetClass, display,
			    XmNgeometry, geometry,
			    XmNtitle, group->title,
			    XmNiconName, group->title, NULL);
		    wInfo = Create1DHistWindow(appShell, grItem, NULL, NULL,
			group->title, winID, appShell, False, NULL, NULL);
		    break;
		}
    	    	else if (grItem != NULL)
		    ViewItem(display, grItem, winID, NULL, False, csInfo, confInfo);
	    }
	    for (j = i + 1; j < group->numItems; ++j) {
	    	grItem = GetItemByID(group->itemId[j], HistoList);
	    	if (grItem != NULL && grItem->type == HS_1D_HISTOGRAM) {
		    /* Put up the window to view the item */
    		    OverlayPlot(wInfo, grItem, HIST1D, NULL, NULL, 
            	    	    ((hs1DHist *) grItem)->nBins, 0, NO_ERROR_BARS,
            	    	    NULL, 0, NULL, csInfo, confInfo);
	    	}
	    }
    	}
	/* update the Windows menu to include the new window */
	UpdateWindowsMenu();
    	return wInfo;
    }

    /* Create a toplevel shell to hold the window */
    /* Create and display the item */
    if (item->type != HS_NTUPLE)
	appShell = XtVaAppCreateShell ("plotShell", "PlotShell",
    		applicationShellWidgetClass, display,
		XmNgeometry, geometry,
		XmNtitle, item->title,
		XmNiconName, item->title, NULL);
    if (item->type == HS_1D_HISTOGRAM) {
	wInfo = Create1DHistWindow(appShell, item, NULL, NULL,
		item->title, winID, appShell, False, csInfo, confInfo);
    } else if (item->type == HS_2D_HISTOGRAM && headingType == CELL_HEADING) {
    	wInfo = CreateCellWindow(appShell, item, NULL, NULL,
    		item->title, winID, appShell, False, csInfo, confInfo);
    } else if (item->type == HS_2D_HISTOGRAM && headingType == COLORCELL_HEADING) {
    	wInfo = CreateColorCellWindow(appShell, item, NULL, NULL,
    		item->title, winID, appShell, False, csInfo, confInfo);
    } else if (item->type == HS_2D_HISTOGRAM) {
    	wInfo = Create2DHistWindow(appShell, item, NULL, NULL,
    		item->title, winID, appShell, False, csInfo, confInfo);
    } else if (item->type == HS_3D_HISTOGRAM) {
    	wInfo = NULL;
    } else if (item->type == HS_INDICATOR) {
    	wInfo = CreateIndicatorWindow(appShell, item, NULL, NULL,
				      item->title, appShell);
    } else if (item->type == HS_NTUPLE) {
    	CreateNTupleWindow(display, (hsNTuple *)item, geometry);
    	wInfo = NULL;
    } else if (item->type == HS_CONTROL) {
    	wInfo = CreateControlWindow(appShell, item, NULL, NULL,
    		item->title, appShell);
    } else if (item->type == HS_TRIGGER) {
    	wInfo = CreateTriggerWindow(appShell, item, NULL, NULL,
    		item->title, appShell);
    }

    /* if the item is a control or trigger, disable it if histoscope is
       not connected to a client process or the client is waiting to finish */
    if ((PanelState != CONNECTED || ClientWaiting) && (item->type == HS_CONTROL
     	    || item->type == HS_TRIGGER))
     	SetControlInsensitive(item);
    
    /* update the Windows menu to include the new window */
    UpdateWindowsMenu();
    
    return wInfo;
}

/*
 * putUpMultiPlot - convenience routine to put up a multiplot window from
 * 		    a list of histograms.  Figures out #rows, #cols, sizes
 *		    the window and uses the routines CreateMultPlotFromList
 *		    and DefaultMultiPlotGeom.  Also updates the windows menu.
 *   Arguments:
 *	parent	  - parent window for multiplot window
 *	title	  - title for multiplot window
 *	numItems  - number of histograms to put in multiplot window
 *	ids	  - an array of histoscope id's for the histograms to put in
 *			the window
 *	errsDisp  - array corresponding to ids which tells whether to plot
 *			errors in the histograms.  Individual values are
 *			defined in plotWindows.h and may be NO_ERROR_BARS,
 *			DATA_ERROR_BARS, or GAUSSIAN_ERROR_BAR.
 *		    errsDisp may equal NULL, signifying no error bars at all.
 */
static multWindow *putUpMultiPlot(Widget parent, char *title, int numItems, 
	int *ids, int *errsDisp)
{
    int numRows, numCols, width, height;
    multWindow *mult;
    
    numRows = (int) sqrt((double) numItems);
    numCols = numRows;
    if (numCols * numRows < numItems) {
    	++ numCols;
    	if (numCols * numRows < numItems)
    	    ++ numRows;
    }
    mult = CreateMultPlotFromList(parent, title, NULL, numRows, numCols,
	       numItems, ids, errsDisp);
    DefaultMultiPlotGeom(numRows, numCols, &width, &height);
    XtVaSetValues(mult->appShell, XmNwidth, width, XmNheight, height, NULL);
    
    /* Update the Windows menu to include the new window */
    UpdateWindowsMenu();
    
    return mult;
}

/*
** Update the variable list in an NTuple (this routine is here in mainPanel.c
** rather than in ntuplePanel.c because NPlot needs to do the same thing, but
** NPlot and Histo ntuple window lists are handled differently between the
** two programs)
*/
void UpdateNTuplePanelList(int id)
{
    FindAndUpdateNTuplePanelList(id);
}

/*
** Set the state of the MainPanel, and change the title and sensitivity to
** match.
*/
static void setMainPanelState(int state, char *title)
{
    PanelState = state;
    ClientWaiting = 0;
    XtSetSensitive(MainContentsW, state!=NO_FILE_OPEN);
    SET_ONE_RSRC(XtParent(MainPanelW), XmNtitle, title);
    SetRereadItemDim(state==HBOOK_FILE_OPEN || state==HISTO_FILE_OPEN);
    SetCloseItemDim(state!=NO_FILE_OPEN);
}

/*
** Set the top category based on whether all items start with the same initial
** category.  In HBOOK, there is often a single directory (category) in which
** all histograms are placed.  For the convenience of these users, this routine
** looks through all of the items currently listed and decides whether there
** is only a single top category.  If so, that is considered the top category,
** rather than "" (the empty string) which is the top category for histoscope
** data collection routines.
*/
static void resetTopCategory(void)
{
    char category[HS_MAX_CATEGORY_LENGTH+1];
    char firstTopCategory[HS_MAX_CATEGORY_LENGTH+1];
    histoListHeader *h;

    /* Empty histogram list: top category is the empty string */
    if (HistoList == NULL) {
    	TopCategory[0] = '\0';
    	return;
    }

    /* Compare the top category of the first item with the top category of all
       of the others.  If any don't match, top category is the empty string */
    nextCategory(firstTopCategory, HistoList->item->category);
    for (h = HistoList; h != NULL; h = h->next) {
    	nextCategory(category, h->item->category);
    	if (strcmp(category, firstTopCategory)) {
    	    TopCategory[0] = '\0';
    	    return;
	}
    }
    
    /* Top category for all items is the same, use it as the new top category */
    strcpy(TopCategory, firstTopCategory);

    /* If the current category does not descend from the new top category,
       change it to keep the category menu routine from getting confused.
       Otherwise, leave the current category as is even if no items match */
    nextCategory(category, CurrentCategory);
    if (strcmp(category, firstTopCategory))
	strcpy(CurrentCategory, firstTopCategory);
}

/*
** Redisplay the histogram/ntuple/indicator/control list
** from CurrentCategory and HistoList
*/
static void redisplayItems(void)
{
    int i, nItems = 0;
    histoListHeader *h;
    XmString *stringTable;
    char entryText[HS_MAX_TITLE_LENGTH + 17];
    static char dType[N_HS_DATA_TYPES][3] = {
	"1d", "2d", "nt", "i ", "c ", "t ", "  ", "nf", "gr", "cs", "3d"
    };

    /* Count the number of matching items on the histogram list */
    for (h = HistoList; h != NULL; h = h->next)
    	if (itemInCategory(h->item, CurrentCategory) 
    		&& h->item->type != HS_NFIT && h->item->type != HS_NONE)
    	    nItems++;

    /* Allocate enough memory for a string table to supply to the list
       widget, and an id table to map the list positions to histogram ids */
    if (ListedIDs != NULL)
    	free(ListedIDs);
    if (nItems == 0)			/* JMK - 6/28/93	*/
    	ListedIDs = NULL;
    else
    	ListedIDs = (int *)XtMalloc(sizeof(int) * nItems);
    stringTable = (XmString *)XtMalloc((nItems+1) * sizeof(XmString));
       
    /* Build the id table and the string table */
    nItems = 0;
    for (h=HistoList, i=0; h!=NULL; h=h->next, i++) {
    	if (itemInCategory(h->item, CurrentCategory) 
    		&& h->item->type != HS_NFIT && h->item->type != HS_NONE) {
    	    ListedIDs[nItems] = h->item->id;
    	    if (XmToggleButtonGetState(HbookBtnW) && 
		XmToggleButtonGetState(DataTypeBtnW)) {
    	    	sprintf(entryText, "%-11d %-2s  %s", h->item->hbookID != 0 ? 
    	    		h->item->hbookID : h->item->uid, dType[h->item->type], 
    	    		h->item->title);
    	    } else if (XmToggleButtonGetState(HsidBtnW) &&
		       XmToggleButtonGetState(DataTypeBtnW)) {
    	    	sprintf(entryText, "%-11d %-2s  %s", h->item->id,
			dType[h->item->type], h->item->title);
    	    } else if (XmToggleButtonGetState(HbookBtnW)) {
    	    	sprintf(entryText, "%-11d %s", h->item->hbookID != 0 ? 
    	    		h->item->hbookID : h->item->uid, h->item->title);
    	    } else if (XmToggleButtonGetState(HsidBtnW)) {
    	    	sprintf(entryText, "%-11d %s", h->item->id, h->item->title);
    	    } else if (XmToggleButtonGetState(DataTypeBtnW)) {
    	    	sprintf(entryText, "%-2s  %s", dType[h->item->type], 
    	    		h->item->title);
    	    } else {
    	    	strcpy(entryText, h->item->title);
    	    }
    	    stringTable[nItems++] = MKSTRING(entryText);
    	}
    }
    stringTable[nItems] = (XmString)0; /* end of table is marked with a 0 */

    /* Display the items in the histogram list widget */
    setListItems(HistogramListW, stringTable, nItems);

    /* free the string table */
    FreeStringTable(stringTable);
}

/*
** Redisplay the subcategory list from CurrentCategory and HistoList
*/
static void redisplaySubcategories(void)
{
    int i, c, duplicate, nCategories = 0;
    char subCat[HS_MAX_CATEGORY_LENGTH];
    histoListHeader *h;
    XmString mSubCat, localCategories[100];
    XmString *subCategories = localCategories;
    int maxCategories = sizeof(localCategories)/sizeof(localCategories[0])-1;

    /* Make the entry for moving up in the category hierarchy */
    if (strcmp(CurrentCategory, TopCategory))
    	subCategories[nCategories++] = MKSTRING("<-- Up One Level");

    /* Fill in the sub-categories from the histogram list */
    for (h = HistoList; h != NULL; h = h->next) {
    	if (subCategory(CurrentCategory, h->item->category, subCat)) {
    	    mSubCat = MKSTRING(subCat);
    	    /* Check for existing duplicate */
    	    duplicate = False;
    	    for (c = 0; c < nCategories; c++) {
    	    	if (XmStringCompare(subCategories[c], mSubCat)) {
    	    	    duplicate = True;
    	    	    XmStringFree(mSubCat);	/* JMK - 6/28/93	*/
    	    	    break;
    	    	}
    	    }
    	    if (!duplicate) {
                if (nCategories == maxCategories)
                {
                    XmString *newcat;
                    maxCategories += maxCategories/2;
                    newcat = (XmString *)malloc((maxCategories+1)*sizeof(XmString));
                    if (newcat == NULL)
                    {
                        fprintf(stderr, "Out of memory. Exiting.\n");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(newcat, subCategories, nCategories*sizeof(XmString));
                    if (subCategories != localCategories)
                        free(subCategories);
                    subCategories = newcat;
                }
    	    	subCategories[nCategories++] = mSubCat;
    	    }
    	}
    }
    subCategories[nCategories] = NULL;

    /* Display the list of categories in the category list widget */
    setListItems(CategoryListW, subCategories, nCategories);

    /* Free the Xm strings */
    for(i = 0; i<nCategories; i++)
	XmStringFree(subCategories[i]);
    if (subCategories != localCategories)
        free(subCategories);
}

/*
** Redisplay the category menu from CurrentCategory and HistoList
*/
static void redisplayCategoryMenu(void)
{
    char *cats, categories[HS_MAX_CATEGORY_LENGTH], cat[HS_MAX_CATEGORY_LENGTH];
    static Widget categoryBtns[HS_MAX_CATEGORY_DEPTH];
    Widget oldBtns[HS_MAX_CATEGORY_DEPTH];
    static int nBtns = 0;
    int oldNBtns;
    static Arg args[1] = {{XmNlabelString, (XtArgVal)0}};
    Widget button = NULL;
    int i, catNum, nCats = 0;

    /* leave the old buttons in the menu until after the new ones are added.
       (Motif option menus are tempramental, this is necessary on SGI) */
    for (i=0; i<nBtns; i++)
    	oldBtns[i] = categoryBtns[i];
    oldNBtns = nBtns;
    nBtns = 0;
    
    /* If the top category is "", add a fake one called "Uncategorized",
       start the category numbers: 0 == "" 1 == "cat1" 2 == "cat1/cat2" */
    if (!strcmp(TopCategory, "")) {
    	sprintf(categories, "Uncategorized/%s", CurrentCategory);
    	catNum = 0;
    } else {
    	strcpy(categories, CurrentCategory);
    	catNum = 1;
    }
    cats = categories;

    /* loop through the categories creating a new menu button for each */
    while (True) {
    	cats = nextCategory(cat, cats);
    	if (cats == NULL)
    	    break;
    	nCats++;
    	if (args[0].value != 0)
    	    XmStringFree((XmString)args[0].value);
    	args[0].value = (XtArgVal)MKSTRING(cat);
    	button =  XmCreatePushButton(CategoryMenuW, "btn", args, 1);
    	XtAddCallback(button, XmNactivateCallback,
    		      (XtCallbackProc)categoryMenuCB,
                      (caddr_t)((long)catNum++));
    	XtManageChild(button);
    	categoryBtns[nBtns++] = button;
    }
    /* Safety check, stuff below will bomb if no categories were processed */
    if (nCats == 0)
    	fprintf(stderr, "Internal Error, unreadable current category\n");
    
    /* Set the option menu to show the last category in the list.  Use
       the button widget and same argument list used in the last iteration
       of the loop above to change the position of the option menu and to
       set the label on the label gadget associated with the option menu.
       (In some cases, setting XmNmenuHistory for the option menu is
       is not sufficient to make motif change the label.)	     */
    SET_ONE_RSRC(CategoryOptMenuW, XmNmenuHistory, button)
    XtSetValues(XmOptionButtonGadget(CategoryOptMenuW), args, 1);
    XmStringFree((XmString)args[0].value);
    args[0].value = (XtArgVal)0;

    /* destroy all of the old buttons */
    for (i=0; i<oldNBtns; i++)
    	XtDestroyWidget(oldBtns[i]);
}    

static int itemInCategory(hsGeneral *item, char *category)
{
    return !strcmp(category, item->category);
}

static char *nextCategory(char *toString, char *fromString)
{
    char *to = toString, *from = fromString;
    
    while (TRUE) {
        if (*from =='\0') {
            *to = '\0';
            if (to == toString)
            	return NULL;		/* No categories left in string */
            else
            	return from;		/* will return NULL on next call */
        } else if (*from=='/') {
	    *to = '\0';
	    if (to == toString)		/* ignore leading '/' and // */
	    	from++;
	    else
	    	return from;
	} else {
	    *to++ = *from++;
	}
    }
}

static int subCategory(char *category, char *categories, char *subCategory)
{
    int catLen;
    
    catLen = strlen(category);
    /* make sure that entire subcategory is compared */
    /* fixed bug 2/24/97 _added_rest_of_stmt___v     */
    if (!strncmp(category, categories, catLen) && (categories[catLen] == '\0'
    		|| categories[catLen] == '/' || catLen == 0) )
    	if (nextCategory(subCategory, &categories[catLen]) != NULL)
    	    return True;
    return False;
}

/*
** Set the contents of a list and compensate for various motif problems
** associated with this apparently bizarre act.  (It might be worth
** further investigation to better understand why these workarounds
** are necessary).
*/
static void setListItems(Widget w, XmString *strings, int nStrings)
{
    XmString *st1 = 0;
    Arg  args[10];
    Cardinal n = 0;

    /* Motif doesn't reset the selection when items are changed */
    XmListDeselectAllItems(w);

    XtSetSensitive(ViewOvrlBtnW, False);
    XtSetSensitive(ViewMultBtnW, False);
    XtSetSensitive(ViewBtnW, False);

    /* .. and sometimes leaves stray scroll bars if nothing is in the list */
    if (nStrings == 0) {
        st1 = StringTable(" ");
        XtSetArg(args[n], XmNitems, st1); n++;
        XtSetArg(args[n], XmNitemCount, 1); n++;
        XtSetArg(args[n], XmNsensitive, False); n++;
    } else {
        XtSetArg(args[n], XmNitems, strings); n++;
        XtSetArg(args[n], XmNitemCount, nStrings); n++;
        XtSetArg(args[n], XmNsensitive, True); n++;
    }
    XtSetValues(w, args, n);
    FreeStringTable(st1);
}

/*
** present user with a non-standard file selection dialog (asks for blocksize)
** to get HBOOK file name and file block size.  Also checks for all lower
** case in Unix case because HBOOK is so extremely brain dead it can't handle
** upper case letters in file names
*/
static int getHbookFilename(char *filename, int *blockSize)
{
    int resp, bsOK, newBlockSize;
    Widget fileSB, form, label, text;
    XmString s1, s2;
    int n;
    Arg args[10];
    
    while(True) {
	n=0;
	XtSetArg(args[n], XmNlistLabelString,
    		s1=XmStringCreateSimple("HBOOK file to open:")); n++;
	XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
	XtSetArg(args[n], XmNdialogTitle, s2=XmStringCreateSimple(" ")); n++;
	fileSB = XmCreateFileSelectionDialog(MainPanelW, "hbFileSB", args, n);
	XmStringFree(s1); XmStringFree(s2);
	XtUnmanageChild(XmFileSelectionBoxGetChild(fileSB, XmDIALOG_TEXT)); 
	XtUnmanageChild(XmFileSelectionBoxGetChild(fileSB,
		XmDIALOG_SELECTION_LABEL));
	form = XtVaCreateManagedWidget("bsForm", xmFormWidgetClass, fileSB, NULL);
	label = XtVaCreateManagedWidget("bsLabel", xmLabelGadgetClass, form,
    		XmNlabelString,
    		    s1=XmStringCreateSimple("HBOOK file record length:"),
    		XmNtopAttachment, XmATTACH_FORM,
    		XmNbottomAttachment, XmATTACH_FORM, NULL);
	text = XtVaCreateManagedWidget("bsText", xmTextFieldWidgetClass, form,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNrightAttachment, XmATTACH_FORM,
    		XmNleftWidget, label, NULL);
    	RemapDeleteKey(text);
    	SetIntText(text, *blockSize);
    
	resp = HandleCustomExistFileSB(fileSB, filename);
	XmStringFree(s1);		/* JMK - 6/28/93	*/
    	if (resp == GFN_CANCEL)
    	    return False;
    	bsOK = GetIntText(text, &newBlockSize);
    	if (bsOK == TEXT_READ_OK) {
	    *blockSize = newBlockSize;
    	    return True;
	} else if (bsOK == TEXT_IS_BLANK)
	    DialogF(DF_ERR, MainPanelW, 1, "Please enter record length\n\
for HBOOK file",  "Acknowledged");
	else /* bsOK == TEXT_NOT_NUMBER */
	    DialogF(DF_ERR, MainPanelW, 1, "Can't read value for HBOOK\n\
file record length",  "Acknowledged");
    }
}
