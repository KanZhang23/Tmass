/*******************************************************************************
*									       *
* plotMenus.c -- Popup menus for histoscope and nplot plot windows	       *
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
#include <string.h>
#include <unistd.h>
#include <float.h>
#include <assert.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#if XmVersion >= 1002
#include <Xm/PrimitiveP.h>
#endif
#include "../util/misc.h"
#include "../util/printUtils.h"
#include "../util/help.h"
#include "../util/getfiles.h"
#include "../util/psUtils.h"
#include "../util/DialogF.h"
#include "../plot_widgets/ScatP.h"	/* The private versions of the widget */
#include "../plot_widgets/XYP.h"	/*   coordinates for positioning the  */
#include "../plot_widgets/CellP.h"	/*   titles and stats on printouts.   */
#include "../plot_widgets/3DScat.h"
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/H1D.h"
#include "../plot_widgets/XYDialogs.h"
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "interpret.h"
#include "variablePanel.h"
#include "auxWindows.h"
#include "axisSettings.h"
#include "help.h"
#include "plotMenus.h"
#include "controls.h"
#include "auxWindows.h"
#include "multPlot.h"
#include "colorScaleDialog.h"
#include "mainMenu.h"
#include "ntuplePanel.h"
#include "communications.h"
#include "defaultColorScale.h"

#define STATS_FONT "Times-Roman-ISOLatin1"

typedef struct _rowColStruct {
    windowInfo *wInfo;			/* ptr to window Info structure  */
    Widget plotWidget;			/* plot widget			 */
    Widget numRowsText;			/* text widget to find # rows    */
    Widget numColsText;			/* text widget to find # cols    */
} rowColStruct;

static void createPlotMenu(windowInfo *wInfo, helpMenuInfo **helpText);
static void popupCB(Widget w, Widget menu, XButtonEvent *event);
static void closeCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void showRangeCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void toggleUpdateCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void updateCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void growOnlyCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void showSlidersCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void showBinLimitCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void showStatsCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void showCoordCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void dataErrorsCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void gaussErrorsCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void splitHalfCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cntrOfGravCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellViewCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void colorCellViewCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void histoViewCB(Widget w, windowInfo *wInfo, caddr_t callData);

static void scatAxisCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatResetCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatThickenCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatPrintCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatPSCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatColorCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scatColorLimitCB(Widget, XtPointer, LimitsCbResult *);
static void colorApplyCB(Widget, XtPointer, ColorScaleArgs *);
static void colorCloseCB(Widget, XtPointer, DialogStatus);

static void scat3DAxisCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scat3DZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scat3DZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scat3DResetCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scat3DThickenCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scat3DPrintCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void scat3DPSCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void xyAxisCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void xyZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void xyZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void xyResetZoomCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void xySetMarkLineStyleCB(Widget w,windowInfo *wInfo,caddr_t callData);
static void xySetHistogramStyleCB(Widget w, windowInfo *wInfo,
    	caddr_t callData);
static void xyShowLegendCB(Widget w,windowInfo *wInfo,caddr_t callData);
static void xyPrintCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void xyPSCB(Widget w, windowInfo *wInfo, caddr_t callData);
static Boolean showLegend(windowInfo *wInfo, Boolean isTimeSeries); 
static void hist1DShowRebinCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist1DLabelingCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DShowRebinCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DAxisCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DResetCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DBackplaneCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DLabelingCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DPrintCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void hist2DPSCB(Widget w, windowInfo *wInfo, caddr_t callData);

static void cellShowRebinCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellNormCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellAxisCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellResetCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellPrintCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellPSCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellColorCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellColorLimitCB(Widget, XtPointer, LimitsCbResult *);

static void styleApplyCB(XYCurve *curves, int nCurves, void *arg);
static void styleOKCB(XYCurve *curves, int nCurves, void *arg);
static void hStyleApplyCB(XYHistogram *hists, int nHists, void *arg);
static void hStyleOKCB(XYHistogram *hists, int nHists, void *arg);
static void printPlot(windowInfo *wInfo, int statX, int statY,
	int titleCenter, void (*writePSFn)(Widget, FILE *));
static void writePSFile(windowInfo *wInfo, int statX, int statY,
	int titleCenter, void (*writePSFn)(Widget, FILE *));
static void generatePS(windowInfo *wInfo, char *fileName, int statX, int statY,
	int titleCenter, void (*writePSFn)(Widget, FILE *));
static void delMiniCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void showTitlesCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void spcRowColCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void chgTitleCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void multPrintCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void multPSCB(Widget w, windowInfo *wInfo, caddr_t callData);
static Widget createOverlayRemovalMenu(windowInfo *wInfo);
static void removePlotCB(Widget w, windowInfo *wInfo, caddr_t callData);
static int isUp(Widget w);
static void hStyleDismissCB(XYHistogram *hists, int nHists, void *arg);
static void styleDismissCB(XYCurve *curves, int nCurves, void *arg);
static Widget createSetRowColDialog(windowInfo *wInfo);
static void closeRowColCB(Widget w, rowColStruct *rcStruct, caddr_t callData);
static void okCB(Widget w, rowColStruct *rcStruct, caddr_t callData);
static void cancelCB(Widget w, rowColStruct *rcStruct, caddr_t callData); 
static void setupColorScaleArgs(windowInfo *wInfo, ColorScaleArgs *scaleArgs,
				void (*lim_cb)(Widget, XtPointer, LimitsCbResult *));

void CreateTSPlotMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, TSPlotHelpM);
    else
    	createPlotMenu(wInfo, TSPlotHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    AddMenuSeparator(wInfo->menu, "s1");
    AddMenuItem(wInfo->menu, "setMark&LineStyle", "Set Mark & Line Style...", 
                'M', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetMarkLineStyleCB, wInfo);
    wInfo->legendMenuItem =  AddMenuToggle(wInfo->menu, "showLegend",
    	    "Show Legend", 'g', "Ctrl<Key>G", "Ctrl G",
    	    (XtCallbackProc)xyShowLegendCB, wInfo, showLegend(wInfo, True));
    AddMenuSeparator(wInfo->menu, "s2");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo,
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s3");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)xyAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)xyZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)xyZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "resetZoom", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)xyResetZoomCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)xyPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)xyPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close Window", 'C', "Ctrl<Key>w",
	    "Ctrl W", (XtCallbackProc)closeCB, wInfo);
}

void CreateXYPlotMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, XYPlotHelpM);
    else
    	createPlotMenu(wInfo, XYPlotHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    AddMenuSeparator(wInfo->menu, "s1");
    AddMenuItem(wInfo->menu, "setMark&LineStyle", "Set Mark & Line Style...", 
                'M', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetMarkLineStyleCB, wInfo);
    wInfo->legendMenuItem =  AddMenuToggle(wInfo->menu, "showLegend",
    	    "Show Legend", 'g', "Ctrl<Key>G", "Ctrl G",
    	    (XtCallbackProc)xyShowLegendCB, wInfo, showLegend(wInfo, True));
    AddMenuSeparator(wInfo->menu, "s2");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles",
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s3");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)xyAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)xyZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)xyZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "resetZoom", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)xyResetZoomCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)xyPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)xyPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close Window", 'C', "Ctrl<Key>w",
    	    "Ctrl W", (XtCallbackProc)closeCB, wInfo);
}

void CreateXYSortMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, XYSortHelpM);
    else
    	createPlotMenu(wInfo, XYSortHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    AddMenuSeparator(wInfo->menu, "s1");
    AddMenuItem(wInfo->menu, "setMark&LineStyle", "Set Mark & Line Style...", 
                'M', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetMarkLineStyleCB, wInfo);
    AddMenuToggle(wInfo->menu, "showLegend", "Show Legend", 'g', "Ctrl<Key>G",
    		  "Ctrl G", (XtCallbackProc)xyShowLegendCB, wInfo, 
    		  showLegend(wInfo, False));
    AddMenuSeparator(wInfo->menu, "s2");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s3");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)xyAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)xyZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)xyZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "resetZoom", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)xyResetZoomCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)xyPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)xyPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void Create2DScatMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, Scat2DHelpM);
    else
    	createPlotMenu(wInfo, Scat2DHelp);

    if (wInfo->pInfo[0]->plotType == COLORSCAT2D)
    {
	AddMenuItem(wInfo->menu, "color", "Adjust Colors...", 0, "Ctrl<Key>i",
		    "Ctrl I",  (XtCallbackProc)scatColorCB, wInfo);
	AddMenuSeparator(wInfo->menu, "s0");
    }

    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrlc<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s2");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)scatAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)scatZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)scatZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)scatResetCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s3");
    wInfo->thickenMenuItem = 
    	AddMenuToggle(wInfo->menu, "thicken", "Thicken Points", 'h', "",
    		"", (XtCallbackProc)scatThickenCB, wInfo, (Boolean )getThickenPointsScat());
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)scatPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G',
    		    "Ctrl<Key>f",  "Ctrl F",  (XtCallbackProc)scatPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void Create3DScatMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, Scat3DHelpM);
    else
    	createPlotMenu(wInfo, Scat3DHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s2");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)scat3DAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)scat3DZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)scat3DZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)scat3DResetCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s3");
    wInfo->thickenMenuItem = 
    	AddMenuToggle(wInfo->menu, "thicken", "Thicken Points", 'h', "",
    		"", (XtCallbackProc)scat3DThickenCB, wInfo, (Boolean)getThickenPointsScat3D());
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)scat3DPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)scat3DPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void Create1DHistMenu(windowInfo *wInfo, int fromNtuple)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, Hist1DHelpM);
    else
    	createPlotMenu(wInfo, Hist1DHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo, False);
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    wInfo->rebinMenuItem =
    	AddMenuToggle(wInfo->menu, "rebin", "Show Rebin Slider", 'R',
    		"Ctrl<Key>R", "Ctrl R", (XtCallbackProc)hist1DShowRebinCB,
    		wInfo, isUp(wInfo->rebinWindow));
    XtSetSensitive(wInfo->rebinMenuItem, fromNtuple);
    AddMenuSeparator(wInfo->menu, "s1");
    AddMenuItem(wInfo->menu, "setHistogramStyle", "Set Histogram Style...", 
                'F', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetHistogramStyleCB, wInfo);
    wInfo->legendMenuItem = AddMenuToggle(wInfo->menu, "showLegend",
    	    "Show Legend", 'g', "Ctrl<Key>G", "Ctrl G",
    	    (XtCallbackProc)xyShowLegendCB, wInfo, showLegend(wInfo, True));
    AddMenuSeparator(wInfo->menu, "s2");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s3");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)xyAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)xyZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)xyZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)xyResetZoomCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->errorDataMenuItem =
    	AddMenuToggle(wInfo->menu, "dErrors", "Error Data", 'E', "",
    		  "", (XtCallbackProc)dataErrorsCB, wInfo,
    		  wInfo->pInfo[0]->errorBars == DATA_ERROR_BARS);
    XtSetSensitive(wInfo->errorDataMenuItem, !fromNtuple);
    wInfo->gaussErrorMenuItem =
    	AddMenuToggle(wInfo->menu, "gErrors", "Gaussian Errors", 'G', "",
    		  "", (XtCallbackProc)gaussErrorsCB, wInfo,
    		  wInfo->pInfo[0]->errorBars == GAUSSIAN_ERROR_BARS);
    wInfo->binEdgeMenuItem = 
    	AddMenuToggle(wInfo->menu, "labeling", "Label At Bin Edges", 'G', "",
    		  "", (XtCallbackProc)hist1DLabelingCB, wInfo, False);
    AddMenuSeparator(wInfo->menu, "s5");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s6");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)xyPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)xyPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s7");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close Window", 'C', "Ctrl<Key>w",
    	    "Ctrl W", (XtCallbackProc)closeCB, wInfo);
}

void CreateOverlayMenu(windowInfo *wInfo, int fromNtuple)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, OverlayHelpM);
    else
    	createPlotMenu(wInfo, OverlayHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    if (wInfo->pInfo[0]->plotType == HIST1D && fromNtuple)
	wInfo->rebinMenuItem = AddMenuToggle(wInfo->menu, "rebin",
	    	"Show Rebin Slider", 'R', "Ctrl<Key>R", "Ctrl R",
	    	(XtCallbackProc)hist1DShowRebinCB, wInfo,
	    	isUp(wInfo->rebinWindow));
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->nHists != 0)
    	AddMenuItem(wInfo->menu, "histFillLineStyle",
    	    	"Set Histogram Style...", 'F', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetHistogramStyleCB, wInfo);
    if (wInfo->nCurves != 0)
    	AddMenuItem(wInfo->menu, "setMark&LineStyle", "Set Mark & Line Style...", 
                'M', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetMarkLineStyleCB, wInfo);
    wInfo->legendMenuItem = AddMenuToggle(wInfo->menu, "showLegend",
    	    "Show Legend", 'g', "Ctrl<Key>G", "Ctrl G",
    	    (XtCallbackProc)xyShowLegendCB, wInfo, showLegend(wInfo, True));
    AddMenuSeparator(wInfo->menu, "s2");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s3");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)xyAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)xyZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)xyZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)xyResetZoomCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)xyPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'n', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)xyPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    createOverlayRemovalMenu(wInfo);
    AddMenuItem(wInfo->menu, "close", "Close Window", 'C', "Ctrl<Key>w",
    	    "Ctrl W", (XtCallbackProc)closeCB, wInfo);
}

