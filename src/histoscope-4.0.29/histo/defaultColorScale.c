#include "defaultColorScale.h"
#include <X11/Intrinsic.h>

#define DEFAULT_COLOR_SPACE HSV
#define DEFAULT_COLOR_GRADES 100
#define DEFAULT_BEGIN_COLOR "blue"
#define DEFAULT_END_COLOR "red"
#define DEFAULT_UNDERFLOW_COLOR "black"
#define DEFAULT_OVERFLOW_COLOR "magenta"

extern Widget MainPanelW;

const ColorScale * defaultColorScale(void)
{
    const ColorScale *defaultScale;

    defaultScale = findColorScale(DEFAULT_COLOR_SCALE_NAME);
    if (defaultScale == NULL)
    {
	int maxColors = maxColorScaleColors();
	Display *display = XtDisplay(MainPanelW);

	defaultScale = createLinearColorScale(
	    DEFAULT_COLOR_SCALE_NAME, display,
	    DefaultColormapOfScreen(XtScreen(MainPanelW)), DEFAULT_COLOR_SPACE,
	    DEFAULT_COLOR_GRADES < maxColors ? DEFAULT_COLOR_GRADES : maxColors,
	    DEFAULT_BEGIN_COLOR, DEFAULT_END_COLOR,
	    DEFAULT_UNDERFLOW_COLOR, DEFAULT_OVERFLOW_COLOR);
	if (defaultScale)
	    incrColorScaleRefCount(defaultScale);
    }
    return defaultScale;
}
