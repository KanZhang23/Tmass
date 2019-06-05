/*******************************************************************************
*									       *
* plotWindows.c -- Windows and support for plotting widgets		       *
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
* May 20, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Modified by Joy Kyriakopulos 5/12/93 to add slider > or < 		       *
*			       6/29/93 to fix memory leaks found by Purify     *
*									       *
* Lots of changes and fixes by igv, 2002-2003		                       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include "../util/misc.h"
#include "../util/help.h"
#include "../util/DialogF.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "../plot_widgets/H1D.h"
#include "../plot_widgets/Scat.h"
#include "../plot_widgets/XY.h"
#include "../plot_widgets/Cell.h"
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/3DScat.h"
#include "../plot_widgets/adaptHist.h"
#include "../plot_widgets/XYTransform.h"
#include "colorScaleDialog.h"
#include "histoP.h"
#include "MarginSlider.h"
#include "plotWindows.h"
#include "plotMenus.h"
#include "mainPanel.h"
#include "mainMenu.h"
#include "communications.h"
#include "help.h"
#include "interpret.h"
#include "variablePanel.h"
#include "auxWindows.h"
#include "ntuplePanel.h"
#include "multPlot.h"
#include "dragPlot.h"
#include "defaultColorScale.h"

#define NICE_COUNTS_PER_BIN 8	/* average number of counts desired
				   per histogram bin for a useful initial
				   histogram from ntuple data */
#define MAX_1DHIST_BINS 100	/* initial maximum number of bins for 1D 
				   histograms derived from ntuple data */
#define MIN_1DHIST_BINS 10	/* initial minimum number of bins for 1D 
				   histograms derived from ntuple data */
#define MAX_2DHIST_BINS 50	/* initial maximum number of bins for 2D 
				   histograms derived from ntuple data */
#define MIN_2DHIST_BINS 5	/* initial minimum number of bins for 2D 
				   histograms derived from ntuple data */

/* id of Xt timer procedure for refreshing windows after new data arrives */
static XtIntervalId RefreshProcID = 0;

static void invalidateWindow(windowInfo *wInfo);
static void refreshWorkProc(XtPointer client_data, XtIntervalId *id);
static void moveToEndOfWindowList(windowInfo *window);
static plotInfo *createPlotInfo(hsGeneral *item, int plotType, int *ntVars,
    	int *sliderVars, int errorBars);
static void setErrorIndicator(windowInfo *window, int state);
static void ntupleToHistData(hsNTuple *ntuple, plotInfo *pInfo,
    	XYHistogram *hist);
static void ntupleToAdaptHistData(hsNTuple *ntuple, plotInfo *pInfo,
    	XYHistogram *hist);
static void histToHistData(hs1DHist *hist, plotInfo *pInfo,
    	XmString *yLabel, XYHistogram *xyHist);
static void ntupleToTimeSeriesCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XYCurve *curves, int *nCurves);
static void ntupleToTSErrCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XYCurve *curves, int *nCurves);
static void tsCurveCommon(hsNTuple *ntuple, plotInfo *pInfo,
	int nVars, int *vars, int *topErrs, int *bottomErrs, XYCurve *curves,
	int *nCurves);
static void ntupleToXYCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves);
static void ntupleToXYErrCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves);
static void ntupleToXYSortCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves);
static void ntupleToXYSortErrCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves);
static void xyCurveCommon(hsNTuple *ntuple, plotInfo *pInfo,
	int nVars, int *xVars, int *yVars, int *topErrs, int *bottomErrs,
	int *leftErrs, int *rightErrs, int sort, XmString *xLabel,
	XYCurve *curves, int *nCurves);
static void redisplay2D(windowInfo *window, int mode);
static void redisplay2DHist(windowInfo *window, int mode);
static void redisplay2DAdaptHist(windowInfo *window, int mode);
static void redisplayScat(windowInfo *window, int mode);
static void redisplay3DScat(windowInfo *window, int mode);
static void redisplayCell(windowInfo *window, int mode);
static void redisplayColorCell(windowInfo *window, int mode);
static void setWidgetBuffering(Widget w, int state);
static void addGaussianErrorBars(XYHistogram *hist);
static int hasActiveSliders(windowInfo *window);
static int comparePoints(const void *point1, const void *point2);
static void makeUniqueCurveStyle(XYCurve *styleList, int nStyles, int index);
static int isUniqueCurveStyle(XYCurve *style, XYCurve *styleList, int nStyles,
    	int ignoreIndex);
static void makeUniqueHistStyle(XYHistogram *styleList, int nStyles, int index);
static int isUniqueHistStyle(XYHistogram *style, XYHistogram *styleList,
    	int nStyles, int ignoreIndex);
static void freeCurves(XYCurve *curves, int nCurves);
static void freeHists(XYHistogram *hists, int nHists);
static void closeCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void destroyPlotCB(XtPointer client_data, XtIntervalId *id);
static void executeReadyCallbacks(windowInfo *wInfo);
static int duplicateColor(Display *display, Colormap cmap,
			  Pixel in, Pixel *out);
static void printCoordsCB(Widget w, XtPointer wPtr, XtPointer callData);

/* List of all plot windows (this list does not include n-tuple windows, which
   are maintained by the module ntuple.c, or other types of windows) */
windowInfo *WindowList = NULL;

/* Whether plots should be done with graphics buffering on or off */
static int GraphicsBuffering = False;

/* Initial thicken points status for scatter plots */
static Boolean ThickenPointsScat = True;
static Boolean ThickenPointsScat3D = True;

/* Should we allow printing of mouse coordinates to stdout? */
static int allowCoordPrint = False;

/* A copy of the default topShadow and bottomShadow pixmaps for plot widgets,
   squirreled away when the first window was created, to be used for restoring
   normal border shadows to a widget after it has had error shadows displayed */
static int HaveNormalShadows = False;
Pixmap NormalTopPixmap;
Pixmap NormalBottomPixmap;

/* Variables related to title fonts in PostScript files */
static char *titlePSFont = NULL;
static int titlePSFontSize = 14;
static Boolean printTitleInPS = True;

void SetTitlePSFont(const char *font)
{
    if (font) {
	if (titlePSFont)
	    free(titlePSFont);
	titlePSFont = strdup(font);
    }
}
void SetTitlePSFontSize(int size)
{
    if (size > 0)
	titlePSFontSize = size;
}
void SetTitlePSOnOrOff(Boolean state)
{
    printTitleInPS = state;
}
const char *GetTitlePSFont(void)
{
    static char defaultFont[] = "Times-Roman-ISOLatin1";
    if (titlePSFont)
	return titlePSFont;
    else
	return defaultFont;
}
int GetTitlePSFontSize(void)
{
    return titlePSFontSize;
}
Boolean GetTitlePSOnOrOff(void)
{
    return printTitleInPS;
}

/*
** Update all displayed windows using data with histoscope id id.  This
** routine no longer does the actual refresh, it just marks the window
** as needing to be updated, and sets up an Xt timer proc to do the refreshing
** when there's time for it.
*/
void RefreshItem(int id)
{
    windowInfo *w;
    int p;
    
    for (w=WindowList; w!=NULL; w=w->next) {
    	for (p=0; p<w->nPlots; p++) {
    	    if (w->pInfo[p]->id == id) {
    		if (w->update) {
    		    w->needsUpdate = True;
		    RunRefreshCycle(w);
    		} else {
    	    	    StarWindowTitle(w, True);
    		}
    	    }
    	}
    }
}

void RunRefreshCycle(windowInfo *w)
{
    if (RefreshProcID == 0) {
	RefreshProcID = XtAppAddTimeOut(
	    XtWidgetToApplicationContext(w->shell),
	    0, refreshWorkProc, NULL);
    }
}

/*
** Reprocess the data from item "item" to update the plot in window "window"
** using redisplay mode "redisplayMode"
*/
void RedisplayPlotWindow(windowInfo *window, int redisplayMode)
{
    char *lastMessage;
    
    ResetExtNTRefErrors();
    (*window->redisplay)(window, redisplayMode);
    setErrorIndicator(window, CheckExtNTRefErrors(&lastMessage));
}

/*
** Overlay an additional plot over the plot(s) in "window".  This tries to
** be like ViewItem or ViewNTuplePlot, but requires some additional arguments,
** nXBins and nYBins, because the defaulting provided for these in the
** CreateXXX routines for ntuple plots is too complicated to duplicate here,
** and happens to be available to the callers of this routine (in other words,
** it's a hack).  The "curveStyles", "histStyles", and "nCurves" arguments
** optionally provide style information to be applied to the new plot.
** "curveStyles" and "histStyles" can be passed as NULL if no such information
** is available to the caller.
*/
void OverlayPlot(windowInfo *window, hsGeneral *item, int plotType, int *ntVars,
    	int *sliderVars, int nXBins, int nYBins, int errorBars,
    	void *curveStyles, int nCurves, void *histStyle,
        colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    plotInfo **oldPlotList, **newPlotList, *newPlot;
    int i, oldNCurves = window->nCurves, oldNHists = window->nHists;
    XYCurve *curve, *oldCurveStyles, *newCurveStyles;
    XYHistogram *oldHistStyles, *newHistStyles;
    hsGeneral *firstItem;
    Pixel black;
    Display *display;
    Colormap cmap;
    
    /* If data is not already supplied, request it */
    if (!ItemIsDisplayed(item->id))
    	RequestUpdates(item->id);

    /* Pop down dialogs which will be invalid in the new context */
    if (window->curveStyleDlg != NULL) {
    	XtDestroyWidget(window->curveStyleDlg);
    	window->curveStyleDlg = NULL;
    }
    if (window->histStyleDlg != NULL) {
    	XtDestroyWidget(window->histStyleDlg);
    	window->histStyleDlg = NULL;
    }
    if (window->statsWindow != NULL) {
    	XtDestroyWidget(XtParent(window->statsWindow));
    	window->statsWindow = NULL;
    }

    /* Create a new plotInfo data structure based on the function arguments */
    newPlot = createPlotInfo(item, plotType, ntVars, sliderVars, errorBars);
    newPlot->nXBins = nXBins;
    newPlot->nYBins = nYBins;
    newPlot->plotType = plotType;

    /* Add the new plot to the windowInfo structure */
    oldPlotList = window->pInfo;
    newPlotList = (plotInfo **)XtMalloc(sizeof(plotInfo *) *
    	    (window->nPlots+1));
    for (i=0; i<window->nPlots; i++)
    	newPlotList[i] = oldPlotList[i];
    newPlotList[i] = newPlot;
    window->pInfo = newPlotList;
    window->nPlots++;
    XtFree((char *)oldPlotList);

    display = XtDisplay(window->widget);
    cmap = DefaultColormapOfScreen(XtScreen(window->widget));
    black = BlackPixelOfScreen(XtScreen(window->widget));

    /* Copy optional curveStyles and histStyles to the new plots */
    if (curveStyles != NULL && nCurves != 0) {
	oldCurveStyles = (XYCurve *)window->curveStyles;
	nCurves = NCurvesInPlot(plotType, ntVars);
	newCurveStyles = (XYCurve *)XtMalloc(sizeof(XYCurve) *
    		(oldNCurves + nCurves));
	memcpy(newCurveStyles, oldCurveStyles, sizeof(XYCurve) * oldNCurves);
	memcpy(&newCurveStyles[oldNCurves], (XYCurve *)curveStyles,
	    	sizeof(XYCurve) * nCurves);
	for (i=0, curve=&newCurveStyles[oldNCurves]; i<nCurves; i++, curve++)
    	    curve->name = XmStringCopy(curve->name);
	for (i=oldNCurves; i<oldNCurves+nCurves; i++)
	{
	    makeUniqueCurveStyle(newCurveStyles, oldNCurves+nCurves, i);
	    if (newCurveStyles[i].markerPixel != black)
		if (duplicateColor(display, cmap, newCurveStyles[i].markerPixel,
				   &newCurveStyles[i].markerPixel))
		    newCurveStyles[i].markerPixel = black;
	    if (newCurveStyles[i].linePixel != black)
		if (duplicateColor(display, cmap, newCurveStyles[i].linePixel,
				   &newCurveStyles[i].linePixel))
		    newCurveStyles[i].linePixel = black;
	}
	if (oldCurveStyles != NULL)
    	    XtFree((char *)oldCurveStyles);
	window->curveStyles = newCurveStyles;
	window->nCurves += nCurves;
    }
    if (histStyle != NULL && PlotIsHist(plotType)) {
	oldHistStyles = (XYHistogram *)window->histStyles;
	newHistStyles = (XYHistogram *)XtMalloc(sizeof(XYHistogram) *
	    	(oldNHists + 1));
	memcpy(newHistStyles, oldHistStyles, sizeof(XYHistogram) * oldNHists);
	memcpy(&newHistStyles[oldNHists], histStyle, sizeof(XYHistogram));
	newHistStyles[oldNHists].name =
	    	XmStringCopy(((XYHistogram *)histStyle)->name);
	makeUniqueHistStyle(newHistStyles, oldNHists+1, oldNHists);
	if (newHistStyles[oldNHists].linePixel != black)
	    if (duplicateColor(display, cmap, newHistStyles[oldNHists].linePixel,
			       &newHistStyles[oldNHists].linePixel))
		newHistStyles[oldNHists].linePixel = black;
	if (newHistStyles[oldNHists].fillPixel != black)
	    if (duplicateColor(display, cmap, newHistStyles[oldNHists].fillPixel,
			       &newHistStyles[oldNHists].fillPixel))
		newHistStyles[oldNHists].fillPixel = black;
	if (oldHistStyles != NULL)
    	    XtFree((char *)oldHistStyles);
	window->histStyles = newHistStyles;
	window->nHists++;
    }

    /* Redisplay the plot in "OVERLAY" mode to apply changes and maintain
       scaling */
    RedisplayPlotWindow(window, OVERLAY);
    
    /* Update the plot menu to reflect the new plot contents */
    RemovePlotMenu(window);
    firstItem = GetMPItemByID(window->pInfo[0]->id);
    CreateOverlayMenu(window, firstItem->type == HS_NTUPLE);
}

void RemoveOverlayed(windowInfo *window, int index)
{
    int i;
    XYCurve *curve, *c, *curveStyles = (XYCurve *)window->curveStyles;
    XYHistogram *hist, *histStyles = (XYHistogram *)window->histStyles;
    int nCurvesInOldPlot, plotType;
    Pixel black;
    Display *display;
    Colormap cmap;

    /* Pop down dialogs which will be invalid in the new context */
    if (window->curveStyleDlg != NULL) {
    	XtDestroyWidget(window->curveStyleDlg);
    	window->curveStyleDlg = NULL;
    }
    if (window->histStyleDlg != NULL) {
    	XtDestroyWidget(window->histStyleDlg);
    	window->histStyleDlg = NULL;
    }
    if (window->statsWindow != NULL) {
    	XtDestroyWidget(XtParent(window->statsWindow));
    	window->statsWindow = NULL;
    }
    if (index == 0) {
    	if (window->sliderWindow != NULL) {
    	    XtDestroyWidget(XtParent(window->sliderWindow));
    	    window->sliderWindow = NULL;
	}
    	if (window->rebinWindow != NULL) {
    	    XtDestroyWidget(XtParent(window->rebinWindow));
    	    window->rebinWindow = NULL;
	}
    }

    display = XtDisplay(window->widget);
    cmap = DefaultColormapOfScreen(XtScreen(window->widget));
    black = BlackPixelOfScreen(XtScreen(window->widget));

    /* remove associated styles from the window's curve styles list */
    curve = curveStyles;
    nCurvesInOldPlot = NCurvesInPlot(window->pInfo[index]->plotType,
    	    	window->pInfo[index]->ntVars);
    for (i=0; i<index; i++)
    	curve += NCurvesInPlot(window->pInfo[i]->plotType,
    	    	window->pInfo[i]->ntVars);
    for (c=curve; c<curve+nCurvesInOldPlot; c++)
    {
	if (c->linePixel != black)
	    XFreeColors(display, cmap, &c->linePixel, 1, 0);
	if (c->markerPixel != black)
	    XFreeColors(display, cmap, &c->markerPixel, 1, 0);
    	XmStringFree(c->name);
    }
    for (; c<curveStyles + window->nCurves; c++, curve++)
    	*curve = *c;
    window->nCurves -= nCurvesInOldPlot;
    
    /* remove associated styles from the window's hist styles list */
    hist = histStyles;
    if (PlotIsHist(window->pInfo[index]->plotType)) {
    	for (i=0; i<index; i++)
    	    hist += PlotIsHist(window->pInfo[i]->plotType);
    	XmStringFree(hist->name);
	if (hist->linePixel != black)
	    XFreeColors(display, cmap, &hist->linePixel, 1, 0);
	if (hist->fillPixel != black)
	    XFreeColors(display, cmap, &hist->fillPixel, 1, 0);
    	for (; hist<histStyles+window->nHists-1; hist++)
    	    *hist = *(hist+1);
    	window->nHists--;
    }

    /* remove the plot from the plot list */
    for (i=index; i<window->nPlots-1; i++)
    	window->pInfo[i] = window->pInfo[i+1];
    window->nPlots--;

    /* Update the plot menu to reflect the new plot contents */
    RemovePlotMenu(window);
    if (window->nPlots == 1) {
    	plotType = window->pInfo[0]->plotType;
    	if (plotType == HIST1D)
    	    Create1DHistMenu(window,
    	    	    GetMPItemByID(window->pInfo[0]->id)->type == HS_NTUPLE);
    	else if (plotType == TSPLOT || plotType == TSPLOTERR)
	    CreateTSPlotMenu(window);
	else if (plotType == XYPLOT || plotType == XYPLOTERR)
	    CreateXYPlotMenu(window);
	else if (plotType == XYSORT || plotType == XYSORTERR)
	    CreateXYSortMenu(window);
	else if (plotType == ADAPTHIST1D)
	    Create1DAdaptHistMenu(window);
    } else
    	CreateOverlayMenu(window, 
    	    	GetMPItemByID(window->pInfo[0]->id)->type == HS_NTUPLE);
    
    /* Redisplay the plot */
    RedisplayPlotWindow(window, OVERLAY);
}

