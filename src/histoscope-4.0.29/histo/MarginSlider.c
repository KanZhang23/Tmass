/*******************************************************************************
*									       *
* MarginSlider.c -- Margin Slider Widget				       *
*									       *
* Copyright (c) 1994 Universities Research Association, Inc.		       *
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
* September 15, 1994							       *
*									       *
* From Margin.c, written by Joy Kyriakopulos				       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion >= 1002
#include <Xm/PrimitiveP.h>
#endif
#include "MarginSliderP.h"

#include "../plot_widgets/obsolete.h"

#define BAR_WIDTH 10
#define LINE_WIDTH 2

#define REDRAW_NONE 0
#define REDRAW_CONTENTS 1
#define REDRAW_ALL 2

#define LEFT_MARGIN 3		/* empty space to left of widget */
#define TOP_MARGIN 3		/* empty space at top of widget */
#define RIGHT_MARGIN 3		/* empty space to right of widget */
#define BOTTOM_MARGIN 3		/* empty space at bottom of widget */

#define TRUE 1
#define FALSE 0

enum dragStates {NOT_DRAGGING, DRAGGING_NOTHING, DRAGGING_LEFT_BAR, 
		 DRAGGING_RIGHT_BAR, DRAGGED_LEFT_BAR, DRAGGED_RIGHT_BAR};

static void motionAP(MarginSliderWidget w, XEvent *event, char *args,
	int n_args);
static void btnUpAP(MarginSliderWidget w, XEvent *event, char *args,
	int n_args);
static void initialize(MarginSliderWidget request, MarginSliderWidget new);
static void redisplay(MarginSliderWidget w, XEvent *event, Region region);
static void redisplayContents(MarginSliderWidget w);
static void drawMarginHairs(MarginSliderWidget w, Drawable drawBuf);
static void destroy(MarginSliderWidget w);
static void resize(MarginSliderWidget w);
static Boolean setValues(MarginSliderWidget current,
	MarginSliderWidget request, MarginSliderWidget new);
static void setValidLeftBarStop(MarginSliderWidget new);
static void setValidLeftBar(MarginSliderWidget new);
static void setValidRightBarStop(MarginSliderWidget new);
static void setValidRightBar(MarginSliderWidget new);
#ifdef DEBUG
static void printWidgetValues(MarginSliderWidget new);
#endif
static void resetBarDragging(int *dragState);
static int dragBar(XEvent *event, int leftBar, int rightBar, int boxTop,
		   int boxBottom, int boxLeft, int boxRight, int leftBarStop,
		   int rightBarStop, int *dragState, int *xDragStart, 
		   int *xDragEnd, int *dragDiff);


static char defaultTranslations[] = 
    "<Btn1Motion>: Motion()\n\
     <Btn1Down>: Motion()\n\
     <Btn1Up>: BtnUp()\n";

static XtActionsRec actionsList[] = {
    {"Motion", (XtActionProc)motionAP},
    {"BtnUp", (XtActionProc)btnUpAP}
};

static XtResource resources[] = {
    {XmNleftBar, XmCLeftBar, XmRInt, sizeof (int),
      XtOffset (MarginSliderWidget, margin.leftBar), XmRString, "0"   },
    {XmNrightBar, XmCRightBar, XmRInt, sizeof (int),
      XtOffset (MarginSliderWidget, margin.rightBar), XmRString, "0" },
    {XmNleftBarStop, XmCLeftBarStop, XmRInt, sizeof (int),
      XtOffset (MarginSliderWidget, margin.leftBarStop), XmRString, "0"   },
    {XmNrightBarStop, XmCRightBarStop, XmRInt, sizeof (int),
      XtOffset (MarginSliderWidget, margin.rightBarStop), XmRString, "0" },
    {XmNleftBarStopMin, XmCLeftBarStopMin, XmRInt, sizeof (int),
      XtOffset (MarginSliderWidget, margin.leftBarStopMin), XmRString, "0"   },
    {XmNrightBarStopMax, XmCRightBarStopMax, XmRInt, sizeof (int),
      XtOffset (MarginSliderWidget, margin.rightBarStopMax), XmRString, "0" },
    {XmNmarginChangedCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (MarginSliderWidget, margin.marginChanged), XmRCallback, NULL},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (MarginSliderWidget, margin.resize), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (MarginSliderWidget, margin.redisplay), XtRCallback, NULL},
    {XmNmarginDragCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (MarginSliderWidget, margin.marginDrag), XtRCallback, NULL},
};

