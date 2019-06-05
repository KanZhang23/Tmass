#ifndef COLORSCALEDIALOG_H_
#define COLORSCALEDIALOG_H_

#include <X11/Intrinsic.h>
#include "ColorScale.h"

typedef struct {
    float min;
    float max;
} LimitsCbResult;

typedef enum {
    DIALOG_OK = 0,
    DIALOG_CANCEL,
    DIALOG_APPLY,
    DIALOG_DELETE
} DialogStatus;

typedef struct _ColorScaleArgs ColorScaleArgs;

struct _ColorScaleArgs {
    const ColorScale *scale;
    float colorMin;
    float colorMax;
    int mappingIsLog;
    int rangeIsDynamic;
    void (*limits_cb)(Widget, XtPointer, LimitsCbResult *);
    XtPointer limits_data;
    void (*apply_cb)(Widget, XtPointer, ColorScaleArgs *);
    XtPointer apply_data;
    void (*close_cb)(Widget, XtPointer, DialogStatus);
    XtPointer close_data;

    int startVersion;       /* The version of the plot color scale
			       when the dialog started. Useful to
			       remember because the scale may change if
			       somebody updates it not from the dialog. */
    DialogStatus reason;    /* This will be set by the dialog when
			       apply_cb callback is invoked */
};

/* The creation function returns a pointer to an opaque structure */
void * CreateColorScale(Widget parent, char *title, ColorScaleArgs *args);

/* The deletion function may be used to destroy the dialog properly
   from outside the dialog itself. It will trigger the close_cb
   call (so it should never be called within close_cb). */
void DestroyColorScale(void *colorScale);

#endif /* COLORSCALEDIALOG_H_ */
