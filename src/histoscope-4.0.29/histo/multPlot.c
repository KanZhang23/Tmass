/*******************************************************************************
*                                        				       *
* multPlot.c -- Multiple plot code for Nirvana Histoscope tool		       *
*									       *
* Copyright (c) 1995 Universities Research Association, Inc.		       *
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
* April 10, 1995							       *
*									       *
* Written by Joy Kyriakopulos						       *
* 									       *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/errno.h>
#include <math.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "histoP.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "histoMainProgram.h"
#include "ntuplePanel.h"
#include "dragPlot.h"
#include "../util/DialogF.h"
#include "../util/help.h"
#include "../util/misc.h"
#include "../util/psUtils.h"
#include "../plot_widgets/Scat.h"
#include "../plot_widgets/H1D.h"
#include "../plot_widgets/XY.h"
#include "../plot_widgets/Cell.h"
#include "../plot_widgets/3DScat.h"
#include "../plot_widgets/2DHist.h"
#include "configFile.h"
#include "multPlot.h"
#include "communications.h"
#include "mainMenu.h"

#define WINDOW_TITLE_MARGIN 10		/* separation between window title
					   and plots on PS output */
/* Global Variables */
multWindow *MultWindowList = NULL;	/* Listhead for multi-window structs */

/* Module-wide global variables */
static multWindow * PtrToReturn = NULL;
static Widget WinTitleText;
static Widget NumRowsText;
static Widget NumColsText;
static Widget CancelBtn;
static Widget ResetBtn;
static Widget CreateWinBtn;
static Widget HelpBtn;
static Widget Form = NULL;
static Widget CurCatText;
static Boolean OneCat = False;
static int NumRows;
static int NumCols;
static int DoneWithDialog = 0;
static int *IdList = NULL;
static int NumIdsInCat = 0;
static char CurrentCat[HS_MAX_CATEGORY_LENGTH+1] = "";
static char CatString[HS_MAX_CATEGORY_LENGTH+21] = "";
static char *HelpText = "\nThe Create Multiple Plot Window \
allows you to specify to Histo-Scope the parameters for \
creating a plot window with more than one plot.  There are two ways \
these multiple-plot windows can be populated with plots:\n\n\
1)  All the 1-D and 2-D Histograms in the current category (as listed \
in the Histo-Scope Main Window) can be displayed in a single multiple \
plot window by pressing the \"Use Current Category\" pushbutton.  \
After checking that the default window title and number of rows and columns \
is what you want, and changing them if they\'re not, press the \
\"Create Window\" button to create the multiple \
plot window with these histograms. \n\n\
2) For more flexibility, don\'t press the \"Use Current Category\" button.  \
Instead just type the window title and number of rows \
and columns for the Multiple Plot window, and after pressing \"Create Window\" \
use the mouse to drag the plots one-by-one into the initially empty \
multi-plot window.  \
This method gives you the most flexibility for choosing and placing plots \
for display.   Each plot comes up in its initial state - that is the state you \
would see when selecting the plot for viewing from the Main Panel.  Once the \
plot is viewed, however, all the interactive controls and plot settings are \
available similar to regular plots.\n\n\
The Create Window pushbutton verifies the input parameters and creates the \
Multiple Plot window according to your specifications.  If you have used \
a category but reduced the default number of rows or columns so that \
there are more histograms than will fit in the window, Histo-Scope will \
print a message and ask whether to continue or cancel the Create Window.  \
If you elect to continue, the remaining histograms will not be displayed.\n\n\
The Reset button erases all input fields in the dialog thereby allowing you \
to start afresh in specifying the parameters for the Multiple Plot Window.\n\n\
The Cancel button dismisses the dialog without any action.  This will not \
create a Multiple Plot window.\n\n\
To change the size of the individual plots in Multiple Plot windows, use \
Motif window controls such as the window sides or corners to make the entire \
window larger or smaller.\n\n\
Each mini-plot in the Multiple Plot window allows you to adjust its plot \
settings in a manner similar to the individual plot windows.  To bring up a \
Plot Settings menu, point to the plot with the mouse and press the right \
mouse button.\n\n\
To close a Multiple Plot window, use the bar-shaped Motif window menu at the \
top left corner and select Close, or select Close from the plot settings \
menu.\n\
";

/* Prototypes for local routines */
static void createMultPDialog(Widget parent);
static void reInitDialog(int done);
static void closeMplotWindow(multWindow *w);
static void removeFromMPWindowList(multWindow *w);
static void cancelCB(Widget w, caddr_t clientData, caddr_t callData);
static void resetCB(Widget w, caddr_t clientData, caddr_t callData); 
static void helpCB(Widget w, caddr_t clientData, caddr_t callData); 
static void createWinCB(Widget w, caddr_t clientData, caddr_t callData); 
static void setCategoryCB(Widget w, caddr_t clientData, caddr_t callData); 
static void closeCB(Widget w, multWindow *mW, caddr_t callData);
static void trimLeadNtrailWhite(char *string);
static void printMiniPlot(FILE *psFile, int x, int y, int width, int height,
	Widget plotWidget, Widget labelWidget);
static void setMenuToggle(Widget w, int buttonState, int notify);
static void closeMini(windowInfo **wInfoP, Widget *labelP, Widget *plotWidgetP);
static void destroyPlotCB(XtPointer client_data, XtIntervalId *id);
static void executeMPlotReadyCallbacks(multWindow *mp);
static void freeUpWinMemory(windowInfo *wInfo);

/* Variables related to miniplot title fonts in PostScript files */
static char *mplotTitlePSFont = NULL;
static int mplotTitlePSFontSize = 12;
static Boolean printMplotTitleInPS = True;

void SetMplotTitlePSFont(const char *font)
{
    if (font) {
	if (mplotTitlePSFont)
	    free(mplotTitlePSFont);
	mplotTitlePSFont = strdup(font);
    }
}
void SetMplotTitlePSFontSize(int size)
{
    if (size > 0)
	mplotTitlePSFontSize = size;
}
void SetMplotTitlePSOnOrOff(Boolean state)
{
    printMplotTitleInPS = state;
}
const char *GetMplotTitlePSFont(void)
{
    static char defaultFont[] = "Times-Roman-ISOLatin1";
    if (mplotTitlePSFont)
	return mplotTitlePSFont;
    else
	return defaultFont;
}
int GetMplotTitlePSFontSize(void)
{
    return mplotTitlePSFontSize;
}
Boolean GetMplotTitlePSOnOrOff(void)
{
    return printMplotTitleInPS;
}

/*
** Create Multiple Plot Window - Put Up MultiPlot Dialog & Create Multiple
**                               Plot Window from the user's specifications.
**
**                               Note:  Dialog is NOT modal, so other parts
**                                      of Histo-Scope will be executing while
**                                      the Multi-Plot Dialog is up.
*/
multWindow *CreateMultPlotWin(void)
{
    multWindow *ptrToReturn;
    
    if (Form == NULL) {
    	createMultPDialog(MainPanelW);	/* Create multiple plot dialog    */
    }
    XtManageChild(Form);		/* Pop it up,                     */
    DoneWithDialog = 0;
    
    while (DoneWithDialog == 0)		/* Wait for user input & callbacks */
        XtAppProcessEvent(XtWidgetToApplicationContext(Form), XtIMAll);
    
    ptrToReturn = PtrToReturn;		/* PtrToReturn set by createWinCB */
    XtUnmanageChild(Form);
    if (ptrToReturn != NULL)
    	XtRealizeWidget(ptrToReturn->appShell);
    reInitDialog(1);			/* Re-Initialize dialog & globals  */
    return(ptrToReturn);
}

multWindow *CreateMultPlotFromList(Widget parent, char *windowTitle, char *winID,
                int numRows, int numCols, int numPlots, int *ids, int *errsDisp)
{
    multWindow *multPlot = NULL;
    int i, j, n, dispErrs;
    hsGeneral *item;
    
    multPlot = CreateEmptyMultPlot(parent, windowTitle, winID, numRows, numCols, NULL);
    if (multPlot == NULL)
    	return NULL;
    
    for (i = 0, n = 0; i < numRows; ++i) {
    	for (j = 0; j < numCols; ++j) {
    	    item = GetMPItemByID(ids[n]);
    	    dispErrs = errsDisp != NULL ? errsDisp[n] : NO_ERROR_BARS;
	    if (!LoadItemData(parent, item))
    		continue;
    	    /* ensure item hasn't been deleted or changed by a client program */
    	    while ((item = GetMPItemByID(ids[n])) == NULL 
    	    	    || (item->type != HS_1D_HISTOGRAM 
    	    	    &&  item->type != HS_2D_HISTOGRAM))
    	    	if (++n >= numPlots)
    	    	    return multPlot;
    	    AddHistToMultiPlot(multPlot, item, i, j, NULL, False, dispErrs, NULL, NULL);
    	    if (++n >= numPlots)
    	    	return multPlot;
    	}
    }
    return multPlot;
}
 