/*
** Turn graphics buffering on and off for all displayed
** windows, and all subsequently displayed ones.
*/
void BufferGraphics(int state)
{
    windowInfo *w;

    GraphicsBuffering = state;
    for (w=WindowList; w!=NULL; w=w->next)
    	setWidgetBuffering(w->widget, state);
}
int GetGraphicsBuffering(void)
{
    return GraphicsBuffering;
}

/*
** Invalidate all plot windows that depend on data from a particular item.
** Invalidating a window means marking it as no longer connected with a source
** of data such as a process connection or file.
*/
void InvalidateItem(int id)
{
    windowInfo *w;
    int p;

    /* find windows that contain plots with matching ids and invalidate them */
    for (w=WindowList; w!=NULL; w=w->next)
    	for (p=0; p<w->nPlots; p++)
    	    if (w->pInfo[p]->id == id)
    	    	invalidateWindow(w);
}

/*
** The main panel needs to renumber items when a new file is opened or new
** process is connected, to avoid conflicts with new ids.  This updates
** the plot window list when an item id changes
*/
void ChangeWindowItemID(int oldID, int newID)
{
    windowInfo *w;
    int p;

    /* find windows that match id and renumber them */
    for (w=WindowList; w!=NULL; w=w->next)
    	for (p=0; p<w->nPlots; p++)
    	    if (w->pInfo[p]->id == oldID) w->pInfo[p]->id = newID;
}


/*
** Count the number of plot windows displayed
*/
int CountPlotWindows(void)
{
    windowInfo *w;
    int n = 0;
    
    for (w=WindowList; w!=NULL; w=w->next)
    	if (!w->multPlot)			/* do not count mini-plots */
    	    n++;
    return n;
}

/*
** Get the names and top level X windows for all of the plot windows
** currently displayed (so a windows menu can be created for them).  Arrays
** of length CountPlotWindows() to hold the string pointers and window
** ids must be provided by the caller.
*/
void GetPlotWindowsAndTitles(Window *windows, char **titles)
{
    windowInfo *w;
    int i;
    
    for (i=0, w=WindowList; w!=NULL; w=w->next)
    	if (!w->multPlot) {			/* do not include mini-plots */
    	    XtVaGetValues(w->shell, XmNtitle, &titles[i], NULL);
    	    windows[i] = XtWindow(w->shell);
    	    ++i;
        }
}

/*
** Invalidate all plot windows currently displayed regardless of item id
*/
void InvalidateAllWindows(void)
{
    windowInfo *w;

    for (w=WindowList; w!=NULL; w=w->next)
    	invalidateWindow(w);
}

static void invalidateWindow(windowInfo *wInfo)
{
    Pixel foreground, background;
    Pixmap pixmap;
    int i;
    
    /* create dotted outline around invalidated window */
    XtVaGetValues(wInfo->widget, XmNforeground, &foreground,
    		  XmNbackground, &background, NULL);
    pixmap = XmGetPixmap(XtScreen(wInfo->widget),
	    	"invalidWindowShadow", foreground, background);
    XtVaSetValues(wInfo->widget, XmNtopShadowPixmap, pixmap,
    		  XmNbottomShadowPixmap, pixmap, NULL);

    /* set the ids to not match any subsequent valid ids */
    for (i=0; i<wInfo->nPlots; i++)
    	wInfo->pInfo[i]->id = -1;
    
    /* close slider windows and dim out menu items that require fresh data */
    if (wInfo->rebinMenuItem != NULL) {
    	XmToggleButtonSetState(wInfo->rebinMenuItem, False, True);
    	XtSetSensitive(wInfo->rebinMenuItem, False);
    }
    if (wInfo->sliderMenuItem != NULL) {
    	XmToggleButtonSetState(wInfo->sliderMenuItem, False, True);
    	XtSetSensitive(wInfo->sliderMenuItem, False);
    }
    if (wInfo->statsMenuItem != NULL) {
    	XmToggleButtonSetState(wInfo->statsMenuItem, False, True);
    	XtSetSensitive(wInfo->statsMenuItem, False);
    }
    if (wInfo->coordsMenuItem != NULL) {
    	XmToggleButtonSetState(wInfo->coordsMenuItem, False, True);
    	XtSetSensitive(wInfo->coordsMenuItem, False);
    }
    if (wInfo->errorDataMenuItem != NULL)
    	XtSetSensitive(wInfo->errorDataMenuItem, False);
    if (wInfo->gaussErrorMenuItem != NULL)
	XtSetSensitive(wInfo->gaussErrorMenuItem, False);
    if (wInfo->autoUpMenuItem != NULL) {
	XtSetSensitive(wInfo->autoUpMenuItem, False);
	XmToggleButtonSetState(wInfo->autoUpMenuItem, False, True);
    }
    if (wInfo->updateMenuItem != NULL)
    	XtSetSensitive(wInfo->updateMenuItem, False);

    /* Gray out the "remove overlay" menu */
    if (wInfo->nPlots > 1 && wInfo->menu) {
	Widget menu = XtNameToWidget(wInfo->menu, "*removeOvly");
	if (menu) XtSetSensitive(menu, False);
    }
}

/*
** Xt timer procedure for refreshing windows with newly arrived data.
** Removes itself when no windows are left that need updating.
**
** Refreshing is done as a "background" task, rather than each time data
** arrives, because redrawing can be time consuming, and if the next
** update comes before everything is redrawn, it will just have to be
** redrawn again.  An Xt timer procedure with a duration of 0 msec
** is used rather than a work procedure, which would be more natural,
** because the X toolkit can't handle work procs without XtMainLoop, and
** we aren't given the hooks to emulate their handling in our main loop.
*/
static void refreshWorkProc(XtPointer client_data, XtIntervalId *id)
{
    windowInfo *w, *tmp;
    int i, haveData, ready, runFutureCallbacks = 0;
    hsGeneral *item;
    unsigned long interval = 50;

    /* find normal windows which have "ready" callbacks */
    for (tmp=WindowList; tmp!=NULL; ) {
	w = tmp;
	tmp = tmp->next;
	if (w->readyCB && w->multPlot == 0) {
	    ready = 1;
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
	    if (ready)
		executeReadyCallbacks(w);
	    else
		runFutureCallbacks = 1;
	}
    }

    /* find multiplot windows which have "ready" callbacks */
    if (MPlotRefreshCycle())
	runFutureCallbacks = 1;

    /* find the first window on the list that needs updating */
    for (w=WindowList; w!=NULL; w=w->next) {
    	if (w->needsUpdate && overlayedObjectsReady(w->decor))
    	    break;
    }
    if (w)
    {
	/* refresh the window that was marked as needing update */
	RedisplayPlotWindow(w, w->needsData ? REINIT : UPDATE);
	UpdateStatsWindow(w);
	UpdateSliderRange(w);
	if (w->needsData) {
	    haveData = True;
	    for (i=0; i<w->nPlots; i++)
		haveData = haveData && ItemHasData(GetMPItemByID(w->pInfo[i]->id));
	    w->needsData = !haveData;
	}
	w->needsUpdate = False;
	StarWindowTitle(w, False);
	interval = 0;
    
	/* move the window to the end of the list so others can be refreshed next */
	moveToEndOfWindowList(w);
    }

    if (w || runFutureCallbacks)
    {
	/* re-establish the timer proc (this routine) to continue processing */
	extern Widget MainPanelW;
	RefreshProcID = XtAppAddTimeOut(XtWidgetToApplicationContext(MainPanelW),
					interval, refreshWorkProc, NULL);
    }
    else
	RefreshProcID = 0;
}

void ReinitItem(int id)
{
    windowInfo *w;
    int p;
    
    for (w=WindowList; w!=NULL; w=w->next) {
    	for (p=0; p<w->nPlots; p++) {
    	    if (w->pInfo[p]->id == id) {
    		RedisplayPlotWindow(w, REINIT);
    		UpdateSliderRange(w);
    	    	break;
    	    }
    	}
    }	    
}

windowInfo *CreateTSPlotWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell, int isMiniPlot,
        colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xyWidgetClass,
    	    redisplay2D, title, winID, shell, isMiniPlot, TSPLOT, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
    	    (XtCallbackProc) TimeSeriesHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);

    /* add the appropriate menu items */
    CreateTSPlotMenu(wInfo);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateXYPlotWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell, int isMiniPlot,
        colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
  windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xyWidgetClass,
    	    redisplay2D, title, winID, shell, isMiniPlot, XYPLOT, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                 (XtCallbackProc)  XYHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);

    /* add the appropriate menu items */
    CreateXYPlotMenu(wInfo);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateXYSortWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell, int isMiniPlot,
	colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
  windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xyWidgetClass,
    	    redisplay2D, title, winID, shell, isMiniPlot, XYSORT, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                 (XtCallbackProc)  XYHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);

    /* add the appropriate menu items */
    CreateXYSortMenu(wInfo);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateTSPlotErrWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
  windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xyWidgetClass,
    	    redisplay2D, title, winID, shell, isMiniPlot, TSPLOTERR, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  TimeSeriesHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);

    /* add the appropriate menu items */
    CreateTSPlotMenu(wInfo);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateXYPlotErrWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
  windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xyWidgetClass,
    	    redisplay2D, title, winID, shell, isMiniPlot, XYPLOTERR, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  XYHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);

    /* add the appropriate menu items */
    CreateXYPlotMenu(wInfo);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateXYSortErrWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
  windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xyWidgetClass,
    	    redisplay2D, title, winID, shell, isMiniPlot, XYSORTERR, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  XYHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);

    /* add the appropriate menu items */
    CreateXYSortMenu(wInfo);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *Create2DScatWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    scatWidgetClass, redisplayScat, title, winID, shell,
	    isMiniPlot, SCAT2D, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  Scat2DHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget,
		  XmNwidth, 425,
		  XmNheight, 375,
		  XmNdarkerPoints, ThickenPointsScat,
		  NULL);

    /* add the appropriate menu items */
    Create2DScatMenu(wInfo);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateColor2DScatWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    scatWidgetClass, redisplayScat, title, winID, shell,
	    isMiniPlot, COLORSCAT2D, confInfo);

    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc) Scat2DHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 425, XmNheight, 375, NULL);

    /* add the appropriate menu items */
    Create2DScatMenu(wInfo);

    /* Set up the color scale settings */
    if (csInfo)
	wInfo->pInfo[0]->csi = *csInfo;
    else
    {
	wInfo->pInfo[0]->csi.colorScale = defaultColorScale();
	if (wInfo->pInfo[0]->csi.colorScale == NULL)
	{
	    fprintf(stderr, "Error in CreateColor2DScatWindow: "
		    "can't find color scale \"%s\"\n", DEFAULT_COLOR_SCALE_NAME);
	    /* The following "return" leaks memory, but it 
	       should never happen on a color display. Normal
	       cleanup is difficult at this point. */
	    return NULL;
	}
	wInfo->pInfo[0]->csi.rangeIsDynamic = 0;
    }
    incrColorScaleRefCount(wInfo->pInfo[0]->csi.colorScale);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *Create3DScatWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    scat3DWidgetClass, redisplay3DScat, title, winID, shell,
    	    isMiniPlot, SCAT3D, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                   (XtCallbackProc) Scat3DHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget,
		  XmNwidth, 425,
		  XmNheight, 375,
		  XmNdarkerPoints, ThickenPointsScat3D,
		  NULL);

    /* add the appropriate menu items */
    Create3DScatMenu(wInfo);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp3D(wInfo->widget);
    
    return wInfo;
}

windowInfo *Create1DHistWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;
    
    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    xyWidgetClass, redisplay2D, title, winID, shell, isMiniPlot,
            HIST1D, confInfo);
    
    /* set the initial number of bins for histograms from ntuple data */
    if (item->type == HS_NTUPLE) {
	wInfo->pInfo[0]->nXBins = ((hsNTuple *)item)->n / NICE_COUNTS_PER_BIN;
	if (wInfo->pInfo[0]->nXBins > MAX_1DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MAX_1DHIST_BINS;
	if (wInfo->pInfo[0]->nXBins < MIN_1DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MIN_1DHIST_BINS;
    }

    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                   (XtCallbackProc)  Hist1DHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);
    
    /* add the appropriate menu items to it */
    Create1DHistMenu(wInfo, item->type == HS_NTUPLE);
    
    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *Create2DHistWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
	int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    hist2DWidgetClass, redisplay2DHist, title, winID, shell, isMiniPlot,
    	    HIST2D, confInfo);
    
    /* set the initial number of bins for histograms from ntuple data */
    if (item->type == HS_NTUPLE) {
	wInfo->pInfo[0]->nXBins = ((hsNTuple *)item)->n / NICE_COUNTS_PER_BIN;
	if (wInfo->pInfo[0]->nXBins > MAX_2DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MAX_2DHIST_BINS;
	if (wInfo->pInfo[0]->nXBins < MIN_2DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MIN_2DHIST_BINS;
	wInfo->pInfo[0]->nYBins = wInfo->pInfo[0]->nXBins;
    }

    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                   (XtCallbackProc)  Hist2DHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 400, XmNheight, 400, NULL);

    /* add the appropriate menu items */
    Create2DHistMenu(wInfo, item->type == HS_NTUPLE);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2DHist(wInfo->widget);
    
    return wInfo;
}

windowInfo *Create1DAdaptHistWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    xyWidgetClass, redisplay2D, title, winID, shell, isMiniPlot,
            ADAPTHIST1D, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  Hist1DAHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 450, XmNheight, 325, NULL);
    
    /* set the initial bin limit value (stored in nXBins) to 1/20 of the
       number of data points, but with a minimum of 20 */
    wInfo->pInfo[0]->nXBins = ((hsNTuple *)item)->n / 20;
    if (wInfo->pInfo[0]->nXBins < 20) wInfo->pInfo[0]->nXBins = 20;
    
    /* draw the adaptive histogram with separated bins */
    XtVaSetValues(wInfo->widget, XmNbarSeparation, 1, NULL);

    /* add the appropriate menu items */
    Create1DAdaptHistMenu(wInfo);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *Create2DAdaptHistWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    hist2DWidgetClass, redisplay2DAdaptHist, title, winID, shell,
    	    isMiniPlot, ADAPTHIST2D, confInfo);
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  Hist2DAHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget, XmNwidth, 400, XmNheight, 400, NULL);
    
    /* set the initial bin limit value (stored in nXBins) to 1/20 of the
       number of data points, but with a minimum of 20 */
    wInfo->pInfo[0]->nXBins = ((hsNTuple *)item)->n / 20;
    if (wInfo->pInfo[0]->nXBins < 20) wInfo->pInfo[0]->nXBins = 20;

    /* add the appropriate menu items */
    Create2DAdaptHistMenu(wInfo);

    /* initialize the data in the histogram */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2DHist(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateCellWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    cellWidgetClass, redisplayCell, title, winID, shell,
            isMiniPlot, CELL, confInfo);
    	    
    /* set the initial number of bins for plots from ntuple data */
    if (item->type == HS_NTUPLE) {
	wInfo->pInfo[0]->nXBins = ((hsNTuple *)item)->n / NICE_COUNTS_PER_BIN;
	if (wInfo->pInfo[0]->nXBins > MAX_2DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MAX_2DHIST_BINS;
	if (wInfo->pInfo[0]->nXBins < MIN_2DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MIN_2DHIST_BINS;
	wInfo->pInfo[0]->nYBins = wInfo->pInfo[0]->nXBins;
    }
    
    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                   (XtCallbackProc) CellHelpCB, NULL);

    /* override the default initial width and height */
    XtVaSetValues(wInfo->widget,
		  XmNwidth, 425,
		  XmNheight, 375,
		  NULL);

    /* add the appropriate menu items */
    CreateCellMenu(wInfo, item->type == HS_NTUPLE);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);
    
    /* Display the window */
    XtRealizeWidget(wInfo->shell);
    
    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);
    
    return wInfo;
}