void Create2DHistMenu(windowInfo *wInfo, int fromNtuple)
{
    Widget viewsMenu;
    
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, Hist2DHelpM);
    else
    	createPlotMenu(wInfo, Hist2DHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    wInfo->rebinMenuItem =
    	AddMenuToggle(wInfo->menu, "rebin", "Show Rebin Sliders", 'R',
    	    	"Ctrl<Key>R", "Ctrl R", (XtCallbackProc)hist2DShowRebinCB,
    	    	wInfo, isUp(wInfo->rebinWindow));
    XtSetSensitive(wInfo->rebinMenuItem, fromNtuple);
    if (!fromNtuple) {
    	viewsMenu = AddSubMenu(wInfo->menu, "otherViews", "Other Views", 'V');
    	AddMenuItem(viewsMenu, "cellPlot", "Cell Plot", 'C', NULL,
    		NULL, (XtCallbackProc)cellViewCB, wInfo);
    	AddMenuItem(viewsMenu, "colorCellPlot", "Color Cell Plot", 'o', NULL,
    		NULL, (XtCallbackProc)colorCellViewCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s2");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A", (XtCallbackProc)hist2DAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z", (XtCallbackProc)hist2DZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)hist2DZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)hist2DResetCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s3");
    wInfo->errorDataMenuItem =
    	AddMenuToggle(wInfo->menu, "dErrors", "Error Data", 'E', "",
    		  "", (XtCallbackProc)dataErrorsCB, wInfo, 
    		  wInfo->pInfo[0]->errorBars == DATA_ERROR_BARS);
    XtSetSensitive(wInfo->errorDataMenuItem, !fromNtuple);
    wInfo->gaussErrorMenuItem =
    	AddMenuToggle(wInfo->menu, "gErrors", "Gaussian Errors", 'G', "",
    		  "", (XtCallbackProc)gaussErrorsCB, wInfo,
    		  wInfo->pInfo[0]->errorBars == GAUSSIAN_ERROR_BARS);
    wInfo->backplanesMenuItem = 
	AddMenuToggle(wInfo->menu, "backplanes", "Show Backplanes", 'B', "",
    		    "", (XtCallbackProc)hist2DBackplaneCB, wInfo, True);
    wInfo->binEdgeMenuItem = 
    	AddMenuToggle(wInfo->menu, "labeling", "Label At Bin Edges", 'g', "",
    		  "", (XtCallbackProc)hist2DLabelingCB, wInfo, False);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s5");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)hist2DPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f",  "Ctrl F", (XtCallbackProc)hist2DPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s6");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void Create1DAdaptHistMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, Hist1DAHelpM);
    else
    	createPlotMenu(wInfo, Hist1DAHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    wInfo->rebinMenuItem =
    	AddMenuToggle(wInfo->menu, "rebin", "Show Bin Limit Sliders", 'm',
    	    	"Ctrl<Key>R", "Ctrl R", (XtCallbackProc)showBinLimitCB,
    	    	wInfo, isUp(wInfo->rebinWindow));
    AddMenuSeparator(wInfo->menu, "s1");
    AddMenuItem(wInfo->menu, "setFillLineStyle", "Set Fill & Line Style...", 
                'F', "Ctrl<Key>m",
    		"Ctrl M",  (XtCallbackProc)xySetHistogramStyleCB, wInfo);
    wInfo->legendMenuItem = AddMenuToggle(wInfo->menu, "showLegend",
    	    "Show Legend", 'g', "Ctrl<Key>G", "Ctrl G",
    	    (XtCallbackProc)xyShowLegendCB, wInfo, showLegend(wInfo, True));
    AddMenuSeparator(wInfo->menu, "s2");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s3");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;

    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)xyAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)xyZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)xyZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)xyResetZoomCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->splitHalfMenuItem =
    	AddMenuToggle(wInfo->menu, "splitHalf", "Split In Half Strategy", 'p',
    	    	"", "", (XtCallbackProc)splitHalfCB,
    	    	wInfo, wInfo->pInfo[0]->aHistBinStrategy==SPLIT_IN_HALF);
    wInfo->cntrOfGravMenuItem =
    	AddMenuToggle(wInfo->menu, "cOfG", "Center of Grav. Strategy", 'G',
    	    	"", "", (XtCallbackProc)cntrOfGravCB,
    	    	wInfo, wInfo->pInfo[0]->aHistBinStrategy==CENTER_OF_GRAVITY);
    XtVaSetValues(wInfo->splitHalfMenuItem, XmNindicatorType, XmONE_OF_MANY, NULL);
    XtVaSetValues(wInfo->cntrOfGravMenuItem, XmNindicatorType, XmONE_OF_MANY, NULL);
    AddMenuSeparator(wInfo->menu, "s5");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s6");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)xyPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f",  "Ctrl F", (XtCallbackProc)xyPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s7");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void Create2DAdaptHistMenu(windowInfo *wInfo)
{
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, Hist2DAHelpM);
    else
    	createPlotMenu(wInfo, Hist2DAHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    wInfo->rebinMenuItem =
    	AddMenuToggle(wInfo->menu, "rebin", "Show Bin Limit Sliders", 'm',
    	    	"Ctrl<Key>R", "Ctrl R", (XtCallbackProc)showBinLimitCB,
    	    	wInfo, isUp(wInfo->rebinWindow));
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s2");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)hist2DAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)hist2DZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)hist2DZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)hist2DResetCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s3");
    wInfo->splitHalfMenuItem =
    	AddMenuToggle(wInfo->menu, "splitHalf", "Split In Half Strategy", 'p',
    	    	"", "", (XtCallbackProc)splitHalfCB,
    	    	wInfo, wInfo->pInfo[0]->aHistBinStrategy==SPLIT_IN_HALF);
    wInfo->cntrOfGravMenuItem =
    	AddMenuToggle(wInfo->menu, "cOfG", "Center of Grav. Strategy", 'G',
    	    	"", "", (XtCallbackProc)cntrOfGravCB,
    	    	wInfo, wInfo->pInfo[0]->aHistBinStrategy==CENTER_OF_GRAVITY);
    XtVaSetValues(wInfo->splitHalfMenuItem, XmNindicatorType, XmONE_OF_MANY, NULL);
    XtVaSetValues(wInfo->cntrOfGravMenuItem, XmNindicatorType, XmONE_OF_MANY, NULL);
    AddMenuSeparator(wInfo->menu, "s4");
    wInfo->backplanesMenuItem = 
    	AddMenuToggle(wInfo->menu, "backplanes", "Show Backplanes", 'B', "",
    		"", (XtCallbackProc)hist2DBackplaneCB, wInfo, True);
    AddMenuSeparator(wInfo->menu, "s5");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s6");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)hist2DPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)hist2DPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s7");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void CreateCellMenu(windowInfo *wInfo, int fromNtuple)
{
    Widget viewsMenu;
    
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, CellHelpM);
    else
    	createPlotMenu(wInfo, CellHelp);
    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    wInfo->rebinMenuItem =
    	AddMenuToggle(wInfo->menu, "rebin", "Show Rebin Sliders", 'R',
    	    	"Ctrl<Key>R", "Ctrl R", (XtCallbackProc)cellShowRebinCB,
    	    	wInfo, isUp(wInfo->rebinWindow));
    wInfo->cellNormMenuItem =
    	AddMenuToggle(wInfo->menu, "cellNorm", "Show Normalize Slider", 'z',
    	    	"Ctrl<Key>N", "Ctrl N", (XtCallbackProc)cellNormCB,
    	    	wInfo, False);
    XtSetSensitive(wInfo->rebinMenuItem, fromNtuple);
    if (!fromNtuple) {
    	viewsMenu = AddSubMenu(wInfo->menu, "otherViews", "Other Views", 'V');
    	AddMenuItem(viewsMenu, "2dHist", "2D Histogram", '2', NULL,
    		NULL, (XtCallbackProc)histoViewCB, wInfo);
    	AddMenuItem(viewsMenu, "colorCellPlot", "Color Cell Plot", 'o', NULL,
    		NULL, (XtCallbackProc)colorCellViewCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s2");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)cellAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)cellZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)cellZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)cellResetCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s3");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s4");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)cellPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)cellPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s5");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void CreateColorCellMenu(windowInfo *wInfo, int fromNtuple)
{
    Widget viewsMenu;
    
    if (wInfo->multPlot)
    	createPlotMenu(wInfo, CellHelpM);
    else
    	createPlotMenu(wInfo, CellHelp);

    AddMenuItem(wInfo->menu, "color", "Adjust Colors...", 0, "Ctrl<Key>i",
		"Ctrl I",  (XtCallbackProc)cellColorCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s0");

    wInfo->statsMenuItem = AddMenuToggle(wInfo->menu, "stats",
    		"Show Statistics", 't', "Ctrl<Key>t", "Ctrl T",
    		(XtCallbackProc)showStatsCB, wInfo, isUp(wInfo->statsWindow));
    wInfo->coordsMenuItem = AddMenuToggle(wInfo->menu, "coords",
                "Grab Coordinates", 'b', "Ctrl<Key>b", "Ctrl B",
                (XtCallbackProc)showCoordCB, wInfo, isUp(wInfo->coordsWindow));
    wInfo->sliderMenuItem =
    	AddMenuToggle(wInfo->menu, "sliders", "Show Sliders", 'l', "Ctrl<Key>L",
    		  "Ctrl L", (XtCallbackProc)showSlidersCB, wInfo,
    		  isUp(wInfo->sliderWindow));
    XtSetSensitive(wInfo->sliderMenuItem, wInfo->pInfo[0]->sliderVars[0] != -1
    	    || wInfo->pInfo[0]->sliderVars[1] != -1
    	    || wInfo->pInfo[0]->sliderVars[2] != -1);
    wInfo->rebinMenuItem =
    	AddMenuToggle(wInfo->menu, "rebin", "Show Rebin Sliders", 'R',
    	    	"Ctrl<Key>R", "Ctrl R", (XtCallbackProc)cellShowRebinCB,
    	    	wInfo, isUp(wInfo->rebinWindow));
    XtSetSensitive(wInfo->rebinMenuItem, fromNtuple);
    if (!fromNtuple) {
    	viewsMenu = AddSubMenu(wInfo->menu, "otherViews", "Other Views", 'V');
    	AddMenuItem(viewsMenu, "2dHist", "2D Histogram", '2', NULL,
    		NULL, (XtCallbackProc)histoViewCB, wInfo);
    	AddMenuItem(viewsMenu, "cellPlot", "Cell Plot", 'C', NULL,
    		NULL, (XtCallbackProc)cellViewCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s1");
    if (wInfo->multPlot) {
    	wInfo->showTitlesMenuItem = AddMenuToggle(wInfo->menu, "showTitles", 
    		"Show Titles", 'h', "Ctrl<Key>H", "Ctrl H", 
    		(XtCallbackProc)showTitlesCB, wInfo, 
    		InqMiniTitleState(wInfo->shell));
	AddMenuItem(wInfo->menu, "spcRowCol", "Specify # Rows/Cols...",'c',
		"Ctrl<Key>J", "Ctrl J", (XtCallbackProc)spcRowColCB, wInfo);
    	AddMenuSeparator(wInfo->menu, "s2");
    }
    else
    	wInfo->showTitlesMenuItem = NULL;
    AddMenuItem(wInfo->menu, "axis", "Axis Settings...", 'A', "Ctrl<Key>a",
    		"Ctrl A",  (XtCallbackProc)cellAxisCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomIn", "Zoom In", 'I', "Ctrl<Key>z",
    		"Ctrl Z",  (XtCallbackProc)cellZoomInCB, wInfo);
    AddMenuItem(wInfo->menu, "zoomOut", "Zoom Out", 'O', "Shift Ctrl<Key>z",
    		"Shift Ctrl Z", (XtCallbackProc)cellZoomOutCB, wInfo);
    AddMenuItem(wInfo->menu, "reset", "Reset View", 'R', "Ctrl<Key>r",
    		"Ctrl R", (XtCallbackProc)cellResetCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s3");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    wInfo->growOnlyMenuItem =
    	AddMenuToggle(wInfo->menu, "growOnly", "Grow Only", 'w', "Ctrl<Key>o",
    		"Ctrl O", (XtCallbackProc)growOnlyCB, wInfo, wInfo->growOnly);
    AddMenuSeparator(wInfo->menu, "s4");
    if (wInfo->multPlot) {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P',"Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)multPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)multPSCB, wInfo);
    }
    else {
	AddMenuItem(wInfo->menu, "print", "Print...", 'P', "Ctrl<Key>p",
    		    "Ctrl P",  (XtCallbackProc)cellPrintCB, wInfo);
	AddMenuItem(wInfo->menu, "ps", "Generate PostScript...", 'G', 
    		    "Ctrl<Key>f", "Ctrl F",  (XtCallbackProc)cellPSCB, wInfo);
    }
    AddMenuSeparator(wInfo->menu, "s5");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    if (wInfo->multPlot)
	AddMenuItem(wInfo->menu, "delPlot", "Delete MiniPlot",'D',"Ctrl<Key>x",
		"Ctrl X", (XtCallbackProc)delMiniCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void CreateIndicatorMenu(windowInfo *wInfo)
{
    createPlotMenu(wInfo, IndicatorHelp);
    AddMenuToggle(wInfo->menu, "btn1", "Show Range", 'S', "Ctrl<Key>R", "Ctrl R",
    		(XtCallbackProc)showRangeCB, wInfo, True);
    AddMenuSeparator(wInfo->menu, "s1");
    wInfo->autoUpMenuItem =
    	AddMenuToggle(wInfo->menu, "autoUp", "Automatic Update", 'd', "",
    		  "", (XtCallbackProc)toggleUpdateCB, wInfo, wInfo->update);
    wInfo->updateMenuItem =
    	AddMenuItem(wInfo->menu, "update", "Update", 'U', "Ctrl<Key>u",
    		"Ctrl U", (XtCallbackProc)updateCB, wInfo);
    AddMenuSeparator(wInfo->menu, "s2");
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void CreateControlMenu(windowInfo *wInfo)
{
    createPlotMenu(wInfo, ControlHelp);
    AddMenuToggle(wInfo->menu, "btn1", "Show Range", 'S', "Ctrl<Key>R", "Ctrl R",
    		(XtCallbackProc)showRangeCB, wInfo, True);
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

void CreateTriggerMenu(windowInfo *wInfo)
{
    createPlotMenu(wInfo, TriggerHelp);
    AddMenuItem(wInfo->menu, "changeTitle", "Change Window Title", 0, 
    		    "Ctrl<Key>e", "Ctrl E", (XtCallbackProc)chgTitleCB, wInfo);
    AddMenuItem(wInfo->menu, "close", "Close", 'C', "Ctrl<Key>w", "Ctrl W", 
    		(XtCallbackProc)closeCB, wInfo);
}

static void createPlotMenu(windowInfo *wInfo, helpMenuInfo **helpText)
{

    /* create the menu pane and make it pop up on right button events */
    wInfo->menu = XmCreatePopupMenu(wInfo->widget, "plotMenu", NULL, 0);
    XtAddEventHandler(wInfo->widget, ButtonPressMask, False,
    		      (XtEventHandler)popupCB, (XtPointer)wInfo->menu);
    
    /* add help if it is provided */
    if (helpText != NULL) {
	CreateHelpPulldownMenu(wInfo->menu, helpText);
	/* reset XmNmenuHelpWidget so help goes at the beginning, not end */
	XtVaSetValues(wInfo->menu, XmNmenuHelpWidget, NULL, NULL);
    }
    AddMenuSeparator(wInfo->menu, "s0");
}

/*
** Remove the plot menu from a window (XtDestroyWidget alone will leave a
** dangerous event handler attached and stale entries in the window info
** data structure).
*/
void RemovePlotMenu(windowInfo *wInfo)
{
    XtRemoveEventHandler(wInfo->widget, ButtonPressMask, False,
    	    (XtEventHandler)popupCB, (XtPointer)wInfo->menu);
    XtDestroyWidget(wInfo->menu);
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
    wInfo->rebinMenuItem = NULL;
    wInfo->cellNormMenuItem = NULL;
}

static void popupCB(Widget w, Widget menu, XButtonEvent *event)
{
    if (event->button != Button3)
    	return;

    /* Check that the Shift button is not pressed */
    if (event->state & ShiftMask)
        return;

    XmMenuPosition(menu, event);
    XtManageChild(menu);
}

/*
** Generic callback routines common to more than one plot type
*/
static void showRangeCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ShowRange(wInfo, XmToggleButtonGetState(w));
}

static void closeCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->multPlot)
    	CloseMPlotFromWinfo(wInfo);
    else
    	ClosePlotWindow(wInfo);
}

static void delMiniCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    CloseMiniPlot(wInfo);
}

static void toggleUpdateCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    wInfo->update = XmToggleButtonGetState(w);
}

static void updateCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    RedisplayPlotWindow(wInfo, UPDATE);
    StarWindowTitle(wInfo, False);
}

static void growOnlyCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    wInfo->growOnly = XmToggleButtonGetState(w);
}

static void showSlidersCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->sliderWindow == NULL)
    	    CreateSliderWindow(wInfo, NULL);
    	XtManageChild(wInfo->sliderWindow);
    } else
    	XtUnmanageChild(wInfo->sliderWindow);
}

