/*******************************************************************************
*									       *
* Scat.c -- Scatter Plot Widget						       *
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
* May 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#include <Xm/DrawP.h>
#endif
#include "../util/psUtils.h"
#include "drawAxes.h"
#include "dragAxes.h"
#include "ScatP.h"
#include "XYTransform.h"

/* hack for some obsolete stuff -- igv */
void    _XmHighlightBorder();
void    _XmUnhighlightBorder();
void    _XmDrawShadow();
Boolean _XmDifferentBackground();

#define REDRAW_NONE 0
#define REDRAW_H_AXIS 1
#define REDRAW_V_AXIS 2
#define REDRAW_CONTENTS 4
#define REDRAW_LABELS 8
#define REDRAW_ALL 15

#define ZOOM_FACTOR .25		/* (linear) fraction of currently displayed
				   data to place outside of current limits 
				   when user invokes zoom command */
#define LEFT_MARGIN 0		/* empty space to left of widget.  Should be
				   small since v axis usually reserves more
				   space than it needs (for long numbers) */
#define TOP_MARGIN 7		/* empty space at top of widget */
#define RIGHT_MARGIN 0		/* empty space to right of widget.  Should be
				   small because h axis reserves more room
				   than it needs for last label to stick out */
#define BOTTOM_MARGIN 3		/* empty space at bottom of widget */
#define X_LABEL_MARGIN 7	/* space between x axis label and numbers */
#define Y_LABEL_MARGIN 5	/* space between y axis label and axis line */
#define MAX_AXIS_PERCENT 22	/* maximum percentage of widget width that may
				   be used for drawing axes */

/* formula for deciding between sparse (XDrawPoints)
   and dense (bitmap) drawing modes */
#define USE_BITMAP(width, height, n) ((width)*(height)/8 < 4*(n)*(int)(sizeof(XPoint)))

static void motionAP(ScatWidget w, XEvent *event, char *args, int n_args);
static void btnUpAP(ScatWidget w, XEvent *event, char *args, int n_args);
static void btn2AP(ScatWidget w, XEvent *event, char *args, int n_args);
static void btn3AP(ScatWidget w, XEvent *event, char *args, int n_args);
static void initialize(ScatWidget request, ScatWidget new);
static void redisplay(ScatWidget w, XEvent *event, Region region);
static void redisplayContents(ScatWidget w, int outDevice,
	int redrawArea, int thicken);
static void drawScatterPlot(ScatWidget w, Drawable drawBuf, int outDevice,
	int thicken);
static void drawSparseScatterPlot(ScatWidget w, Drawable drawBuf, int outDevice,
	int thicken);
static void drawDenseScatterPlot(ScatWidget w, Drawable drawBuf, int thicken);
static void destroy(ScatWidget w);
static void resize(ScatWidget w);
static Boolean setValues(ScatWidget current,ScatWidget request,ScatWidget new);
static void updateBufferAllocation(ScatWidget w);
static void updatePlotBitmapAllocation(ScatWidget w);
static XFontStruct *getFontStruct(XmFontList font);
static int comparePoints(const void *pt1, const void *pt2);
static void verifyDataForLogScaling(ScatWidget w);
void calcDataRange(ScatWidget w, double *xMin, double *xMax, double *yMin,
	double *yMax);
static double dMin(double d1, double d2);
static double dMax(double d1, double d2);
static void computeTransform(ScatWidget w, XYTransform *xform);

static char defaultTranslations[] = 
    "<Btn1Motion>: Motion()\n\
     <Btn1Down>: Motion()\n\
     <Btn1Up>: BtnUp()\n\
     <Btn2Down>: Btn2Press()\n\
     <Btn3Down>: Btn3Press()\n";

static XtActionsRec actionsList[] = {
    {"Motion", (XtActionProc)motionAP},
    {"BtnUp", (XtActionProc)btnUpAP},
    {"Btn2Press", (XtActionProc)btn2AP},
    {"Btn3Press", (XtActionProc)btn3AP}
};

