/*******************************************************************************
*									       *
* nplot.c -- Main program for NPlot application				       *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
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
* July 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#ifdef VMS
#include "../util/VMSparam.h"
#include "../util/VMSUtils.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include "../util/misc.h"
#include "../util/fileUtils.h"
#include "../util/getfiles.h"
#include "../util/DialogF.h"
#include "../util/help.h"
#include "../util/printUtils.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "histoP.h"
#include "help.h"
#include "preferences.h"
#include "plotWindows.h"
#include "ntuplePanel.h"
#include "configFile.h"
#include "interpret.h"
#include "variablePanel.h"
#include "globalgc.h"
#include "ColorScale.h"

/* Globals needed by some code in Histo-Scope */
GC globalGC = 0;
Widget MainPanelW = 0;

/* The math.h on the Sun mysteriously excludes strtod and other functions
   when POSIX compliance is turned on */
extern double strtod();

#define MAX_ERROR_LINE_LEN 40	/* how much of blown line to show user */
#define MAX_LINE_LEN 2048	/* maximum # of characters on a line */
#define APP_NAME "nplot"	/* application name for loading X resources */
#define APP_CLASS "NPlot"	/* application class for loading X resources */
#ifndef VMS
#define NPLOT_PREF_FILE ".nplot"/* name of preferences file */
#else
#define NPLOT_PREF_FILE ".NPLOT"
#endif /*VMS*/

enum windowMenuItemTypes {PERMANENT, WINDOW_NAME};

typedef struct _ntupleWindow {
    struct _ntupleWindow *next;		/* pointer to next in list */
    int empty;				/* window is empty */
    Widget shell;			/* application shell for window */
    Widget closeItem;			/* file menu close button */
    Widget windowMenuPane;		/* menu pane for windows menu */
    Widget windowMenuBtn;		/* cascade button for windows menu */
    ntWidgets widgets;			/* widgets for controling panel */
    hsNTuple *nTuple;			/* data from the file */
    char filename[MAXPATHLEN];		/* name of file being displayed */
    char path[MAXPATHLEN];		/* file system path to file */
} ntupleWindow;

/* The list of all NPlot windows */
static ntupleWindow *NtuplePanelList = NULL;

/* application fallback resources */
static char *fallbackResources[] = {
    "plotShell*plotWidget.background: gray90",
    "plotShell.plotWidget.foreground: black",
    0
};

/* Valid characters for separating numbers on a line of data */
static char Separators[] = " \t,\n";

int InhibitWindowMenuUpdates = False;

/* This global means nothing to nplot, but must be present for nplot to link
 * correctly because of routines used by both nplot and histo.
 */
int MultWindowList = 0;

static ntupleWindow *createNPlotWindow(Display *display, char *name);
static ntupleWindow *findWindowWithFile(char *name, char *path);
void UpdateWindowsMenu(void);
static void openFile(Widget parent);
static int readFile(char *name, char *path, ntupleWindow *window);
static int readHeadingLine(char *line, char **headings, int maxCols);
static int readDataLine(char *line, float *vector, int maxCols);
static void truncateErrorLine(char *line);
static int isBlank(char *line);
static void closeWindow(ntupleWindow *window);
static void setWindowEmpty(ntupleWindow *window, int empty);
static void removeFromWindowList(ntupleWindow *window);
static void closeCB(Widget w, ntupleWindow *wInfo, caddr_t callData);
static void windowFrameCloseCB(Widget w, ntupleWindow *wInfo, caddr_t callData);
static void openCB(Widget w, ntupleWindow *wInfo, caddr_t callData);
static void loadCB(Widget w, caddr_t clientData, caddr_t callData);
static void saveCB(Widget w, caddr_t clientData, caddr_t callData);
static void exitCB(Widget w, ntupleWindow *wInfo, caddr_t callData);
static void bufferGraphicsCB(Widget w, ntupleWindow *wInfo, caddr_t callData);
static void autoPlotHelpCB(Widget w, caddr_t clientData, caddr_t callData);
static void savePrefsCB(Widget w, ntupleWindow *wInfo, caddr_t callData);
static void closeAllCB(Widget w, caddr_t clientData, caddr_t callData);
static void raiseCB(Widget w, Window window, caddr_t callData);
static void registerPixmaps(void);