MarginSliderClassRec marginSliderClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass       */
    "Margin",                       /* class_name            */
    sizeof(MarginSliderRec),        /* widget_size           */
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
/*
 * This #if is necessary because XmArmAndActivate is not defined on Motif
 * V1.2 systems.  However, XmArmAndActivate only has two parameters whereas
 * XtActionProc has four parameters.  See X11/Intrinsic.h and Xm/XmP.h.
 */
#if XmVersion == 1001
     (XmArmAndActivate)motionAP,    	/* arm_and_activate             */
#else
     (XtActionProc)motionAP,    	/* arm_and_activate             */
#endif
     NULL,				/* get resources      		*/
     0,					/* num get_resources  		*/
     NULL,         			/* extension                    */
  },
  /* Margin class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass marginSliderWidgetClass = (WidgetClass)&marginSliderClassRec;

/*
** Widget initialize method
*/
static void initialize(MarginSliderWidget request, MarginSliderWidget new)
{
    XGCValues values;
    Display *display = XtDisplay(new);
    int borderWidth =
    	new->primitive.shadow_thickness + new->primitive.highlight_thickness;
    XRectangle clipRect;

#ifdef DEBUG   
    printf("\nin initialize...\n");
#endif /* DEBUG */

    /* Make sure the window size is not zero. The Core 
       initialize() method doesn't do this. */
    if (request->core.width == 0)
    	new->core.width = 300;
    if (request->core.height == 0)
   	new->core.height = 25;
    if (request->core.width < 2*BAR_WIDTH+4*LINE_WIDTH)
    	new->core.width = 2*BAR_WIDTH+4*LINE_WIDTH;

    /* Create graphics contexts for drawing in the widget */
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    values.line_width = LINE_WIDTH;
    new->margin.gc = XCreateGC(display, XDefaultRootWindow(display),
    			     GCForeground|GCBackground|GCLineWidth, &values);
    			     
#ifdef DEBUG   
    printf(" request widget:\n");
    printWidgetValues(request);
    printf(" new widget:\n");
    printWidgetValues(new);
#endif /* DEBUG */
    
    /* Initialize various fields */
    resetBarDragging(&new->margin.dragState);
    
    /* Set size dependent items */
    /* calculate the area of the widget where contents can be drawn */
    new->margin.xMin = borderWidth;
    new->margin.yMin = borderWidth;
    new->margin.xMax = new->core.width - borderWidth;
    new->margin.yMax = new->core.height - borderWidth;

    new->margin.boxTop = new->margin.yMax - LINE_WIDTH;
    new->margin.boxBottom = new->margin.yMin + LINE_WIDTH;

    new->margin.boxLeft = new->margin.xMin  + BAR_WIDTH + LINE_WIDTH;
    new->margin.boxRight = new->margin.xMax - BAR_WIDTH - LINE_WIDTH;
    new->margin.leftBarStopMin = new->margin.boxLeft;
    new->margin.rightBarStopMax = new->margin.boxRight;
    
    /* set the values for bars and bar stops */
    setValidLeftBarStop(new);
    setValidLeftBar(new);
    setValidRightBarStop(new);
    setValidRightBar(new);

#ifdef DEBUG   
    printf(" new widget changed to:\n");
    printWidgetValues(new);
#endif /* DEBUG */
    
    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = new->core.width - 2 * borderWidth;
    clipRect.height = new->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(new), new->margin.gc, 0,0, &clipRect, 1, 
    		Unsorted);
}