static XtResource resources[] = {
    {XmNdoubleBuffer, XmCDoubleBuffer, XmRBoolean, sizeof(Boolean),
      XtOffset(ScatWidget, scat.doubleBuffer), XmRString, "False"},
    {XmNdarkerPoints, XmCDarkerPoints, XmRBoolean, sizeof(Boolean),
      XtOffset(ScatWidget, scat.darkerPoints), XmRString, "True"},
    {XmNxLogScaling, XmCXLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(ScatWidget, scat.xLogScaling), XmRString, "False"},
    {XmNyLogScaling, XmCYLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(ScatWidget, scat.yLogScaling), XmRString, "False"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(ScatWidget, scat.font), XmRImmediate, NULL},
    {XmNxAxisLabel, XmCXAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(ScatWidget, scat.xAxisLabel), XmRString, NULL},
    {XmNyAxisLabel, XmCYAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(ScatWidget, scat.yAxisLabel), XmRString, NULL},
    {XmNpsFont, XmCPsFont, XmRXmString, sizeof (XmString),
      XtOffset(ScatWidget, scat.psFont), XmRString, NULL},
    {XmNpsFontSize, XmCPsFontSize, XmRInt, sizeof(int),
      XtOffset(ScatWidget, scat.psFontSize), XmRString, "0"},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (ScatWidget, scat.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (ScatWidget, scat.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (ScatWidget, scat.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (ScatWidget, scat.redisplay), XtRCallback, NULL},
    {XmNaddLeftMargin, XmCAddLeftMargin, XmRInt, sizeof(int),
      XtOffset(ScatWidget, scat.addLeft), XmRString, "0"},
    {XmNaddRightMargin, XmCAddRightMargin, XmRInt, sizeof(int),
      XtOffset(ScatWidget, scat.addRight), XmRString, "0"},
    {XmNaddBottomMargin, XmCAddBottomMargin, XmRInt, sizeof(int),
      XtOffset(ScatWidget, scat.addBottom), XmRString, "0"},
    {XmNaddTopMargin, XmCAddTopMargin, XmRInt, sizeof(int),
      XtOffset(ScatWidget, scat.addTop), XmRString, "0"}
};

ScatClassRec  scatClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass       */
    "Scat",                         /* class_name            */
    sizeof(ScatRec),                /* widget_size           */
    NULL,                           /* class_initialize      */
    NULL,                           /* class_part_initialize */
    FALSE,                          /* class_inited          */
    (XtInitProc)initialize,         /* initialize            */
    NULL,                           /* initialize_hook       */
    XtInheritRealize,               /* realize               */
    actionsList,                    /* actions               */
    XtNumber(actionsList),          /* num_actions           */
    resources,                      /* resources             */
    XtNumber(resources),            /* num_resources         */
    NULLQUARK,                      /* xrm_class             */
    TRUE,                           /* compress_motion       */
    TRUE,                           /* compress_exposure     */
    TRUE,                           /* compress_enterleave   */
    TRUE,                           /* visible_interest      */
    (XtWidgetProc)destroy,          /* destroy               */
    (XtWidgetProc)resize,           /* resize                */
    (XtExposeProc)redisplay,        /* expose                */
    (XtSetValuesFunc)setValues,     /* set_values            */
    NULL,                           /* set_values_hook       */
    XtInheritSetValuesAlmost,       /* set_values_almost     */
    NULL,                           /* get_values_hook       */
    NULL,                           /* accept_focus          */
    XtVersion,                      /* version               */
    NULL,                           /* callback private      */
    defaultTranslations,            /* tm_table              */
    NULL,                           /* query_geometry        */
    NULL,                           /* display_accelerator   */
    NULL,                           /* extension             */
  },
  /* Motif primitive class fields */
  {
     (XtWidgetProc)_XtInherit,   	/* Primitive border_highlight   */
     (XtWidgetProc)_XtInherit,   	/* Primitive border_unhighlight */
     XtInheritTranslations,		/* translations                 */
    (XtActionProc)motionAP,		/* arm_and_activate             */
     NULL,				/* get resources      		*/
     0,					/* num get_resources  		*/
     NULL,         			/* extension                    */
  },
  /* Scat class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass scatWidgetClass = (WidgetClass)&scatClassRec;

/*
** Widget initialize method
*/
static void initialize(ScatWidget request, ScatWidget new)
{
    XGCValues values;
    Display *display = XtDisplay(new);
   
    /* Make sure the window size is not zero. The Core 
       initialize() method doesn't do this. */
    if (request->core.width == 0)
    	new->core.width = 500;
    if (request->core.height == 0)
   	new->core.height = 400;

    /* Make a local copy of the fontlist,
       or get the default if not specified */
    if (new->scat.font == NULL)
#ifdef MOTIF10
	new->scat.font = XmFontListCreate(
	    XLoadQueryFont(display, "fixed"),
	    XmSTRING_DEFAULT_CHARSET);
#else
    	new->scat.font =
    	    XmFontListCopy(_XmGetDefaultFontList(
		(Widget) new, XmLABEL_FONTLIST));
#endif
    else
        new->scat.font = XmFontListCopy(new->scat.font);

    /* Make local copies of the XmStrings */
    if (new->scat.xAxisLabel != NULL)
    	new->scat.xAxisLabel = XmStringCopy(new->scat.xAxisLabel);
    if (new->scat.yAxisLabel != NULL)
    	new->scat.yAxisLabel = XmStringCopy(new->scat.yAxisLabel);
    if (new->scat.psFont != NULL)
    	new->scat.psFont = XmStringCopy(new->scat.psFont);

    /* Create graphics contexts for drawing in the widget */
    values.font = getFontStruct(new->scat.font)->fid;
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    new->scat.gc = XCreateGC(display, XDefaultRootWindow(display),
    			     GCForeground|GCBackground|GCFont, &values);
    new->scat.contentsGC = XCreateGC(display, XDefaultRootWindow(display),
    				     GCForeground|GCBackground, &values);
    
    /* Initialize various fields */
    ResetAxisDragging(&new->scat.dragState);
    new->scat.points = NULL;
    new->scat.nPoints = 0;
    new->scat.isColor = False;
    new->scat.drawBuffer = 0;
    new->scat.plotImage = NULL;
    new->scat.xEnd = 0;
    new->scat.xOrigin = 0;
    new->scat.yOrigin = 0;
    new->scat.yEnd = 0;
    
    /* Default plotting boundaries */
    ScatSetContents((Widget)new, NULL, 0, SCAT_RESCALE);

    /* Set size dependent items */
    resize(new);
}

/*
** Widget destroy method
*/
static void destroy(ScatWidget w)
{
    XFreeGC(XtDisplay(w), w->scat.gc);
    XFreeGC(XtDisplay(w), w->scat.contentsGC);
    if (w->scat.font != NULL)
    	XmFontListFree(w->scat.font);
    if (w->scat.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->scat.drawBuffer);
    if (w->scat.plotImage != NULL) {
    	XtFree((char *)w->scat.plotImage);
    	XtFree((char *)w->scat.plotBits);
    }
    XtRemoveAllCallbacks ((Widget)w, XmNresizeCallback);
    XtRemoveAllCallbacks ((Widget)w, XmNbtn2Callback);
    XtRemoveAllCallbacks ((Widget)w, XmNbtn3Callback);
    XtRemoveAllCallbacks ((Widget)w, XmNredisplayCallback);
    if (w->scat.xAxisLabel != NULL)
    	XmStringFree(w->scat.xAxisLabel);
    if (w->scat.yAxisLabel != NULL)
    	XmStringFree(w->scat.yAxisLabel);
    if (w->scat.psFont != NULL)
        XmStringFree(w->scat.psFont);
    if (w->scat.points != NULL)
    	XtFree((char *) w->scat.points);
}

/*
** Widget resize method
*/
static void resize(ScatWidget w)
{
    XFontStruct *fs = getFontStruct(w->scat.font);
    int borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;

    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(w);

    /* calculate the area of the widget where contents can be drawn */
    setWidgetContentArea(
	w->core.width, w->core.height, borderWidth, fs,
	w->scat.addLeft, w->scat.addRight, w->scat.addTop, w->scat.addBottom,
	&w->scat.xMin, &w->scat.yMin, &w->scat.xMax, &w->scat.yMax);

    /* calculate positions for the axes and contents depending on whether
       axis labels are specified, and the measurements of the current font */
    if (w->scat.yAxisLabel != NULL)
    	w->scat.yEnd = w->scat.yMin + fs->ascent + fs->descent + TOP_MARGIN;
    else
    	w->scat.yEnd = VAxisEndClearance(fs) + fs->ascent/2 + TOP_MARGIN;
    w->scat.axisTop = w->scat.yEnd - VAxisEndClearance(fs);
    if (w->scat.xAxisLabel != NULL)
    	w->scat.axisBottom = w->scat.yMax - BOTTOM_MARGIN - fs->ascent -
    				fs->descent - X_LABEL_MARGIN;
    else
    	w->scat.axisBottom = w->scat.yMax - fs->ascent/2 - BOTTOM_MARGIN;
    w->scat.yOrigin = w->scat.axisBottom - HAxisHeight(fs);
    w->scat.axisLeft = w->scat.xMin + LEFT_MARGIN;
    w->scat.xOrigin = w->scat.axisLeft + VAxisWidth(fs);
    if (w->scat.xOrigin > w->scat.xMin + (w->scat.xMax - w->scat.xMin) * MAX_AXIS_PERCENT/100)
    	w->scat.xOrigin = w->scat.xMin + (w->scat.xMax - w->scat.xMin) * MAX_AXIS_PERCENT/100;
    w->scat.axisRight = w->scat.xMax - RIGHT_MARGIN;
    w->scat.xEnd = w->scat.axisRight - HAxisEndClearance(fs);
    if (w->scat.xMax - w->scat.xEnd > w->scat.xOrigin - w->scat.xMin)
    	w->scat.xEnd = w->scat.xMax - (w->scat.xOrigin - w->scat.xMin);
    
    /* set plot contents gc to clip drawing at the edges */
    clipRect.x = w->scat.xOrigin;
    clipRect.y = w->scat.yEnd;
    clipRect.width = w->scat.xEnd - w->scat.xOrigin;
    clipRect.height = w->scat.yOrigin - w->scat.yEnd;
    XSetClipRectangles(XtDisplay(w), w->scat.contentsGC, 0, 0, &clipRect,
    		       1, Unsorted);

    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), w->scat.gc, 0, 0, &clipRect, 1, Unsorted);

    /* decide if picture is sparse or dense and reallocate plot image bitmap */
    updatePlotBitmapAllocation(w);

    /* call the resize callback */
    if (XtIsRealized((Widget)w))
    {
	XYCallbackStruct cbStruct;
	XYTransform xform;
	computeTransform(w, &xform);
	cbStruct.reason = XmCR_RESIZE;
	cbStruct.event = NULL;
	cbStruct.xform = &xform;
        cbStruct.outDevice = X_SCREEN;
	cbStruct.drawBuf = XtWindow((Widget)w);
	cbStruct.wingc = w->scat.gc;
	cbStruct.plotgc = w->scat.contentsGC;
    	XtCallCallbacks((Widget)w, XmNresizeCallback, (XtPointer)&cbStruct);
    }
}