multWindow *CreateEmptyMultPlot(Widget parent, char *windowTitle, char *winID,
        int numRows, int numCols, char *geometry)
{
    int i, j;
    multWindow *mplotInfo;
    float fNumCols = (float) numCols, fNumRows = (float) numRows;
    float fCol, fRow;
    
    /* Create structures for multiple plot window information */
    mplotInfo = (multWindow *) XtMalloc(sizeof(multWindow));
    mplotInfo->plot = (multiPlot *) XtMalloc(sizeof(multiPlot) 
    						 * numRows * numCols);
    if (winID)
    {
	strncpy(mplotInfo->windowID, winID, MAX_WINDOWID_LEN - 1);
	mplotInfo->windowID[MAX_WINDOWID_LEN - 1]  = '\0';
    }
    else
	mplotInfo->windowID[0] = '\0';
    mplotInfo->numRows = numRows;
    mplotInfo->numCols = numCols;
    mplotInfo->dispLabels = True;	      /* by default, display titles */
    mplotInfo->readyCB = NULL;
    
    /* Create a toplevel shell to hold the window */
    mplotInfo->appShell = XtVaAppCreateShell ("multPlotShell", "MultPlotShell",
    	    applicationShellWidgetClass, XtDisplay(parent),
	    XmNtitle, windowTitle,
	    XmNiconName, windowTitle, NULL);
    if (geometry)
    {
	strncpy(mplotInfo->geometry, geometry, 31);
	mplotInfo->geometry[31] = '\0';
	XtVaSetValues(mplotInfo->appShell,
		      XmNgeometry, mplotInfo->geometry, NULL);
    }

    /* set up closeCB to be called when the user selects close from the
       window frame menu */
    AddMotifCloseCallback(mplotInfo->appShell, (XtCallbackProc) closeCB,
    	mplotInfo);

    /* Create the form onto which everything goes */
    mplotInfo->multWidget = XmCreateForm(mplotInfo->appShell, "form", NULL, 0);
    
    /* Divide form into frame widgets: numRows x numCols.
       Initialize pointers to plotWidget and windowInfo structures. 
       Allow frame widget to receive a dragged plot */
    for (i = 0; i < numRows; ++i) {
    	fRow = i;
    	for (j = 0; j < numCols; ++j) {
    	    fCol = j;
    	    mplotInfo->plot[i*numCols+j].frame = 
    	    	XtVaCreateManagedWidget("frame",
    	    	xmFrameWidgetClass, mplotInfo->multWidget, 
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNleftPosition, (int)(fCol*(100./fNumCols)),
    		XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    		XmNtopPosition, (int)(fRow*(100./fNumRows)),
    		XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    		NULL);
    	    XtManageChild(mplotInfo->plot[i*numCols+j].frame);
    	    RegisterWidgetAsPlotReceiver(mplotInfo->plot[i*numCols+j].frame);
    	    mplotInfo->plot[i*numCols+j].plotWidget = NULL;
    	    mplotInfo->plot[i*numCols+j].wInfo = NULL;
    	    mplotInfo->plot[i*numCols+j].label = NULL;
    	}
    }
    XtManageChild(mplotInfo->multWidget);
    /* Add this multiplot to list of multiplots created. */
    mplotInfo->next = MultWindowList;
    MultWindowList = mplotInfo;
    
    return mplotInfo;			/* return multWindow pointer */
}

/*
** AddNtupleToMultiPlot - Add an ntuple mini-plot to an already-created
**                        Multi-Plot window.
**         Arguments:
**                      w       - MultWindow ptr ret'nd from CreateEmptyMultPlot
**                      ntuple  - hsNTuple pointer to the Histo Ntuple
**                      row     - row #, counting from 0 to numRows-1
**                      col     - column #, counting from 0 to numCols-1
**
**         Returns:	windowInfo pointer from ViewMiniNtuplePlot,
**				   or NULL if error
*/
windowInfo *AddNtupleToMultiPlot(multWindow *w, hsNTuple *item, int row,
		int col, int plotType, int *varList, int *sliderList,
		colorScaleInfo *csInfo, widgetConfigInfo *confInfo, char **errMsg)
{
    XmString s1;
    char *ntupleTitle = NULL;
    float fCol = (float)col, fRow = (float)row;
    float fNumCols = (float)w->numCols, fNumRows = (float)w->numRows;
    
    
    /* Ensure the multiple plot window info pointer isn't NULL */
    if (w == NULL) {
	fprintf(stderr, 
	    "Error calling AddNtupleToMultiPlot.  MultWindow == NULL\n");
	return NULL;
    }
    
    /* Ensure row and column are within range */
    if (row < 0 || row >= w->numRows) {
	fprintf(stderr, 
	    "Error calling AddNtupleToMultiPlot.  Row given: %d. #Rows = %d.\n",
	    row, w->numRows);
	return NULL;
    }
    
    if (col < 0 || col >= w->numCols) {
	fprintf(stderr, 
	  "Error calling AddNtupleToMultiPlot.  Column given: %d. #Cols = %d.\n",
	   col, w->numCols);
	return NULL;
    }
	    
    /* Create the Mini-Ntuple */
    if (item->type == HS_NTUPLE)
    	w->plot[row*w->numCols+col].wInfo = ViewMiniNtuplePlot(w->multWidget, 
    		item, plotType, varList, sliderList, errMsg, w->appShell, 
    		&ntupleTitle, csInfo, confInfo);
    else {
    	fprintf(stderr,
    	      "Error calling AddNtupleToMultiPlot:  Id %d is not an ntuple.\n", 
              item->id);
    	return NULL;
    }
    
    /* Unmanage the frame widget that visually defines an empty multiplot to 
       the user & do not allow any plot to be dropped into it */
    UnRegisterWidgetAsPlotReceiver(w->plot[row*w->numCols+col].frame);
    XtUnmanageChild(w->plot[row*w->numCols+col].frame);
    w->plot[row*w->numCols+col].label = XtVaCreateWidget("label",
    	       xmLabelWidgetClass, w->multWidget, 
    	       XmNlabelString,s1=XmStringCreateSimple(ntupleTitle),
    	       XmNtopAttachment, XmATTACH_POSITION,
    	       XmNleftAttachment, XmATTACH_POSITION,
    	       XmNrightAttachment, XmATTACH_POSITION,
    	       XmNleftPosition, (int)(fCol*(100./fNumCols)),
    	       XmNrightPosition,(int)((fCol+1.)*(100./fNumCols)),
    	       XmNtopPosition, (int)(fRow*(100./fNumRows)),
    	       NULL);
    XmStringFree(s1);
    if (ntupleTitle != NULL) {
    	XtFree(ntupleTitle);
    	ntupleTitle = NULL;
    }
    
    /* Position the plot, and its label too if not disabled by the user */
    if (w->dispLabels) {
    	XtManageChild(w->plot[row*w->numCols+col].label);
    	XtVaSetValues(w->plot[row*w->numCols+col].wInfo->widget,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNleftPosition, (int)(fCol*(100./fNumCols)),
    		XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    		XmNtopWidget, w->plot[row*w->numCols+col].label,
    		XmNbottomPosition, (int)((fRow+1.)*(100./fNumRows)),
    		XmNtraversalOn, False,
    		NULL);
    }
    else
    	XtVaSetValues(w->plot[row*w->numCols+col].wInfo->widget,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNleftPosition, (int)(fCol*(100./fNumCols)),
    		XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    		XmNtopPosition, (int)(fRow*(100./fNumRows)),
    		XmNbottomPosition, (int)((fRow+1.)*(100./fNumRows)),
    		XmNtraversalOn, False,
    		NULL);
    
    /* Save & manage the plot widget and return the Window Info pointer */
    w->plot[row*w->numCols+col].plotWidget =
    		w->plot[row*w->numCols+col].wInfo->widget;
    XtManageChild(w->plot[row*w->numCols+col].plotWidget);
    return w->plot[row*w->numCols+col].wInfo;
} 