/*
** Widget destroy method
*/
static void destroy(MarginSliderWidget w)
{
#ifdef DEBUG   
    printf("in destroy...\n");
#endif /* DEBUG */

    XFreeGC(XtDisplay(w), w->margin.gc);
    XtRemoveAllCallbacks ((Widget)w, XmNresizeCallback);
    XtRemoveAllCallbacks ((Widget)w, XmNredisplayCallback);
    XtRemoveAllCallbacks ((Widget)w, XmNmarginChangedCallback);
    XtRemoveAllCallbacks ((Widget)w, XmNmarginDragCallback);
}

/*
** Widget resize method
*/
static void resize(MarginSliderWidget w)
{
    int dif;
    int prevRightBarStopMax = w->margin.rightBarStopMax;
    int prevRightBarStop = w->margin.rightBarStop;
    int prevRightBar = w->margin.rightBar;
    int borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;

#ifdef DEBUG   
    printf("in resize...\n");
#endif /* DEBUG */

    /* calculate the area of the widget where contents can be drawn */
    w->margin.xMin = borderWidth;
    w->margin.yMin = borderWidth;
    w->margin.xMax = w->core.width - borderWidth;
    w->margin.yMax = w->core.height - borderWidth;
    w->margin.boxLeft = w->margin.xMin  + BAR_WIDTH + LINE_WIDTH;
    w->margin.boxTop = w->margin.yMax - LINE_WIDTH;
    w->margin.boxRight = w->margin.xMax - BAR_WIDTH - LINE_WIDTH;
    w->margin.boxBottom = w->margin.yMin + LINE_WIDTH;
    w->margin.leftBarStopMin = w->margin.boxLeft;
    w->margin.rightBarStopMax = w->margin.boxRight;
    
    /* reset the min and max values for bar stops */
    if (w->margin.leftBarStop < w->margin.leftBarStopMin)
    	w->margin.leftBarStop = w->margin.leftBarStopMin;
    if (w->margin.leftBar < w->margin.leftBarStop)
    	w->margin.leftBar = w->margin.leftBarStop;

    /* as convenience to caller, reset the right bar stop and right bar so
       that the distance from right is maintained without passing left bar */
    dif = prevRightBarStopMax - prevRightBarStop;
    w->margin.rightBarStop = w->margin.rightBarStopMax - dif;
    if (w->margin.rightBarStop > w->margin.rightBarStopMax)
    	w->margin.rightBarStop = w->margin.rightBarStopMax;
    if (w->margin.rightBarStop < w->margin.leftBar)
    	w->margin.rightBarStop = w->margin.leftBar;

    dif = prevRightBarStop - prevRightBar;
    w->margin.rightBar = w->margin.rightBarStop - dif;
    if (w->margin.rightBar > w->margin.rightBarStop)
    	w->margin.rightBar = w->margin.rightBarStop;
    if (w->margin.rightBar < w->margin.leftBar)
    	w->margin.rightBar = w->margin.leftBar;

#ifdef DEBUG   
    printf("  xMin = %d, xmax = %d, yMin = %d, ymax = %d,\n", w->margin.xMin, 
    	    w->margin.xMax, w->margin.yMin, w->margin.yMax);
    printf("  boxLeft = %d, boxRight = %d, boxTop = %d, boxBottom = %d\n",  
    	    w->margin.boxLeft, w->margin.boxRight, w->margin.boxTop,
    	    w->margin.boxBottom);
    printf("  leftBarStopMin = %d, rightBarStopMax = %d\n", 
    	    w->margin.leftBarStopMin, w->margin.rightBarStopMax);
    printf("  leftBarStop = %d, rightBarStop = %d\n", w->margin.leftBarStop, 
    	    w->margin.rightBarStop);
    printf("  leftBar = %d, rightBar = %d\n", w->margin.leftBar, 
    	    w->margin.rightBar);
#endif /* DEBUG */
    
    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), w->margin.gc, 0,0, &clipRect, 1, Unsorted);
    
    /* call the resize callback */
    if (XtIsRealized((Widget)w))
    	XtCallCallbacks((Widget)w, XmNresizeCallback, NULL);
}