/*
** Widget redisplay method
*/
static void redisplay(ScatWidget w, XEvent *event, Region region)
{
    XEvent ev;
    Display *dpy;

    /* igv: don't do anything until the expose count drops to 0 */
    if (event)
	if (event->type == Expose)
	    if (event->xexpose.count > 0)
		return;

    /* igv: don't do anything if the next event
       is again an expose event for this window */
    dpy = XtDisplay(w);
    if (XQLength(dpy) > 0)
    {
  	XPeekEvent(dpy, &ev);
  	if (ev.type == Expose)
  	    if (ev.xexpose.window == XtWindow(w))
 		return;
    }

    /* Draw the Motif required shadows and highlights */
    if (w->primitive.shadow_thickness > 0) {
	_XmDrawShadow (dpy, XtWindow(w), 
		       w->primitive.bottom_shadow_GC,
		       w->primitive.top_shadow_GC,
                       w->primitive.shadow_thickness,
                       w->primitive.highlight_thickness,
                       w->primitive.highlight_thickness,
                       w->core.width - 2 * w->primitive.highlight_thickness,
                       w->core.height-2 * w->primitive.highlight_thickness);
    }
    if (w->primitive.highlighted)
	_XmHighlightBorder((Widget)w);
    else if (_XmDifferentBackground((Widget)w, XtParent((Widget)w)))
	_XmUnhighlightBorder((Widget)w);
    
    /* Now draw the contents of the scat widget */
    redisplayContents(w, X_SCREEN, REDRAW_ALL, True);
}

/*
** Widget setValues method
*/
static Boolean setValues(ScatWidget current, ScatWidget request, ScatWidget new)
{
    Boolean redraw = False, doResize = False;
    Display *display = XtDisplay(new);

    /* If the foreground or background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetForeground(display, new->scat.gc, new->primitive.foreground);
    	XSetForeground(display, new->scat.contentsGC,new->primitive.foreground);
    	redraw = TRUE;
    }
    if (new->primitive.foreground != current->primitive.foreground) {
    	XSetBackground(display, new->scat.gc, new->core.background_pixel);
    	XSetBackground(display,new->scat.contentsGC,new->core.background_pixel);
    	redraw = TRUE;
    }

    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->scat.doubleBuffer != current->scat.doubleBuffer) {
    	updateBufferAllocation(new);
    	redraw = TRUE;
    }

    /* if point darkening changes, redraw */
    if (new->scat.darkerPoints != current->scat.darkerPoints)
    	redraw = TRUE;

    /* if log scaling changes, verify data, reset dragging, and redraw */
    if   (new->scat.xLogScaling != current->scat.xLogScaling ||
    	  new->scat.yLogScaling != current->scat.yLogScaling) {
    	verifyDataForLogScaling(new);
    	ResetAxisDragging(&new->scat.dragState);
    	redraw = TRUE;
    }

    /* if labels are changed, free the old ones and copy the new ones */
    if (new->scat.xAxisLabel != current->scat.xAxisLabel) {
    	if (current->scat.xAxisLabel != NULL)
    	    XmStringFree(current->scat.xAxisLabel);
    	new->scat.xAxisLabel = XmStringCopy(new->scat.xAxisLabel);
    	doResize = TRUE;
    }
    if (new->scat.yAxisLabel != current->scat.yAxisLabel) {
    	if (current->scat.yAxisLabel != NULL)
    	    XmStringFree(current->scat.yAxisLabel);
    	new->scat.yAxisLabel = XmStringCopy(new->scat.yAxisLabel);
    	doResize = TRUE;
    }
    
    /* if highlight thickness or shadow thickness changed, resize and redraw */
    if  ((new->primitive.highlight_thickness != 
          current->primitive.highlight_thickness) ||
         (new -> primitive.shadow_thickness !=
          current->primitive.shadow_thickness)) {
    	redraw = TRUE;
    }
    if (doResize)
    	resize(new);
    return redraw; 
} 

/*
** Button press and button motion action proc.
*/
static void motionAP(ScatWidget w, XEvent *event, char *args, int n_args)
{
    int chgdArea, redrawArea = REDRAW_NONE;

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    
    chgdArea = DragAxes(event, w->scat.xOrigin, w->scat.xEnd, w->scat.yOrigin,
    	w->scat.yEnd, w->scat.axisLeft, w->scat.axisTop, w->scat.axisBottom,
    	w->scat.axisRight, w->scat.minXData, w->scat.maxXData, w->scat.minYData,
    	w->scat.maxYData, w->scat.xLogScaling, w->scat.yLogScaling,
    	&w->scat.minXLim, &w->scat.maxXLim, &w->scat.minYLim, &w->scat.maxYLim,
    	&w->scat.dragState, &w->scat.xDragStart, &w->scat.yDragStart);
    if (chgdArea & DA_REDRAW_H_AXIS) redrawArea |= REDRAW_H_AXIS;
    if (chgdArea & DA_REDRAW_V_AXIS) redrawArea |= REDRAW_V_AXIS;
    if (chgdArea & DA_REDRAW_CONTENTS) redrawArea |= REDRAW_CONTENTS;

    redisplayContents(w, X_SCREEN, redrawArea, False);
}
/*
** Button up action proc.
*/
static void btnUpAP(ScatWidget w, XEvent *event, char *args, int n_args)
{
    ResetAxisDragging(&w->scat.dragState);
    if (w->scat.darkerPoints) {
    	if (w->scat.plotImage != NULL)
    	    redisplayContents(w, X_SCREEN, REDRAW_CONTENTS, True);
	else
    	    drawScatterPlot(w, XtWindow(w), X_SCREEN, True); 
    }
}

static void btn2AP(ScatWidget w, XEvent *event, char *args, int n_args)
{
    ScatCallbackStruct cbStruct;

#ifdef MOTIF10
    _XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
#else
    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
#endif   

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks((Widget)w, XmNbtn2Callback, (XtPointer)&cbStruct);
}

static void btn3AP(ScatWidget w, XEvent *event, char *args, int n_args)
{    
    XYCallbackStruct cbStruct;
    XYTransform xform;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Compute constants for user coordinate transformation */
    computeTransform(w, &xform);

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    cbStruct.xform = &xform;
    cbStruct.outDevice = X_SCREEN;
    cbStruct.drawBuf = 0;
    cbStruct.wingc = w->scat.gc;
    cbStruct.plotgc = w->scat.contentsGC;
    XtCallCallbacks((Widget)w, XmNbtn3Callback, (XtPointer)&cbStruct);
}