/*
** AddHistToMultiPlot - Add a histogram mini-plot to an already-created
**                      Multi-Plot window.  AddHistToMultiPlot is used ONLY
**                      to add a mini-plot to a previously empty frame.
**         Arguments:
**                      mPlot    - Widget returned from CreateEmptyMultPlot
**                      item     - hsGeneral pointer to the Histo Item
**                      row      - row #, counting from 0 to numRows-1
**                      col      - column #, counting from 0 to numCols-1
**                      ntupleSourceWinfo
**                      headingType - plot type as in configFile.h
**                      dispErrs - value for wInfo->errorBars (display errors)
**				   should be == 0 for ntuples.
**
**         Returns:	windowInfo pointer from Create*DHistWindow,
**				   or NULL if error
*/
windowInfo *AddHistToMultiPlot(multWindow *w, hsGeneral *item, int row, int col,
			       windowInfo *ntupleSourceWinfo, int headingType,
			       int dispErrs, colorScaleInfo *csInfo,
                               widgetConfigInfo *confInfo)
{
    XmString s1;
    char *ntupleTitle = NULL, *errMsg = NULL;
    int needErrs = 0, hasData = 0;
    float fCol = (float)col, fRow = (float)row;
    float fNumCols = (float)w->numCols, fNumRows = (float)w->numRows;
    
    /* Ensure the multiple plot window info pointer isn't NULL */
    if (w == NULL) {
	fprintf(stderr, 
	    "Error calling AddHistToMultiPlot.  MultWindow == NULL\n");
	return NULL;
    }
    
    /* Ensure row and column are within range */
    if (row < 0 || row >= w->numRows) {
	fprintf(stderr, 
	    "Error calling AddHistToMultiPlot.  Row given: %d. #Rows = %d.\n",
	    row, w->numRows);
	return NULL;
    }
    
    if (col < 0 || col >= w->numCols) {
	fprintf(stderr, 
	  "Error calling AddHistToMultiPlot.  Column given: %d. #Cols = %d.\n",
	   col, w->numCols);
	return NULL;
    }
	    
    /* Create the Mini-Histogram */
    if (item->type == HS_1D_HISTOGRAM) {
	w->plot[row*w->numCols+col].wInfo = 
		Create1DHistWindow(w->multWidget, item, 
		NULL, NULL, item->title, NULL, w->appShell,
		True, csInfo, confInfo);
    	needErrs = ((hs1DHist *)item)->pErrs == NULL;
    	hasData = ((hs1DHist *)item)->bins != NULL;
    }
    else if (item->type == HS_2D_HISTOGRAM)
    	if (headingType == CELL_HEADING)
    	    w->plot[row*w->numCols+col].wInfo = 
    		CreateCellWindow(w->multWidget, item, NULL, NULL,
    		item->title, NULL, w->appShell, True, csInfo, confInfo);
	else if (headingType == COLORCELL_HEADING)
    	    w->plot[row*w->numCols+col].wInfo = 
    		CreateColorCellWindow(w->multWidget, item, NULL, NULL,
    		item->title, NULL, w->appShell, True, csInfo, confInfo);	    
    	else {
    	    w->plot[row*w->numCols+col].wInfo = 
    		Create2DHistWindow(w->multWidget, item, 
    		NULL, NULL, item->title, NULL, w->appShell,
                True, csInfo, confInfo);
    	    needErrs = ((hs2DHist *)item)->pErrs == NULL;
    	    hasData = ((hs2DHist *)item)->bins != NULL;
    	}
    else if (item->type == HS_NTUPLE && ntupleSourceWinfo != NULL)
    	w->plot[row*w->numCols+col].wInfo = /*... temporary fix */
    		ViewMiniNtuplePlot(w->multWidget, (hsNTuple *)item, 
    		ntupleSourceWinfo->pInfo[0]->plotType,
    		ntupleSourceWinfo->pInfo[0]->ntVars,
    		ntupleSourceWinfo->pInfo[0]->sliderVars, &errMsg, w->appShell, 
    		&ntupleTitle, csInfo, confInfo);
    else {
    	fprintf(stderr,
    	      "Error calling AddHistToMultiPlot:  Id %d is not a histogram.\n", 
              item->id);
    	return NULL;
    }
    
    /* Handle all cases of displaying error bars: coming from dragPlot.c
     * where histogram has been dragged to new multi-plot window, or coming
     * from mainPanel code, or from CreateMultPlotWin where a histogram,
     * perhaps from a client, is to be displayed, or from configFile.c.
     */ 
    w->plot[row*w->numCols+col].wInfo->pInfo[0]->errorBars = dispErrs;
    /* printf("AddHistToMultiPlot: title: %s, dispErrs = %d, needErrs = %d\n\
        row/col = %d/%d\n", item->title, dispErrs, needErrs, row, col); */
    if (needErrs)
	RequestErrors(item->id);	   /* Request errors from client */
    if (dispErrs == DATA_ERROR_BARS) {
    	setMenuToggle(w->plot[row*w->numCols+col].wInfo->errorDataMenuItem, 
    		True, False); 
    	if (hasData && !needErrs)
    	    RedisplayPlotWindow(w->plot[row*w->numCols+col].wInfo, REINIT);
    } else if (dispErrs == GAUSSIAN_ERROR_BARS) {
    	setMenuToggle(w->plot[row*w->numCols+col].wInfo->gaussErrorMenuItem, 
    		True, False); 
    	if (hasData && !needErrs)
    	    RedisplayPlotWindow(w->plot[row*w->numCols+col].wInfo, REINIT);
    }
    
    /* Unmanage the frame widget that visually defines an empty multiplot to 
       the user & do not allow any plot to be dropped into it */
    UnRegisterWidgetAsPlotReceiver(w->plot[row*w->numCols+col].frame);
    XtUnmanageChild(w->plot[row*w->numCols+col].frame);
    w->plot[row*w->numCols+col].label = XtVaCreateWidget("label",
    	    	xmLabelWidgetClass, w->multWidget, 
    	    	XmNlabelString,s1=XmStringCreateSimple(
    	    		item->type == HS_NTUPLE ? ntupleTitle : item->title),
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNleftPosition, (int)(fCol*(100./fNumCols)),
    		XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    		XmNtopPosition, (int)(fRow*(100./fNumRows)),
    		NULL);
    XmStringFree(s1);
    if (ntupleTitle != NULL) {
    	XtFree(ntupleTitle);
    	ntupleTitle = NULL;
    }
    
    /* Position the plot, and its label too if not disabled by the user */
    if (w->dispLabels) {
    	XtManageChild(w->plot[row*w->numCols+col].label);
    	XtVaSetValues(w->plot[row*w->numCols+col].wInfo->widget,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNleftPosition, (int)(fCol*(100./fNumCols)),
    		XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    		XmNtopWidget, w->plot[row*w->numCols+col].label,
    		XmNbottomPosition, (int)((fRow+1.)*(100./fNumRows)),
    		XmNtraversalOn, False,
    		NULL);
    }
    else
    	XtVaSetValues(w->plot[row*w->numCols+col].wInfo->widget,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNleftPosition, (int)(fCol*(100./fNumCols)),
    		XmNrightPosition, (int)((fCol+1)*(100./fNumCols)),
    		XmNtopPosition, (int)(fRow*(100./fNumRows)),
    		XmNbottomPosition, (int)((fRow+1.)*(100./fNumRows)),
    		XmNtraversalOn, False,
    		NULL);
    
    /* Save & manage the plot widget and return the Window Info pointer */
    w->plot[row*w->numCols+col].plotWidget =
    		w->plot[row*w->numCols+col].wInfo->widget;
    XtManageChild(w->plot[row*w->numCols+col].plotWidget);
    return w->plot[row*w->numCols+col].wInfo;
} 

/*
** Count the number of plot windows displayed
*/
int CountMultWindows(void)
{
    multWindow *w;
    int n = 0;
    
    for (w=MultWindowList; w!=NULL; w=w->next)
    	n++;
    return n;
}

/*
** Get the names and top level X windows for all of the multi-plot windows
** currently displayed (so a windows menu can be created for them).  Arrays
** of length CountMplotWindows() to hold the string pointers and window
** ids must be provided by the caller.
*/
void GetMplotWindowsAndTitles(Window *windows, char **titles)
{
    multWindow *w;
    int i;
    
    for (i=0, w=MultWindowList; w!=NULL; w=w->next) {
    	XtVaGetValues(w->appShell, XmNtitle, &titles[i], NULL);
    	windows[i] = XtWindow(w->appShell);
    	++i;
    }
}

/*
** GetMiniPlotLabel - returns the corresponding mini-plot label widget from a
**			window info pointer.
*/
Widget GetMiniPlotLabel(windowInfo *wInfo)
{
    multWindow *w;
    int i;
    
    /* Find the mini-plot by window info pointer.
    ** Return the corresponding mini-plot label widget.
    */
    if (wInfo == NULL)
    	return False;
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
   	    	return w->plot[i].label;
    	    }
    
    return NULL;
}

/*
** Close a Mini-Plot - returns True if wInfo found & closed, otherwise False.
*/
int CloseMiniPlot(windowInfo *wInfo)
{
    multWindow *w;
    int i;
    
    /* Find the mini-plot by window info pointer.
    ** Call closeMini to close the mini-plot: if it's the last window 
    **      displaying this item, stop updates; free memory; destroy the plot 
    **      widget and the label widget for its title; and set pointers 
    **      to NULL.  
    ** Re-manage frame so its extent is outlined & allow it to receive 
    ** dragged plots.
    */
    if (wInfo == NULL)
    	return False;
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
   	    	closeMini(&w->plot[i].wInfo, &w->plot[i].label, 
   	    		  &w->plot[i].plotWidget);
   	    	XtManageChild(w->plot[i].frame);
		RegisterWidgetAsPlotReceiver(w->plot[i].frame);
    	    	return True;
    	    }
    
    return False;
}

