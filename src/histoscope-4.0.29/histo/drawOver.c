#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include "drawOver.h"
#include "../histo_util/hsTypes.h"
#include "../plot_widgets/H1D.h"
#include "../plot_widgets/Scat.h"
#include "../plot_widgets/XY.h"
#include "../plot_widgets/Cell.h"
#include "../plot_widgets/2DGeom.h"
#include "../plot_widgets/2DHistDefs.h"
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/uniLab.h"
#include "../plot_widgets/labels.h"
#include "../plot_widgets/2DHistP.h"
#include "../plot_widgets/3DScat.h"
#include "../plot_widgets/XYTransform.h"
#include "../util/psUtils.h"
#include "histoP.h"
#include "plotWindows.h"
#include "gsComm.h"
#include "globalgc.h"

#define LABEL_PADX 6
#define LABEL_PADY_TOP 1
#define LABEL_PADY_BOTTOM 1
#define MAX_LINE_SEGMENTS (MAX_DRAWING_POINTS+3) /* Segments needed to draw
                                                    a bidirectional arrow */
#define ARROW_LENGTH_THIN 12
#define ARROW_LENGTH_MEDIUM 15
#define ARROW_LENGTH_THICK 18
#define ARROW_ANGLE_DEG 20

#define PI 3.1415926535897931

#ifndef MAXPATHLEN
#define MAXPATHLEN 512
#endif

static XmFontList defaultCommentFont(Widget w);
static int defaultPScommentFontSize(void);
static char *defaultPScommentFont(void);
static float floatMin(float f1, float f2);
static float floatMax(float f1, float f2);
static void destroyDrawingObject(DrawingObject *obj, Display *dpy,
				 Colormap cmap, Pixel black, Pixel white);
static void destroyXYString(XYString *string, Display *dpy,
			    Colormap cmap, Pixel black, Pixel white);
static void destroyPSPixmap(PSPixmap *p, Display *dpy,
			    Colormap cmap, Pixel black, Pixel white);
static void destroyOverlayedObject(OverlayedObject *obj, Display *dpy,
				   Colormap cmap, Pixel black, Pixel white);
static float changeCoordX(float x, int fromCoords,
			  int toCoords, Dimension width,
			  XYTransform *xform);
static float changeCoordY(float y, int fromCoords,
			  int toCoords, Dimension height,
			  XYTransform *xform);
/* The following function returns 1 if there is a piece
   in the FloatSegment which can be drawn, and 0
   if there isn't. */
static int getXSegment(const FloatSegment *fs, XSegment *xs,
		       float xmin, float ymin, float xmax, float ymax);
/* The following function returns 1 if segments
   cross, and coordinate of the crossing point
   is returned in *cr. */
static int crossingPoint(const  FloatSegment *s1,
			 const FloatSegment *s2,
			 FloatPoint *cr);
static int isPointOnSegment(float x, float y, const FloatSegment *s);
static void rotateSegment(const FloatSegment *old, FloatSegment *new,
			  double angle);
static void rotatePoint(float x, float y, float *xout, float *yout,
			double angle);
static int closestint(float f);
static int fillBackArrowSegments(const FloatSegment *s,
				 FloatSegment *fill,
				 int arrow_length);
static int enter2DcompatibilityMode(WidgetClass wClass, int windowHeight);
static void leave2DcompatibilityMode(int psCompatMode);
static void drawXYString(XYString *string, XYCallbackStruct *cbstruct,
			 Display *display, Pixel black,
			 Widget w, Dimension width, Dimension height);
static void drawPSPixmap(windowInfo *wInfo, PSPixmap *p, XYCallbackStruct *cbstruct,
			 Display *display, Pixel black, Widget w,
			 Dimension width, Dimension height);
static void allocatePSPixmap(windowInfo *wInfo, PSPixmap *p);
static void drawDrawingObject(DrawingObject *obj, XYCallbackStruct *cbstruct,
			      Display *display, Pixel black,
			      Widget w, Dimension width, Dimension height);
static int is_executable_file(const char *path);
static int is_readable_file(const char *path);
static const char *find_executable(const char *exename);
static const char *find_ghostscript(void);
static void loadPSPixmapFile(PSPixmap *p);
static const char *sgets(char *buffer, size_t size, const char *stream);

/* Functions for copying overlayed objects */
static OverlayedObject *copyOverlayedObject(
    const OverlayedObject *obj, Display *dpy,
    Colormap cmap, Pixel black, Pixel white);
static void copyXYString(XYString *to, const XYString *from, Display *dpy,
                         Colormap cmap, Pixel black, Pixel white);
static void copyDrawingObject(DrawingObject *to, const DrawingObject *from,
                              Display *dpy, Colormap cmap,
                              Pixel black, Pixel white);
static void copyPSPixmap(PSPixmap *to, const PSPixmap *from, Display *dpy,
                         Colormap cmap, Pixel black, Pixel white);

typedef struct _drawingObjectProperties {
    int minPoints;
    char *category;
} drawingObjectProperties;

static const drawingObjectProperties
drawingObjectInfo[N_DRAWING_OBJECTS] = {
    {1, "point"},
    {2, "line"},
    {2, "rectangle"},
    {2, "ellipse"},
    {3, "polygon"}
};

/* Ideally, first letters of the following names should be distinct */
static const char *overlayedTypes[N_OVERLAYED_TYPES] = {
    "Comment", "Draw", "PSPixmap", "Latex"
};

int drawingObjectType(const char *category)
{
    int i;
    for (i=0; i<N_DRAWING_OBJECTS; ++i)
	if (strcmp(drawingObjectInfo[i].category, category) == 0)
	    return i;
    return -1;
}

int drawingObjectMinPoints(int otype)
{
    if (otype < 0 || otype >= N_DRAWING_OBJECTS)
	return -1;
    else
	return drawingObjectInfo[otype].minPoints;
}

const char *drawingObjectCategory(int otype)
{
    if (otype < 0 || otype >= N_DRAWING_OBJECTS)
	return NULL;
    else
	return drawingObjectInfo[otype].category;
}

void drawOverCB(Widget w, XtPointer wPtr, XtPointer callData)
{
    windowInfo *wInfo = (windowInfo *)wPtr;
    XYCallbackStruct *cbstruct = (XYCallbackStruct *)callData;
    Display *display;
    Pixel black;
    OverlayedObject *obj;
    int psCompatMode = 0;
    Dimension width, height;
    WidgetClass wClass;

    /* It is conceivable that this function may be called 
     * on a window which has just been closed
     */
    if (!existsInWindowList(wInfo))
	return;

    /* Check that we have the geometry info */
    if (cbstruct == NULL)
    {
	fprintf(stderr, "Warning: drawOverCB called without geometry info\n");
	return;
    }

    /* Check that we have something to draw */
    if (wInfo->decor == NULL)
	return;
    obj = wInfo->decor;

    /* Prepare to draw things */
    XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);
    if (width == 0 || height == 0)
	return;
    display = XtDisplay(w);
    black = BlackPixelOfScreen(XtScreen(w));
    wClass = XtClass(w);

    /* Fix PostScript for 3d widgets */
    if (cbstruct->outDevice == PS_PRINTER)
	psCompatMode = enter2DcompatibilityMode(wClass, height);

    /* Reset the line width of plot gc for XY plots */
    if (wClass == xyWidgetClass)
    {
	XGCValues gcValues;
	gcValues.line_width = 0;
	gcValues.line_style = LineSolid;
	XChangeGC(display, cbstruct->plotgc, GCLineWidth|GCLineStyle, &gcValues);
    }

    /* Go over all items to draw */
    do {
	switch (obj->type)
	{
	case OVER_STRING:
	    drawXYString(&obj->item.s, cbstruct, display,
			 black, w, width, height);
	    break;
	case OVER_DRAW:
	    drawDrawingObject(&obj->item.d, cbstruct, display,
			      black, w, width, height);
	    break;
	case OVER_LATEX:
	case OVER_PSPIXMAP:
	    drawPSPixmap(wInfo, &obj->item.p, cbstruct, display,
			 black, w, width, height);
	    break;
	default:
	    assert(0);
	}
	obj = obj->next;
    } while(obj);

    /* Some widget classes do not reset
       their gc before drawing the plots */
    if (wClass == scatWidgetClass)
	XSetForeground(display, cbstruct->plotgc, black);

    /* Leave the 2d compatibility mode */
    if (cbstruct->outDevice == PS_PRINTER)
	leave2DcompatibilityMode(psCompatMode);
}

