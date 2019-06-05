/*******************************************************************************
*									       *
* mainMenu.c -- Histoscope main menu					       *
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
*******************************************************************************/

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include "../util/misc.h"
#include "../util/help.h"
#include "../util/getfiles.h"
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "ntuplePanel.h"
#include "preferences.h"
#include "communications.h"
#include "help.h"
#include "mainMenu.h"
#include "configFile.h"
#include "multPlot.h"

enum windowMenuItemTypes {PERMANENT, WINDOW_NAME};

static void openCB(Widget w, caddr_t clientData, caddr_t callData);
static void openHbookCB(Widget w, caddr_t clientData, caddr_t callData);
static void loadCB(Widget w, caddr_t clientData, caddr_t callData);
static void saveCB(Widget w, caddr_t clientData, caddr_t callData);
static void closeCaptiveCB(Widget w, caddr_t clientData, caddr_t callData);
static void exitCB(Widget w, caddr_t clientData, caddr_t callData);
static void connectCB(Widget w, caddr_t clientData, caddr_t callData);
static void updateFreqCB(Widget w, caddr_t clientData, caddr_t callData);
static void bufferGraphicsCB(Widget w, caddr_t clientData, caddr_t callData);
static void autoPlotHelpCB(Widget w, caddr_t clientData, caddr_t callData);
static void savePrefsCB(Widget w, caddr_t clientData, caddr_t callData);
static void rereadCB(Widget w, caddr_t clientData, caddr_t callData);
static void closeCB(Widget w, caddr_t clientData, caddr_t callData);
static void closeAllCB(Widget w, caddr_t clientData, caddr_t callData);
static void multPlotCB(Widget w, caddr_t clientData, caddr_t callData);
static void raiseCB(Widget w, Window window, caddr_t callData);
static void modifyWindowMenuCB(Widget w, caddr_t clientData, caddr_t callData);
static void UpdateWindowsMenu_old(void);

/* Globals */
int InhibitWindowMenuUpdates = False;

/* Module Globals */
static Widget RereadItem = NULL;
static Widget CloseItem = NULL;
static Widget WindowMenuPane;
static Widget WindowMenuBtn;
static Widget BufferGraphicsBtn = NULL;
static Widget AutoHelpBtn = NULL;
static int windowMenuNeedsUpdating = 0;