/* Close the mini-plot  - If it's the last window displaying this
**    item, stop updates.  Free memory, destroy the plot  widget and 
**    the label widget for its title, and set pointers to NULL.  
** Calling routine should re-manage frame & allow it to receive plots
**    (if applicable).
*/
static void closeMini(windowInfo **wInfoP, Widget *labelP, Widget *plotWidgetP)
{
    windowInfo *wInfo = *wInfoP;
    Widget label = *labelP, plotWidget = *plotWidgetP;
    int p;

    /* close the plot's auxiliary windows - We do this because Motif has
       a bug where histo can core dump if plot windows are closed when
       auxiliary windows are up. */
    CloseAuxWinsForPlot(wInfo);
    
    /* remove it from the window list */
    RemoveFromWindowList(wInfo);

    /* if it's the last window displaying this item, stop updates */
    for (p=0; p<wInfo->nPlots; p++)
    	if (!ItemIsDisplayed(wInfo->pInfo[p]->id))
    	    EndUpdates(wInfo->pInfo[p]->id);

    /* free memory  */
    freeUpWinMemory(wInfo);
    *wInfoP = NULL;

    /* Destroy label for plot widget */
    XtDestroyWidget(label);

    /* set a timer before destroying plot widget, because of Motif bug in
     * phase 2 destroy (30 seconds) */
    XtUnmapWidget(plotWidget);
    XtAppAddTimeOut(XtWidgetToApplicationContext(plotWidget), 30000, 
    	(XtTimerCallbackProc)destroyPlotCB, plotWidget);
    *labelP = NULL;
    *plotWidgetP = NULL;
}

static void destroyPlotCB(XtPointer client_data, XtIntervalId *id)
{
    Widget widgetToDestroy  = (Widget) client_data;
    
    /* destroy the plot widget */
    XtDestroyWidget(widgetToDestroy);
}

/*
** Close all Multiple-Plot Windows
*/
void CloseAllMultWindows(void)
{
    while (MultWindowList !=NULL)
    	closeMplotWindow(MultWindowList);
    UpdateWindowsMenu();
}

/*
** Close a Multiple-Plot Window from the Window Info Ptr of one of its
**    mini-plots
*/
void CloseMPlotFromWinfo(windowInfo *wInfo)
{
    multWindow *w;
    int i;

    if (wInfo == NULL)
    	return;
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
    		closeMplotWindow(w);
		UpdateWindowsMenu();
		return;
    	    }
}

/*
** SetLabelsOnMultPlot - Set ALL Mini-Plots in the window containing THIS
**			 mini-plot to have Labels. 
**
**		       - The window info pointer of the mini-plot used to
**			 set this plot option is passed to this routine
**			 because it'll be called (at least) by plotMenus.c
**			 and that's all the info it has about the mini-plot.
**
**	Returns:  True if wInfo found and plot labels are restored.
**		  Else, False.
**		
*/
int SetLabelsOnMultPlot(windowInfo *wInfo)
{
    multWindow *w;
    int i, row, col;
    
    if (wInfo == NULL)
    	return False;
    /* Find window info ptr for this mini-plot */
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
		float fNumCols = (float)w->numCols, fNumRows=(float)w->numRows;
		if (w->dispLabels == True)
    		    return True;
    	    	
    	    	/* Found a match; now change each mini-plot in window */
    	    	for (row = 0; row < w->numRows; ++row)
    	    	    for (col = 0; col < w->numCols; ++col) {
			float fCol = (float)col, fRow = (float)row;
    	    		int n = row*w->numCols+col;
    	    		if (w->plot[n].plotWidget == NULL)
    	    		    continue;
    	    		XtManageChild(w->plot[n].label);
    			XtVaSetValues(w->plot[n].plotWidget,
    			    XmNtopAttachment, XmATTACH_WIDGET,
    			    XmNbottomAttachment, XmATTACH_POSITION,
    			    XmNleftAttachment, XmATTACH_POSITION,
    			    XmNrightAttachment, XmATTACH_POSITION,
    			    XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			    XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			    XmNtopWidget, w->plot[row*w->numCols+col].label,
    			    XmNbottomPosition, (int)((fRow+1.)*(100./fNumRows)),
    			    NULL);
    	    		if (w->plot[n].wInfo->showTitlesMenuItem != NULL)
    			    XmToggleButtonSetState(
    			    	w->plot[n].wInfo->showTitlesMenuItem, 
    			    	True, False);	/* On, no notify */
    	    	    }
    	    	w->dispLabels = True;		/* set label/title state */
    	    	return True;
    	    }
    
    return False;
}

/*
** SetNoMultPlotLabels - Set ALL Mini-Plots in the window containing THIS
**			 mini-plot to NOT have Labels. 
**
**		       - The window info pointer of the mini-plot used to
**			 set this plot option is passed to this routine
**			 because it'll be called (at least) by plotMenus.c
**			 and that's all the info it has about the mini-plot.
**
**	Returns:  True if wInfo found and plot labels are unmanaged.
**		  Else, False.
**		
*/
int SetNoMultPlotLabels(windowInfo *wInfo)
{
    multWindow *w;
    int i, row, col;
    
    if (wInfo == NULL)
    	return False;
    
    /* Find window info ptr for this mini-plot */
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
		float fNumCols = (float)w->numCols, fNumRows =(float)w->numRows;
		if (w->dispLabels == False)
    		    return True;
    	    	
    	    	/* Found a match; now change each mini-plot in window */
    	    	for (row = 0; row < w->numRows; ++row)
    	    	    for (col = 0; col < w->numCols; ++col) {
			float fCol = (float)col, fRow = (float)row;
			int n = row*w->numCols+col;
    	    		if (w->plot[n].plotWidget == NULL)
    	    		    continue;
    			XtVaSetValues(w->plot[n].plotWidget,
    			    XmNtopAttachment, XmATTACH_POSITION,
    			    XmNbottomAttachment, XmATTACH_POSITION,
    			    XmNleftAttachment, XmATTACH_POSITION,
    			    XmNrightAttachment, XmATTACH_POSITION,
    			    XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			    XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			    XmNtopPosition, (int)(fRow*(100./fNumRows)),
    			    XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    			    NULL);
    	    		XtUnmanageChild(w->plot[n].label);
    	    		if (w->plot[n].wInfo->showTitlesMenuItem != NULL)
    			    XmToggleButtonSetState(
    			    	w->plot[n].wInfo->showTitlesMenuItem, 
    			    	False, False);	/* Off, no notify */
    	    	    }
    	    	w->dispLabels = False;		/* set label/title state */
    	    	return True;
    	    }
    
    return False;
}


/*
** GetMPlotInfoFromFrame - Get the Multi-Plot Window ptr and the row and column
**			   position of a frame widget within that multi-plot. 
**
**		         - This routine is used by the drag & drop routines
**			   to locate where the drop occurred.
**	Parameters:
**		
**		frame    - The frame widget of an existing multi-plot window
**		row      - the address of an integer to store the row position
**		column   - the address of an integer to store the column pos.
**
**	Returns:  The Multi-Plot widget containing the frame widget.  
**	                   Else NULL.
**		
*/
multWindow *GetMPlotInfoFromFrame(Widget frame, int *ret_row, int *ret_col)
{
    multWindow *w;
    int row, col;
    
    /* Find frame widget in mini-plot list */
    for (w=MultWindowList; w!=NULL; w=w->next)
	for (row = 0; row < w->numRows; ++row)
    	    for (col = 0; col < w->numCols; ++col) {
    	    	int n = row * w->numCols + col;
    	        if (w->plot[n].frame == frame) {
    	            *ret_row = row;
    	            *ret_col = col;
    	            return w;	
    	        }
    	    }
    
    /* Frame widget not found */
    *ret_row = 0;
    *ret_col = 0;
    return NULL;
} 

/*
** InqMiniTitleState - Inquire whether Mini-Plot titles are displayed
**
**	Parameters:
**		
**		shell - The shell widget of the multi-plot window
**
**	Returns:  True or False.  
*/
Boolean InqMiniTitleState(Widget shell)
{
    multWindow *w;
    
    for (w=MultWindowList; w!=NULL; w=w->next)
    	if (w->appShell == shell)
    	    return w->dispLabels;
    return False;
}

void DefaultMultiPlotGeom(int numRows, int numCols, int *width, int *height)
{
    int lgeWidth = 950, lgeHeight = 850;
    
    if (numRows < 3 && numCols < 3) {
    	*width  = 350*numCols; 
    	*height = 300*numRows;
    } else if (numRows > 3 && numCols > 3) {
	*width  = lgeWidth; 
    	*height = lgeHeight;
    } else {
   	float ratioWtoH = (float) lgeWidth / (float) lgeHeight;
	int w = (int) (((float)lgeWidth)  / ((float) numCols));
	int h = (int) (((float)lgeHeight) / ((float) numRows));
	if (numCols > numRows && (((float)w) / ((float)h)) < ratioWtoH)
	    h = (int) (1./ratioWtoH * ((float)w));
	else if (numRows > numCols && (((float)w) / ((float)h)) > ratioWtoH) 
	    w = (int) (ratioWtoH * ((float)h));
	*width  = w*numCols;
    	*height = h*numRows;
    }
}