static void drawXYString(XYString *string, XYCallbackStruct *cbstruct,
			 Display *display, Pixel black,
			 Widget w, Dimension width, Dimension height)
{
    XYTransform *xform = cbstruct->xform;
    Drawable drawBuf = cbstruct->drawBuf;
    GC gc;
    Pixel lastColor = 0;
    XmFontList stringFont;
    int firstColor = 1, psClippingRectangleSet = 0;
    int stringWidth, stringHeight, stringBase;
    int anchorX, anchorY, alignment, drawRect;
    int rectWidth, rectHeight, padx, pady_top, pady_bot;
    float x, y, stringX, stringY, rectX, rectY;
    char *psfont;
    int psfontsize;

    if (cbstruct->outDevice == X_SCREEN)
	assert(drawBuf);

    if (string->position.refTypeX == LAST_POINT_REF ||
	string->position.refTypeY == LAST_POINT_REF)
	/* This is not a valid reference type
	   for labels. Just ignore this object. */
	return;

    /* Make sure that we will not cross the plot boundaries
       when something is specified in plot coordinates.
       Use graphic context with an appropriate clipping mask. */
    if (string->reference.coordTypeX == PLOT_COORDS ||
	string->reference.coordTypeY == PLOT_COORDS ||
	string->position.coordTypeX == PLOT_COORDS ||
	string->position.coordTypeY == PLOT_COORDS)
    {
	if (xform == NULL)
	    /* Can't draw this string */
	   return;
	gc = cbstruct->plotgc;
    }
    else
	gc = cbstruct->wingc;
    assert(gc);

    /* Check if we need to set the PostScript clipping rectangle */
    if (cbstruct->outDevice == PS_PRINTER && gc == cbstruct->plotgc)
    {
	fprintf(PSGetFile(), "gsave\n");
	PSSetClipRectangle(xform->xOrigin+1, xform->yEnd,
			   xform->xEnd, xform->yOrigin-1);
	psClippingRectangleSet = 1;
    }

    /* Draw a rectangle around the string? */
    drawRect = (string->options & XYSTRING_FILLMASK) ||
	(string->options & XYSTRING_BORDERMASK);

    /* Figure out the rectangle coordinates */
    assert(string->position.coordTypeX < N_COORD_TYPES);
    switch (string->position.refTypeX)
    {
    case COORD_ORIGIN_REF:
	rectX = changeCoordX(
	    string->position.x, string->position.coordTypeX,
	    WINDOW_ABS_COORDS, width, xform);
	break;
    case REF_POINT_REF:
	x = changeCoordX(string->reference.x,
			 string->reference.coordTypeX,
			 string->position.coordTypeX,
			 width, xform);
	rectX = changeCoordX(
	    x + string->position.x, string->position.coordTypeX,
	    WINDOW_ABS_COORDS, width, xform);
	break;
    default:
	assert(0);
    }
    assert(string->position.coordTypeY < N_COORD_TYPES);
    switch (string->position.refTypeY)
    {
    case COORD_ORIGIN_REF:
	rectY = changeCoordY(
	    string->position.y, string->position.coordTypeY,
	    WINDOW_ABS_COORDS, height, xform);
	break;
    case REF_POINT_REF:
	y = changeCoordY(string->reference.y,
			 string->reference.coordTypeY,
			 string->position.coordTypeY,
			 height, xform);
	rectY = changeCoordY(
	    y + string->position.y, string->position.coordTypeY,
	    WINDOW_ABS_COORDS, height, xform);
	break;
    default:
	assert(0);
    }
    rectY = height - rectY;

    /* Set string font */
    stringFont = string->font == NULL ? 
	defaultCommentFont(w) : string->font;

    /* Define label rectangle paddings */
    if (drawRect)
    {
	padx = LABEL_PADX;
	pady_top = LABEL_PADY_TOP;
	pady_bot = LABEL_PADY_BOTTOM;
	if (string->options & XYSTRING_BORDERMASK)
	{
	    /* Increase paddings to account for the border */
	    ++padx;
	    ++pady_top;
	    ++pady_bot;
	}
    }
    else
    {
	padx = 0;
	pady_top = 0;
	pady_bot = 0;
    }

    /* String coordinates which take paddings into account */
    stringX = rectX + padx;
    stringY = rectY + pady_top;

    /* Multiline string justification */
    /* 0 is W, 1 is C, 2 is E         */
    switch ((string->options & JUSTIFY_TEXT_MASK) >> JUSTIFY_TEXT_SHIFT)
    {
    case 0:
	alignment = XmALIGNMENT_BEGINNING;
	break;
    case 1:
	alignment = XmALIGNMENT_CENTER;
	break;
    case 2:
    case 3:
	alignment = XmALIGNMENT_END;
	break;
    default:
	assert(0);
    }

    /* Adjust horizontal position. The "string->alignment" member
     * is actually responsible for string anchoring, not alignment.
     */
    stringWidth = XmStringWidth(stringFont, string->string);
    rectWidth   = stringWidth + 2*padx;
    anchorX = string->alignment % 3; /* 0 is W, 1 is C, 2 is E */
    if (anchorX == 0)
    {
	/* Don't have to do anything */
    }
    else if (anchorX == 1)
    {
	stringX -= rectWidth/2.f;
	rectX -= rectWidth/2.f;
    }
    else
    {
	stringX -= rectWidth;
	rectX -= rectWidth;
    }

    /* Adjust vertical position */
    stringHeight = XmStringHeight(stringFont, string->string);
    rectHeight = stringHeight + pady_top + pady_bot;
    anchorY = string->alignment / 3; /* 0 is N, 1 is C, 2 is S */
    if (anchorY >= 2)
    {
	if (drawRect)
	{
	    stringY -= rectHeight;
	    rectY   -= rectHeight;
	}
	else
	{
	    stringBase = XmStringBaseline(stringFont, string->string);
	    stringY -= stringBase;
	    rectY   -= stringBase;
	}
    }
    else if (anchorY == 1)
    {
	stringY -= rectHeight/2.f;
	rectY   -= rectHeight/2.f;
    }

    /* Fill the rectangle around the string */
    if (string->options & XYSTRING_FILLMASK)
    {
	if (string->background != lastColor || firstColor) {
	    XSetForeground(display, gc, string->background);
	    lastColor = string->background;
	    firstColor = 0;
	}
	if (cbstruct->outDevice == X_SCREEN)
	    XFillRectangle(display, drawBuf, gc, rectX,
			   rectY, rectWidth, rectHeight);
	else
	    PSFillRectangle(display, drawBuf, gc, rectX,
			    rectY, rectWidth, rectHeight);
    }

    /* Draw the rectangle border */
    if (string->options & XYSTRING_BORDERMASK)
    {
	if (black != lastColor || firstColor) {
	    XSetForeground(display, gc, black);
	    lastColor = black;
	    firstColor = 0;
	}
	if (cbstruct->outDevice == X_SCREEN)
	    XDrawRectangle(display, drawBuf, gc, rectX, rectY,
			   rectWidth, rectHeight);
	else
	    PSDrawRectangle(display, drawBuf, gc, rectX, rectY,
			    rectWidth, rectHeight);
    }

    /* Draw the string */
    if (string->color != lastColor || firstColor) {
	XSetForeground(display, gc, string->color);
	lastColor = string->color;
	firstColor = 0;
    }
    if (cbstruct->outDevice == X_SCREEN)
	XmStringDraw(display, drawBuf, stringFont, string->string,
		     gc, stringX, stringY, stringWidth, alignment,
		     XmSTRING_DIRECTION_L_TO_R, NULL);
    else if (cbstruct->outDevice == PS_PRINTER)
    {
	if (string->psfont)
	{
	    psfont = string->psfont;
	    psfontsize = string->psfontsize;
	}
	else
	{
	    psfont = defaultPScommentFont();
	    psfontsize = defaultPScommentFontSize();
	}
	PSDrawXmString(display, drawBuf, stringFont, string->string,
		       gc, stringX, stringY, stringWidth, alignment,
		       psfont, psfontsize);
    }
    else
	assert(0);

    /* Check if we need to restore the PostScript clipping rectangle */
    if (psClippingRectangleSet)
	fprintf(PSGetFile(), "grestore\n");
}