/*
** Create the menu bar
*/
Widget CreateMainMenuBar(Widget parent, int captiveMode)
{
    Widget menuBar, menuPane, btn;
    XmString s1;

    /*
    ** Create MenuBar widget.
    */
    menuBar = XmCreateMenuBar(parent, "menuBar", NULL, 0);

    /*
    ** Create "File/Process", or "Window" pull down menu depending on
    ** whether this is a captive histoscope started by a user program
    */
    menuPane = XmCreatePulldownMenu(menuBar, "file", NULL, 0);
    if (!captiveMode) {
	AddMenuItem(menuPane, "open", "Open Histoscope File...", 'O',
	     "Ctrl<Key>o", "Ctrl O", 
	     (XtCallbackProc) openCB, NULL);
	AddMenuItem(menuPane, "openHbook", "Open HBOOK File...", 'H',
	     "Ctrl<Key>h", "Ctrl H",
	     (XtCallbackProc)  openHbookCB, NULL);
	AddMenuItem(menuPane, "connect", "Connect to Process...", 'C',
    	     "Ctrl<Key>n", "Ctrl N",
    	     (XtCallbackProc)  connectCB, NULL);
    	RereadItem = AddMenuItem(menuPane, "reread", "Re-read Same File", 'R',
    	     "Ctrl<Key>r", "Ctrl R",
    	     (XtCallbackProc)  rereadCB, NULL);
    	CloseItem = AddMenuItem(menuPane, "close", "Close File/Connection", 'l',
    	     "C", "", 
    	     (XtCallbackProc) closeCB, NULL);
	AddMenuSeparator(menuPane, "separator1");
    }
    AddMenuItem(menuPane, "loadConfiguration", "Load Configuration...", 'g',
    	    "Ctrl<Key>L", "Ctrl+L", (XtCallbackProc)loadCB, NULL);
    AddMenuItem(menuPane, "saveConfiguration", "Save Configuration...", 'S',
    	    "Ctrl<Key>S", "Ctrl+S", (XtCallbackProc)saveCB, NULL);
    AddMenuSeparator(menuPane, "separator2");
    AddMenuItem(menuPane, "exit", "Exit", 'E', "Ctrl<Key>D", "Ctrl+D",
    	    (XtCallbackProc)(captiveMode ? closeCaptiveCB : exitCB), NULL);
    s1 = XmStringCreateSimple("File/Process");
    {
      Arg args[20];
      Cardinal n;
      n = 0;
      XtSetArg (args[n], XmNlabelString, s1); n++;
      XtSetArg (args[n], XmNmnemonic, 'F'); n++;
      XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
      XtCreateManagedWidget("fileB", xmCascadeButtonWidgetClass, menuBar, args, n);
    }
    XmStringFree(s1);
    
    /*
    ** Create the Preferences pull down menu
    */
    menuPane = XmCreatePulldownMenu(menuBar, "preferences", NULL, 0);
    AddMenuItem(menuPane, "updateFreq", "Update Frequency...", 'U',
    	    NULL, "", (XtCallbackProc)  updateFreqCB, NULL);
    BufferGraphicsBtn = AddMenuToggle(menuPane, "bufferGraphics", "Buffer Graphics", 'B',
    	    NULL, "",
    	    (XtCallbackProc)  bufferGraphicsCB, NULL, GetGraphicsBuffering());
    AutoHelpBtn = AddMenuToggle(menuPane, "autoPlotHelp", "Automatic Plot Help", 'A',
    	    NULL, "",
    	    (XtCallbackProc)  autoPlotHelpCB, NULL, PrefData.plotAutoHelp);
    AddMenuSeparator(menuPane, "separator1");
    AddMenuItem(menuPane, "savePrefs", "Save Preferences", 'S',
    	    NULL, "", (XtCallbackProc)  savePrefsCB, NULL);
    XtVaCreateManagedWidget("prefB", xmCascadeButtonWidgetClass, menuBar,
    	    XmNlabelString, s1=XmStringCreateSimple("Preferences"),
    	    XmNmnemonic, 'P', XmNsubMenuId, menuPane, NULL);
    XmStringFree(s1);

    /*
    ** Create the Windows menu.  Mark the items in it as permanent,
    ** since the remainder of the menu is filled with names of open windows
    ** which are changed as the user opens and closes windows.
    */
    WindowMenuPane = XmCreatePulldownMenu(menuBar, "windows", NULL, 0);
    btn = AddMenuItem(WindowMenuPane, "closeAll", "Close All Windows", 'C',
    	    NULL, "", (XtCallbackProc) closeAllCB, NULL);
    XtVaSetValues(btn, XmNuserData, (XtPointer)PERMANENT, NULL);
    btn = AddMenuItem(WindowMenuPane, "multPlot", "Multiple Plot Window...", 
    	    'M', NULL, "", (XtCallbackProc) multPlotCB, NULL);
    XtVaSetValues(btn, XmNuserData, (XtPointer)PERMANENT, NULL);
    btn = AddMenuSeparator(WindowMenuPane, "sep");
    XtVaSetValues(btn, XmNuserData, (XtPointer)PERMANENT, NULL);
    s1=XmStringCreateSimple("Windows");
    {
      Arg args[20];
      Cardinal n;
      n = 0;
      XtSetArg (args[n], XmNlabelString, s1); n++;
      XtSetArg (args[n], XmNsensitive, True); n++;
      XtSetArg (args[n], XmNmnemonic, 'W'); n++;
      XtSetArg (args[n], XmNsubMenuId, WindowMenuPane); n++;
      WindowMenuBtn = XtCreateManagedWidget("windowsB",
	  xmCascadeButtonWidgetClass, menuBar, args, n);
    }
    XmStringFree(s1);
    XtAddCallback(WindowMenuBtn, XmNcascadingCallback,
		  (XtCallbackProc)modifyWindowMenuCB, (XtPointer)WindowMenuPane);

    /* Create the Help menu */
    CreateHelpPulldownMenu(menuBar, captiveMode?CapMainMenuHelp:MainMenuHelp);
    
    return (menuBar);
}

void SetCloseItemDim(int state)
{
    if (CloseItem != NULL)
    	XtSetSensitive(CloseItem, state);
}


void SetRereadItemDim(int state)
{
    if (RereadItem != NULL)
    	XtSetSensitive(RereadItem, state);
}

static void openCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    OpenNewFile(HISTO_FORMAT);
}

static void openHbookCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    OpenNewFile(HBOOK_FORMAT);
}

static void connectCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    OpenNewConnection();
}

static void loadCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    char filename[MAXPATHLEN];
    
    if (GetExistingFilename(w, "HistoScope Configuration File:", filename)
    		== GFN_CANCEL)
    	return;    
    ReadConfigFile(w, filename);
}

static void saveCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    char filename[MAXPATHLEN];
    
    if (GetNewFilename(w, "Name for Configuration File:", filename) 
    		== GFN_CANCEL)
    	return;
    SaveConfigFile(w, filename);
}

static void closeCaptiveCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    DisconnectProcess();
    exit(0);
}

static void exitCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    DisconnectProcess();
    exit(0);
}

static void updateFreqCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    ShowUpdateFreqPanel(MainPanelW);
}

static void bufferGraphicsCB(Widget w, caddr_t clientData, caddr_t callData)
{
    BufferGraphics(XmToggleButtonGetState(w));
}