/*
** ReFillMultiPlot - Re-fill the Mini-Plots in the window containing THIS
**			 mini-plot to a new number of rows and columns. 
**
**		       - The window info pointer of the mini-plot used to
**			 set this plot option is passed to this routine
**			 because it'll be called (at least) by plotMenus.c
**			 and that's all the info it has about the mini-plot.
**
**	Returns:  True if wInfo found and multi-plot now numRows x numCols.
**		  Else, False.
**		
*/
int ReFillMultiPlot(windowInfo *wInfo, int numRows, int numCols)
{
    multWindow *w;
    int i, n, row, col, width, height;
    float fRow, fCol, fNumRows = numRows, fNumCols = numCols;
    
    if (wInfo == NULL)
    	return False;
    
    /* Find window info ptr for this mini-plot */
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
    	    	int j, k, ii, last = numRows * numCols;
		if (w->numRows == numRows && w->numCols == numCols)
    		    return True;		/* nothing to do */
    		    
    	    	/* XtUnmanageWidget(w->appShell); */
    	    	
    	    	/* Found a match; now change the multi-plot window */
    	    	/* But may need new multiplot table first */
    	    	if (numRows * numCols > w->numRows * w->numCols) {
    	    	    multiPlot *tmpPlot = &w->plot[0];
    	    	    w->plot = (multiPlot *) XtMalloc(sizeof(multiPlot) 
    						 * numRows * numCols);
    	    	    memset(w->plot, 0, sizeof(multiPlot) * numRows * numCols);
    	    	    
    	    	    /* Copy all multiPlot structures		     */
    	    	    /* Put all empty frames at the end of the window */
    	    	    for (j = 0, ii = 0; j < w->numRows; ++j)
    	    	        for (k = 0; k < w->numCols; ++k)
    	    	            if (tmpPlot[j*w->numCols+k].plotWidget != NULL)
    	    	            	w->plot[ii++] = tmpPlot[j*w->numCols+k];
    	    	            else
    	    	            	w->plot[--last].frame = 
    	    	            			tmpPlot[j*w->numCols+k].frame;
    	    	    XtFree((char *)tmpPlot);
    	    	}
    	    	
    	    	/* Else, if multiplot table is same or less in size... */
    	    	/* Copy all filled multiPlot structures		       */
    	    	/* Put all empty frames at the end of the window       */
    	    	/* Truncate multwindow if new one is smaller	       */
    	    	else {
    	    	    for (j = 0, ii = 0; 
    	    	         j < w->numRows * w->numCols && ii < numRows * numCols;
    	    	         j++) {
    	    	        if (w->plot[j].plotWidget != NULL) {
    	    	            if (j == ii)
    	    	            	++ii;
    	    	            else {
    	    	            	w->plot[ii++] = w->plot[j];
    	    	            	w->plot[j].wInfo = NULL;
    	    	            	w->plot[j].frame = NULL;
    	    	            	w->plot[j].label = NULL;
    	    	            	w->plot[j].plotWidget = NULL;
    	    	            }
    	    	        } 
    	    	        else {
    	    	            if (w->plot[j].frame != NULL) {
    	    	               UnRegisterWidgetAsPlotReceiver(w->plot[j].frame);
    	    	               XtDestroyWidget(w->plot[j].frame);
    	    	            }
    	    	            w->plot[j].frame = NULL;
    	    	            w->plot[j].wInfo = NULL;
    	    	            w->plot[j].label = NULL;
    	    	        }
    	    	    }
    	    	    while (ii < w->numRows * w->numCols) {
    	    	    	if (w->plot[ii].wInfo != NULL)
    	    	    	    /* Close Mini-Plot - multwindow truncated */
    	    	    	    closeMini(&w->plot[ii].wInfo, &w->plot[ii].label,
    	    	    	    	&w->plot[ii].plotWidget);
    	    	    	if (w->plot[ii].frame != NULL) {
    	    	    	    UnRegisterWidgetAsPlotReceiver(w->plot[ii].frame);
    	    	    	    XtDestroyWidget(w->plot[ii].frame);
    	    	    	    w->plot[ii].frame = NULL;
    	    	    	}
    	    	        ++ii;
    	    	    }
    	    	}
    	    	
    	    	/* Compute new size for the Form, Unmanage it, and Set size */
    	    	w->numRows = numRows;
    	    	w->numCols = numCols;
    	    	DefaultMultiPlotGeom(numRows, numCols, &width, &height);
    	    	XtVaSetValues(w->appShell, XmNwidth, width, XmNheight, height, 
    	    		      NULL);
    	    	
    	    	/*** Now, see if Motif will allow me to re-layout the Form ***/
    	    	for (row = 0; row < w->numRows; ++row) {
    	    	    fRow = row;
    	    	    for (col = 0; col < w->numCols; ++col) {
    	    		fCol = col;
    	    		n = row*w->numCols+col;
    	    		if (w->plot[n].plotWidget == NULL) {
    	    		    if (w->plot[n].frame == NULL) {
    	    		     w->plot[n].frame = XtVaCreateManagedWidget("frame",
    	    		       xmFrameWidgetClass, w->multWidget, 
    			       XmNtopAttachment, XmATTACH_POSITION,
    			       XmNbottomAttachment, XmATTACH_POSITION,
    			       XmNleftAttachment, XmATTACH_POSITION,
    			       XmNrightAttachment, XmATTACH_POSITION,
    			       XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			       XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			       XmNtopPosition, (int)(fRow*(100./fNumRows)),
    			       XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    			       NULL);
    			     XtManageChild(w->plot[n].frame);
    			     RegisterWidgetAsPlotReceiver(w->plot[n].frame);
    	    		   }
    	    		   else {
    	    		   /* we unregister before changing values because
    	    		      otherwise Motif draws outlines for the drop
    	    		      with the old values */
    	    		    UnRegisterWidgetAsPlotReceiver(w->plot[n].frame);
    	    		    XtVaSetValues(w->plot[n].frame,
    	    		     XmNtopAttachment, XmATTACH_POSITION,
    			     XmNbottomAttachment, XmATTACH_POSITION,
    			     XmNleftAttachment, XmATTACH_POSITION,
    			     XmNrightAttachment, XmATTACH_POSITION,
    			     XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			     XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			     XmNtopPosition, (int)(fRow*(100./fNumRows)),
    			     XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    			     NULL);
    			    RegisterWidgetAsPlotReceiver(w->plot[n].frame);
    	    		   }
    	    		}
    			else {
    	    		   XtVaSetValues(w->plot[n].frame,
    	    		     XmNtopAttachment, XmATTACH_POSITION,
    			     XmNbottomAttachment, XmATTACH_POSITION,
    			     XmNleftAttachment, XmATTACH_POSITION,
    			     XmNrightAttachment, XmATTACH_POSITION,
    			     XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			     XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			     XmNtopPosition, (int)(fRow*(100./fNumRows)),
    			     XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    			     NULL);
    	    		   XtVaSetValues(w->plot[n].label,
    	    		     XmNtopAttachment, XmATTACH_POSITION,
    			     XmNleftAttachment, XmATTACH_POSITION,
    			     XmNrightAttachment, XmATTACH_POSITION,
    			     XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			     XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			     XmNtopPosition, (int)(fRow*(100./fNumRows)),
    			     NULL);
    			   /* we unregister before changing values because
    	    		    * otherwise Motif draws outlines for the drop
    	    		    * with the old values */
    			   if (XtClass(w->plot[n].plotWidget) == xyWidgetClass)
    			     UnRegisterWidgetAsPlotReceiver(
    			     	w->plot[n].plotWidget);
    	    		   if (w->dispLabels)
    			    XtVaSetValues(w->plot[n].plotWidget,
    			     XmNtopAttachment, XmATTACH_WIDGET,
    			     XmNbottomAttachment, XmATTACH_POSITION,
    			     XmNleftAttachment, XmATTACH_POSITION,
    			     XmNrightAttachment, XmATTACH_POSITION,
    			     XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			     XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			     XmNtopWidget, w->plot[n].label,
    			     XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    			     NULL);
    			   else
    			    XtVaSetValues(w->plot[n].plotWidget,
    			     XmNtopAttachment, XmATTACH_POSITION,
    			     XmNbottomAttachment, XmATTACH_POSITION,
    			     XmNleftAttachment, XmATTACH_POSITION,
    			     XmNrightAttachment, XmATTACH_POSITION,
    			     XmNleftPosition, (int)(fCol*(100./fNumCols)),
    			     XmNrightPosition, (int)((fCol+1.)*(100./fNumCols)),
    			     XmNtopPosition, (int)(fRow*(100./fNumRows)),
    			     XmNbottomPosition,(int)((fRow+1.)*(100./fNumRows)),
    			     NULL);
    			   if (XtClass(w->plot[n].plotWidget) == xyWidgetClass)
    			     RegisterWidgetAsPlotReceiver
    			        (w->plot[n].plotWidget);
    			}
    	    	    }
    	    	}
    	    	return True;
    	    }
    	    
    /* No windowInfo pointer matched, couldn't find multiplot window */
    return False;
}