windowInfo *CreateColorCellWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo)
{
    windowInfo *wInfo;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, sliderVars,
    	    cellWidgetClass, redisplayColorCell, title, winID, shell,
	    isMiniPlot, COLORCELL, confInfo);

    /* set the initial number of bins for plots from ntuple data */
    if (item->type == HS_NTUPLE) {
	wInfo->pInfo[0]->nXBins = ((hsNTuple *)item)->n / NICE_COUNTS_PER_BIN;
	if (wInfo->pInfo[0]->nXBins > MAX_2DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MAX_2DHIST_BINS;
	if (wInfo->pInfo[0]->nXBins < MIN_2DHIST_BINS)
	    wInfo->pInfo[0]->nXBins = MIN_2DHIST_BINS;
	wInfo->pInfo[0]->nYBins = wInfo->pInfo[0]->nXBins;
    }

    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                   (XtCallbackProc) CellHelpCB, NULL);

    /* override the default initial width and height. Also specify
       that the plot will be completely filled with color. */
    XtVaSetValues(wInfo->widget,
		  XmNwidth, 425,
		  XmNheight, 375,
		  XmNcolorFilled, True,
		  NULL);

    /* add the appropriate menu items */
    CreateColorCellMenu(wInfo, item->type == HS_NTUPLE);

    /* Set up the color scale settings */
    if (csInfo)
	wInfo->pInfo[0]->csi = *csInfo;
    else
    {
	wInfo->pInfo[0]->csi.colorScale = defaultColorScale();
	if (wInfo->pInfo[0]->csi.colorScale == NULL)
	{
	    fprintf(stderr, "Error in CreateColorCellWindow: "
		    "can't find color scale \"%s\"\n", DEFAULT_COLOR_SCALE_NAME);
	    /* The following "return" leaks memory, but it 
	       should never happen on a color display. Normal
	       cleanup is difficult at this point. */
	    return NULL;
	}
	wInfo->pInfo[0]->csi.rangeIsDynamic = 1;
    }
    incrColorScaleRefCount(wInfo->pInfo[0]->csi.colorScale);

    /* initialize the data in the plot */
    RedisplayPlotWindow(wInfo, REINIT);

    /* Display the window */
    XtRealizeWidget(wInfo->shell);

    /* Display automatic help if it has not yet been shown */
    AutoHelp2D(wInfo->widget);

    return wInfo;
}

/*
** Show or hide error bars on a (non-overlaid) histogram plot.  Error bar
** state can be: NO_ERROR_BARS, DATA_ERROR_BARS, or GAUSSIAN_ERROR_BARS.
*/
int ShowErrorBars(windowInfo *wInfo, int errorBarState, int calledFromMenu)
{
    hsGeneral *item;
    int needErrors, err;
    char *errReason;

    /* Warn user if data error bars were requested and no error data avail. */
    item = GetMPItemByID(wInfo->pInfo[0]->id);
    needErrors = False;
    if (errorBarState == DATA_ERROR_BARS) {
	if (item->type == HS_1D_HISTOGRAM) {
	    if (!((hs1DHist *)item)->errFlg) {
		if (calledFromMenu) {
		    DialogF(DF_WARN, wInfo->widget, 1,
			    "No error data is stored for this histogram", "OK");
		    return False;
		} else {
		    /* Assume that config file/string knows better ... */
		    ((hs1DHist *)item)->errFlg = 1;
		    if (wInfo->errorDataMenuItem)
			XmToggleButtonSetState(wInfo->errorDataMenuItem, True, False);
		}
    	    }
    	    needErrors = ((hs1DHist *)item)->pErrs == NULL;
    	} else if (item->type == HS_2D_HISTOGRAM) {
    	    if (!((hs2DHist *)item)->errFlg) {
		if (calledFromMenu) {
		    DialogF(DF_WARN, wInfo->widget, 1,
			    "No error data is stored for this histogram", "OK");
		    return False;
		} else {
		    /* Assume that config file/string knows better ... */
		    ((hs2DHist *)item)->errFlg = 1;
		    if (wInfo->errorDataMenuItem)
			XmToggleButtonSetState(wInfo->errorDataMenuItem, True, False);
		}
    	    }
    	    needErrors = ((hs2DHist *)item)->pErrs == NULL;
	} else if (item->type == HS_NTUPLE) {
    	    DialogF(DF_WARN, wInfo->widget, 1,
    	       "Histogram data is from an ntuple.\nNo error data is available.",
    	       "OK");
    	    return False;
	} else {
	    fprintf(stderr, "Internal Error: ShowErrorBars, wrong item type\n");
    	}
    }

    /* if necessary, request error data from the client */
    if (needErrors) {
    	err = RequestErrors(wInfo->pInfo[0]->id);
    	if (err != COM_OK) {
    	    if (err == COM_ERR_NOCONNECT)
    		errReason = ":\nNot connected to client";
    	    else if (err == COM_ERR_V1CLIENT)
    		errReason = ":\nV1 clients do not support error data";
    	    else
    		errReason = "";
    	    DialogF(DF_WARN, wInfo->widget, 1,
    	    	    "Unable to fetch stored error data%s", "OK", errReason);
    	    return False;
    	}
    }

    /* set the new error bar state and redisplay the window */
    wInfo->pInfo[0]->errorBars = errorBarState;
    RedisplayPlotWindow(wInfo, REINIT);
    return True;
}

/* ClosePlotWindow closes regular plot windows.  Use CloseMiniPlot for
 *		   mini plots in a multiple-plot window.
 */
void ClosePlotWindow(windowInfo *wInfo)
{
    int i, p;
    Widget widgetToDestroy = wInfo->shell;
    readyCallBack *rcb, *tmp;
    Display *dpy = XtDisplay(wInfo->widget);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(wInfo->widget));
    Pixel black = BlackPixelOfScreen(XtScreen(wInfo->widget));
    unsigned long planes = 0;

    /* Protect from multiple calls about the same window */
    if (!existsInWindowList(wInfo)) return;

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

    /* update the windows menu in the main panel */
    UpdateWindowsMenu();

    /* Unmap the widget */
    XtUnmapWidget(widgetToDestroy);

    /* free memory  */
    if (wInfo->curveStyles != NULL) {
    	for (i=0; i<wInfo->nCurves; i++)
	{
	    Pixel markerPixel = ((XYCurve *)wInfo->curveStyles)[i].markerPixel;
	    Pixel linePixel   = ((XYCurve *)wInfo->curveStyles)[i].linePixel;
	    if (linePixel != black)
		XFreeColors(dpy, cmap, &linePixel, 1, planes);
	    if (markerPixel != black)
		XFreeColors(dpy, cmap, &markerPixel, 1, planes);
    	    XmStringFree(((XYCurve *)wInfo->curveStyles)[i].name);
	}
    	XtFree((char *)wInfo->curveStyles);
    }
    if (wInfo->histStyles != NULL) {
    	for (i=0; i<wInfo->nHists; i++)
	{
	    Pixel linePixel = ((XYHistogram *)wInfo->histStyles)[i].linePixel;
	    Pixel fillPixel = ((XYHistogram *)wInfo->histStyles)[i].fillPixel;
	    if (linePixel != black)
		XFreeColors(dpy, cmap, &linePixel, 1, planes);
	    if (fillPixel != black)
		XFreeColors(dpy, cmap, &fillPixel, 1, planes);
    	    XmStringFree(((XYHistogram *)wInfo->histStyles)[i].name);
	}
    	XtFree((char *)wInfo->histStyles);
    }
    for (p=0; p<wInfo->nPlots; p++)
    {
	decrColorScaleRefCount(wInfo->pInfo[p]->csi.colorScale);
    	XtFree((char *)wInfo->pInfo[p]);
    }
    XtFree((char *)wInfo->pInfo);

    for (rcb = wInfo->readyCB; rcb; ) {
	tmp = rcb;
	rcb = rcb->next;
	XtFree((char *)tmp);
    }
    wInfo->readyCB = NULL;

    ClearOverlayedObjects(wInfo);
    XtFree((char *)wInfo);

    /* set a timer before destroying widget, because of Motif bug in
     * phase 2 destroy (30 seconds) */
    XtAppAddTimeOut(XtWidgetToApplicationContext(widgetToDestroy), 30000, 
    	(XtTimerCallbackProc)destroyPlotCB, widgetToDestroy);
}

static void destroyPlotCB(XtPointer client_data, XtIntervalId *id)
{
    Widget widgetToDestroy  = (Widget) client_data;
    
    /* destroy the plot widget (and shell) */
    XtDestroyWidget(widgetToDestroy);
}

void CloseAllPlotWindows(void)
{
    while (WindowList != NULL)
	if (WindowList->multPlot)
    	    CloseMPlotFromWinfo(WindowList);
	else
    	    ClosePlotWindow(WindowList);
}

/*
** Check if a window is listed in the list of windows
*/
int existsInWindowList(windowInfo *window)
{
    windowInfo *temp;

    if (window)
	for (temp = WindowList; temp != NULL; temp = temp->next)
	    if (temp == window)
		return 1;
    return 0;
}

/*
** Remove a window from the list of windows
*/
void RemoveFromWindowList(windowInfo *window)
{
    windowInfo *temp;

    if (WindowList == window)
	WindowList = window->next;
    else {
	for (temp = WindowList; temp != NULL; temp = temp->next) {
	    if (temp->next == window) {
		temp->next = window->next;
		break;
	    }
	}
    }
}

void StarWindowTitle(windowInfo *wInfo, int state)
{
    char *newTitle, *oldTitle;
    
    if (wInfo->titleStarred == state || wInfo->multPlot)
    	return;
    
    XtVaGetValues(wInfo->shell, XmNtitle, &oldTitle, NULL);
    newTitle = XtMalloc(strlen(oldTitle)+2);
    if (state == True)
    	sprintf(newTitle, "*%s", oldTitle);
    else
    	strcpy(newTitle, oldTitle+1);
    XtVaSetValues(wInfo->shell, XmNtitle, newTitle, NULL);
    XtFree(newTitle);
    wInfo->titleStarred = state;
}
  
windowInfo *CreateGenericWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, WidgetClass plotWidgetClass,
	redisplayProc redisplay, char *title, char *winID, Widget shell,
	int isMiniPlot, int plotType, widgetConfigInfo *confInfo)
{
    Widget plotWidget;
    windowInfo *wInfo;
    plotInfo **pInfoList;
    Arg *args = NULL;
    int nargs = 0;

    /* If data is not already supplied, request it */
    if (!ItemIsDisplayed(item->id))
    	RequestUpdates(item->id);

    /* allocate a plotInfo structure to hold plot-specific information */
    pInfoList = (plotInfo **)XtMalloc(sizeof(plotInfo *));
    pInfoList[0] = createPlotInfo(item, plotType, ntVars, sliderVars,
    	    NO_ERROR_BARS);

    /* allocate a windowInfo structure to hold information about the window,
       and add it to the list of plot windows */
    wInfo = (windowInfo *)XtMalloc(sizeof(windowInfo));
    wInfo->next = WindowList;
    WindowList = wInfo;

    if (confInfo)
    {
	args = confInfo->args;
	nargs = confInfo->nargs;
    }
    if (isMiniPlot)
	plotWidget = XtCreateWidget(
	    "plotWidget", plotWidgetClass, parent, args, nargs);
    else
	plotWidget = XtCreateManagedWidget(
	    "plotWidget", plotWidgetClass, parent, args, nargs);

    /* fill in the window and plot info data structures */
    if (winID)
    {
	strncpy(wInfo->windowID, winID, MAX_WINDOWID_LEN - 1);
	wInfo->windowID[MAX_WINDOWID_LEN - 1]  = '\0';
    }
    else
	wInfo->windowID[0] = '\0';
    wInfo->xlabel[0] = '\0';
    wInfo->ylabel[0] = '\0';
    wInfo->pInfo = pInfoList;
    wInfo->curveStyles = NULL;
    wInfo->histStyles = NULL;
    wInfo->nCurves = 0;
    wInfo->nHists = 0;
    wInfo->nPlots = 1;
    wInfo->update = True;
    wInfo->needsUpdate = False;
    wInfo->needsData = !ItemHasData(item);
    wInfo->titleStarred = False;
    wInfo->growOnly = False;
    wInfo->multPlot = isMiniPlot;
    wInfo->cellNormMin = -FLT_MAX;
    wInfo->cellNormMax = FLT_MAX;
    wInfo->redisplay = redisplay;
    wInfo->shell = shell;
    wInfo->widget = plotWidget;
    wInfo->menu = NULL;		/* filled in later */
    wInfo->rebinMenuItem = NULL;
    wInfo->cellNormMenuItem = NULL;
    wInfo->cellLogA = NULL;
    wInfo->sliderMenuItem = NULL;
    wInfo->statsMenuItem = NULL;
    wInfo->coordsMenuItem = NULL;
    wInfo->autoUpMenuItem = NULL;
    wInfo->updateMenuItem = NULL;
    wInfo->gaussErrorMenuItem = NULL;
    wInfo->backplanesMenuItem = NULL;
    wInfo->binEdgeMenuItem = NULL;
    wInfo->thickenMenuItem = NULL;
    wInfo->legendMenuItem = NULL;
    wInfo->errorDataMenuItem = NULL;
    wInfo->splitHalfMenuItem = NULL;
    wInfo->cntrOfGravMenuItem = NULL;
    wInfo->growOnlyMenuItem = NULL;
    wInfo->showTitlesMenuItem = NULL;
    wInfo->axisSettings = NULL;
    wInfo->rebinWindow = NULL;
    wInfo->cellNormWindow = NULL;
    wInfo->sliderWindow = NULL;
    wInfo->statsWindow = NULL;
    wInfo->coordsWindow = NULL;
    wInfo->coAbs = NULL;
    wInfo->coRel = NULL;
    wInfo->coPlot = NULL;
    wInfo->curveStyleDlg = NULL;
    wInfo->histStyleDlg = NULL;
    wInfo->setRowColDlg = NULL;
    wInfo->colorDialog = NULL;
    wInfo->readyCB = NULL;
    wInfo->decor = NULL;

    /* set the correct graphics buffering state for widgets that do it */
    setWidgetBuffering(plotWidget, GraphicsBuffering);
    
    /* Store a set of normal shadow pixmaps (this is just done once for the
       first plot window created) to use for restoring normal shadows after
       error shadows were used to indicate an error */
    if (!HaveNormalShadows)
    	XtVaGetValues(plotWidget, XmNtopShadowPixmap, &NormalTopPixmap,
    	    XmNbottomShadowPixmap, &NormalBottomPixmap, NULL);
    
    /* Allow the plot to be dragged into a multi-plot window */
    MakePlotADragSource(plotWidget);
    
    /* Allow an XY Plot to accept other (dragged) XY Plots for overlaying */
    if (plotWidgetClass == xyWidgetClass)
    	RegisterWidgetAsPlotReceiver(plotWidget);
    
    /* Add a close callback to the window frame */
    if (!isMiniPlot)
    	AddMotifCloseCallback(wInfo->shell, (XtCallbackProc)closeCB, wInfo);

    /* Install the string drawing callback */
    XtAddCallback(plotWidget, XmNredisplayCallback,
		  drawOverCB, wInfo);
    XtAddCallback(plotWidget, XmNresizeCallback,
		  drawOverCB, wInfo);

    /* Install callback for priniting coordinates */
    XtAddCallback(plotWidget, XmNbtn3Callback,
                  printCoordsCB, wInfo);

    return wInfo;
}