int main(int argc, char **argv)
{
    int i, windowCreated, stat;
    XtAppContext context;
    Display *display;
    XrmDatabase prefDB;
    char filename[MAXPATHLEN], pathname[MAXPATHLEN];
    char *configFile = NULL;
    ntupleWindow *window = NULL;
    
    /* Initialize the toolkit and create the obligatory application context */
    XtToolkitInitialize ();
    context = XtCreateApplicationContext();
    
    /* Motif still has problems with releasing non-existent passive grabs */
    SuppressPassiveGrabWarnings();

    /* Set up default resources if no app-defaults file is found */
    XtAppSetFallbackResources(context, fallbackResources);
    
#ifdef VMS
    /* Convert VMS style command line to Unix style */
    ConvertVMSCommandLine(&argc, &argv);
#endif /*VMS*/
    
    /* Read the preferences file and command line into a database */
    prefDB = CreateHistoPrefDB(NPLOT_PREF_FILE, APP_NAME,
    	    (unsigned int *)&argc, argv);
    
    /* Open the display, give up if it can't be opened */
    display = XtOpenDisplay (context, NULL, "nplot", "NPlot", NULL,
                                0, &argc, argv);
    if (!display) {
	fprintf(stderr, "nplot: Can't open display");
	exit(0);
    }

    /* Store preferences from the command line and .nedit file  */
    RestoreHistoPrefs(prefDB, XtDatabase(display), APP_NAME, APP_CLASS);
    LoadPrintPreferences(XtDatabase(display), APP_NAME, APP_CLASS, True);

    /* Register pixmaps for use with Motif */
    registerPixmaps();
    
    /* Process command line arguments.  For now, just files to display */
    windowCreated = FALSE;
    for (i=1; i<argc; i++) {
    	if (*argv[i] != '-') {
#ifdef VMS
	    int numFiles, j;
	    char **nameList = NULL;
	    /* Use VMS's LIB$FILESCAN for filename in argv[i] to process */
	    /* wildcards and to obtain a full VMS file specification     */
	    numFiles = VMSFileScan(argv[i], &nameList, NULL, EXCLUDE_FNF);
	    /* for each expanded file name do: */
	    for (j = 0; j < numFiles; ++j) {
	    	ParseFilename(nameList[j], filename, pathname);
	        window = createNPlotWindow(display, filename);
	        stat = readFile(filename, pathname, window);
	        if (!stat && windowCreated)
		    closeWindow(window);
	        windowCreated = TRUE;
		free(nameList[j]);
	    }
	    if (nameList != NULL)
	    	free(nameList);
#else
	    ParseFilename(argv[i], filename, pathname);
	    window = createNPlotWindow(display, filename);
	    stat = readFile(filename, pathname, window);
	    if (!stat && windowCreated)
		closeWindow(window);
	    windowCreated = TRUE;
#endif /*VMS*/
	} else if (!strcmp(argv[i], "-config")) {
	    i++;
	    if (i >= argc) {
		fprintf(stderr, "nplot: -config requires an argument\n");
		exit(0);
	    }
	    configFile = argv[i];
	}
    }
    
    /* If no file names were given on the command line create an empty window */
    if (!windowCreated)
        window = createNPlotWindow(display, "NPlot");

    /* If a configuration file was specified, load it */
    if (configFile != NULL)
        ReadConfigFile(window->shell, configFile);

    /* Process events */
    XtAppMainLoop (context);
    return 0;
}

