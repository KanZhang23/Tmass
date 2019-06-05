#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "ColorScale.h"

#include <X11/Xutil.h>

/*  #define DEBUG_COLOR_ALLOCATION */
/*  #define DEBUG_COLORSCALE_REFCOUNTS */

#define MAX_PIXELS 16777216
#define INIT_PIX_COUNT -10

typedef struct _ColorSpaceAttrib {
    char *name;
    XcmsColorFormat format;
} ColorSpaceAttrib;

static const ColorSpaceAttrib ColorSpaceData[N_COLOR_SPACES] = {
    {"RGB",    XcmsRGBiFormat},
    {"RGBi",   XcmsRGBiFormat},
    {"CIEXYZ", XcmsCIEXYZFormat},
    {"CIEuvY", XcmsCIEuvYFormat},
    {"CIExyY", XcmsCIExyYFormat},
    {"CIELab", XcmsCIELabFormat},
    {"CIELuv", XcmsCIELuvFormat},
    {"TekHVC", XcmsTekHVCFormat},
    {"HSV",    XcmsRGBiFormat},
    {"HLS",    XcmsRGBiFormat}
};

static ColorScale **scaleSet = NULL;
static int nscales = 0;
static int max_shade_colors = MAX_COLORS_8BITCOLOR;

#ifdef DEBUG_COLOR_ALLOCATION
static signed char *pixel_counts = NULL;