/*
** Allocate and initialize a plotInfo data structure
*/
static plotInfo *createPlotInfo(hsGeneral *item, int plotType, int *ntVars,
    	int *sliderVars, int errorBars)
{
    plotInfo *pInfo;
    int i, nSliderVars=0;
    
    pInfo = (plotInfo *)XtMalloc(sizeof(plotInfo));
    pInfo->id = item->id;
    pInfo->plotType = plotType;
    pInfo->nXBins = 0;
    pInfo->nYBins = 0;
    pInfo->errorBars = errorBars;
    pInfo->aHistBinStrategy = SPLIT_IN_HALF;
    if (item->type == HS_NTUPLE) {
    	for (i=0; i<MAX_DISP_VARS; i++)
    	    pInfo->ntVars[i] = ntVars[i];
    } else {
    	for (i=0; i<MAX_DISP_VARS; i++)
    	    pInfo->ntVars[i] = -1;
    }
    for (i=0; i<N_SLIDERS; i++) {
    	pInfo->sliderThresholds[i] = FLT_MAX;
    	pInfo->sliderGTorLT[i] = SLIDER_LT;
   	pInfo->sliderVars[i] = -1;
    }
    if (sliderVars != NULL && item->type == HS_NTUPLE) {
    	for (i=0; i<N_SLIDERS; i++)
    	    if (sliderVars[i] != -1)
    	    	pInfo->sliderVars[nSliderVars++] = sliderVars[i];
	pInfo->nSliders = nSliderVars;
    } else
    	pInfo->nSliders = 0;
    pInfo->csi.colorScale = NULL;
    pInfo->csi.colorMin = FLT_MAX;
    pInfo->csi.colorMax = -FLT_MAX;
    pInfo->csi.colorScale = NULL;
    pInfo->csi.colorIsLog = 0;
    pInfo->csi.rangeIsDynamic = 0;
    return pInfo;
}

/*
** Return true if any currently displayed windows represent an item
** with the given id
*/
int ItemIsDisplayed(int id)
{
    windowInfo *w;
    int p;
    
    for (w=WindowList; w!=NULL; w=w->next)
    	for (p=0; p<w->nPlots; p++)
    	    if (w->pInfo[p]->id == id)
	    	return True;
    return False;	    
}

/*
** Return first found windowInfo pointer of an item with the given id, 
** or NULL if the item has not been displayed
*/
windowInfo *FirstDisplayedItem(int id)
{
    windowInfo *w;
    int p;

    for (w=WindowList; w!=NULL; w=w->next)
    	for (p=0; p<w->nPlots; p++)
    	    if (w->pInfo[p]->id == id)
		return w;
    return NULL;	    
}

/*
** Return first found windowInfo pointer of a plot with the given widget 
** pointer, or NULL if the widget wasn't found.
*/
windowInfo *GetWinfoFromWidget(Widget w)
{
    windowInfo *wInfo;
    
    for (wInfo=WindowList; wInfo!=NULL; wInfo=wInfo->next) {
    	if (wInfo->widget == w)
	    return wInfo;
    }
    return NULL;	    
}

/*
** Turn on or off graphics double buffering on the widgets that support it
*/
static void setWidgetBuffering(Widget w, int state)
{
    WidgetClass class = XtClass(w);
    
    if (class == hist2DWidgetClass || class == h1DWidgetClass ||
            class == scatWidgetClass || class == scat3DWidgetClass ||
            class == xyWidgetClass)
	XtVaSetValues(w, XmNdoubleBuffer, state, NULL);
}


/*
** Move a window to the end of the window list to reduce it's priority
** for being refreshed
*/
static void moveToEndOfWindowList(windowInfo *window)
{
    windowInfo *w;

    RemoveFromWindowList(window);
    if (WindowList == NULL) {
    	WindowList = window;
    } else {
    	for (w=WindowList; w->next!=NULL; w=w->next);
    	w->next = window;
    	window->next = NULL;
    }
}

/*
** Set or clear special shadows for indicating computation errors on
** user defined ntuple variables
*/
static void setErrorIndicator(windowInfo *window, int state)
{
    Pixel foreground, background;
    Pixmap pixmap, topPixmap, bottomPixmap;
    
    /* Don't do anything if the window already has invalid-window shadows */
    if (window->pInfo[0]->id == -1)
    	return;
    	
    /* Check if the widget shadow pixmaps are already correct */
    XtVaGetValues(window->widget, XmNtopShadowPixmap, &topPixmap,
    	    XmNbottomShadowPixmap, &bottomPixmap, NULL);
    if (state == (topPixmap != NormalTopPixmap))
    	return;
    
    /* Set the window shadow to normal or error */
    if (state) {
	XtVaGetValues(window->widget, XmNforeground, &foreground,
    		XmNbackground, &background, NULL);
	pixmap = XmGetPixmap(XtScreen(window->widget),
		"errorWindowShadow", foreground, background);
	XtVaSetValues(window->widget, XmNtopShadowPixmap, pixmap,
    		XmNbottomShadowPixmap, pixmap, NULL);
    } else
	XtVaSetValues(window->widget, XmNtopShadowPixmap, NormalTopPixmap,
    		XmNbottomShadowPixmap, NormalBottomPixmap, NULL);
}

/*
** Re-incorporate the data into the 2D plot in "window" according to
** redisplay mode "mode".
*/
static void redisplay2D(windowInfo *window, int mode)
{
    int i, p, nHists = 0, nCurves = 0;
    int rescaleMode, oldNCurves, oldNHists;
    Boolean showLegend, oldShowLegend;
    hsGeneral *item;
    XmString oldXLabel, oldYLabel, xLabel = NULL, yLabel = NULL;
    Pixel black = BlackPixelOfScreen(XtScreen(window->widget));
    XYCurve *curves, *curve, *cStyle, *oldCurveStyles, *newCurveStyles;
    XYHistogram *hists, *hist, *hStyle, *oldHistStyles, *newHistStyles;
    plotInfo *pInfo;
    
    /* Convert the necessary data for the plot(s) into XYCurve and
       XYHistogram format for display by the XY widget */
    hists = (XYHistogram *)XtMalloc(sizeof(XYHistogram) * window->nPlots);
    curves = (XYCurve *)XtMalloc(sizeof(XYCurve) * window->nPlots *
    	    MAX_DISP_VARS);
    for (p=0; p<window->nPlots; p++) {
    	pInfo = window->pInfo[p];
    	item = GetMPItemByID(pInfo->id);
    	if (item->type == HS_1D_HISTOGRAM)
    	    histToHistData((hs1DHist *)item, pInfo, &yLabel, &hists[nHists++]);
    	else if (item->type == HS_NTUPLE) {
    	    if (pInfo->plotType == TSPLOT)
    	    	ntupleToTimeSeriesCurves((hsNTuple *)item, pInfo, curves,
    	    	    	&nCurves);
    	    else if (pInfo->plotType == TSPLOTERR)
    	    	ntupleToTSErrCurves((hsNTuple *)item, pInfo, curves,
    	    	    	&nCurves);
    	    else if (pInfo->plotType == XYPLOT)
    	    	ntupleToXYCurves((hsNTuple *)item, pInfo, &xLabel, curves,
    	    	    	&nCurves);
    	    else if (pInfo->plotType == XYPLOTERR)
    	    	ntupleToXYErrCurves((hsNTuple *)item, pInfo, &xLabel, curves,
    	    	    	&nCurves);
    	    else if (pInfo->plotType == XYSORT)
    	    	ntupleToXYSortCurves((hsNTuple *)item, pInfo, &xLabel, curves,
    	    	    	&nCurves);
     	    else if (pInfo->plotType == XYSORTERR)
     	    	ntupleToXYSortErrCurves((hsNTuple *)item, pInfo, &xLabel,
     	    	    	curves, &nCurves);
   	    else if (pInfo->plotType == HIST1D)
    	    	ntupleToHistData((hsNTuple *)item, pInfo, &hists[nHists++]);
    	    else if (pInfo->plotType == ADAPTHIST1D)
    	    	ntupleToAdaptHistData((hsNTuple *)item, pInfo,&hists[nHists++]);
    	} else
    	    return; /* Wrong item type, can't redisplay */
    }
    
    /* Bring the window curve and histogram style information up to date if new
       curves or histograms have been added (by overlay).  The window style
       information is used for XY style dialogs and maintaining continuity of
       styles across plot changes). */
    if (window->nCurves < nCurves) {
	oldCurveStyles = window->curveStyles;
	newCurveStyles = (XYCurve *)XtMalloc(sizeof(XYCurve) * nCurves);
	oldNCurves = window->nCurves;
	memcpy(newCurveStyles, oldCurveStyles, sizeof(XYCurve) * oldNCurves);
	for (i=oldNCurves, curve=&curves[oldNCurves],
	    	cStyle=&newCurveStyles[oldNCurves];
	    	i<nCurves; i++, curve++, cStyle++) {
	    cStyle->name = XmStringCopy(curve->name);
	    cStyle->points = NULL;
	    cStyle->horizBars = NULL;
	    cStyle->vertBars = NULL;
    	    cStyle->markerStyle = XY_NO_MARK;
    	    cStyle->markerSize = XY_SMALL;
    	    cStyle->lineStyle = XY_PLAIN_LINE;
    	    cStyle->markerPixel = black;
    	    cStyle->linePixel = black;
	}
	for (i=oldNCurves; i<nCurves; i++)
	    makeUniqueCurveStyle(newCurveStyles, nCurves, i);
	if (oldCurveStyles != NULL)
    	    XtFree((char *)oldCurveStyles);
	window->curveStyles = newCurveStyles;
	window->nCurves = nCurves;
    }
    if (window->nHists < nHists) {
	oldHistStyles = window->histStyles;
	newHistStyles = (void *)XtMalloc(sizeof(XYHistogram) * nHists);
	oldNHists = window->nHists;
	memcpy(newHistStyles, oldHistStyles, sizeof(XYHistogram) * oldNHists);
	for (i=oldNHists, hist=&hists[oldNHists],
	    	hStyle=&newHistStyles[oldNHists];
	    	i<nHists; i++, hist++, hStyle++) {
    	    hStyle->name = XmStringCopy(hist->name);
    	    hStyle->bins = NULL;
    	    hStyle->edges = NULL;
    	    hStyle->errorBars = NULL;
    	    hStyle->barSeparation = 0.;
    	    hStyle->lineStyle = XY_PLAIN_LINE;
    	    hStyle->fillStyle = XY_NO_FILL;
    	    hStyle->linePixel = black;
    	    hStyle->fillPixel = black;
	}
	for (i=oldNHists; i<nHists; i++)
	    makeUniqueHistStyle(newHistStyles, nHists, i);
	if (oldHistStyles != NULL)
	    XtFree((char *)oldHistStyles);
	window->histStyles = newHistStyles;
	window->nHists = nHists;
    }

    /* Set the curve and histogram style according to the saved style info */
    for (i=0, hist=hists, hStyle=(XYHistogram *)window->histStyles;
	    i<nHists; i++, hist++, hStyle++) {
    	hist->lineStyle = hStyle->lineStyle;
    	hist->fillStyle = hStyle->fillStyle;
    	hist->linePixel = hStyle->linePixel;
    	hist->fillPixel = hStyle->fillPixel;
    }
    for (i=0, curve=curves, cStyle=(XYCurve *)window->curveStyles;
	    i<nCurves; i++, curve++, cStyle++) {
    	curve->markerStyle = cStyle->markerStyle;
    	curve->markerSize = cStyle->markerSize;
    	curve->lineStyle = cStyle->lineStyle;
    	curve->markerPixel = cStyle->markerPixel;
    	curve->linePixel = cStyle->linePixel;
    }

    /* Set the axis labels and decide whether to show a legend.  If there's
       just a single curve, put the name on the Y axis.  For a single
       histogram, put the name on the X axis.  XY Plots can explicitly set
       the X axis label, and histograms can explicitly set the y axis
       labels (above).  Clear these labels in overlay mode.  (it would be
       nicer but more complicated to check if the labels were the same and
       allow them to stay up). */
    /* 08/25/2002 igv -- allow the user to specify overlay labels */
    if (mode == REINIT || mode == OVERLAY || 
	window->xlabel[0] || window->ylabel[0]) {
	if (nCurves == 0 && nHists == 0) { /* empty plot */
	    xLabel = yLabel = NULL;
	    showLegend = False;
	} else if (nCurves == 0 && nHists == 1) { /* single histogram */
	    if (window->xlabel[0])
	    {
		if (xLabel != NULL) XmStringFree(xLabel);
		xLabel = XmStringCreateSimple(window->xlabel);
	    }
	    else
		xLabel = XmStringCopy(hists[0].name);
	    if (window->ylabel[0])
	    {
		if (yLabel != NULL) XmStringFree(yLabel);
		yLabel = XmStringCreateSimple(window->ylabel);
	    }
	    showLegend = False;
	} else if (nCurves == 1 && nHists == 0) { /* single curve */
	    if (window->xlabel[0])
	    {
		if (xLabel != NULL) XmStringFree(xLabel);
		xLabel = XmStringCreateSimple(window->xlabel);
	    }
	    if (window->ylabel[0])
	    {
		if (yLabel != NULL) XmStringFree(yLabel);
		yLabel = XmStringCreateSimple(window->ylabel);
	    }
	    else
		yLabel = XmStringCopy(curves[0].name);
	    showLegend = False;
	} else { /* multiple plots overlayed */
	    if (yLabel != NULL) /* histograms may have set yLabel */
	    	XmStringFree(yLabel);
	    if (xLabel != NULL) /* XY plots may have set xLabel */
	    	XmStringFree(xLabel);
	    xLabel = yLabel = NULL;
	    if (window->xlabel[0])
		xLabel = XmStringCreateSimple(window->xlabel);
	    if (window->ylabel[0])
		yLabel = XmStringCreateSimple(window->ylabel);
	    if (window->legendMenuItem != NULL)
		showLegend = XmToggleButtonGetState(window->legendMenuItem);
	    else
		showLegend = True;
	}
	/* igv: setting the labels causes the Expose event to be sent
	 * to the window. Therefore, it is much better not to set
	 * the labels in case they do not change.
	 */
 	XtVaGetValues(window->widget,
		      XmNshowLegend, &oldShowLegend,
		      XmNxAxisLabel, &oldXLabel,
		      XmNyAxisLabel, &oldYLabel, NULL);
	if (XmStringCompare(xLabel, oldXLabel) == False ||
	    XmStringCompare(yLabel, oldYLabel) == False ||
	    oldShowLegend != showLegend)
	    XtVaSetValues(window->widget,
			  XmNxAxisLabel, xLabel,
			  XmNyAxisLabel, yLabel,
			  XmNshowLegend, showLegend, NULL);
    }
    if (xLabel != NULL) XmStringFree(xLabel);
    if (yLabel != NULL) XmStringFree(yLabel);

    /* Determine the rescale mode to use for integrating the new data */
    if (mode == REINIT) rescaleMode = XY_RESCALE;
    else if (mode == ANIMATION) rescaleMode = XY_NO_RESCALE;
    else if (mode == REFRESH) rescaleMode = XY_REFRESH;
    else if (mode == REBIN) rescaleMode = XY_REBIN_MODE;
    else if (mode == UPDATE) rescaleMode = (hasActiveSliders(window) ||
    	    window->growOnly) ? XY_GROW_ONLY : XY_RESCALE_AT_MAX;
    else /* mode == OVERLAY */ rescaleMode = XY_RESCALE_AT_MAX;
    
    /* Apply the data to the widget */
    XYSetCurves(window->widget, curves, nCurves, rescaleMode, False);
    XYSetHistograms(window->widget, hists, nHists, rescaleMode, True);
    
    /* Free the data used for passing to the widget */
    freeCurves(curves, nCurves);
    freeHists(hists, nHists);
}
    