static void openFile(Widget parent)
{
    char fullname[MAXPATHLEN], name[MAXPATHLEN], path[MAXPATHLEN];
    ntupleWindow *window;
    int resp;
    
    /* Get the name of the file to open from the user */
    resp = GetExistingFilename(parent, "File to open:", fullname);
    if (resp != GFN_OK)
    	return;
    ParseFilename(fullname, name, path);
    
    /* First look for the file already displayed in a window */
    window = findWindowWithFile(name, path);
    if (window != NULL) {
    	XRaiseWindow(XtDisplay(window->shell), XtWindow(window->shell));
	return;
    }
    
    /* If the empty window is up, read a file into it */
    if (NtuplePanelList->empty) {
    	readFile(name, path, NtuplePanelList);
    	return;
    }
    
    /* Create a new window and read the file into it */
    window = createNPlotWindow(XtDisplay(parent), name);
    if (!readFile(name, path, window)) {
    	closeWindow(window);
    }
}

static ntupleWindow *createNPlotWindow(Display *display, char *name)
{
    ntupleWindow *window;
    Widget appShell, wmain, menuBar, menuPane, btn;
    XmString s1;
    
    /* Create a window data structure */
    window = (ntupleWindow *)XtMalloc(sizeof(ntupleWindow));
    window->next = NtuplePanelList;
    window->nTuple = NULL;
    NtuplePanelList = window;
    
    /* Create a window with a menu bar */
    appShell = XtVaAppCreateShell ("nTuple", "NTuple",
		applicationShellWidgetClass, display,
		XmNtitle, name,
		XmNiconName, name,
		XmNallowShellResize, False, NULL);
    MainPanelW = appShell;
    AddMotifCloseCallback(appShell, (XtCallbackProc)windowFrameCloseCB, window);
    window->shell = appShell;
    wmain = XtVaCreateManagedWidget("mainWin", xmMainWindowWidgetClass,
    				   appShell, NULL);
    menuBar = XmCreateMenuBar(wmain, "menuBar", NULL, 0);
     
    /* Create the File menu */
    menuPane = XmCreatePulldownMenu(menuBar, "file", NULL, 0);
    AddMenuItem(menuPane, "open", "Open", 'O', "Ctrl<Key>o",
    		"Ctrl+O", (XtCallbackProc)openCB, window);
    AddMenuSeparator(menuPane, "s1");
    AddMenuItem(menuPane, "loadConfiguration", "Load Configuration...", 'L',
    	    "Ctrl<Key>L", "Ctrl+L", (XtCallbackProc)loadCB, NULL);
    AddMenuItem(menuPane, "saveConfiguration", "Save Configuration...", 'S',
    	    "Ctrl<Key>S", "Ctrl+S", (XtCallbackProc)saveCB, NULL);
    AddMenuSeparator(menuPane, "separator2");
    window->closeItem = AddMenuItem(menuPane, "close", "Close", 'C',
    		"Ctrl<Key>w", "Ctrl+W", (XtCallbackProc)closeCB, window);
    AddMenuItem(menuPane, "exit", "Exit", 'E', "Ctrl<Key>D",
    		"Ctrl+D", (XtCallbackProc)exitCB, window);
    XtVaCreateManagedWidget("fileB", xmCascadeButtonWidgetClass, menuBar,
    		XmNsubMenuId, menuPane,
    		XmNlabelString, s1=MKSTRING("File"),
    		XmNmnemonic, 'F', NULL);
    XmStringFree(s1);
    
    /* Create the Preferences menu */
    menuPane = XmCreatePulldownMenu(menuBar, "pref", NULL, 0);
    AddMenuToggle(menuPane, "bufferGraphics", "Buffer Graphics", 'B', NULL,
    	    "", (XtCallbackProc)bufferGraphicsCB, NULL, GetGraphicsBuffering());
    AddMenuToggle(menuPane, "autoPlotHelp", "Automatic Plot Help", 'A',  NULL,
    	    "", (XtCallbackProc)autoPlotHelpCB, NULL, PrefData.plotAutoHelp);
    AddMenuSeparator(menuPane, "separator1");
    AddMenuItem(menuPane, "savePrefs", "Save Preferences", 'S', NULL,
    	    "", (XtCallbackProc)savePrefsCB, window);
    XtVaCreateManagedWidget("prefB", xmCascadeButtonWidgetClass, menuBar,
    		XmNsubMenuId, menuPane,
    		XmNlabelString, s1=MKSTRING("Preferences"),
    		XmNmnemonic, 'P', NULL);
    XmStringFree(s1);

    /*Create the Windows menu.  Mark the items in it as permanent,
      since the remainder of the menu is filled with names of open windows
      which are changed as the user opens and closes windows. */
    menuPane = XmCreatePulldownMenu(menuBar, "windows", NULL, 0);
    btn = AddMenuItem(menuPane, "closeAll", "Close All Plot Windows", 'C',
    	    NULL, "", (XtCallbackProc) closeAllCB, NULL);
    XtVaSetValues(btn, XmNuserData, (XtPointer)PERMANENT, NULL);
    btn = AddMenuSeparator(menuPane, "sep");
    XtVaSetValues(btn, XmNuserData, (XtPointer)PERMANENT, NULL);
    window->windowMenuBtn = XtVaCreateManagedWidget("windowsB",
    	    xmCascadeButtonWidgetClass, menuBar,
    	    XmNlabelString, s1=XmStringCreateSimple("Windows"),
    	    XmNmnemonic, 'W', XmNsubMenuId, menuPane, NULL);
    XmStringFree(s1);
    window->windowMenuPane = menuPane;

    /* Create the Help menu */
    CreateHelpPulldownMenu(menuBar, NPlotHelp);

    XtManageChild(menuBar);
    
    /* Create the ntuple panel inside, set it to empty */
    CreateNTupleWindowContents(wmain, &window->widgets);
    setWindowEmpty(window, TRUE);

    /* Get rid of the buttons that are now represented in the menu */ 
    XtUnmanageChild(window->widgets.dismissBtn);
    XtUnmanageChild(window->widgets.helpBtn);
    
    /* Move the plot button to the center */
    XtVaSetValues(window->widgets.plotBtn, XmNleftPosition, 8,
    	    XmNrightPosition, 36, NULL);
    XtVaSetValues(window->widgets.createBtn, XmNleftPosition, 36,
    	    XmNrightPosition, 64, NULL);
    XtVaSetValues(window->widgets.modifyBtn, XmNleftPosition, 64,
    	    XmNrightPosition, 92, NULL);
    	    
    /* Update the Windows menus of all nplot panels with the new window */
    UpdateWindowsMenu();

    /* Display it all */
    XtRealizeWidget(appShell);

    /* Global GC */
    globalGC = XCreateGC(XtDisplay(appShell),
			 RootWindowOfScreen(XtScreen(appShell)), 0, NULL);

    /* Some info for future color scales */
    setMaxColorScaleColors(guessMaxColorScaleColors(
	display, XScreenNumberOfScreen(XtScreen(MainPanelW))));

    return window;
}

