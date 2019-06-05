/*******************************************************************************
*									       *
* CellP.h - Rectangular Cell Plot Widget, Private Header File		       *
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
#ifndef CELLP_H
#define CELLP_H

#include "Cell.h"
#include <Xm/XmP.h>
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>

typedef struct _CellClassPart{
    int ignore;
} CellClassPart;

typedef struct _CellClassRec{
    CoreClassPart  core_class;
    XmPrimitiveClassPart primitive_class;
    CellClassPart  cell_class;
} CellClassRec;

extern CellClassRec cellClassRec;

typedef struct _CellPart {
    GC gc;	   	       	/* Graphics context for axes & labels */
    GC contentsGC;		/* Graphics context for plot contents */
    Pixmap drawBuffer;		/* Double buffering for non-flashing draws */
    int xMin, yMin, xMax, yMax;	/* Boundaries of the drawable area of widget */
    XmFontList font;		/* Motif font list associated with widget */
    XtCallbackList resize;	/* Callbacks */
    XtCallbackList btn2;
    XtCallbackList btn3;
    XtCallbackList redisplay;
    Boolean doubleBuffer;	/* When set, draw first to offscreen pixmap */
    Boolean colorFilled;        /* When set, don't bother to clear the
                                   background because the picture will be
                                   completely filled with color. */
    XmString xAxisLabel;	/* Compound string labels for axes */
    XmString yAxisLabel;
    XmString psFont;            /* PostScript font associated with widget */
    int psFontSize;
    int addLeft, addRight,
        addTop, addBottom;      /* Additional number of margin pixels */
    int xOrigin, yOrigin;	/* The point where the axis lines meet */
    int xEnd, yEnd;		/* The ends of the x and y axis lines */
    int axisLeft, axisTop;	/* Along with xOrigin and yOrigin, define */
    int axisBottom, axisRight;	/*    the boundaries of the axis areas	  */
    int dragState;		/* Is the user currently dragging the mouse? */
    double xDragStart;		/* X (data coord) position of start of drag */
    double yDragStart;		/* Y (data coord) position of start of drag */
    double minXData, maxXData;	/* Minimum and maximum x data values, including
				   possible range increase due to explicit
				   setting of axis limits */
    double minYData, maxYData;	/* Minimum and maximum y data values, including
				   possible range increase due to explicit
				   setting of axis limits */
    double dataMinX, dataMaxX;  /* Minimum and maximum x data values */
    double dataMinY, dataMaxY;  /* Minimum and maximum y data values */
    double minXLim, maxXLim;	/* Min and max x data actually displayed */
    double minYLim, maxYLim;	/* Min and max y data actually displayed */
    CellRect *rects;		/* Contents expressed as colored rectangles */
    int nRects;			/* Number of points */
} CellPart;

typedef struct _CellRec {
   CorePart        core;
   XmPrimitivePart primitive;
   CellPart        cell;
} CellRec;

#endif