/*
 * GetStatsForMultiPlot -  returns statistics for a Multi-Plot Window given
 * 			   the windowInfo pointer of one plot in the
 *			   Multi-Plot:
 *      numRows:   the number of rows in the multi-plot
 *      numCols:   the number of cols in the multi-plot
 *      numPlots:  the number of plots actually displayed (i.e. not blank)
 *      numUnused: the number of empty frames in the multi-plot
 *                               (= numRows * numCols - numPlots)
 *      returns 0 if multi-plot found, otherwise -1 (error).
 */
int GetStatsForMultiPlot(windowInfo *wInfo, int *numRows, int *numCols,
			 int *numPlots, int *numUnused)
{
    multWindow *w;
    int i, j, nP = 0;

    /* Find window info ptr for this mini-plot */
    for (w=MultWindowList; w!=NULL; w=w->next)
    	for (i = 0; i < w->numRows * w->numCols; ++i)
    	    if (w->plot[i].wInfo == wInfo) {
		nP = 0;
		for (j = 0; j < w->numRows * w->numCols; ++j) {
		    if (w->plot[j].plotWidget != NULL)
		        ++nP;
    	    	}
		*numPlots = nP;
		*numRows = w->numRows;
		*numCols = w->numCols;
		*numUnused = w->numRows * w->numCols - nP;
		return 0;
	    }
    return -1;
}

/*
** Close one Multiple-Plot Window
*/
static void closeMplotWindow(multWindow *w)
{
    int i, j, p;
    readyCallBack *rcb, *tmp;
    
    /* remove it from the window list */
    removeFromMPWindowList(w);

    /* Go through each mini-plot structure and remove its windowInfo from that
       list.  Check whether updates for the plot can be stopped now. */
    for (i = 0; i < w->numRows; ++i) {
    	for (j = 0; j < w->numCols; ++j) {
    	    if (w->plot[i*w->numCols+j].wInfo != NULL) {
    	    	windowInfo *wInfo = w->plot[i*w->numCols+j].wInfo;
    	    	RemoveFromWindowList(wInfo);
		/* if it's the last window displaying any of the items in
		   this plot, stop updates */
   	    	for (p=0; p<wInfo->nPlots; p++)
    	    	    if (!ItemIsDisplayed(wInfo->pInfo[p]->id))
    	    		EndUpdates(wInfo->pInfo[p]->id);
		/* free memory  */
		freeUpWinMemory(wInfo);
    	    } 
    	}
    }

    /* Destroy the multi-plot widget (and all its miniplots) & free memory */
    XtDestroyWidget(w->appShell);
    XtFree((char *)w->plot);
    for (rcb = w->readyCB; rcb; ) {
	tmp = rcb;
	rcb = rcb->next;
	XtFree((char *)tmp);
    }
    w->readyCB = NULL;
    XtFree((char *)w);
}


/*
** Remove a multi-window from the list of multi-windows (without freeing its
** 	memory)
*/
static void removeFromMPWindowList(multWindow *w)
{
    multWindow *temp;

    if (MultWindowList == w)
	MultWindowList = w->next;
    else {
	for (temp = MultWindowList; temp != NULL; temp = temp->next) {
	    if (temp->next == w) {
		temp->next = w->next;
		break;
	    }
	}
    }
}

/*
** Close callback - Called when user selects Close from the Window menu of
** 		    the Multi-Plot window.
*/
static void closeCB(Widget w, multWindow *mW, caddr_t callData)
{
    closeMplotWindow(mW);
    UpdateWindowsMenu();
}

/*
** Create the Histoscope multiple plot dialog
*/

