#ifndef COLOR_SCALE_H_
#define COLOR_SCALE_H_

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xcms.h>

#define DEFAULT_COLOR_SCALE_NAME "default"
#define MAX_COLORS_HIGHCOLOR 2000
#define MAX_COLORS_8BITCOLOR 50

typedef enum {
    RGB = 0,
    RGBI,
    CIEXYZ,
    CIEUVY,
    CIEXYY,
    CIELAB,
    CIELUV,
    TEKHVC,
    HSV,
    HLS,
    N_COLOR_SPACES
} ColorSpace;

typedef struct {
    char *name;
    int index;            /* Will be negative for a backup copy */
    int referenceCounter;
    Display *display;
    Colormap colormap;
    ColorSpace colorspace;
    Pixel *pixels;
    int ncolors;
    int isLinear;
    Pixel startPixel;
    Pixel endPixel;
    Pixel underflowPixel;
    Pixel overflowPixel;
    int version;          /* Incremented every time the scale is modified */
} ColorScale;

/* Color scale creation functions. Recognized color names include
   all valid Xlib color names plus color names in the notation
   "HSV:h/s/v" or "HLS:h/l/s", where h, s, v, and l are strings
   which represent a double. The number of requested colors must
   be >= 2 and <= the limit provided by the maxColorScaleColors()
   function. The scale goes between startColor and endColor along
   a straight line in the coordinates of the specified color space.
   NULL is returned in case something goes wrong. Note that if
   an existing color scale with the same name exists, it will be
   modified instead of creation of a new scale. */
const ColorScale * createLinearColorScale(
    const char *name, Display *display, Colormap cmap,
    ColorSpace colorspace, int ncolors,
    const char *startColor, const char *endColor,
    const char *underflowColor, const char *overflowColor);

/* In the following creation function the scale assumes
   the ownership of the given pixels. */
const ColorScale * createLinearColorScaleFromPix(
    const char *name, Display *display, Colormap cmap,
    ColorSpace colorspace, int ncolors,
    Pixel startPixel, Pixel endPixel,
    Pixel underflowPixel, Pixel overflowPixel);

/* The next function increments the reference counter */
void incrColorScaleRefCount(const ColorScale *scale);

/* The next function decrements the reference counter
   and deletes the structure, releasing the color cells,
   if the count drops to 0 */
void decrColorScaleRefCount(const ColorScale *scale);

/* A function to create a backup copy. Returns NULL on failure.
 * Virtually none of the modifier functions (such as setScalePixel)
 * can be used on the backup copies. This is because all such
 * functions have to allocate or deallocate some colors, and
 * the backup copy does not own the colors. If such a modifier
 * is called on a backup copy, the program will be aborted.
 */
const ColorScale * backupColorScale(const ColorScale *);

/* A function to compare the color scale to its backup copy.
 * Returns 0 if the color scale was not modified since its
 * backup was made.
 */
int compareColorScaleToBackup(const ColorScale *scale,
			      const ColorScale *backup);

/* A function to clone an existing color scale or 
   to restore a color scale from backup */
const ColorScale * cloneColorScale(const char *newname,
				   const ColorScale *);

/* Function to look up the color scale by name */
const ColorScale * findColorScale(const char *name);

/* Function to look up the color scale by index */
const ColorScale * getColorScaleAt(int index);

/* Total number of color scales currently in use */
int numUsedColorScales(void);

/* Function to get a pixel value from the given scale. The fraction
   argument should be between 0.0 and 1.0, otherwise underflow or
   overflow pixels are returned. */
Pixel getScalePixel(float fraction, const ColorScale *scale);
Pixel getScalePixelAt(const ColorScale *scale, int num);

/* Note that start pixel and end pixel may differ from the 0th
   and ncolors-1th pixels in case the user redefined individual
   pixels in the scale */
Pixel getStartPixel(const ColorScale *scale);
Pixel getEndPixel(const ColorScale *scale);

/* Function to set a scale pixel value. num values below 0 or 
   above scale->ncolors - 1 will set underflow and overflow pixels.
   The scale assumes the ownership of the given pixel if it is
   not a backup scale. */
void setScalePixel(const ColorScale *scale, int num, Pixel pixel);

/* Function to linearize an existing scale */
void linearizeColorScale(const ColorScale *scale);

/* The following functions automatically linearize the scale */
void setStartPixel(const ColorScale *scale, Pixel pixel);
void setEndPixel(const ColorScale *scale, Pixel pixel);
void setNumColors(const ColorScale *scale, int n);
void setColorSpace(const ColorScale *scale, ColorSpace colorspace);

/* Function to get the name of the color space.
   Returns NULL in case the argument is out of range. */
const char *colorSpaceName(ColorSpace);

/* Function to get the number of the color space. 
 * Returns -1 if the color space name is not valid.
 * The color space names are NOT case sensitive.
 */
int colorSpaceNumber(const char *name);

/* The next function guesses the color space from the color name */
ColorSpace guessColorSpace(const char *colorname);

/* Functions to set/get the max. number of colors */
int guessMaxColorScaleColors(Display *dpy, int screen_num);
void setMaxColorScaleColors(int);
int maxColorScaleColors(void);

/* Function which converts HSV, HLS, and hex color names
   into something parseable by XcmsLookupColor. Also
   converts empty or pure space strings to NULLs. The size
   of the buffer should be at least 32 bytes. If the conversion
   is necessary, the converted color name will be written into
   the buffer, and the pointer to the buffer will be returned.
   Otherwise, the pointer to the color name itself is returned.
   The function makes no attempt to ensure that the color name
   is valid -- the names which are not exactly HSV, HLS, or hex
   colors are returned without any changes. */
const char *normalizeColorName(const char *colorname, char *buffer);

/* Function to print a name of an RGBi color in the given color space */
void printColorName(Display *dpy, Colormap cmap, char *buffer,
		    ColorSpace s, double red, double green, double blue);

/* Functions used to perform the color allocation. They can be used
   to debug the allocation if DEBUG_COLOR_ALLOCATION is defined. Their
   usage is identical to the usage of XcmsAllocColor and XFreeColors. */
Status AllocSharedColor(Display *dpy, Colormap cmap, XcmsColor *color_in_out,
			XcmsColorFormat result_format);
void FreeSharedColors(Display *dpy, Colormap cmap, unsigned long *pixels,
		      int npixels, unsigned long planes);
Status CopySharedColor(Display *dpy, Colormap cmap,
                       Pixel fromcolor, Pixel *tocolor);

/* Function to print color statistics. Will do nothing
   if DEBUG_COLOR_ALLOCATION is not defined. */
void PrintSharedColorsInfo(void);

/* Function to generate an unused color scale name. 
   The buffer should be at least 32 bytes long. */
void unusedColorScaleName(char *buffer);

#endif /* COLOR_SCALE_H_ */
