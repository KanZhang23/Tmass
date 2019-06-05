/*******************************************************************************
*									       *
* MarginSliderP.h - Margin Slider Widget, Private Header File		       *
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
* December 30, 1993							       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
*******************************************************************************/
#ifndef MARGINP_H
#define MARGINP_H

#include "MarginSlider.h"
#include <Xm/XmP.h>

typedef struct _MarginSliderClassPart {
    int ignore;
} MarginSliderClassPart;

typedef struct _MarginSliderClassRec {
    CoreClassPart         core_class;
    XmPrimitiveClassPart  primitive_class;
    MarginSliderClassPart margin_class;
} MarginSliderClassRec;

extern MarginSliderClassRec marginSliderClassRec;

typedef struct _MarginSliderPart{/* Instance record: current state of widget */
    GC gc;	   	       	/* Graphics context */
    int xMin, yMin, xMax, yMax;	/* Boundaries of the drawable area of widget */
    XtCallbackList resize;	/* Callbacks */
    XtCallbackList redisplay;
    XtCallbackList marginChanged;
    XtCallbackList marginDrag;
    int leftBar, rightBar;	/* Where to draw left and right bars	     */
    int boxLeft, boxTop;	/* Defines the boundaries of the margin box  */
    int boxRight, boxBottom;
    int leftBarStop;		/* user-settable boxLeft		     */
    int rightBarStop;		/*   "     "     boxRight		     */
    int leftBarStopMin;		/* read-only value to guide user	     */
    int rightBarStopMax;	/*   "     "    "   "	"    "		     */
    int dragState;		/* Is the user currently dragging the mouse? */
    int xDragStart;		/* X position of start of drag		     */
    int xDragEnd;		/* X position of end of drag		     */
    int dragDiff;		/* difference between ptr loc & bar location */
    int leftPos;		/* user coordinate left bar position	     */
    int rightPos;		/* user coordinate right bar position        */ 
} MarginSliderPart;

typedef struct _MarginSliderRec {
   CorePart         core;
   XmPrimitivePart  primitive;
   MarginSliderPart margin;
} MarginSliderRec;

#endif