/*
** Widget redisplay method
*/
static void redisplay(MarginSliderWidget w, XEvent *event, Region region)
{
#ifdef DEBUG   
    printf("in redisplay...\n");
#endif /* DEBUG */

    /* Draw the Motif required shadows and highlights */
    if (w->primitive.shadow_thickness > 0) {
	_XmDrawShadow (XtDisplay(w), XtWindow(w), 
		       w->primitive.bottom_shadow_GC,
		       w->primitive.top_shadow_GC,
                       w->primitive.shadow_thickness,
                       w->primitive.highlight_thickness,
                       w->primitive.highlight_thickness,
                       w->core.width - 2 * w->primitive.highlight_thickness,
                       w->core.height- 2 * w->primitive.highlight_thickness);
    }
    if (w->primitive.highlighted)
	_XmHighlightBorder((Widget)w);
    else if (_XmDifferentBackground((Widget)w, XtParent((Widget)w)))
	_XmUnhighlightBorder((Widget)w);
    
    /* Now draw the contents of the margin widget */
    redisplayContents(w);
}

/*
** Widget setValues method
*/
static Boolean setValues(MarginSliderWidget current,
	MarginSliderWidget request, MarginSliderWidget new)
{
    Boolean redraw = False;
    Display *display = XtDisplay(new);

#ifdef DEBUG   
    printf("in setValues...\n");
#endif /* DEBUG */

    /* If the foreground or background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetForeground(display, new->margin.gc, new->primitive.foreground);
    	redraw = TRUE;
    }
    if (new->primitive.foreground != current->primitive.foreground) {
    	XSetBackground(display, new->margin.gc, new->core.background_pixel);
    	redraw = TRUE;
    }
    
    /* if highlight thickness or shadow thickness changed, resize and redraw */
    if  ((new->primitive.highlight_thickness != 
          current->primitive.highlight_thickness) ||
         (new -> primitive.shadow_thickness !=
          current->primitive.shadow_thickness)) {
    	redraw = TRUE;
    }
    
#ifdef DEBUG   
    printf(" new widget values:\n");
    printWidgetValues(new);
    printf(" current widget values:\n");
    printWidgetValues(current);
    printf(" request widget values:\n");
    printWidgetValues(request);
#endif /* DEBUG */

    /* Do not allow leftBarStopMin or rightBarStopMax to be changed */
    if (new->margin.rightBarStopMax != current->margin.rightBarStopMax)
    	new->margin.rightBarStopMax = current->margin.rightBarStopMax;
    if (new->margin.leftBarStopMin != current->margin.leftBarStopMin)
    	new->margin.leftBarStopMin = current->margin.leftBarStopMin;

    /* If a margin bar or margin bar stop has been changed, make sure the
       new values are OK and change any related values too */
    /* leftBarStop */
    if (new->margin.leftBarStop != current->margin.leftBarStop)
	setValidLeftBarStop(new);
    if (new->margin.leftBar != current->margin.leftBar) {
        setValidLeftBar(new);
    	redraw = TRUE;
    }
    if (new->margin.rightBarStop != current->margin.rightBarStop)
        setValidRightBarStop(new);
    if (new->margin.rightBar != current->margin.rightBar) {
        setValidRightBar(new);
    	redraw = TRUE;
    }
    
#ifdef DEBUG   
    printf(" new widget values changed to:\n");
    printWidgetValues(new);
#endif /* DEBUG */

    return redraw; 
} 