/*
** ScatSetContents
**
** Specify the contents of the scat widget in the form of points in 2D space.
** Takes an array of ScatPoint data structures which contain the X and Y
** coordinates of the point.
**
** Parameters
**
** 	w		A scat widget
**	points		An array of points to display
**	nPoints		The number of elements specified in points
**	rescale		One of: SCAT_NO_RESCALE, SCAT_RESCALE, or
**			SCAT_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
*/
void ScatSetContents(Widget w, ScatPoint *points, int nPoints, int rescale)
{
    ScatWidget sw = (ScatWidget)w;
    int redrawArea = REDRAW_NONE;
    double minX, minY, maxX, maxY;
    
    /* Free the previous data */
    if (sw->scat.nPoints != 0) {
    	XtFree((char *)sw->scat.points);
    	sw->scat.nPoints = 0;
    }
    
    /* Copy in the new data, if any, and calculate the min and max values */
    sw->scat.nPoints = nPoints;
    if (nPoints == 0) {
    	if (rescale == SCAT_RESCALE) {
    	    minX = minY = 0.;
    	    maxX = maxY = 1.;
    	} else {
    	    minX = sw->scat.minXData; minY = sw->scat.minYData;
    	    maxX = sw->scat.maxXData; maxY = sw->scat.maxYData;
    	}
    	sw->scat.isColor = False;
    } else {
	Pixel black = BlackPixelOfScreen(XtScreen(w));
    	/* allocate memory and copy the data */
    	sw->scat.points = (ScatPoint *)XtMalloc(sizeof(ScatPoint) * nPoints);
    	memcpy(sw->scat.points, points, sizeof(ScatPoint) * nPoints);
	sw->scat.nPoints = nPoints;
	/* Sort points by color so the drawing routines can batch drawing calls */
	qsort(sw->scat.points, nPoints, sizeof(ScatPoint), comparePoints);
	sw->scat.isColor = sw->scat.points[0].pixel != black ||
	                   sw->scat.points[nPoints-1].pixel != black;
	/* Calculate the range of the data */
	calcDataRange(sw, &minX, &maxX, &minY, &maxY);
    }
    
    /* recalculate scale and limits for the widget */
    if (rescale == SCAT_RESCALE) {
	if (maxX == minX) {maxX += 1.; minX -= 1.;}  /* fix zero range data */
	if (maxY == minY) {maxY += 1.; minY -= 1.;}
    	sw->scat.maxXData = maxX; sw->scat.maxXLim = maxX;
    	sw->scat.minXData = minX; sw->scat.minXLim = minX;
    	sw->scat.maxYData = maxY; sw->scat.maxYLim = maxY;
    	sw->scat.minYData = minY; sw->scat.minYLim = minY;
    	redrawArea = REDRAW_CONTENTS | REDRAW_H_AXIS | REDRAW_V_AXIS;
    } else if (rescale == SCAT_NO_RESCALE || rescale == SCAT_REFRESH) {
	if (rescale == SCAT_NO_RESCALE)
	    redrawArea = REDRAW_CONTENTS;
	else
	    redrawArea = REDRAW_ALL;
	if (maxX > sw->scat.maxXData) {
	    sw->scat.maxXData = maxX;
	    redrawArea |= REDRAW_H_AXIS;
	}
	if (minX < sw->scat.minXData) {
	    sw->scat.minXData = minX;
	    redrawArea |= REDRAW_H_AXIS;
	}
	if (maxY > sw->scat.maxYData) {
	    sw->scat.maxYData = maxY;
	    redrawArea |= REDRAW_V_AXIS;
	}
	if (minY < sw->scat.minYData) {
	    sw->scat.minYData = minY;
	    redrawArea |= REDRAW_V_AXIS;
	}
    } else if (rescale == SCAT_RESCALE_AT_MAX || rescale == SCAT_GROW_ONLY) {
	redrawArea |= REDRAW_CONTENTS;
	if (rescale == SCAT_GROW_ONLY) {
	    minX = dMin(sw->scat.minXData, minX);
	    maxX = dMax(sw->scat.maxXData, maxX);
	    minY = dMin(sw->scat.minYData, minY);
	    maxY = dMax(sw->scat.maxYData, maxY);
    	}
    	if (sw->scat.maxXData != maxX || sw->scat.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (sw->scat.maxYData != maxY || sw->scat.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
    	if (sw->scat.maxXData == sw->scat.maxXLim)
    	    sw->scat.maxXLim = maxX;
    	if (sw->scat.minXData == sw->scat.minXLim)
    	    sw->scat.minXLim = minX;
    	if (sw->scat.maxYData == sw->scat.maxYLim)
    	    sw->scat.maxYLim = maxY;
    	if (sw->scat.minYData == sw->scat.minYLim)
    	    sw->scat.minYLim = minY;
    	sw->scat.maxXData = maxX; sw->scat.minXData = minX;
   	sw->scat.maxYData = maxY; sw->scat.minYData = minY;
    }
    
    /* decide on drawing method based on how sparse or dense data is */
    updatePlotBitmapAllocation(sw);
    
    /* if log scaling was requested, make sure data is log scaleable */
    verifyDataForLogScaling(sw);
    
    /* redraw the widget with the new data */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN, redrawArea, True);
}

/*
** ScatSetVisibleRange, ScatGetVisibleRange
**
** Set (Get) the range of data that is visible.  minXLim, minYLim, maxXLim, and
** maxYLim specify the endpoints of the x and y axes.  ScatSetVisibleRange,
** unlike the widgets interactive rescaling routines, can zoom out past the
** actual minimum and maximum data points.
*/
void ScatSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim)
{
    ScatWidget sw = (ScatWidget)w;
    double minX, minY, maxX, maxY;
    
    /* calculate the actual range of the data */
    calcDataRange(sw, &minX, &maxX, &minY, &maxY);

    /* allow user to zoom beyond the range of the data */
    sw->scat.maxXData = dMax(maxXLim, maxX);
    sw->scat.minXData = dMin(minXLim, minX);
    sw->scat.maxYData = dMax(maxYLim, maxY);
    sw->scat.minYData = dMin(minYLim, minY);

    /* Set the range */
    sw->scat.minXLim = minXLim;
    sw->scat.maxXLim = maxXLim;
    sw->scat.minYLim = minYLim;
    sw->scat.maxYLim = maxYLim;
    
    /* if log scaling was requested, make sure new range is log scaleable */
    verifyDataForLogScaling(sw);

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS, True);
}
void ScatGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			 double *maxXLim, double *maxYLim)
{
    *minXLim = ((ScatWidget)w)->scat.minXLim;
    *maxXLim = ((ScatWidget)w)->scat.maxXLim;
    *minYLim = ((ScatWidget)w)->scat.minYLim;
    *maxYLim = ((ScatWidget)w)->scat.maxYLim;
}