static void showBinLimitCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->rebinWindow == NULL)
    	    CreateBinLimitWindow(wInfo, NULL);
    	XtManageChild(wInfo->rebinWindow);
    } else
    	XtUnmanageChild(wInfo->rebinWindow);
}

static void showStatsCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->statsWindow == NULL)
    	    CreateStatsWindow(wInfo, NULL);
    	XtManageChild(wInfo->statsWindow);
    } else
    	XtUnmanageChild(wInfo->statsWindow);
}

static void showCoordCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->coordsWindow == NULL)
        {
            Widget parent = wInfo->widget;
            WidgetClass wClass = XtClass(parent);
            int hasPlotCoords = (wClass == cellWidgetClass ||
                                 wClass == h1DWidgetClass ||
                                 wClass == scatWidgetClass ||
                                 wClass == xyWidgetClass);
    	    CreateCoordsWindow(wInfo, NULL, hasPlotCoords);
        }
        XtVaSetValues(wInfo->coAbs, XmNvalue, "", NULL);
        XtVaSetValues(wInfo->coRel, XmNvalue, "", NULL);
        if (wInfo->coPlot)
            XtVaSetValues(wInfo->coPlot, XmNvalue, "", NULL);
    	XtManageChild(wInfo->coordsWindow);
    } else
    	XtUnmanageChild(wInfo->coordsWindow);
}

static void dataErrorsCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	XmToggleButtonSetState(wInfo->gaussErrorMenuItem, False, False);
    	if (!ShowErrorBars(wInfo, DATA_ERROR_BARS, 1))
    	    XmToggleButtonSetState(wInfo->errorDataMenuItem, False, False);
    } else if (!XmToggleButtonGetState(wInfo->gaussErrorMenuItem))
    	ShowErrorBars(wInfo, NO_ERROR_BARS, 1);
}

static void gaussErrorsCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	XmToggleButtonSetState(wInfo->errorDataMenuItem, False, False);
    	ShowErrorBars(wInfo, GAUSSIAN_ERROR_BARS, 1);
    } else if (!XmToggleButtonGetState(wInfo->errorDataMenuItem))
    	ShowErrorBars(wInfo, NO_ERROR_BARS, 1);
}

static void splitHalfCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int state = XmToggleButtonGetState(w);
    
    XmToggleButtonSetState(wInfo->cntrOfGravMenuItem, !state, False); 
    wInfo->pInfo[0]->aHistBinStrategy = XmToggleButtonGetState(w) ?
    	    SPLIT_IN_HALF : CENTER_OF_GRAVITY;
    RedisplayPlotWindow(wInfo, REBIN);
}

static void cntrOfGravCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int state = XmToggleButtonGetState(w);
    
    XmToggleButtonSetState(wInfo->splitHalfMenuItem, !state, False);
    wInfo->pInfo[0]->aHistBinStrategy = XmToggleButtonGetState(w) ?
    	    CENTER_OF_GRAVITY : SPLIT_IN_HALF;
    RedisplayPlotWindow(wInfo, REBIN);
}

static void cellViewCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    String title;
    Widget appShell;
    
    XtVaGetValues(wInfo->shell, XmNtitle, &title, NULL);
    appShell = XtVaAppCreateShell ("plotShell", "PlotShell",
    	    applicationShellWidgetClass, XtDisplay(w),
	    XmNgeometry, NULL,
	    XmNtitle, title,
	    XmNiconName, title, NULL);
    CreateCellWindow(appShell, GetMPItemByID(wInfo->pInfo[0]->id),
	    wInfo->pInfo[0]->ntVars, wInfo->pInfo[0]->sliderVars, title, NULL,
	    appShell, False, NULL, NULL);

    /* update the Windows menu to include the new window */
    UpdateWindowsMenu();
}

static void colorCellViewCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    String title;
    Widget appShell;
    
    XtVaGetValues(wInfo->shell, XmNtitle, &title, NULL);
    appShell = XtVaAppCreateShell ("plotShell", "PlotShell",
    	    applicationShellWidgetClass, XtDisplay(w),
	    XmNgeometry, NULL,
	    XmNtitle, title,
	    XmNiconName, title, NULL);
    CreateColorCellWindow(appShell, GetMPItemByID(wInfo->pInfo[0]->id),
	    wInfo->pInfo[0]->ntVars, wInfo->pInfo[0]->sliderVars, title, NULL,
	    appShell, False, NULL, NULL);
    
    /* update the Windows menu to include the new window */
    UpdateWindowsMenu();
}

