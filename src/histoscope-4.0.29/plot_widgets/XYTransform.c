#include <math.h>
#include <limits.h>
#include <Xm/Xm.h>

#include "XYTransform.h"
#include "drawAxes.h"

#define VERTICAL_MARGINS 30

#define imin(a, b) (a < b ? a : b)
#define imax(a, b) (a > b ? a : b)

/* Convert X and Y values from data coordinates to window coordinates.
   Requires valid transformation data from widget-dependent computeTransform
   function. */
double XYDataToWindowX(XYTransform *xform, double value)
{
    if (xform->xLogScaling) {
    	if (value > 0.)
	    return xform->minXPix +
		(log10(value)-xform->minXData) * xform->xScale;
	else
	    return SHRT_MIN/4;
    }
    return xform->minXPix + (value - xform->minXData) * xform->xScale;
}

double XYDataToWindowY(XYTransform *xform, double value)
{
    if (xform->yLogScaling) {
    	if (value > 0.)
	    return xform->maxYPix -
	    	    (log10(value)-xform->minYData) * xform->yScale;
	else
	    return SHRT_MAX/4;
	    
    } else
	return xform->maxYPix - (value - xform->minYData) * xform->yScale;
}

double XYWindowToDataX(XYTransform *xform, double value)
{
    double result = xform->minXLim + (double)(value - xform->xOrigin) / xform->xScale;
    if (xform->xLogScaling)
        result = pow(10.0, result);
    return result;
}

double XYWindowToDataY(XYTransform *xform, double value)
{
    double result = xform->minYLim + (double)(xform->yOrigin - value) / xform->yScale;
    if (xform->yLogScaling)
        result = pow(10.0, result);
    return result;
}

void setWidgetContentArea(int width, int height, int borderWidth, XFontStruct *fs,
			 int addLeft, int addRight, int addTop, int addBottom,
			 int *xMin, int *yMin, int *xMax, int *yMax)
{
    int availableWidth, availableHeight, axisWidth, axisHeight, add;

    /* Minimal Histo-Scope settings */
    *xMin = borderWidth;
    *yMin = borderWidth;
    *xMax = width - borderWidth;
    *yMax = height - borderWidth;

    axisWidth  = VAxisWidth(fs) + HAxisEndClearance(fs);
    axisHeight = HAxisHeight(fs) + VAxisEndClearance(fs);
    availableWidth = imax(0, width - 2*borderWidth - axisWidth);
    availableHeight = imax(0, height - 2*borderWidth - VERTICAL_MARGINS -
			   axisHeight - fs->ascent - fs->descent);

    if (addRight > 0)
    {
	add = imin(availableWidth, addRight);
	*xMax -= add;
	availableWidth -= add;
    }
    if (addLeft > 0)
    {
	add = imin(availableWidth, addLeft);
	*xMin += add;
	availableWidth -= add;
    }
    if (addTop > 0)
    {
	add = imin(availableHeight, addTop);
	*yMin += add;
	availableHeight -= add;
    }
    if (addBottom > 0)
    {
	add = imin(availableHeight, addBottom);
	*yMax -= add;
	availableHeight -= add;
    }
}