/*
** ScatZoomOut, ScatZoomIn, ScatResetZoom
**
** Zoom in and out by ZOOM_FACTOR.  Zoom in is centered on the current
** center of the plot.
*/
void ScatZoomOut(Widget w)
{
    ScatWidget sw = (ScatWidget)w;
    int xLogScaling = sw->scat.xLogScaling, yLogScaling = sw->scat.yLogScaling;
    double xOffset, yOffset, newMaxXLim, newMinXLim, newMaxYLim, newMinYLim;
    double minXLim, maxXLim, minYLim, maxYLim;
    int redrawArea = REDRAW_NONE;
    
    /* if log scaling was requested, express limits in log coordinates */
    minXLim = xLogScaling ? log10(sw->scat.minXLim) : sw->scat.minXLim;
    maxXLim = xLogScaling ? log10(sw->scat.maxXLim) : sw->scat.maxXLim;
    minYLim = yLogScaling ? log10(sw->scat.minYLim) : sw->scat.minYLim;
    maxYLim = yLogScaling ? log10(sw->scat.maxYLim) : sw->scat.maxYLim;

    /* Calculate a suitable offset to reverse a zoom in by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;
    yOffset = (maxYLim - minYLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;

    /* widen the plotting limits by the offsets calculated above,
       stopping when the limits reach the limits of the data */
    newMaxXLim = dMin(sw->scat.maxXData,
    	    xLogScaling ? pow(10., maxXLim + xOffset) : maxXLim + xOffset);
    newMinXLim = dMax(sw->scat.minXData,
    	    xLogScaling ? pow(10., minXLim - xOffset) : minXLim - xOffset);
    newMaxYLim = dMin(sw->scat.maxYData,
    	    yLogScaling ? pow(10., maxYLim + yOffset) : maxYLim + yOffset);
    newMinYLim = dMax(sw->scat.minYData,
    	    yLogScaling ? pow(10., minYLim - yOffset) : minYLim - yOffset);
    
    /* Tell widget to redraw, and what parts, if limits have changed */
    if (newMaxXLim != maxXLim || newMinXLim != minXLim)
    	redrawArea |= REDRAW_H_AXIS | REDRAW_CONTENTS;
    if (newMaxYLim != maxYLim || newMinYLim != minYLim)
    	redrawArea |= REDRAW_V_AXIS | REDRAW_CONTENTS;
    
    /* Set the new limits */
    sw->scat.maxXLim = newMaxXLim;
    sw->scat.minXLim = newMinXLim;
    sw->scat.maxYLim = newMaxYLim;
    sw->scat.minYLim = newMinYLim;

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN, redrawArea, True);
}
void ScatZoomIn(Widget w)
{
    ScatWidget sw = (ScatWidget)w;
    int xLogScaling = sw->scat.xLogScaling, yLogScaling = sw->scat.yLogScaling;
    double xOffset, yOffset;
    double minXLim, maxXLim, minYLim, maxYLim;

    /* if log scaling was requested, express limits in log coordinates */
    minXLim = xLogScaling ? log10(sw->scat.minXLim) : sw->scat.minXLim;
    maxXLim = xLogScaling ? log10(sw->scat.maxXLim) : sw->scat.maxXLim;
    minYLim = yLogScaling ? log10(sw->scat.minYLim) : sw->scat.minYLim;
    maxYLim = yLogScaling ? log10(sw->scat.maxYLim) : sw->scat.maxYLim;
    
    /* Calculate offsets for limits of displayed data to zoom by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * ZOOM_FACTOR / 2;
    yOffset = (maxYLim - minYLim) * ZOOM_FACTOR / 2;

    /* Narrow the plotting limits by the offsets calculated above */
    maxXLim -= xOffset;
    minXLim += xOffset;
    maxYLim -= yOffset;
    minYLim += yOffset;
    
    /* Set the new limits */
    sw->scat.maxXLim = xLogScaling ? pow(10.,maxXLim) : maxXLim;
    sw->scat.minXLim = xLogScaling ? pow(10.,minXLim) : minXLim;
    sw->scat.maxYLim = yLogScaling ? pow(10.,maxYLim) : maxYLim;
    sw->scat.minYLim = yLogScaling ? pow(10.,minYLim) : minYLim;
   
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS, True);
}
void ScatResetZoom(Widget w)
{
    ScatWidget sw = (ScatWidget)w;
    double minX, minY, maxX, maxY;

    calcDataRange(sw, &minX, &maxX, &minY, &maxY);
    sw->scat.minXLim = sw->scat.minXData = minX;
    sw->scat.minYLim = sw->scat.minYData = minY;
    sw->scat.maxXLim = sw->scat.maxXData = maxX;
    sw->scat.maxYLim = sw->scat.maxYData = maxY;

    if (XtIsRealized(w))
	redisplayContents(sw, X_SCREEN,
  		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS, True);
}

/*
** ScatPrintContents
**
** Prints the contents Scat widget to an Encapsulated PostScript file.
**
** Parameters
**
**	w		A scat widget
**	psFileName	Name for the PostScript file that will be created
*/
void ScatPrintContents(Widget w, char *psFileName)
{
    FILE *ps;
    ScatWidget sw = (ScatWidget)w;

    ps = OpenPS(psFileName, sw->core.width, sw->core.height);
    if (ps != NULL) {
	redisplayContents(sw, PS_PRINTER, REDRAW_ALL, False);
	EndPS();
    }    
}

/*
** ScatWritePS
**
** Writes out PostScript drawing commands to draw the contents of the widget
** to an open file.  The PostScript code written does not stand alone.  It
** depends on the preamble definitions and scaling normally provided by
** OpenPS in the utilities directory.  To get the bounding rectangle, use
** XGetGeometry on the XtWindow of the widget to get the width and height,
** the plot is drawn between 0,0 and the width and height of the widget.
**
** Parameters
**
**	w	A scat widget
**	fp	File pointer for an open file in which to
**		write the drawing commands
*/
void ScatWritePS(Widget w, FILE *fp)
{
    FILE *oldFP;
    
    oldFP = PSGetFile();
    PSSetFile(fp);
    redisplayContents((ScatWidget)w, PS_PRINTER, REDRAW_ALL, False);
    PSSetFile(oldFP);   
}