static void ntupleToHistData(hsNTuple *ntuple, plotInfo *pInfo,
    	XYHistogram *hist)
{
    float *bins, x, min, max;
    int nBins = pInfo->nXBins;
    int i, var = pInfo->ntVars[0];
    ntupleExtension *ntExt = GetNTupleExtension(ntuple->id);
    char *name;
    
    /* Calculate the range of the ntuple data, but fix the
       interval between the min and max to make it > 0 */
    ExtCalcNTVarRange(ntuple, ntExt, var, &min, &max);
    if (min == max) {
/* Change by igv:                           */
/*      	min = (int)min - (nBins-1); */
/*      	max = (int)max + (nBins-1); */
    	min = (int)min - 1;
    	max = (int)max + 1;
    }

    /* Allocate memory for the bins and fill the histogram  Add
       an extra bin at the maximum end because by definition, the
       maximum value(s) will end up in this overflow bin. */
    bins = (float *)XtMalloc(sizeof(float) * (nBins+1));
    for (i=0; i<=nBins; i++)
    	bins[i] = 0;
    for (i=0; i<ntuple->n; i++) {
	if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
	    	pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
	    	ExtNTRef(ntuple, ntExt, var, i, &x)) {
    	    bins[(int)(((x-min) * (float)nBins) / (max-min))]++;
    	}
    }

    /* Transfer the contents of the overflow bin back into the last 
       bin of the histogram */
    bins[nBins-1] += bins[nBins];

    /* Get the label name from the ntuple */
    name = ExtNTVarName(ntuple, var);
    hist->name = name == NULL ? NULL : XmStringCreateSimple(name);
    
    /* Fill in the XYHistogram structure */
    hist->bins = bins;
    hist->nBins = nBins;
    hist->xMin = min;
    hist->xMax = max;
    hist->edges = NULL;
    hist->barSeparation = 0.;
    hist->errorBars = NULL;

    /* Generate gaussian error bar data if gaussian error bars are turned on */
    if (pInfo->errorBars == GAUSSIAN_ERROR_BARS)
    	addGaussianErrorBars(hist);
}

    
static void ntupleToAdaptHistData(hsNTuple *ntuple, plotInfo *pInfo,
    	XYHistogram *hist)
{
    float *data, *bins, *edges;
    int i, varX = pInfo->ntVars[0], n = 0;
    int nBins;
    ntupleExtension *ntExt = GetNTupleExtension(ntuple->id);
    char *name;
    
    /* read the data from the ntuple */
    data = (float *)XtMalloc(sizeof(float) * ntuple->n);
    for (i=0; i<ntuple->n; i++) {
	if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
		pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
		ExtNTRef(ntuple, ntExt, varX, i, &data[n])) {
    	    n++;
    	}
    }

    /* bin the data */
    Bin1DAdaptHist(data, n, pInfo->nXBins, pInfo->aHistBinStrategy,
    	    &bins, &edges, &nBins);
    XtFree((char *)data);
    
    /* Get the label name from the ntuple */
    name = ExtNTVarName(ntuple, varX);
    hist->name = name == NULL ? NULL : XmStringCreateSimple(name);
    
    /* Fill in the XYHistogram structure */
    if (n == 0) {
	nBins = 1;
	bins = (float *)XtCalloc(sizeof(float), nBins);
	edges = (float *)XtCalloc(sizeof(float), nBins + 1);
	edges[0] = 0.f;
	edges[1] = 1.f;
    }
    hist->bins = bins;
    hist->nBins = nBins;
    hist->xMin = edges[0];
    hist->xMax = edges[nBins];
    hist->edges = edges;
    hist->barSeparation = 0.;
    hist->errorBars = NULL;
}

static void histToHistData(hs1DHist *hist, plotInfo *pInfo,
    	XmString *yLabel, XYHistogram *xyHist)
{
    float *bin;
    int i;
    XYErrorBar *errBar;
    
    /* Copy the data from the hs1DHist structure to the XYHistogram structure */
    xyHist->xMin = hist->min;
    xyHist->xMax = hist->max;
    xyHist->bins = (float *)XtMalloc(sizeof(float) * hist->nBins);
    if (hist->bins == NULL)
    	for (i=0; i<hist->nBins; i++)
    	    xyHist->bins[i] = 0.;
    else
	for (i=0; i<hist->nBins; i++)
    	    xyHist->bins[i] = hist->bins[i];
    xyHist->nBins = hist->nBins;
    xyHist->edges = NULL;
    xyHist->barSeparation = 0.;
    xyHist->errorBars = NULL;

    /* Copy error bar information if requested and available */
    if (pInfo->errorBars == DATA_ERROR_BARS && hist->pErrs != NULL) {
    	xyHist->errorBars = (XYErrorBar *)XtMalloc(sizeof(XYErrorBar) *
    	    	hist->nBins);
    	for (i=0, errBar=xyHist->errorBars, bin=hist->bins; i<hist->nBins;
    	    	i++, errBar++, bin++) {
    	    errBar->max = hist->pErrs[i];
    	    errBar->min = (hist->mErrs == NULL) ?
    	    	    hist->pErrs[i] : hist->mErrs[i];
    	}
    }

    /* Generate gaussian error bar data if gaussian error bars are requested */
    if (pInfo->errorBars == GAUSSIAN_ERROR_BARS)
    	addGaussianErrorBars(xyHist);
    
    /* Use the x axis label name as the histogram name, if that's
       missing, use the item title. */
    if (hist->xLabel == NULL || hist->xLabel[0] == '\0')
    	xyHist->name = XmStringCreateSimple(hist->title);
    else
    	xyHist->name = XmStringCreateSimple(hist->xLabel);

    /* If a yLabel was specified and one hasn't already been specified by
       another (overlaid) plot, return an XmString version of the name. */
    if (*yLabel == NULL && hist->yLabel != NULL)
	*yLabel = XmStringCreateSimple(hist->yLabel);
}

/*
** Create XYCurve structures for the XY widget from the data in the ntuple
** and the plot information in pInfo.  Deposits resulting XYCurve structures
** in "curves" and increments "nCurves"
*/
static void ntupleToTimeSeriesCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XYCurve *curves, int *nCurves)
{
    int i, nVars = 0;
    int vars[MAX_DISP_VARS];

    /* Gather and count the variables to be displayed */
    for (i=0; i<MAX_DISP_VARS; i++) {
	if (pInfo->ntVars[i] != -1)
            vars[nVars++] = pInfo->ntVars[i];
    }
    
    tsCurveCommon(ntuple, pInfo, nVars, vars, NULL, NULL, curves, nCurves);
}

/*
** Same as ntupleToTimeSeriesCurves but interprets some of the ntuple
** variables as error bar data.
*/
static void ntupleToTSErrCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XYCurve *curves, int *nCurves)
{
    int i, nVars = 0;
    int vars[MAX_DISP_VARS], topErrs[MAX_DISP_VARS], bottomErrs[MAX_DISP_VARS];

    /* Gather and count the variables to be displayed */
    for (i=0; i+2<MAX_DISP_VARS; i+=3) {
	if (pInfo->ntVars[i] != -1) {
            vars[nVars] = pInfo->ntVars[i];
            topErrs[nVars] = pInfo->ntVars[i+1];
            bottomErrs[nVars] = pInfo->ntVars[i+2];
            nVars++;
        }
    }
    
    tsCurveCommon(ntuple, pInfo, nVars, vars, topErrs, bottomErrs, curves,
    	    nCurves);
}

/*
** Common part of ntupleToTimeSeriesCurves and ntupleToTSErrCurves.  Transforms
** ntuple data from "ntuple" according to plot information in "pInfo" into
** XYCurve data structures suitable for display by the XY widget.
*/
static void tsCurveCommon(hsNTuple *ntuple, plotInfo *pInfo,
	int nVars, int *vars, int *topErrs, int *bottomErrs, XYCurve *curves,
	int *nCurves)
{
    int i, j;
    XYCurve *curve;
    ntupleExtension *ntExt;
    float value;

    /* Create a curve structure for each variable in "vars" */
    ntExt = GetNTupleExtension(ntuple->id);
    for (i=0, curve=&curves[*nCurves]; i<nVars; i++, curve++) {
    	curve->name = XmStringCreateSimple(ExtNTVarName(ntuple, vars[i]));
    	curve->nPoints = ntuple->n;
    	curve->points = (XYPoint *)XtMalloc(sizeof(XYPoint) * ntuple->n);
        for (j=0; j<ntuple->n; j++) {
            curve->points[j].x = j;
            ExtNTRef(ntuple, ntExt, vars[i], j, &curve->points[j].y);
        }
    	curve->horizBars = NULL;
    	if (topErrs != NULL && topErrs[i] != -1) {
    	    curve->vertBars = (XYErrorBar *)XtMalloc(sizeof(XYErrorBar) *
    	    	    ntuple->n);
    	    for (j=0; j<ntuple->n; j++) {
    	    	ExtNTRef(ntuple, ntExt, topErrs[i], j, &value);
    	    	curve->vertBars[j].max = curve->points[j].y + value;
    	    	if (bottomErrs[i] != -1)
		    ExtNTRef(ntuple, ntExt, bottomErrs[i], j, &value);
		curve->vertBars[j].min = curve->points[j].y - value;
	    }
    	} else
    	    curve->vertBars = NULL;
    }
    
    /* Increment nCurves to indicate how many curves have been added */
    *nCurves += nVars;
}

static void ntupleToXYCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves)
{
    int i, nVars = 0, xVar, yVar, lastXVar = -1, lastYVar = -1;
    int xVars[MAX_DISP_VARS], yVars[MAX_DISP_VARS];

    /* Gather and count the variables to be displayed */
    for (i=0; i+1<MAX_DISP_VARS; i+=2) {
	xVar = pInfo->ntVars[i];
	yVar = pInfo->ntVars[i+1];
	if (xVar != -1 || yVar != -1) {
	    if (xVar == -1)
	    	xVars[nVars] = lastXVar;
	    else {
	    	xVars[nVars] = xVar;
	    	lastXVar = xVar;
            }
	    if (yVar == -1)
	    	yVars[nVars] = lastYVar;
	    else {
	    	yVars[nVars] = yVar;
	    	lastYVar = yVar;
            }
            nVars++;
        }
    }
    if (lastXVar == -1 || lastYVar == -1) {
    	fprintf(stderr, "Internal Error: XY plot, missing variable\n");
    	return;
    }
    
    xyCurveCommon(ntuple, pInfo, nVars, xVars, yVars,
    	    NULL, NULL, NULL, NULL, False, xLabel, curves, nCurves);
}

static void ntupleToXYErrCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves)
{
    int i, nVars = 0, xVar, yVar, lastXVar = -1, lastYVar = -1;
    int xVars[MAX_DISP_VARS], yVars[MAX_DISP_VARS];
    int topErrs[MAX_DISP_VARS], bottomErrs[MAX_DISP_VARS];
    int leftErrs[MAX_DISP_VARS], rightErrs[MAX_DISP_VARS];

    /* Gather and count the variables to be displayed */
    for (i=0; i+5<MAX_DISP_VARS; i+=6) {
	xVar = pInfo->ntVars[i];
	yVar = pInfo->ntVars[i+1];
	if (xVar != -1 || yVar != -1) {
	    if (xVar == -1)
	    	xVars[nVars] = lastXVar;
	    else {
	    	xVars[nVars] = xVar;
	    	lastXVar = xVar;
            }
	    if (yVar == -1)
	    	yVars[nVars] = lastYVar;
	    else {
	    	yVars[nVars] = yVar;
	    	lastYVar = yVar;
            }
            topErrs[nVars] = pInfo->ntVars[i+2];
            bottomErrs[nVars] = pInfo->ntVars[i+3];
            rightErrs[nVars] = pInfo->ntVars[i+4];
            leftErrs[nVars] = pInfo->ntVars[i+5];
            nVars++;
        }
    }
    if (lastXVar == -1 || lastYVar == -1) {
    	fprintf(stderr, "Internal Error: Missing variable in XY plot\n");
    	return;
    }
    
    xyCurveCommon(ntuple, pInfo, nVars, xVars, yVars, topErrs,
    	    bottomErrs, leftErrs, rightErrs, False, xLabel, curves, nCurves);
}

static void ntupleToXYSortCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves)
{
    int i, nVars = 0, xVar;
    int xVars[MAX_DISP_VARS], yVars[MAX_DISP_VARS];

    /* Gather and count the variables to be displayed */
    xVar = pInfo->ntVars[0];
    for (i=1; i<MAX_DISP_VARS; i++) {
	if (pInfo->ntVars[i] != -1) {
	    xVars[nVars] = xVar;
	    yVars[nVars] = pInfo->ntVars[i];
	    nVars++;
	}
    }
    
    xyCurveCommon(ntuple, pInfo, nVars, xVars,
    	    yVars, NULL, NULL, NULL, NULL, True, xLabel, curves, nCurves);
}

static void ntupleToXYSortErrCurves(hsNTuple *ntuple, plotInfo *pInfo,
    	XmString *xLabel, XYCurve *curves, int *nCurves)
{
    int i, nVars = 0, xVar;
    int xVars[MAX_DISP_VARS], yVars[MAX_DISP_VARS];
    int topErrs[MAX_DISP_VARS], bottomErrs[MAX_DISP_VARS];

    /* Gather and count the variables to be displayed */
    xVar = pInfo->ntVars[0];
    for (i=1; i+2<MAX_DISP_VARS; i+=3) {
	if (pInfo->ntVars[i] != -1) {
	    xVars[nVars] = xVar;
	    yVars[nVars] = pInfo->ntVars[i];
            topErrs[nVars] = pInfo->ntVars[i+1];
            bottomErrs[nVars] = pInfo->ntVars[i+2];
            nVars++;
        }
    }
    
    xyCurveCommon(ntuple, pInfo, nVars, xVars, yVars, topErrs,
    	    bottomErrs, NULL, NULL, True, xLabel, curves, nCurves);
}

static void xyCurveCommon(hsNTuple *ntuple, plotInfo *pInfo, int nVars,
    	int *xVars, int *yVars, int *topErrs, int *bottomErrs, int *leftErrs,
    	int *rightErrs, int sort, XmString *xLabel, XYCurve *curves,
    	int *nCurves)
{
    int i, j, *sortIndicies, ntLen = ntuple->n;
    XYCurve *curve;
    ntupleExtension *ntExt;
    float value;
    XYPoint **sortPtrs, *oldPoints;
    XYErrorBar *oldHorizBars, *oldVertBars;

    /* Create a curve structure for each variable pair in "xVars"/"yVars" */
    ntExt = GetNTupleExtension(ntuple->id);
    for (i=0, curve=&curves[*nCurves]; i<nVars; i++, curve++) {
    	curve->name = XmStringCreateSimple(ExtNTVarName(ntuple, yVars[i]));
    	curve->nPoints = ntLen;
    	curve->points = (XYPoint *)XtMalloc(sizeof(XYPoint) * ntLen);
        for (j=0; j<ntLen; j++) {
            ExtNTRef(ntuple, ntExt, xVars[i], j, &curve->points[j].x);
            ExtNTRef(ntuple, ntExt, yVars[i], j, &curve->points[j].y);
        }
    	if (leftErrs != NULL && leftErrs[i] != -1) {
    	    curve->horizBars = (XYErrorBar *)XtMalloc(sizeof(XYErrorBar) *
    	    	    ntLen);
    	    for (j=0; j<ntLen; j++) {
    	    	ExtNTRef(ntuple, ntExt, leftErrs[i], j, &value);
    	    	curve->horizBars[j].max = curve->points[j].x - value;
    	    	if (rightErrs[i] != -1)
		    ExtNTRef(ntuple, ntExt, rightErrs[i], j, &value);
		curve->horizBars[j].min = curve->points[j].x + value;
	    }
    	} else
    	    curve->horizBars = NULL;
    	if (topErrs != NULL && topErrs[i] != -1) {
    	    curve->vertBars = (XYErrorBar *)XtMalloc(sizeof(XYErrorBar) *
    	    	    ntLen);
    	    for (j=0; j<ntLen; j++) {
    	    	ExtNTRef(ntuple, ntExt, topErrs[i], j, &value);
    	    	curve->vertBars[j].max = curve->points[j].y + value;
    	    	if (bottomErrs[i] != -1)
		    ExtNTRef(ntuple, ntExt, bottomErrs[i], j, &value);
		curve->vertBars[j].min = curve->points[j].y - value;
	    }
    	} else
    	    curve->vertBars = NULL;
    }
    
    /* Sort the elements (if requested) */
    if (sort) {
        sortPtrs = (XYPoint **)XtMalloc(sizeof(XYPoint *) * ntLen);
        for (i=0; i<ntLen; i++)
            sortPtrs[i] = &curves[0].points[i];
	qsort(sortPtrs, ntLen, sizeof(XYPoint *), comparePoints);
	sortIndicies = (int *)XtMalloc(sizeof(int) * ntLen);
	for (i=0; i<ntLen; i++)
            sortIndicies[i] = sortPtrs[i] - curves[0].points;
	XtFree((char *)sortPtrs);
	for (i=0, curve=curves; i<nVars; i++, curve++) {
            oldPoints = curve->points;
	    curve->points = (XYPoint *)XtMalloc(sizeof(XYPoint) * ntLen);
	    for (j=0; j<ntLen; j++)
		curve->points[j] = oldPoints[sortIndicies[j]];
    	    XtFree((char *)oldPoints);
            if (curve->horizBars) {
    		oldHorizBars = curve->horizBars;
    		curve->horizBars =
    			(XYErrorBar *)XtMalloc(sizeof(XYErrorBar) * ntLen);
    		for (j=0; j<ntLen; j++)
    	            curve->horizBars[j] = oldHorizBars[sortIndicies[j]];
    		XtFree((char *)oldHorizBars);
    	    }
            if (curve->vertBars) {
    		oldVertBars = curve->vertBars;
    		curve->vertBars =
    			(XYErrorBar *)XtMalloc(sizeof(XYErrorBar) * ntLen);
    		for (j=0; j<ntLen; j++)
    	            curve->vertBars[j] = oldVertBars[sortIndicies[j]];
    		XtFree((char *)oldVertBars);
    	    }
   	}
   	XtFree((char *)sortIndicies);
    }
    
    /* Specify an X label (if there's only one curve and it's not overlaid
       over something else that's already specifying an X label) */
    if (*xLabel == NULL && nVars == 1)
    	*xLabel = XmStringCreateSimple(ExtNTVarName(ntuple, xVars[0]));
    	
    /* Increment nCurves to indicate how many curves have been added */
    *nCurves += nVars;
}