/*
** Reads a file.  On fail, displays error message dialog(s) and returns false
*/
static int readFile(char *name, char *path, ntupleWindow *window)
{
    char fullname[MAXPATHLEN];
    char line[MAX_LINE_LEN];
    hsNTuple *nTuple;
    int i, resp, nRows = 0, nCols = 0, minCols = INT_MAX, nHeadings = 0;
    int extraHeadings = FALSE, wrongNumberOfColumns = FALSE;
    FILE *fp = NULL;
    char firstChar, **names;
    float *data;
    static int curID = 1;

    /* Get the full name of the file */
    strcpy(fullname, path);
    strcat(fullname, name);

    /* Open the file */
    if ((fp = fopen(fullname, "r")) == NULL) { 
    	DialogF(DF_WARN, window->shell, 1, "Can't open %s:\n%s",
	    	"Acknowledged", fullname, strerror(errno));
	return FALSE;
    }
    
    /* read the file once to find out how much data is there */
    while (fgets(line, MAX_LINE_LEN, fp) != NULL) {
    	firstChar = line[strspn(line, " \t")];
    	if (firstChar == '>') {
    	    if (nHeadings != 0)
    	    	extraHeadings = TRUE;
    	    nHeadings = readHeadingLine(line, NULL, 0);
    	    if (nHeadings == 0) {
    	    	truncateErrorLine(line);
    	    	resp = DialogF(DF_WARN, window->shell, 2,
    	    		"Couldn't read column heading line in %s:\n%s", 
    	    		"Continue", "Cancel Open", name, line);
    	        if (resp == 2)
    	            return FALSE;
    	    }
    	} else if (firstChar != '#' && !isBlank(line)) {
    	    nCols = readDataLine(line, NULL, 0);
    	    if (nCols == 0) {
    	    	truncateErrorLine(line);
    	    	resp = DialogF(DF_WARN, window->shell, 2,
    	    		    "Couldn't read data line in %s:\n%s", 
    	    		    "Continue", "Cancel Open", name, line);
    	        if (resp == 2)
    	            return FALSE;
    	    } else {
    		if (nRows != 0 && nCols != minCols)
    	    	    wrongNumberOfColumns = TRUE;
    		minCols = nCols<minCols ? nCols : minCols;
    		nRows++;
    	    }
    	}
    }
    if (ferror(fp)) {
        DialogF(DF_ERR, window->shell, 1, "Error reading %s:\n%s",
        	"Acknowledged", fullname, strerror(errno));
        return FALSE;
    }
    if (nRows == 0) {
    	DialogF(DF_ERR, window->shell, 1,
    	 	"Error reading file %s:\nNo lines were read",
    	 	"Acknowledged", fullname);
    	return FALSE;
    }
    if (wrongNumberOfColumns)
    	DialogF(DF_WARN, window->shell, 1,
	   "Some rows in %s have different\nnumbers of values.  Used first %d",
	   "Continue", name, minCols);
    else if  (minCols != nHeadings && nHeadings != 0)
    	DialogF(DF_WARN, window->shell, 1,
    	   "Number of headings in %s is\ninconsistent with number of variables",
    	   "Continue", name);
    else if (extraHeadings)
    	DialogF(DF_WARN, window->shell, 1,
    		"Extra column heading lines in\n%s were ignored",
    		"Continue", name);

    /* Allocate enough memory to hold the data and the name array */
    data = (float *)XtMalloc(sizeof(float) * nRows * minCols);
    names = (char **)XtMalloc(sizeof(char *) * minCols);
    
    /* read the file again, this time store the data */
    rewind(fp);
    nRows = 0;
    while (fgets(line, MAX_LINE_LEN, fp) != NULL) {
    	firstChar = line[strspn(line, " \t")];
    	if (firstChar == '>') {
    	    readHeadingLine(line, names, minCols);
    	} else if (firstChar != '#' && !isBlank(line)) {
    	    if (readDataLine(line, &data[nRows*minCols], minCols) != 0)
    	    	nRows++;
    	}
    }
    if (ferror(fp)) {
        /* this is very unlikely given that we've already read the file once */
        DialogF(DF_ERR, window->shell, 1, "Error reading data from %s:\n%s",
        	"Acknowledged", fullname, strerror(errno));
        return FALSE;
    }
 
    /* make up column headings for unnamed variables */
    for (i=0; i<minCols; i++) {
    	if (i >= nHeadings) {
    	    names[i] = XtMalloc(strlen("column 999")+1);
    	    sprintf(names[i], "column %d", i+1);
    	}
    }
    
    /* build an hsNTuple data structure for the panel to display */
    nTuple = (hsNTuple *)XtMalloc(sizeof(hsNTuple));
    nTuple->type = HS_NTUPLE;
    nTuple->id = curID++;
    nTuple->uid = 0;
    nTuple->title = CopyString(name);
    nTuple->category = NULL;
    nTuple->hbookID = 0;
    nTuple->n = nRows;
    nTuple->nVariables = minCols;
    nTuple->names = names;
    nTuple->chunkSzData = nRows;
    nTuple->chunkSzExt = 0;
    nTuple->data = data;
    nTuple->extensions[0] = NULL; nTuple->extensions[1] = NULL;
    nTuple->extensions[2] = NULL; nTuple->extensions[3] = NULL;
    window->nTuple = nTuple;
    
    /* hand it off to the ntuple panel code to display */
    FillNTupleWindow(&window->widgets, nTuple);
    
    /* set the filename and window title and update the windows menu */
    strcpy(window->filename, name);
    strcpy(window->path, path);
    setWindowEmpty(window, False);
    XtVaSetValues(window->shell, XmNtitle, name, XmNiconName, name, NULL);
    UpdateWindowsMenu();
    return TRUE;
}