static void drawDrawingObject(DrawingObject *obj, XYCallbackStruct *cbstruct,
			      Display *display, Pixel black,
			      Widget w, Dimension width, Dimension height)
{
    /* Buffer of points for various drawings */
    static XPoint xpts[360*POINTS_PER_DEGREE+1]; 

    XYTransform *xform = cbstruct->xform;
    Drawable drawBuf = cbstruct->drawBuf;
    GC gc;
    Pixel lastColor = 0;
    int firstColor = 1, psClippingRectangleSet = 0;
    int i, j, havePlotCoords;
    FloatPoint refpoint[N_COORD_TYPES];
    FloatPoint points[MAX_DRAWING_POINTS+1];
    DrawingPoint *drawpoint;
    float x, y, fwidth, fheight;
    FloatSegment lineSegments[MAX_LINE_SEGMENTS];
    int n_line_segments = 0;

    if (cbstruct->outDevice == X_SCREEN)
	assert(drawBuf);

    /* Check if this object can be ignored */
    if ((obj->options & XYSTRING_FILLMASK) == 0 &&
	(obj->options & XYSTRING_BORDERMASK) == 0)
	return;
    if (obj->npoints == 0)
	return;
    if (obj->points[0].refTypeX == LAST_POINT_REF ||
	obj->points[0].refTypeY == LAST_POINT_REF)
	/* This is not a valid reference type for the first
	   point to draw. Just ignore this object. */
	return;

    /* Make sure that we will not cross the plot boundaries
       when something is specified in plot coordinates.
       Use the graphic context with appropriate clipping mask. */
    havePlotCoords = 0;
    for (i=0; i<obj->npoints; ++i)
	if (obj->points[i].coordTypeX == PLOT_COORDS ||
	    obj->points[i].coordTypeY == PLOT_COORDS)
	{
	    havePlotCoords = 1;
	    break;
	}
    if (obj->reference.coordTypeX == PLOT_COORDS ||
	obj->reference.coordTypeY == PLOT_COORDS)
	havePlotCoords = 1;
    if (havePlotCoords)
    {
	if (xform == NULL)
	    /* Can't draw this object */
	    return;
	gc = cbstruct->plotgc;
    }
    else
	gc = cbstruct->wingc;
    assert(gc);

    /* Check if we need to set the PostScript clipping rectangle */
    if (cbstruct->outDevice == PS_PRINTER && gc == cbstruct->plotgc)
    {
	fprintf(PSGetFile(), "gsave\n");
	PSSetClipRectangle(xform->xOrigin+1, xform->yEnd,
			   xform->xEnd, xform->yOrigin-1);
	psClippingRectangleSet = 1;
    }

    /* Figure out coordinates of the reference 
       point in all coordinate systems */
    for (i=0; i<N_COORD_TYPES; ++i)
    {
	refpoint[i].x = changeCoordX(
	    obj->reference.x, obj->reference.coordTypeX,
	    i, width, xform);
	refpoint[i].y = changeCoordY(
	    obj->reference.y, obj->reference.coordTypeY,
	    i, height, xform);
    }

    /* Check the number of points */
    assert(obj->npoints <= (int)(sizeof(points)/sizeof(points[0])));
    assert(obj->npoints >= drawingObjectMinPoints(obj->category));

    /* Calculate the window coordinates of the object points */
    for (i=0; i<obj->npoints; ++i)
    {
	drawpoint = obj->points + i;
	assert(drawpoint->coordTypeX < N_COORD_TYPES);
	switch (drawpoint->refTypeX)
	{
	case COORD_ORIGIN_REF:
	    points[i].x = changeCoordX(
		drawpoint->x, drawpoint->coordTypeX,
		WINDOW_ABS_COORDS, width, xform);
	    break;
	case LAST_POINT_REF:
	    x = changeCoordX(points[i-1].x,
			     WINDOW_ABS_COORDS,
			     drawpoint->coordTypeX,
			     width, xform);
	    points[i].x = changeCoordX(
		drawpoint->x + x, drawpoint->coordTypeX,
		WINDOW_ABS_COORDS, width, xform);
	    break;
	case REF_POINT_REF:
	    points[i].x = changeCoordX(
		drawpoint->x + refpoint[drawpoint->coordTypeX].x,
		drawpoint->coordTypeX, WINDOW_ABS_COORDS, width, xform);
	    break;
	default:
	    assert(0);
	}
	assert(drawpoint->coordTypeY < N_COORD_TYPES);
	switch (drawpoint->refTypeY)
	{
	case COORD_ORIGIN_REF:
	    points[i].y = changeCoordY(
		drawpoint->y, drawpoint->coordTypeY,
		WINDOW_ABS_COORDS, height, xform);
	    break;
	case LAST_POINT_REF:
	    y = changeCoordY(height - points[i-1].y,
			     WINDOW_ABS_COORDS,
			     drawpoint->coordTypeY,
			     height, xform);
	    points[i].y = changeCoordY(
		drawpoint->y + y, drawpoint->coordTypeY,
		WINDOW_ABS_COORDS, height, xform);
	    break;
	case REF_POINT_REF:
	    points[i].y = changeCoordY(
		drawpoint->y + refpoint[drawpoint->coordTypeY].y,
		drawpoint->coordTypeY, WINDOW_ABS_COORDS, height, xform);
	    break;
	default:
	    assert(0);
	}
	points[i].y = height - points[i].y;
    }

    /* At this point we have everything in window
       coordinates. Draw stuff... */
    switch (obj->category)
    {
    case DRAW_POINT:
	do {
	    int nxpts = 0;
	    for (i=0; i<obj->npoints; ++i)
	    {
		if (points[i].x > SHRT_MIN/4*3 &&
		    points[i].x < SHRT_MAX/4*3 &&
		    points[i].y > SHRT_MIN/4*3 &&
		    points[i].y < SHRT_MAX/4*3)
		{
		    /* Round the coordinates same way as axis ticks */
		    xpts[nxpts].x = (int)(points[i].x);
		    xpts[nxpts].y = closestint(points[i].y);
		    ++nxpts;
		}
	    }
	    if (nxpts > 0)
	    {
		if (obj->linecolor != lastColor || firstColor) {
		    XSetForeground(display, gc, obj->linecolor);
		    lastColor = obj->linecolor;
		    firstColor = 0;
		}
		if (cbstruct->outDevice == X_SCREEN)
		    XDrawPoints(display, drawBuf, gc, xpts,
				nxpts, CoordModeOrigin);
		else if (cbstruct->outDevice == PS_PRINTER)
		    PSDrawPoints(display, drawBuf, gc, xpts,
				 nxpts, CoordModeOrigin);
		else
		    assert(0);
	    }
	} while (0);
	break;

    case DRAW_LINE:
	do {
	    int changeLineStyle = (obj->linestyle >= 0 &&
				   obj->linestyle < XY_N_LINE_STYLES);
	    int draw_same_style_arrows, max_segments, arrow_length;
	    if (obj->linestyle == XY_NO_LINE)
		break;
	    if (changeLineStyle && 
		obj->linestyle != XY_PLAIN_LINE &&
		obj->linestyle != XY_THICK_LINE && 
		obj->linestyle != XY_X_THICK_LINE)
		draw_same_style_arrows = 0;
	    else
		draw_same_style_arrows = 1;

	    /* Figure out a good-looking arrow length */
	    if (obj->linestyle == XY_THICK_LINE)
		arrow_length = ARROW_LENGTH_MEDIUM;
	    else if (obj->linestyle == XY_X_THICK_LINE)
		arrow_length = ARROW_LENGTH_THICK;
	    else
		arrow_length = ARROW_LENGTH_THIN;

	    if (obj->linecolor != lastColor || firstColor) {
		XSetForeground(display, gc, obj->linecolor);
		lastColor = obj->linecolor;
		firstColor = 0;
	    }
	    for (i=0; i<obj->npoints-1; ++i)
	    {
		lineSegments[i].x1 = points[i].x;
		lineSegments[i].y1 = points[i].y;
		lineSegments[i].x2 = points[i+1].x;
		lineSegments[i].y2 = points[i+1].y;
	    }
	    n_line_segments = obj->npoints-1;

	    /* Check if this is an arrow */
	    if (obj->options & BACKWARD_ARROW_MASK)
	    {
		n_line_segments += fillBackArrowSegments(
		    lineSegments, lineSegments+n_line_segments,
		    arrow_length);
	    }
	    if (obj->options & FORWARD_ARROW_MASK)
	    {
		FloatSegment tempSegment;
		tempSegment.x1 = points[obj->npoints-1].x;
		tempSegment.y1 = points[obj->npoints-1].y;
		tempSegment.x2 = points[obj->npoints-2].x;
		tempSegment.y2 = points[obj->npoints-2].y;
		n_line_segments += fillBackArrowSegments(
		    &tempSegment, lineSegments+n_line_segments,
		    arrow_length);
	    }

	    /* Verify that the number of segments is OK */
	    assert(n_line_segments <= MAX_LINE_SEGMENTS);

	    /* Check if we need to set a special line style */
	    if (changeLineStyle)
	    {
		int style = obj->linestyle;
		const char *dashes = XYDashList(style);
		/* The following code is similar to the
		   "setLineStyle" function in XY.c */
		XGCValues gcValues;
		gcValues.line_width = style == XY_THICK_LINE ? 2 :
		    (style == XY_X_THICK_LINE ? 3 : 0);
		gcValues.line_style = (
		    style==XY_PLAIN_LINE || style==XY_THICK_LINE ||
		    style==XY_X_THICK_LINE) ? LineSolid : LineOnOffDash;
		XChangeGC(display, gc, GCLineWidth|GCLineStyle, &gcValues);
		XSetDashes(display, gc, 0, dashes, strlen(dashes));
	    }

	    /* Draw the segments */
	    max_segments = (draw_same_style_arrows ?
			    n_line_segments : obj->npoints-1);
	    if (cbstruct->outDevice == X_SCREEN)
	    {
		XSegment xLineSegments[MAX_LINE_SEGMENTS];
		int n_x_segments = 0;
		for (j=0; j<max_segments; ++j)
		    /* igv: apparently, some X servers have a bug
		     * in handling XDrawSegments function. Therefore,
		     * we can't set limits to SHRT_MIN and SHRT_MAX.
		     * This is related to the position of the window
		     * on the screen. I suspect that coordinates may
		     * overflow when they are converted from window
		     * coordinates into screen coordinates.
		     */
		    n_x_segments += getXSegment(
			lineSegments+j, xLineSegments+n_x_segments,
			SHRT_MIN/4*3, SHRT_MIN/4*3,
			SHRT_MAX/4*3, SHRT_MAX/4*3);
		if (n_x_segments > 0)
		    XDrawSegments(display, drawBuf, gc,
				  xLineSegments, n_x_segments);
	    }
	    else if (cbstruct->outDevice == PS_PRINTER)
	    {
		if (changeLineStyle)
		    PSFloatDrawDashedSegments(display, drawBuf, gc,
					      lineSegments, max_segments,
					      (char *)XYDashList(obj->linestyle), 0);
		else
		    PSFloatDrawSegments(display, drawBuf, gc,
					lineSegments, max_segments);
	    }
	    else
		assert(0);

	    /* Return the line style to default */
	    if (changeLineStyle)
	    {
		XGCValues gcValues;
		gcValues.line_width = 0;
		gcValues.line_style = LineSolid;
		XChangeGC(display, gc, GCLineWidth|GCLineStyle, &gcValues);
	    }

	    /* We may now have to finish drawing the arrows */
	    if (n_line_segments > max_segments)
	    {
		if (cbstruct->outDevice == X_SCREEN)
		{
		    XSegment xLineSegments[4];
		    int n_x_segments = 0;
		    for (j=max_segments; j<n_line_segments; ++j)
			n_x_segments += getXSegment(
			    lineSegments+j, xLineSegments+n_x_segments,
			    SHRT_MIN/4*3, SHRT_MIN/4*3,
			    SHRT_MAX/4*3, SHRT_MAX/4*3);
		    if (n_x_segments > 0)
			XDrawSegments(display, drawBuf, gc,
				      xLineSegments, n_x_segments);
		}
		else if (cbstruct->outDevice == PS_PRINTER)
		{
		    PSFloatDrawSegments(display, drawBuf, gc,
					lineSegments+max_segments,
					n_line_segments-max_segments);
		}
		else
		    assert(0);
	    }
	} while(0);
	break;

    case DRAW_RECTANGLE:
	do {
	    int ix, iy, iwidth, iheight;

	    x = floatMin(points[0].x, points[1].x);
	    y = floatMin(points[0].y, points[1].y);
	    fwidth = floatMax(points[0].x, points[1].x) - x;
	    fheight = floatMax(points[0].y, points[1].y) - y;

	    /* Round the coordinates same way as axis ticks */
	    ix = (int)x;
	    iy = closestint(y);
	    iwidth = (int)(x+fwidth) - ix;
	    iheight = closestint(y+fheight) - iy;

	    if (obj->options & XYSTRING_FILLMASK)
	    {
		if (obj->fillcolor != lastColor || firstColor) {
		    XSetForeground(display, gc, obj->fillcolor);
		    lastColor = obj->fillcolor;
		    firstColor = 0;
		}
		if (cbstruct->outDevice == X_SCREEN)
		    XFillRectangle(display, drawBuf, gc, ix, iy,
				   iwidth, iheight);
		else if (cbstruct->outDevice == PS_PRINTER)
		    PSFillRectangle(display, drawBuf, gc, ix, iy,
				    iwidth, iheight);
		else
		    assert(0);
	    }
	    if (obj->options & XYSTRING_BORDERMASK)
	    {
		if (obj->linecolor != lastColor || firstColor) {
		    XSetForeground(display, gc, obj->linecolor);
		    lastColor = obj->linecolor;
		    firstColor = 0;
		}
		if (cbstruct->outDevice == X_SCREEN)
		    XDrawRectangle(display, drawBuf, gc, ix, iy,
				   iwidth, iheight);
		else if (cbstruct->outDevice == PS_PRINTER)
		    PSDrawRectangle(display, drawBuf, gc, ix, iy,
				    iwidth, iheight);
		else
		    assert(0);
	    }
	} while(0);
	break;

    case DRAW_ELLIPSE:
	do {
	    /* Correlation coefficient */
	    float rho = 0.f;
	    if (obj->npoints > 2)
		rho = obj->points[2].x;

	    x = floatMin(points[0].x, points[1].x);
	    y = floatMin(points[0].y, points[1].y);
	    fwidth = floatMax(points[0].x, points[1].x) - x;
	    fheight = floatMax(points[0].y, points[1].y) - y;

	    if (rho == 0.f)
	    {
		/* Round the coordinates same way as axis ticks */
		int ix, iy, iwidth, iheight;
		ix = (int)x;
		iy = closestint(y);
		iwidth = (int)(x+fwidth) - ix;
		iheight = closestint(y+fheight) - iy;

		/* The major axes of this ellipse 
		   are parallel to the coordinate axes */
		if (obj->options & XYSTRING_FILLMASK)
		{
		    if (obj->fillcolor != lastColor || firstColor) {
			XSetForeground(display, gc, obj->fillcolor);
			lastColor = obj->fillcolor;
			firstColor = 0;
		    }
		    if (cbstruct->outDevice == X_SCREEN)
			XFillArc(display, drawBuf, gc, ix, iy,
				 iwidth, iheight, 0, 360*64);
		    else if (cbstruct->outDevice == PS_PRINTER)
			PSFillArc(display, drawBuf, gc, ix, iy,
				  iwidth, iheight, 0, 360*64);
		    else
			assert(0);
		}
		if (obj->options & XYSTRING_BORDERMASK)
		{
		    if (obj->linecolor != lastColor || firstColor) {
			XSetForeground(display, gc, obj->linecolor);
			lastColor = obj->linecolor;
			firstColor = 0;
		    }
		    if (cbstruct->outDevice == X_SCREEN)
			XDrawArc(display, drawBuf, gc, ix, iy,
				 iwidth, iheight, 0, 360*64);
		    else if (cbstruct->outDevice == PS_PRINTER)
			PSDrawArc(display, drawBuf, gc, ix, iy,
				  iwidth, iheight, 0, 360*64);
		    else
			assert(0);
		}
	    }
	    else if (rho == 1.f || rho == -1.f)
	    {
		/* Draw a line to symbolize a degenerate ellipse */
		int drawit = 0;
		Pixel lineColor = 0;
		
		if (obj->options & XYSTRING_FILLMASK)
		{
		    drawit = 1;
		    lineColor = obj->fillcolor;
		}
		if (obj->options & XYSTRING_BORDERMASK)
		{
		    drawit = 1;
		    lineColor = obj->linecolor;
		}
		if (drawit && fwidth > 0.f && fheight > 0.f)
		{
		    int x1, y1, x2, y2;

		    if (rho == 1.f)
		    {
			x += fwidth;
			fwidth = -fwidth;
		    }
		    x1 = (int)(x);
		    y1 = closestint(y);
		    if (!(x1 >= SHRT_MIN/4*3 && x1 <= SHRT_MAX/4*3 &&
			  y1 >= SHRT_MIN/4*3 && y1 <= SHRT_MAX/4*3))
			drawit = 0;
		    x2 = (int)(x+fwidth);
		    y2 = closestint(y+fheight);
		    if (!(x2 >= SHRT_MIN/4*3 && x2 <= SHRT_MAX/4*3 &&
			  y2 >= SHRT_MIN/4*3 && y2 <= SHRT_MAX/4*3))
			drawit = 0;
		    if (drawit)
		    {
			if (lineColor != lastColor || firstColor) {
			    XSetForeground(display, gc, lineColor);
			    lastColor = lineColor;
			    firstColor = 0;
			}
			if (cbstruct->outDevice == X_SCREEN)
			    XDrawLine(display, drawBuf, gc, x1, y1, x2, y2);
			else if (cbstruct->outDevice == PS_PRINTER)
			    PSFloatDrawLine(display, drawBuf, gc, x, y,
					    x+fwidth, y+fheight);
			else
			    assert(0);
		    }
		}
	    }
	    else
	    {
		/* We will use the following ellipse representation:
		   points 0 and 1 will define the corners of the
		   smallest rectangle which completely encloses
		   the ellipse. X coordinate of point 2 will define
		   the correlation coefficient, no matter what coordinate
		   system it is referred to. Y coordinate of point 2
		   is ignored. If the absolute value of the correlation
		   coefficient is 1 or larger then the ellipse is skipped.

		   Internally, we will measure the angle in units
		   of degrees times POINTS_PER_DEGREE, similar to
		   X servers. This will allow us to precompute
		   trigonometric functions used inside the drawing loop.
		*/
		static float cosvalues[POINTS_PER_DEGREE*360];
		static float sinvalues[POINTS_PER_DEGREE*360];
		static int table_ready = 0;

		double sxsq, sysq, det, drho;

		if (!table_ready)
		{
		    for (i=0; i<POINTS_PER_DEGREE*360; ++i)
		    {
			sinvalues[i] = sin(i/(POINTS_PER_DEGREE*180.0/PI));
			cosvalues[i] = cos(i/(POINTS_PER_DEGREE*180.0/PI));
		    }
		    sinvalues[POINTS_PER_DEGREE*0] = 0.f;
		    sinvalues[POINTS_PER_DEGREE*90] = 1.f;
		    sinvalues[POINTS_PER_DEGREE*180] = 0.f;
		    sinvalues[POINTS_PER_DEGREE*270] = -1.f;
		    cosvalues[POINTS_PER_DEGREE*0] = 1.f;
		    cosvalues[POINTS_PER_DEGREE*90] = 0.f;
		    cosvalues[POINTS_PER_DEGREE*180] = -1.f;
		    cosvalues[POINTS_PER_DEGREE*270] = 0.f;
		    table_ready = 1;
		}

		drho = rho;
		sxsq = fwidth*(1.0-drho*drho)*fwidth/4.0;
		sysq = fheight*(1.0-drho*drho)*fheight/4.0;
		det = (sxsq - sysq)*(sxsq - sysq) + 4.0*sxsq*sysq*drho*drho;
		if (sxsq > 0.0 && sysq > 0.0 && det > 0.0)
		{
		    float cosa, sina, cost, sint, xcenter, ycenter;
		    float r, cx, cy, asq, bsq;
		    int ix, iy, angle, nxpts = 0, drawit = 1;
		    double d1, d2, d3;

		    xcenter = x + fwidth/2.f;
		    ycenter = y + fheight/2.f;
		    det = sqrt(det);
		    asq = 2.0*sxsq*sysq/(sxsq + sysq - det);
		    bsq = 2.0*sxsq*sysq/(sxsq + sysq + det);
		    d1 = sxsq-sysq+det;
		    d2 = 0.5*drho*fwidth*(1.0-drho*drho)*fheight;
		    d3 = sqrt(d1*d1 + d2*d2);
		    cost = d1/d3;
		    sint = d2/d3;
		    for (angle=0; angle<360*POINTS_PER_DEGREE; ++angle)
		    {
			cosa = cosvalues[angle];
			sina = sinvalues[angle];
			r = sqrt(1.f/(cosa*cosa/asq + sina*sina/bsq));
			cx = r*cosa;
			cy = r*sina;
			ix = (int)(xcenter + cx*cost + cy*sint);
			iy = closestint(ycenter - cx*sint + cy*cost);
			if (ix >= SHRT_MIN/4*3 && ix <= SHRT_MAX/4*3 &&
			    iy >= SHRT_MIN/4*3 && iy <= SHRT_MAX/4*3)
			{			
			    if (nxpts == 0)
			    {
				xpts[nxpts].x = ix;
				xpts[nxpts].y = iy;
				++nxpts;
			    }
			    else if (xpts[nxpts-1].x != ix ||
				     xpts[nxpts-1].y != iy)
			    {
				if (nxpts == 1)
				{
				    xpts[nxpts].x = ix;
				    xpts[nxpts].y = iy;
				    ++nxpts;
				}
				else
				{
				    /* Check if the new point results
				       in a parallel segment */
				    if ((ix - xpts[nxpts-1].x)*
					(xpts[nxpts-1].y-xpts[nxpts-2].y) == 
					(iy - xpts[nxpts-1].y)*
					(xpts[nxpts-1].x-xpts[nxpts-2].x))
				    {
					xpts[nxpts-1].x = ix;
					xpts[nxpts-1].y = iy;
				    }
				    else
				    {
					xpts[nxpts].x = ix;
					xpts[nxpts].y = iy;
					++nxpts;
				    }
				}
			    }
			}
			else
			{
			    drawit = 0;
			    break;
			}
		    }
		    if (drawit)
		    {
			if (obj->options & XYSTRING_FILLMASK)
			{
			    if (obj->fillcolor != lastColor || firstColor) {
				XSetForeground(display, gc, obj->fillcolor);
				lastColor = obj->fillcolor;
				firstColor = 0;
			    }
			    if (cbstruct->outDevice == X_SCREEN)
				XFillPolygon(display, drawBuf, gc, xpts, nxpts,
					     Convex, CoordModeOrigin);
			    else if (cbstruct->outDevice == PS_PRINTER)
				PSFloatFillEllipse(display, drawBuf, gc,
						   xcenter, ycenter,
						   2.0*sqrt(asq), 2.0*sqrt(bsq),
						   atan2(sint,cost)/PI*180.0);
			    else
				assert(0);
			}
			if (obj->options & XYSTRING_BORDERMASK)
			{
			    if (obj->linecolor != lastColor || firstColor) {
				XSetForeground(display, gc, obj->linecolor);
				lastColor = obj->linecolor;
				firstColor = 0;
			    }
			    if (cbstruct->outDevice == X_SCREEN)
			    {
				if (xpts[nxpts-1].x != xpts[0].x || 
				    xpts[nxpts-1].y != xpts[0].y)
				{
				    xpts[nxpts] = xpts[0];
				    ++nxpts;
				}
				XDrawLines(display, drawBuf, gc,
					   xpts, nxpts, CoordModeOrigin);
			    }
			    else if (cbstruct->outDevice == PS_PRINTER)
				PSFloatDrawEllipse(display, drawBuf, gc,
						   xcenter, ycenter,
						   2.0*sqrt(asq), 2.0*sqrt(bsq),
						   atan2(sint,cost)/PI*180.0);
			    else
				assert(0);
			}
		    }
		}
	    }
	} while(0);
	break;

    case DRAW_POLYGON:
	/* Will only support polygons in which
	   the path does not self-intersect. We will
	   also uppress polygons which are stretched
	   a lot to avoid problems with conversions
	   to Xlib shorts. */
	if ((obj->options & XYSTRING_FILLMASK) ||
	    (obj->options & XYSTRING_BORDERMASK))
	{
	    int ix, iy, nxpts = 0, drawit = 1;
	    for (i=0; i<obj->npoints; ++i)
	    {
		ix = (int)(points[i].x);
		iy = closestint(points[i].y);
		if (ix >= SHRT_MIN/4*3 && ix <= SHRT_MAX/4*3 &&
		    iy >= SHRT_MIN/4*3 && iy <= SHRT_MAX/4*3)
		{
		    xpts[nxpts].x = ix;
		    xpts[nxpts].y = iy;
		    ++nxpts;
		}
		else
		{
		    drawit = 0;
		    break;
		}
	    }
	    if (drawit)
	    {
		if (obj->options & XYSTRING_FILLMASK)
		{
		    if (obj->fillcolor != lastColor || firstColor) {
			XSetForeground(display, gc, obj->fillcolor);
			lastColor = obj->fillcolor;
			firstColor = 0;
		    }
		    if (cbstruct->outDevice == X_SCREEN)
			XFillPolygon(display, drawBuf, gc, xpts, nxpts,
				     Nonconvex, CoordModeOrigin);
		    else if (cbstruct->outDevice == PS_PRINTER)
			PSFloatFillPolygon(display, drawBuf, gc,
					   points, obj->npoints);
		    else
			assert(0);
		}
		if (obj->options & XYSTRING_BORDERMASK)
		{
		    if (obj->linecolor != lastColor || firstColor) {
			XSetForeground(display, gc, obj->linecolor);
			lastColor = obj->linecolor;
			firstColor = 0;
		    }
		    if (cbstruct->outDevice == X_SCREEN)
		    {
			if (xpts[nxpts-1].x != xpts[0].x || 
			    xpts[nxpts-1].y != xpts[0].y)
			{
			    xpts[nxpts] = xpts[0];
			    ++nxpts;
			}
			XDrawLines(display, drawBuf, gc,
				   xpts, nxpts, CoordModeOrigin);
		    }
		    else if (cbstruct->outDevice == PS_PRINTER)
		    {
			points[obj->npoints] = points[0];
			PSFloatDrawLines(display, drawBuf, gc,
					 points, obj->npoints+1);
		    }
		    else
			assert(0);
		}
	    }
	}
	break;

    default:
	assert(0);
    }

    /* Check if we need to restore the PostScript clipping rectangle */
    if (psClippingRectangleSet)
	fprintf(PSGetFile(), "grestore\n");
}