static void histoViewCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    String title;
    Widget appShell;
    
    XtVaGetValues(wInfo->shell, XmNtitle, &title, NULL);
    appShell = XtVaAppCreateShell ("plotShell", "PlotShell",
    	    applicationShellWidgetClass, XtDisplay(w),
	    XmNgeometry, NULL,
	    XmNtitle, title,
	    XmNiconName, title, NULL);
    Create2DHistWindow(appShell, GetMPItemByID(wInfo->pInfo[0]->id),
		       wInfo->pInfo[0]->ntVars, wInfo->pInfo[0]->sliderVars,
                       title, NULL, appShell, False, NULL, NULL);
    
    /* update the Windows menu to include the new window */
    UpdateWindowsMenu();
    
}

static void showTitlesCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    Boolean state = XmToggleButtonGetState(w);
    
    if (!state)
    	SetNoMultPlotLabels(wInfo);
    else
    	SetLabelsOnMultPlot(wInfo);
}

static void spcRowColCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->setRowColDlg == NULL)
    	wInfo->setRowColDlg = createSetRowColDialog(wInfo);
    else
    	XRaiseWindow(XtDisplay(wInfo->shell), XtWindow(wInfo->shell));
}

static void chgTitleCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    char *tmp;
    char title[2*HS_MAX_WINDOW_TITLE_LENGTH]; /* can't give DialogF limit */

    GET_ONE_RSRC(wInfo->shell, XmNtitle, &tmp);
    strcpy(title, tmp);

    if (DialogF(DF_PROMPT, w, 3, "Please enter new window title:", title,
    		"OK", "Cancel", "Clear") != 1)
    	return;
    
    /* limit the title to a certain length */
    if (strlen(title) > HS_MAX_WINDOW_TITLE_LENGTH)
    	title[HS_MAX_WINDOW_TITLE_LENGTH] = '\0';

    SET_ONE_RSRC(wInfo->shell, XmNtitle, title);
    UpdateWindowsMenu();
}

/*
** Callbacks for 1D Histogram Plot Widget
*/
static void hist1DShowRebinCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->rebinWindow == NULL)
    	    CreateRebinWindow(wInfo, 1, NULL);
    	XtManageChild(wInfo->rebinWindow);
    } else
    	XtUnmanageChild(wInfo->rebinWindow);
}
static void hist1DLabelingCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
#ifdef notdef /*... functionality not included in XY widget yet */
    XtVaSetValues(wInfo->widget,
    	    XmNbinEdgeLabeling, XmToggleButtonGetState(w), NULL);
#endif
}

/*
** Callbacks for 2D Scatter Plot Widget
*/
static void scatAxisCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->axisSettings == NULL)
    	wInfo->axisSettings = Create2DAxisDialog(wInfo, True, True);
    else
    	XRaiseWindow(XtDisplay(wInfo->shell), XtWindow(wInfo->shell));
}
static void scatZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ScatZoomIn(wInfo->widget);
}
static void scatZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ScatZoomOut(wInfo->widget);
}
static void scatResetCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ScatResetZoom(wInfo->widget);
}
static void scatThickenCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XtVaSetValues(wInfo->widget, XmNdarkerPoints, XmToggleButtonGetState(w), NULL);
}
static void scatPrintCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ScatWidget sw = (ScatWidget)wInfo->widget;

    printPlot(wInfo, sw->scat.xEnd, sw->core.height - sw->scat.yEnd,
    	    (sw->scat.xEnd + sw->scat.xOrigin)/2, ScatWritePS);
}

static void scatPSCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ScatWidget sw = (ScatWidget)wInfo->widget;

    writePSFile(wInfo, sw->scat.xEnd, sw->core.height - sw->scat.yEnd,
    	    (sw->scat.xEnd + sw->scat.xOrigin)/2, ScatWritePS);
}

/*
** Callbacks for 3D Scatter Plot Widget
*/
static void scat3DAxisCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->axisSettings == NULL)
    	wInfo->axisSettings = Create3DAxisDialog(wInfo, False);
    else
    	XRaiseWindow(XtDisplay(wInfo->shell), XtWindow(wInfo->shell));
}
static void scat3DZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    Scat3DZoomIn(wInfo->widget);
}
static void scat3DZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    Scat3DZoomOut(wInfo->widget);
}
static void scat3DResetCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    Scat3DResetView(wInfo->widget);
}
static void scat3DThickenCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
   XtVaSetValues(wInfo->widget, XmNdarkerPoints, XmToggleButtonGetState(w), NULL);
}
static void scat3DPrintCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    unsigned int width, height, dummyU;
    int dummyI;
    Window dummyW;

    XGetGeometry(XtDisplay(wInfo->widget), XtWindow(wInfo->widget), &dummyW,
    	    &dummyI, &dummyI, &width, &height, &dummyU, &dummyU);
    printPlot(wInfo, width, height, width/2, Scat3DWritePS);
}
static void scat3DPSCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    unsigned int width, height, dummyU;
    int dummyI;
    Window dummyW;

    XGetGeometry(XtDisplay(wInfo->widget), XtWindow(wInfo->widget), &dummyW,
    	    &dummyI, &dummyI, &width, &height, &dummyU, &dummyU);
    writePSFile(wInfo, width, height, width/2, Scat3DWritePS);
}

/*
** callbacks for XY Plot Widget
*/
static void xyAxisCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->axisSettings == NULL)
    	wInfo->axisSettings = Create2DAxisDialog(wInfo, True, True);
    else
    	XRaiseWindow(XtDisplay(wInfo->shell), XtWindow(wInfo->shell));
}
static void xyZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XYZoomIn(wInfo->widget);
}
static void xyZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XYZoomOut(wInfo->widget);
}
static void xyResetZoomCB(Widget w, windowInfo *wInfo, caddr_t callData) 
{
    XYResetZoom(wInfo->widget);
}
static void xySetMarkLineStyleCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->curveStyleDlg == NULL)
	wInfo->curveStyleDlg = XYCreateCurveStylesDialog(wInfo->widget,
    		wInfo->curveStyles, wInfo->nCurves, styleOKCB, wInfo,
    		styleApplyCB, wInfo, styleDismissCB, wInfo);
    else
    	XMapRaised(XtDisplay(wInfo->curveStyleDlg),
    	    	XtWindow(wInfo->curveStyleDlg));
}
static void xySetHistogramStyleCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->histStyleDlg == NULL)
        wInfo->histStyleDlg = XYCreateHistogramStylesDialog(wInfo->widget,
    		wInfo->histStyles, wInfo->nHists, hStyleOKCB, wInfo,
    		hStyleApplyCB, wInfo, hStyleDismissCB, wInfo);
    else
    	XMapRaised(XtDisplay(wInfo->histStyleDlg),
    	    	XtWindow(wInfo->histStyleDlg));
}
static void xyShowLegendCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XtVaSetValues(wInfo->widget, XmNshowLegend, XmToggleButtonGetState(w), NULL);
}
static Boolean showLegend(windowInfo *wInfo, Boolean isTimeSeries) 
{
#ifdef notdef /*... Why was this necessary?  make sure Show Legend button
    	            begins in correct state before removing this code */
  int i, num_var = 0;
  
  for (i = 0; i < MAX_DISP_VARS; i++)
    if (wInfo->ntVars[i] != -1)
      num_var++;
  
  if (isTimeSeries && num_var == 1)
    return False;
  if (!isTimeSeries && num_var == 2)
    return False;
    
  return True;
#endif
    Boolean legendState;
    
    XtVaGetValues(wInfo->widget, XmNshowLegend, &legendState, NULL);
    return legendState;
}

static void xyPrintCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XYWidget xyw = (XYWidget)wInfo->widget;

    printPlot(wInfo, xyw->xy.xEnd, xyw->core.height - xyw->xy.yEnd,
    	    (xyw->xy.xEnd + xyw->xy.xOrigin)/2, XYWritePS);
}
static void xyPSCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XYWidget xyw = (XYWidget)wInfo->widget;

    writePSFile(wInfo, xyw->xy.xEnd, xyw->core.height - xyw->xy.yEnd,
    	    (xyw->xy.xEnd + xyw->xy.xOrigin)/2, XYWritePS);
}