static void autoPlotHelpCB(Widget w, caddr_t clientData, caddr_t callData)
{
    PrefData.plotAutoHelp = XmToggleButtonGetState(w);
}

static void savePrefsCB(Widget w, caddr_t clientData, caddr_t callData)
{
    SaveHistoPreferences(MainPanelW, PREF_FILE_NAME);
}

static void rereadCB(Widget w, caddr_t clientData, caddr_t callData)
{
    RereadFile();
}

static void closeAllCB(Widget w, caddr_t clientData, caddr_t callData)
{
    /* Mini-plots in Multi-Plot windows MUST be closed first !!!
       This is because each mini-plot is also on the WindowInfo list,
       and we don't want to delete/destroy them twice!  CloseAllMultWindows
       closes all multiple-plot windows and removes each mini-plot from
       the WindowInfo list */
    CloseAllMultWindows();
    CloseAllPlotWindows();
    CloseAllNTupleWindows();
}

static void multPlotCB(Widget w, caddr_t clientData, caddr_t callData)
{
    CreateMultPlotWin();
}

static void closeCB(Widget w, caddr_t clientData, caddr_t callData)
{
    CloseFileOrConnection();
}

/*
** Update the Window menu to reflect the currently open plot windows
** and ntuple windows.
*/
static void UpdateWindowsMenu_old(void)
{
    Widget btn;
    WidgetList items;
    int nItems, n, nNtupleNames, nPlotNames, nNames;
    XtPointer userData;
    char **names;
    Window *windows;
    
    /* While it is not possible on some systems (ibm at least) to substitute
       a new menu pane, it is possible to substitute menu items, as long as
       at least one remains in the menu at all times. This routine assumes
       that the menu contains permanent items marked with the value PERMANENT
       in the userData resource, and adds and removes items which it marks
       with the value WINDOW_NAME */
    
    /* Updating of the window menu can be inhibited by setting
       InhibitWindowMenuUpdates to False.  This prevents an apparent
       synchronization problem which occurs when you load a configuration
       file at start up time.  The cause of this problem is unknown */
    if (InhibitWindowMenuUpdates)
        return;

    /* Remove all of the existing window names from the menu */
    XtVaGetValues(WindowMenuPane, XmNchildren, &items,
    	    XmNnumChildren, &nItems, NULL);
    for (n=0; n<nItems; n++) {
    	XtVaGetValues(items[n], XmNuserData, &userData, NULL);
    	if (userData == (XtPointer)WINDOW_NAME) {
    	    /* unmanaging before destroying stops parent from displaying */
    	    XtUnmanageChild(items[n]);
    	    XtDestroyWidget(items[n]);
    	}
    }
    
    /* Count the open windows */
    nNtupleNames = CountNtupleWindows();
    nPlotNames = CountPlotWindows();
    nNames = nNtupleNames + nPlotNames + CountMultWindows();
    
    /* Get the current windows and their titles from plot, ntuple and
       multi-plot windows */
    names = (char **)XtMalloc(sizeof(char *) * nNames);
    windows = (Window *)XtMalloc(sizeof(Window *) * nNames);
    GetNtupleWindowsAndTitles(windows, names);
    GetPlotWindowsAndTitles(&windows[nNtupleNames], &names[nNtupleNames]);
    GetMplotWindowsAndTitles(&windows[nNtupleNames + nPlotNames], 
    	&names[nNtupleNames + nPlotNames]);
    
    /* Add the window titles to the windows menu */
    for (n=0; n<nNames; n++) {
    	btn = AddMenuItem(WindowMenuPane, "win", names[n], 0, NULL, "",
    		(XtCallbackProc)raiseCB, (void *)windows[n]);
    	XtVaSetValues(btn, XmNuserData, (XtPointer)WINDOW_NAME, NULL);
    }
    XtFree((char *)names);
    XtFree((char *)windows);
}

static void raiseCB(Widget w, Window window, caddr_t callData)
{
    /* XMapRaised as opposed to XRaiseWindow will uniconify as well as raise */
    XMapRaised(XtDisplay(w), window);
}

void UpdateWindowsMenu(void)
{
    windowMenuNeedsUpdating = 1;
}

static void modifyWindowMenuCB(Widget w, caddr_t clientData, caddr_t callData)
{
    if (windowMenuNeedsUpdating)
    {
	windowMenuNeedsUpdating = 0;
	UpdateWindowsMenu_old();
    }
}

void SetBufferGraphicsBtnState(int state)
{
    Boolean s = (state ? True : False);
    if (BufferGraphicsBtn)
	XmToggleButtonSetState(BufferGraphicsBtn, s, True);
}

void SetAutoHelpBtnState(int state)
{
    Boolean s = (state ? True : False);
    if (AutoHelpBtn)
	XmToggleButtonSetState(AutoHelpBtn, s, True);
}
