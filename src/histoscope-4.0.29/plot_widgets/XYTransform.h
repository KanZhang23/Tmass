#ifndef XYTRANSFORM_H_
#define XYTRANSFORM_H_

#include <X11/Xlib.h>

/* Constants for data to window coordinate transformation */
typedef struct {
    double minXData, minYData, minXLim, minYLim;
    double xScale, yScale, minXPix, maxYPix;
    int xOrigin, yOrigin, xEnd, yEnd;
    char xLogScaling, yLogScaling, pad0, pad1;
} XYTransform;

typedef struct {
    int reason;
    XEvent *event;
    XYTransform *xform;
    int outDevice;
    Drawable drawBuf;
    GC wingc;
    GC plotgc;
} XYCallbackStruct;

double XYDataToWindowX(XYTransform *xform, double value);
double XYDataToWindowY(XYTransform *xform, double value);
double XYWindowToDataX(XYTransform *xform, double value);
double XYWindowToDataY(XYTransform *xform, double value);
void setWidgetContentArea(int width, int ht, int borderWidth, XFontStruct *fs,
			 int addLeft, int addRight, int addTop, int addBottom,
			 int *xMin, int *yMin, int *xMax, int *yMax);

#endif /* XYTRANSFORM_H_ */