/*
** Callbacks for 2D Histogram Widget
*/
static void hist2DShowRebinCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->rebinWindow == NULL)
    	    CreateRebinWindow(wInfo, 2, NULL);
    	XtManageChild(wInfo->rebinWindow);
    } else
    	XtUnmanageChild(wInfo->rebinWindow);
}
static void hist2DAxisCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->axisSettings == NULL)
    	wInfo->axisSettings = Create3DAxisDialog(wInfo, True);
    else
    	XRaiseWindow(XtDisplay(wInfo->shell), XtWindow(wInfo->shell));
}
static void hist2DZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    hist2DZoomIn(wInfo->widget);
}
static void hist2DZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    hist2DZoomOut(wInfo->widget);
}
static void hist2DResetCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    hist2DResetView(wInfo->widget);
}
static void hist2DBackplaneCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XtVaSetValues(wInfo->widget, XmNbackPlanesOn, XmToggleButtonGetState(w), NULL);
}
static void hist2DLabelingCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XtVaSetValues(wInfo->widget,
    	    XmNbinEdgeLabeling, XmToggleButtonGetState(w), NULL);
}
static void hist2DPrintCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    unsigned int width, height, dummyU;
    int dummyI;
    Window dummyW;

    XGetGeometry(XtDisplay(wInfo->widget), XtWindow(wInfo->widget), &dummyW,
    	    &dummyI, &dummyI, &width, &height, &dummyU, &dummyU);
    printPlot(wInfo, width, height, width/2, hist2DwritePs);
}
static void hist2DPSCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    unsigned int width, height, dummyU;
    int dummyI;
    Window dummyW;

    XGetGeometry(XtDisplay(wInfo->widget), XtWindow(wInfo->widget), &dummyW,
    	    &dummyI, &dummyI, &width, &height, &dummyU, &dummyU);
    writePSFile(wInfo, width, height, width/2, hist2DwritePs);
}

/*
** Callbacks for Cell Plot Widget
*/
static void cellShowRebinCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->rebinWindow == NULL)
    	    CreateRebinWindow(wInfo, 2, NULL);
    	XtManageChild(wInfo->rebinWindow);
    } else
    	XtUnmanageChild(wInfo->rebinWindow);
}
static void cellNormCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (XmToggleButtonGetState(w)) {
    	if (wInfo->cellNormWindow == NULL)
    	    CreateCellNormalizeWindow(wInfo, NULL);
    	XtManageChild(wInfo->cellNormWindow);
    } else
    	XtUnmanageChild(wInfo->cellNormWindow);
}
static void cellAxisCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    if (wInfo->axisSettings == NULL)
    	wInfo->axisSettings = Create2DAxisDialog(wInfo, False, False);
    else
    	XRaiseWindow(XtDisplay(wInfo->shell), XtWindow(wInfo->shell));
}
static void cellZoomInCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    CellZoomIn(wInfo->widget);
}
static void cellZoomOutCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    CellZoomOut(wInfo->widget);
}
static void cellResetCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    CellResetZoom(wInfo->widget);
}
static void cellPrintCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    CellWidget cw = (CellWidget)wInfo->widget;

    printPlot(wInfo, cw->cell.xEnd, cw->core.height - cw->cell.yEnd,
    	    (cw->cell.xEnd + cw->cell.xOrigin)/2, CellWritePS);
}
static void cellPSCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    CellWidget cw = (CellWidget)wInfo->widget;

    writePSFile(wInfo, cw->cell.xEnd, cw->core.height - cw->cell.yEnd,
    	    (cw->cell.xEnd + cw->cell.xOrigin)/2, CellWritePS);
}

/*
** Callbacks from the mark and line style and histogram style dialogs
*/
static void styleApplyCB(XYCurve *curves, int nCurves, void *arg)
{
    XYUpdateCurveStyles(((windowInfo *)arg)->widget,
    	    ((windowInfo *)arg)->curveStyles, True);
}
static void styleOKCB(XYCurve *curves, int nCurves, void *arg)
{
    XYUpdateCurveStyles(((windowInfo *)arg)->widget,
    	    ((windowInfo *)arg)->curveStyles, True);
    ((windowInfo *)arg)->curveStyleDlg = NULL;
}
static void hStyleApplyCB(XYHistogram *hists, int nHists, void *arg)
{
    XYUpdateHistogramStyles(((windowInfo *)arg)->widget,
    	    ((windowInfo *)arg)->histStyles, True);
}
static void hStyleOKCB(XYHistogram *hists, int nHists, void *arg)
{
    XYUpdateHistogramStyles(((windowInfo *)arg)->widget,
    	    ((windowInfo *)arg)->histStyles, True);
    ((windowInfo *)arg)->histStyleDlg = NULL;
}

static void hStyleDismissCB(XYHistogram *hists, int nHists, void *arg)
{
    ((windowInfo *)arg)->histStyleDlg = NULL;
}

static void styleDismissCB(XYCurve *curves, int nCurves, void *arg)
{
    ((windowInfo *)arg)->curveStyleDlg = NULL;
}

/*
** Print a the plot in window "wInfo" to a PostScript printer, presenting
** a print dialog for the user to choose print queues and method.  Title
** and optional statistics (if the statistics window is showing and plot is
** not a multi-plot window) are added to the postscript output.  Title is 
** centered on x coordinate "titleCenter", and statistics information is
** added with upper left corner at ("statX","statY").
** This routine is NOT called for mini-plots.
*/
static void printPlot(windowInfo *wInfo, int statX, int statY,
	int titleCenter, void (*writePSFn)(Widget, FILE *))
{
    char *windowTitle, tmpFileName[L_tmpnam];
    
    tmpnam(tmpFileName);
    XtVaGetValues(wInfo->shell, XmNtitle, &windowTitle, NULL);
    generatePS(wInfo, tmpFileName, statX, statY, titleCenter, writePSFn);
#ifdef VMS
    strcat(tmpFileName, ".");
    PrintFile(wInfo->widget, tmpFileName, windowTitle, True); /* delete file */
#else
    PrintFile(wInfo->widget, tmpFileName, windowTitle);
    remove(tmpFileName);
#endif /*VMS*/
}

/*
** Write a PostScript file from window "wInfo", presenting a file-open
** dialog for the user to enter the file name to save into.  Title
** and optional statistics (if the statistics window is showing and plot is
** not a multi-plot window) are added to the postscript output.  Title is 
** centered on x coordinate "titleCenter", and statistics information is
** added with upper left corner at ("statX","statY").
** This routine is NOT called for mini-plots.
*/
static void writePSFile(windowInfo *wInfo, int statX, int statY,
	int titleCenter, void (*writePSFn)(Widget, FILE *))
{
    char fileName[MAXPATHLEN];

    if (GetNewFilename(wInfo->widget,"Save PostScript in:", fileName) != GFN_OK)
    	return;
#ifndef VMS
    /* remove file if already exists, otherwise print routines will append */
    unlink(fileName);
#endif /*VMS*/

    	generatePS(wInfo, fileName, statX, statY, titleCenter, writePSFn);
}


/*
** Print a multiple-plot window to a PostScript printer, presenting
** a print dialog for the user to choose print queues and method.
** All mini-plots in the multiple-plot window are output and their
** titles are added to the postscript output.
*/
static void multPrintCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    char *windowTitle, tmpFileName[L_tmpnam];
    
    tmpnam(tmpFileName);
    XtVaGetValues(wInfo->shell, XmNtitle, &windowTitle, NULL);
    PrintMultiPlotWin(wInfo->shell, tmpFileName);
#ifdef VMS
    strcat(tmpFileName, ".");
    PrintFile(wInfo->shell, tmpFileName, windowTitle, True); /* delete file */
#else
    PrintFile(wInfo->shell, tmpFileName, windowTitle);
    remove(tmpFileName);
#endif /*VMS*/
}

/*
** Write a PostScript file from mini-plot window "wInfo", presenting a 
** file-open dialog for the user to enter the file name to save into.  
** All mini-plots in the multiple-plot window are output and their
** titles are added to the postscript output.
*/
static void multPSCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    char fileName[MAXPATHLEN];

    if (GetNewFilename(wInfo->shell, "Save PostScript in:", fileName) != GFN_OK)
    	return;
#ifndef VMS
    /* remove file if already exists, otherwise print routines will append */
    unlink(fileName);
#endif /*VMS*/
    PrintMultiPlotWin(wInfo->shell, fileName);
}