/*
** Read an nplot column heading line, return the number of headings in
** the line, and copy the first maxCols heading strings into the array headings.
** headings should be an array of char* and readHeadingLine will allocate memory
** for each string it copies, which the caller is responsible for freeing.
*/
static int readHeadingLine(char *line, char **headings, int maxCols)
{
    char *ptr;
    int len, nCols = 0;
    
    /* allocate a string for each column name and copy it from the line */
    ptr = strchr(line, '>') + 1;
    while (TRUE) {
    	ptr += strspn(ptr, Separators);
    	if (*ptr == '\0' || *ptr == '\n')
    	    break;
    	if (*ptr == '\"') {
    	    ptr++;
    	    len = strcspn(ptr, "\"");
    	    if (*(ptr+len) == '\0' || *(ptr+len) == '\n')
    	    	return 0; /* unmatched quotes */
    	    if (nCols < maxCols) {
    		headings[nCols] = XtMalloc(len + 1);
    		memcpy(headings[nCols], ptr, len);
    		(headings[nCols])[len] = '\0';
    	    }
    	    ptr++;
    	} else {
    	    len = strcspn(ptr, Separators);
    	    if (nCols < maxCols) {
    		headings[nCols] = XtMalloc(len + 1);
    		memcpy(headings[nCols], ptr, len);
    		(headings[nCols])[len] = '\0';
    	    }
    	}
    	ptr += len;
    	nCols++;
    }
    return nCols;
}

