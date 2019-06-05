#ifndef DRAW_OVER_H_
#define DRAW_OVER_H_

#include <Xm/Xm.h>

#define XYSTRING_FILLMASK   1
#define XYSTRING_BORDERMASK 2
#define FORWARD_ARROW_MASK  4
#define BACKWARD_ARROW_MASK 8
#define JUSTIFY_TEXT_MASK   0x30
#define JUSTIFY_TEXT_SHIFT  4
#define PSPIXMAP_UNLINKMASK 0x40

#define POINTS_PER_DEGREE 8

#define MAX_DRAWING_POINTS 32  /* Must not exceed 99 because of
                                  how point keywords are parsed. 
			          Must not exceed 360*POINTS_PER_DEGREE 
				  because of how various objects are
                                  drawn in the "drawOverCB" function. */

enum DrawingObjects {DRAW_POINT = 0, DRAW_LINE, DRAW_RECTANGLE, DRAW_ELLIPSE,
		     DRAW_POLYGON, N_DRAWING_OBJECTS};
enum OverlayedObjTypes {OVER_STRING = 0, OVER_DRAW, OVER_PSPIXMAP,
                        OVER_LATEX, N_OVERLAYED_TYPES};

typedef struct _DrawingPoint {
    float x, y;
    unsigned char coordTypeX, coordTypeY, refTypeX, refTypeY;
} DrawingPoint;

typedef struct _XYString {
    Pixel color;
    Pixel background;
    XmFontList font;
    XmString string;
    char *psfont;
    int psfontsize;
    char *text;
    int options;
    int alignment;
    DrawingPoint reference;
    DrawingPoint position;
} XYString;

typedef struct _DrawingObject {
    int category;
    int options;
    int linestyle;
    Pixel linecolor;
    Pixel fillcolor;
    DrawingPoint reference;
    DrawingPoint *points;
    int npoints;
} DrawingObject;

typedef struct _PSPixmap {
    char *filename;
    char *psdata;
    Pixel bordercolor;
    Pixel background;
    int options;
    int alignment;
    float scale;
    DrawingPoint reference;
    DrawingPoint position;
    int channel, pixmapAllocated;
    int xmin, xmax, ymin, ymax;
    int addLeft, addRight, addTop, addBottom;
    int width, height;
    Pixmap pixmap;
} PSPixmap;

typedef struct _OverlayedObject OverlayedObject;
struct _OverlayedObject {
    int type;
    OverlayedObject *next;
    union {
	XYString s;
	DrawingObject d;
	PSPixmap p;
    } item;
};

void drawOverCB(Widget w, XtPointer wInfo, XtPointer callData);
void setDefaultCommentFont(XmFontList f, char *psfont, int psfontsize);

OverlayedObject *copyOverlayedObjectList(const OverlayedObject *obj, Widget w);

/* The following functions return the number of items destroyed */
int destroyOverlayedObjectList(OverlayedObject *obj, Widget w);
int destroyLastOverlayedObject(OverlayedObject **obj, Widget w);
int destroyLastOverlayedOfType(OverlayedObject **obj, Widget w, int type);
int destroyAllOverlayedOfType(OverlayedObject **obj, Widget w, int type);

int overlayedObjectsReady(OverlayedObject *obj);
int drawingObjectType(const char *category);
int drawingObjectMinPoints(int otype);
const char *drawingObjectCategory(int otype);
char *loadTextFile(const char *filename);
int stringToOverlayedType(const char *str);
const char *overlayedTypeString(int type);

#endif /* DRAW_OVER_H_ */