/*
** Button press and button motion action proc.
*/
/*
 * Note: do not use third & fourth arguments. (See note above for motionAP
 * used in Motif primitive class fields.  XmArmAndActivate only has two
 * parameters whereas XtActionProc has four parameters.  See X11/Intrinsic.h
 * and Xm/XmP.h.                                                                  
 */
static void motionAP(MarginSliderWidget w, XEvent *event, char *args, int n_args)
{
    int redraw;

#ifdef DEBUG   
    printf("in motionAP...\n");
#endif /* DEBUG */

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    
    redraw = dragBar(event, w->margin.leftBar, w->margin.rightBar, 
    	w->margin.boxTop, w->margin.boxBottom, w->margin.boxLeft, 
    	w->margin.boxRight, w->margin.leftBarStop, w->margin.rightBarStop, 
    	&w->margin.dragState, &w->margin.xDragStart, &w->margin.xDragEnd,
    	&w->margin.dragDiff);
    if (w->margin.dragState == DRAGGING_LEFT_BAR) {
#ifdef DEBUG   
	printf("    xDragStart = %d, xDragEnd = %d, prevLeftBar = %d\n",
			w->margin.xDragStart, w->margin.xDragEnd,
			w->margin.leftBar);   
#endif /* DEBUG */
	w->margin.leftBar += w->margin.xDragEnd - w->margin.xDragStart;
#ifdef DEBUG   
	printf("  leftBar changed to %d, calling dragCallback...\n",
		 w->margin.leftBar);
#endif /* DEBUG */
    }
    else if (w->margin.dragState == DRAGGING_RIGHT_BAR) {
#ifdef DEBUG   
	printf("    xDragStart = %d, xDragEnd = %d, prevRightBar = %d\n",
			w->margin.xDragStart, w->margin.xDragEnd,
			w->margin.rightBar);   
#endif /* DEBUG */
	w->margin.rightBar += w->margin.xDragEnd - w->margin.xDragStart;
#ifdef DEBUG   
	printf("  rightBar changed to %d, calling dragCallback...\n",
		 w->margin.rightBar);
#endif /* DEBUG */
    }
    if (redraw)	{
        redisplayContents(w);
        if (w->margin.xDragStart != w->margin.xDragEnd)
            XtCallCallbacks((Widget)w, XmNmarginDragCallback, NULL);
    }
}

/*
** Button up action proc.
*/
static void btnUpAP(MarginSliderWidget w, XEvent *event, char *args, int n_args)
{
    MarginSliderCallbackStruct cbStruct;
    int dragState = w->margin.dragState;

#ifdef DEBUG   
    printf("in btnUpAP...\n");
#endif /* DEBUG */
    
    resetBarDragging(&w->margin.dragState);
    redisplayContents(w);

    /* Call margin changed callback if user changed bar location */
    if (dragState == DRAGGING_RIGHT_BAR || dragState == DRAGGING_LEFT_BAR) {
       cbStruct.reason = XmCR_INPUT;
       cbStruct.event = event;
#ifdef DEBUG   
       printf("calling marginChangedCallback...\n");
#endif /* DEBUG */
       XtCallCallbacks((Widget)w, XmNmarginChangedCallback, (char *)&cbStruct);
    }
}

/*
** Redisplays the contents part of the widget, without the motif shadows and
** highlights.
*/
static void redisplayContents(MarginSliderWidget w)
{
    Display *display = XtDisplay(w);
    GC gc = w->margin.gc;
    Drawable drawBuf;
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if ((!w->core.visible))
        return;

#ifdef DEBUG   
    printf("in redisplayContents...\n");
#endif /* DEBUG */

    /* Set destination for drawing commands */
    drawBuf = XtWindow(w);

    /* Clear the drawing window */
    XSetForeground(display, gc, w->core.background_pixel);
    XFillRectangle(display, drawBuf, gc, w->margin.xMin, w->margin.yMin,
    	     w->margin.xMax - w->margin.xMin, 
    	     w->margin.yMax - w->margin.yMin);
    
    /* Draw the crosshairs */
    XSetForeground(display, gc, w->primitive.foreground);
    drawMarginHairs(w, drawBuf);
    
    /* Call the redisplay callback so an application which draws on the margin
       widget can refresh its graphics */
    if (XtIsRealized((Widget)w))
    	XtCallCallbacks((Widget)w, XmNredisplayCallback, NULL);
}