static void createMultPDialog(Widget parent)
{
    XmString xmstr;
    Widget numColsLabel, numRowsLabel;
    Widget winTitleLabel, instrLabel, categoryButton;

    /* Create the form onto which everything goes */
    Form = XmCreateFormDialog(parent, "form", NULL, 0);
    XtVaSetValues(Form, XmNdialogTitle, xmstr=XmStringCreateSimple(
    	    				"Create Multiple Plot Window"),
    	    XmNautoUnmanage, FALSE, 
    	    NULL);
    XmStringFree(xmstr);
    XtAddCallback(Form, XmNhelpCallback, (XtCallbackProc)  helpCB, NULL);

    instrLabel = XtVaCreateManagedWidget("instrLabel",
    	    xmLabelGadgetClass, Form, 
    	    XmNlabelString, xmstr=XmStringCreateLtoR(
"Press the Use Current Category button to display all\n\
   its histograms in one Multi-Plot Window.\n\
OR Enter a window title, # rows, # cols, and later drag\n\
   each plot itself into the empty Multi-Plot Window.    ",
    	     XmSTRING_DEFAULT_CHARSET),
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNtopOffset, 10,
    	    XmNleftOffset, 10,
    	    XmNrightOffset, 10,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(instrLabel);

    CreateWinBtn = XtVaCreateManagedWidget("createWinBtn", 
    	    xmPushButtonWidgetClass, Form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("Create Window"),
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNleftOffset, 10,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 27,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, 10,
    	    NULL);
    XtAddCallback(CreateWinBtn, XmNactivateCallback,  
    	    (XtCallbackProc)  createWinCB, NULL);
    XtManageChild(CreateWinBtn);
    XmStringFree(xmstr);
    XtVaSetValues(Form, XmNdefaultButton, CreateWinBtn, NULL);

    ResetBtn = XtVaCreateManagedWidget("resetBtn", 
    	    xmPushButtonGadgetClass, Form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("Reset"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 32,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 50,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, 10,
    	    NULL);
    XtAddCallback(ResetBtn, XmNactivateCallback,
            (XtCallbackProc)  resetCB, NULL);
    XtManageChild(ResetBtn);
    XmStringFree(xmstr);

    CancelBtn = XtVaCreateManagedWidget("cancelBtn", 
    	    xmPushButtonGadgetClass, Form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("Cancel"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 55,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 73,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, 10,
    	    NULL);
    XtAddCallback(CancelBtn, XmNactivateCallback,
            (XtCallbackProc)  cancelCB, NULL);
    XtManageChild(CancelBtn);
    XmStringFree(xmstr);

    HelpBtn = XtVaCreateManagedWidget("helpBtn", 
    	    xmPushButtonWidgetClass, Form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("Help"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 78,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNrightOffset, 10,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, 10,
    	    NULL);
    XtAddCallback(HelpBtn, XmNactivateCallback, (XtCallbackProc) helpCB, NULL);
    XtManageChild(HelpBtn);
    XmStringFree(xmstr);
    
    numRowsLabel = XtVaCreateManagedWidget("numRowsLabel",
    	    xmLabelGadgetClass, Form, 
    	    XmNlabelString, xmstr=XmStringCreateSimple("# Rows:"),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNbottomWidget, CreateWinBtn,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNbottomOffset, 14,
    	    XmNleftOffset, 10,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(numRowsLabel);
    
    NumRowsText = XtVaCreateManagedWidget("numRowsText",
    	    xmTextWidgetClass, Form,
    	    XmNrows, (short)1,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNleftPosition, 25,
    	    XmNbottomWidget, CreateWinBtn,
    	    XmNbottomOffset, 5,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 46,
    	    XmNcolumns, 5,
    	    NULL);
    XtManageChild(NumRowsText);
    RemapDeleteKey(NumRowsText);

    numColsLabel = XtVaCreateManagedWidget("numColsLabel",
    	    xmLabelGadgetClass, Form, 
    	    XmNlabelString, xmstr=XmStringCreateSimple("# Columns:"),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 60,
    	    XmNbottomWidget, CancelBtn,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNbottomOffset, 14,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(numColsLabel);
    
    NumColsText = XtVaCreateManagedWidget("numColsText",
    	    xmTextWidgetClass, Form,
    	    XmNrows, (short)1,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNleftPosition, 75,
    	    XmNbottomWidget, CancelBtn,
    	    XmNrightOffset, 10,
    	    XmNbottomOffset, 4,
    	    XmNcolumns, 5,
    	    NULL);
    XtManageChild(NumColsText);
    RemapDeleteKey(NumColsText);

    winTitleLabel = XtVaCreateManagedWidget("winTitleLabel",
    	    xmLabelGadgetClass, Form, 
    	    XmNlabelString, xmstr=XmStringCreateSimple("Window Title:"),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNbottomWidget, NumRowsText,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNbottomOffset, 14,
    	    XmNleftOffset, 10,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(winTitleLabel);
    
    WinTitleText = XtVaCreateManagedWidget("winTitleText",
    	    xmTextWidgetClass, Form,
    	    XmNrows, (short)1,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNleftWidget, winTitleLabel,
    	    XmNleftPosition, 25,
    	    XmNbottomWidget, NumRowsText,
    	    XmNbottomOffset, 5,
    	    XmNrightOffset, 10,
    	    NULL);
    XtManageChild(NumRowsText);
    RemapDeleteKey(NumRowsText);

    categoryButton = XtVaCreateManagedWidget("categoryButton",
    	    xmPushButtonWidgetClass, Form, 
    	    XmNlabelString, xmstr=XmStringCreateLtoR(" Use Current\nCategory  ",
    	    	XmSTRING_DEFAULT_CHARSET),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNbottomWidget, WinTitleText,
    	    XmNbottomOffset, 5,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNleftOffset, 10,
    	    XmNtopOffset, 7,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, instrLabel,
    	    NULL);
    XmStringFree(xmstr);
    XtAddCallback(categoryButton, XmNactivateCallback,
            (XtCallbackProc)  setCategoryCB, NULL);
    XtManageChild(winTitleLabel);
    
    CurCatText = XtVaCreateManagedWidget("curCatText",
    	    xmTextWidgetClass, Form,
    	    XmNrows, (short)1,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 25,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNbottomWidget, WinTitleText,
    	    XmNbottomOffset, 5,
    	    XmNtopOffset, 7,
    	    XmNrightOffset, 10,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, instrLabel,
    	    XmNeditable, FALSE,
    	    XmNeditMode, XmMULTI_LINE_EDIT,
    	    NULL);
    XtManageChild(CurCatText);
    RemapDeleteKey(CurCatText);
}


/*
 *  createWinCB - Create Multiple Plot Window.
 */
static void createWinCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    char *windowTitle, itemStr[32];
    int *idList = NULL, numIdsInCat;
        
    /* Get the window title & the number of rows and columns from dialog 
     * Make sure numeric values are valid                                */
    if (GetIntTextWarn(NumRowsText, &NumRows, "Number of Rows", True) 
            != TEXT_READ_OK) 
        return;
    if (GetIntTextWarn(NumColsText, &NumCols, "Number of Columns", True) 
            != TEXT_READ_OK) 
        return;
    if (NumRows * NumCols <= 0 || NumRows < 0 || NumCols < 0) {
        DialogF(DF_INF, Form, 1, 
            "Please specify a proper number of rows and columns",
            "Acknowledged");
        return;
    }
    
    if (OneCat) {		/* Making multiplot from one category? */
    	/* Get new histogram list, in case client changed what's there */
    	/*     since user pressed the set category button              */
    	numIdsInCat = GetCurrentCategoryHists(CurrentCat, &idList);
    	if (numIdsInCat != NumIdsInCat) {
	    strcpy(CatString, CurrentCat);
	    sprintf(itemStr, "\n(%d histograms)", numIdsInCat);
	    strcat(CatString, itemStr);
	    XmTextSetString(CurCatText, CatString);
    	}
    	if (NumRows * NumCols < numIdsInCat) {
    	    int btnNum = DialogF(DF_INF, Form, 2, 
    	      "You have specified %d rows and %d columns \n\
(%d total mini-plots) which is less than \n\
the number of histograms in category: \n\
%s.", "Cancel",
           "Continue", NumRows, NumCols, NumRows * NumCols, CurrentCat);
    	    if (btnNum == 1) {
    	        if (idList != NULL)
    	            XtFree((char *)idList);
    	        return;
    	    }
    	}
        /* Create the Multiple Plot Window and give a default initial width */
        /* and height. If no window title has been given, give a default one. */
   	windowTitle = XmTextGetString(WinTitleText);	/* Window title */
   	if (windowTitle[0] == '\0') {
   	    XtFree(windowTitle);
   	    windowTitle = XtMalloc(strlen("Untitled")+1);
   	    strcpy(windowTitle, "Untitled");
   	}
   	if (numIdsInCat > 0) {
   	    int width, height;
   	    PtrToReturn = CreateMultPlotFromList(XtParent(Form), 
    		windowTitle, NULL, NumRows, NumCols, numIdsInCat, idList, NULL);
	    DefaultMultiPlotGeom(NumRows, NumCols, &width, &height);
	    XtVaSetValues(PtrToReturn->appShell, XmNwidth, width,
    					         XmNheight, height, NULL);
	    XtFree((char *)idList);
	    idList = NULL;
	}
    }
    else {
        /* Create the Multiple Plot Window */
   	windowTitle = XmTextGetString(WinTitleText);	/* Window title */
   	if (windowTitle[0] == '\0') {
   	    XtFree(windowTitle);
   	    windowTitle = XtMalloc(strlen("Untitled")+1);
   	    strcpy(windowTitle, "Untitled");
   	}
    	PtrToReturn = CreateEmptyMultPlot(XtParent(Form), windowTitle, NULL,
    	    NumRows, NumCols, NULL); 
	/* Give a default initial width and height */
	XtVaSetValues(PtrToReturn->appShell, XmNwidth,350, XmNheight,275, NULL);
    }
    XtFree(windowTitle);
    UpdateWindowsMenu();
    DoneWithDialog = 1;
}

static void cancelCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    SET_ONE_RSRC(Form,  XmNdefaultButton, CreateWinBtn);
    XmProcessTraversal (CreateWinBtn, XmTRAVERSE_CURRENT);
    					   /* position keyboard focus */
    XtUnmanageChild(Form);
    reInitDialog(1);			/* Set dialog to creation state   */
}

static void resetCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    SET_ONE_RSRC(Form,  XmNdefaultButton, CreateWinBtn);
    XmProcessTraversal (CreateWinBtn, XmTRAVERSE_CURRENT);
    					   /* position keyboard focus */
    reInitDialog(0);			/* Set dialog to creation state   */
}

static void helpCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    CreateHelpDialog(HelpBtn, "Creating a Multiple Plot Window", HelpText);
}

static void setCategoryCB(Widget w, caddr_t clientData, caddr_t callData) 
{
    char itemStr[32];

    if (IdList != NULL) {
    	XtFree((char *)IdList);
    	IdList = NULL;
    }
    NumIdsInCat = GetCurrentCategoryHists(CurrentCat, &IdList);
    trimLeadNtrailWhite(CurrentCat);	/* trim leading whitespace */
    XmTextSetString(WinTitleText, CurrentCat);
    strcpy(CatString, CurrentCat);
    sprintf(itemStr, "\n(%d histograms)", NumIdsInCat);
    strcat(CatString, itemStr);
    XmTextSetString(CurCatText, CatString);
    NumRows = (int) sqrt((double) NumIdsInCat);
    NumCols = NumRows;
    if (NumCols * NumRows < NumIdsInCat) {
    	++ NumCols;
    	if (NumCols * NumRows < NumIdsInCat)
    	    ++ NumRows;
    }
    SetIntText(NumRowsText, NumRows);
    SetIntText(NumColsText, NumCols);
    OneCat = True;
    XmProcessTraversal (CreateWinBtn, XmTRAVERSE_CURRENT);
    					   /* position keyboard focus */
}
static void reInitDialog(int done)
{
    XmTextSetString(NumRowsText, "");
    XmTextSetString(NumColsText, "");
    XmTextSetString(WinTitleText, "");
    XmTextSetString(CurCatText, "");
    NumRows = 0;
    NumCols = 0;
    if (IdList != NULL) {
    	XtFree((char *)IdList);
    	IdList = NULL;
    }
    CurrentCat[0] = '\0';
    CatString[0] = '\0';
    OneCat = False;
    PtrToReturn = NULL;
    DoneWithDialog = done;
}

/*
** trimLeadNtrailWhite - Trim off leading and trailing spaces & tabs
**			 from an input string
** 			 If string is NULL, NULL is returned.
*/
static void trimLeadNtrailWhite(char *string)
{
    char *c;
    int len, whiteLen, i;
    
    if (string == NULL || string[0] == '\0')
    	return;
    
    /* Find the first non-white character & move non-white chars left */
    for (whiteLen = 0; isspace(string[whiteLen]) != 0; ++whiteLen);
     
    for ( c = &string[whiteLen], i = 0, len = strlen(c); 
          whiteLen != 0 && i < len; ++i)
    	string[i] = c[i];
    for ( len = strlen(string); whiteLen != 0 && i < len; ++i)
    	string[i] = ' ';		/* pad with spaces */     	
    
    /* Remove trailing whitespace by converting to nulls */
    if (len == 0)		/* special case for all white space strings */
    	return;
    for (c = &string[len-1]; ; c--) {
    	if (isspace(c[0]))
    	    *c = '\0';
    	else
    	    break;
    }
}    

/*
** Generate a PostScript representation for a multi-plot window.  The
** confusing parameter "shell" is the shell widget of the multi-plot window
** to be printed.  It is used to identify the window because multWindow
** data structures are not known outside of this module.
*/
void PrintMultiPlotWin(Widget shell, char *fileName)
{
    int i, j;
    multiPlot *plot;
    Dimension width, height;
    int plotWidth, plotHeight, printTitle;
    char *windowTitle;
    FILE *psFile;
    multWindow *w;
    
    /* Translate shell into a multi-plot window (w) */
    for (w=MultWindowList; w!=NULL; w=w->next)
    	if (w->appShell == shell)
    	    break;
    if (w == NULL)
    	return;

    /* Get the width, height, and title of the window */
    XtVaGetValues(shell, XmNwidth, &width, XmNheight, &height,
    	    XmNtitle, &windowTitle, NULL);

    /* Open the PS file and draw the window title into it.
     * Skip titles which start with double forward slash.
     */
    printTitle = strncmp(windowTitle, "//", 2) && GetTitlePSOnOrOff();
    psFile = OpenPS(fileName, width, height + 
		    (printTitle ? GetTitlePSFontSize() + WINDOW_TITLE_MARGIN : 0));
    if (printTitle)
    {
	fprintf(psFile, "/%s findfont %04d scalefont setfont\n",
		GetTitlePSFont(), GetTitlePSFontSize());
	fprintf(psFile,"(%s) dup stringwidth pop 2 div neg %d add %d moveto show\n",
		windowTitle, width/2, height + WINDOW_TITLE_MARGIN);
    }

    /* Go through each mini-plot structure and print the title and plot. */
    plotWidth = width / w->numCols;
    plotHeight = height / w->numRows;
    for (i = 0; i < w->numRows; ++i) {
    	for (j = 0; j < w->numCols; ++j) {
    	    plot = &w->plot[i*w->numCols+j];
    	    if (plot->wInfo != NULL) {
		if (w->dispLabels)
    	    	    printMiniPlot(psFile, width*j/w->numCols,
    	    		height - height*(i+1)/w->numRows, plotWidth, plotHeight,
    	    		plot->wInfo->widget, plot->label);
		else
    	    	    printMiniPlot(psFile, width*j/w->numCols,
    	    		height - height*(i+1)/w->numRows, plotWidth, plotHeight,
    	    		plot->wInfo->widget, NULL);
    	    } 
    	}
    }
    EndPS();
}

/*
** Generate PostScript code for a mini-plot.  If labelWidget is non-null,
** include the label at the top of the plot.
*/
static void printMiniPlot(FILE *psFile, int x, int y, int width, int height,
	Widget plotWidget, Widget labelWidget)
{
    XmString labelMString;
    char *labelString;
    XmFontList labelFontList;
    int labelFontSize, labelHeight = 0;
    WidgetClass class;

    /* Print the mini-plot title */
    if (labelWidget != NULL && GetMplotTitlePSOnOrOff()) {
	XtVaGetValues(labelWidget, XmNlabelString, &labelMString,
		      XmNfontList, &labelFontList, NULL);
	labelString = GetXmStringText(labelMString);
	if (strncmp(labelString, "//", 2))
	{
	    labelFontSize = GetMplotTitlePSFontSize();
	    labelHeight = labelFontSize + 2;
	    fprintf(PSGetFile(), "gsave\n");
	    PSSetWindowPagePosition(x, y, width, height);
	    fprintf(psFile, "/%s findfont %04d scalefont setfont\n",
		    GetMplotTitlePSFont(), labelFontSize);
	    fprintf(psFile,
		    "(%s) dup stringwidth pop 2 div neg %d add %d moveto show\n",
		    labelString, width/2, height - labelHeight);
	    fprintf(PSGetFile(), "grestore\n");
	}
	XtFree(labelString);
    }

    /* Print the plot contents */
    fprintf(PSGetFile(), "gsave\n");
    PSSetWindowPagePosition(x, y, width, height - labelHeight);
    class = XtClass(plotWidget);
    if (class == scatWidgetClass) ScatWritePS(plotWidget, psFile);
    else if (class == h1DWidgetClass) H1DWritePS(plotWidget, psFile);
    else if (class == xyWidgetClass) XYWritePS(plotWidget, psFile);
    else if (class == cellWidgetClass) CellWritePS(plotWidget, psFile);
    else if (class == hist2DWidgetClass) hist2DwritePs(plotWidget, psFile);
    else if (class == scat3DWidgetClass) Scat3DWritePS(plotWidget, psFile);
    fprintf(PSGetFile(), "grestore\n");
 }

/*
** Version of XmToggleButtonSetState which checks for "w" being NULL
** first before trying to set the button state.  Used for safety so that
** if an improper keyword tries to set a button that doesn't exist, the
** program won't respond with a crash.
*/
static void setMenuToggle(Widget w, int buttonState, int notify)
{
    if (w != NULL)
    	XmToggleButtonSetState(w, buttonState, notify);
}

void CloseMPlotWindowById(void *w, int taskNumber, char *dumm)
{
    closeCB(NULL, (multWindow *)w, NULL);
    ReportTaskCompletion(taskNumber, 0, NULL);
}

void GenerateMPlotPSById(void *w, int taskNumber, char *filename)
{
    PrintMultiPlotWin(((multWindow *)w)->appShell, filename);
    ReportTaskCompletion(taskNumber, 0, NULL);
}

Boolean MPlotRefreshCycle(void)
{
    multWindow *mp, *tmp;
    windowInfo *w;
    int i, j, ready, nwindows;
    hsGeneral *item;
    Boolean runFutureCallbacks = False;

    for (tmp=MultWindowList; tmp!=NULL; ) {
	mp = tmp;
	tmp = tmp->next;
	if (mp->readyCB)
	{   
	    ready = 1;
	    nwindows = mp->numRows * mp->numCols;
	    for (j=0; j<nwindows; ++j) {
		if (ready) {
		    w = mp->plot[j].wInfo;
		    if (w) {
			if (w->update) {
			    if (w->needsUpdate == True) {
				ready = 0;
			    } else {
				for (i=0; i<w->nPlots; i++) {
				    item = GetMPItemByID(w->pInfo[i]->id);
				    if (!ItemHasData(item)) {
					ready = 0;
					break;
				    }
				}
			    }
			}
			if (ready)
			    if (!overlayedObjectsReady(w->decor))
				ready = 0;
		    }
		}
	    }
	    if (ready)
		executeMPlotReadyCallbacks(mp);
	    else
		runFutureCallbacks = True;
	}
    }
    return runFutureCallbacks;
}

void executeMPlotReadyCallbacks(multWindow *wInfo)
{
    /* execute the callbacks in the FIFO order */
    multWindow *w;
    readyCallBack cbData;
    readyCallBack *last, *tmp;

    /* the window can be destroyed as a result of a callback, so try
       to make sure that we do not operate on a destroyed window */
    for (w=MultWindowList; w!=NULL; w=w->next) {
	if (w == wInfo) {
	    if (wInfo->readyCB) {
		tmp = NULL;
		for (last = wInfo->readyCB; last->next; ) {
		    tmp = last;
		    last = tmp->next;
		}
		memcpy(&cbData, last, sizeof(readyCallBack));
		XtFree((char *)last);
		if (tmp)
		    tmp->next = NULL;
		if (last == wInfo->readyCB)
		    wInfo->readyCB = NULL;
		cbData.callback(wInfo, cbData.taskNumber, cbData.data);
		executeMPlotReadyCallbacks(wInfo);
	    }
	    break;
	}
    }
}

static void freeUpWinMemory(windowInfo *wInfo)
{
    int p;
    if (wInfo->curveStyles != NULL) {
    	for (p=0; p<wInfo->nCurves; p++)
    	    XmStringFree(((XYCurve *)wInfo->curveStyles)[p].name);
    	XtFree((char *)wInfo->curveStyles);
    }
    if (wInfo->histStyles != NULL) {
    	for (p=0; p<wInfo->nHists; p++)
    	   XmStringFree(((XYHistogram *)wInfo->histStyles)[p].name);
    	XtFree((char *)wInfo->histStyles);
    }
    for (p=0; p<wInfo->nPlots; p++)
    {
	decrColorScaleRefCount(wInfo->pInfo[p]->csi.colorScale);
	XtFree((char *)wInfo->pInfo[p]);
    }
    XtFree((char *)wInfo->pInfo);
    XtFree((char *)wInfo);
}

void RedisplayAllColoredWindows(const ColorScale *scale)
{
    const int redisplayMode = ANIMATION;
    int i, row, col;
    windowInfo *w;
    multWindow *window;
    plotInfo *pInfo;
    multiPlot *plot;

    if (scale == NULL)
	return;
    for (w=WindowList; w!=NULL; w=w->next)
    {
	if (!w->multPlot) 
	{
	    for (i=0; i<w->nPlots; i++)
	    {
		pInfo = w->pInfo[i];
		if (pInfo->csi.colorScale == scale)
		    RedisplayPlotWindow(w, redisplayMode);
	    }
	}
    }
    for (window=MultWindowList; window != NULL; window=window->next) {
	for (row = 0; row < window->numRows; ++row) {
	    for (col = 0; col < window->numCols; ++col) {
		plot = &window->plot[row*window->numCols+col];
		if (plot->wInfo != NULL) {
		    w = plot->wInfo;
		    for (i=0; i<w->nPlots; i++)
		    {
			pInfo = w->pInfo[i];
			if (pInfo->csi.colorScale == scale)
			    RedisplayPlotWindow(w, redisplayMode);
		    }
		}
	    }
	}
    }
}