/* Default comment font */
static XmFontList commentFont = NULL;
static char *PScommentFont = NULL;
static int PScommentFontSize = 0;

static XmFontList defaultCommentFont(Widget w)
{
    if (commentFont == NULL)
	commentFont = XmFontListCopy(_XmGetDefaultFontList(
	    w, XmLABEL_FONTLIST));
    return commentFont;
}

static int defaultPScommentFontSize(void)
{
    return PScommentFontSize;
}

static char *defaultPScommentFont(void)
{
    return PScommentFont;
}

void setDefaultCommentFont(XmFontList f, char *psfont, int psfontsize)
{
    if (f)
    {
	if (commentFont)
	    XmFontListFree(commentFont);
	commentFont = XmFontListCopy(f);
	if (commentFont == NULL)
	    fprintf(stderr, "ERROR in setDefaultCommentFont: out of memory!");
    }

    if (psfont)
    {
	PScommentFontSize = psfontsize;
	if (PScommentFont)
	    free(PScommentFont);
	PScommentFont = strdup(psfont);
	if (PScommentFont == NULL)
	{
	    fprintf(stderr, "ERROR in setDefaultCommentFont: out of memory!\n");
	    PScommentFontSize = 0;
	    return;
	}
    }
}

static void copyPSPixmap(PSPixmap *to, const PSPixmap *from, Display *dpy,
                         Colormap cmap, Pixel black, Pixel white)
{
    *to = *from;
    if (from->filename)
    {
        to->filename = strdup(from->filename);
        assert(to->filename);
    }
    if (from->psdata)
    {
        to->psdata = strdup(from->psdata);
        assert(to->psdata);
    }
    if (from->bordercolor != black && from->bordercolor != white)
    {
        if (CopySharedColor(dpy, cmap, from->bordercolor,
                            &to->bordercolor) == XcmsFailure)
            to->bordercolor = black;
    }
    if (from->background != black && from->background != white)
    {
        if (CopySharedColor(dpy, cmap, from->background,
                            &to->background) == XcmsFailure)
            to->background = white;
    }
    if (from->pixmap != XmUNSPECIFIED_PIXMAP)
    {
        extern Widget MainPanelW;
        Window mywin;
        mywin = XtWindow(MainPanelW);
        to->pixmap = XCreatePixmap(dpy, mywin, from->width, from->height,
                                   DefaultDepthOfScreen(XtScreen(MainPanelW)));
        XCopyArea(dpy, from->pixmap, to->pixmap, globalGC, 0, 0,
                  from->width, from->height, 0, 0);
    }
    to->channel = -10;
}