/*
** Draw the margin box itself using the margin hair locations stored in the
** widget.
*/
static void drawMarginHairs(MarginSliderWidget w, Drawable drawBuf)
{
    int nSegs = 0;
    XSegment *xSeg, *xSegs;
    Display *display = XtDisplay(w);
    GC gc = w->margin.gc;
        
#ifdef DEBUG   
    printf("in drawMarginHairs...\n");
#endif /* DEBUG */

    _XmDrawShadow (display, drawBuf, w->primitive.top_shadow_GC,
	    w->primitive.bottom_shadow_GC, 2,
	    w->margin.leftBar - BAR_WIDTH, w->margin.boxBottom,
	    BAR_WIDTH, w->margin.boxTop - w->margin.boxBottom);
    _XmDrawShadow (display, drawBuf, w->primitive.top_shadow_GC,
	    w->primitive.bottom_shadow_GC, 2,
	    w->margin.rightBar, w->margin.boxBottom,
	    BAR_WIDTH, w->margin.boxTop - w->margin.boxBottom);

    /* allocate memory for an array of XSegment structures for drawing lines */
    xSegs = (XSegment *)XtMalloc(sizeof(XSegment) * 9);
    if (xSegs == NULL) {
	fprintf(stderr, "Margin Can't Allocate Memory for Drawing\n");
	return;
    }    
    xSeg = xSegs;

    /* assign line segments based on location of cross-hairs */    
    xSeg->x1 = w->margin.leftBar - BAR_WIDTH + 3;
    xSeg->x2 = w->margin.leftBar - 3;
    xSeg->y1 = w->margin.boxTop-3;
    xSeg->y2 = (w->margin.boxTop - w->margin.boxBottom) / 2 
    		+ w->margin.boxBottom;
    xSeg++; nSegs++;
    
    xSeg->x1 = w->margin.leftBar - BAR_WIDTH + 3;
    xSeg->x2 = w->margin.leftBar - 3;
    xSeg->y1 = w->margin.boxBottom+3;
    xSeg->y2 = (w->margin.boxTop - w->margin.boxBottom) / 2 
    		+ w->margin.boxBottom;
    xSeg++; nSegs++;
    
    xSeg->x1 = w->margin.rightBar + BAR_WIDTH - 3;
    xSeg->x2 = w->margin.rightBar + 3;
    xSeg->y1 = w->margin.boxTop-3;
    xSeg->y2 = (w->margin.boxTop - w->margin.boxBottom) / 2 
    		+ w->margin.boxBottom;
    xSeg++; nSegs++;
    
    xSeg->x1 = w->margin.rightBar + BAR_WIDTH - 3;
    xSeg->x2 = w->margin.rightBar + 3;
    xSeg->y1 = w->margin.boxBottom+3;
    xSeg->y2 = (w->margin.boxTop - w->margin.boxBottom) / 2 
    		+ w->margin.boxBottom;
    xSeg++; nSegs++;

    XDrawSegments(display, drawBuf, gc, xSegs, nSegs);

    XtFree((char *)xSegs);
}

/*
 * Validate Bar and Bar Stop Values - These routines MUST be called in order:
 */
static void setValidLeftBarStop(MarginSliderWidget new)
{ 
    /* leftBarStop */
    if (new->margin.leftBarStop < new->margin.leftBarStopMin) {
    	if (new->margin.leftBarStop != 0)
    	    fprintf(stderr,"Value specified for XmNleftBarStop is too small\n");
    	new->margin.leftBarStop = new->margin.leftBarStopMin;
    }
    else if (new->margin.leftBarStop > new->margin.rightBarStopMax) {
    	    fprintf(stderr,"Value specified for XmNleftBarStop is too large\n");
    	    new->margin.leftBarStop = new->margin.rightBarStopMax;
    }
}

