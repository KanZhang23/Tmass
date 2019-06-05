/*******************************************************************************
*									       *
* ntuplePanel.c -- Histoscope ntuple window, and NPlot main window	       *
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
* July 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/DrawingA.h>
#include "../util/misc.h"
#include "../util/help.h"
#include "../util/DialogF.h"
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "mainMenu.h"
#include "plotWindows.h"
#include "ntuplePanel.h"
#include "mainPanel.h"
#include "interpret.h"
#include "variablePanel.h"
#include "help.h"

/* These symbols set the widget spacing within the ntuple panel. */
#define MARGIN_HEIGHT 6
#define FIELD_MARGIN 8
#define FIELD_SPACING 0
#define LIST_LEFT_POS 2
#define LIST_RIGHT_POS 36
#define FIELD_LEFT_POS 37
#define FIELD_RIGHT_POS 98
#define FIELD_WIDTH (FIELD_RIGHT_POS-FIELD_LEFT_POS)
#define BUTTON_SEPARATION 2
/* relationships to allow for different sized borders between motif widgets */
#define LABEL_BUTTON_OFFSET 4
#define LABEL_MENU_OFFSET 7
#define DEFAULT_BUTTON_OFFSET 0
/* number of fields initially shown in the window (user resizes to show more) */
#define N_INITIAL_FIELDS 7
enum fieldTypeEnum {
    REQ1, REQ2, REQ3, REQ4, REQ5, REQ6, REQ7, REQ8, REQ9, REQ10,
    REQ11, REQ12, REQ13, REQ14, REQ15, REQ16, REQ17, REQ18, REQ19, REQ20,
    REQ21, REQ22, REQ23, REQ24, REQ25, REQ26, REQ27, REQ28, REQ29, REQ30,
    REQ31, REQ32, REQ33, REQ34, REQ35, REQ36,
    OPT1, OPT2, OPT3, OPT4, OPT5, OPT6, OPT7, OPT8, OPT9, OPT10,
    OPT11, OPT12, OPT13, OPT14, OPT15, OPT16, OPT17, OPT18, OPT19, OPT20,
    OPT21, OPT22, OPT23, OPT24, OPT25, OPT26, OPT27, OPT28, OPT29, OPT30,
    OPT31, OPT32, OPT33, OPT34, OPT35, OPT36,
    ERR, SLI, NONE};

typedef struct _ntWindow {
    struct _ntWindow *next;	/* pointer to next in list */
    Widget shell;		/* application shell for window */
    ntWidgets widgets;		/* widgets for controling panel */
} ntWindow;

typedef windowInfo *(*plotProcedure)(Widget parent, hsGeneral *item, int *ntVars,
	                 int *sliderVars, char *title, char *winID, Widget shell,
	                 int isMiniPlot, colorScaleInfo *csInfo,
                         widgetConfigInfo *confInfo);

typedef struct _plotType {
    char *name;			/* name of plot for menu */
    int nRequired;		/* number of required arguments */
    int varArgs;		/* accepts a variable # of args */
    plotProcedure plotProc;	/* procedure for creating plot window */
    char *vNames[N_FIELDS];	/* field names for panel */
    char fieldTypes[N_FIELDS];	/* types for each field REQ, OPT, ERR, NONE */
} plotTypeRec;

/* Function Prototypes */
static void closeNTWindow(ntWindow *wInfo);
static void closeCB(Widget w, ntWindow *wInfo, caddr_t callData);
static void assignCB(Widget w, ntWidgets *widgets, caddr_t callData);
static void plotCB(Widget w, ntWidgets *widgets, caddr_t callData);
static void createCB(Widget w, ntWidgets *widgets, caddr_t callData);
static void modifyCB(Widget w, ntWidgets *widgets, caddr_t callData);
static void typeMenuCB(Widget w, ntWidgets *widgets, caddr_t callData);
static void varListCB(Widget w, ntWidgets *widgets,
	XmListCallbackStruct *cbStruct);
static void listSelCB(Widget w, ntWidgets *widgets,
	XmListCallbackStruct *cbStruct);
static void resizeCB(Widget w, ntWidgets *widgets, caddr_t callData);
static void removeFromNTWindowList(ntWindow *window);
static char *createNTupleWindowTitle(int plotType, hsNTuple *ntuple, int *vars);

/* List of all displayed ntuple windows */
static ntWindow *NTWindowList = NULL;

/* Frequently used XmString consts (initialized in CreateNTupleWindowConts) */
static XmString EmptyField = (XmString)0, ClearString = (XmString)0;