static void redisplay2DHist(windowInfo *window, int mode)
{
    plotInfo *pInfo = window->pInfo[0];
    hsGeneral *item = GetMPItemByID(pInfo->id);
    hs2DHist *hist = (hs2DHist *)item;
    hsNTuple *ntuple = (hsNTuple *)item;
    h2DHistSetup newHist;
    float *bins, *topErrs = NULL, *botErrs = NULL;
    float x, y, xMin, xMax, yMin, yMax, binValue;
    char *xLabel, *yLabel, *zLabel;
    int i, j, varX, varY, binX, binY;
    int nXBins = pInfo->nXBins, nYBins = pInfo->nYBins;
    ntupleExtension *ntExt;
    
    /*
    ** If the item is an ntuple, generate the data for the histogram from
    ** the named variables set in the ntuple panel.  Otherwise, just copy
    ** the data from the hs2DHist structure
    */
    if (item->type == HS_NTUPLE) {
    	varX = pInfo->ntVars[0];
    	varY = pInfo->ntVars[1];
    	ntExt = GetNTupleExtension(item->id);

    	/* Calculate the range of the ntuple data.  If the range between min
    	   and max is 0, fix the interval to make it > 0.  If the ntuple is
    	   empty, temporarily set an arbitray interval of -1 to 1 so the
    	   labels and grid will appear properly */
    	if (ntuple->n == 0) {
    	    xMin = yMin = -1.;
    	    xMax = yMax = 1.;
    	} else {
    	    ExtCalcNTVarRange(ntuple, ntExt, varX, &xMin, &xMax);
    	    ExtCalcNTVarRange(ntuple, ntExt, varY, &yMin, &yMax);
    	    if (xMin == xMax) {
    		xMin = (int)xMin - (nXBins-1);
    		xMax = (int)xMax + (nXBins-1);
    	    }
    	    if (yMin == yMax) {
    		yMin = (int)yMin - (nYBins-1);
    		yMax = (int)yMax + (nYBins-1);
    	    }
    	}
    	
    	/* Allocate memory for the bins and zero the histogram */
    	bins = (float *)XtMalloc(sizeof(float) * nXBins * nYBins);
    	for (i=0; i<nXBins; i++)
    	    for (j=0; j<nYBins; j++)
    	    	bins[i*nYBins + j] = 0;
    	
    	/* Bin the data.  By definition, the maximum value(s) overflow the
    	   last bin.  Move these values back into the last bins of the hist. */
    	for (i=0; i<ntuple->n; i++) {
	    if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
	    	    pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
	    	    ExtNTRef(ntuple, ntExt, varX, i, &x) &&
	    	    ExtNTRef(ntuple, ntExt, varY, i, &y)) {
    		binX = (int)(((x-xMin)*(float)nXBins)/(xMax-xMin));
    		binY = (int)(((y-yMin)*(float)nYBins)/(yMax-yMin));
    		if (binX >= nXBins) binX = nXBins-1;
    		if (binY >= nYBins) binY = nYBins-1;
    		bins[binX * nYBins + binY]++;
    	    }
    	}
    	
    	/* Get names for the axis labels */
    	xLabel = ExtNTVarName(ntuple, varX);
    	yLabel = ExtNTVarName(ntuple, varY);
    	zLabel = NULL;
    
    } else { /* item is a histogram */
    	nXBins = hist->nXBins;
    	nYBins = hist->nYBins;
    	xMin = hist->xMin;
    	xMax = hist->xMax;
    	yMin = hist->yMin;
    	yMax = hist->yMax;
    	xLabel = hist->xLabel;
    	yLabel = hist->yLabel;
    	zLabel = hist->zLabel;
    	bins = hist->bins;
	if (window->pInfo[0]->errorBars==DATA_ERROR_BARS && hist->pErrs!=NULL) {
	    topErrs = hist->pErrs;
	    botErrs = (hist->mErrs == NULL) ? hist->pErrs : hist->mErrs;
	}
    }
    
    /*
    ** Generate gaussian error bar data if gaussian error bars are turned on
    */
    if (window->pInfo[0]->errorBars == GAUSSIAN_ERROR_BARS) {
    	topErrs = (float *)XtMalloc(sizeof(float) * nXBins * nYBins);
    	botErrs = (float *)XtMalloc(sizeof(float) * nXBins * nYBins);
    	for (i=0; i<nXBins; i++) {
    	    for (j=0; j<nYBins; j++) {
    	    	binValue = bins[i*nYBins + j];
    	    	topErrs[i*nYBins + j] = binValue > 0 ? sqrt(binValue) : 0;
    	    	botErrs[i*nYBins + j] = 0 - topErrs[i*nYBins + j];
    	    }
    	}
    }
    
    /*
    ** Apply the new data to the histogram widget
    */
    if (mode == REINIT) {
    	/* set up the plotting widget */
    	newHist.nXBins = nXBins;
    	newHist.nYBins = nYBins;
    	newHist.xMin = xMin;
    	newHist.xMax = xMax;
    	newHist.yMin = yMin;
    	newHist.yMax = yMax;
    	newHist.xScaleType = H2D_LINEAR;
    	newHist.xScaleBase = 0.;
    	newHist.yScaleType = H2D_LINEAR;
    	newHist.yScaleBase = 0.;
    	newHist.xLabel = xLabel;
    	newHist.yLabel = yLabel;
    	newHist.zLabel = zLabel;
    	newHist.bins = bins;
    	hist2DSetHistogram(window->widget, &newHist);
    	hist2DUpdateHistogramData(window->widget, bins, topErrs, botErrs,
    	    	HIST2D_SCALING);
    } else if (mode == UPDATE) {
    	hist2DUpdateHistogramData(window->widget, bins, topErrs, botErrs,
    		HIST2D_RESCALE_AT_MAX);
    } else if (mode == ANIMATION) {
    	hist2DUpdateHistogramData(window->widget, bins, topErrs, botErrs,
    	    	HIST2D_NO_SCALING);
    } else if (mode == REFRESH) {
    	hist2DUpdateHistogramData(window->widget, bins, topErrs, botErrs,
    	    	HIST2D_REFRESH);
    } else { /* mode == REBIN */
    	hist2DSetRebinnedData(window->widget, bins, topErrs, botErrs, nXBins,
    	    	nYBins, xMin, xMax, yMin, yMax, HIST2D_RESCALE_AT_MAX);
    }
    if (item->type == HS_NTUPLE)
    	XtFree((char *)bins);
    if (window->pInfo[0]->errorBars == GAUSSIAN_ERROR_BARS) {
    	XtFree((char *)topErrs);
    	XtFree((char *)botErrs);
    }
}

static void redisplay2DAdaptHist(windowInfo *window, int mode)
{
    plotInfo *pInfo = window->pInfo[0];
    hsGeneral *item = GetMPItemByID(pInfo->id);
    hsNTuple *ntuple = (hsNTuple *)item;
    h2DHistSetup newHist;
    float xMin, xMax, yMin, yMax;
    t *data;
    aHistStruct *bins;
    int i, varX, varY, nPairs=0;
    ntupleExtension *ntExt;

    if (item->type != HS_NTUPLE) {
    	fprintf(stderr, "Internal Error: adaptive hist2, non-ntuple source\n");
    	return;
    }
    
    varX = pInfo->ntVars[0];
    varY = pInfo->ntVars[1];
    ntExt = GetNTupleExtension(item->id);

    /* read the data from the ntuple */
    data = (t *)XtMalloc(sizeof(t) * ntuple->n);
    for (i=0; i<ntuple->n; i++) {
	if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
		pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
		ExtNTRef(ntuple, ntExt, varX, i, &data[nPairs].c[_X]) &&
		ExtNTRef(ntuple, ntExt, varY, i, &data[nPairs].c[_Y])) {
    	    nPairs++;
    	}
    }

    /* bin the data */
    bins = Bin2DAdaptHist(data, nPairs, pInfo->nXBins,
    	    pInfo->aHistBinStrategy);
    XtFree((char *)data);
    
    /*
    ** Apply the new data to the histogram widget
    */
    if (mode == REINIT) {
    	/* set up the plotting widget */
    	ExtCalcNTVarRange(ntuple, ntExt, varX, &xMin, &xMax);
    	ExtCalcNTVarRange(ntuple, ntExt, varY, &yMin, &yMax);
    	newHist.nXBins = 1;
    	newHist.nYBins = 1;
    	newHist.xMin = xMin;
    	newHist.xMax = xMax;
    	newHist.yMin = yMin;
    	newHist.yMax = yMax;
    	newHist.xScaleType = H2D_LINEAR;
    	newHist.xScaleBase = 0.;
    	newHist.yScaleType = H2D_LINEAR;
    	newHist.yScaleBase = 0.;
    	newHist.xLabel = ExtNTVarName(ntuple, varX);
    	newHist.yLabel = ExtNTVarName(ntuple, varY);
    	newHist.zLabel = NULL;
    	newHist.bins = NULL;
    	hist2DSetHistogram(window->widget, &newHist);
    	hist2DSetAdaptiveHistogramData (window->widget, bins,
    		HIST2D_SCALING);
    } else if (mode == UPDATE) {
    	hist2DSetAdaptiveHistogramData (window->widget, bins,
    		HIST2D_RESCALE_AT_MAX);
    } else if (mode == ANIMATION) {
    	hist2DSetAdaptiveHistogramData (window->widget, bins,
                HIST2D_NO_SCALING);
    } else if (mode == REFRESH) {
    	hist2DSetAdaptiveHistogramData (window->widget, bins,
                HIST2D_REFRESH);
    } else {/* mode == REBIN */
    	hist2DSetAdaptiveHistogramData (window->widget, bins,
    		HIST2D_RESCALE_AT_MAX);
    }
    XtFree((char *)bins);
}

static void redisplayScat(windowInfo *window, int mode)
{
    plotInfo *pInfo = window->pInfo[0];
    hsNTuple *ntuple = (hsNTuple *)GetMPItemByID(pInfo->id);
    ScatPoint *points, *point;
    Pixel black = BlackPixelOfScreen(XtScreen(window->widget));
    int i, rescaleMode, colorIsLog, nPoints = 0;
    int varX = pInfo->ntVars[0], varY = pInfo->ntVars[1];
    int varC;
    float colVal, colMin, range;
    XmString s1, s2;
    ntupleExtension *ntExt;

    if (ntuple->type != HS_NTUPLE) {
    	fprintf(stderr, "Internal Error: redisplayScat, non-ntuple source\n");
    	return;
    }

    ntExt = GetNTupleExtension(ntuple->id);

    /* Prepare the color plot if necessary */
    if (pInfo->plotType == COLORSCAT2D)
    {
	varC = pInfo->ntVars[2];
	if (pInfo->csi.rangeIsDynamic ||
	    pInfo->csi.colorMin == FLT_MAX ||
	    pInfo->csi.colorMax == -FLT_MAX)
	{
	    ExtCalcNTVarRange(ntuple, ntExt, varC, &colMin, &colVal);
	    if (pInfo->csi.rangeIsDynamic || pInfo->csi.colorMin == FLT_MAX)
		pInfo->csi.colorMin = colMin;
	    if (pInfo->csi.rangeIsDynamic || pInfo->csi.colorMax == -FLT_MAX)
		pInfo->csi.colorMax = colVal;
	}
	colorIsLog = pInfo->csi.colorIsLog;
	if (colorIsLog)
	{
	    if (pInfo->csi.colorMin <= 0.f && pInfo->csi.colorMax <= 0.f)
	    {
		/* Everything will be in the underflow bin */
		colMin = 0.f;
		range = 1.f;
	    }
	    else
	    {
		colVal = log(pInfo->csi.colorMax);
		if (pInfo->csi.colorMin > 0.f)
		    colMin = log(pInfo->csi.colorMin);
		else
		{
		    if (colVal > 0.f)
			colMin = -1.0f;
		    else
			colMin = colVal - 1.0f;
		}
		range = colVal - colMin;
	    }
	}
	else
	{
	    colMin = pInfo->csi.colorMin;
	    range = pInfo->csi.colorMax - pInfo->csi.colorMin;
	}
	if (range <= 0.f)
	    range = 1.f;
    }

    /* Create the points array for the plot widget */
    points = (ScatPoint *)XtMalloc(sizeof(ScatPoint) * ntuple->n);
    point = points;
    if (pInfo->plotType == COLORSCAT2D)
    {
	for (i=0; i<ntuple->n; i++) {
	    if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
			    pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
		ExtNTRef(ntuple, ntExt, varX, i, &point->x) &&
		ExtNTRef(ntuple, ntExt, varY, i, &point->y) &&
		ExtNTRef(ntuple, ntExt, varC, i, &colVal)) {
		if (colorIsLog)
		{
		    if (colVal > 0.f)
			colVal = (log(colVal)-colMin)/range;
		    else
			colVal = -1.f;
		}
		else
		    colVal = (colVal-colMin)/range;
		point->pixel = getScalePixel(colVal, pInfo->csi.colorScale);
		point++;
		nPoints++;
	    }
	}
    }
    else
    {
	for (i=0; i<ntuple->n; i++) {
	    if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
			    pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
		ExtNTRef(ntuple, ntExt, varX, i, &point->x) &&
		ExtNTRef(ntuple, ntExt, varY, i, &point->y)) {
		point->pixel = black;
		point++;
		nPoints++;
	    }
	}
    }
 
    /* Display the points, and if REINIT mode is specified, reset the axis
       labels and rescale the widget */
    if (mode == REINIT) {
	XtVaSetValues(window->widget,
		XmNxAxisLabel,
		s1=XmStringCreateSimple(ExtNTVarName(ntuple, varX)),
		XmNyAxisLabel,
		s2=XmStringCreateSimple(ExtNTVarName(ntuple, varY)), NULL);
	XmStringFree(s1); XmStringFree(s2);
	ScatSetContents(window->widget, points, nPoints, SCAT_RESCALE);
    } else if (mode == UPDATE) {
 	rescaleMode = (hasActiveSliders(window) || window->growOnly) ?
 		SCAT_GROW_ONLY : SCAT_RESCALE_AT_MAX;
    	ScatSetContents(window->widget, points, nPoints, rescaleMode);
    } else if (mode == REFRESH) {
    	ScatSetContents(window->widget, points, nPoints, SCAT_REFRESH);
    } else { /* mode == ANIMATION */
    	ScatSetContents(window->widget, points, nPoints, SCAT_NO_RESCALE);
    }
    XtFree((char *)points);
}

static void redisplay3DScat(windowInfo *window, int mode)
{
    plotInfo *pInfo = window->pInfo[0];
    hsNTuple *ntuple = (hsNTuple *)GetMPItemByID(pInfo->id);
    Scat3DPoint *points, *point;
    int i, rescaleMode, nPoints = 0, varX = pInfo->ntVars[0];
    int varY = pInfo->ntVars[1], varZ = pInfo->ntVars[2];
    ntupleExtension *ntExt;

    if (ntuple->type != HS_NTUPLE) {
    	fprintf(stderr, "Internal Error: redisplay3DScat, non-ntuple source\n");
    	return;
    }

    /* Create the points array for the plot widget */
    ntExt = GetNTupleExtension(ntuple->id);
    points = (Scat3DPoint *)XtMalloc(sizeof(Scat3DPoint) * ntuple->n);
    point = points;
    for (i=0; i<ntuple->n; i++) {
	if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
		pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
		ExtNTRef(ntuple, ntExt, varX, i, &point->x) &&
		ExtNTRef(ntuple, ntExt, varY, i, &point->y) &&
		ExtNTRef(ntuple, ntExt, varZ, i, &point->z)) {
    	    point++;
    	    nPoints++;
    	}
    }
    
    /* Display the points, and if REINIT mode is specified, reset the axis
       labels and rescale the widget */
    if (mode == REINIT) {
    	Scat3DSetAxesNames(window->widget, ExtNTVarName(ntuple, varX),
    		ExtNTVarName(ntuple, varY), ExtNTVarName(ntuple, varZ));
	Scat3DSetContents(window->widget, points, nPoints, SCAT3D_RESCALE);
    } else if (mode == UPDATE) {
 	rescaleMode = (hasActiveSliders(window) || window->growOnly) ?
 		SCAT3D_GROW_ONLY : SCAT3D_RESCALE_AT_MAX;
    	Scat3DSetContents(window->widget, points, nPoints, rescaleMode);
    } else if (mode == REFRESH) {
    	Scat3DSetContents(window->widget, points, nPoints, SCAT3D_REFRESH);
    } else { /* mode == ANIMATION */
    	Scat3DSetContents(window->widget, points, nPoints, SCAT3D_NO_RESCALE);
    }
    XtFree((char *)points);
}