/*
** Redisplays the contents part of the widget, without the motif shadows and
** highlights.
*/
static void redisplayContents(ScatWidget w, int outDevice, int redrawArea,
			      int thicken)
{
    Display *display = XtDisplay(w);
    GC gc = w->scat.gc;
    XFontStruct *fs = getFontStruct(w->scat.font);
    Drawable drawBuf;
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if ((outDevice==X_SCREEN && !w->core.visible) || redrawArea == REDRAW_NONE)
        return;

    /* Set destination for drawing commands, offscreen pixmap or window */
    if (w->scat.doubleBuffer)
    	drawBuf = w->scat.drawBuffer;
    else
    	drawBuf = XtWindow(w);

    /* Clear the drawing buffer or window only in the areas that have
       changed.  The other parts are still redrawn, but the net effect
       is that the unchanged areas do not flicker */
    if (outDevice == X_SCREEN) {
	XSetForeground(display, gc, w->core.background_pixel);
	if (redrawArea == REDRAW_ALL || w->scat.doubleBuffer) {
	    XFillRectangle(display, drawBuf, gc, 0, 0,
			   w->core.width, w->core.height);
	} else {
    	    if (redrawArea & REDRAW_V_AXIS)
		XFillRectangle(display, drawBuf, gc, w->scat.axisLeft,
	   		w->scat.axisTop, w->scat.xOrigin - w->scat.axisLeft,
    			w->scat.axisBottom - w->scat.axisTop);
    	    if (redrawArea & REDRAW_H_AXIS)
    		XFillRectangle(display, drawBuf, gc, w->scat.axisLeft,
    	    		w->scat.yOrigin + 1, w->scat.axisRight-w->scat.axisLeft,
    	    		w->scat.axisBottom - w->scat.yOrigin + 1);
    	    if (redrawArea & REDRAW_CONTENTS && w->scat.plotImage == NULL)
    		XFillRectangle(display, drawBuf, gc, w->scat.xOrigin + 1,
    	   		w->scat.yEnd, w->scat.xEnd - w->scat.xOrigin,
    	   		w->scat.yOrigin - w->scat.yEnd);
	}
    }
    
    /* Draw the axes */
    XSetForeground(display, gc, w->primitive.foreground);
    if (w->scat.nPoints == 0) {
        /* empty of data, just draw axis lines */
    	XSegment segs[2];
    	segs[0].x1 = segs[0].x2 = segs[1].x1 = w->scat.xOrigin;
    	segs[0].y1 = segs[1].y1 = segs[1].y2 = w->scat.yOrigin;
    	segs[1].x2 = w->scat.xEnd; segs[0].y2 = w->scat.yEnd;
	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, segs, 2);
	else /* PS_PRINTER */
    	    PSDrawSegments(display, drawBuf, gc, segs, 2);
    } else {
	DrawHorizontalAxis(display, drawBuf, gc, fs, outDevice,
    	    w->scat.yOrigin, w->scat.xOrigin, w->scat.xEnd, w->scat.minXData,
    	    w->scat.maxXData, w->scat.minXLim, w->scat.maxXLim,
    	    w->scat.xLogScaling, 0, w->scat.psFont, w->scat.psFontSize);
	DrawVerticalAxis(display, drawBuf, gc, fs, outDevice,
    	    w->scat.xOrigin, w->scat.xMin, w->scat.yEnd,
    	    w->scat.yOrigin, w->scat.minYData,  w->scat.maxYData,
    	    w->scat.minYLim, w->scat.maxYLim, w->scat.yLogScaling,
            w->scat.psFont, w->scat.psFontSize);
    }

    /* Draw the axis labels */
    if (w->scat.xAxisLabel != NULL)
    {
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->scat.font, w->scat.xAxisLabel,
		gc, w->scat.xOrigin, w->scat.axisBottom + X_LABEL_MARGIN,
		w->scat.xEnd - w->scat.xOrigin, XmALIGNMENT_CENTER,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
	{
	    char *labelfont = NULL;
	    if (w->scat.psFont)
		XmStringGetLtoR(w->scat.psFont, XmSTRING_DEFAULT_CHARSET, &labelfont);
    	    PSDrawXmString(display, drawBuf, w->scat.font, w->scat.xAxisLabel,
		gc, w->scat.xOrigin, w->scat.axisBottom + X_LABEL_MARGIN,
		w->scat.xEnd - w->scat.xOrigin, XmALIGNMENT_CENTER, labelfont,
		w->scat.psFontSize);
	    if (labelfont)
		XtFree(labelfont);
	}
    }
    if (w->scat.yAxisLabel != NULL)
    {
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->scat.font, w->scat.yAxisLabel, gc,
    		w->scat.xOrigin + Y_LABEL_MARGIN, w->scat.yMin + TOP_MARGIN,
		w->scat.xEnd - w->scat.xOrigin, XmALIGNMENT_BEGINNING,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
	{
	    char *labelfont = NULL;
	    if (w->scat.psFont)
		XmStringGetLtoR(w->scat.psFont, XmSTRING_DEFAULT_CHARSET, &labelfont);
    	    PSDrawXmString(display, drawBuf, w->scat.font, w->scat.yAxisLabel,
    		gc, w->scat.xOrigin + Y_LABEL_MARGIN, w->scat.yMin + TOP_MARGIN,
		w->scat.xEnd - w->scat.xOrigin, XmALIGNMENT_BEGINNING, labelfont,
                w->scat.psFontSize);
	    if (labelfont)
		XtFree(labelfont);
	}
    }

    /* Draw the contents of the plot.  If point thickening is requested
       (makes points more visible but draws slower) the two different
       drawing methods do it differently.  The sparse method sends the
       points a second time offset by one.  The dense method generates a
       bitmap with thickened points and thus only needs to be called once */
    /* Modified by igv 09/28/2002. Forget the difference, let sparse method
       figure it out. */
    if (thicken && w->scat.darkerPoints) {
/*      	if (w->scat.plotImage != NULL) { */
	drawScatterPlot(w, drawBuf, outDevice, True);
/*      	} else { */
/*      	    drawScatterPlot(w, drawBuf, outDevice, False); */
/*      	    drawScatterPlot(w, drawBuf, outDevice, True); */
/*      	} */
    } else {
    	drawScatterPlot(w, drawBuf, outDevice, False);
    }
    
    /* After drawing using the dense method, the part of the arrow heads
       that protrudes into the plot area needs to be redrawn */
    if (w->scat.plotImage != NULL) {
	RedrawHAxisArrows(display, drawBuf, gc, w->scat.yOrigin,
		w->scat.xOrigin, w->scat.xEnd, w->scat.minXData,
    	        w->scat.maxXData, w->scat.minXLim, w->scat.maxXLim);
	RedrawVAxisArrows(display, drawBuf, gc, w->scat.xOrigin,
		w->scat.yEnd, w->scat.yOrigin, w->scat.minYData,
    	    	w->scat.maxYData, w->scat.minYLim, w->scat.maxYLim);
    }  

    /* Call the redisplay callback so an application which draws on the scat
       widget can refresh it's graphics */
    if (XtIsRealized((Widget)w) || outDevice != X_SCREEN)
    {
	XYCallbackStruct cbStruct;
	XYTransform xform;
	computeTransform(w, &xform);
	cbStruct.reason = XmCR_EXPOSE;
	cbStruct.event = NULL;
        cbStruct.outDevice = outDevice;
	cbStruct.drawBuf = drawBuf;
	cbStruct.xform = &xform;
	cbStruct.wingc = w->scat.gc;
	cbStruct.plotgc = w->scat.contentsGC;
    	XtCallCallbacks((Widget)w, XmNredisplayCallback, (XtPointer)&cbStruct);
    }

    /* For double buffering, now copy offscreen pixmap to screen */
    if (w->scat.doubleBuffer && outDevice == X_SCREEN)
    	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
    		  w->core.width, w->core.height, 0, 0);    
}

/*
** Draw the scatter plot itself using the list of points stored in the
** widget.  The argument "thicken" when true, causes the routine to draw
** the points offset by one pixel in the positive x direction.  This is
** used to make points more visible by drawing them as two pixels instead
** of one.
**
** Modified by igv 09/28/2002 -- use 4 more pixels instead of 1 to thicken
** when drawing sparse plots.
*/
static void drawScatterPlot(ScatWidget w, Drawable drawBuf, int outDevice,
			    int thicken)
{
    /* don't bother if there's no data */
    if (w->scat.nPoints == 0)
    	return;

    /* choose the drawing method */
    if (w->scat.plotImage == NULL || outDevice != X_SCREEN)
    	drawSparseScatterPlot(w, drawBuf, outDevice, thicken);
    else
    	drawDenseScatterPlot(w, drawBuf, thicken);
}