static void copyDrawingObject(DrawingObject *to, const DrawingObject *from,
                              Display *dpy, Colormap cmap,
                              Pixel black, Pixel white)
{
    *to = *from;
    if (from->linecolor != black && from->linecolor != white)
    {
        if (CopySharedColor(dpy, cmap, from->linecolor,
                            &to->linecolor) == XcmsFailure)
            to->linecolor = black;
    }
    if (from->fillcolor != black && from->fillcolor != white)
    {
        if (CopySharedColor(dpy, cmap, from->fillcolor,
                            &to->fillcolor) == XcmsFailure)
            to->fillcolor = white;
    }
    if (from->npoints > 0 && from->points)
    {
        to->points = (DrawingPoint *)malloc(from->npoints*sizeof(DrawingPoint));
        assert(to->points);
        memcpy(to->points, from->points, from->npoints*sizeof(DrawingPoint));
    }
    else
    {
        to->points = NULL;
        to->npoints = 0;
    }
}

static void copyXYString(XYString *to, const XYString *from, Display *dpy,
                         Colormap cmap, Pixel black, Pixel white)
{
    *to = *from;
    if (from->color != black && from->color != white)
    {
        if (CopySharedColor(dpy, cmap, from->color,
                            &to->color) == XcmsFailure)
            to->color = black;
    }
    if (from->background != black && from->background != white)
    {
        if (CopySharedColor(dpy, cmap, from->background,
                            &to->background) == XcmsFailure)
            to->background = white;
    }
    if (from->font)
        to->font = XmFontListCopy(from->font);
    if (from->string)
        to->string = XmStringCopy(from->string);
    if (from->psfont)
    {
        to->psfont = strdup(from->psfont);
        assert(to->psfont);
    }
    if (from->text)
    {
        to->text = strdup(from->text);
        assert(to->text);
    }
}

static OverlayedObject *copyOverlayedObject(
    const OverlayedObject *obj, Display *dpy, Colormap cmap,
    Pixel black, Pixel white)
{
    OverlayedObject *result;

    result = (OverlayedObject *)malloc(sizeof(OverlayedObject));
    if (result == NULL)
    {
        fprintf(stderr, "copyOverlayedObject: out of memory\n");
        fflush(stderr);
        return NULL;
    }
    result->type = obj->type;
    result->next = obj->next;
    switch (obj->type)
    {
    case OVER_STRING:
	copyXYString(&result->item.s, &obj->item.s,
                     dpy, cmap, black, white);
	break;
    case OVER_DRAW:
	copyDrawingObject(&result->item.d, &obj->item.d,
                          dpy, cmap, black, white);
	break;
    case OVER_LATEX:
    case OVER_PSPIXMAP:
	copyPSPixmap(&result->item.p, &obj->item.p,
                     dpy, cmap, black, white);
	break;
    default:
	assert(0);
    }
    return result;
}

OverlayedObject *copyOverlayedObjectList(const OverlayedObject *obj, Widget w)
{
    OverlayedObject *first = NULL, *previous = NULL;
    Display *dpy;
    Colormap cmap;
    Pixel black, white;

    if (obj == NULL)
        return NULL;
    dpy = XtDisplay(w);
    cmap = DefaultColormapOfScreen(XtScreen(w));
    black = BlackPixelOfScreen(XtScreen(w));
    white = WhitePixelOfScreen(XtScreen(w));
    while (obj)
    {
        OverlayedObject *current;
        current = copyOverlayedObject(obj, dpy, cmap, black, white);
        assert(current);
        if (first == NULL)
            first = current;
        if (previous)
            previous->next = current;
        previous = current;
        obj = obj->next;
    }
    previous->next = NULL;
    return first;
}

int destroyOverlayedObjectList(OverlayedObject *pobj, Widget w)
{
    Display *dpy;
    Colormap cmap;
    Pixel black, white;
    OverlayedObject *obj;
    int n = 0;

    if (pobj)
    {
	dpy = XtDisplay(w);
	cmap = DefaultColormapOfScreen(XtScreen(w));
	black = BlackPixelOfScreen(XtScreen(w));
	white = WhitePixelOfScreen(XtScreen(w));
	while (pobj != NULL)
	{
	    obj = pobj;
	    pobj = pobj->next;
	    destroyOverlayedObject(obj, dpy, cmap, black, white);
	    ++n;
	}
    }
    return n;
}

static void destroyOverlayedObject(OverlayedObject *obj, Display *dpy,
				   Colormap cmap, Pixel black, Pixel white)
{
    switch (obj->type)
    {
    case OVER_STRING:
	destroyXYString(&obj->item.s, dpy, cmap, black, white);
	break;
    case OVER_DRAW:
	destroyDrawingObject(&obj->item.d, dpy, cmap, black, white);
	break;
    case OVER_LATEX:
    case OVER_PSPIXMAP:
	destroyPSPixmap(&obj->item.p, dpy, cmap, black, white);
	break;
    default:
	assert(0);
    }
    free(obj);
}

int destroyLastOverlayedObject(OverlayedObject **poblist, Widget w)
{
    Display *dpy;
    Colormap cmap;
    Pixel black, white;
    OverlayedObject *str = NULL, *pstr;

    assert(poblist);
    pstr = *poblist;
    if (pstr == NULL)
	return 0;
    dpy = XtDisplay(w);
    cmap = DefaultColormapOfScreen(XtScreen(w));
    black = BlackPixelOfScreen(XtScreen(w));
    white = WhitePixelOfScreen(XtScreen(w));
    if (pstr->next == NULL)
	*poblist = NULL;
    while (pstr->next != NULL)
    {
	str = pstr;
	pstr = pstr->next;
    }
    destroyOverlayedObject(pstr, dpy, cmap, black, white);
    if (str)
	str->next = NULL;
    return 1;
}

int destroyAllOverlayedOfType(OverlayedObject **poblist, Widget w, int type)
{
    int nd = destroyLastOverlayedOfType(poblist, w, type);
    if (nd == 0)
	return 0;
    while (destroyLastOverlayedOfType(poblist, w, type))
	++nd;
    return nd;
}