static void setValidLeftBar(MarginSliderWidget new)
{ 
    /* leftBar */
    if (new->margin.leftBar < new->margin.leftBarStop) {
    	if (new->margin.leftBar != 0)
    	    fprintf(stderr,"Value specified for XmNleftBar is too small\n");
    	new->margin.leftBar = new->margin.leftBarStop;
    }
    else if (new->margin.leftBar > new->margin.rightBarStopMax) {
    	fprintf(stderr,"Value specified for XmNleftBar is too large\n");
    	new->margin.leftBar = new->margin.rightBarStopMax;
    }
}

static void setValidRightBarStop(MarginSliderWidget new)
{
    /* rightBarStop */
    if (new->margin.rightBarStop > new->margin.rightBarStopMax) {
    	fprintf(stderr, "Value specified for XmNrightBarStop is too large\n");
    	new->margin.rightBarStop = new->margin.rightBarStopMax;
    }
    else if (new->margin.rightBarStop == 0)
    	new->margin.rightBarStop = new->margin.rightBarStopMax;
    else if (new->margin.rightBarStop < new->margin.leftBar) {
    	fprintf(stderr, "Value specified for XmNrightBarStop is too small\n");
    	new->margin.rightBarStop = new->margin.leftBar;
    }
}

static void setValidRightBar(MarginSliderWidget new)
{
    /* rightBar */
    if (new->margin.rightBar > new->margin.rightBarStop) {
    	    fprintf(stderr,"Value specified for XmNrightBar is too large\n");
    	new->margin.rightBar = new->margin.rightBarStop;
    }
    else if (new->margin.rightBar == 0)
        new->margin.rightBar = new->margin.rightBarStop;
    else if (new->margin.rightBar < new->margin.leftBar) {
    	fprintf(stderr,"Value specified for XmNrightBar is too small\n");
    	new->margin.rightBar = new->margin.leftBar;
    }
}

#ifdef DEBUG
static void printWidgetValues(MarginSliderWidget new) 
{
    printf("  xMin = %d, xmax = %d, yMin = %d, ymax = %d,\n", new->margin.xMin, 
    	    new->margin.xMax, new->margin.yMin, new->margin.yMax);
    printf("  boxLeft = %d, boxRight = %d, boxTop = %d, boxBottom = %d\n",  
    	    new->margin.boxLeft, new->margin.boxRight, new->margin.boxTop,
    	    new->margin.boxBottom);
    printf("  leftBarStopMin = %d, rightBarStopMax = %d\n", 
    	    new->margin.leftBarStopMin, new->margin.rightBarStopMax);
    printf("  leftBarStop = %d, rightBarStop = %d\n", new->margin.leftBarStop, 
    	    new->margin.rightBarStop);
    printf("  leftBar = %d, rightBar = %d\n", new->margin.leftBar, 
    	    new->margin.rightBar);
}
#endif

/*
** resetBarDragging
**
** Call this before calling dragBar to initialize the drag state.  Can also
** be called to stop any bar dragging currently in progress.
*/
static void resetBarDragging(int *dragState)
{
    *dragState = NOT_DRAGGING;
}