static void init_pixel_counting(void)
{
    if (pixel_counts == NULL)
    {
	pixel_counts = (signed char *)malloc(MAX_PIXELS);
	if (pixel_counts == NULL)
	{
	    fprintf(stderr, "Fatal error: out of memory. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	memset(pixel_counts, INIT_PIX_COUNT, MAX_PIXELS);
    }
}
#endif

void PrintSharedColorsInfo(void)
{
#ifdef DEBUG_COLOR_ALLOCATION
    int i, n_alloc = 0, n_free = 0;
    register signed char pix;

    init_pixel_counting();
    for (i=0; i<MAX_PIXELS; ++i)
    {
	pix = pixel_counts[i];
	if (pix != INIT_PIX_COUNT)
	    ++n_alloc;
	if (pix == 0)
	    ++n_free;
    }
    printf("%d pixels allocated, %d freed\n", n_alloc, n_free);
#endif
}

int numUsedColorScales(void)
{
    int i, count = 0;
    for (i=0; i<nscales; ++i)
	if (scaleSet[i])
	    ++count;
    return count;
}

const ColorScale * getColorScaleAt(int index)
{
    if (index < 0 || index >= nscales)
	return NULL;
    return scaleSet[index];
}

Status CopySharedColor(Display *dpy, Colormap cmap,
                       Pixel fromcolor, Pixel *tocolor)
{
    XcmsColor cmsColor;
    Status status;

    cmsColor.pixel = fromcolor;
    XcmsQueryColor(dpy, cmap, &cmsColor, XcmsRGBiFormat);
    status = XcmsAllocColor(dpy, cmap, &cmsColor, XcmsRGBiFormat);
#ifdef DEBUG_COLOR_ALLOCATION
    init_pixel_counting();
    if (status != XcmsFailure)
    {
	if (cmsColor.pixel >= MAX_PIXELS)
	{
	    fprintf(stderr, "Fatal alloc error: pixel out of range. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	if (pixel_counts[cmsColor.pixel] == INIT_PIX_COUNT)
	    pixel_counts[cmsColor.pixel] = 1;
	else if (++pixel_counts[cmsColor.pixel] == 127)
	{
	    fprintf(stderr, "Fatal alloc error: pixel allocated to many times. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
    }
#endif
    *tocolor = cmsColor.pixel;
    return status;
}

Status AllocSharedColor(Display *dpy, Colormap cmap, XcmsColor *color_in_out,
			XcmsColorFormat result_format)
{
#ifdef DEBUG_COLOR_ALLOCATION
    Status status;

    init_pixel_counting();
    status = XcmsAllocColor(dpy, cmap, color_in_out, result_format);
    if (status != XcmsFailure)
    {
	if (color_in_out->pixel >= MAX_PIXELS)
	{
	    fprintf(stderr, "Fatal alloc error: pixel out of range. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	if (pixel_counts[color_in_out->pixel] == INIT_PIX_COUNT)
	    pixel_counts[color_in_out->pixel] = 1;
	else if (++pixel_counts[color_in_out->pixel] == 127)
	{
	    fprintf(stderr, "Fatal alloc error: pixel allocated to many times. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
    }
    return status;
#else
    return XcmsAllocColor(dpy, cmap, color_in_out, result_format);
#endif
}

void FreeSharedColors(Display *dpy, Colormap cmap, unsigned long *pixels,
		      int npixels, unsigned long planes)
{
#ifdef DEBUG_COLOR_ALLOCATION
    int i;

    init_pixel_counting();
    for (i=0; i<npixels; ++i)
    {
	if (pixels[i] >= MAX_PIXELS)
	{
	    fprintf(stderr, "Fatal free error: pixel out of range. Aborting.\n");
	    abort();
	}
	if (pixel_counts[pixels[i]] == INIT_PIX_COUNT)
	{
	    fprintf(stderr, "Fatal free error: pixel has never been allocated. Aborting.\n");
	    abort();
	}
	if (pixel_counts[pixels[i]]-- == 0)
	{
	    fprintf(stderr, "Fatal free error: pixel is already free. Aborting.\n");
	    abort();
	}
    }
#endif
    XFreeColors(dpy, cmap, pixels, npixels, planes);
}

static void freeUpAllColors(ColorScale *scale)
{
    if (scale->ncolors > 0 && scale->index >= 0)
    {
	/* This is not a backup copy, and it has colors to free */
	FreeSharedColors(scale->display, scale->colormap,
		    scale->pixels, scale->ncolors, 0);
	FreeSharedColors(scale->display, scale->colormap,
		    &scale->underflowPixel, 1, 0);
	FreeSharedColors(scale->display, scale->colormap,
		    &scale->overflowPixel, 1, 0);
	FreeSharedColors(scale->display, scale->colormap,
		    &scale->startPixel, 1, 0);
	FreeSharedColors(scale->display, scale->colormap,
		    &scale->endPixel, 1, 0);
	scale->ncolors = 0;
	++scale->version;
    }
}

const ColorScale * backupColorScale(const ColorScale *scale)
{
    ColorScale *tmp;
    int compare;

    if (scale == NULL)
	return NULL;
    tmp = (ColorScale *)malloc(sizeof(ColorScale));
    if (tmp == NULL)
    {
	fprintf(stderr, "Out of memory!\n");
	return NULL;
    }
    *tmp = *scale;
    tmp->index = -1;
    tmp->referenceCounter = 0;
    tmp->name = NULL;
    tmp->pixels = NULL;
    tmp->name = strdup(scale->name);
    tmp->pixels = malloc(scale->ncolors*sizeof(Pixel));
    if (tmp->name == NULL || tmp->pixels == NULL)
    {
	fprintf(stderr, "Out of memory!\n");
	decrColorScaleRefCount(tmp);
	return NULL;
    }
    memcpy(tmp->pixels, scale->pixels,
	   scale->ncolors*sizeof(Pixel));
    /* Verify that things got copied OK */
    compare = compareColorScaleToBackup(scale, tmp);
    assert(compare == 0);
    return tmp;
}

int compareColorScaleToBackup(const ColorScale *scale,
			      const ColorScale *backup)
{
    int i;

    /* Backup copy may differ from the original
       only in index and reference counter */
    assert(scale);
    assert(backup);
    if (scale->version != backup->version)
	return 1;
    if (strcmp(scale->name, backup->name))
	return 1;
    if (scale->display != backup->display)
	return 1;
    if (scale->colormap != backup->colormap)
	return 1;
    if (scale->colorspace != backup->colorspace)
	return 1;
    if (scale->ncolors != backup->ncolors)
	return 1;
    if (scale->isLinear != backup->isLinear)
	return 1;
    if (scale->startPixel != backup->startPixel)
	return 1;
    if (scale->endPixel != backup->endPixel)
	return 1;
    if (scale->overflowPixel != backup->overflowPixel)
	return 1;
    if (scale->underflowPixel != backup->underflowPixel)
	return 1;
    for (i=0; i<backup->ncolors; ++i)
	if (scale->pixels[i] != backup->pixels[i])
	    return 1;
    return 0;
}

const ColorScale * findColorScale(const char *name)
{
    int i;
    if (name == NULL)
	return NULL;
    for (i=0; i<nscales; ++i)
	if (scaleSet[i])
	    if (strcmp(scaleSet[i]->name, name) == 0)
		return scaleSet[i];
    return NULL;
}

void setMaxColorScaleColors(int i)
{
    max_shade_colors = i;
}

int maxColorScaleColors(void)
{
    return max_shade_colors;
}

int guessMaxColorScaleColors(Display *display, int screen_num)
{
    int default_depth;
    int visualClass = 5;
    XVisualInfo visual_info;

    default_depth = DefaultDepth(display, screen_num);
    if (default_depth < 8)
    {
	/* Will not be able to get enough colors anyway */
	return 0;
    }
    while (!XMatchVisualInfo(display, screen_num, default_depth,
			     visualClass--, &visual_info))
	;
    ++visualClass;
    if (visualClass < 2)
    {
	/* No color visual available at default depth */
	return 0;
    }
    /* The visual we found is not necessarily the default visual, and
     * therefore it is not necessarily the one we used to create our
     * windows. However, we now know for sure that color is supported.
     */
    if (default_depth == 8)
	return MAX_COLORS_8BITCOLOR;
    else
	return MAX_COLORS_HIGHCOLOR;
}

static int newScalePosition(void)
{
    int i;
    ColorScale **newset;

    if (nscales > 0)
	for (i=0; i<nscales; ++i)
	    if (scaleSet[i] == NULL)
		return i;
    newset = (ColorScale **)realloc(scaleSet, (nscales+1)*sizeof(ColorScale *));
    if (newset == NULL)
    {
	fprintf(stderr, "Out of memory\n");
	return -1;
    }
    scaleSet = newset;
    scaleSet[nscales] = NULL;
    return nscales++;
}

static void RGBi_to_HSV(const XcmsColor *color, XcmsFloat *h,
			XcmsFloat *s, XcmsFloat *v)
{
    XcmsFloat tmp, r, g, b, min, max, delta;

    r = color->spec.RGBi.red;
    if (r < 0.0) r = 0.0;
    else if (r > 1.0) r = 1.0;
    g = color->spec.RGBi.green;
    if (g < 0.0) g = 0.0;
    else if (g > 1.0) g = 1.0;
    b = color->spec.RGBi.blue;
    if (b < 0.0) b = 0.0;
    else if (b > 1.0) b = 1.0;

    tmp = r < g ? r : g;
    min = tmp < b ? tmp : b;
    tmp = r > g ? r : g;
    max = tmp > b ? tmp : b;
    delta = max - min;

    *v = max;
    if (max == 0.0 || delta == 0.0)
    {
	/* r = g = b = 0 or 1 */
	*s = 0.0;
	*h = 0.0;
	return;
    }
    *s = delta / max;
    if (r == max)
	*h = ( g - b ) / delta;         /* between yellow & magenta */
    else if ( g == max )
	*h = 2 + ( b - r ) / delta;     /* between cyan & yellow    */
    else
	*h = 4 + ( r - g ) / delta;     /* between magenta & cyan   */
    *h *= 60.0;                         /* degrees */
    if (*h < 0.0)
	*h += 360.0;
}

static void HSV_to_RGBi(XcmsFloat h, XcmsFloat s,
			XcmsFloat v, XcmsColor *color)
{
    int i = 0;
    XcmsFloat f, p, q, t;

    if (v < 0.0) v = 0.0;
    else if (v > 1.0) v = 1.0;
    if (s < 0.0) s = 0.0;
    else if (s > 1.0) s = 1.0;
    while (h < 0.0 && i++ < 10000)
	h += 360.0;
    while (h > 360.0 && i++ < 10000)
	h -= 360.0;
    if (h < 0.0 || h >= 360.0) h = 0.0;

    if (s == 0.0) {
	/* achromatic (grey) */
	color->spec.RGBi.red   = v;
	color->spec.RGBi.green = v;
	color->spec.RGBi.blue  = v;
	return;
    }

    h /= 60.0;                      /* sector 0 to 5 */
    i = floor( h );
    f = h - i;                      /* factorial part of h */
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch ( i )
    {
    case 0:
	color->spec.RGBi.red   = v;
	color->spec.RGBi.green = t;
	color->spec.RGBi.blue  = p;
	break;
    case 1:
	color->spec.RGBi.red   = q;
	color->spec.RGBi.green = v;
	color->spec.RGBi.blue  = p;
	break;
    case 2:
	color->spec.RGBi.red   = p;
	color->spec.RGBi.green = v;
	color->spec.RGBi.blue  = t;
	break;
    case 3:
	color->spec.RGBi.red   = p;
	color->spec.RGBi.green = q;
	color->spec.RGBi.blue  = v;
	break;
    case 4:
	color->spec.RGBi.red   = t;
	color->spec.RGBi.green = p;
	color->spec.RGBi.blue  = v;
	break;
    default:   /* case 5: */
	color->spec.RGBi.red   = v;
	color->spec.RGBi.green = p;
	color->spec.RGBi.blue  = q;
	break;
    }
}

static void RGBi_to_HLS(const XcmsColor *color, XcmsFloat *h,
			XcmsFloat *l, XcmsFloat *s)
{
    XcmsFloat tmp, r, g, b, min, max, delta, sum;

    r = color->spec.RGBi.red;
    if (r < 0.0) r = 0.0;
    else if (r > 1.0) r = 1.0;
    g = color->spec.RGBi.green;
    if (g < 0.0) g = 0.0;
    else if (g > 1.0) g = 1.0;
    b = color->spec.RGBi.blue;
    if (b < 0.0) b = 0.0;
    else if (b > 1.0) b = 1.0;

    tmp = r < g ? r : g;
    min = tmp < b ? tmp : b;
    tmp = r > g ? r : g;
    max = tmp > b ? tmp : b;
    delta = max - min;
    sum = max + min;

    *l = sum/2.0;
    if (delta == 0.0)
    {
	*s = 0.0;
	*h = 0.0;
	return;
    }
    if (*l <= 0.5)
	*s = delta/sum;
    else
	*s = delta/(2.0-sum);
    if( r == max )
	*h = ( g - b ) / delta;
    else if ( g == max )
	*h = 2 + ( b - r ) / delta;
    else
	*h = 4 + ( r - g ) / delta;
    *h *= 60.0;
    if (*h < 0.0)
	*h += 360.0;
}

static XcmsFloat get_hls_value(XcmsFloat n1, XcmsFloat n2, XcmsFloat h)
{
    XcmsFloat hls_value;

    if (h < 0.0)
	h += 360.0;
    else if (h >= 360.0)
	h -= 360.0;

    if ( h < 60.0 )
	hls_value = n1 + ( n2 - n1 ) * h / 60.0;
    else if ( h < 180.0 )
	hls_value = n2;
    else if ( h < 240.0 )
	hls_value = n1 + ( n2 - n1 ) * ( 240.0 - h ) / 60.0;
    else
	hls_value = n1;
    return hls_value;
}

static void HLS_to_RGBi(XcmsFloat h, XcmsFloat l,
			XcmsFloat s, XcmsColor *color)
{
    int i = 0;
    XcmsFloat m1, m2;

    if (l < 0.0) l = 0.0;
    else if (l > 1.0) l = 1.0;
    if (s < 0.0) s = 0.0;
    else if (s > 1.0) s = 1.0;
    while (h < 0.0 && i++ < 10000)
	h += 360.0;
    while (h > 360.0 && i++ < 10000)
	h -= 360.0;
    if (h < 0.0 || h >= 360.0) h = 0.0;

    if (s == 0.0) {
	/* achromatic (grey) */
	color->spec.RGBi.red   = l;
	color->spec.RGBi.green = l;
	color->spec.RGBi.blue  = l;
    } else {
	if (l <= 0.5)
	    m2 = l + l*s;
	else
	    m2 = l + s - l*s;
	m1 = 2.0*l - m2;
	color->spec.RGBi.red   = get_hls_value(m1, m2, h + 120.0);
	color->spec.RGBi.green = get_hls_value(m1, m2, h);
	color->spec.RGBi.blue  = get_hls_value(m1, m2, h - 120.0);
    }
}

static int parse_hex_triplet(const char *hexcolor, XcmsFloat *a,
			     XcmsFloat *b, XcmsFloat *c)
{
    unsigned int red, green, blue, shift, len = 0;
    const char *col = hexcolor;
    char format[16];

    while (*col && !isspace(*col))
    {
  	if (!isxdigit(*col))
 	    return 1;
	++len; ++col;
    }
    if (len <= 0 || len > 12 || len % 3)
	return 1;

    /* Check that we do not have more stuff in this string */
    while (*col && isspace(*col)) ++col;
    if (*col) return 1;

    /* Get the color */
    len /= 3;
    sprintf(format, "%%%dx%%%dx%%%dx", len, len, len);
    sscanf(hexcolor, format, &red, &green, &blue);
    shift = 16 - len*4;
    *a = (red   << shift)/(XcmsFloat)0xffff;
    *b = (green << shift)/(XcmsFloat)0xffff;
    *c = (blue  << shift)/(XcmsFloat)0xffff;
    return 0;
}

static int parse_triplet(const char *triplet, XcmsFloat *a,
			 XcmsFloat *b, XcmsFloat *c)
{
    /* The following algorithm approximately corresponds
     * to the way Xlib parses colors (same bugs).
     */
    const char *beginptr = triplet;
    char *endptr;

    *a = strtod(beginptr, &endptr);
    if (beginptr == endptr || endptr[0] == '\0')
	return 1;
    beginptr = endptr + 1;
    *b = strtod(beginptr, &endptr);
    if (beginptr == endptr || endptr[0] == '\0')
	return 1;
    beginptr = endptr + 1;
    *c = strtod(beginptr, &endptr);
    if (beginptr == endptr)
	return 1;
    else
	return 0;
}

const char * normalizeColorName(const char *colorname, char *buffer)
{
    XcmsFloat a, b, c;
    XcmsColor cmscolor;

    if (colorname == NULL)
	return NULL;
    while (*colorname && isspace(*colorname))
	++colorname;
    if (*colorname == '\0')
	return NULL;
    if (strncasecmp(colorname, "HSV:", 4) == 0)
    {
	if (parse_triplet(colorname+4, &a, &b, &c) == 0)
	{
	    HSV_to_RGBi(a, b, c, &cmscolor);
	    sprintf(buffer, "RGBi:%f/%f/%f",
		    (double)cmscolor.spec.RGBi.red,
		    (double)cmscolor.spec.RGBi.green,
		    (double)cmscolor.spec.RGBi.blue);
	    return buffer;
	}
    }
    else if (strncasecmp(colorname, "HLS:", 4) == 0)
    {
	if (parse_triplet(colorname+4, &a, &b, &c) == 0)
	{
	    HLS_to_RGBi(a, b, c, &cmscolor);
	    sprintf(buffer, "RGBi:%f/%f/%f",
		    (double)cmscolor.spec.RGBi.red,
		    (double)cmscolor.spec.RGBi.green,
		    (double)cmscolor.spec.RGBi.blue);
	    return buffer;
	}
    }
    else if (strncasecmp(colorname, "#", 1) == 0)
    {
	/* Try to parse it as an RGB color */
	if (parse_hex_triplet(colorname+1, &a, &b, &c) == 0)
	{
	    sprintf(buffer, "RGBi:%f/%f/%f",
		    (double)a, (double)b, (double)c);
	    return buffer;
	}
    }
    return colorname;
}

static void normalize_color_names(
    const char *in1, const char *in2, const char *in3, const char *in4,
    const char **out1, const char **out2, const char **out3, const char **out4)
{
    static char buffer[400];
    *out1 = normalizeColorName(in1, buffer);
    *out2 = normalizeColorName(in2, buffer+100);
    *out3 = normalizeColorName(in3, buffer+200);
    *out4 = normalizeColorName(in4, buffer+300);
}

#define check_color(c) do {\
    if (XcmsLookupColor(display, colormap, c, &exact_def,\
			&screen_def, format) == XcmsFailure)\
    {\
	fprintf(stderr, "Error in createLinearColorScale: "\
		"bad color \"%s\"\n", c);\
	return NULL;\
    }\
} while(0);

#define alloc_color(name, color) do {\
    if (AllocSharedColor(display, colormap, &color, format) == XcmsFailure)\
    {\
	fprintf(stderr, "Error in createLinearColorScale: "\
		"failed to allocate color %s\n", name);\
	return NULL;\
    }\
} while(0);

#define alloc_cspace_colorscale(Colorspace) do {\
    RGBi_to_ ## Colorspace (&start_def, &h_begin, &s_begin, &v_begin);\
    RGBi_to_ ## Colorspace (&end_def, &h_end, &s_end, &v_end);\
    dx = (h_end - h_begin)/ncolors;\
    dy = (s_end - s_begin)/ncolors;\
    dz = (v_end - v_begin)/ncolors;\
    for (i=0; i<ncolors; ++i)\
    {\
	if (i == 0)\
	{\
	    h = h_begin;\
	    s = s_begin;\
	    v = v_begin;\
	}\
	else if (i == ncolors-1)\
	{\
	    h = h_end;\
	    s = s_end;\
	    v = v_end;\
	}\
	else\
	{\
	    h = h_begin + dx*(i+0.5);\
	    s = s_begin + dy*(i+0.5);\
	    v = v_begin + dz*(i+0.5);\
	}\
	Colorspace ## _to_RGBi (h, s, v, &exact_def);\
	if (AllocSharedColor(display, colormap, &exact_def, format) == XcmsFailure)\
	{\
	    printColorName(display, colormap, buf, CSpace,\
			   exact_def.spec.RGBi.red,\
		           exact_def.spec.RGBi.green,\
		           exact_def.spec.RGBi.blue);\
	    fprintf(stderr, "Error in createLinearColorScaleFromPix: "\
		    "failed to allocate color %s\n", buf);\
	    decrColorScaleRefCount(newscale);\
	    goto fail;\
	}\
	newscale->pixels[i] = exact_def.pixel;\
	newscale->ncolors = i+1;\
    }\
} while(0);

#define alloc_xcms_colorscale(uname, red, green, blue) do {\
    dx = (end_def.spec.uname.red - start_def.spec.uname.red)/ncolors;\
    dy = (end_def.spec.uname.green - start_def.spec.uname.green)/ncolors;\
    dz = (end_def.spec.uname.blue - start_def.spec.uname.blue)/ncolors;\
    for (i=0; i<ncolors; ++i)\
    {\
	if (i == 0)\
	    exact_def = start_def;\
	else if (i == ncolors-1)\
	    exact_def = end_def;\
	else\
	{\
	    exact_def.spec.uname.red = start_def.spec.uname.red + dx*(i+0.5);\
	    exact_def.spec.uname.green = start_def.spec.uname.green + dy*(i+0.5);\
	    exact_def.spec.uname.blue = start_def.spec.uname.blue + dz*(i+0.5);\
	}\
	if (AllocSharedColor(display, colormap, &exact_def, format) == XcmsFailure)\
	{\
	    fprintf(stderr, "Error in createLinearColorScaleFromPix: "\
		    "color allocation failed\n");\
	    decrColorScaleRefCount(newscale);\
	    goto fail;\
	}\
        newscale->pixels[i] = exact_def.pixel;\
        newscale->ncolors = i+1;\
    }\
} while(0);

static int buildLinearScale(ColorScale *newscale, int ncolors)
{
    int i;
    Display *display = newscale->display;
    Colormap colormap = newscale->colormap;
    ColorSpace CSpace = newscale->colorspace;
    XcmsColorFormat format = ColorSpaceData[newscale->colorspace].format;
    XcmsColor start_def, end_def, exact_def;
    XcmsFloat dx, dy, dz;
    XcmsFloat h, s, v, h_begin, h_end, s_begin, s_end, v_begin, v_end;
    char buf[64];

    assert(newscale);
    assert(ncolors >= 2 && ncolors <= max_shade_colors);

    /* Look up the starting and ending color definitions */
    start_def.format = format;
    start_def.pixel = newscale->startPixel;
    XcmsQueryColor(display, colormap, &start_def, format);
    end_def.pixel = newscale->endPixel;
    XcmsQueryColor(display, colormap, &end_def, format);

    /* Assign the scale colors */
    exact_def.format = format;
    switch (CSpace)
    {
    case HLS:
	alloc_cspace_colorscale(HLS);
	break;
    case HSV:
	alloc_cspace_colorscale(HSV);
	break;
    case RGB:
    case RGBI:
	alloc_xcms_colorscale(RGBi, red, green, blue);
	break;
    case CIEXYZ:
	alloc_xcms_colorscale(CIEXYZ, X, Y, Z);
	break;
    case CIEUVY:
	alloc_xcms_colorscale(CIEuvY, u_prime, v_prime, Y);
	break;
    case CIEXYY:
	alloc_xcms_colorscale(CIExyY, x, y, Y);
	break;
    case CIELAB:
	alloc_xcms_colorscale(CIELab, L_star, a_star, b_star);
	break;
    case CIELUV:
	alloc_xcms_colorscale(CIELuv, L_star, u_star, v_star);
	break;
    case TEKHVC:
	alloc_xcms_colorscale(TekHVC, H, V, C);
	break;
    default:
	assert(0);
    }
    newscale->isLinear = 1;
    return 0;
 fail:
    return 1;
}

const ColorScale *createLinearColorScale(
    const char *name, Display *display, Colormap colormap,
    ColorSpace CSpace, int ncolors,
    const char *startColor_in, const char *endColor_in,
    const char *underflowColor_in, const char *overflowColor_in)
{
    const char *startColor, *endColor, *underflowColor, *overflowColor;
    XcmsColor start_def, end_def, over_def, under_def, exact_def, screen_def;
    XcmsColorFormat format;

    assert(name);
    assert(display);

    /* Check the color space argument */
    if (CSpace >= N_COLOR_SPACES)
    {
	fprintf(stderr, "Error in createLinearColorScale: bad color space\n");
	return NULL;
    }
    format = ColorSpaceData[CSpace].format;

    /* Must have at least 2 colors in the scale */
    if (ncolors < 2 || ncolors > max_shade_colors)
    {
	fprintf(stderr, "Error in createLinearColorScale: "
		"bad ncolors argument %d, "
		"must be between 2 and %d\n",
		ncolors, max_shade_colors);
	return NULL;
    }

    /* Normalize the color names in order to recognize HLS and HSV
     * colors. The function also converts empty strings to NULLs.
     */
    normalize_color_names(
	startColor_in, endColor_in, underflowColor_in, overflowColor_in,
	&startColor,   &endColor,   &underflowColor,   &overflowColor);

    /* Check that all colors are present */
    if (startColor == NULL)
    {
	fprintf(stderr, "Error in createLinearColorScale: "
		"starting color nor specified\n");
	return NULL;
    }
    check_color(startColor);
    start_def = exact_def;

    if (endColor == NULL)
    {
	fprintf(stderr, "Error in createLinearColorScale: "
		"ending color nor specified\n");
	return NULL;
    }
    check_color(endColor);
    end_def = exact_def;

    if (underflowColor == NULL)
    {
	fprintf(stderr, "Error in createLinearColorScale: "
		"underflow color nor specified\n");
	return NULL;
    }
    check_color(underflowColor);
    under_def = exact_def;

    if (overflowColor == NULL)
    {
	fprintf(stderr, "Error in createLinearColorScale: "
		"overflow color nor specified\n");
	return NULL;
    }
    check_color(overflowColor);
    over_def = exact_def;

    alloc_color(underflowColor, under_def);
    alloc_color(overflowColor, over_def);
    alloc_color(startColor, start_def);
    alloc_color(endColor, end_def);

    return createLinearColorScaleFromPix(
	name, display, colormap, CSpace, ncolors,
	start_def.pixel, end_def.pixel,
	under_def.pixel, over_def.pixel);
}

const ColorScale *createLinearColorScaleFromPix(
    const char *name, Display *display, Colormap colormap,
    ColorSpace CSpace, int ncolors,
    Pixel startPixel, Pixel endPixel,
    Pixel underflowPixel, Pixel overflowPixel)
{
    int pos, oldrefcount = 0, oldmodified;
    ColorScale *newscale;

    assert(name);
    assert(display);

    /* Check the color space argument */
    if (CSpace >= N_COLOR_SPACES)
    {
	fprintf(stderr, "Error in createLinearColorScale: bad color space\n");
	return NULL;
    }

    /* Must have at least 2 colors in the scale */
    if (ncolors < 2 || ncolors > max_shade_colors)
    {
	fprintf(stderr, "Error in createLinearColorScale: "
		"bad ncolors argument %d, "
		"must be between 2 and %d\n",
		ncolors, max_shade_colors);
	return NULL;
    }

    /* Check if a color scale with the given name already exists */
    newscale = (ColorScale *)findColorScale(name);
    if (newscale)
    {
	/* Clear the existing scale */
	pos = newscale->index;
	freeUpAllColors(newscale);
	if (newscale->pixels) free(newscale->pixels);
	if (newscale->name) free(newscale->name);
	oldrefcount = newscale->referenceCounter;
	oldmodified = newscale->version;
	memset(newscale, 0, sizeof(ColorScale));
	newscale->version = oldmodified + 1;
    }
    else
    {
	/* Create a new scale */
	pos = newScalePosition();
	if (pos < 0)
	    return NULL;
	newscale = (ColorScale *)calloc(1, sizeof(ColorScale));
	if (newscale == NULL)
	{
	    fprintf(stderr, "Out of memory!\n");
	    return NULL;
	}
    }
    scaleSet[pos] = newscale;
    newscale->index = pos;
    newscale->display = display;
    newscale->colormap = colormap;
    newscale->colorspace = CSpace;
    newscale->name = strdup(name);
    newscale->pixels = (Pixel *)malloc(ncolors*sizeof(Pixel));
    if (newscale->pixels == NULL || newscale->name == NULL)
    {
	fprintf(stderr, "Out of memory!\n");
	decrColorScaleRefCount(newscale);
	return NULL;
    }
    newscale->startPixel = startPixel;
    newscale->endPixel = endPixel;
    newscale->underflowPixel = underflowPixel;
    newscale->overflowPixel = overflowPixel;

    if (buildLinearScale(newscale, ncolors))
	return NULL;
    newscale->referenceCounter = oldrefcount;
    return newscale;
}

#define reallocatePixel(whichpixel) do {\
    def.pixel = whichpixel;\
    XcmsQueryColor(display, colormap, &def, format);\
    if (AllocSharedColor(display, colormap, &def, format) == XcmsFailure)\
    {\
	FreeSharedColors(display, colormap, newpixels, n_new_pixels, 0);\
	return NULL;\
    }\
    newpixels[n_new_pixels++] = def.pixel;\
} while(0);

const ColorScale * cloneColorScale(const char *newname,
				   const ColorScale *oldscale)
{
    int i;
    const ColorScale *newscale;
    Display *display = oldscale->display;
    Colormap colormap = oldscale->colormap;
    XcmsColorFormat format = ColorSpaceData[oldscale->colorspace].format;
    XcmsColor def;
    Pixel *newpixels;
    int n_new_pixels = 0;

    assert(newname);
    assert(oldscale);
    if (oldscale->index >= 0 && strcmp(newname, oldscale->name) == 0)
	return oldscale;
    newpixels = (Pixel *)malloc(sizeof(Pixel)*(oldscale->ncolors+4));
    if (newpixels == NULL)
    {
	fprintf(stderr, "Error in cloneColorScale: out of memory!\n");
	return NULL;
    }

    /* Reallocate the scale defining pixels */
    reallocatePixel(oldscale->startPixel);
    reallocatePixel(oldscale->endPixel);
    reallocatePixel(oldscale->underflowPixel);
    reallocatePixel(oldscale->overflowPixel);

    /* The following code is suboptimal: it builds
     * a linear scale first, and then modifies things.
     * Should rewrite in the future...
     */
    newscale = createLinearColorScaleFromPix(
	newname, display, colormap,
	oldscale->colorspace, oldscale->ncolors,
	newpixels[0], newpixels[1],
	newpixels[2], newpixels[3]);
    if (newscale && !oldscale->isLinear)
	for (i=0; i<oldscale->ncolors; ++i)
	{
	    reallocatePixel(getScalePixelAt(oldscale, i));
	    setScalePixel(newscale, i, newpixels[n_new_pixels-1]);
	}
    if (oldscale->index < 0)
    {
	/* We are restoring something from backup */
	((ColorScale *)newscale)->version = oldscale->version;
    }

    free(newpixels);
    return newscale;
}

Pixel getScalePixelAt(const ColorScale *scale, int num)
{
    if (num < 0)
	return scale->underflowPixel;
    else if (num >= scale->ncolors)
	return scale->overflowPixel;
    else
	return scale->pixels[num];
}

void linearizeColorScale(const ColorScale *s)
{
    ColorScale *scale = (ColorScale *)s;
    int ncolors = s->ncolors;

    assert(scale->index >= 0);
    if (scale->isLinear)
	return;
    if (ncolors > 0)
    {
	FreeSharedColors(scale->display, scale->colormap,
			 scale->pixels, ncolors, 0);
	scale->ncolors = 0;
    }
    buildLinearScale(scale, ncolors);
    ++scale->version;
}

void printColorName(Display *display, Colormap colormap,
		    char *buffer, ColorSpace s,
		    double red, double green, double blue)
{
    XcmsFloat a, b, c;
    char rgbicolor[32];
    XcmsColor exact_def, screen_def;
    XcmsColorFormat format;

    assert(buffer);
    assert(s < N_COLOR_SPACES);
    if (red < 0.0) red = 0.0;
    else if (red > 1.0) red = 1.0;
    if (green < 0.0) green = 0.0;
    else if (green > 1.0) green = 1.0;
    if (blue < 0.0) blue = 0.0;
    else if (blue > 1.0) blue = 1.0;

    if (s == RGB)
    {
	/* Special case, print as hex */
	sprintf(buffer, "#%04x%04x%04x", (unsigned int)(red*0xffff),
		(unsigned int)(green*0xffff), (unsigned int)(blue*0xffff));
	return;
    }
    sprintf(rgbicolor, "RGBi:%f/%f/%f", red, green, blue);
    if (s == RGBI)
    {
	/* Special case, already have the correct result */
	strcpy(buffer, rgbicolor);
	return;
    }

    /* Print the color space name into the buffer */
    strcpy(buffer, ColorSpaceData[s].name);
    strcat(buffer, ":");
    buffer += strlen(buffer);

    /* Lookup the color */
    format = ColorSpaceData[s].format;
    switch (s)
    {
    case CIEXYZ:
	XcmsLookupColor(display, colormap, rgbicolor,
			&exact_def, &screen_def, format);
	a = exact_def.spec.CIEXYZ.X;
	b = exact_def.spec.CIEXYZ.Y;
	c = exact_def.spec.CIEXYZ.Z;
	break;
    case CIEUVY:
	XcmsLookupColor(display, colormap, rgbicolor,
			&exact_def, &screen_def, format);
	a = exact_def.spec.CIEuvY.u_prime;
	b = exact_def.spec.CIEuvY.v_prime;
	c = exact_def.spec.CIEuvY.Y;
	break;
    case CIEXYY:
	XcmsLookupColor(display, colormap, rgbicolor,
			&exact_def, &screen_def, format);
	a = exact_def.spec.CIExyY.x;
	b = exact_def.spec.CIExyY.y;
	c = exact_def.spec.CIExyY.Y;
	break;
    case CIELAB:
	XcmsLookupColor(display, colormap, rgbicolor,
			&exact_def, &screen_def, format);
	a = exact_def.spec.CIELab.L_star;
	b = exact_def.spec.CIELab.a_star;
	c = exact_def.spec.CIELab.b_star;
	break;
    case CIELUV:
	XcmsLookupColor(display, colormap, rgbicolor,
			&exact_def, &screen_def, format);
	a = exact_def.spec.CIELuv.L_star;
	b = exact_def.spec.CIELuv.u_star;
	c = exact_def.spec.CIELuv.v_star;
	break;
    case TEKHVC:
	XcmsLookupColor(display, colormap, rgbicolor,
			&exact_def, &screen_def, format);
	a = exact_def.spec.TekHVC.H;
	b = exact_def.spec.TekHVC.V;
	c = exact_def.spec.TekHVC.C;
	break;
    case HSV:
	exact_def.spec.RGBi.red   = red;
	exact_def.spec.RGBi.green = green;
	exact_def.spec.RGBi.blue  = blue;
	RGBi_to_HSV(&exact_def, &a, &b, &c);
	break;
    case HLS:
	exact_def.spec.RGBi.red   = red;
	exact_def.spec.RGBi.green = green;
	exact_def.spec.RGBi.blue  = blue;
	RGBi_to_HLS(&exact_def, &a, &b, &c);
	break;
    default:
	assert(0);
    }

    sprintf(buffer, "%f/%f/%f", (double)a, (double)b, (double)c);
}

void incrColorScaleRefCount(const ColorScale *s)
{
    ColorScale *scale = (ColorScale *)s;

    assert(scale);
#ifdef DEBUG_COLORSCALE_REFCOUNTS
    if (scale->index < 0)
	printf("incrColorScaleRefCount is called on backup scale \"%s\".\n", scale->name);
    else
	printf("incrColorScaleRefCount is called on scale \"%s\".\n", scale->name);
    printf("New reference counter value is %d.\n", scale->referenceCounter+1);
    printf("Color scales in use: %d.\n", numUsedColorScales());
    fflush(stdout);
#endif
    ++scale->referenceCounter;
}

void decrColorScaleRefCount(const ColorScale *s)
{
    /* Make sure we can destroy an incompletely constructed object */
    ColorScale *scale = (ColorScale *)s;
    if (scale == NULL) return;
#ifdef DEBUG_COLORSCALE_REFCOUNTS
    if (scale->index < 0)
	printf("decrColorScaleRefCount is called on backup scale \"%s\".\n", scale->name);
    else
	printf("decrColorScaleRefCount is called on scale \"%s\".\n", scale->name);
    printf("New reference counter value is %d.\n", scale->referenceCounter-1);
#endif
    if (--scale->referenceCounter <= 0)
    {
	if (scale->name) free(scale->name);
	freeUpAllColors(scale);
	if (scale->pixels) free(scale->pixels);
	if (scale->index >= 0)
	    scaleSet[scale->index] = NULL;
	free(scale);
    }
#ifdef DEBUG_COLORSCALE_REFCOUNTS
    printf("Color scales in use: %d.\n", numUsedColorScales());
    fflush(stdout);
#endif
}

Pixel getStartPixel(const ColorScale *scale)
{
    return scale->startPixel;
}

Pixel getEndPixel(const ColorScale *scale)
{
    return scale->endPixel;
}

void setStartPixel(const ColorScale *s, Pixel pixel)
{
    ColorScale *scale = (ColorScale *)s;

    assert(scale->index >= 0);
    FreeSharedColors(scale->display, scale->colormap,
		&scale->startPixel, 1, 0);
    if (scale->startPixel != pixel || !scale->isLinear)
    {
	scale->startPixel = pixel;
	scale->isLinear = 0;
	linearizeColorScale(scale);
    }
}

void setEndPixel(const ColorScale *s, Pixel pixel)
{
    ColorScale *scale = (ColorScale *)s;

    assert(scale->index >= 0);
    FreeSharedColors(scale->display, scale->colormap,
		&scale->endPixel, 1, 0);
    if (scale->endPixel != pixel || !scale->isLinear)
    {
	scale->endPixel = pixel;
	scale->isLinear = 0;
	linearizeColorScale(scale);
    }
}

void setNumColors(const ColorScale *s, int ncolors)
{
    ColorScale *scale = (ColorScale *)s;

    assert(scale->index >= 0);
    assert(ncolors >= 2 && ncolors <= max_shade_colors);
    if (ncolors != scale->ncolors || !scale->isLinear)
    {
	scale->isLinear = 0;
	FreeSharedColors(scale->display, scale->colormap,
			 scale->pixels, scale->ncolors, 0);
	if (ncolors != scale->ncolors)
	{
	    if (scale->pixels)
		free(scale->pixels);
	    scale->pixels = (Pixel *)malloc(ncolors*sizeof(Pixel));
	    if (scale->pixels == NULL)
	    {
		fprintf(stderr, "Out of memory. Can't recover. Aborting.\n");
		fflush(stderr);
		abort();
	    }
	}
	scale->ncolors = 0;
	buildLinearScale(scale, ncolors);
	++scale->version;
    }
}

void setColorSpace(const ColorScale *s, ColorSpace CSpace)
{
    ColorScale *scale = (ColorScale *)s;

    assert(scale->index >= 0);
    if (CSpace >= N_COLOR_SPACES)
    {
	fprintf(stderr, "Error in setColorSpace: bad color space argument\n");
	return;
    }
    if (scale->colorspace != CSpace || !scale->isLinear)
    {
	scale->isLinear = 0;
	scale->colorspace = CSpace;
	linearizeColorScale(scale);
    }
}

void setScalePixel(const ColorScale *s, int num, Pixel pixel)
{
    ColorScale *scale = (ColorScale *)s;

    assert(scale->index >= 0);
    /* Have to deallocate colors */
    if (num < 0)
    {
	FreeSharedColors(scale->display, scale->colormap,
			 &scale->underflowPixel, 1, 0);
	scale->underflowPixel = pixel;
    }
    else if (num >= scale->ncolors)
    {
	FreeSharedColors(scale->display, scale->colormap,
			 &scale->overflowPixel, 1, 0);
	scale->overflowPixel = pixel;
    }
    else
    {
	FreeSharedColors(scale->display, scale->colormap,
			 &scale->pixels[num], 1, 0);
	if (scale->pixels[num] != pixel)
	{
	    scale->isLinear = 0;
	    scale->pixels[num] = pixel;
	}
    }
    ++scale->version;
}

Pixel getScalePixel(float fraction, const ColorScale *scale)
{
    int index;

    if (fraction < 0.f)
	return scale->underflowPixel;
    else if (fraction > 1.f)
	return scale->overflowPixel;
    else
    {
	index = (int)(fraction*scale->ncolors);
	if (index >= scale->ncolors)
	    index = scale->ncolors-1;
	return scale->pixels[index];
    }
}

const char *colorSpaceName(ColorSpace c)
{
    if (c >= N_COLOR_SPACES)
	return NULL;
    return ColorSpaceData[c].name;
}

int colorSpaceNumber(const char *name)
{
    int i;
    if (name)
	for (i=0; i<N_COLOR_SPACES; ++i)
	    if (strcasecmp(name, ColorSpaceData[i].name) == 0)
		return i;
    return -1;
}

void unusedColorScaleName(char *buffer)
{
    static int scale_number = 0;
    sprintf(buffer, "color_scale_%d", scale_number++);
    while (findColorScale(buffer))
	sprintf(buffer, "color_scale_%d", scale_number++);
}