int destroyLastOverlayedOfType(OverlayedObject **poblist, Widget w, int type)
{
    Display *dpy;
    Colormap cmap;
    Pixel black, white;
    OverlayedObject *prev, *last = NULL;

    assert(poblist);
    assert(type >= 0 && type < N_OVERLAYED_TYPES);
    if (*poblist == NULL)
	return 0;
    else
    {
	OverlayedObject *paren = NULL, *pstr = *poblist;
	do {
	    if (pstr->type == type)
	    {
		prev = paren;
		last = pstr;
	    }
	    paren = pstr;
	    pstr = pstr->next;
	} while (pstr);
    }
    if (last == NULL)
	return 0;
    if (prev)
	prev->next = last->next;
    else
	*poblist = NULL;
    dpy = XtDisplay(w);
    cmap = DefaultColormapOfScreen(XtScreen(w));
    black = BlackPixelOfScreen(XtScreen(w));
    white = WhitePixelOfScreen(XtScreen(w));
    destroyOverlayedObject(last, dpy, cmap, black, white);
    return 1;
}

static void destroyDrawingObject(DrawingObject *obj, Display *dpy,
				 Colormap cmap, Pixel black, Pixel white)
{
    if (obj->linecolor != black && obj->linecolor != white)
	FreeSharedColors(dpy, cmap, &obj->linecolor, 1, 0);
    if (obj->fillcolor != black && obj->fillcolor != white)
	FreeSharedColors(dpy, cmap, &obj->fillcolor, 1, 0);
    if (obj->points)
	free(obj->points);
}

static void destroyPSPixmap(PSPixmap *p, Display *dpy,
			    Colormap cmap, Pixel black, Pixel white)
{
    if (p->filename)
	free(p->filename);
    if (p->psdata)
	free(p->psdata);
    if (p->pixmap != XmUNSPECIFIED_PIXMAP)
	XFreePixmap(dpy, p->pixmap);
    if (p->bordercolor != black && p->bordercolor != white)
	FreeSharedColors(dpy, cmap, &p->bordercolor, 1, 0);
    if (p->background != black && p->background != white)
	FreeSharedColors(dpy, cmap, &p->background, 1, 0);
    if (p->channel >= 0)
	updateStatusPointer(p->channel, NULL);
}

static void destroyXYString(XYString *str, Display *dpy,
			    Colormap cmap, Pixel black, Pixel white)
{
    if (str->color != black && str->color != white)
	FreeSharedColors(dpy, cmap, &str->color, 1, 0);
    if (str->background != black && str->background != white)
	FreeSharedColors(dpy, cmap, &str->background, 1, 0);
    if (str->font)
	XmFontListFree(str->font);
    if (str->psfont)
	free(str->psfont);
    if (str->text)
	free(str->text);
    if (str->string)
	XmStringFree(str->string);
}

static float floatMin(float f1, float f2)
{
    if (f1 < f2)
	return f1;
    return f2;
}

static float floatMax(float f1, float f2)
{
    if (f1 < f2)
	return f2;
    return f1;
}

static float changeCoordX(float value, int fromCoords,
			  int toCoords, Dimension scale,
			  XYTransform *xform)
{
    switch (fromCoords)
    {
    case WINDOW_ABS_COORDS:
	switch (toCoords)
	{
	case WINDOW_ABS_COORDS:
	    return value;
	case WINDOW_REL_COORDS:
	    if (scale)
		return value / scale;
	    else
		return 0.f;
	case PLOT_COORDS:
	    if (xform)
		return XYWindowToDataX(xform, value);
	    else
		return 0.f;
	default:
	    assert(0);
	}
    case WINDOW_REL_COORDS:
	switch (toCoords)
	{
	case WINDOW_ABS_COORDS:
	    return value * scale;
	case WINDOW_REL_COORDS:
	    return value;
	case PLOT_COORDS:
	    if (xform)
		return XYWindowToDataX(xform, value * scale);
	    else
		return 0.f;
	default:
	    assert(0);
	}
    case PLOT_COORDS:
	switch (toCoords)
	{
	case WINDOW_ABS_COORDS:
	    if (xform)
		return XYDataToWindowX(xform, value);
	    else
		return 0.f;
	case WINDOW_REL_COORDS:
	    if (xform && scale)
		return XYDataToWindowX(xform, value)/scale;
	    else
		return 0.f;
	case PLOT_COORDS:
	    return value;
	default:
	    assert(0);
	}
    default:
	assert(0);
    }
}

static float changeCoordY(float value, int fromCoords,
			  int toCoords, Dimension scale,
			  XYTransform *xform)
{
    switch (fromCoords)
    {
    case WINDOW_ABS_COORDS:
	switch (toCoords)
	{
	case WINDOW_ABS_COORDS:
	    return value;
	case WINDOW_REL_COORDS:
	    if (scale)
		return value / scale;
	    else
		return 0.f;
	case PLOT_COORDS:
	    if (xform)
		return XYWindowToDataY(xform, scale-value);
	    else
		return 0.f;
	default:
	    assert(0);
	}
    case WINDOW_REL_COORDS:
	switch (toCoords)
	{
	case WINDOW_ABS_COORDS:
	    return value * scale;
	case WINDOW_REL_COORDS:
	    return value;
	case PLOT_COORDS:
	    if (xform)
		return XYWindowToDataY(xform, (1.f-value)*scale);
	    else
		return 0.f;
	default:
	    assert(0);
	}
    case PLOT_COORDS:
	switch (toCoords)
	{
	case WINDOW_ABS_COORDS:
	    if (xform)
		return scale-XYDataToWindowY(xform, value);
	    else
		return 0.f;
	case WINDOW_REL_COORDS:
	    if (xform && scale)
		return 1.f-XYDataToWindowY(xform, value)/scale;
	    else
		return 0.f;
	case PLOT_COORDS:
	    return value;
	default:
	    assert(0);
	}
    default:
	assert(0);
    }
}

static int getXSegment(const FloatSegment *fs, XSegment *xs,
		       float xmin, float ymin, float xmax, float ymax)
{
    int i, have_point_inside, crossing_number;
    FloatPoint p[2];
    FloatSegment border[4];

    border[0].x1 = xmin; border[0].y1 = ymin;
    border[0].x2 = xmin; border[0].y2 = ymax;
    border[1].x1 = xmin; border[1].y1 = ymin;
    border[1].x2 = xmax; border[1].y2 = ymin;
    border[2].x1 = xmax; border[2].y1 = ymin;
    border[2].x2 = xmax; border[2].y2 = ymax;
    border[3].x1 = xmin; border[3].y1 = ymax;
    border[3].x2 = xmax; border[3].y2 = ymax;

    /* Check the case which we expect to be
       the most common: both ends of the segment
       are inside the rectangle. */
    if (fs->x1 >= xmin && fs->x1 <= xmax &&
	fs->y1 >= ymin && fs->y1 <= ymax &&
	fs->x2 >= xmin && fs->x2 <= xmax &&
	fs->y2 >= ymin && fs->y2 <= ymax)
    {
	xs->x1 = (int)(fs->x1);
	xs->y1 = closestint(fs->y1);
	xs->x2 = (int)(fs->x2);
	xs->y2 = closestint(fs->y2);
	return 1;
    }

    /* Check if one of the points is inside the rectangle */
    have_point_inside = 0;
    if (fs->x1 > xmin && fs->x1 < xmax &&
	fs->y1 > ymin && fs->y1 < ymax)
    {
	have_point_inside = 1;
	p[0].x = fs->x1;
	p[0].y = fs->y1;
    }
    else if (fs->x2 > xmin && fs->x2 < xmax &&
	     fs->y2 > ymin && fs->y2 < ymax)
    {
	have_point_inside = 1;
	p[0].x = fs->x2;
	p[0].y = fs->y2;	
    }
    if (have_point_inside)
    {
	xs->x1 = (int)(p[0].x);
	xs->y1 = closestint(p[0].y);
	for (i=0; i<4; ++i)
	    if (crossingPoint(border+i, fs, &p[1]) == 1)
	    {
		xs->x2 = (int)(p[1].x);
		xs->y2 = closestint(p[1].y);
		return 1;
	    }
	return 0;
    }

    /* All points are outside. Should have 2 or 0 crossing points. */
    crossing_number = 0;
    for (i=0; i<4; ++i)
	if (crossingPoint(border+i, fs, p + crossing_number) == 1)
	    if (++crossing_number == 2)
	    {
		xs->x1 = (int)(p[0].x);
		xs->y1 = closestint(p[0].y);
		xs->x2 = (int)(p[1].x);
		xs->y2 = closestint(p[1].y);
		return 1;
	    }
    return 0;
}

static int crossingPoint(const FloatSegment *s1,
			 const FloatSegment *s2,
			 FloatPoint *cr)
{
    FloatSegment s1copy = *s1;
    FloatSegment s2copy = *s2;
    FloatSegment s1rot, s2rot;
    float x_cr;
    double angle;

    /* First, check the special cases */
    if (s1->x1 == s1->x2 && s1->y1 == s1->y2)
    {
	if (isPointOnSegment(s1->x1, s1->y1, s2))
	{
	    cr->x = s1->x1;
	    cr->y = s1->y1;
	    return 1;
	}
	else
	    return 0;
    }
    else if (s2->x1 == s2->x2 && s2->y1 == s2->y2)
    {
	if (isPointOnSegment(s2->x1, s2->y1, s1))
	{
	    cr->x = s2->x1;
	    cr->y = s2->y1;
	    return 1;
	}
	else
	    return 0;
    }

    /* Coordinates of the segments relative to
       the beginning of the first segment */
    s1copy.x1 -= s1->x1;
    s1copy.x2 -= s1->x1;
    s1copy.y1 -= s1->y1;
    s1copy.y2 -= s1->y1;
    s2copy.x1 -= s1->x1;
    s2copy.x2 -= s1->x1;
    s2copy.y1 -= s1->y1;
    s2copy.y2 -= s1->y1;

    /* Go to the coordinate system in which
       the first segment lies along the x axis */
    angle = atan2(s1->y2 - s1->y1, s1->x2 - s1->x1);
    rotateSegment(&s1copy, &s1rot, -angle);
    rotateSegment(&s2copy, &s2rot, -angle);

    if ((s2rot.y1 < 0.f && s2rot.y2 < 0.f) ||
	(s2rot.y1 > 0.f && s2rot.y2 > 0.f))
	return 0;
    if (s2rot.y1 == 0.f)
	x_cr = s2rot.x1;
    else if (s2rot.y2 == 0.f)
	x_cr = s2rot.x2;
    else
	x_cr = s2rot.x1 - (s2rot.x2-s2rot.x1)*s2rot.y1/(s2rot.y2-s2rot.y1);
    if (x_cr < 0.f || x_cr > s1rot.x2)
	return 0;
    rotatePoint(x_cr, 0.f, &cr->x, &cr->y, angle);
    cr->x += s1->x1;
    cr->y += s1->y1;
    return 1;
}