static void drawSparseScatterPlot(ScatWidget w, Drawable drawBuf,
				  int outDevice, int thicken)
{
    int nPts = 0;
    Display *display = XtDisplay(w);
    GC gc = w->scat.contentsGC;
    Pixel lastPixel;
    int xMin = w->scat.xOrigin, yMin = w->scat.yEnd;
    int xMax = w->scat.xEnd, yMax = w->scat.yOrigin;
    double minXData, minYData, minXLim, minYLim, maxXLim, maxYLim;
    double xScale, yScale, minXPix, maxYPix;
    XPoint *pt, *pts, *startPt;
    ScatPoint *point;
    int x, y, i;

    /* if log scaling was requested, express limits in log coordinates */
    if (w->scat.xLogScaling) {
    	minXData = log10(w->scat.minXData);
    	minXLim = log10(w->scat.minXLim); maxXLim = log10(w->scat.maxXLim);
    } else {
    	minXData = w->scat.minXData;
    	minXLim = w->scat.minXLim; maxXLim = w->scat.maxXLim;
    }
    if (w->scat.yLogScaling) {
    	minYData = log10(w->scat.minYData);
    	minYLim = log10(w->scat.minYLim); maxYLim = log10(w->scat.maxYLim);
    } else {
    	minYData = w->scat.minYData;
    	minYLim = w->scat.minYLim; maxYLim = w->scat.maxYLim;
    }
    xScale = (w->scat.xEnd - w->scat.xOrigin) / (maxXLim - minXLim);
    yScale = (w->scat.yOrigin - w->scat.yEnd) / (maxYLim - minYLim);
    minXPix = w->scat.xOrigin - (minXLim - minXData) * xScale;
    maxYPix = w->scat.yOrigin + (minYLim - minYData) * yScale;
    
    /* allocate memory for an array of XPoint structures for drawing points */
    if (thicken) i = 5;
    else i = 1;
    pts = (XPoint *)XtMalloc(sizeof(XPoint)*i*(w->scat.nPoints));
    
    /* loop through all of the data converting the data coordinates to
       X coordinates and drawing.  Accumulate runs of the same color
       (the points were sorted by color in ScatSetContents) in the array
       pts, and draw whenever the color changes. */
    if (outDevice == PS_PRINTER)
    {
	fprintf(PSGetFile(), "gsave\n");
	PSSetClipRectangle(w->scat.xOrigin+1, w->scat.yEnd,
			   w->scat.xEnd, w->scat.yOrigin-1);
    }    
    startPt = pt = pts;
    lastPixel = w->scat.points[0].pixel;
    for (i=0, point=w->scat.points; i<w->scat.nPoints; i++, point++) {
    	if (point->pixel != lastPixel) {
    	    /* draw the points in the current color, then switch */
    	    if (nPts != 0) {
		XSetForeground(display, gc, lastPixel);
    	    	if (outDevice == X_SCREEN)
    		    XDrawPoints(display, drawBuf, gc, startPt, nPts, 0);
    		else
    		    PSDrawPoints(display, drawBuf, gc, startPt, nPts, 0);
    	    }
    	    lastPixel = point->pixel;
	    startPt = pt;
	    nPts = 0;
    	}
    	if (w->scat.xLogScaling)
    	    x = (int)(minXPix + (log10(point->x) - minXData) * xScale);
    	else
            x = (int)(minXPix + (point->x - minXData) * xScale);
    	if (w->scat.yLogScaling)
            y = (int)(maxYPix - (log10(point->y) - minYData) * yScale);
    	else
            y = (int)(maxYPix - (point->y - minYData) * yScale);
        if (x > xMin && x <= xMax && y > yMin && y <= yMax) {
            pt->x = x;
            pt->y = y;
            pt++; nPts++;
	}
	if (thicken)
	{
	    x += 1;
	    if (x > xMin && x <= xMax && y > yMin && y <= yMax) {
		pt->x = x;
		pt->y = y;
		pt++; nPts++;
	    }
	    x -= 2;
	    if (x > xMin && x <= xMax && y > yMin && y <= yMax) {
		pt->x = x;
		pt->y = y;
		pt++; nPts++;
	    }
	    x += 1; y += 1;
	    if (x > xMin && x <= xMax && y > yMin && y <= yMax) {
		pt->x = x;
		pt->y = y;
		pt++; nPts++;
	    }
	    y -= 2;
	    if (x > xMin && x <= xMax && y > yMin && y <= yMax) {
		pt->x = x;
		pt->y = y;
		pt++; nPts++;
	    }
	}
    }
    if (nPts != 0) {
    	XSetForeground(display, gc, lastPixel);
    	if (outDevice == X_SCREEN)
    	    XDrawPoints(display, drawBuf, gc, startPt, nPts, 0);
    	else
    	    PSDrawPoints(display, drawBuf, gc, startPt, nPts, 0);
    }
    XtFree((char *)pts);
    if (outDevice == PS_PRINTER)
	fprintf(PSGetFile(), "grestore\n");
}

static void drawDenseScatterPlot(ScatWidget w, Drawable drawBuf, int thicken)
{
    const int width = w->scat.xEnd - w->scat.xOrigin > 0 ?
        w->scat.xEnd - w->scat.xOrigin : 0;
    const int height = w->scat.yOrigin - w->scat.yEnd > 0 ?
        w->scat.yOrigin - w->scat.yEnd : 0;
    int xLogScaling = w->scat.xLogScaling, yLogScaling = w->scat.yLogScaling;
    double minXLim, minYLim, maxXLim, maxYLim;
    double xScale, yScale;
    int xMin = 0, xMax = width;
    int yMin = 0, yMax = height;
    ScatPoint *point;
    int x, y, i;
    unsigned char *bits = w->scat.plotBits;
    int bytesPerLine = w->scat.plotImage->bytes_per_line;

    /* if log scaling was requested, express limits in log coordinates */
    if (xLogScaling) {
    	minXLim = log10(w->scat.minXLim); maxXLim = log10(w->scat.maxXLim);
    } else {
    	minXLim = w->scat.minXLim; maxXLim = w->scat.maxXLim;
    }
    if (yLogScaling) {
    	minYLim = log10(w->scat.minYLim); maxYLim = log10(w->scat.maxYLim);
    } else {
    	minYLim = w->scat.minYLim; maxYLim = w->scat.maxYLim;
    }
    xScale = width / (maxXLim - minXLim);
    yScale = height / (maxYLim - minYLim);

    /* clear the bitmap */
    memset(bits, 0, bytesPerLine * height);

    /* loop through all of the data converting the data coordinates to
       bitmap coordinates and ORing the points into the bitmap.
       igv 09/28/2002 -- add more thickening.
    */
    for (i=0, point=w->scat.points; i<w->scat.nPoints; i++, point++) {
        if (xLogScaling)
            x = (int)((log10(point->x) - minXLim) * xScale);
        else
            x = (int)((point->x - minXLim) * xScale);
        if (yLogScaling)
            y = (int)((maxYLim - log10(point->y)) * yScale);
        else
            y = (int)((maxYLim - point->y) * yScale);
        if (x > xMin && x < xMax && y > yMin && y < yMax)
            bits[y*bytesPerLine + x/8] |= 0x80 >> x%8;
	if (thicken) {
	    x += 1;
	    if (x > xMin && x < xMax && y > yMin && y < yMax)
		bits[y*bytesPerLine + x/8] |= 0x80 >> x%8;
	    x -= 2;
	    if (x > xMin && x < xMax && y > yMin && y < yMax)
		bits[y*bytesPerLine + x/8] |= 0x80 >> x%8;
	    x += 1; y += 1;
	    if (x > xMin && x < xMax && y > yMin && y < yMax)
		bits[y*bytesPerLine + x/8] |= 0x80 >> x%8;
	    y -= 2;
	    if (x > xMin && x < xMax && y > yMin && y < yMax)
		bits[y*bytesPerLine + x/8] |= 0x80 >> x%8;
	}
    }

    /* draw the bitmap image to the screen */
    XPutImage(XtDisplay(w), drawBuf, w->scat.contentsGC, w->scat.plotImage,
    	      1, 0, w->scat.xOrigin+1, w->scat.yEnd, width-1, height);
}