/*
** Write a PostScript version of window "wInfo" to file "fileName".  Title
** and optional statistics (if the statistics window is showing) are added
** to the postscript output.  Title is centered on x coordinate "titleCenter",
** and statistics information is added with upper left corner at ("statX",
** "statY").
*/
static void generatePS(windowInfo *wInfo, char *fileName, int statX, int statY,
	int titleCenter, void (*writePSFn)(Widget, FILE *))
{
    char *windowTitle;
    unsigned int width, height, statsWidth, dummyU, printTitle;
    int dummyI;
    Window dummyW;
    FILE *psFile;
    XmFontList statsFontList;
    XFontStruct *statsFS;
    
    XGetGeometry(XtDisplay(wInfo->widget), XtWindow(wInfo->widget), &dummyW,
    	    &dummyI, &dummyI, &width, &height, &dummyU, &dummyU);
    XtVaGetValues(wInfo->shell, XmNtitle, &windowTitle, NULL);
    if (wInfo->statsWindow != NULL && XtIsManaged(wInfo->statsWindow))
    	XGetGeometry(XtDisplay(wInfo->widget), XtWindow(wInfo->widget), &dummyW,
    	    	&dummyI, &dummyI, &statsWidth, &dummyU, &dummyU, &dummyU);
    else
    	statsWidth = 0;

    printTitle = strncmp(windowTitle, "//", 2) && GetTitlePSOnOrOff();
    psFile = OpenPS(fileName, width + statsWidth, height + 
		    (printTitle ? GetTitlePSFontSize() : 0));
    /* Skip titles which start with double forward slash */
    if (printTitle)
    {
	fprintf(psFile, "/%s findfont %04d scalefont setfont\n",
		GetTitlePSFont(), GetTitlePSFontSize());
	fprintf(psFile, "(%s) dup stringwidth pop 2 div neg %d add %d moveto show\n",
		windowTitle, titleCenter, height);
    }
    if (wInfo->statsWindow != NULL && XtIsManaged(wInfo->statsWindow)) {
	XtVaGetValues(wInfo->statsWindow, XmNfontList, &statsFontList, NULL);
	statsFS = GetDefaultFontStruct(statsFontList);
	WriteStatsPS(psFile, wInfo, STATS_FONT, statsFS->ascent +
		statsFS->descent - 2, statX, statY);
    }
    PSSetWindowPagePosition(0, 0, width, height);
    (*writePSFn)(wInfo->widget, psFile);
    EndPS();
}

static Widget createOverlayRemovalMenu(windowInfo *wInfo)
{
    Widget menu, btn;
    int i, c, nCurves;
    XYCurve *curve;
    XYHistogram *hist;
    XmString temp, comma, name = NULL;
    
    menu = AddSubMenu(wInfo->menu, "removeOvly", "Remove Overlayed Plot", 'v');
    curve = (XYCurve *)wInfo->curveStyles;
    hist = (XYHistogram *)wInfo->histStyles;
    comma = XmStringCreateSimple(", ");
    for (i=0; i<wInfo->nPlots; i++) {
    	nCurves = NCurvesInPlot(wInfo->pInfo[i]->plotType,
    	    	wInfo->pInfo[i]->ntVars);
    	if (nCurves > 0) {
    	    name = XmStringCopy(curve->name);
    	    curve++;
    	    for (c=1; c<nCurves; c++) {
    		temp = XmStringConcat(name, comma);
    		XmStringFree(name);
    		name = XmStringConcat(temp, curve->name);
    		XmStringFree(temp);
    		curve++;
    	    }
    	} else if (PlotIsHist(wInfo->pInfo[i]->plotType)) {
    	    if (hist->name == NULL)
    	    	name = XmStringCreateSimple("Unnamed");
    	    else
    	    	name = XmStringCopy(hist->name);
    	    hist++;
    	}
    	btn = XtVaCreateManagedWidget("removePlot", xmPushButtonWidgetClass,
                                      menu, XmNlabelString, name, XmNuserData,
                                      (XtPointer)((long)i), NULL);
    	XtAddCallback(btn, XmNactivateCallback,
    	    	(XtCallbackProc)removePlotCB, wInfo);
	XmStringFree(name);
    }
    XmStringFree(comma);
    return menu;
}

static void removePlotCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XtPointer userData;
    
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    RemoveOverlayed(wInfo, (int)((long)userData));
}