static plotTypeRec PlotTypes[N_PLOT_TYPES] = {
    {"Time Series Plot", 1, True, CreateTSPlotWindow,
     {"V1", "V2", "V3", "V4", "V5", "V6", "V7", "V8", "V9", "V10", 
      "V11", "V12", "V13", "V14", "V15", "V16", "V17", "V18", "V19", "V20",
      "V21", "V22", "V23", "V24", "V25", "V26", "V27", "V28", "V29", "V30",
      "V31", "V32", "V33", "V34", "V35", "V36"},
     {REQ1, OPT2, OPT3, OPT4, OPT5, OPT6, OPT7, OPT8, OPT9, OPT10,
      OPT11, OPT12 ,OPT13, OPT14, OPT15, OPT16, OPT17, OPT18, OPT19, OPT20,
      OPT21, OPT22 ,OPT23, OPT24, OPT25, OPT26, OPT27, OPT28, OPT29, OPT30,
      OPT31, OPT32 ,OPT33, OPT34, OPT35, OPT36}},
    {"TS Plot w/Errors", 1, True, CreateTSPlotErrWindow,
     {"V1","E1+","E1-","V2","E2+","E2-","V3","E3+","E3-","V4","E4+","E4-",
      "V5","E5+","E5-","V6","E6+","E6-","V7","E7+","E7-","V8","E8+","E8-",
      "V9","E9+","E9-","V10","E10+","E10-","V11","E11+","E11-","V12","E12+","E12-"},
     {REQ1, ERR,  ERR, OPT2, ERR,  ERR, OPT3, ERR,  ERR, OPT4, ERR, ERR,
      OPT5, ERR,  ERR, OPT6, ERR,  ERR, OPT7, ERR,  ERR, OPT8, ERR, ERR, 
      OPT9, ERR,  ERR, OPT10, ERR,  ERR, OPT11, ERR,  ERR, OPT12, ERR, ERR}},
    {"XY Plot", 2, True, CreateXYPlotWindow,
     {"X1", "Y1", "X2", "Y2", "X3", "Y3", "X4", "Y4", "X5", "Y5", "X6", "Y6",
      "X7", "Y7", "X8", "Y8", "X9", "Y9", "X10","Y10","X11","Y11","X12","Y12",
      "X13","Y13","X14","Y14","X15","Y15","X16","Y16","X17","Y17","X18","Y18"},
     {REQ1, REQ1, OPT2, OPT2, OPT3, OPT3, OPT4, OPT4, OPT5, OPT5, OPT6, OPT6,
      OPT7, OPT7, OPT8, OPT8, OPT9, OPT9,OPT10,OPT10,OPT11,OPT11,OPT12,OPT12,
      OPT13,OPT13,OPT14,OPT14,OPT15,OPT15,OPT16,OPT16,OPT17,OPT17,OPT18,OPT18}},
    {"XY Plot w/Errors", 2, True, CreateXYPlotErrWindow,
     {"X1", "Y1", "V1+", "V1-", "H1+", "H1-",
      "X2", "Y2", "V2+", "V2-", "H2+", "H2-",
      "X3", "Y3", "V3+", "V3-", "H3+", "H3-",
      "X4", "Y4", "V4+", "V4-", "H4+", "H4-",
      "X5", "Y5", "V5+", "V5-", "H5+", "H5-",
      "X6", "Y6", "V6+", "V6-", "H6+", "H6-"},
     {REQ1, REQ1, ERR,   ERR,   ERR,   ERR,
      OPT2, OPT2, ERR,   ERR,   ERR,   ERR,
      OPT3, OPT3, ERR,   ERR,   ERR,   ERR,
      OPT4, OPT4, ERR,   ERR,   ERR,   ERR,
      OPT5, OPT5, ERR,   ERR,   ERR,   ERR,
      OPT6, OPT6, ERR,   ERR,   ERR,   ERR}},
    {"Sorted XY Plot", 2, True, CreateXYSortWindow,
     {"X", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10",
      "Y11", "Y12", "Y13", "Y14", "Y15", "Y16", "Y17", "Y18", "Y19", "Y20",
      "Y21", "Y22", "Y23", "Y24", "Y25", "Y26", "Y27", "Y28", "Y29", "Y30",
      "Y31", "Y32", "Y33", "Y34", "Y35"},
     {REQ1,REQ1, OPT2, OPT3, OPT4, OPT5, OPT6, OPT7, OPT8, OPT9, OPT10,
      OPT11,OPT12,OPT13,OPT14,OPT15,OPT16,OPT17,OPT18,OPT19,OPT20,
      OPT21,OPT22,OPT23,OPT24,OPT25,OPT26,OPT27,OPT28,OPT29,OPT30,
      OPT31,OPT32,OPT33,OPT34,OPT35}},
    {"Sorted XY w/Errors", 2, True, CreateXYSortErrWindow,
     {"X", "Y1", "E1+", "E1-", "Y2", "E2+", "E2-", "Y3", "E3+", "E3-",
      "Y4", "E4+", "E4-", "Y5", "E5+", "E5-", "Y6", "E6+", "E6-", 
      "Y7", "E7+", "E7-", "Y8", "E8+", "E8-", "Y9", "E9+", "E9-", 
      "Y10", "E10+", "E10-", "Y11", "E11+", "E11-", NULL, NULL},
     {REQ1,REQ1, ERR,   ERR,   OPT2, ERR,   ERR,   OPT3, ERR,   ERR,   OPT4,
      ERR,   ERR,   OPT5, ERR,   ERR,  OPT6, ERR,   ERR,
      OPT7, ERR,   ERR, OPT8, ERR,   ERR, OPT9, ERR,   ERR, 
      OPT10, ERR,   ERR, OPT11, ERR,   ERR, NONE, NONE}},
    {"XY Scatter Plot", 2, False, Create2DScatWindow,
     {"X", "Y", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10",
      "S11","S12","S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34"},
     {REQ1,REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"XYZ Scatter Plot", 3, False, Create3DScatWindow,
     {"X", "Y", "Z","S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11",
      "S12","S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33"},
     {REQ1,REQ1,REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"Histogram", 1, False, Create1DHistWindow,
     {"X", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11","S12","S13",
      "S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34","S35"},
     {REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,  SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"2D Histogram", 2, False, Create2DHistWindow,
     {"X", "Y", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11","S12",
      "S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34"},
     {REQ1,REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,  SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"Adaptive Histogram", 1, False, Create1DAdaptHistWindow,
     {"X", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11","S12","S13",
      "S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34","S35"},
     {REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,  SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"2D Adaptive Histogram", 2, False, Create2DAdaptHistWindow,
     {"X", "Y", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11","S12",
      "S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34"},
     {REQ1,REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,  SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"2D Cell Plot", 2, False, CreateCellWindow,
     {"X", "Y", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11","S12",
      "S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34"},
     {REQ1,REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,  SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"Color XY Scatter Plot", 3, False, CreateColor2DScatWindow,
     {"X", "Y","Cl","S1","S2","S3","S4","S5","S6","S7","S8","S9","S10",
      "S11","S12","S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33"},
     {REQ1,REQ1,REQ1, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}},
    {"Color Cell Plot", 2, False, CreateColorCellWindow,
     {"X", "Y", "S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11","S12",
      "S13","S14","S15","S16","S17","S18","S19","S20",
      "S21","S22","S23","S24","S25","S26","S27","S28","S29","S30",
      "S31","S32","S33","S34"},
     {REQ1,REQ1,SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI, SLI,  SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,SLI,
      SLI,SLI,SLI,SLI,SLI,SLI}}
};

void CreateNTupleWindow(Display *display, hsNTuple *item, char *geometry)
{
    Widget appShell;
    ntWindow *ntWin;

    /* If an ntuple panel already exists for this item, just pop it back to
       the top and return */
    for (ntWin=NTWindowList; ntWin!=NULL; ntWin=ntWin->next) {
    	if (ntWin->widgets.id == item->id) {
    	    XMapRaised(display, XtWindow(ntWin->shell));
    	    return;
    	}
    }	    
    
    /* Allocate an ntWindow structure to hold information about the window,
       and add it to the list of plot windows */
    ntWin = (ntWindow *)XtMalloc(sizeof(ntWindow));
    ntWin->next = NTWindowList;
    NTWindowList = ntWin;

    /* Create an toplevel shell to hold the window */
    appShell = XtVaAppCreateShell ("nTuple", "NTuple",
	    applicationShellWidgetClass, display,
	    XmNgeometry, geometry,
	    XmNtitle, item->title,
	    XmNiconName, item->title,
	    XmNallowShellResize, False, NULL);
    ntWin->shell = appShell;
    
    /* Create the window contents */
    CreateNTupleWindowContents(appShell, &ntWin->widgets);
    FillNTupleWindow(&ntWin->widgets, item);
    
    /* Add close callbacks to the dismiss button and Motif window menu */
    XtAddCallback(ntWin->widgets.dismissBtn, XmNactivateCallback,
    		  (XtCallbackProc)closeCB, (caddr_t)ntWin);
    AddMotifCloseCallback(appShell, (XtCallbackProc)closeCB, ntWin);
    
    /* Display it */
    XtRealizeWidget(appShell);
}

void CreateNTupleWindowContents(Widget parent, ntWidgets *widgets)
{
    Widget drawArea, form, listLabel, list, typeMenu;
    Widget dismissBtn, plotBtn, helpBtn, plotTypeOptMenu, fieldForm;
    Widget button, nameLabel, fieldLabel, topWidget, typeMenuItemSel;
    Widget modifyBtn, createBtn;
    Arg args[50];
    int ac, topOffset, i;
    XmString s1, *st1;
    
    /* initialize the XmString constants used throughout the module */
    if (EmptyField == (XmString)0) {
    	EmptyField = XmStringCreateSimple("--");
    	ClearString = XmStringCreateSimple("<Clear Entry>");
    }
    
    /* form to hold everything */
    form = XtVaCreateManagedWidget("ntForm", xmFormWidgetClass, parent,
    		XmNmarginHeight, MARGIN_HEIGHT, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
 
    /* Add the help callback */
    XtAddCallback(form, XmNhelpCallback, (XtCallbackProc)NtuplePanelHelpCB,
    	    NULL);

    /* the purpose of this drawing are widget is simply to get a resize call
       back to add and remove fields from the window */
    drawArea = XtVaCreateManagedWidget("ntda", xmDrawingAreaWidgetClass, form,
    		XmNwidth, 1,
    		XmNtopAttachment, XmATTACH_FORM,
    		XmNleftAttachment, XmATTACH_FORM,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_FORM,
    		XmNleftOffset, 1,
    		XmNrightPosition, LIST_LEFT_POS, NULL);
    XtAddCallback(drawArea, XmNresizeCallback,
    	        (XtCallbackProc)resizeCB, (caddr_t)widgets);
    
    ac = 0;
    typeMenu = XmCreatePulldownMenu(form, "plotType", args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNspacing, 0); ac++;
    XtSetArg(args[ac], XmNmarginWidth, 0); ac++;
    XtSetArg(args[ac], XmNresizeWidth, False); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNtopOffset, MARGIN_HEIGHT); ac++;
    XtSetArg(args[ac], XmNrightPosition, FIELD_RIGHT_POS); ac++;
    XtSetArg(args[ac], XmNsubMenuId, typeMenu); ac++;
    plotTypeOptMenu = XmCreateOptionMenu(form, "plotTypeMenu", args, ac);
    XtManageChild(plotTypeOptMenu);
 

    /* plot type menu items */
    for (i=0; i<N_PLOT_TYPES; i++) {
    	button = XtVaCreateManagedWidget("typeBtn", xmPushButtonWidgetClass, typeMenu,
    		XmNlabelString, s1=XmStringCreateSimple(PlotTypes[i].name),
                XmNuserData, (XtPointer)((long)i), NULL);
	XmStringFree(s1);
    	XtAddCallback(button, XmNactivateCallback, (XtCallbackProc)typeMenuCB,
    		      (caddr_t)widgets);
    	if (i == 0)
    	    typeMenuItemSel = button;
    }

    XtVaCreateManagedWidget("typeLabel", xmLabelWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Plot Type"),
    		XmNtopAttachment, XmATTACH_FORM,
    		XmNrightAttachment, XmATTACH_WIDGET,
    		XmNrightWidget, plotTypeOptMenu,
    		XmNrightOffset, 0,
    		XmNtopOffset, MARGIN_HEIGHT + LABEL_MENU_OFFSET, NULL);
    XmStringFree(s1);

    fieldForm = XtVaCreateManagedWidget("fieldForm", xmFormWidgetClass, form,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopWidget, plotTypeOptMenu,
    		XmNtopOffset, FIELD_MARGIN,
    		XmNleftPosition, FIELD_LEFT_POS,
    		XmNrightPosition, FIELD_RIGHT_POS, NULL);
    		
    topWidget = NULL;
    topOffset = 0;
    for (i=0; i<N_FIELDS; i++) {
    	button = XtVaCreateWidget("aBtn", xmPushButtonWidgetClass,fieldForm,
    		XmNlabelString, s1=XmStringCreateSimple("Assign To ----"),
    		XmNrecomputeSize, False,
                XmNuserData, (XtPointer)((long)i),
    		XmNtopAttachment, topWidget==NULL?XmATTACH_FORM:XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_FORM,
    		XmNtopWidget, topWidget,
    		XmNtopOffset, topOffset, NULL);
    	XmStringFree(s1);
    	XtAddCallback(button, XmNactivateCallback, (XtCallbackProc)assignCB,
    		      (caddr_t)widgets);
    	nameLabel = XtVaCreateWidget("nLbl", xmLabelWidgetClass, fieldForm,
    		XmNlabelString, s1=XmStringCreateSimple("----:"),
    		XmNrecomputeSize, False,
    		XmNalignment, XmALIGNMENT_END,
    		XmNtopAttachment, topWidget==NULL?XmATTACH_FORM:XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, topWidget,
    		XmNleftWidget, button,
    		XmNtopOffset, topOffset + LABEL_BUTTON_OFFSET,
    		XmNleftOffset, 0, NULL);
    	XmStringFree(s1);
    	fieldLabel = XtVaCreateWidget("fLbl", xmLabelWidgetClass, fieldForm,
    		XmNlabelString, EmptyField,
    		XmNalignment, XmALIGNMENT_BEGINNING,
    		XmNtopAttachment, topWidget==NULL?XmATTACH_FORM:XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNrightAttachment, XmATTACH_FORM,
    		XmNtopWidget, topWidget,
    		XmNtopOffset, topOffset + LABEL_BUTTON_OFFSET,
    		XmNleftWidget, nameLabel,
    		XmNleftOffset, 2, NULL);

    	if (i < N_INITIAL_FIELDS) {
    	    XtManageChild(button);
    	    XtManageChild(nameLabel);
    	    XtManageChild(fieldLabel);
    	}
    	widgets->assignBtns[i] = button;
    	widgets->fieldLabels[i] = nameLabel;
    	widgets->fields[i] = fieldLabel;
    	topWidget = button;
    	topOffset = FIELD_SPACING;
    }
    
    /* Create the buttons at the bottom of the dialog */
    plotBtn = XtVaCreateManagedWidget("pBtn", xmPushButtonWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Plot"),
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNshowAsDefault, True,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_NONE,
    		XmNtopWidget, fieldForm,
    		XmNtopOffset, FIELD_MARGIN,
    		XmNleftPosition, 2,
    		XmNrightPosition, 16, NULL);
    XmStringFree(s1);
    XtAddCallback(plotBtn, XmNactivateCallback, (XtCallbackProc)plotCB,
    		  (caddr_t)widgets);
    createBtn = XtVaCreateManagedWidget("cBtn", xmPushButtonWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Create Variable"),
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_NONE,
    		XmNtopWidget, fieldForm,
    		XmNtopOffset, FIELD_MARGIN,
    		XmNleftPosition, 16,
    		XmNrightPosition, 42, NULL);
    XmStringFree(s1);
    XtAddCallback(createBtn, XmNactivateCallback, (XtCallbackProc)createCB,
    		  (caddr_t)widgets);
    modifyBtn = XtVaCreateManagedWidget("mBtn", xmPushButtonWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Modify Variable"),
    		XmNsensitive, False,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_NONE,
    		XmNtopWidget, fieldForm,
    		XmNtopOffset, FIELD_MARGIN,
    		XmNleftPosition, 42,
    		XmNrightPosition, 68, NULL);
    XmStringFree(s1);
    XtAddCallback(modifyBtn, XmNactivateCallback, (XtCallbackProc)modifyCB,
    		  (caddr_t)widgets);
    dismissBtn = XtVaCreateManagedWidget("dBtn", xmPushButtonWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopWidget, fieldForm,
    		XmNtopOffset, FIELD_MARGIN + DEFAULT_BUTTON_OFFSET,
    		XmNleftPosition, 68,
    		XmNrightPosition, 84, NULL);
    XmStringFree(s1);
    helpBtn = XtVaCreateManagedWidget("hBtn", xmPushButtonWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Help"),
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopWidget, fieldForm,
    		XmNtopOffset, FIELD_MARGIN + DEFAULT_BUTTON_OFFSET,
    		XmNleftPosition, 84,
    		XmNrightPosition, 98, NULL);
    XtAddCallback(helpBtn, XmNactivateCallback,
    		(XtCallbackProc)NtuplePanelHelpCB, NULL);
    XmStringFree(s1);

    /* Create the variable list */
    listLabel = XtVaCreateManagedWidget("listLabel", xmLabelWidgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Variables"),
    		XmNtopAttachment, XmATTACH_FORM,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNtopOffset, MARGIN_HEIGHT,
    		XmNleftPosition, LIST_LEFT_POS, NULL);
    XmStringFree(s1);
    ac = 0;
    XtSetArg(args[ac], XmNitems,
    	     (st1=StringTable("(No data to display)    "))); ac++;
    XtSetArg(args[ac], XmNitemCount, 1); ac++;
    XtSetArg(args[ac], XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
    XtSetArg(args[ac], XmNselectionPolicy, XmBROWSE_SELECT); ac++;
    XtSetArg(args[ac], XmNlistSizePolicy, XmCONSTANT); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNtopWidget, listLabel); ac++;
    XtSetArg(args[ac], XmNtopOffset, 0); ac++;
    XtSetArg(args[ac], XmNleftPosition, LIST_LEFT_POS); ac++;
    XtSetArg(args[ac], XmNrightPosition, LIST_RIGHT_POS); ac++;
    XtSetArg(args[ac], XmNbottomOffset, FIELD_MARGIN); ac++;
    XtSetArg(args[ac], XmNbottomWidget, plotBtn); ac++;
    list = XmCreateScrolledList(form, "variableList", args, ac);
    XtAddCallback(list, XmNdefaultActionCallback, (XtCallbackProc)varListCB,
    	    (caddr_t)widgets);
    XtAddCallback(list, XmNbrowseSelectionCallback, (XtCallbackProc)listSelCB,
    	    (caddr_t)widgets);
    FreeStringTable(st1);
    XtManageChild(list);
    
    /* make plot button default button */
    XtVaSetValues(form, XmNdefaultButton, plotBtn, NULL);
    
    /* fill in the ntWidgets data structures so our callbacks can function */
    widgets->form = form;
    widgets->list = list;
    widgets->plotBtn = plotBtn;
    widgets->createBtn = createBtn;
    widgets->modifyBtn = modifyBtn;
    widgets->dismissBtn = dismissBtn;
    widgets->helpBtn = helpBtn;
    widgets->typeMenu = plotTypeOptMenu;
    widgets->initialHeight = 0;
    widgets->plotType = 0;
    
    /* assign the proper strings to the buttons and field labels */
    typeMenuCB(typeMenuItemSel, widgets, (caddr_t)NULL);
}

void FillNTupleWindow(ntWidgets *widgets, hsNTuple *item)
{
    /* Fill in the ntuple list */
    UpdateNTupleWindow(widgets, item);
    
    /* save the ntuple id to use when the user decides to plot something */
    widgets->id = item->id;
}

/*
** Update the ntuple panel "widgets" for ntuple "item" for changes to
** the variable list
*/
void UpdateNTupleWindow(ntWidgets *widgets, hsNTuple *item)
{
    int i, nVars, nStrings, field;
    XmString *stringTable, labelString;
    ntupleExtension *ntExt = GetNTupleExtension(item->id);
    
    /* Build a string table to supply to the list widget */
    nVars = item->nVariables + (ntExt != NULL ? ntExt->nVars : 0);
    nStrings = nVars + 1;
    stringTable = (XmString *)XtMalloc((nStrings+1) * sizeof(XmString));
    stringTable[0] = XmStringCopy(ClearString);
    for (i=0; i<nVars; i++)
    	stringTable[i+1] = XmStringCreateSimple(ExtNTVarName(item, i));
    stringTable[i+1] = (XmString)0; /* mark end of table with 0 */
    
    /* Display the items in the variable list */
    XtVaSetValues(widgets->list, XmNitems, stringTable,
    		  XmNitemCount, nStrings, NULL);
    /* Motif doesn't reset the selection when items are changed */
    XmListDeselectAllItems(widgets->list);

    /* Remove obsolete variables from the assignment fields */
    for (field=0; field<N_FIELDS; field++) {
	XtVaGetValues(widgets->fields[field], XmNlabelString, &labelString, NULL);
    	if (!XmStringCompare(labelString, EmptyField)) {
    	    for (i=1; i<nStrings; i++)
    		if (XmStringCompare(labelString, stringTable[i]))
    	    	    break;
    	    if (i == nStrings)
    		XtVaSetValues(widgets->fields[field], XmNlabelString,
    		    	EmptyField, NULL);
    	}
    	XmStringFree(labelString); /* GetValues on labelString does a copy */
    }
    FreeStringTable(stringTable);
}

void ClearNTupleWindow(ntWidgets *widgets)
{
    XmString *st1;
    int i;
    
    XtVaSetValues(widgets->list,
    		  XmNitems, st1=StringTable(" "), XmNitemCount, 1, NULL);
    FreeStringTable(st1);
    XmListDeselectAllItems(widgets->list);
    for (i=0; i<N_FIELDS; i++)
	XtVaSetValues(widgets->fields[i], XmNlabelString, EmptyField, NULL);
}

/*
** Count the number of ntuple windows displayed
*/
int CountNtupleWindows(void)
{
    ntWindow *w;
    int n = 0;
    
    for (w=NTWindowList; w!=NULL; w=w->next)
    	n++;
    return n;
}

/*
** Get the names and top level X windows for all of the ntuple windows
** currently displayed (so a windows menu can be created for them).  Arrays
** of length CountNtupleWindows() to hold the string pointers and window
** ids must be provided by the caller.
*/
void GetNtupleWindowsAndTitles(Window *windows, char **titles)
{
    ntWindow *w;
    int i;
    
    for (i=0, w=NTWindowList; w!=NULL; w=w->next, i++) {
    	XtVaGetValues(w->shell, XmNtitle, &titles[i], NULL);
    	windows[i] = XtWindow(w->shell);
    }
}

/*
** Find NTuple window with id matching "id" and update the variable list
*/
void FindAndUpdateNTuplePanelList(int id)
{
    ntWindow *w;
    int i;
    
    for (i=0, w=NTWindowList; w!=NULL; w=w->next, i++) {
    	if (w->widgets.id == id) {
    	    UpdateNTupleWindow(&w->widgets, (hsNTuple *)GetMPItemByID(id));
    	    break;
    	}
    }
}

void InvalidateNTupleWindow(ntWidgets *widgets)
{
    int i;
    Pixel foreground, background;
    Pixmap pixmap;
    
    /* create dotted outline around invalidated window */
    XtVaGetValues(widgets->form, XmNforeground, &foreground,
    		  XmNbackground, &background, NULL);
    pixmap = XmGetPixmap(XtScreen(widgets->form),
	    	"invalidWindowShadow", foreground, background);
    XtVaSetValues(widgets->form, XmNtopShadowPixmap, pixmap,
    		  XmNbottomShadowPixmap, pixmap, NULL);

    /* dim out everything but dismiss button */
    for (i=0; i<N_FIELDS; i++) {
	XtVaSetValues(widgets->fields[i], XmNlabelString, EmptyField, NULL);
	XtSetSensitive(widgets->fields[i], False);
	XtSetSensitive(widgets->assignBtns[i], False);
	XtSetSensitive(widgets->fieldLabels[i], False);
    }
    XtSetSensitive(widgets->plotBtn, False);
    XtSetSensitive(widgets->typeMenu, False);
    XtSetSensitive(widgets->createBtn, False);
    XtSetSensitive(widgets->modifyBtn, False);
    XtSetSensitive(widgets->list, False);
    CloseVariablePanelsWithItem(widgets->id);
}

void InvalidateAllNTupleWindows(void)
{
    ntWindow *w;

    for (w=NTWindowList; w!=NULL; w=w->next)
    	InvalidateNTupleWindow(&w->widgets);
}

/*
** The main panel needs to renumber item ids when it opens a new file,
** this updates the ntuple window list when an id changes
*/
void ChangeNtuplePanelItemID(int oldID, int newID)
{
    ntWindow *w;

    for (w=NTWindowList; w!=NULL; w=w->next)
    	if (w->widgets.id == oldID) w->widgets.id = newID;
}

/*
** Determine whether an ntuple panel displays this item
*/
int NtupleIsDisplayed(int id)
{
    ntWindow *w;

    for (w=NTWindowList; w!=NULL; w=w->next)
    	if (w->widgets.id == id) return True;
    return False;
}

void InvalidateNTuple(int id)
{
    ntWindow *w;

    for (w=NTWindowList; w!=NULL; w=w->next) {
    	if (w->widgets.id == id)
    	    InvalidateNTupleWindow(&w->widgets);
    }
}

void CloseAllNTupleWindows(void)
{
    while (NTWindowList != NULL)
    	closeNTWindow(NTWindowList);
}

/*
** Put up a mini-plot from an NTuple, given the ntuple item "ntuple", and a 
** list of the ntuple variable indecies to match the plot variables and slider
** variables in "varList" and "sliderList".  XtFree *ntupleTitle when finished
** with its string contents (see createNTupleWindowTitle).  Note this routine
** does not position the plot within the multi-plot, it merely creates it.
*/
windowInfo *ViewMiniNtuplePlot(Widget parent, hsNTuple *nTuple, int plotType, 
		int *varList, int *sliderList, char **errMsg, Widget appShell,
		char **ntupleTitle, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    plotTypeRec *plotStyle = &PlotTypes[plotType];
    windowInfo *wInfo;
    int i;

    /* Check variable list to ensure that all required arguments are supplied */
    for (i=0; i<plotStyle->nRequired; i++) {
    	if (varList[i] == -1) {
    	    *errMsg = "Required plot variable not supplied";
    	    return NULL;
    	}
    }

    /* Create a plot window and plot the variables */
    *ntupleTitle = createNTupleWindowTitle(plotType, nTuple, varList);
    wInfo = (*plotStyle->plotProc)(parent, (hsGeneral *)nTuple, varList, sliderList, 
    			*ntupleTitle, NULL, appShell, True, csInfo, confInfo);
    return wInfo;
}

int NCurvesInPlot(int plotType, int *vars)
{
    plotTypeRec *plotStyle = &PlotTypes[plotType];
    int i, fieldType, haveCurve[N_FIELDS], nCurves = 0;
    
    /* Only look at the plot types which create curves */
    if (!(plotType == TSPLOT || plotType == TSPLOTERR || plotType == XYPLOT ||
    	    plotType == XYPLOTERR || plotType == XYSORT ||
    	    plotType == XYSORTERR))
    	return 0;
    
    /* The fieldType entry in the PlotTypes table codes which curve in a multi-
       curve plot (like a time series plot or XY plot) a field represents.  Use
       that information to mark the haveCurve array as having data */
    for (i=0; i<N_FIELDS; i++)
    	haveCurve[i] = False;
    for (i=0; i<N_FIELDS; i++) {	
    	if (vars[i] != -1) {
    	    fieldType = plotStyle->fieldTypes[i];
    	    if (fieldType >= REQ1 && fieldType < REQ1+N_FIELDS)
    	    	 haveCurve[fieldType - REQ1] = True;
    	    if (fieldType >= OPT1 && fieldType < OPT1+N_FIELDS)
    	    	 haveCurve[fieldType - OPT1] = True;
    	}
    }
    
    /* Return the number of curves that were marked as having data supplied */
    for (i=0; i<N_FIELDS; i++)
    	if (haveCurve[i])
    	    nCurves++;
    return nCurves;
}

int PlotIsHist(int plotType)
{
    return plotType == HIST1D || plotType == ADAPTHIST1D;
}
static void closeNTWindow(ntWindow *wInfo)
{
    removeFromNTWindowList(wInfo);
    /* destroy all of the widgets from the application shell down */
    XtDestroyWidget(wInfo->shell);
    XtFree((XtPointer) wInfo);
    UpdateWindowsMenu();
}

static void closeCB(Widget w, ntWindow *wInfo, caddr_t callData)
{
    closeNTWindow(wInfo);
}

static void assignCB(Widget w, ntWidgets *widgets, caddr_t callData)
{
    XmString *selections, fieldStr;
    int nSelected, fieldNum;
    XtPointer userData;
   
    /* get the variable from the variable list corresponding to the
       selection in the list widget */
    XtVaGetValues(widgets->list, XmNselectedItemCount, &nSelected,
    		  XmNselectedItems, &selections, NULL);
    if (nSelected == 0) {
    	DialogF(DF_INF, w, 1,
		"Please select a variable to\nassign from the list on the left",
		"Acknowledged");
    	return;
    }
    
    /* set the field corresponding to this button to the selected string */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    fieldNum = (long)userData;
    if (XmStringCompare(selections[0], ClearString))
    	fieldStr = EmptyField;
    else
    	fieldStr = selections[0];
    XtVaSetValues(widgets->fields[fieldNum], XmNlabelString, fieldStr, NULL);
}

static void plotCB(Widget w, ntWidgets *widgets, caddr_t callData)
{
    int nVars, field, var, varList[N_FIELDS], sliderList[N_FIELDS];
    int nSliders = 0;
    XmString *listConts, labelString;
    hsGeneral *nTuple;
    plotTypeRec *plotStyle = &PlotTypes[widgets->plotType];
    char *errMsg;
    
    /* Construct arrays, varList and sliderList, of the index numbers of
       the ntuple requested for this plot, or warn the user that he
       hasn't the all of the required ones */
    for (field=0; field<N_FIELDS; field++) {
    	varList[field] = -1;
    	sliderList[field] = -1;
    }
    XtVaGetValues(widgets->list, XmNitemCount, &nVars, XmNitems, &listConts, NULL);
    for (field=0; field<N_FIELDS; field++) {
	XtVaGetValues(widgets->fields[field], XmNlabelString, &labelString, NULL);
    	if (XmStringCompare(labelString, EmptyField)) {
    	    if (field < plotStyle->nRequired) {
    	    	DialogF(DF_WARN, w, 1, "%ss require\nat least %d variable(s)",
    	    		"Acknowledged", plotStyle->name, plotStyle->nRequired);
    	    	XmStringFree(labelString);
    	    	return;
    	    }
    	} else {
    	    for (var=1; var<nVars; var++) {
    		if (XmStringCompare(labelString, listConts[var])) {
    	    	    if (plotStyle->fieldTypes[field] == SLI)
    	    	    	sliderList[nSliders++] = var-1;
    	    	    else
    	    		varList[field] = var-1;
    	    	    break;
    		}
    	    }
    	    if (var == nVars)
    		fprintf(stderr,
    	    	    "Internal Error: can't read value in field %d\n", field);
    	}
    	XmStringFree(labelString);
    }
    
    /* Put up the plot (ignore errors from ViewNtuplePlot since we've
       checked over varList already */
    nTuple = GetMPItemByID(widgets->id);
    ViewNtuplePlot(XtDisplay(w), (hsNTuple *)nTuple, NULL, widgets->plotType,
		   varList, sliderList, NULL, NULL, NULL, &errMsg);
}

static void createCB(Widget w, ntWidgets *widgets, caddr_t callData)
{
    CreateVariablePanel(widgets->form, (hsNTuple *)GetMPItemByID(widgets->id),
    	    NULL);
}

static void modifyCB(Widget w, ntWidgets *widgets, caddr_t callData)
{
    int *posList, posCount, extIndex;
    hsNTuple *ntuple;
    ntupleExtension *ntExt;
    
    /* Get the ntuple variable index of selected item in the list */
    if (!XmListGetSelectedPos(widgets->list, &posList, &posCount))
    	return;
    ntuple = (hsNTuple *)GetMPItemByID(widgets->id);
    extIndex = posList[0] - 2 - ntuple->nVariables;
    XtFree((char *)posList);
    if (extIndex < 0)
    	return;
    
    /* Put up a variable panel to edit the expression */
    ntExt = GetNTupleExtension(ntuple->id);
    if (ntExt == NULL) {
    	fprintf(stderr, "Internal error: ntuple extension missing");
    	return;
    }
    CreateVariablePanel(widgets->form, ntuple, ntExt->vars[extIndex].name);
}

/*
** Put up a plot from an NTuple, given the ntuple item "ntuple", and a list
** of the ntuple variable indecies to match the plot variables and slider
** variables in "varList" and "sliderList"
*/
windowInfo *ViewNtuplePlot(Display *display, hsNTuple *nTuple, char *winID,
    int plotType, int *varList, int *sliderList, char *geometry,
    colorScaleInfo *csInfo, widgetConfigInfo *confInfo, char **errMsg)
{
    plotTypeRec *plotStyle = &PlotTypes[plotType];
    char *title;
    windowInfo *wInfo;
    int i;
    Widget appShell;

    /* Check variable list to ensure that all required arguments are supplied */
    for (i=0; i<plotStyle->nRequired; i++) {
    	if (varList[i] == -1) {
    	    *errMsg = "Required plot variable not supplied";
    	    return NULL;
    	}
    }

    /* Create a plot window and plot the variables */
    title = createNTupleWindowTitle(plotType, (hsNTuple *)nTuple, varList);
    appShell = XtVaAppCreateShell ("plotShell", "PlotShell",
	    applicationShellWidgetClass, display,
	    XmNgeometry, geometry,
	    XmNtitle, title,
	    XmNiconName, title, NULL);
    wInfo = (*plotStyle->plotProc)(appShell, (hsGeneral *)nTuple, varList,
	    sliderList, title, winID, appShell, False, csInfo, confInfo);
    XtFree(title);
    UpdateWindowsMenu();
    return wInfo;
}

static void typeMenuCB(Widget w, ntWidgets *widgets, caddr_t callData)
{
    XmString s1;
    int i, plotType;
    char *fieldName, assignString[15], labelString[5];
    XtPointer userData;
    
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    plotType = (long)userData;
    if (plotType < 0 || plotType > N_PLOT_TYPES) {
    	fprintf(stderr, "Internal Error: invalid plot type in typeMenuCB\n");
    	return;
    }
    widgets->plotType = plotType;
    
    for (i=0; i<N_FIELDS; i++) {
    	fieldName = PlotTypes[plotType].vNames[i];
    	if (fieldName == NULL) {
    	    fieldName = "--";
    	    XtSetSensitive(widgets->assignBtns[i], False);
    	    XtSetSensitive(widgets->fieldLabels[i], False);
    	    XtSetSensitive(widgets->fields[i], False);
    	} else {
    	    XtSetSensitive(widgets->assignBtns[i], True);
    	    XtSetSensitive(widgets->fieldLabels[i], True);
    	    XtSetSensitive(widgets->fields[i], True);
    	}
    	sprintf(assignString, "Assign to %s", fieldName);
    	sprintf(labelString, "%s:", fieldName);
    	XtVaSetValues(widgets->assignBtns[i], XmNlabelString,
    		      s1=XmStringCreateSimple(assignString), NULL);
    	XmStringFree(s1);
    	XtVaSetValues(widgets->fieldLabels[i], XmNlabelString,
    		      s1=XmStringCreateSimple(labelString), NULL);
    	XmStringFree(s1);
    }
}

static void listSelCB(Widget w, ntWidgets *widgets,
	XmListCallbackStruct *cbStruct)
{
    XtSetSensitive(widgets->modifyBtn, cbStruct->item_position-2 >=
    	    ((hsNTuple *)GetMPItemByID(widgets->id))->nVariables);
}

static void varListCB(Widget w, ntWidgets *widgets,
		      XmListCallbackStruct *cbStruct)
{
    int field;
    XmString labelString;
    
    /* The panel's default button should take priority over the default action
       for the list (this routine) and not execute when the return or enter key
       is pressed.  The proper way to remove the default action on these key
       presses is to change the translation table for the list widget, but I
       don't want to spend my life making one stupid button work right, so
       this piece of code simply skips the routine if it was activated
       because of a key press.  (double clicking the space bar will still
       do the action because that works on a KeyRelease event instead) */
    if (cbStruct->event->type == KeyPress)
    	return;
        
    /* if the item is the "clear entry" item, clear the last non-empty field */
    if (cbStruct->item_position == 1) {
    	for (field=N_FIELDS-1; field>=0; field--) {
    	    XtVaGetValues(widgets->fields[field],XmNlabelString,&labelString, NULL);
    	    if (!XmStringCompare(labelString, EmptyField)) {
    		SimulateButtonPress(widgets->assignBtns[field]);
    		XmStringFree(labelString);
    		return;
    	    }
    	    XmStringFree(labelString);
    	}
    	XBell(XtDisplay(w), 100);	/* beep if they're all already empty */
    }
    
    /* find an empty field and store the variable from the list there */
    for (field=0; field<N_FIELDS; field++) {
    	if (!XtIsSensitive(widgets->fields[field]))
    	    break;
	XtVaGetValues(widgets->fields[field], XmNlabelString, &labelString, NULL);
    	if (XmStringCompare(labelString, EmptyField)) {
    	    SimulateButtonPress(widgets->assignBtns[field]);
    	    XmStringFree(labelString);
    	    return;
    	}
    	XmStringFree(labelString);
    }
    XBell(XtDisplay(w), 100);	/* beep if they're all full */
}

static void resizeCB(Widget w, ntWidgets *widgets, caddr_t callData)
{
    short btnHeight, formHeight;
    int i, fieldHeight, noFieldsHeight, nFields, nWidgets;
    Widget widgetList[N_FIELDS*3];
    
    /* if the initial height of the drawArea widget has not yet been recorded,
       do so.  This depends on the resizing being done correctly the first time
       which I hope is a reasonable assumption.  If not, we're in trouble */
    if (widgets->initialHeight == 0) {
    	XtVaGetValues(w, XmNheight, &widgets->initialHeight, NULL);
    	return;
    }
    
    /* determine how many fields should be visible */
    XtVaGetValues(w, XmNheight, &formHeight, NULL);
    XtVaGetValues(widgets->assignBtns[0], XmNheight, &btnHeight, NULL);
    fieldHeight = btnHeight + FIELD_SPACING;
    noFieldsHeight = widgets->initialHeight - N_INITIAL_FIELDS*fieldHeight;
    nFields = (int)floor((formHeight - noFieldsHeight) / (float)fieldHeight);
    if (nFields < 1)
    	nFields = 1;
    else if (nFields > N_FIELDS)
    	nFields = N_FIELDS;

    /* make the desired fields visible or invisible by managing or unmanaging */
    nWidgets = 0;
    for (i=0; i<nFields; i++) {
        widgetList[nWidgets++] = widgets->assignBtns[i];
        widgetList[nWidgets++] = widgets->fieldLabels[i];
        widgetList[nWidgets++] = widgets->fields[i];
    }
    XtManageChildren(widgetList, nWidgets);
    nWidgets = 0;
    for (i=nFields; i<N_FIELDS; i++) {
        widgetList[nWidgets++] = widgets->assignBtns[i];
        widgetList[nWidgets++] = widgets->fieldLabels[i];
        widgetList[nWidgets++] = widgets->fields[i];
    }
    XtUnmanageChildren(widgetList, nWidgets);
}

/*
** Remove a window from the list of windows
*/
static void removeFromNTWindowList(ntWindow *window)
{
    ntWindow *temp;

    if (NTWindowList == window)
	NTWindowList = window->next;
    else {
	for (temp = NTWindowList; temp != NULL; temp = temp->next) {
	    if (temp->next == window) {
		temp->next = window->next;
		break;
	    }
	}
    }
}

/*
** Generate a title for an ntuple plot window.  The caller is responsible
** for freeing the returned string
*/
static char *createNTupleWindowTitle(int plotType, hsNTuple *ntuple, int *vars)
{
    int i, nArgs = 0, totLen = 0;
    int nRequired = PlotTypes[plotType].nRequired;
    char *title, *name, *strings[N_FIELDS];
    
    /* collect, count, and measure the argument strings */
    for (i=0; i<nRequired; i++) {
    	name = ExtNTVarName(ntuple, vars[i]);
    	strings[nArgs++] = name;
    	totLen += strlen(name);
    }
    if (PlotTypes[plotType].varArgs) {
    	for (i=PlotTypes[plotType].nRequired; i<N_FIELDS; i++) {
    	    if (vars[i] != -1 && PlotTypes[plotType].fieldTypes[i] != ERR) {
    	    	name = ExtNTVarName(ntuple, vars[i]);
    	    	strings[nArgs++] = name;
    	    	totLen += strlen(name);
    	    }
    	}
    }
    
    /* allocate space for the title string based on the number of arguments
       and the total length */
    title = XtMalloc(totLen + 6*nArgs + strlen(" ... ()") +
    		      strlen(ntuple->title));
    
    /* generate the string */
    if (nArgs == 1)
    	sprintf(title,"%s (%s)", strings[0], ntuple->title);
    else if (nArgs == 2 && nRequired == 2)
    	sprintf(title,"%s vs. %s (%s)", strings[1], strings[0], ntuple->title);
    else if (nArgs == 2)
    	sprintf(title,"%s and %s (%s)", strings[0], strings[1], ntuple->title);
    else if (nArgs == 3 && nRequired == 3)
    	sprintf(title,"%s vs. %s vs. %s (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    else if (nArgs == 3 && nRequired == 2)
    	sprintf(title,"%s vs. %s and %s (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    else if (nArgs == 3)
    	sprintf(title,"%s, %s, and %s (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    else if (nArgs > 3 && nRequired > 3)
    	sprintf(title,"%s vs. %s vs. %s vs. ... (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    else if (nArgs > 3 && nRequired == 3)
    	sprintf(title,"%s vs. %s vs. %s, ... (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    else if (nArgs > 3 && nRequired == 2)
    	sprintf(title,"%s vs. %s, %s, ... (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    else
    	sprintf(title,"%s, %s, %s, ... (%s)", strings[0], strings[1],
    		strings[2], ntuple->title);
    
    /* limit the title to a certain length */
    if (strlen(title) > HS_MAX_WINDOW_TITLE_LENGTH)
    	title[HS_MAX_WINDOW_TITLE_LENGTH] = '\0';
    
    return title;
}
