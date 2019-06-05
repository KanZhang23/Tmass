/*******************************************************************************
*									       *
* axisSettings.c -- Dialogs for setting plot parameters and labels	       *
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
* Sept 22, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <math.h>
#include <stdio.h>
#include <float.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/stringUtils.h"
#include "../histo_util/hsTypes.h"
#include "../plot_widgets/Scat.h"
#include "../plot_widgets/XY.h"
#include "../plot_widgets/H1D.h"
#include "../plot_widgets/Cell.h"
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/3DScat.h"
#include "histoP.h"
#include "plotWindows.h"
#include "axisSettings.h"

enum axisPart {TOP_PART, BOTTOM_PART};

/* Spacing parameters for 2D and 3D windows */
#define MARGIN_WIDTH 3
#define BUTTON_MARGIN 12

/* Spacing parameters for 2D windows */
#define NUM_FIELD_WIDTH 28
#define AXIS_LEFT (MARGIN_WIDTH+NUM_FIELD_WIDTH+1)
#define AXIS_BOTTOM 60
#define MESSAGE_LEFT (100-MARGIN_WIDTH-NUM_FIELD_WIDTH-4)

/* Spacing parameters for 3D windows */
#define NUM_FIELD_WIDTH_3D 23
#define AXIS_LEFT_3D (MARGIN_WIDTH+NUM_FIELD_WIDTH_3D+1)
#define AXIS_CENTER 50
#define AXIS_RIGHT (100-MARGIN_WIDTH-NUM_FIELD_WIDTH_3D-1)
#define Z_BOTTOM 46
#define XY_BOTTOM 77
#define XY_TEXT_DIST 10
#define MESSAGE_BOTTOM (Z_BOTTOM - 12)

typedef struct _axisWindow {
    double minXLim, minYLim, maxXLim, maxYLim;
    Boolean xIsLog, yIsLog;
    Widget minXT, minYT, maxXT, maxYT, linX, logX, linY, logY;
    windowInfo *wInfo;
} axisWindow;

typedef struct _axisWindow3D {
    double minXLim, minYLim, minZLim, maxXLim, maxYLim, maxZLim;
    Boolean xIsLog, yIsLog, zIsLog;
    Widget minXT, minYT, minZT, maxXT, maxYT, maxZT;
    Widget linX, logX, linY, logY, linZ, logZ;
    windowInfo *wInfo;
} axisWindow3D;

static char dragMessage[] = "Axes can also\nbe scaled by\ndragging on the\n\
axis ticks in\nthe plot window\nwith the left\nmouse button";
static char dragMessage3D[] = "Axes can also be scaled\nby dragging \
on the axis\nticks in the plot window\nwith the left mouse button";

static void destroy2DCB(Widget w, axisWindow *axisW, caddr_t callData);
static void close2DCB(Widget w, axisWindow *axisW, caddr_t callData);
static void ok2DCB(Widget w, axisWindow *axisW, caddr_t callData);
static void apply2DCB(Widget w, axisWindow *axisW, caddr_t callData);
static void cancel2DCB(Widget w, axisWindow *axisW, caddr_t callData);
static void axisExpose2DCB(Widget w, axisWindow *axisW, caddr_t callData);
static int apply2D(axisWindow *axisW);
static void destroy3DCB(Widget w, axisWindow3D *axisW, caddr_t callData);
static void close3DCB(Widget w, axisWindow3D *axisW, caddr_t callData);
static void ok3DCB(Widget w, axisWindow3D *axisW, caddr_t callData);
static void apply3DCB(Widget w, axisWindow3D *axisW, caddr_t callData);
static void cancel3DCB(Widget w, axisWindow3D *axisW, caddr_t callData);
static void axisExpose3DCB(Widget w, XtPointer whichPartXP, XtPointer callData);
static int apply3D(axisWindow3D *axisW);
static void matchAdjustFloat(double prototype, double *toFix);
static void get2DVisibleRange(Widget plotW, double *minXLim, double *minYLim,
    	double *maxXLim, double *maxYLim);