/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(ScatWidget w)
{ 
    if (w->scat.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->scat.drawBuffer);
    if (w->scat.doubleBuffer) {
    	w->scat.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
    } else {
    	w->scat.drawBuffer = 0;
    }
}

/*
** When the number of points becomes large, the widget shifts from drawing
** points using XDrawPoints, to writing a bitmap locally and sending that.
** This routine updates the allocation of the image bitmap when the 
** window is resized or shifts between drawing modes.
*/
static void updatePlotBitmapAllocation(ScatWidget w)
{ 
    unsigned char *bits;
    XImage *image;
    int width = w->scat.xEnd - w->scat.xOrigin;
    int height = w->scat.yOrigin-w->scat.yEnd;
    
    if (w->scat.plotImage != NULL) {
    	XtFree((char *)w->scat.plotImage);
    	XtFree((char *)w->scat.plotBits);
    }
   if (USE_BITMAP(width, height, w->scat.nPoints) && !w->scat.isColor) {
	image = (XImage *)XtCalloc(1, sizeof(XImage));
	image->bytes_per_line = (width + 7) >> 3;
	bits = (unsigned char *)XtMalloc(image->bytes_per_line * height);
	image->width = width;
	image->height = height;
	image->data = (char *)bits;
	image->depth = 1;
	image->xoffset = 0;
	image->format = XYBitmap;
	image->byte_order = LSBFirst;
	image->bitmap_unit = 8;
	image->bitmap_bit_order = MSBFirst;
	image->bitmap_pad = 8;
	w->scat.plotImage = image;
	w->scat.plotBits = bits;
    } else {
    	w->scat.plotImage = NULL;
    	w->scat.plotBits = NULL;
    }
}

/*
** Get the XFontStruct that corresponds to the default (first) font in
** a Motif font list.  Since Motif stores this, it saves us from storing
** it or querying it from the X server.
*/
static XFontStruct *getFontStruct(XmFontList font)
{
#ifdef MOTIF10
    return font->font;
#else
    XFontStruct *fs;
    XmFontContext context;
    XmStringCharSet charset;

    XmFontListInitFontContext(&context, font);
    XmFontListGetNextFont(context, &charset, &fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
    return fs;
#endif
}

/* compare procedure for qsort for sorting points by color */
static int comparePoints(const void *point1, const void *point2)
{
    ScatPoint *pt1 = (ScatPoint *)point1, *pt2 = (ScatPoint *)point2;
    
    if (pt1->pixel < pt2->pixel)
    	return -1;
    else if (pt1->pixel == pt2->pixel)
    	return 0;
    else
    	return 1;
}

/*
** Checks that log scaling is possible with current data and current settings
** of xLogScaling and yLogScaling.  If it is not possible, this routine
** resets x or yLogScaling to False and takes no further action.  If the
** caller wants to take further action, it should test these flags.
*/
static void verifyDataForLogScaling(ScatWidget w)
{
    if (w->scat.xLogScaling && w->scat.minXData <= 0.)
	w->scat.xLogScaling = False;
    if (w->scat.yLogScaling && w->scat.minYData <= 0.)
	w->scat.yLogScaling = False;
}

void calcDataRange(ScatWidget w, double *xMin, double *xMax, double *yMin,
	double *yMax)
{
    ScatPoint *point;
    double minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    int i;

    for (i=0, point=w->scat.points; i<w->scat.nPoints; i++, point++) {
    	if (point->x > maxX) maxX = point->x;
    	if (point->y > maxY) maxY = point->y;
    	if (point->x < minX) minX = point->x;
    	if (point->y < minY) minY = point->y;
    }
    *xMin = minX; *yMin = minY; *xMax = maxX; *yMax = maxY;
}    

/* minimum and maximum of two doubles */
static double dMin(double d1, double d2)
{
    if (d2 < d1)
    	return d2;
    return d1;
}
static double dMax(double d1, double d2)
{
    if (d2 > d1)
    	return d2;
    return d1;
}

static void computeTransform(ScatWidget w, XYTransform *xform)
{
    double maxXLim, maxYLim;

    /* If log scaling was requested, express limits in log coordinates */
    if (w->scat.xLogScaling) {
    	xform->minXData = w->scat.minXData > 0. ? log10(w->scat.minXData): 0.;
    	xform->minXLim = w->scat.minXLim > 0. ? log10(w->scat.minXLim) : 0.;
    	maxXLim = w->scat.maxXLim > 0. ? log10(w->scat.maxXLim) : 0.;
    } else {
    	xform->minXData = w->scat.minXData;
    	xform->minXLim = w->scat.minXLim;
    	maxXLim = w->scat.maxXLim;
    }
    if (w->scat.yLogScaling) {
    	xform->minYData = w->scat.minYData > 0. ? log10(w->scat.minYData) : 0.;
    	xform->minYLim = w->scat.minYLim > 0. ? log10(w->scat.minYLim) : 0.;
    	maxYLim = w->scat.maxYLim > 0. ? log10(w->scat.maxYLim) : 0.;
    } else {
    	xform->minYData = w->scat.minYData;
    	xform->minYLim = w->scat.minYLim;
    	maxYLim = w->scat.maxYLim;
    }
    xform->xScale = (w->scat.xEnd - w->scat.xOrigin) /
    	    (maxXLim - xform->minXLim);
    xform->yScale = (w->scat.yOrigin - w->scat.yEnd) /
    	    (maxYLim - xform->minYLim);
    xform->minXPix = w->scat.xOrigin -
    	    (xform->minXLim - xform->minXData) * xform->xScale;
    xform->maxYPix = w->scat.yOrigin +
    	    (xform->minYLim - xform->minYData) * xform->yScale;
    xform->xLogScaling = w->scat.xLogScaling;
    xform->yLogScaling = w->scat.yLogScaling;
    xform->xOrigin = w->scat.xOrigin;
    xform->yOrigin = w->scat.yOrigin;
    xform->xEnd = w->scat.xEnd;
    xform->yEnd = w->scat.yEnd;
}