static void redisplayColorCell(windowInfo *window, int mode)
{
    plotInfo *pInfo = window->pInfo[0];
    hsGeneral *item = GetMPItemByID(pInfo->id);
    hs2DHist *hist = (hs2DHist *)item;
    hsNTuple *ntuple = (hsNTuple *)item;
    float *bins = 0, *xcenters = 0, *ycenters = 0;
    float x, y, xMin, xMax, yMin, yMax, binValue;
    float xBinWidth, yBinWidth, dx, dy, range;
    float minBinValue = FLT_MAX, maxBinValue = -FLT_MAX;
    char *xLabel = 0, *yLabel = 0;
    int i, j, varX, varY, binX, binY, nRects, rescaleMode;
    int nXBins = pInfo->nXBins, nYBins = pInfo->nYBins;
    CellRect *rect = 0, *rects = 0;
    XmString s1, s2;
    ntupleExtension *ntExt = 0;
    int scaleIsLog;

    /*
    ** If the item is an ntuple, generate the bin data for the histogram from
    ** the named variables set in the ntuple panel.  Otherwise, just use
    ** the data from the hs2DHist structure
    */
    if (item->type == HS_NTUPLE) {
    	varX = pInfo->ntVars[0];
    	varY = pInfo->ntVars[1];
    	ntExt = GetNTupleExtension(item->id);

    	/* Calculate the range of the ntuple data.  If the range between min
    	   and max is 0, fix the interval to make it > 0.  If the ntuple is
    	   empty, temporarily set an arbitray interval of -1 to 1 so the
    	   labels and grid will appear properly */
    	if (ntuple->n == 0) {
    	    xMin = yMin = -1.;
    	    xMax = yMax = 1.;
    	} else {
    	    ExtCalcNTVarRange(ntuple, ntExt, varX, &xMin, &xMax);
    	    ExtCalcNTVarRange(ntuple, ntExt, varY, &yMin, &yMax);
    	    if (xMin == xMax) {
    		xMin = (int)xMin - (nXBins-1);
    		xMax = (int)xMax + (nXBins-1);
    	    }
    	    if (yMin == yMax) {
    		yMin = (int)yMin - (nYBins-1);
    		yMax = (int)yMax + (nYBins-1);
    	    }
    	}
    	
    	/* Allocate memory for the bins and zero the bin array */
    	bins = (float *)XtMalloc(sizeof(float) * nXBins * nYBins);
	memset(bins, 0, sizeof(float) * nXBins * nYBins);

    	/* Bin the data.  By definition, the maximum value(s) overflow the
    	   last bin.  Move these values back into the last bins of the hist. */
    	for (i=0; i<ntuple->n; i++) {
	    if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
	    	    pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
	    	    ExtNTRef(ntuple, ntExt, varX, i, &x) &&
	    	    ExtNTRef(ntuple, ntExt, varY, i, &y)) {
    		binX = (int)(((x-xMin)*(float)nXBins)/(xMax-xMin));
    		binY = (int)(((y-yMin)*(float)nYBins)/(yMax-yMin));
    		if (binX >= nXBins) binX = nXBins-1;
    		if (binY >= nYBins) binY = nYBins-1;
    		bins[binX * nYBins + binY]++;
    	    }
    	}
    	/* Get names for the axis labels */
    	xLabel = ExtNTVarName(ntuple, varX);
    	yLabel = ExtNTVarName(ntuple, varY);
    
    } else { /* item is a histogram */
    	nXBins = hist->nXBins;
    	nYBins = hist->nYBins;
    	xMin = hist->xMin;
    	xMax = hist->xMax;
    	yMin = hist->yMin;
    	yMax = hist->yMax;
    	xLabel = hist->xLabel;
    	yLabel = hist->yLabel;
    	if (hist->bins == NULL) {
	    if (mode == REINIT) {
		CellSetContents(window->widget, NULL, 0, CELL_RESCALE);
    	    } else if (mode == UPDATE || mode == REBIN || mode == RENORMALIZE) {
 		rescaleMode = (hasActiveSliders(window) || window->growOnly) ?
 			CELL_GROW_ONLY : CELL_RESCALE_AT_MAX;
		CellSetContents(window->widget, NULL, 0, rescaleMode);
    	    } else if (mode == ANIMATION) {
		CellSetContents(window->widget, NULL, 0, CELL_NO_RESCALE);
	    } else if (mode == REFRESH) {
		CellSetContents(window->widget, NULL, 0, CELL_REFRESH);
	    }
    	    return;
    	}
    	bins = hist->bins;
    }

    /* Find minimum and maximum bin values if the range is dynamic 
       or this is the first time the scale is created */
    if (pInfo->csi.rangeIsDynamic ||
	pInfo->csi.colorMin == FLT_MAX ||
	pInfo->csi.colorMax == -FLT_MAX)
    {
	for (i=0; i<nXBins; i++)
	    for (j=0; j<nYBins; j++)
	    {
		if (bins[i*nYBins + j] < minBinValue)
		    minBinValue = bins[i*nYBins + j];
		if (bins[i*nYBins + j] > maxBinValue)
		    maxBinValue = bins[i*nYBins + j];
	    }
	if (pInfo->csi.rangeIsDynamic || pInfo->csi.colorMin == FLT_MAX)
	    pInfo->csi.colorMin = minBinValue;
	if (pInfo->csi.rangeIsDynamic || pInfo->csi.colorMax == -FLT_MAX)
	    pInfo->csi.colorMax = maxBinValue;
    }
    scaleIsLog = pInfo->csi.colorIsLog;
    if (scaleIsLog)
    {
	if (pInfo->csi.colorMin <= 0.f && pInfo->csi.colorMax <= 0.f)
	{
	    /* Everything is going to be in the underflow bin */
	    minBinValue = 0.f;
	    range = 1.f;
	}
	else
	{
	    maxBinValue = log(pInfo->csi.colorMax);
	    if (pInfo->csi.colorMin > 0.f)
		minBinValue = log(pInfo->csi.colorMin);
	    else
	    {
		if (maxBinValue > 0.f)
		    minBinValue = -1.0f;
		else
		    minBinValue = maxBinValue - 1.0f;
	    }
	    range = maxBinValue - minBinValue;
	}
    }
    else
    {
	minBinValue = pInfo->csi.colorMin;
	range = pInfo->csi.colorMax - pInfo->csi.colorMin;
    }
    if (range <= 0.f)
	range = 1.f;

    /* calculate constants used in creating rectangles for cell widget */
    xcenters = (float *)XtMalloc(sizeof(float) * (nXBins + nYBins));
    ycenters = xcenters + nXBins;

    xBinWidth = (xMax - xMin)/nXBins;
    yBinWidth = (yMax - yMin)/nYBins;
    dx = xBinWidth/2.f + xMin;
    dy = yBinWidth/2.f + yMin;
    for (i=0; i<nXBins; i++)
	xcenters[i] = i*xBinWidth + dx;
    for (i=0; i<nYBins; i++)
	ycenters[i] = i*yBinWidth + dy;

    nRects = nXBins*nYBins;

    /* create the rectangles for the cell widget to draw */
    rects = (CellRect *)XtMalloc(nRects*sizeof(CellRect));
    for (i=0, rect = rects; i<nXBins; i++) {
    	for (j=0; j<nYBins; j++, rect++) {
    	    binValue = bins[i*nYBins + j];
	    if (scaleIsLog)
	    {
		if (binValue > 0.f)
		    binValue = (log(binValue)-minBinValue)/range;
		else
		    binValue = -1.f;
	    }
	    else
		binValue = (binValue-minBinValue)/range;
    	    rect->x = xcenters[i];
    	    rect->y = ycenters[j];
    	    rect->dx = xBinWidth;
    	    rect->dy = yBinWidth;
    	    rect->pixel = getScalePixel(binValue, pInfo->csi.colorScale);
    	}
    }

    /*
    ** Apply the new data to the cell widget
    */
    if (mode == REINIT) {
	s1 = xLabel!=NULL ? XmStringCreateSimple(xLabel) : NULL;
	s2 = yLabel!=NULL ? XmStringCreateSimple(yLabel) : NULL;
	XtVaSetValues(window->widget, XmNxAxisLabel, s1, XmNyAxisLabel, s2, NULL);
	if (s1!=NULL) XmStringFree(s1);
	if (s2!=NULL) XmStringFree(s2);
	CellSetContents(window->widget, rects, nRects, CELL_RESCALE);
    } else if (mode == UPDATE || mode == REBIN || mode == RENORMALIZE) {
 	rescaleMode = (hasActiveSliders(window) || window->growOnly) ?
 		CELL_GROW_ONLY : CELL_RESCALE_AT_MAX;
 	CellSetContents(window->widget, rects, nRects, rescaleMode);
    } else if (mode == ANIMATION) {
 	CellSetContents(window->widget, rects, nRects, CELL_NO_RESCALE);
    } else if (mode == REFRESH) {
 	CellSetContents(window->widget, rects, nRects, CELL_REFRESH);
    }
    XtFree((char *)rects);
    XtFree((char *)xcenters);
    if (item->type == HS_NTUPLE)
    	XtFree((char *)bins);
}

static void redisplayCell(windowInfo *window, int mode)
{
    plotInfo *pInfo = window->pInfo[0];
    hsGeneral *item = GetMPItemByID(pInfo->id);
    hs2DHist *hist = (hs2DHist *)item;
    hsNTuple *ntuple = (hsNTuple *)item;
    float *bins;
    float x, y, xMin, xMax, yMin, yMax, binValue;
    float xBinWidth, yBinWidth, xScale = 1., yScale = 1.;
    float cellNormMin, cellNormMax, cellNormRange, dx, dy;
    char *xLabel, *yLabel;
    int i, j, varX, varY, binX, binY, nRects, rescaleMode;
    int nXBins = pInfo->nXBins, nYBins = pInfo->nYBins;
    float minBinValue = FLT_MAX, maxBinValue = -FLT_MAX;
    CellRect *rect, *rects;
    Pixel black;
    XmString s1, s2;
    ntupleExtension *ntExt;
    Boolean scaleIsLog = False;
    
    if(window->cellLogA != NULL)
        scaleIsLog =  XmToggleButtonGetState(window->cellLogA);
    /*
    ** If the item is an ntuple, generate the bin data for the histogram from
    ** the named variables set in the ntuple panel.  Otherwise, just use
    ** the data from the hs2DHist structure
    */
    if (item->type == HS_NTUPLE) {
    	varX = pInfo->ntVars[0];
    	varY = pInfo->ntVars[1];
    	ntExt = GetNTupleExtension(item->id);

    	/* Calculate the range of the ntuple data.  If the range between min
    	   and max is 0, fix the interval to make it > 0.  If the ntuple is
    	   empty, temporarily set an arbitray interval of -1 to 1 so the
    	   labels and grid will appear properly */
    	if (ntuple->n == 0) {
    	    xMin = yMin = -1.;
    	    xMax = yMax = 1.;
    	} else {
    	    ExtCalcNTVarRange(ntuple, ntExt, varX, &xMin, &xMax);
    	    ExtCalcNTVarRange(ntuple, ntExt, varY, &yMin, &yMax);
    	    if (xMin == xMax) {
    		xMin = (int)xMin - (nXBins-1);
    		xMax = (int)xMax + (nXBins-1);
    	    }
    	    if (yMin == yMax) {
    		yMin = (int)yMin - (nYBins-1);
    		yMax = (int)yMax + (nYBins-1);
    	    }
    	}
    	
    	/* Allocate memory for the bins and zero the bin array */
    	bins = (float *)XtMalloc(sizeof(float) * nXBins * nYBins);
    	for (i=0; i<nXBins; i++)
    	    for (j=0; j<nYBins; j++)
    	    	bins[i*nYBins + j] = 0;
    	
    	/* Bin the data.  By definition, the maximum value(s) overflow the
    	   last bin.  Move these values back into the last bins of the hist. */
    	for (i=0; i<ntuple->n; i++) {
	    if (SliderNTRef(ntuple, ntExt, pInfo->sliderVars, pInfo->nSliders,
	    	    pInfo->sliderThresholds, i, pInfo->sliderGTorLT) &&
	    	    ExtNTRef(ntuple, ntExt, varX, i, &x) &&
	    	    ExtNTRef(ntuple, ntExt, varY, i, &y)) {
    		binX = (int)(((x-xMin)*(float)nXBins)/(xMax-xMin));
    		binY = (int)(((y-yMin)*(float)nYBins)/(yMax-yMin));
    		if (binX >= nXBins) binX = nXBins-1;
    		if (binY >= nYBins) binY = nYBins-1;
    		bins[binX * nYBins + binY]++;
    	    }
    	}
    	/* Get names for the axis labels */
    	xLabel = ExtNTVarName(ntuple, varX);
    	yLabel = ExtNTVarName(ntuple, varY);
    
    } else { /* item is a histogram */
    	nXBins = hist->nXBins;
    	nYBins = hist->nYBins;
    	xMin = hist->xMin;
    	xMax = hist->xMax;
    	yMin = hist->yMin;
    	yMax = hist->yMax;
    	xLabel = hist->xLabel;
    	yLabel = hist->yLabel;
    	if (hist->bins == NULL) {
	    if (mode == REINIT) {
		CellSetContents(window->widget, NULL, 0, CELL_RESCALE);
    	    } else if (mode == UPDATE || mode == REBIN || mode == RENORMALIZE) {
 		rescaleMode = (hasActiveSliders(window) || window->growOnly) ?
 			CELL_GROW_ONLY : CELL_RESCALE_AT_MAX;
		CellSetContents(window->widget, NULL, 0, rescaleMode);
    	    } else if (mode == ANIMATION) {
		CellSetContents(window->widget, NULL, 0, CELL_NO_RESCALE);
	    } else if (mode == REFRESH) {
		CellSetContents(window->widget, NULL, 0, CELL_REFRESH);
	    }
    	    return;
    	}
    	bins = hist->bins;
        if (scaleIsLog) {
          bins = (float *)XtMalloc(sizeof(float) * nXBins * nYBins);
          memcpy(bins, hist->bins, (sizeof(float) * nXBins * nYBins));
        }

    }
    
    /*
    ** Convert the bin array into a list of rectangles for the cell
    ** widget to draw.  Take into account Log vs Linear scaling.
    * find the range of the data: minBinValue & maxBinValue
    */
    for (i=0; i<nXBins; i++) {
    	for (j=0; j<nYBins; j++) {
    	   /*
    	   ** If the scale is logarythmic, convert to Log now
    	   ** if content is o., set the result to -0.9 arbitrarily
    	   */
    	   if (scaleIsLog) {
    	       if (bins[i*nYBins + j] <= 0. ) bins[i*nYBins + j] = -0.9;
    	       else bins[i*nYBins + j] =
    	          (float) log10((double) bins[i*nYBins + j] );
    	    }   
    	    binValue = bins[i*nYBins + j];
    	    if (binValue < minBinValue) minBinValue = binValue;
    	    if (binValue > maxBinValue) maxBinValue = binValue;
	}
    }
    
    /* Normalization depends on binning and slider settings, so it's
       too complicated and costly to calculate it in the normalization
       slider callback.  Here we do the calculation and set the value
       labels in the normalization panel (ick) */
    if (mode == RENORMALIZE) {
    	int left, leftMin, right, rightMax;
    	XtVaGetValues(window->cellNormScale, XmNleftBar, &left,
    		XmNleftBarStopMin, &leftMin, XmNrightBar, &right,
    		XmNrightBarStopMax, &rightMax, NULL);
	if (left == leftMin)
	    window->cellNormMin = -FLT_MAX;
	else
	    window->cellNormMin = (maxBinValue - minBinValue) *
		    (float)(left - leftMin) / (float)(rightMax - leftMin);
	if (right == rightMax)
	    window->cellNormMax = FLT_MAX;
	else
	    window->cellNormMax = (maxBinValue - minBinValue) *
		    (float)(right - leftMin) / (float)(rightMax - leftMin);
	UpdateCellNormValueLabels(window);
    }

    /* calculate constants used in creating rectangles for cell widget */
    xBinWidth = (xMax - xMin)/nXBins;
    yBinWidth = (yMax - yMin)/nYBins;
    cellNormMax = window->cellNormMax;
    if (cellNormMax > maxBinValue) cellNormMax = maxBinValue;
    cellNormMin = window->cellNormMin;
    if (cellNormMin < minBinValue) cellNormMin = minBinValue;
    cellNormRange = cellNormMax - cellNormMin;
    if (cellNormRange != 0) {
	xScale = xBinWidth / cellNormRange;
	yScale = yBinWidth / cellNormRange;
    }
    black = BlackPixelOfScreen(XtScreen(window->widget));
    nRects = nXBins*nYBins;
    
    /* create the rectangles for the cell widget to draw */
    rects = (CellRect *)XtMalloc(nRects*sizeof(CellRect));
    rect = rects;
    for (i=0; i<nXBins; i++) {
    	for (j=0; j<nYBins; j++) {
    	    binValue = bins[i*nYBins + j];
    	    dx = (binValue - cellNormMin) * xScale;
    	    if (dx < 0) dx = 0;
    	    if (dx > xBinWidth) dx = xBinWidth;
    	    dy = (binValue - cellNormMin) * yScale;
    	    if (dy < 0) dy = 0;
    	    if (dy > yBinWidth) dy = yBinWidth;
    	    rect->x = xMin + i*xBinWidth + xBinWidth/2;
    	    rect->y = yMin + j*yBinWidth + yBinWidth/2;
    	    rect->dx = dx;
    	    rect->dy = dy;
    	    rect->pixel = black;
    	    rect++;
    	}
    }
    
    /*
    ** Apply the new data to the cell widget
    */
    if (mode == REINIT) {
	s1 = xLabel!=NULL ? XmStringCreateSimple(xLabel) : NULL;
	s2 = yLabel!=NULL ? XmStringCreateSimple(yLabel) : NULL;
	XtVaSetValues(window->widget, XmNxAxisLabel, s1, XmNyAxisLabel, s2, NULL);
	if (s1!=NULL) XmStringFree(s1);
	if (s2!=NULL) XmStringFree(s2);
	CellSetContents(window->widget, rects, nRects, CELL_RESCALE);
    } else if (mode == UPDATE || mode == REBIN || mode == RENORMALIZE) {
 	rescaleMode = (hasActiveSliders(window) || window->growOnly) ?
 		CELL_GROW_ONLY : CELL_RESCALE_AT_MAX;
 	CellSetContents(window->widget, rects, nRects, rescaleMode);
    } else if (mode == ANIMATION) {
 	CellSetContents(window->widget, rects, nRects, CELL_NO_RESCALE);
    } else if (mode == REFRESH) {
	CellSetContents(window->widget, rects, nRects, CELL_REFRESH);
    }
    XtFree((char *)rects);
    if ((item->type == HS_NTUPLE) || (scaleIsLog)) 
    	XtFree((char *)bins);
}