static void rotateSegment(const FloatSegment *old, FloatSegment *new,
			  double angle)
{
    rotatePoint(old->x1, old->y1, &new->x1, &new->y1, angle);
    rotatePoint(old->x2, old->y2, &new->x2, &new->y2, angle);
}
 
static void rotatePoint(float x, float y, float *xout,
			float *yout, double angle)
{
    double cs = cos(angle);
    double si = sin(angle);
    *xout = x*cs - y*si;
    *yout = x*si + y*cs;
}

static int isPointOnSegment(float x, float y, const FloatSegment *s)
{
    float dxp, dxs, dyp, dys, lpsq, lssq, scalp;

    if (s->x1 == s->x2 && s->y1 == s->y2)
    {
	if (s->x1 == x && s->y1 == y)
	    return 1;
	else
	    return 0;
    }
    if (x == s->x1 && y == s->y1)
	return 1;
    dxp  = x - s->x1;
    dyp  = y - s->y1;
    dxs  = s->x2 - s->x1;
    dys  = s->y2 - s->y1;
    lpsq = dxp*dxp + dyp*dyp;
    lssq = dxs*dxs + dys*dys;
    if (lpsq > lssq)
	return 0;
    scalp = dxp*dxs + dyp*dys;
    if (scalp < 0.9*sqrt(lpsq)*sqrt(lssq))
	return 0;
    if (scalp*scalp == lpsq*lssq)
	return 1;
    else
	return 0;
}

static int closestint(float f)
{
    return (f <= -0.5f ? (int)(f - 0.5f) : (int)(f + 0.5f));
}

static int fillBackArrowSegments(const FloatSegment *s,
				 FloatSegment *fill,
				 int arrow_length)
{
    const double angle = ARROW_ANGLE_DEG/180.0*PI;
    FloatSegment scopy;
    float slen, scale;

    if (s->x1 == s->x2 && s->y1 == s->y2)
	return 0;
    scopy.x1 = 0.f;
    scopy.y1 = 0.f;
    scopy.y2 = s->y2 - s->y1;
    scopy.x2 = s->x2 - s->x1;
    slen = sqrt(scopy.y2*scopy.y2 + scopy.x2*scopy.x2);
    scale = arrow_length/slen;
    scopy.y2 *= scale;
    scopy.x2 *= scale;
    rotateSegment(&scopy, fill, angle);
    fill->x1 += s->x1;
    fill->x2 += s->x1;
    fill->y1 += s->y1;
    fill->y2 += s->y1;
    rotateSegment(&scopy, fill+1, -angle);
    (fill+1)->x1 += s->x1;
    (fill+1)->x2 += s->x1;
    (fill+1)->y1 += s->y1;
    (fill+1)->y2 += s->y1;
    return 2;
}

static int enter2DcompatibilityMode(WidgetClass wClass, int windowHeight)
{
    FILE *PSFile;

    if (wClass != scat3DWidgetClass && wClass != hist2DWidgetClass)
	return 0;
    PSFile = PSGetFile();
    assert(PSFile);
    fprintf(PSFile, "gsave\n");
    if (wClass == scat3DWidgetClass)
    {
	fprintf(PSFile, "0 %d translate\n", windowHeight);
	fprintf(PSFile, "1 -1 scale\n");
    }
    else if (wClass == hist2DWidgetClass)
    {
	fprintf(PSFile, "0 %d translate\n", windowHeight << LOWBITS);
	fprintf(PSFile, "%d %d scale\n", MAXPIC, -MAXPIC);
    }
    else
	assert(0);
    return 1;
}

static void leave2DcompatibilityMode(int psCompatMode)
{
    if (psCompatMode)
	fprintf(PSGetFile(), "grestore\n");
}

static int is_executable_file(const char *path)
{
    struct stat stbuf;
    if (access(path, X_OK) == 0)
	if (stat(path, &stbuf) == 0)
	    if (S_ISREG(stbuf.st_mode))
		return 1;
    return 0;
}

static int is_readable_file(const char *path)
{
    struct stat stbuf;
    if (access(path, R_OK) == 0)
	if (stat(path, &stbuf) == 0)
	    if (S_ISREG(stbuf.st_mode))
		return 1;
    return 0;
}

/* The following function returns a pointer to a static 
   area if the executable is found, 0 otherwise. */
static const char *find_executable(const char *exename)
{
    static char filename[MAXPATHLEN];
    const char *searchpath, *default_path = "/bin:/usr/bin:.";
    char *copypath;

    if (exename == NULL)
	return NULL;
    if (exename[0] == '\0')
	return NULL;
    if (exename[0] == '/')
    {
	/* Absolute path specified */
	strncpy(filename, exename, MAXPATHLEN);
	filename[MAXPATHLEN-1] = '\0';
	if (is_executable_file(filename))
	    return filename;
	else
	    return NULL;
    }
    searchpath = getenv("PATH");
    if (searchpath == NULL)
	searchpath = default_path;
    copypath = strdup(searchpath);
    if (copypath == NULL)
    {
	fprintf(stderr, "Error in \"find_executable\": out of memory\n");
	return NULL;
    }
    for (searchpath = strtok(copypath, ":"); searchpath;
	 searchpath = strtok(NULL, ":"))
    {
	strncpy(filename, searchpath, MAXPATHLEN-2);
	filename[MAXPATHLEN-3] = '\0';
	strcat(filename, "/");
	strncat(filename, exename, MAXPATHLEN-strlen(filename));
	filename[MAXPATHLEN-1] = '\0';
	if (is_executable_file(filename))
	    break;
    }
    free(copypath);
    if (searchpath)
	return filename;
    else
	return NULL;
}

static const char *find_ghostscript(void)
{
    static int status = 0;
    static char filename[MAXPATHLEN];
    const char *try;

    if (status == 0)
	if ((try = find_executable(getenv("GHOSTSCRIPT"))))
	{
	    strcpy(filename, try);
	    status = 1;
	}
    if (status == 0)
	if ((try = find_executable("gs")))
	{
	    strcpy(filename, try);
	    status = 1;
	}
    if (status == 0)
    {
	fprintf(stderr, "Warning: failed to find ghostscript executable. "
		"Pixmap generation from PostScript files is disabled.\n");
	status = 2;
    }
    if (status == 1)
	return filename;
    else
	return NULL;
}

static void loadPSPixmapFile(PSPixmap *p)
{
    int ndim = 0, fd = -1, has_bbox = 0, has_tag = 0, isEPSF = 0;
    char buffer[256];
    const char *pos;
    char *prev, *next;
    long dim[4];

    if (p->filename == NULL || p->psdata != NULL)
	return;
    if (!is_readable_file(p->filename))
    {
	fprintf(stderr, "Warning: no file \"%s\" (or unreadable)\n",
		p->filename);
	goto cleanup;
    }
    p->psdata = loadTextFile(p->filename);
    if (p->psdata == NULL)
    {
	perror(p->filename);
	goto cleanup;
    }
    for (pos = sgets(buffer, 256, p->psdata); 
	 pos; pos = sgets(buffer, 256, pos))
    {
        if (buffer[0] != '%' && buffer[0] != '\n')
            break;
	if (strncmp(buffer, "%!", 2) == 0)
	{
	    /* Find the "EPSF" tag */
	    if (strstr(buffer, "EPSF"))
		has_tag = 1;
	}
        else if (strncmp(buffer, "%%BoundingBox:", 14) == 0)
        {
            /* Parse the four integers */
            prev = buffer + 14;
            for (ndim = 0; ndim<4; ++ndim)
            {
                dim[ndim] = strtol(prev, &next, 10);
                if (next == prev)
		{
		    fprintf(stderr, "Warning: bad BoundingBox definition "
			    "in file \"%s\"\n", p->filename);
                    goto cleanup;
		}
                prev = next;
            }
	    p->xmin = dim[0];
	    p->ymin = dim[1];
	    p->xmax = dim[2];
	    p->ymax = dim[3];
            has_bbox = 1;
            break;
        }
    }

    isEPSF = has_bbox && has_tag;
    if (!isEPSF)
	fprintf(stderr, "Warning: file \"%s\" is not"
		" EPS conforming. Ignored.\n", p->filename);

 cleanup:
    if (fd >= 0)
	close(fd);
    if (!isEPSF)
    {
	/* Bad file name or not EPS file */
	if (p->filename)
	{
	    free(p->filename);
	    p->filename = NULL;
	}
	if (p->psdata)
	{
	    free(p->psdata);
	    p->psdata = NULL;
	}
    }
}

char * loadTextFile(const char *filename)
{
    int fd, status;
    size_t nread, filesize;
    char *filedata;
    struct stat statbuf;
    extern int errno;

    fd = open(filename, O_RDONLY);
    if (fd == -1)
	return NULL;
    if (fstat(fd, &statbuf))
	return NULL;
    filesize = statbuf.st_size;
    filedata = (char *)malloc(filesize+1);
    if (filedata == NULL)
	return NULL;
    nread = 0;
    do {
	status = read(fd, filedata+nread, filesize-nread);
	if (status < 0)
	{
	    if (errno == EINTR)
		continue;
	    else
	    {
		status = errno;
		free(filedata);
		close(fd);
		errno = status;
		return NULL;
	    }
	}
	nread += status;
    } while (nread != filesize);
    close(fd);
    filedata[filesize] = '\0';
    return filedata;
}

static const char * sgets(char *buffer, size_t size, const char *stream)
{
    register char c;

    if (buffer == NULL || size == 0)
	return NULL;
    *buffer = '\0';
    if (stream == NULL)
	return NULL;
    if (*stream == '\0')
	return NULL;
    for (--size; size > 0; --size)
    {
	c = *stream;
	if (c == '\0')
	    break;
	*buffer++ = *stream++;
	if (c == '\n')
	    break;
    }
    *buffer = '\0';
    return stream;
}