Widget Create2DAxisDialog(windowInfo *wInfo, int canLogX, int canLogY)
{
    Widget dialog, form, axisArea, plotW = wInfo->widget;
    Widget yLogRadio, linY, logY, frame, xLogRadio, linX, logX;
    Widget minYT, maxYT, minXT, maxXL, maxXT;
    Widget okBtn, applyBtn, cancelBtn;
    XmString s1;
    char minXString[20], minYString[20], maxXString[20], maxYString[20];
    axisWindow *axisW;
    
    
    /* allocate a data structure to pass the information about this window
       to the callback routines that need to read and change it */
    axisW = (axisWindow *)XtMalloc(sizeof(axisWindow));
    
    /* get the plotting bounds & scaling from the plot widget */
    get2DVisibleRange(plotW, &axisW->minXLim, &axisW->minYLim,
    	    &axisW->maxXLim, &axisW->maxYLim);
    if (XtClass(plotW) == scatWidgetClass || XtClass(plotW) == xyWidgetClass) {
    	XtVaGetValues(plotW, XmNxLogScaling, &axisW->xIsLog,
    		      XmNyLogScaling, &axisW->yIsLog, NULL);
    } else if (XtClass(plotW) == h1DWidgetClass) {
    	XtVaGetValues(plotW, XmNlogScaling, &axisW->yIsLog, NULL);
    	axisW->xIsLog = False;
    } else if (XtClass(plotW) == cellWidgetClass) {
    	axisW->xIsLog = False;
    	axisW->yIsLog = False;
    }

    /* create the dialog and contents */
    dialog = XtVaCreateWidget("axisSettings", xmDialogShellWidgetClass, plotW,
    		XmNtitle, "Axis Settings", NULL);
    AddMotifCloseCallback(dialog, (XtCallbackProc)close2DCB, axisW);
    XtAddCallback(dialog, XmNdestroyCallback, (XtCallbackProc)destroy2DCB,
    		  (caddr_t)axisW);
    form = XtVaCreateWidget("form", xmFormWidgetClass, dialog,
    		XmNnoResize, True,
    		XmNautoUnmanage, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    
    sprintf(maxYString, "%g", axisW->maxYLim);
    maxYT = XtVaCreateManagedWidget("maxYT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, maxYString,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, MARGIN_WIDTH,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH, NULL);
    RemapDeleteKey(maxYT);
    XtVaCreateManagedWidget("maxYL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Max Y"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxYT, NULL);
    XmStringFree(s1);
    yLogRadio = XtVaCreateManagedWidget("yLogRadio",xmRowColumnWidgetClass,form,
    		XmNradioBehavior, True,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT+2,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, MARGIN_WIDTH, NULL);
    linY = XtVaCreateManagedWidget("linY", xmToggleButtonWidgetClass, yLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Linear Y"),
    		XmNset, !axisW->yIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    logY = XtVaCreateManagedWidget("logY", xmToggleButtonWidgetClass, yLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Log Y"),
    		XmNset, axisW->yIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    xLogRadio = XtVaCreateManagedWidget("xLogRadio",xmRowColumnWidgetClass,form,
    		XmNradioBehavior, True,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MESSAGE_LEFT-2,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, AXIS_BOTTOM-3, NULL);
    linX = XtVaCreateManagedWidget("linX", xmToggleButtonWidgetClass, xLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Linear X"),
    		XmNset, !axisW->xIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    logX = XtVaCreateManagedWidget("logX", xmToggleButtonWidgetClass, xLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Log X"),
    		XmNset, axisW->xIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    frame = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, form,
        	XmNleftAttachment, XmATTACH_POSITION,
        	XmNleftPosition, MESSAGE_LEFT,
        	XmNrightAttachment, XmATTACH_POSITION,
        	XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, MARGIN_WIDTH,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, AXIS_BOTTOM - 4, NULL);
    XtVaCreateManagedWidget("dragMsg", xmLabelWidgetClass, frame, 
    		XmNlabelString, s1=XmStringCreateLtoR(dragMessage,
    					XmSTRING_DEFAULT_CHARSET), NULL);
    XmStringFree(s1);
    axisArea = XtVaCreateManagedWidget("axisAea", xmDrawingAreaWidgetClass,form, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, MARGIN_WIDTH,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, AXIS_BOTTOM, NULL);
    XtAddCallback(axisArea, XmNexposeCallback, (XtCallbackProc)axisExpose2DCB,
    		  (caddr_t)axisW);
    sprintf(minYString, "%g", axisW->minYLim);
    minYT = XtVaCreateManagedWidget("minYT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, minYString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, AXIS_BOTTOM, NULL);
    RemapDeleteKey(minYT);
    XtVaCreateManagedWidget("minYL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Min Y"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minYT, NULL);
    XmStringFree(s1);
    sprintf(minXString, "%g", axisW->minXLim);
    minXT = XtVaCreateManagedWidget("minXT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, minXString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_LEFT+NUM_FIELD_WIDTH,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, AXIS_BOTTOM+1, NULL);
    RemapDeleteKey(minXT);
    XtVaCreateManagedWidget("minXL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Min X"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_LEFT+NUM_FIELD_WIDTH,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minXT, NULL);
    XmStringFree(s1);
    sprintf(maxXString, "%g", axisW->maxXLim);
    maxXT = XtVaCreateManagedWidget("maxXT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, maxXString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 100-MARGIN_WIDTH-NUM_FIELD_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, AXIS_BOTTOM+1, NULL);
    RemapDeleteKey(maxXT);
    maxXL = XtVaCreateManagedWidget("maxXL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Max X"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 100-MARGIN_WIDTH-NUM_FIELD_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_WIDGET,
     		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxXT, NULL);
    XmStringFree(s1);
    okBtn = XtVaCreateManagedWidget("okBtn", xmPushButtonWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("OK"),
    		XmNshowAsDefault, True,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 12,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 28,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxXL,
    		XmNtopOffset, BUTTON_MARGIN, NULL);
    XmStringFree(s1);
    XtAddCallback(okBtn, XmNactivateCallback, (XtCallbackProc)ok2DCB,
    		  (caddr_t)axisW);
    applyBtn = XtVaCreateManagedWidget("apply", xmPushButtonWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Apply"),
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 42,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 58,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxXL,
    		XmNtopOffset, BUTTON_MARGIN, NULL);
    XtAddCallback(applyBtn, XmNactivateCallback, (XtCallbackProc)apply2DCB,
    		  (caddr_t)axisW);
    XmStringFree(s1);
    cancelBtn = XtVaCreateManagedWidget("cancel", xmPushButtonWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Cancel"),
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 72,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 88,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxXL,
    		XmNtopOffset, BUTTON_MARGIN, NULL);
    XtAddCallback(cancelBtn, XmNactivateCallback, (XtCallbackProc)cancel2DCB,
    		  (caddr_t)axisW);
    XmStringFree(s1);
    XtVaSetValues(form, XmNdefaultButton, okBtn, XmNcancelButton,cancelBtn, NULL);
    if (!canLogX)
    	XtUnmanageChild(xLogRadio);
    if (!canLogY)
    	XtUnmanageChild(yLogRadio);

    /* fill in rest of the axis window data structure for callback routines */
    axisW->minXT = minXT; axisW->maxXT = maxXT;
    axisW->minYT = minYT; axisW->maxYT = maxYT;
    axisW->linX = linX; axisW->logX = logX;
    axisW->linY = linY; axisW->logY = logY;
    axisW->wInfo = wInfo;

    /* pop up the dialog */
    XtManageChild(form);
    
    return dialog;
}

static void destroy2DCB(Widget w, axisWindow *axisW, caddr_t callData)
{
    /* deallocate the axisWindow data structure */
    XtFree((char *)axisW);
}

static void close2DCB(Widget w, axisWindow *axisW, caddr_t callData)
{
    Widget axisShell;
    
    /* destroy all of the widgets and the pointer to them.  The dialog's
       destroy CB takes care of deallocating the axisWindow data structure */
    axisShell = axisW->wInfo->axisSettings;
    axisW->wInfo->axisSettings = NULL;
    XtDestroyWidget(axisShell);
}

static void ok2DCB(Widget w, axisWindow *axisW, caddr_t callData)
{
    /* apply the changes to the axis settings dialog to the plot widget,
       and close the window if there were no errors in the users entries */
    if (apply2D(axisW))
    	close2DCB(w, axisW, callData);
}

static void apply2DCB(Widget w, axisWindow *axisW, caddr_t callData)
{
    apply2D(axisW);
}

static void cancel2DCB(Widget w, axisWindow *axisW, caddr_t callData)
{
    Widget plotW = axisW->wInfo->widget;
    
    /* reset the plotting bounds and scaling */
    if (XtClass(plotW) == scatWidgetClass) {
    	ScatSetVisibleRange(plotW, axisW->minXLim, axisW->minYLim,
    		axisW->maxXLim, axisW->maxYLim);
	XtVaSetValues(plotW, XmNxLogScaling, axisW->xIsLog,
		XmNyLogScaling, axisW->yIsLog, NULL);
    } else if (XtClass(plotW) == h1DWidgetClass) {
    	H1DSetVisibleRange(plotW, axisW->minXLim, axisW->minYLim,
    		axisW->maxXLim, axisW->maxYLim);
	XtVaSetValues(plotW, XmNlogScaling, axisW->yIsLog, NULL);
    } else if (XtClass(plotW) == xyWidgetClass) {
    	XYSetVisibleRange(plotW, axisW->minXLim, axisW->minYLim,
    		axisW->maxXLim, axisW->maxYLim);
	XtVaSetValues(plotW, XmNxLogScaling, axisW->xIsLog,
		XmNyLogScaling, axisW->yIsLog, NULL);
    }
    
    /* close the window and reclaim the memory */
    close2DCB(w, axisW, callData);
}

static void axisExpose2DCB(Widget w, axisWindow *axisW, caddr_t callData)
{
    GC gc;
    XGCValues values;
    Pixel foreground, background;
    Dimension width, height;

    XtVaGetValues(w, XmNforeground, &foreground, XmNbackground, &background,
    		  XmNwidth, &width, XmNheight, &height, NULL);
    values.foreground = foreground;
    values.background = background;
    values.line_width = 10;
    gc = XtGetGC(w, GCForeground|GCBackground|GCLineWidth, &values);
    XDrawLine(XtDisplay(w), XtWindow(w), gc, 1, 1, 1, height-1);
    XDrawLine(XtDisplay(w), XtWindow(w), gc, 1, height-1, width-1, height-1);
    XtReleaseGC(w, gc);
}

static int apply2D(axisWindow *axisW)
{
    Widget plotW = axisW->wInfo->widget;
    WidgetClass class = XtClass(plotW);
    double minXLim, minYLim, maxXLim, maxYLim, temp;
    double origMinXLim, origMinYLim, origMaxXLim, origMaxYLim;
    int resp;
    Boolean logX, logX1, logY, logY1;
    char *whichVar;

    /* get the axis limits from the dialog */
    if (GetFloatTextWarn(axisW->minXT, &minXLim, "Min X", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->minYT, &minYLim, "Min Y", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->maxXT, &maxXLim, "Max X", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->maxYT, &maxYLim, "Max Y", True) != TEXT_READ_OK)
    	return False;
    if (minXLim == maxXLim) {
        DialogF(DF_ERR, axisW->minXT, 1,"X axis minimum and maximum are equal\n\
Please re-enter value in Min X and Max X", "Acknowledged");
	return False;
    }
    if (minYLim == maxYLim) {
        DialogF(DF_ERR, axisW->minYT, 1,"Y axis minimum and maximum are equal\n\
Please re-enter value in Min Y and Max Y", "Acknowledged");
	return False;
    }
    if (minXLim > maxXLim) {
        resp = DialogF(DF_ERR, axisW->minXT, 2,
        		"X axis minimum is greater than maximum",
			"Reverse Min & Max", "Re-enter");
	if (resp == 2)
	    return False;
	temp = minXLim;
	minXLim = maxXLim;
	maxXLim = temp;
	SetFloatText(axisW->minXT, minXLim);
	SetFloatText(axisW->maxXT, maxXLim);
    }
    if (minYLim > maxYLim) {
        resp = DialogF(DF_ERR, axisW->minYT, 2,
        		"Y axis minimum is greater than maximum",
			"Reverse Min & Max", "Re-enter");
	if (resp == 2)
	    return False;
	temp = minYLim;
	minYLim = maxYLim;
	maxYLim = temp;
	SetFloatText(axisW->minYT, minYLim);
	SetFloatText(axisW->maxYT, maxYLim);
    }

    /* get the plot type (linear or log) information from the dialog */
    logX = logX1 = XmToggleButtonGetState(axisW->logX);
    logY = logY1 = XmToggleButtonGetState(axisW->logY);

    /* modify the plot widget with the linear/log settings from the dialog.
       If the widget rejects log scaling (by resetting the log scaling resource
       to false), tell the user and either cancel or continue the apply */
    if (class == scatWidgetClass || class == xyWidgetClass) {
    	XtVaSetValues(plotW, XmNxLogScaling, logX, XmNyLogScaling, logY, NULL);
    	XtVaGetValues(plotW, XmNxLogScaling, &logX1, XmNyLogScaling, &logY1, NULL);
    } else if (class == h1DWidgetClass) {
    	XtVaSetValues(plotW, XmNlogScaling, logY, NULL);
    	logY1 = logY;
    } else if (class == cellWidgetClass) {
    	logX1 = logX; logX1 = logX;
    }
    if (logX != logX1 && logY != logY1)
    	whichVar = "X and Y";
    else if (logX != logX1)
    	whichVar = "X";
    else if (logY != logY1)
    	whichVar = "Y";
    if (logX != logX1 || logY != logY1) {
    	XmToggleButtonSetState(axisW->logX, logX1, False);
    	XmToggleButtonSetState(axisW->logY, logY1, False);
    	XmToggleButtonSetState(axisW->linX, !logX1, False);
    	XmToggleButtonSetState(axisW->linY, !logY1, False);
    	resp = DialogF(DF_ERR, axisW->minXT, 2,
"Data contains %s values that are\nless than or equal to zero and\n\
can not be viewed with log scaling", "Change to Linear", "Cancel", whichVar);
	if (resp == 2)
	    return False;
	logX = logX1;
	logY = logY1;
    }
    
    /* Conversion of floating point numbers to text and back causes rounding
       errors which will make plotting limits no longer match data limits.
       If numbers match their printed representation, leave them alone. */
    matchAdjustFloat(axisW->minXLim, &minXLim);
    matchAdjustFloat(axisW->minYLim, &minYLim);
    matchAdjustFloat(axisW->maxXLim, &maxXLim);
    matchAdjustFloat(axisW->maxYLim, &maxYLim);
    
    /* If the requested limit is negative, and therefore inapplicable, in
       the log scaling case, leave that limit alone.  The result will be
       either no action or more importantly, to use the widget's better
       judgement on how the scaling should change when shifting to log. */
    get2DVisibleRange(plotW, &origMinXLim, &origMinYLim, &origMaxXLim,
    	    &origMaxYLim);
    if (logX && minXLim <= 0.)
    	minXLim = origMinXLim;
    if (logY && minYLim <= 0.)
    	minYLim = origMinYLim;
    
    /* modify the plot widget with the new range information from the dialog */
    if (class == scatWidgetClass)
    	ScatSetVisibleRange(plotW, minXLim, minYLim, maxXLim, maxYLim);
    if (class == h1DWidgetClass)
    	H1DSetVisibleRange(plotW, minXLim, minYLim, maxXLim, maxYLim);
    if (class == xyWidgetClass)
    	XYSetVisibleRange(plotW, minXLim, minYLim, maxXLim, maxYLim);
    if (class == cellWidgetClass)
    	CellSetVisibleRange(plotW, minXLim, minYLim, maxXLim, maxYLim);
    
    return True;
}

Widget Create3DAxisDialog(windowInfo *wInfo, int discreteXandY)
{
    Widget dialog, form, axisAreaTop, axisAreaBottom, frame;
    Widget yLogRadio, linY, logY, xLogRadio, linX, logX, zLogRadio, linZ, logZ;
    Widget minYT, maxYT, minXT, maxXT, minZT, maxZT, minYL;
    Widget okBtn, applyBtn, cancelBtn, plotW = wInfo->widget;
    XmString s1;
    char minXString[20], minYString[20], maxXString[20], maxYString[20];
    char minZString[20], maxZString[20];
    axisWindow3D *axisW;
    
    /* allocate a data structure to pass the information about this window
       to the callback routines that need to read and change it */
    axisW = (axisWindow3D *)XtMalloc(sizeof(axisWindow3D));
    
    /* get the plotting bounds and scaling from the plot widget */
    if (XtClass(plotW) == hist2DWidgetClass) {
    	hist2DGetVisiblePart(plotW, &axisW->minXLim, &axisW->maxXLim,
    	    &axisW->minYLim, &axisW->maxYLim, &axisW->minZLim, &axisW->maxZLim);
    	XtVaGetValues(plotW, XmNzLogScaling, &axisW->zIsLog, NULL);
    	axisW->xIsLog = False; axisW->yIsLog = False;
    } else if (XtClass(plotW) == scat3DWidgetClass) {
    	float  minXLim, maxXLim, minYLim, maxYLim, minZLim, maxZLim;
    	Scat3DGetVisiblePart(plotW, &minXLim, &maxXLim,
    	    &minYLim, &maxYLim, &minZLim, &maxZLim);
    	axisW->minXLim = minXLim; axisW->maxXLim = maxXLim;
    	axisW->minYLim = minYLim; axisW->maxYLim = maxYLim;
    	axisW->minZLim = minZLim; axisW->maxZLim = maxZLim;
    	XtVaGetValues(plotW, XmNxLogScaling, &axisW->xIsLog, 
    	    XmNyLogScaling, &axisW->yIsLog, XmNzLogScaling, &axisW->zIsLog, NULL);
    } else {
    	fprintf(stderr, "Internal error: 3DAxisDialog given wrong widget type");
    	return NULL;
    }
    
    /* create the dialog and contents */
    dialog = XtVaCreateWidget("axisSettings", xmDialogShellWidgetClass, plotW,
    		XmNtitle, "Axis Settings", NULL);
    AddMotifCloseCallback(dialog, (XtCallbackProc)close3DCB, axisW);
    XtAddCallback(dialog, XmNdestroyCallback, (XtCallbackProc)destroy3DCB,
    		  (caddr_t)axisW);
    form = XtVaCreateWidget("form", xmFormWidgetClass, dialog,
    		XmNnoResize, True,
    		XmNautoUnmanage, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    
    sprintf(maxZString, "%g", axisW->maxZLim);
    maxZT = XtVaCreateManagedWidget("maxZT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, maxZString,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, MARGIN_WIDTH,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH_3D, NULL);
    RemapDeleteKey(maxZT);
    XtVaCreateManagedWidget("maxZL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Max Z"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH_3D,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxZT, NULL);
    XmStringFree(s1);
    zLogRadio = XtVaCreateManagedWidget("zLogRadio",xmRowColumnWidgetClass,form,
    		XmNradioBehavior, True,
    		XmNmarginWidth, 0,
    		XmNmarginHeight, 0,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT_3D+2,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, MARGIN_WIDTH, NULL);
    linZ = XtVaCreateManagedWidget("linZ", xmToggleButtonWidgetClass, zLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Linear Z"),
    		XmNset, !axisW->zIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    logZ = XtVaCreateManagedWidget("logZ", xmToggleButtonWidgetClass, zLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Log Z"),
    		XmNset, axisW->zIsLog,
   		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    yLogRadio = XtVaCreateManagedWidget("yLogRadio",xmRowColumnWidgetClass,form,
    		XmNradioBehavior, True,
    		XmNmarginWidth, 0,
    		XmNmarginHeight, 0,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_CENTER-1,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, MESSAGE_BOTTOM, NULL);
    linY = XtVaCreateManagedWidget("linY", xmToggleButtonWidgetClass, yLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Linear Y"),
    		XmNset, !axisW->yIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    logY = XtVaCreateManagedWidget("logY", xmToggleButtonWidgetClass, yLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Log Y"),
    		XmNset, axisW->yIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    xLogRadio = XtVaCreateManagedWidget("xLogRadio",xmRowColumnWidgetClass,form,
    		XmNradioBehavior, True,
    		XmNmarginWidth, 0,
    		XmNmarginHeight, 0,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_CENTER+1,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, MESSAGE_BOTTOM, NULL);
    linX = XtVaCreateManagedWidget("linX", xmToggleButtonWidgetClass, xLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Linear X"),
    		XmNset, !axisW->xIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    logX = XtVaCreateManagedWidget("logX", xmToggleButtonWidgetClass, xLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Log X"),
    		XmNset, axisW->xIsLog,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    frame = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, form,
        	XmNleftAttachment, XmATTACH_POSITION,
        	XmNleftPosition, AXIS_CENTER+1,
        	XmNrightAttachment, XmATTACH_POSITION,
        	XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, MARGIN_WIDTH,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, MESSAGE_BOTTOM, NULL);
    XtVaCreateManagedWidget("dragMsg", xmLabelWidgetClass, frame, 
    		XmNlabelString, s1=XmStringCreateLtoR(dragMessage3D,
    					XmSTRING_DEFAULT_CHARSET), NULL);
    XmStringFree(s1);
    axisAreaTop = XtVaCreateManagedWidget("axisAeaTop",
    					  xmDrawingAreaWidgetClass, form, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT_3D,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_LEFT_3D + 10,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, MARGIN_WIDTH,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, Z_BOTTOM, NULL);
    XtAddCallback(axisAreaTop, XmNexposeCallback,
    		  (XtCallbackProc)axisExpose3DCB, (XtPointer)TOP_PART);
    sprintf(minZString, "%g", axisW->minZLim);
    minZT = XtVaCreateManagedWidget("minZT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, minZString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH_3D,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, Z_BOTTOM, NULL);
    RemapDeleteKey(minZT);
    XtVaCreateManagedWidget("minZL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Min Z"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH_3D,
    		XmNbottomAttachment, XmATTACH_WIDGET,
    		XmNbottomWidget, minZT, NULL);
    XmStringFree(s1);
    sprintf(maxXString, "%g", axisW->maxXLim);
    maxXT = XtVaCreateManagedWidget("maxXT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, maxXString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 100-MARGIN_WIDTH-NUM_FIELD_WIDTH_3D,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, Z_BOTTOM, NULL);
    RemapDeleteKey(maxXT);
    XtVaCreateManagedWidget("maxXL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Max X"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 100-MARGIN_WIDTH-NUM_FIELD_WIDTH_3D,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 100-MARGIN_WIDTH,
    		XmNtopAttachment, XmATTACH_WIDGET,
     		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxXT, NULL);
    XmStringFree(s1);
    sprintf(minXString, "%g", axisW->minXLim);
    minXT = XtVaCreateManagedWidget("minXT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, minXString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_CENTER+XY_TEXT_DIST,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_CENTER+XY_TEXT_DIST+NUM_FIELD_WIDTH_3D,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, XY_BOTTOM, NULL);
    RemapDeleteKey(minXT);
    XtVaCreateManagedWidget("minXL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Min X"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_CENTER+XY_TEXT_DIST,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_CENTER+XY_TEXT_DIST+NUM_FIELD_WIDTH_3D,
     		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minXT, NULL);
    XmStringFree(s1);
    sprintf(maxYString, "%g", axisW->maxYLim);
    maxYT = XtVaCreateManagedWidget("maxYT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, maxYString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH_3D,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, Z_BOTTOM, NULL);
    RemapDeleteKey(maxYT);
    XtVaCreateManagedWidget("maxYL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Max Y"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, MARGIN_WIDTH,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, MARGIN_WIDTH+NUM_FIELD_WIDTH_3D,
    		XmNtopAttachment, XmATTACH_WIDGET,
     		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, maxYT, NULL);
    XmStringFree(s1);
    sprintf(minYString, "%g", axisW->minYLim);
    minYT = XtVaCreateManagedWidget("minYT", xmTextWidgetClass, form, 
    		XmNcolumns, 13,
    		XmNvalue, minYString,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_CENTER-NUM_FIELD_WIDTH_3D-XY_TEXT_DIST,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_CENTER-XY_TEXT_DIST,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, XY_BOTTOM, NULL);
    RemapDeleteKey(minYT);
    minYL = XtVaCreateManagedWidget("minYL", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Min Y"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_CENTER-NUM_FIELD_WIDTH_3D-XY_TEXT_DIST,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_CENTER-XY_TEXT_DIST,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minYT, NULL);
    XmStringFree(s1);
    axisAreaBottom = XtVaCreateManagedWidget("axisAeaBottom",
    					     xmDrawingAreaWidgetClass, form, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, AXIS_LEFT_3D,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, AXIS_RIGHT,
    		XmNtopAttachment, XmATTACH_POSITION,
   		XmNtopPosition, Z_BOTTOM,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNbottomPosition, XY_BOTTOM, NULL);
    XtAddCallback(axisAreaBottom, XmNexposeCallback,
    		  (XtCallbackProc)axisExpose3DCB, (XtPointer)BOTTOM_PART);
    okBtn = XtVaCreateManagedWidget("okBtn", xmPushButtonWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("OK"),
    		XmNshowAsDefault, True,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 12,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 28,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minYL,
    		XmNtopOffset, BUTTON_MARGIN, NULL);
    XmStringFree(s1);
    XtAddCallback(okBtn, XmNactivateCallback, (XtCallbackProc)ok3DCB,
    		  (caddr_t)axisW);
    applyBtn = XtVaCreateManagedWidget("apply", xmPushButtonWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Apply"),
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 42,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 58,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minYL,
    		XmNtopOffset, BUTTON_MARGIN, NULL);
    XtAddCallback(applyBtn, XmNactivateCallback, (XtCallbackProc)apply3DCB,
    		  (caddr_t)axisW);
    XmStringFree(s1);
    cancelBtn = XtVaCreateManagedWidget("cancel", xmPushButtonWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("Cancel"),
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 72,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 88,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, minYL,
    		XmNtopOffset, BUTTON_MARGIN, NULL);
    XtAddCallback(cancelBtn, XmNactivateCallback, (XtCallbackProc)cancel3DCB,
    		  (caddr_t)axisW);
    XmStringFree(s1);
    XtVaSetValues(form, XmNdefaultButton, okBtn, XmNcancelButton,cancelBtn, NULL);
    if (discreteXandY) {
    	XtUnmanageChild(xLogRadio);
    	XtUnmanageChild(yLogRadio);
    }
    
    /* fill in rest of the axis window data structure for callback routines */
    axisW->minXT = minXT; axisW->maxXT = maxXT;
    axisW->minYT = minYT; axisW->maxYT = maxYT;
    axisW->minZT = minZT; axisW->maxZT = maxZT;
    axisW->linX = linX; axisW->logX = logX;
    axisW->linY = linY; axisW->logY = logY;
    axisW->linZ = linZ; axisW->logZ = logZ;
    axisW->wInfo = wInfo;

    /* pop up the dialog */
    XtManageChild(form);
    
    return dialog;
}

static void destroy3DCB(Widget w, axisWindow3D *axisW, caddr_t callData)
{
    /* deallocate the axisWindow3D data structure */
    XtFree((char *)axisW);
}

static void close3DCB(Widget w, axisWindow3D *axisW, caddr_t callData)
{
    Widget axisShell;
    
    /* destroy all of the widgets and the pointer to them.  The dialog's
       destroy CB takes care of deallocating the axisWindow3D data structure */
    axisShell = axisW->wInfo->axisSettings;
    axisW->wInfo->axisSettings = NULL;
    XtDestroyWidget(axisShell);
}

static void ok3DCB(Widget w, axisWindow3D *axisW, caddr_t callData)
{
    /* apply the changes to the axis settings dialog to the plot widget,
       and close the window if there were no errors in the users entries */
    if (apply3D(axisW))
    	close3DCB(w, axisW, callData);
}

static void apply3DCB(Widget w, axisWindow3D *axisW, caddr_t callData)
{
    apply3D(axisW);
}

static void cancel3DCB(Widget w, axisWindow3D *axisW, caddr_t callData)
{
    Widget plotW = axisW->wInfo->widget;
    
    /* reset the plotting bounds and scaling from the plot widget */
    if (XtClass(plotW) == hist2DWidgetClass) {
    	hist2DSetVisiblePart(plotW, axisW->minXLim, axisW->maxXLim,
    		axisW->minYLim, axisW->maxYLim, axisW->minZLim, axisW->maxZLim);
    	XtVaSetValues(plotW, XmNzLogScaling, axisW->zIsLog, NULL);
    }
    
    /* close the window and reclaim the memory */
    close3DCB(w, axisW, callData);
}

static void axisExpose3DCB(Widget w, XtPointer whichPartXP, XtPointer callData)
{
    GC gc;
    XGCValues values;
    Pixel foreground, background;
    Dimension width, height;
    long whichPart = (long)whichPartXP;

    XtVaGetValues(w, XmNforeground, &foreground, XmNbackground, &background,
    		  XmNwidth, &width, XmNheight, &height, NULL);
    values.foreground = foreground;
    values.background = background;
    values.line_width = 5;
    gc = XtGetGC(w, GCForeground|GCBackground|GCLineWidth, &values);
    if (whichPart == TOP_PART) {
    	XDrawLine(XtDisplay(w), XtWindow(w), gc, 2, 0, 2, height);
    } else {
    	XDrawLine(XtDisplay(w), XtWindow(w), gc, -2, -2, width/2+1, height-3);
    	XDrawLine(XtDisplay(w), XtWindow(w), gc, width/2-1, height-3,width-3,2);
    }
    XtReleaseGC(w, gc);
}

static int apply3D(axisWindow3D *axisW)
{
    double minXLim, minYLim, minZLim, maxXLim, maxYLim, maxZLim, temp;
    Widget plotW = axisW->wInfo->widget;
    WidgetClass class = XtClass(plotW);
    int resp;
    Boolean logX, logX1, logY, logY1, logZ, logZ1;
    char *whichVar;
    
    /* get the axis limits from the dialog and put them in the plot widget */
    if (GetFloatTextWarn(axisW->minXT, &minXLim, "Min X", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->minYT, &minYLim, "Min Y", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->minZT, &minZLim, "Min Z", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->maxXT, &maxXLim, "Max X", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->maxYT, &maxYLim, "Max Y", True) != TEXT_READ_OK)
    	return False;
    if (GetFloatTextWarn(axisW->maxZT, &maxZLim, "Max Z", True) != TEXT_READ_OK)
    	return False;
    if (minXLim == maxXLim) {
        DialogF(DF_ERR, axisW->minXT, 1,"X axis minimum and maximum are equal\n\
Please re-enter value in Min X and Max X", "Acknowledged");
	return False;
    }
    if (minYLim == maxYLim) {
        DialogF(DF_ERR, axisW->minYT, 1,"Y axis minimum and maximum are equal\n\
Please re-enter value in Min Y and Max Y", "Acknowledged");
	return False;
    }
    if (minZLim == maxZLim) {
        DialogF(DF_ERR, axisW->minZT, 1,"Z axis minimum and maximum are equal\n\
Please re-enter value in Min Z and Max Z", "Acknowledged");
	return False;
    }
    if (minXLim > maxXLim) {
        resp = DialogF(DF_ERR, axisW->minXT, 2,
        		"X axis minimum is greater than maximum",
			"Reverse Min & Max", "Re-enter");
	if (resp == 2)
	    return False;
	temp = minXLim;
	minXLim = maxXLim;
	maxXLim = temp;
	SetFloatText(axisW->minXT, minXLim);
	SetFloatText(axisW->maxXT, maxXLim);
    }
    if (minYLim > maxYLim) {
        resp = DialogF(DF_ERR, axisW->minYT, 2,
        		"Y axis minimum is greater than maximum",
			"Reverse Min & Max", "Re-enter");
	if (resp == 2)
	    return False;
	temp = minYLim;
	minYLim = maxYLim;
	maxYLim = temp;
	SetFloatText(axisW->minYT, minYLim);
	SetFloatText(axisW->maxYT, maxYLim);
    }
    if (minZLim > maxZLim) {
        resp = DialogF(DF_ERR, axisW->minZT, 2,
        		"Z axis minimum is greater than maximum",
			"Reverse Min & Max", "Re-enter");
	if (resp == 2)
	    return False;
	temp = minZLim;
	minZLim = maxZLim;
	maxZLim = temp;
	SetFloatText(axisW->minZT, minZLim);
	SetFloatText(axisW->maxZT, maxZLim);
    }

    /* get the plot type (linear or log) information from the dialog */
    logX = logX1 = XmToggleButtonGetState(axisW->logX);
    logY = logY1 = XmToggleButtonGetState(axisW->logY);
    logZ = logZ1 = XmToggleButtonGetState(axisW->logZ);

    /* modify the plot widget with the linear/log settings from the dialog.
       If the widget rejects log scaling (by resetting the log scaling resource
       to false), tell the user and either cancel or continue the apply */
    if (class == hist2DWidgetClass) {
   	XtVaSetValues(plotW, XmNzLogScaling, logZ, NULL);
    } else {
   	/* widget may reject log scaling on one or more axes: warn the user */
   	XtVaSetValues(plotW, XmNxLogScaling, logX, XmNyLogScaling, logY,
   		XmNzLogScaling, logZ, NULL);
    	XtVaGetValues(plotW, XmNxLogScaling, &logX1, XmNyLogScaling, &logY1,
   		XmNzLogScaling, &logZ1, NULL);
	if (logX != logX1 && logY != logY1 && logZ != logZ1)
    	    whichVar = "X, Y, and Z";
	else if (logX != logX1 && logY != logY1)
    	    whichVar = "X and Y";
	else if (logX != logX1 && logZ != logZ1)
    	    whichVar = "X and Z";
	else if (logY != logY1 && logZ != logZ1)
    	    whichVar = "Y and Z";
	else if (logX != logX1)
    	    whichVar = "X";
	else if (logY != logY1)
    	    whichVar = "Y";
	else if (logZ != logZ1)
    	    whichVar = "Z";
	if (logX != logX1) {
    	    XmToggleButtonSetState(axisW->logX, logX1, False);
    	    XmToggleButtonSetState(axisW->linX, !logX1, False);
    	}
    	if (logY != logY1) {
    	    XmToggleButtonSetState(axisW->logY, logY1, False);
    	    XmToggleButtonSetState(axisW->linY, !logY1, False);
    	}
    	if (logZ != logZ1) {
    	    XmToggleButtonSetState(axisW->logZ, logZ1, False);
    	    XmToggleButtonSetState(axisW->linZ, !logZ1, False);
    	}
    	if (logX != logX1 || logY != logY1 || logZ != logZ1) {
    	    resp = DialogF(DF_ERR, axisW->minXT, 2,
"Data contains %s values that are\nless than or equal to zero and\n\
can not be viewed with log scaling", "Change to Linear", "Cancel", whichVar);
	    if (resp == 2) {
		XtVaSetValues(plotW, XmNxLogScaling, logX, XmNyLogScaling,
			logY, XmNzLogScaling, logZ, NULL);
		return False;
	    }
	    logX = logX1;
	    logY = logY1;
	    logZ = logZ1;
	}
    }
    
    /* conversion of floating point numbers to text and back causes rounding
       errors which will make plotting limits no longer match data limits */
    matchAdjustFloat(axisW->minXLim, &minXLim);
    matchAdjustFloat(axisW->minYLim, &minYLim);
    matchAdjustFloat(axisW->minZLim, &minZLim);
    matchAdjustFloat(axisW->maxXLim, &maxXLim);
    matchAdjustFloat(axisW->maxYLim, &maxYLim);
    matchAdjustFloat(axisW->maxZLim, &maxZLim);

    /* catch any limits that are less than zero before doing a log plot */
    if (logX) {
    	if (minXLim <= 0) minXLim = FLT_MIN;
    	if (maxXLim <= 0) maxXLim = FLT_MIN*10;
    }
    if (logY) {
    	if (minYLim <= 0) minYLim = FLT_MIN;
    	if (maxYLim <= 0) maxYLim = FLT_MIN*10;
    }
    if (logZ) {
    	if (minZLim <= 0) minZLim = FLT_MIN;
    	if (maxZLim <= 0) maxZLim = FLT_MIN*10;
    }
    
    /* modify the plot widget with the new range information from the dialog */
    if (class == hist2DWidgetClass)
    	hist2DSetVisiblePart(plotW, minXLim, maxXLim,
    		minYLim, maxYLim, minZLim, maxZLim);
    else if (class == scat3DWidgetClass)
    	Scat3DSetVisiblePart(plotW, minXLim, maxXLim,
    		minYLim, maxYLim, minZLim, maxZLim);
    
    return True;
}

/*
** Repair floating point numbers that have lost precision from conversion
** to text and back by checking them against an original.  If the original
** and the converted number generate the same string, the routine corrects
** the number by storing the original back into it.
*/
static void matchAdjustFloat(double original, double *toFix)
{
    char originalStr[16], toFixStr[16];
    
    sprintf(originalStr, "%g", original);
    sprintf(toFixStr, "%g", *toFix);
    if (!strcmp(originalStr, toFixStr))
    	*toFix = original;
}

/*
** get the plotting bounds & scaling from a 2D plot widget
*/
static void get2DVisibleRange(Widget plotW, double *minXLim, double *minYLim,
    	double *maxXLim, double *maxYLim)
{
    if (XtClass(plotW) == scatWidgetClass) {
    	ScatGetVisibleRange(plotW, minXLim, minYLim,
    			    maxXLim, maxYLim);
    } else if (XtClass(plotW) == h1DWidgetClass) {
    	H1DGetVisibleRange(plotW, minXLim, minYLim,
    			    maxXLim, maxYLim);
    } else if (XtClass(plotW) == xyWidgetClass) {
    	XYGetVisibleRange(plotW, minXLim, minYLim,
    			      maxXLim, maxYLim);
    } else if (XtClass(plotW) == cellWidgetClass) {
    	CellGetVisibleRange(plotW, minXLim, minYLim,
    			    maxXLim, maxYLim);
    }
}