/*
** reads a line of floating point numbers and stores them in array vector.
** the entire line is read, but maxCols determines how many values are
** actually stored.  when maxCols == 0, no values are stored and the
** routine just counts the number of values on the line.  On error
** or empty line, the routine returns 0 for the number of values read.
*/
static int readDataLine(char *line, float *vector, int maxCols)
{
    double value;
    char *ptr, *endPtr;
    int nValues = 0;
    
    ptr = line;
    while (TRUE) {
    	ptr += strspn(ptr, Separators);
    	if (*ptr == '\0' || *ptr == '\n')
    	    break;
    	value = strtod(ptr, &endPtr);
    	if (endPtr == ptr)
    	    return 0;
    	ptr = endPtr;
    	if (nValues<maxCols)
    	    vector[nValues] = value;
    	nValues++;
    }
    return nValues;
}

/*
** truncates a string to a maximum of MAX_ERROR_LINE_LEN characters.  If it
** shortens the string, it appends "..." to show that it has been shortened.
** It assumes that the string that it is passed is writeable.
*/
static void truncateErrorLine(char *line)
{
    if (strlen(line) > MAX_ERROR_LINE_LEN)
	memcpy(&line[MAX_ERROR_LINE_LEN-3], "...", 4);
}

static int isBlank(char *line)
{
    line += strspn(line, Separators);
    return *line == '\n' || *line == '\0';
}

static void closeWindow(ntupleWindow *window)
{
    /* invalidate any plot windows displaying the data (stop them from using
       the data), and deallocate the ntuple and any extensions it might have */
    if (window->nTuple != NULL) {
    	InvalidateItem(window->nTuple->id);
    	CloseVariablePanelsWithItem(window->nTuple->id);
    	RemoveNTupleExtension(window->nTuple->id);
    	FreeItem((hsGeneral *)window->nTuple);
    }
    
    /* if this is the last window, don't remove it, just mark it as empty */
    if (NtuplePanelList->next == NULL) {
    	setWindowEmpty(window, TRUE);
    	UpdateWindowsMenu();
    	return;
    }
    
    /* remove and deallocate all of the widgets associated with window */
    XtDestroyWidget(window->shell);
    
    /* remove the window from the global window list */
    removeFromWindowList(window);
    
    /* deallocate the window data structures */
    XtFree((char *)window);
    	    
    /* Update the Windows menus of all nplot panels to remove window */
    UpdateWindowsMenu();
}