/*
** Test if dialog "w" (the child of a dialog shell) has been created (is non
** null), and is managed.
*/
static int isUp(Widget w)
{
    if (w != NULL)
    	return XtIsManaged(w);
    return False;
}
static Widget createSetRowColDialog(windowInfo *wInfo)
{
    Widget dialog, form, plotW = wInfo->widget, okBtn, cancelBtn;
    Widget numRowsLabel, numColsLabel, winLabel;
    XmString xmstr;
    int numRows, numCols, numPlots, numUnused;
    char numPlotsString[188];
    rowColStruct *rcStruct;

    rcStruct = (rowColStruct *) XtMalloc(sizeof(rowColStruct));
    rcStruct->wInfo = wInfo;
    
    dialog = XtVaCreateWidget("setRowCol", xmDialogShellWidgetClass, plotW,
    		XmNtitle, "Set #Rows and #Cols", NULL);
    form = XtVaCreateWidget("form", xmFormWidgetClass, dialog,
    		XmNnoResize, True,
    		XmNautoUnmanage, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    
    okBtn = XtVaCreateManagedWidget("okBtn", 
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("OK"),
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNleftOffset, 10,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 45,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, 10,
    	    NULL);
    XtAddCallback(okBtn, XmNactivateCallback, (XtCallbackProc)  okCB, rcStruct);
    XtManageChild(okBtn);
    XmStringFree(xmstr);
    XtVaSetValues(form, XmNdefaultButton, okBtn, NULL);

    cancelBtn = XtVaCreateManagedWidget("cancelBtn", 
    	    xmPushButtonGadgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("Cancel"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 55,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNrightOffset, 10,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, 10,
    	    NULL);
    XtAddCallback(cancelBtn, XmNactivateCallback,(XtCallbackProc) cancelCB,
            rcStruct);
    XtManageChild(cancelBtn);
    XmStringFree(xmstr);

    numRowsLabel = XtVaCreateManagedWidget("numRowsLabel",
    	    xmLabelGadgetClass, form, 
    	    XmNlabelString, xmstr=XmStringCreateSimple("# Rows:"),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNbottomWidget, okBtn,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNbottomOffset, 14,
    	    XmNleftOffset, 10,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(numRowsLabel);
    
    rcStruct->numRowsText = XtVaCreateManagedWidget("numRowsText",
    	    xmTextWidgetClass, form,
    	    XmNrows, (short)1,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, numRowsLabel,
    	    XmNleftOffset, 12,
    	    XmNbottomWidget, okBtn,
    	    XmNbottomOffset, 5,
    	    XmNcolumns, 5,
    	    NULL);
    XtManageChild(rcStruct->numRowsText);
    RemapDeleteKey(rcStruct->numRowsText);

    numColsLabel = XtVaCreateManagedWidget("numColsLabel",
    	    xmLabelGadgetClass, form, 
    	    XmNlabelString, xmstr=XmStringCreateSimple("# Columns:"),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftOffset, 15,
    	    XmNbottomWidget, cancelBtn,
    	    XmNleftWidget, rcStruct->numRowsText,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNbottomOffset, 14,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(numColsLabel);
    
    rcStruct->numColsText = XtVaCreateManagedWidget("numColsText",
    	    xmTextWidgetClass, form,
    	    XmNrows, (short)1,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNleftOffset, 12,
    	    XmNbottomWidget, cancelBtn,
    	    XmNleftWidget, numColsLabel,
    	    XmNrightOffset, 10,
    	    XmNbottomOffset, 5,
    	    XmNcolumns, 5,
    	    NULL);
    XtManageChild(rcStruct->numColsText);
    RemapDeleteKey(rcStruct->numColsText);

    AddMotifCloseCallback(dialog, (XtCallbackProc)closeRowColCB, rcStruct);
    
    GetStatsForMultiPlot(wInfo, &numRows, &numCols, &numPlots, 
    	    &numUnused);
    sprintf(numPlotsString, "The multi-plot window is now %d rows x %d cols\n\
%d plots are displayed\n\
%d unused cells will NOT be copied\n\
Any new unused cells will appear last\n\nChange Multi-Plot window to:", 
	    numRows, numCols, numPlots, numUnused);
    winLabel = XtVaCreateManagedWidget("winLabel",
    	    xmLabelGadgetClass, form, 
    	    XmNlabelString, xmstr=XmStringCreateLtoR(numPlotsString, 
    	    			  XmSTRING_DEFAULT_CHARSET),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNbottomWidget, rcStruct->numColsText,
    	    XmNalignment, XmALIGNMENT_CENTER,
    	    XmNbottomOffset, 10,
    	    XmNtopOffset, 5,
    	    XmNleftOffset, 5,
    	    XmNrightOffset, 5,
    	    NULL);
    XmStringFree(xmstr);
    XtManageChild(winLabel);
    
    /* pop up the dialog */
    XtManageChild(form);
    
    return dialog;
} 

static void okCB(Widget w, rowColStruct *rcStruct, caddr_t callData) 
{
    int numRows, numCols;
    windowInfo *wInfo = rcStruct->wInfo;
        
    /* Get the window title & the number of rows and columns from dialog 
     * Make sure numeric values are valid                                */
    if (GetIntTextWarn(rcStruct->numRowsText, &numRows, "Number of Rows", True) 
            != TEXT_READ_OK) 
        return;
    if (GetIntTextWarn(rcStruct->numColsText, &numCols, "Number of Columns", 
    	    True) != TEXT_READ_OK) 
        return;
    if (numRows * numCols <= 0 || numRows < 0 || numCols < 0) {
        DialogF(DF_INF, w, 1, 
            "Please specify a proper number of rows and columns",
            "Acknowledged");
        return;
    }
    
    /* Close the dialog before calling ReFillMultiPlot in case that plot
       is deleted when the window is made smaller */
    closeRowColCB(w, rcStruct, callData);
    
    /* Refill the Multiplot. Check whether the plot widget pointed
     * to by rcStruct was deleted by  ReFillMultiPlot, and if so,
     * don't call close callback (just destroy the dialog)
     */
    ReFillMultiPlot(wInfo, numRows, numCols);
}

static void cancelCB(Widget w, rowColStruct *rcStruct, caddr_t callData) 
{
    closeRowColCB(w, rcStruct, callData);
}

static void closeRowColCB(Widget w, rowColStruct *rcStruct, caddr_t callData)
{
    /* destroy the widget and its pointer */
    XtDestroyWidget(rcStruct->wInfo->setRowColDlg);
    rcStruct->wInfo->setRowColDlg = NULL;
    XtFree((char *)rcStruct);
}

void GeneratePlotPSById(void *p, int taskNumber, char *filename)
{
    unsigned int width, height, dummyU;
    int dummyI;
    Window dummyW;
    void (*writePSFn)(Widget, FILE *) = NULL;
    windowInfo *w = (windowInfo *)p;

    XGetGeometry(XtDisplay(w->widget), XtWindow(w->widget), &dummyW,
		 &dummyI, &dummyI, &width, &height, &dummyU, &dummyU);
    switch (w->pInfo[0]->plotType)
    {
    case TSPLOT:
    case TSPLOTERR:
	writePSFn = XYWritePS;
	break;
    case XYPLOT:
    case XYPLOTERR:
	writePSFn = XYWritePS;
	break;
    case XYSORT:
    case XYSORTERR:
	writePSFn = XYWritePS;
	break;
    case SCAT2D:
    case COLORSCAT2D:
	writePSFn = ScatWritePS;
	break;
    case SCAT3D:
	writePSFn = Scat3DWritePS;
	break;
    case HIST1D:
	writePSFn = XYWritePS;
	break;
    case HIST2D:
	writePSFn = hist2DwritePs;
	break;
    case ADAPTHIST1D:
	writePSFn = XYWritePS;
	break;
    case ADAPTHIST2D:
	writePSFn = hist2DwritePs;
	break;
    case CELL:
    case COLORCELL:
	writePSFn = CellWritePS;
	break;
    }
    generatePS(w, filename, width, height, width/2, writePSFn);
    /* Assume that the above operation always succeeds */
    ReportTaskCompletion(taskNumber, 0, NULL);
}

static void scatColorCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ColorScaleArgs scaleArgs;

    /* Do not pop up more than one dialog */
    if (wInfo->colorDialog != NULL)
	return;
    setupColorScaleArgs(wInfo, &scaleArgs, scatColorLimitCB);
    wInfo->colorDialog  = CreateColorScale(wInfo->widget, "Color Editor", &scaleArgs);
}

static void scatColorLimitCB(Widget w, XtPointer xtp, LimitsCbResult *cbres)
{
    windowInfo *window = (windowInfo *)xtp;
    plotInfo *pInfo = window->pInfo[0];
    hsNTuple *ntuple = (hsNTuple *)GetMPItemByID(pInfo->id);
    ntupleExtension *ntExt = GetNTupleExtension(ntuple->id);
    int varC = pInfo->ntVars[2];

    ExtCalcNTVarRange(ntuple, ntExt, varC, &cbres->min, &cbres->max);
}

static void colorApplyCB(Widget w, XtPointer xtp, ColorScaleArgs *colorArgs)
{
    windowInfo *wInfo = (windowInfo *)xtp;
    int change_scale = 0;
    char buf[32];

    /* Have to change the color scale if it has been modified */
    if (colorArgs->reason == DIALOG_APPLY || colorArgs->reason == DIALOG_OK)
    {
	/* We are given the dialog color scale. Change the plot scale
	   only if the dialog color scale has been modified since its
	   creation or if the plot color scale itself has changed */
	if (colorArgs->scale->version > 0)
	    change_scale = 1;
	if (wInfo->pInfo[0]->csi.colorScale->version != colorArgs->startVersion)
	    change_scale = 1;
    }
    if (colorArgs->reason == DIALOG_CANCEL)
    {
	/* We are given the backup color scale. Change the plot scale
	   only if the backup scale is different from our current scale */
	if (compareColorScaleToBackup(colorArgs->scale,
		       wInfo->pInfo[0]->csi.colorScale))
	    change_scale = 1;
    }
    if (change_scale)
    {
	decrColorScaleRefCount(wInfo->pInfo[0]->csi.colorScale);
	if (strcmp(colorArgs->scale->name, DEFAULT_COLOR_SCALE_NAME) == 0)
	{
	    /* This must be the backup of the default scale */
	    assert(colorArgs->reason == DIALOG_CANCEL && colorArgs->scale->index < 0);
	    wInfo->pInfo[0]->csi.colorScale = defaultColorScale();
	}
	else
	{
	    unusedColorScaleName(buf);
	    wInfo->pInfo[0]->csi.colorScale = cloneColorScale(buf, colorArgs->scale);
	}
	incrColorScaleRefCount(wInfo->pInfo[0]->csi.colorScale);
    }
    wInfo->pInfo[0]->csi.colorMin       = colorArgs->colorMin;
    wInfo->pInfo[0]->csi.colorMax       = colorArgs->colorMax;
    wInfo->pInfo[0]->csi.colorIsLog     = colorArgs->mappingIsLog;
    wInfo->pInfo[0]->csi.rangeIsDynamic = colorArgs->rangeIsDynamic;
    RedisplayPlotWindow(wInfo, ANIMATION);
}

static void colorCloseCB(Widget w, XtPointer xtp, DialogStatus s)
{
    windowInfo *wInfo = (windowInfo *)xtp;
    wInfo->colorDialog = NULL;
}

static void cellColorCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    ColorScaleArgs scaleArgs;

    /* Do not pop up more than one dialog */
    if (wInfo->colorDialog != NULL)
	return;
    setupColorScaleArgs(wInfo, &scaleArgs, cellColorLimitCB);
    wInfo->colorDialog  = CreateColorScale(wInfo->widget, "Color Editor", &scaleArgs);
}

static void cellColorLimitCB(Widget w, XtPointer xtp, LimitsCbResult *cbres)
{
    windowInfo *window = (windowInfo *)xtp;
    plotInfo *pInfo = window->pInfo[0];
    hsGeneral *item = GetMPItemByID(pInfo->id);
    hs2DHist *hist = (hs2DHist *)item;
    hsNTuple *ntuple = (hsNTuple *)item;
    ntupleExtension *ntExt;
    int i, j, varC, nXBins, nYBins;
    float *bins;
    float minBinValue = FLT_MAX, maxBinValue = -FLT_MAX;

    if (item->type == HS_NTUPLE)
    {
	ntExt = GetNTupleExtension(ntuple->id);
	varC = pInfo->ntVars[2];
	ExtCalcNTVarRange(ntuple, ntExt, varC, &cbres->min, &cbres->max);
    }
    else
    {
    	nXBins = hist->nXBins;
    	nYBins = hist->nYBins;
    	bins = hist->bins;
	for (i=0; i<nXBins; i++)
	    for (j=0; j<nYBins; j++)
	    {
		if (bins[i*nYBins + j] < minBinValue)
		    minBinValue = bins[i*nYBins + j];
		if (bins[i*nYBins + j] > maxBinValue)
		    maxBinValue = bins[i*nYBins + j];
	    }
	cbres->min = minBinValue;
	cbres->max = maxBinValue;
    }
}

static void setupColorScaleArgs(windowInfo *wInfo, ColorScaleArgs *scaleArgs,
				void (*lim_cb)(Widget, XtPointer, LimitsCbResult *))
{
    assert(wInfo);
    assert(scaleArgs);
    memset(scaleArgs, 0, sizeof(ColorScaleArgs));
    scaleArgs->scale          = wInfo->pInfo[0]->csi.colorScale;
    scaleArgs->colorMin       = wInfo->pInfo[0]->csi.colorMin;
    scaleArgs->colorMax       = wInfo->pInfo[0]->csi.colorMax;
    scaleArgs->mappingIsLog   = wInfo->pInfo[0]->csi.colorIsLog;
    scaleArgs->rangeIsDynamic = wInfo->pInfo[0]->csi.rangeIsDynamic;
    scaleArgs->limits_cb    = lim_cb;
    scaleArgs->limits_data  = wInfo;
    scaleArgs->apply_cb     = colorApplyCB;
    scaleArgs->apply_data   = wInfo;
    scaleArgs->close_cb     = colorCloseCB;
    scaleArgs->close_data   = wInfo;
    scaleArgs->startVersion = wInfo->pInfo[0]->csi.colorScale->version;
}