#define BUFSIZE 256
static void allocatePSPixmap(windowInfo *wInfo, PSPixmap *p)
{
    Window mywin;
    char *gsexe, *fname;
    char buffer[BUFSIZE];
    Atom data_prop;
    pid_t pid = -1;
    Display *display;
    Widget comm;
    int xmin, xmax, ymin, ymax;

    if (p->pixmapAllocated)
	return;
    gsexe = (char *)find_ghostscript();
    if (gsexe == NULL)
	return;
    if (p->filename == NULL)
	return;
    if (p->psdata == NULL)
	loadPSPixmapFile(p);
    if (p->psdata == NULL)
	return;

    /* Check the pixmap dimensions */
    xmin = p->xmin - p->addLeft;
    xmax = p->xmax + p->addRight;
    ymin = p->ymin - p->addBottom;
    ymax = p->ymax + p->addTop;
    if (xmin >= xmax || ymin >= ymax)
	return;
    p->width  = (xmax - xmin + 1) * p->scale;
    p->height = (ymax - ymin + 1) * p->scale;

    /* Get the communication channel number */
    if (p->options & PSPIXMAP_UNLINKMASK)
	fname = p->filename;
    else
	fname = NULL; 
    p->channel = reserveGsChannel(wInfo, fname, p->background,
				  p->width, p->height, &p->channel);
    if (p->channel < 0)
	return;

    comm = getGsChannelWidget(p->channel);
    assert(comm);
    mywin = XtWindow(comm);
    display = XtDisplay(comm);

    /* Create the pixmap */
    p->pixmap = XCreatePixmap(display, mywin, p->width, p->height,
			      DefaultDepthOfScreen(XtScreen(comm)));
    XSetForeground(display, globalGC, p->background);
    XFillRectangle(display, p->pixmap, globalGC, 0, 0, p->width, p->height);

    /* Pass pixmap properties to ghostscript using the "GHOSTVIEW"
       property of the comm window. This is why we need to reserve
       the comm channel. */
    data_prop = XInternAtom(display, "GHOSTVIEW", False);
    assert(data_prop != None);
    sprintf(buffer, "%d %d %d %d %d %d %g %g %d %d %d %d",
            0, /* Pixmap id of the backing pixmap for the window,
                  0 when drawing on a pixmap. */
            0, /* Clockwise rotation of the paper in degrees.
                  Permitted values are 0, 90, 180, 270. */
            xmin, ymin, xmax, ymax,       /* bounding box */
            p->width  * 72.0 / (xmax - xmin + 1), /* xdpi */
            p->height * 72.0 / (ymax - ymin + 1), /* ydpi */
            0, 0, 0, 0    /* left, bottom, top, right: margins around
                             the window. Used for popup zoom windows. */
        );
    XChangeProperty(display, mywin, data_prop, XA_STRING, 8,
                    PropModeReplace, (unsigned char *)buffer, strlen(buffer));
    XSync(display, False);

    /* Run ghostscript */
    {
        char *args[] = {
            NULL, /* Should be gsexe, but some compilers
                     can't take non-const initializers */
	    "-q",
            "-dNOPAUSE",
            "-dBATCH",
            "-dSAFER",
            "-sDEVICE=x11alpha",
            NULL,
            NULL
        };
        args[0] = gsexe;
	args[6] = p->filename;
#ifdef linux
	sprintf(buffer, "%d %d", (int)mywin, (int)p->pixmap);
	setenv("GHOSTVIEW", buffer, 1);
#else
        {
	    static char envbuf[64];
	    sprintf(envbuf, "GHOSTVIEW=%d %d", (int)mywin, (int)p->pixmap);
	    putenv(envbuf);
	}
#endif
        pid = fork();
        if (pid == 0)
        {
            /* in child */
	    execv(gsexe, args);
	    /* If the exec succeeds, we'll never get here */
	    perror(gsexe);
	    exit(EXIT_FAILURE);
        }
    }

    /* Set up the comm channel info */
    fillGsChannelInfo(p->channel, pid, p->pixmap);
    if (pid == -1)
    {
	/* fork failed */
	fprintf(stderr, "Error: failed to start ghostscript\n");
	return;
    }

    /* We are done for now and will be waiting for
       job completion signal from ghostscript */
    p->pixmapAllocated = 1;
    return;
}

static void drawPSPixmap(windowInfo *wInfo, PSPixmap *p,
			 XYCallbackStruct *cbstruct,
			 Display *display, Pixel black,
			 Widget w, Dimension width, Dimension height)
{
    XYTransform *xform = cbstruct->xform;
    Drawable drawBuf = cbstruct->drawBuf;
    GC gc;
    float x, y, rectX, rectY;
    int anchorX, anchorY;
    int psClippingRectangleSet = 0;

    if (!p->pixmapAllocated)
    {
	allocatePSPixmap(wInfo, p);
	if (cbstruct->outDevice == X_SCREEN)
	    /* No point in rushing forward. Pixmap is not filled yet. */
	    return;
    }
    if (cbstruct->outDevice == PS_PRINTER && p->psdata == NULL)
	return;

    if (p->position.refTypeX == LAST_POINT_REF ||
	p->position.refTypeY == LAST_POINT_REF)
	/* This is not a valid reference type
	   for labels. Just ignore this object. */
	return;

    /* Make sure that we will not cross the plot boundaries
       when something is specified in plot coordinates.
       Use graphic context with an appropriate clipping mask. */
    if (p->reference.coordTypeX == PLOT_COORDS ||
	p->reference.coordTypeY == PLOT_COORDS ||
	p->position.coordTypeX == PLOT_COORDS ||
	p->position.coordTypeY == PLOT_COORDS)
    {
	if (xform == NULL)
	    /* Can't draw this string */
	    return;
	gc = cbstruct->plotgc;
    }
    else
	gc = cbstruct->wingc;
    assert(gc);

    /* Check if we need to set the PostScript clipping rectangle */
    if (cbstruct->outDevice == PS_PRINTER && gc == cbstruct->plotgc)
    {
	fprintf(PSGetFile(), "gsave\n");
	PSSetClipRectangle(xform->xOrigin+1, xform->yEnd,
			   xform->xEnd, xform->yOrigin-1);
	psClippingRectangleSet = 1;
    }

    /* Figure out the rectangle coordinates */
    assert(p->position.coordTypeX < N_COORD_TYPES);
    switch (p->position.refTypeX)
    {
    case COORD_ORIGIN_REF:
	rectX = changeCoordX(
	    p->position.x, p->position.coordTypeX,
	    WINDOW_ABS_COORDS, width, xform);
	break;
    case REF_POINT_REF:
	x = changeCoordX(p->reference.x,
			 p->reference.coordTypeX,
			 p->position.coordTypeX,
			 width, xform);
	rectX = changeCoordX(
	    x + p->position.x, p->position.coordTypeX,
	    WINDOW_ABS_COORDS, width, xform);
	break;
    default:
	assert(0);
    }
    assert(p->position.coordTypeY < N_COORD_TYPES);
    switch (p->position.refTypeY)
    {
    case COORD_ORIGIN_REF:
	rectY = changeCoordY(
	    p->position.y, p->position.coordTypeY,
	    WINDOW_ABS_COORDS, height, xform);
	break;
    case REF_POINT_REF:
	y = changeCoordY(p->reference.y,
			 p->reference.coordTypeY,
			 p->position.coordTypeY,
			 height, xform);
	rectY = changeCoordY(
	    y + p->position.y, p->position.coordTypeY,
	    WINDOW_ABS_COORDS, height, xform);
	break;
    default:
	assert(0);
    }
    rectY = height - rectY;

    /* Adjust horizontal position. The "p->alignment" member
     * is actually responsible for string anchoring, not alignment.
     */
    anchorX = p->alignment % 3; /* 0 is W, 1 is C, 2 is E */
    if (anchorX == 0)
	/* Don't have to do anything */;
    else if (anchorX == 1)
	rectX -= p->width/2.f;
    else
	rectX -= p->width;

    /* Adjust vertical position */
    anchorY = p->alignment / 3; /* 0 is N, 1 is C, 2 is S */
    if (anchorY >= 2)
	rectY   -= p->height;
    else if (anchorY == 1)
	rectY   -= p->height/2.f;

    /* Fill the rectangle if this is PostScript. Note that we,
       essentially, ignore XYSTRING_FILLMASK settings. */
    if (cbstruct->outDevice == PS_PRINTER)
    {
	XSetForeground(display, gc, p->background);
	PSFillRectangle(display, drawBuf, gc, rectX, rectY,
			p->width, p->height);
    }

    /* Draw the pixmap */
    if (cbstruct->outDevice == X_SCREEN)
	XCopyArea(display, p->pixmap, drawBuf, gc, 0, 0,
		  p->width, p->height, rectX, rectY);
    else if (cbstruct->outDevice == PS_PRINTER && p->psdata)
    {
	/* Include the text assuming that it is EPS-conforming */
	int xmin = p->xmin - p->addLeft;
	int xmax = p->xmax + p->addRight;
	int ymin = p->ymin - p->addBottom;
	int ymax = p->ymax + p->addTop;
	char *filename = "unknown.eps";
	if (p->filename)
	{
	    filename = strrchr(p->filename, '/');
	    if (filename)
		++filename;
	    else
		filename = p->filename;
	}
	if (xmin < xmax && ymin < ymax)
	    PSImportEPS(filename, p->psdata, rectX, rectY,
			p->width, p->height, xmin, ymin, xmax, ymax);
    }
    else
	assert(0);

    /* Draw the rectangle border */
    if (p->options & XYSTRING_BORDERMASK)
    {
	XSetForeground(display, gc, p->bordercolor);
	if (cbstruct->outDevice == X_SCREEN)
	    XDrawRectangle(display, drawBuf, gc, rectX, rectY,
			   p->width, p->height);
	else
	    PSDrawRectangle(display, drawBuf, gc, rectX, rectY,
			    p->width, p->height);
    }

    /* Check if we need to restore the PostScript clipping rectangle */
    if (psClippingRectangleSet)
	fprintf(PSGetFile(), "grestore\n");
}

/* The following function returns 1 if all objects
   in the list are ready for PostScript generation */
int overlayedObjectsReady(OverlayedObject *obj)
{
    int ready = 1;
    while (obj)
    {
	if (obj->type == OVER_PSPIXMAP || obj->type == OVER_LATEX)
	    if (!obj->item.p.pixmapAllocated || obj->item.p.channel != -10)
	    {
		ready = 0;
		break;
	    }
	obj = obj->next;
    }
    return ready;
}

int stringToOverlayedType(const char *str)
{
    int i;
    for (i=0; i<N_OVERLAYED_TYPES; ++i)
	if (strcmp(str, overlayedTypes[i]) == 0)
	    return i;
    return -1;
}

const char *overlayedTypeString(int type)
{
    if (type >= 0 && type < N_OVERLAYED_TYPES)
	return overlayedTypes[type];
    else
	return NULL;
}