static ntupleWindow *findWindowWithFile(char *name, char *path)
{
    ntupleWindow *w;

    for (w=NtuplePanelList; w!=NULL; w=w->next) {
    	if (!w->empty && !strcmp(w->filename, name) && !strcmp(w->path, path)) {
	    return w;
	}
    }
    return NULL;
}

static void setWindowEmpty(ntupleWindow *window, int empty)
{
    XtSetSensitive(window->widgets.form, !empty);
    XtSetSensitive(window->closeItem, !empty);
    if (empty) {
    	ClearNTupleWindow(&window->widgets);
    	XtVaSetValues(window->shell,XmNtitle, "NPlot", XmNiconName, "NPlot", NULL);
	strcpy(window->filename, "");
	strcpy(window->path, "");
    }
    window->empty = empty;
}

/*
** Remove a window from the list of windows
*/
static void removeFromWindowList(ntupleWindow *window)
{
    ntupleWindow *temp;

    if (NtuplePanelList == window)
	NtuplePanelList = window->next;
    else {
	for (temp = NtuplePanelList; temp != NULL; temp = temp->next) {
	    if (temp->next == window) {
		temp->next = window->next;
		break;
	    }
	}
    }
}

static void closeCB(Widget w, ntupleWindow *wInfo, caddr_t callData)
{
    closeWindow(wInfo);
}

static void windowFrameCloseCB(Widget w, ntupleWindow *wInfo, caddr_t callData)
{
    if (NtuplePanelList->next == NULL)
    	exit(0);
    else
    	closeWindow(wInfo);
}