/*
** dragBar
**
** process a button motion (mouse drag) event to 1) check if it is within
** the boundaries of the bar dragging areas and 2) if so process the
** drag event to adjust the margin cross hairs.  Requires the caller to
** maintain the dragState, xDragStart, and yDragStart variables between
** calls.  Call resetBarDragging to initialize dragState before calling
** this routine.  Returns true or false signifying whether to redraw widget.
*/
static int dragBar(XEvent *event, int leftBar, int rightBar, int boxTop,
		   int boxBottom, int boxLeft, int boxRight, int leftBarStop,
		   int rightBarStop, int *dragState, int *xDragStart, 
		   int *xDragEnd, int *dragDiff)
{
    int x=0, y=0;
    int redrawArea = FALSE;

#ifdef DEBUG   
    printf("in dragBar. Drag state = %d.\n", *dragState);
#endif /* DEBUG */

    if (event->type == ButtonPress) {
    	*xDragStart = x;
    	*xDragEnd = x;
#ifdef DEBUG   
        printf("  event type was ButtonPress\n");
#endif /* DEBUG */
    }
#ifdef DEBUG   
    else if (event->type == MotionNotify)
        printf("  event type was MotionNotify\n");
#endif /* DEBUG */

    if (event->type == ButtonPress || event->type == MotionNotify) {
	x = event->xbutton.x;
	y = event->xbutton.y;
#ifdef DEBUG   
	printf(
       "    x = %d, leftBar = %d, rightBar = %d, boxLeft = %d, boxRight = %d\n",
	    x, leftBar, rightBar, boxLeft, boxRight);
	printf("    y = %d, boxBottom = %d, boxTop = %d\n", y, boxBottom, 
	    boxTop);
#endif /* DEBUG */

    	switch (*dragState) {
    	  case NOT_DRAGGING:
    	    if (  x >= leftBar-BAR_WIDTH 
    	       && x <= leftBar+LINE_WIDTH/2
    	       && y >= boxBottom && y <= boxTop ) {
    	    	*dragState = DRAGGING_LEFT_BAR;
    	    	*dragDiff = x - leftBar;
#ifdef DEBUG   
    	        printf("  changed drag state to DRAGGING_LEFT_BAR\n");
    	        printf("          dif = %d", *dragDiff);
#endif /* DEBUG */
    	    }
    	    else if (x >= rightBar-LINE_WIDTH/2 
    	    	  && x <= rightBar+BAR_WIDTH
    	     	  && y >= boxBottom && y <= boxTop ) {
    	    	*dragState = DRAGGING_RIGHT_BAR;
    	    	*dragDiff = x - rightBar;
#ifdef DEBUG   
    	        printf("  changed drag state to DRAGGING_RIGHT_BAR\n");
    	        printf("          dif = %d", *dragDiff);
#endif /* DEBUG */
    	    }
    	    else 
    	    	*dragState = DRAGGING_NOTHING;
    	    *xDragStart = x;
    	    *xDragEnd = x;
    	    break;
    	  case DRAGGING_NOTHING:
    	    break;
    	  case DRAGGING_LEFT_BAR:
    	    if (y < boxBottom)
    	    	y = boxBottom;
    	    if (y > boxTop)
    	    	y = boxTop;
    	    if (x < boxLeft + *dragDiff)
    	    	x = boxLeft + *dragDiff;
    	    if (x < leftBarStop + *dragDiff)
    	    	x = leftBarStop + *dragDiff;
    	    if (x > rightBar + *dragDiff)
    	    	x = rightBar + *dragDiff;
    	    redrawArea = TRUE;
    	    *xDragStart = *xDragEnd;
    	    *xDragEnd = x;
    	    break;
    	  case DRAGGING_RIGHT_BAR:
    	    if (y < boxBottom)
    	    	y = boxBottom;
    	    if (y > boxTop)
    	    	y = boxTop;
    	    if (x > boxRight + *dragDiff)
    	    	x = boxRight + *dragDiff;
    	    if (x > rightBarStop + *dragDiff)
    	    	x = rightBarStop + *dragDiff;
    	    if (x < leftBar + *dragDiff)
    	    	x = leftBar + *dragDiff;
    	    redrawArea = TRUE;
    	    *xDragStart = *xDragEnd;
    	    *xDragEnd = x;
    	    break;
    	}
    }
    
    return redrawArea;
}