/*
** Free curve structures and the allocated memory they contain
*/
static void freeCurves(XYCurve *curves, int nCurves)
{
    int i;
    XYCurve *curve;
    
    for (i=0, curve=curves; i<nCurves; i++, curve++) {
    	XmStringFree(curve->name);
    	if (curve->vertBars != NULL)
    	    XtFree((char *)curve->vertBars);
    	if (curve->horizBars != NULL)
    	    XtFree((char *)curve->horizBars);
    	XtFree((char *)curve->points);
    }
    XtFree((char *)curves);
}

/*
** Free histogram structures and the allocated memory they contain
*/
static void freeHists(XYHistogram *hists, int nHists)
{
    int i;
    XYHistogram *hist;
    
    for (i=0, hist=hists; i<nHists; i++, hist++) {
    	XmStringFree(hist->name);
    	if (hist->bins != NULL)
    	    XtFree((char *)hist->bins);
    	if (hist->edges != NULL)
    	    XtFree((char *)hist->edges);
    	if (hist->errorBars != NULL)
    	    XtFree((char *)hist->errorBars);
    }
    XtFree((char *)hists);
}

/*
** Generate gaussian error bar data for a histogram
*/
static void addGaussianErrorBars(XYHistogram *hist)
{
    float *bin;
    XYErrorBar *errBar;
    int i;
    
    if (hist->errorBars == NULL)
    	hist->errorBars =
    	    	(XYErrorBar *)XtMalloc(sizeof(XYErrorBar) * hist->nBins);
    
    for (i=0, errBar=hist->errorBars, bin=hist->bins; i<hist->nBins;
    	    i++, errBar++, bin++) {
    	errBar->min = *bin > 0 ? sqrt(*bin) : 0;
    	errBar->max = errBar->min;
    }
}

/*
** Return true if a window has sliders which are displayed, and not
** set in the "ALL" position.  (Note, currently, active sliders apply
** only to the "bottom" overlaid plot, the one in pInfo[0])
*/
static int hasActiveSliders(windowInfo *window)
{
    int i, gtOrLt;
    float thresh;
    plotInfo *pInfo = window->pInfo[0];
    
    if (window->sliderWindow == NULL)
    	return False;
    for (i=0; i<pInfo->nSliders; i++) {
    	gtOrLt = pInfo->sliderGTorLT[i];
    	thresh = pInfo->sliderThresholds[i];
    	if ((gtOrLt == SLIDER_LT && thresh < window->sliderMax[i]) ||
    		(gtOrLt == SLIDER_GT && thresh > window->sliderMin[i]))
    	    return True;
    }
    return False;
}

/* compare procedure for qsort for sorting points by x coordinate */
static int comparePoints(const void *point1, const void *point2)
{
    XYPoint *pt1 = *(XYPoint **)point1, *pt2 = *(XYPoint **)point2;
    
    if (pt1->x < pt2->x)
    	return -1;
    else if (pt1->x == pt2->x)
    	return 0;
    else
    	return 1;
}

/*
** Makes the style of curve styleList[index] unique in styleList by
** incrementing the line style until it no longer matches anything else.
** If there are no unique line styles left, it gives up and leaves the
** style as is.
*/
static void makeUniqueCurveStyle(XYCurve *styleList, int nStyles, int index)
{
    XYCurve *style = &styleList[index];
    int i, origLineStyle = style->lineStyle;
    
    /* if the style is already unique, do nothing */
    if (isUniqueCurveStyle(style, styleList, nStyles, index))
    	return;
    	
    /* alter the line style to try to make the curve style unique */
    for (i=XY_PLAIN_LINE; i<XY_N_LINE_STYLES; i++) {
    	style->lineStyle = i;
    	if (isUniqueCurveStyle(style, styleList, nStyles, index))
    	    return;
    }
    
    /* couldn't generate a unique style, put back the original one */
    style->lineStyle = origLineStyle;
}

/*
** determine if a curve style is unique in a curve style list
*/
static int isUniqueCurveStyle(XYCurve *style, XYCurve *styleList, int nStyles,
    	int ignoreIndex)
{
    int i;
    XYCurve *s;
    
    for (i=0, s=styleList; i<nStyles; i++, s++) {
    	if (	i != ignoreIndex &&
    	    	style->markerStyle == s->markerStyle &&
    	    	style->markerSize == s->markerSize &&
    	    	style->lineStyle == s->lineStyle &&
    	    	style->markerPixel == s->markerPixel &&
    	    	style->linePixel == s->linePixel)
    	    return False;
    }
    return True;
}


/*
** Makes the style of histogram styleList[index] unique in styleList by
** incrementing the line style until it no longer matches anything else.
** If there are no unique line styles left, it gives up and leaves the
** style as is.
*/
static void makeUniqueHistStyle(XYHistogram *styleList, int nStyles, int index)
{
    XYHistogram *style = &styleList[index];
    int i, origLineStyle = style->lineStyle;
    
    /* if the style is already unique, do nothing */
    if (isUniqueHistStyle(style, styleList, nStyles, index))
    	return;
    	
    /* alter the line style to try to make the Histogram style unique */
    for (i=XY_PLAIN_LINE; i<XY_N_LINE_STYLES; i++) {
    	style->lineStyle = i;
    	if (isUniqueHistStyle(style, styleList, nStyles, index))
    	    return;
    }
    
    /* couldn't generate a unique style, put back the original one */
    style->lineStyle = origLineStyle;
}

/*
** determine if a histogram style is unique in a style list (ignoring the
** style in "ignoreIndex").
*/
static int isUniqueHistStyle(XYHistogram *style, XYHistogram *styleList,
    	int nStyles, int ignoreIndex)
{
    int i;
    XYHistogram *s;
    
    for (i=0, s=styleList; i<nStyles; i++, s++) {
    	if (	i != ignoreIndex &&
    	    	style->lineStyle == s->lineStyle &&
    	    	style->fillStyle == s->fillStyle &&
    	    	style->linePixel == s->linePixel &&
    	    	style->fillPixel == s->fillPixel)
    	    return False;
    }
    return True;
}

/*
** Calback for window manager window frame close box
*/
static void closeCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->multPlot)
    	CloseMPlotFromWinfo(wInfo);
    else
    	ClosePlotWindow(wInfo);
}

/* CloseAuxWinsForPlot - routine to close all auxiliary windows that exist
**                       for a plot.
*/
void CloseAuxWinsForPlot(windowInfo *wInfo)
{
    static Widget histStyleDlg = NULL, curveStyleDlg = NULL;
    static Widget axisShell = NULL, setRowColDlg = NULL;
    static Widget sliderMenuItem = NULL, rebinMenuItem = NULL;
    static Widget cellNormMenuItem = NULL, statsMenuItem = NULL;
    static Widget coordsMenuItem = NULL;
    void *colorScaleDlg;

    if (wInfo->sliderWindow != NULL) {
    	/* close slider window */
    	sliderMenuItem = wInfo->sliderMenuItem;
    	XmToggleButtonSetState(sliderMenuItem, False, True);
    }
    if (wInfo->rebinWindow != NULL) {
    	/* close rebin window */
    	rebinMenuItem = wInfo->rebinMenuItem;
    	XmToggleButtonSetState(rebinMenuItem, False, True);
    }
    if (wInfo->cellNormWindow != NULL) {
    	/* close cell normalization window */
    	cellNormMenuItem = wInfo->cellNormMenuItem;
    	XmToggleButtonSetState(cellNormMenuItem, False, True);
    }
    if (wInfo->statsWindow != NULL) {
    	/* close statistics window */
    	statsMenuItem = wInfo->statsMenuItem;
    	XmToggleButtonSetState(statsMenuItem, False, True);
    }
    if (wInfo->coordsWindow != NULL) {
    	/* close coordinates window */
    	coordsMenuItem = wInfo->coordsMenuItem;
    	XmToggleButtonSetState(coordsMenuItem, False, True);
    }
    if (wInfo->axisSettings != NULL) {
    	/* close axis settings window */
	/* destroy all of the widgets and the pointer to them.  The dialog's
	   destroy CB takes care of deallocating the axisWindow data struct */
	axisShell = wInfo->axisSettings;
	wInfo->axisSettings = NULL;
	XtDestroyWidget(axisShell);
    }
    if (wInfo->colorDialog != NULL) {
 	colorScaleDlg = wInfo->colorDialog;
 	wInfo->colorDialog = NULL;
 	DestroyColorScale(colorScaleDlg);
    }
    if (wInfo->setRowColDlg != NULL) {
    	setRowColDlg = wInfo->setRowColDlg;
    	wInfo->setRowColDlg = NULL;
    	XtDestroyWidget(setRowColDlg);
    }
    if (wInfo->curveStyleDlg != NULL) {
    	curveStyleDlg = wInfo->curveStyleDlg;
    	wInfo->curveStyleDlg = NULL;
    	XtDestroyWidget(curveStyleDlg);
    }
    if (wInfo->histStyleDlg != NULL) {
    	histStyleDlg = wInfo->histStyleDlg;
    	wInfo->histStyleDlg = NULL;
    	XtDestroyWidget(histStyleDlg);
    }
    XSync(XtDisplay(wInfo->widget), 0);
}

void ClosePlotWindowById(void *w, int taskNumber, char *dumm)
{
    ClosePlotWindow((windowInfo *)w);
    ReportTaskCompletion(taskNumber, 0, NULL);
}

void executeReadyCallbacks(windowInfo *wInfo)
{
    /* execute the callbacks in the FIFO order */
    windowInfo *w;
    readyCallBack cbData;
    readyCallBack *last, *tmp;

    /* the window can be destroyed as a result of a callback, so try
       to make sure that we do not operate on a destroyed window */
    for (w=WindowList; w!=NULL; w=w->next) {
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
		executeReadyCallbacks(wInfo);
	    }
	    break;
	}
    }
}

void setThickenPointsScat(int state)
{
    ThickenPointsScat = (state ? True : False);
}

void setThickenPointsScat3D(int state)
{
    ThickenPointsScat3D = (state ? True : False);
}

int getThickenPointsScat()
{
    return ThickenPointsScat;
}

int getThickenPointsScat3D()
{
    return ThickenPointsScat3D;
}

void ClearOverlayedObjects(windowInfo *wInfo)
{
    destroyOverlayedObjectList(wInfo->decor, wInfo->shell);
    wInfo->decor = NULL;
}

void AllowCoordPrint(int state)
{
    allowCoordPrint = state;
}

int coordPrintAllowed(void)
{
    return allowCoordPrint;
}

static int duplicateColor(Display *display, Colormap cmap,
			  Pixel in, Pixel *out)
{
    XColor xcolor;

    xcolor.pixel = in;
    XQueryColor(display, cmap, &xcolor);
    if (!XAllocColor(display, cmap, &xcolor))
    {
	fprintf(stderr, "\nCan't allocate color: all colorcells allocated"
		" and no matching cell found.\n");
	return 1;
    }
    *out = xcolor.pixel;
    return 0;
}

static void printCoordsCB(Widget w, XtPointer wPtr, XtPointer callData)
{
    windowInfo *wInfo = (windowInfo *)wPtr;
    Widget grabc = wInfo->coordsMenuItem;
    int displayOn = (grabc ? XmToggleButtonGetState(grabc) : 0);
    XYCallbackStruct *xycb = NULL;
    XmDrawingAreaCallbackStruct *genericcb = NULL;
    XButtonEvent *event;
    Dimension width, height;
    WidgetClass wClass;
    double plotx = 0.0, ploty = 0.0;
    int inPlotArea = 0;
    char buf[64];
    static int callCount = 0;

    /* Check that coordinate display is on */
    if (!displayOn)
        return;

    /* Check that we have the geometry info */
    if (callData == NULL)
    {
	fprintf(stderr, "Warning: printCoordsCB called without geometry info\n");
	return;
    }

    /* Check that this event has Shift */
    wClass = XtClass(w);
    if (wClass == scat3DWidgetClass || wClass == hist2DWidgetClass)
    {
        genericcb = (XmDrawingAreaCallbackStruct *)callData;
        event = (XButtonEvent *)genericcb->event;
    }
    else if (wClass == scatWidgetClass || wClass == h1DWidgetClass ||
             wClass == cellWidgetClass || wClass == xyWidgetClass)
    {
        xycb = (XYCallbackStruct *)callData;
        event = (XButtonEvent *)xycb->event;
    }
    else
        assert(0);
    if (event == NULL)
    {
	fprintf(stderr, "Warning: printCoordsCB called without X event info\n");
	return;
    }
    if (event->type != ButtonPress)
        return;
    if (!(event->state & ShiftMask))
        return;

    /* Figure out the window size */
    XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);

    /* Figure out the plot coordinates */
    if (xycb)
    {
        XYTransform *xform = xycb->xform;
        assert(xform);
        if (event->x >= xform->xOrigin+1 && event->x <= xform->xEnd &&
            event->y >= xform->yEnd && event->y <= xform->yOrigin-1)
        {
            inPlotArea = 1;
            plotx = XYWindowToDataX(xform, event->x);
            ploty = XYWindowToDataY(xform, event->y);
        }
    }

    /* Set the values in the coordinate display */
    sprintf(buf, "%d  %d", event->x, height - event->y);
    XtVaSetValues(wInfo->coAbs, XmNvalue, buf, NULL);
    sprintf(buf, "%7.5f  %7.5f", event->x/(double)width,
            (height - event->y)/(double)height);
    XtVaSetValues(wInfo->coRel, XmNvalue, buf, NULL);
    if (wInfo->coPlot)
    {
        assert(xycb);
        if (inPlotArea)
        {
            sprintf(buf, "%g  %g", plotx, ploty);
            XtVaSetValues(wInfo->coPlot, XmNvalue, buf, NULL);
        }
        else
            XtVaSetValues(wInfo->coPlot, XmNvalue, "", NULL);
    }

    /* Print coordinate info */
    if (allowCoordPrint)
    {
        printf("%03d winabs: %d %d\n", callCount, event->x, height - event->y);
        printf("%03d winrel: %7.5f %7.5f\n", callCount, event->x/(double)width,
               (height - event->y)/(double)height);
        if (inPlotArea)
            printf("%03d plot:   %g %g\n", callCount, plotx, ploty);
        fflush(stdout);
        if (++callCount == 1000)
            callCount = 0;
    }
}