static void openCB(Widget w, ntupleWindow *wInfo, caddr_t callData)
{
    openFile(w);
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

static void exitCB(Widget w, ntupleWindow *wInfo, caddr_t callData)
{
    exit(0);
}

static void bufferGraphicsCB(Widget w, ntupleWindow *wInfo, caddr_t callData)
{
    BufferGraphics(XmToggleButtonGetState(w));
}

static void autoPlotHelpCB(Widget w, caddr_t clientData, caddr_t callData)
{
    PrefData.plotAutoHelp = XmToggleButtonGetState(w);
}

static void savePrefsCB(Widget w, ntupleWindow *wInfo, caddr_t callData)
{
    SaveHistoPreferences(wInfo->shell, NPLOT_PREF_FILE);
}

static void closeAllCB(Widget w, caddr_t clientData, caddr_t callData)
{
    CloseAllPlotWindows();
}

/*
** update the Windows menu of an nplot window
*/
static void updateOneWindowsMenu(ntupleWindow *window)
{
    Widget btn;
    WidgetList items = (WidgetList)0;
    int nItems=0, n=0, nNtupleNames=0, nNames;
    XtPointer userData;
    char **names;
    Window *windows;
    ntupleWindow *w;
    
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
    XtVaGetValues(window->windowMenuPane, XmNchildren, &items,
                  XmNnumChildren, &nItems, NULL);

    if (items != (WidgetList)0)
        for (n=0; n<nItems; n++)
        {
            XtVaGetValues(items[n], XmNuserData, &userData, NULL);
            if (userData == (XtPointer)WINDOW_NAME)
            {
                /* unmanaging before destroying stops parent from displaying */
                XtUnmanageChild(items[n]);
                XtDestroyWidget(items[n]);
            }
        }

    /* Count the open windows, dim the window menu if this is the only window */
    for (w=NtuplePanelList; w!=NULL; w=w->next)
    	nNtupleNames++;
    nNames = nNtupleNames + CountPlotWindows();
    if (nNames == 1) {
    	XtSetSensitive(window->windowMenuBtn, False);
    	return;
    } else
    	XtSetSensitive(window->windowMenuBtn, True);
    
    /* Get the current windows and their titles from plot and ntuple windows */
    names = (char **)XtMalloc(sizeof(char *) * nNames);
    windows = (Window *)XtMalloc(sizeof(Window *) * nNames);
    for (n=0, w=NtuplePanelList; w!=NULL; w=w->next, n++) {
    	XtVaGetValues(w->shell, XmNtitle, &names[n], NULL);
    	windows[n] = XtWindow(w->shell);
    }
    GetPlotWindowsAndTitles(&windows[nNtupleNames], &names[nNtupleNames]);
    
    /* Add the window titles to the windows menu */
    for (n=0; n<nNames; n++) {
    	btn = AddMenuItem(window->windowMenuPane, "win", names[n], 0, NULL,
    		"", (XtCallbackProc)raiseCB, (void *)windows[n]);
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
** Replacement for histoscope main panel routines GetMPItemByID,
** GetMPItemByUID, and GetMPItemByName.
*/
hsGeneral *GetMPItemByID(int id)
{
    ntupleWindow *w;

    for (w=NtuplePanelList; w!=NULL; w=w->next) {
    	if (w->nTuple != NULL && w->nTuple->id == id)
	    return (hsGeneral *)w->nTuple;
    }
    return NULL;
}
hsGeneral *GetMPItemByUID(char *category, int uid)
{
    return NULL;
}

hsGeneral *GetMPItemByName(char *category, char *name)
{
    ntupleWindow *w;

    for (w=NtuplePanelList; w!=NULL; w=w->next) {
    	if (w->nTuple != NULL && !(strcasecmp(w->nTuple->title, name)))
	    return (hsGeneral *)w->nTuple;
    }
    return NULL;
}

/* (This routine should really be in ntuplePanel.c, but because HistoScope
   and NPlot handle ntuple windows differently, it's different for each */
void UpdateNTuplePanelList(int id)
{
    ntupleWindow *w;

    for (w=NtuplePanelList; w!=NULL; w=w->next) {
    	if (w->nTuple != NULL && w->nTuple->id == id) {
    	    UpdateNTupleWindow(&w->widgets, w->nTuple);
    	    break;
    	}
    }
}
     
/*
** Replacement for histoscope UpdateWindowMenu routine, updates all
** nplot window's Windows menu.
*/
void UpdateWindowsMenu(void)
{
    ntupleWindow *w;
    
    for (w=NtuplePanelList; w!=NULL; w=w->next)
    	updateOneWindowsMenu(w);
}

/*
** Stubs for histoscope routines which do nothing in nplot
** but are important in histoscope
*/
void RequestUpdates(int id) {}
void RequestErrors(int id) {}
void EndUpdates(int id) {}
void SetUpdateFreq(int freq) {}
int GetUpdateFreq() {return 0;}
void ShowRange() {}
int LoadItemData() {return 0;}
int ViewItem() {return 0;}
void CloseMiniPlot() {}
void MakePlotADragSource() {}
void CloseMPlotFromWinfo() {}
void PrintMultiPlotWin() {}
Boolean InqMiniTitleState() {return 0;}
void SetNoMultPlotLabels() {}
void SetLabelsOnMultPlot() {}
void AddNtupleToMultiPlot() {}
void AddHistToMultiPlot() {}
void CreateEmptyMultPlot() {}
void RegisterWidgetAsPlotReceiver() {}
void GetMPItemByHbookID() {}
void GetStatsForMultiPlot() {}
void ReFillMultiPlot() {}
void ScheduleDataArrivedCallback() {}
void CloseMPlotWindowById() {}
void GenerateMPlotPSById() {}
void MPlotRefreshCycle() {}
void CloseAllMultWindows() {}
void SetBufferGraphicsBtnState() {}
void SetAutoHelpBtnState() {}
void RedisplayAllColoredWindows() {}
void ReportTaskCompletion() {}
void SetMplotTitlePSFont() {}
void SetMplotTitlePSFontSize() {}
void SetMplotTitlePSOnOrOff() {}
const char *GetMplotTitlePSFont() {return NULL;}
int GetMplotTitlePSFontSize() {return 0;}
Boolean GetMplotTitlePSOnOrOff(void) {return 0;}
