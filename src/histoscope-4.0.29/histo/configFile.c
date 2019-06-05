/*******************************************************************************
*									       *
* configFile.c -- Load and save configuration files 			       *
*									       *
* Copyright (c) 1995 Universities Research Association, Inc.		       *
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
* April 7, 1995								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Heavily modified by igv in 2002-2003.		                               *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#if XmVersion >= 1002
#include <Xm/PrimitiveP.h>
#endif
#include "../util/DialogF.h"
#include "../util/psUtils.h"
#include "../histo_util/hsTypes.h"
#include "../histo_util/histoUtil.h"
#include "../plot_widgets/enableArrows.h"
#include "../plot_widgets/H1DP.h" 
#include "../plot_widgets/ScatP.h"
#include "../plot_widgets/XY.h"
#include "../plot_widgets/XYP.h"
#include "../plot_widgets/XYDialogs.h"
#include "../plot_widgets/CellP.h"
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/3DScat.h"
#include "histoP.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "ntuplePanel.h"
#include "interpret.h"
#include "variablePanel.h"
#include "parse.h"
#include "auxWindows.h"
#include "mainMenu.h"
#include "preferences.h"
#include "multPlot.h"
#include "communications.h"
#include "defaultColorScale.h"
#include "configFile.h"

/* Maximum lengths for keyword and argument strings */
#define MAX_GEOMETRY_LEN 24
#define MAX_KEYWORD_LEN 25
#define MAX_UID_LEN 20
#define MAX_FLOAT_LEN 20
#define MAX_CONFIG_LINE_LEN 4096
#define MAX_PLOTTYPE_LEN 30
#define MAX_COLOR_LEN 64
#define MAX_COLORSPACE_LEN 8
#define MAX_FONTNAME_LEN 100

/* Maximum length for lines written by SaveConfigFile before wrapping */
#define MAX_SAVE_LINE_LEN 78

/* Largest number of multi-plot windows that could ever conceivably be used */
#define MAX_MULTI_PLOT_WINDOWS 1000

/* Require PostScript font specification 
   whenever X windows font is specified ? */
#define REQUIRE_PSFONT_SPEC

/* Default alignment for comments and EPS files */
#define DEFAULT_ALIGNMENT ALIGN_SW

/* Integer indecies representing each keyword and heading */
enum keyIndicies {
    CATEGORY_INDEX = 0,
    UID_INDEX,
    NAME_INDEX,
    GEOMETRY_INDEX,
    XMINLIMIT_INDEX,
    XMAXLIMIT_INDEX,
    YMINLIMIT_INDEX,
    YMAXLIMIT_INDEX,
    ZMINLIMIT_INDEX,
    ZMAXLIMIT_INDEX,
    LOGX_INDEX,
    LOGY_INDEX,
    LOGZ_INDEX,
    DARKEN_INDEX,
    BINEDGE_INDEX,
    BACKPLANES_INDEX,
    HIDELEGEND_INDEX,
    GAUSSERROR_INDEX,
    DATAERROR_INDEX,
    SPLITINHALF_INDEX,
    CNTROFGRAV_INDEX,
    GROWONLY_INDEX,
    FI_INDEX,
    PSI_INDEX,
    ALPHA_INDEX,
    BETA_INDEX,
    GAMMA_INDEX,
    PLOTTYPE_INDEX,
    VAR_INDEX,
    EXPRNAME_INDEX,
    EXPR_INDEX,
    SLIDER_INDEX,
    SLIDERWIN_INDEX,
    REBINWIN_INDEX,
    CELLNORMWIN_INDEX,
    STATSWIN_INDEX,
    SLIDERLT_INDEX,
    SLIDERGT_INDEX,
    NBINS_INDEX,
    NXBINS_INDEX,
    NYBINS_INDEX,
    BINLIMIT_INDEX,
    NORMMIN_INDEX,
    NORMMAX_INDEX,
    MARKSTYLE_INDEX,
    LINESTYLE_INDEX,
    FILLSTYLE_INDEX,
    MARKSIZE_INDEX,
    MARKCOLOR_INDEX,
    LINECOLOR_INDEX,
    FILLCOLOR_INDEX,
    INMULTIPLOT_INDEX,
    INOVERLAY_INDEX,
    ROWS_INDEX,
    COLUMNS_INDEX,
    ROW_INDEX,
    COLUMN_INDEX,
    WINDOWID_INDEX,
    HIDETITLES_INDEX,
    WINDOWNAME_INDEX,
    OVERLAY_INDEX,
    HBOOKID_INDEX,
    WINDOWLOC_INDEX,
    WINDOWACT_INDEX,
    NOCOMPLAIN_INDEX,
    XLABEL_INDEX,
    YLABEL_INDEX,
    COLORSPACE_INDEX,
    NCOLORS_INDEX,
    STARTCOLOR_INDEX,
    ENDCOLOR_INDEX,
    UNDERCOLOR_INDEX,
    OVERCOLOR_INDEX,
    LINEARCOLOR_INDEX,
    PIXELCOLOR_INDEX,
    COLORSCALEPERSIST_INDEX,
    COLORSCALENAME_INDEX,
    DYNAMICCOLOR_INDEX,
    UPDATEFREQUENCY_INDEX,
    MINPSLINEWIDTH_INDEX,
    BUFFERGRAPHICS_INDEX,
    THICKENSCAT_INDEX,
    THICKENSCAT3D_INDEX,
    AUTOMATICHELP_INDEX,
    TASKNUMBER_INDEX,
    FONT_INDEX,
    PSFONT_INDEX,
    PSFONTSIZE_INDEX,
    ALIGNMENT_INDEX,
    FGCOLOR_INDEX,
    BGCOLOR_INDEX,
    BORDER_INDEX,
    ARROWSTYLE_INDEX,
    REFPOINT_INDEX,
    DRAWPOINT_INDEX,
    NOREDRAW_INDEX,
    MARGINLEFT_INDEX,
    MARGINRIGHT_INDEX,
    MARGINTOP_INDEX,
    MARGINBOTTOM_INDEX,
    JUSTIFYTEXT_INDEX,
    MAGNIFICATION_INDEX,
    UNLINK_INDEX,
    PRINTCOORDS_INDEX,
    ERRMARKER_INDEX,
    LEFTARROWS_INDEX,
    RIGHTARROWS_INDEX,
    TOPARROWS_INDEX,
    BOTTOMARROWS_INDEX,
    NKEYWORDS
};

/* Strings for keyword names */	
#define CATEGORY_KEY "Category"
#define UID_KEY "UID"
#define NAME_KEY "Title"
#define HBOOKID_KEY "HbookID"
#define WINDOWNAME_KEY "WindowTitle"
#define GEOMETRY_KEY "WindowGeometry"
#define XMINLIMIT_KEY "XMinLimit"
#define XMAXLIMIT_KEY "XMaxLimit"
#define YMINLIMIT_KEY "YMinLimit"
#define YMAXLIMIT_KEY "YMaxLimit"
#define ZMINLIMIT_KEY "ZMinLimit"
#define ZMAXLIMIT_KEY "ZMaxLimit"
#define LOGX_KEY "LogX"
#define LOGY_KEY "LogY"
#define LOGZ_KEY "LogZ"
#define DARKEN_KEY "DarkenPoints"
#define BINEDGE_KEY "LabelBinEdges"
#define BACKPLANES_KEY "HideBackplanes"
#define HIDELEGEND_KEY "HideLegend"
#define GAUSSERROR_KEY "GaussianErrors"
#define DATAERROR_KEY "ShowErrorData"
#define SPLITINHALF_KEY "SplitInHalfStrategy"
#define CNTROFGRAV_KEY "CenterOfGravStrategy"
#define GROWONLY_KEY "GrowOnly"
#define FI_KEY "Fi"
#define PSI_KEY "Psi"
#define ALPHA_KEY "Alpha"
#define BETA_KEY "Beta"
#define GAMMA_KEY "Gamma"
#define PLOTTYPE_KEY "PlotType"
#define VAR_KEY "V%"
#define EXPRNAME_KEY "Name%"
#define EXPR_KEY "Expr%"
#define SLIDER_KEY "S%"
#define SLIDERWIN_KEY "SliderWindow"
#define REBINWIN_KEY "RebinWindow"
#define CELLNORMWIN_KEY "CellNormalizeWindow"
#define STATSWIN_KEY "StatisticsWindow"
#define SLIDERLT_KEY "Slider<Threshold%"
#define SLIDERGT_KEY "Slider>Threshold%"
#define NBINS_KEY "NumberOfBins"
#define NXBINS_KEY "NumberOfXBins"
#define NYBINS_KEY "NumberOfYBins"
#define BINLIMIT_KEY "BinLimit"
#define NORMMIN_KEY "NormalizationMin"
#define NORMMAX_KEY "NormalizationMax"
#define MARKSTYLE_KEY "MarkerStyle%"
#define LINESTYLE_KEY "LineStyle%"
#define FILLSTYLE_KEY "FillStyle%"
#define MARKSIZE_KEY "MarkerSize%"
#define MARKCOLOR_KEY "MarkerColor%"
#define LINECOLOR_KEY "LineColor%"
#define FILLCOLOR_KEY "FillColor%"
#define INMULTIPLOT_KEY "InMultiPlot"
#define INOVERLAY_KEY "InOverlay"
#define ROWS_KEY "Rows"
#define COLUMNS_KEY "Columns"
#define ROW_KEY "Row"
#define COLUMN_KEY "Column"
#define WINDOWID_KEY "WindowID"
#define OVERLAY_KEY "OverlayID"
#define HIDETITLES_KEY "HideTitles"
#define WINDOWLOC_KEY "WindowName"
#define WINDOWACT_KEY "Action"
#define NOCOMPLAIN_KEY "NoComplain"
#define XLABEL_KEY "XLabel"
#define YLABEL_KEY "YLabel"
#define COLORSPACE_KEY "ColorSpace"
#define NCOLORS_KEY "NumberOfColors"
#define STARTCOLOR_KEY "StartColor"
#define ENDCOLOR_KEY "EndColor"
#define UNDERCOLOR_KEY "UnderflowColor"
#define OVERCOLOR_KEY "OverflowColor"
#define LINEARCOLOR_KEY "LinearColorScale"
#define PIXELCOLOR_KEY "PixelColor"
#define COLORSCALEPERSIST_KEY "PersistentColorScale"
#define COLORSCALENAME_KEY "ColorScaleName"
#define DYNAMICCOLOR_KEY "DynamicColor"
#define UPDATEFREQUENCY_KEY "UpdateFrequency"
#define MINPSLINEWIDTH_KEY "MinPSLineWidth"
#define BUFFERGRAPHICS_KEY "BufferGraphics"
#define THICKENSCAT_KEY "ThickenPointsScat"
#define THICKENSCAT3D_KEY "ThickenPointsScat3D"
#define AUTOMATICHELP_KEY "AutomaticHelp"
#define TASKNUMBER_KEY "TaskNumber"
#define FONT_KEY "Font"
#define PSFONT_KEY "PSFont"
#define PSFONTSIZE_KEY "PSFontSize"
#define ALIGNMENT_KEY "Alignment"
#define FGCOLOR_KEY "Foreground"
#define BGCOLOR_KEY "Background"
#define BORDER_KEY "DrawBorder"
#define ARROWSTYLE_KEY "ArrowMode"
#define REFPOINT_KEY "ReferencePoint"
#define DRAWPOINT_KEY "Point%"
#define NOREDRAW_KEY "NoRedraw"
#define MARGINLEFT_KEY "AddToLeftMargin"
#define MARGINRIGHT_KEY "AddToRightMargin"
#define MARGINTOP_KEY "AddToTopMargin"
#define MARGINBOTTOM_KEY "AddToBottomMargin"
#define JUSTIFYTEXT_KEY "Justify"
#define MAGNIFICATION_KEY "Magnification"
#define UNLINK_KEY "Unlink"
#define PRINTCOORDS_KEY "PrintCoords"
#define ERRMARKER_KEY "ErrorMarkerRatio"
#define LEFTARROWS_KEY "DisplayLeftArrow"
#define RIGHTARROWS_KEY "DisplayRightArrow"
#define TOPARROWS_KEY "DisplayTopArrow"
#define BOTTOMARROWS_KEY "DisplayBottomArrow"

/*
** Collected plot scaling parameters, used in both the "values" structure
** for the parser, and in deferring scaling until after the data for a
** plot is loaded
*/
typedef struct {
    float xMinLimit, xMaxLimit, yMinLimit, yMaxLimit, zMinLimit, zMaxLimit;
    float angle1, angle2, angle3;
} plotScaleInfo;

/* Structure representing the complete contents of all keywords read from
   the configuration file under a single heading.  ReadConfigFile parses
   a heading and all of the keywords below it, depositing the results in
   a keywordValues structure (below), before passing it do displayPlot
   to draw the plot that it represents.  */
typedef struct {
    char category[HS_MAX_CATEGORY_LENGTH];
    char geometry[MAX_GEOMETRY_LEN];
    char name[HS_MAX_TITLE_LENGTH];
    char windowTitle[HS_MAX_WINDOW_TITLE_LENGTH];
    char keyword[MAX_KEYWORD_LEN];
    char xlabel[HS_MAX_LABEL_LENGTH];
    char ylabel[HS_MAX_LABEL_LENGTH];
    char zlabel[HS_MAX_LABEL_LENGTH];
    int uid;
    int hbookID;
    plotScaleInfo scale;
    int logX, logY, logZ;
    int plotType;
    int nXBins, nYBins;
    int row, col;
    int multiPlotID;
    int overlayID;
    float normMin, normMax;
    char vars[MAX_DISP_VARS][HS_MAX_NAME_LENGTH];
    char sliders[N_SLIDERS][HS_MAX_NAME_LENGTH];
    char specified[NKEYWORDS];
    char varsSpecified[MAX_DISP_VARS];
    char slidersSpecified[N_SLIDERS];
    char sliderThreshSpecified[N_SLIDERS];
    float sliderThresh[N_SLIDERS];
    char sliderGT[N_SLIDERS];
    char sliderWinGeom[MAX_GEOMETRY_LEN];
    char rebinWinGeom[MAX_GEOMETRY_LEN];
    char cellNormWinGeom[MAX_GEOMETRY_LEN];
    char statsWinGeom[MAX_GEOMETRY_LEN];
    userDefNTVar userVars[MAX_USER_VARS];
    int fillStyles[MAX_DISP_VARS];
    char markStyles[MAX_DISP_VARS];
    char lineStyles[MAX_DISP_VARS];
    char markSizes[MAX_DISP_VARS];
    char markColors[MAX_DISP_VARS][MAX_COLOR_LEN];
    char lineColors[MAX_DISP_VARS][MAX_COLOR_LEN];
    char fillColors[MAX_DISP_VARS][MAX_COLOR_LEN];
    char windowLoc[MAX_WINDOWID_LEN];
    char windowAct[MAX_CONFIG_LINE_LEN];
    int noComplain;
    const ColorScale *clrScale;
    char colorSpace[MAX_COLORSPACE_LEN];
    int ncolors;
    char startColor[MAX_COLOR_LEN];
    char endColor[MAX_COLOR_LEN];
    char underflowColor[MAX_COLOR_LEN];
    char overflowColor[MAX_COLOR_LEN];
    char pixelColors[MAX_COLORS_HIGHCOLOR][MAX_COLOR_LEN];
    char fontName[MAX_FONTNAME_LEN];
    char psFontName[MAX_FONTNAME_LEN];
    int psFontSize;
    int updateFrequency;
    float minPSlineWidth;
    int bufferGraphics;
    int thickenPointsScat;
    int thickenPointsScat3D;
    int autohelp;
    int taskNumber;
    int alignment;
    int arrowstyle;
    int addLeft, addRight,
        addTop, addBottom;
    int justify;
    float magnify;
    int printCoords;
    float errMarkerRatio;
    int enableLeftArr;
    int enableRightArr;
    int enableTopArr;
    int enableBottomArr;
    DrawingPoint refpoint;
    DrawingPoint points[MAX_DRAWING_POINTS];
    char pointsSpecified[MAX_DRAWING_POINTS];
} keywordValues;

/* structure for associating window IDs with multi-plot windows */
typedef struct {
    int id;
    multWindow *window;
} multiPlotIDRec;

typedef struct {
    int id;
    windowInfo *window;
} overlayIDRec;

/* holds scaling information for deferring scaling until after plot
   data can be loaded from the client */
typedef struct {
    Widget plotW;
    char keysUsed[NKEYWORDS];
    plotScaleInfo scale;
} deferredScaleInfo;

typedef int (HistoCommand)(char *cmd, int taskNumber, int argc,
			   char argv[][HS_MAX_NAME_LENGTH], char **errMsg);
typedef struct {
    char *name;
    HistoCommand *command;
} histoCommandList;

#define add_to_list(list,element) do {\
    element->next = list;\
    list = element;\
} while(0);

/* Function prototypes */
static void writeWindowConfig(FILE *fp, windowInfo *window, int multiWinID,
	int overlayID, int row, int col);
static void writePlotInfo(FILE *fp, Display *dpy, Colormap cmap, Pixel black,
			  plotInfo *pInfo, XYCurve *curveStyles, int nCurveStyles,
			  XYHistogram *histStyles, int nHistStyles);
static void writeWindowInfo(FILE *fp, windowInfo *window, int multiWinID,
	int overlayID, int row, int col);
static void writeMultiWinConfig(FILE *fp, multWindow *window, int id,
    	int *overlayID);
static void writeUsedColorScales(FILE *fp);
static void writeColorScale(const ColorScale *scale, FILE *fp);
static void writeOneScaleColor(const ColorScale *scale, Pixel pix, FILE *fp);
static void writeNonscaleColor(Display *dpy, Colormap cmap, Pixel pix, FILE *fp);
static void writeDrawingObject(Widget w, const DrawingObject *obj,
			       const char *winID, int row, int col, FILE *fp);
static void writeWindowComment(Widget w, const XYString *string,
			       const char *winID, int row, int col, FILE *fp);
static void writeDrawingPoint(const DrawingPoint *point, FILE *fp);
static void writeDrawingsAndComments(FILE *fp, windowInfo *window,
				     const char *winID, int row, int col);
static int readKeyword(char *line, keywordValues *values, char **errMsg);
static void patchValues(keywordValues *values);
static int displayPlot(Widget parent, int headingType, keywordValues *values,
	int headingLineNum, multiPlotIDRec *multiPlotIDs);
static int displayMultiPlotWindow(Widget parent, keywordValues *values,
	int headingLineNum, multiPlotIDRec *multiPlotIDs);
static int enterNtupleExtension(Widget parent, keywordValues *values,
	int headingLineNum);
static int prepareColorScale(Widget parent, keywordValues *values,
	int headingLineNum);
static int getPlotLimits(Widget plotW, int *xMinDefault, double *xMinLim,
	int *xMaxDefault, double *xMaxLim, int *yMinDefault, double *yMinLim,
	int *yMaxDefault, double *yMaxLim, int *zMinDefault, double *zMinLim,
	int *zMaxDefault, double *zMaxLim, int *angle1Default, double *angle1,
	int *angle2Default, double *angle2, int *angle3Default, double *angle3);
static void getLogSettings(Widget plotW, int *xLog, int *yLog, int *zLog);
static void setPlotScale(Widget plotW, char *keyUsed, plotScaleInfo *scale);
static int readGeom(char *line, char *geom, char **errMsg);
static int readCategory(char *line, char *category, char **errMsg);
static int readLabel(char *line, char *label, char **errMsg);
static int readName(char *line, char *name, char **errMsg);
static int readShortString(char *line, char *name, int len, char **errMsg);
static int readWinLoc(char *line, char *name, char **errMsg);
static int readPlotType(char *line, int *plotType, char **errMsg);
static int readVar(char *line, keywordValues *values, char **errMsg);
static int readNumberedPoint(char *line, keywordValues *values, char **errMsg);
static int readExprName(char *line, keywordValues *values, char **errMsg);
static int readExpr(char *line, keywordValues *values, char **errMsg);
static int readStyle(char *line, char *styleList, char **errMsg);
static int readFillStyle(char *line, int *styleList, char **errMsg);
static int readSlider(char *line, keywordValues *values, char **errMsg);
static int readFloatArg(char *line, float *result, char **errMsg);
static int readIntArg(char *line, int *value, char **errMsg);
static int readAlignmentArg(char *line, int *value, char **errMsg);
static int readCoordsType(char *line, unsigned char *value, char **errMsg, int get);
static int readPixelData(char *line, char pixelColors[][MAX_COLOR_LEN], char **errMsg);
static int readColorScaleName(char *line, const ColorScale **cscale, char **errMsg);
static int readDrawingPoint(char *line, DrawingPoint *pt, char **errMsg);
static int getArg(char *line, char **argStart, int required, int maxLength,
	char **errMsg);
static int getIndex(char *line, int maxIndex, char **errMsg);
static int convertVarNamesToIndecies(hsNTuple *ntuple, keywordValues *values,
	int *varList, int *sliderList, char **errVar);
static int lookupNtupleName(hsNTuple *ntuple, ntupleExtension *ntExt,
	char *name);
static void setMenuToggle(Widget w, int buttonState, int notify);
static int toggleSetting(Widget w, int defaultValue);
static int compareKeyword(char *line, const char *keyword);

static const char *getLine(char *line, const char *stream, int *lineNum, int maxLen);
static const char *sgets(char *buffer, size_t size, const char *stream);

static void createGeomString(Widget shell, char *geomString);
static int lineIsBlank(char *line);
static multWindow *lookUpMultiplot(multiPlotIDRec *multiPlotIDs, int id);
static void addMultiPlotID(multiPlotIDRec *multiPlotIDs, int id,
	multWindow *window);
static int readColor(char *line, char *colorList, char **errMsg);
static Pixel allocColor(char *colorName, Display *display, int screen_num);
static windowInfo *findOverlayWindow(int overlayID);
static void saveOverlayWindow(windowInfo *window, int overlayID);
static void clearOverlayList(void);
static char *fillStyleName(unsigned int style);
static void setPlotScaleDeferred(int id, Widget plotW, char *keyUsed,
    	plotScaleInfo *scale);
static void deferredPlotScalingCB(void *cbData);
static int readSliderThreshold(char *line, keywordValues *values,
			       int sliderGT, char **errMsg);
static void safeRedisplay(windowInfo *window);
static char *getFontName(Display *dpy, XmFontList font);
static XmFontListEntry getFontEntry(Display *dpy, char *fontName);
static HistoCommand executeHistoCommand;
static HistoCommand CloseAll_cmd;

/* Commands available through configuration string */
static const histoCommandList CommandList[] = {
    {"CloseAll", CloseAll_cmd}
};

/* Heading strings */
static const char *Headings[NHEADINGS] = {"Histogram", "2DHistogram", "Indicator",
	"Control", "Trigger", "NTupleItem", "CellPlot", "NTupleExtension",
	"MultiPlotWindow", "WindowAction", "ColorCellPlot", "ColorScale",
        "ConfigData", "HistoCommand", "3DHistogram"};

/* Histo-Scope item types corresponding to each heading type */
static const int HeadingItemTypes[NHEADINGS] = {HS_1D_HISTOGRAM, HS_2D_HISTOGRAM,
	HS_INDICATOR, HS_CONTROL, HS_TRIGGER, HS_NTUPLE, HS_2D_HISTOGRAM,
	HS_NTUPLE, 0, -1, HS_2D_HISTOGRAM, -2, -3, -4, HS_3D_HISTOGRAM};

static const char *coordTypeNames[N_COORD_TYPES] = {
    "winabs",
    "winrel",
    "plot"
};

static const char *pointRefTypeNames[N_POINT_REF_TYPES] = {
    "O", /* COORD_ORIGIN_REF */
    "L", /* LAST_POINT_REF   */
    "R"  /* REF_POINT_REF    */
};

static const char *alignmentNames[N_ALIGNMENT_TYPES] = {
    "NW", "NC", "NE",
    "CW", "CC", "CE",
    "SW", "SC", "SE"
};

/* Keyword strings */
static const char *Keywords[NKEYWORDS] = {
    CATEGORY_KEY,
    UID_KEY,
    NAME_KEY,
    GEOMETRY_KEY,
    XMINLIMIT_KEY,
    XMAXLIMIT_KEY,
    YMINLIMIT_KEY,
    YMAXLIMIT_KEY,
    ZMINLIMIT_KEY,
    ZMAXLIMIT_KEY,
    LOGX_KEY,
    LOGY_KEY,
    LOGZ_KEY,
    DARKEN_KEY,
    BINEDGE_KEY,
    BACKPLANES_KEY,
    HIDELEGEND_KEY,
    GAUSSERROR_KEY,
    DATAERROR_KEY,
    SPLITINHALF_KEY,
    CNTROFGRAV_KEY,
    GROWONLY_KEY,
    FI_KEY,
    PSI_KEY,
    ALPHA_KEY,
    BETA_KEY,
    GAMMA_KEY,
    PLOTTYPE_KEY,
    VAR_KEY,
    EXPRNAME_KEY,
    EXPR_KEY,
    SLIDER_KEY,
    SLIDERWIN_KEY,
    REBINWIN_KEY,
    CELLNORMWIN_KEY,
    STATSWIN_KEY,
    SLIDERLT_KEY,
    SLIDERGT_KEY,
    NBINS_KEY,
    NXBINS_KEY,
    NYBINS_KEY,
    BINLIMIT_KEY,
    NORMMIN_KEY,
    NORMMAX_KEY,
    MARKSTYLE_KEY,
    LINESTYLE_KEY,
    FILLSTYLE_KEY,
    MARKSIZE_KEY,
    MARKCOLOR_KEY,
    LINECOLOR_KEY,
    FILLCOLOR_KEY,
    INMULTIPLOT_KEY,
    INOVERLAY_KEY,
    ROWS_KEY,
    COLUMNS_KEY,
    ROW_KEY,
    COLUMN_KEY,
    WINDOWID_KEY,
    HIDETITLES_KEY,
    WINDOWNAME_KEY,
    OVERLAY_KEY,
    HBOOKID_KEY,
    WINDOWLOC_KEY,
    WINDOWACT_KEY,
    NOCOMPLAIN_KEY,
    XLABEL_KEY,
    YLABEL_KEY,
    COLORSPACE_KEY,
    NCOLORS_KEY,
    STARTCOLOR_KEY,
    ENDCOLOR_KEY,
    UNDERCOLOR_KEY,
    OVERCOLOR_KEY,
    LINEARCOLOR_KEY,
    PIXELCOLOR_KEY,
    COLORSCALEPERSIST_KEY,
    COLORSCALENAME_KEY,
    DYNAMICCOLOR_KEY,
    UPDATEFREQUENCY_KEY,
    MINPSLINEWIDTH_KEY,
    BUFFERGRAPHICS_KEY,
    THICKENSCAT_KEY,
    THICKENSCAT3D_KEY,
    AUTOMATICHELP_KEY,
    TASKNUMBER_KEY,
    FONT_KEY,
    PSFONT_KEY,
    PSFONTSIZE_KEY,
    ALIGNMENT_KEY,
    FGCOLOR_KEY,
    BGCOLOR_KEY,
    BORDER_KEY,
    ARROWSTYLE_KEY,
    REFPOINT_KEY,
    DRAWPOINT_KEY,
    NOREDRAW_KEY,
    MARGINLEFT_KEY,
    MARGINRIGHT_KEY,
    MARGINTOP_KEY,
    MARGINBOTTOM_KEY,
    JUSTIFYTEXT_KEY,
    MAGNIFICATION_KEY,
    UNLINK_KEY,
    PRINTCOORDS_KEY,
    ERRMARKER_KEY,
    LEFTARROWS_KEY,
    RIGHTARROWS_KEY,
    TOPARROWS_KEY,
    BOTTOMARROWS_KEY
};

/* Histogram fill styles (must agree with XY.h and XYDialogs.c) */
#define N_FILL_STYLES 39
static struct {
    char *name;
    unsigned int style;
} HistFillStyles[N_FILL_STYLES] = {
	{"none", 0}, {"solid", 0x000000ff},{"fineHoriz", 0x01008004},
	{"coarseHoriz", 0x0100800b}, {"fineVert", 0x01800004},
	{"coarseVert", 0x0180000b}, {"fineGrid", 0x01808005},
	{"coarseGrid", 0x01808010}, {"fineX", 0x01c04006},
	{"coarseX", 0x01c04010}, {"fine45deg", 0x01004005},
	{"med45deg", 0x01004007}, {"coarse45deg", 0x01004010},
	{"fine30deg", 0x01005506}, {"coarse30deg", 0x01005510},
	{"fine60deg", 0x01550006}, {"coarse60deg", 0x01550010},
	{"rFine45deg", 0x0100c005}, {"rMed45deg", 0x0100c007},
	{"rCoarse45deg", 0x0100c010}, {"rFine30deg", 0x0100a506},
	{"rCoarse30deg", 0x0100a510}, {"rFine60deg", 0x01a50006},
	{"rCoarse60deg", 0x01a50010}, {"lFineHoriz", 0x02008004},
	{"lCoarseHoriz", 0x0200800b}, {"lFineVert", 0x02800004},
	{"lCoarseVert", 0x0280000b}, {"lFineGrid", 0x02808005},
	{"lCoarseGrid", 0x02808010}, {"lFineX", 0x02c04006},
	{"lCoarseX", 0x02c04010}, {"lFine45deg", 0x02004005},
	{"lMed45deg", 0x02004007}, {"lCoarse45deg", 0x02004010},
	{"lFine30deg", 0x02005506}, {"lCoarse30deg", 0x02005510},
	{"lFine60deg", 0x02550006}, {"lCoarse60deg", 0x02550010}
};


/* NTuple plot type strings.  Note these names must parallel the enum
   ntuplePlotTypes in plotWindows.h */
static char *PlotTypeNames[N_PLOT_TYPES] = {"TimeSeriesPlot",
	"TSPlotWithErrors", "XYPlot", "XYPlotWithErrors", "SortedXY",
	"SortedXYWithErrors", "XYScatterPlot", "XYZScatterPlot", "1DHistogram",
	"2DHistogram", "1DAdaptiveHistogram", "2DAdaptiveHistogram",
	"CellPlot", "ColorXYScatterPlot", "ColorCellPlot"};

/* List corresponding overlay id's with windows which can have plots layered
   upon them */
static overlayIDRec *Overlays = NULL;
static int NOverlays = 0;

/*
** Save a configuration file from the current state of this Histo-Scope
** session.  Goes through all of the plot windows in WindowList, and writes
** configuration information to the file "filename".  On error, displays
** a dialog (using argument "parent" as the parent widget) and returns False.
*/
int SaveConfigFile(Widget parent, char *filename)
{
    FILE *fp;
    windowInfo *w;
    multWindow *mw;
    int multWindowID = 1;
    int overlayID = 1;

    /* Open the file */
    fp = fopen(filename, "w");
    if (fp == NULL) {
    	DialogF(DF_ERR, parent, 1, "Couldn't open %s", "OK", filename);
    	return False;
    }

    /* If there are any ntuple extensions, write them first */
    WriteNTupleExtensions(fp);

    /* Write out color scales used in plots shown */
    writeUsedColorScales(fp);

    /* Go through the list of displayed windows, writing their descriptions
       on the configuration file */
    for (w=WindowList; w!=NULL; w=w->next) {
    	if (!w->multPlot) {
    	    writeWindowConfig(fp, w, 0, overlayID, 0, 0);
    	    if (w->nPlots > 1)
    	    	overlayID++;
	    writeDrawingsAndComments(fp, w, w->windowID, -1, -1);
    	}
    }

    /* write the descriptions of the multi-plot windows */
    for (mw=MultWindowList; mw!= NULL; mw=mw->next)
    	writeMultiWinConfig(fp, mw, multWindowID++, &overlayID);

    /* Close the file */
    fclose(fp);

    return True;
}

/*
** Read a Histo-Scope configuration file and put up windows.  If there's an
** error, display a dialog and continue or return depending on the user's
** choice and/or the severity of the error.
*/
int ReadConfigFile(Widget parent, char *filename)
{
    int status;
    const char *fp;

    fp = loadTextFile(filename);
    if (fp == NULL) {
    	perror("histo - Couldn't open configuration file");
    	return False;
    }
    status = ParseConfigBuffer(parent, fp);
    free((void *)fp);
    return status;
}

int ParseConfigBuffer(Widget parent, const char *fp)
{
    static keywordValues values;
    int lineNum = 0, eof = False;
    char line[MAX_CONFIG_LINE_LEN];
    char *errMsg;
    int i, headingIndex, plotOk, autoHelpSave, headingLineNum;
    multiPlotIDRec multiPlotWins[MAX_MULTI_PLOT_WINDOWS+1];

    /* Open the file */
    if (fp == NULL)
	return False;

    /* Initialize (to empty) the lists for mapping multi-plot window ids
       to multi-plot window data structures, and overlay ids to base windows */
    multiPlotWins[0].id = 0;
    clearOverlayList();

    /* Read the first line of the file (later lines are read below) */
    if (!(fp = getLine(line, fp, &lineNum, MAX_CONFIG_LINE_LEN))) {
	DialogF(DF_ERR, parent, 1, "Configuration is empty", "OK");
    	return False;
    }

    /*
    ** Loop and read headings and keywords from the file and process them
    */
    while (!eof) {
    	/* Identify the heading.  If not recognized, do dialog and return */
    	for (i=0; i<NHEADINGS; i++) {
    	    if (!strncasecmp(line, Headings[i], strlen(Headings[i]))) {
		headingIndex = i;
		headingLineNum = lineNum;
		break;
    	    }
    	}
    	if (i == NHEADINGS) {
    	    DialogF(DF_ERR, parent, 1,
    	    	    "Unrecognized heading:\n  %s\nat config line %d",
	    	    "OK", line, lineNum);
	    return False;
	}
	
	/* clear out the values structure in preparation for reading keywords */
	memset(&values, 0, sizeof(values));
	for (i=0; i<MAX_DISP_VARS; i++) {
	    values.markStyles[i] = -1;
	    values.lineStyles[i] = -1;
	    values.fillStyles[i] = -1;
	    values.markSizes[i] = -1;
	    values.ncolors = -12345;
	}

	/* Loop reading the keywords under the heading and accumulating the
	   information in the structure "values" until eof or next heading */
	while (True) {
    	    if (!(fp = getLine(line, fp, &lineNum, MAX_CONFIG_LINE_LEN))) {
		eof = True;
		break;
   	    }
    	    if (!(line[0] == ' ' || line[0] == '\t'))
    		break;
    	    if (!readKeyword(line, &values, &errMsg)) {
    	    	DialogF(DF_ERR, parent, 1, "Error at config line %d:\n %s\n%s",
    	    	    "OK", lineNum, line, errMsg);
    	    	return False;
    	    }
            patchValues(&values);
    	}

    	/* Display the item from the heading just read (or in the case of
    	   ntuple extensions, just create them).  Also inhibit auto
    	   help, and defer window menu updates until the end */
	if (headingIndex == NTUPLE_EXT_HEADING) {
	    if (!enterNtupleExtension(parent, &values, headingLineNum)) {
	    	return True;
	    }
	} else if (headingIndex == MULTIPLOT_HEADING) {
    	    InhibitWindowMenuUpdates = True;
	    plotOk = displayMultiPlotWindow(parent, &values,
			     headingLineNum, multiPlotWins);
    	    InhibitWindowMenuUpdates = False;
	    if (!plotOk) {
	    	return True;
	    }
	} else if (headingIndex == COLORSCALE_HEADING) {
	    if (!prepareColorScale(parent, &values, headingLineNum)) {
		return True;
	    }
	} else {
	    autoHelpSave = PrefData.plotAutoHelp;
	    PrefData.plotAutoHelp = False;
    	    InhibitWindowMenuUpdates = True;
    	    plotOk = displayPlot(parent, headingIndex, &values,
				 headingLineNum, multiPlotWins);
    	    InhibitWindowMenuUpdates = False;
            PrefData.plotAutoHelp = autoHelpSave;
	    if (!plotOk) {
	    	return True;
	    }
    	}
    }
    
    /* Window menu updates are deferred until all window are up, for speed,
       and to avoid a poorly understood timing bug. */
    UpdateWindowsMenu();
    return True;
}

#define patchElement(name) do {\
    if (strncmp(name, special, len) == 0)\
    {\
        memset(name, 0, sizeof(name));\
        name[0] = ' ';\
    }\
} while(0);

void patchValues(keywordValues *values)
{
    static const char special[] = "<Space>";
    static const unsigned len = 7U;

    patchElement(values->category);
    patchElement(values->geometry);
    patchElement(values->name);
    patchElement(values->windowTitle);
    patchElement(values->keyword);
    patchElement(values->xlabel);
    patchElement(values->ylabel);
    patchElement(values->zlabel);
}

void WriteMultilineString(FILE *fp, char *string)
{
    char *c;
    int lineLen = 0;
    
    for (c=string; *c!= '\0'; c++) {
        if (lineLen++ > MAX_SAVE_LINE_LEN) {
            fprintf(fp, "\\\n");
            lineLen = 0;
        }
        if (*c == '\n') {
            fprintf(fp, "\\n");
            lineLen++;
        } else
            fputc(*c, fp);
    }
}

/*
** Write the configuration information for a window to file "fp".  If multiWinID
** is > 0, this is part of a multi-plot window and "row" and "col" give the
** location in the multi-plot panel.  "overlayID" is the current overlayID
** number, to be used to identify plots as part of an overlayed group.
**
** Note:  Keywords are not written if the setting is set to its default
** value.  Note that default values for some of these keywords are implied
** here without referencing the real default value, so if actual default
** settings for these parameters are changed elsewhere, they must be changed
** here as well.
*/
static void writeWindowConfig(FILE *fp, windowInfo *window, int multiWinID,
	int overlayID, int row, int col)
{
    int i, nCurves, nHists;
    XYCurve *curveStyles = (XYCurve *)window->curveStyles;
    XYHistogram *histStyles = (XYHistogram *)window->histStyles;
    plotInfo *pInfo;
    Display *display = XtDisplay(window->widget);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(window->widget));
    Pixel black = BlackPixelOfScreen(XtScreen(window->widget));

    for (i=0; i<window->nPlots; i++) {
	pInfo = window->pInfo[i];
	nCurves = NCurvesInPlot(pInfo->plotType, pInfo->ntVars);
	nHists = PlotIsHist(pInfo->plotType);
	writePlotInfo(fp, display, cmap, black, pInfo,
		      curveStyles, nCurves, histStyles, nHists);
	curveStyles += nCurves;
	histStyles += nHists;
	if (i == 0)
	    writeWindowInfo(fp, window, multiWinID, overlayID, row, col);
	else
	    fprintf(fp, " %s %d\n", INOVERLAY_KEY, overlayID);
    }
}

/*
** Write the plot information for one plot from "plotInfo" to file "fp",
** which includes the heading line for the plot.  The arguments "curveStyles",
** and "histStyles" are pointers to the curve and histogram style(s) for
** the current plot.
*/
static void writePlotInfo(FILE *fp, Display *dpy, Colormap cmap,
			  Pixel black, plotInfo *pInfo,
			  XYCurve *curveStyles, int nCurveStyles,
			  XYHistogram *histStyles, int nHistStyles)
{
    int i, index;
    hsGeneral *item;
    float thresh;
    XYCurve *style;
    XYHistogram *hStyle;
    int plotType = pInfo->plotType;

    /* Look up the data item that the window is displaying, and write out
       the appropriate heading for that type of data */
    item = GetMPItemByID(pInfo->id);
    if (item->type == HS_2D_HISTOGRAM && plotType == CELL)
    	fprintf(fp, "%s\n", Headings[CELL_HEADING]);
    else if (item->type == HS_2D_HISTOGRAM && plotType == COLORCELL)
	fprintf(fp, "%s\n", Headings[COLORCELL_HEADING]);
    else {
	for (i=0; i<NHEADINGS; i++) {
    	    if (item->type == HeadingItemTypes[i]) {
    		fprintf(fp, "%s\n", Headings[i]);
    		break;
    	    }
	}
    }
    
    /* Write out the keywords common to all items */
    if (item->category != NULL && item->category[0] != '\0')
    	fprintf(fp, " %s %s\n", CATEGORY_KEY, item->category);
    if (item->uid != 0)
    	fprintf(fp, " %s %d\n", UID_KEY, item->uid);
    fprintf(fp, " %s %s\n", NAME_KEY, item->title);
    if (item->hbookID != 0)
    	fprintf(fp, " %s %d\n", HBOOKID_KEY, item->hbookID);
    
    /* Write out the keywords for ntuple items */
    if (item->type == HS_NTUPLE) {
    	fprintf(fp, " %s %s\n", PLOTTYPE_KEY,
    	    	PlotTypeNames[plotType]);
    	for (i=0; i<MAX_DISP_VARS; i++) {
    	    index = pInfo->ntVars[i];
    	    if (index != -1) {
    	    	fprintf(fp, " V%d %s\n", i+1,
    	    		ExtNTVarName((hsNTuple *)item, index));
    	    }
    	}
    	for (i=0; i<N_SLIDERS; i++) {
    	    index = pInfo->sliderVars[i];
    	    if (index != -1) {
    	    	fprintf(fp, " S%d %s\n", i+1,
    	    		ExtNTVarName((hsNTuple *)item, index));
    	    	thresh = pInfo->sliderThresholds[i];
    	    	if (thresh != FLT_MAX && thresh != -FLT_MAX)
    	    	    fprintf(fp, " Slider%sThreshold%d %g\n",
    	    	    	    pInfo->sliderGTorLT[i] == SLIDER_LT ? "<" :
    	    	    	    ">", i+1, thresh);
    	    }
    	}
    	if (plotType == HIST1D)
    	    fprintf(fp, " %s %d\n", NBINS_KEY, pInfo->nXBins);
    	else if (plotType == HIST2D || plotType == CELL || plotType == COLORCELL)
    	    fprintf(fp, " %s %d\n %s %d\n", NXBINS_KEY, pInfo->nXBins,
    	    	    NYBINS_KEY, pInfo->nYBins);
    	else if (plotType == ADAPTHIST1D || plotType == ADAPTHIST2D)
    	    fprintf(fp, " %s %d\n", BINLIMIT_KEY, pInfo->nXBins);
    }

    /* Write out the color scale info */
    if (plotType == COLORSCAT2D || plotType == COLORCELL)
    {
	fprintf(fp, " %s %s\n", COLORSCALENAME_KEY,
		pInfo->csi.colorScale->name);
	if (pInfo->csi.rangeIsDynamic)
	    fprintf(fp, " %s\n", DYNAMICCOLOR_KEY);
	else
	{
	    fprintf(fp, " %s %g\n", ZMINLIMIT_KEY, pInfo->csi.colorMin);
	    fprintf(fp, " %s %g\n", ZMAXLIMIT_KEY, pInfo->csi.colorMax);
	}
	if (pInfo->csi.colorIsLog)
	    fprintf(fp, " %s\n", LOGZ_KEY);
    }

    /* Write out settings for histograms */
    if (pInfo->errorBars == DATA_ERROR_BARS)
    	fprintf(fp, " %s\n", DATAERROR_KEY);
    if (pInfo->errorBars == GAUSSIAN_ERROR_BARS)
    	fprintf(fp, " %s\n", GAUSSERROR_KEY);
    if (pInfo->aHistBinStrategy == CENTER_OF_GRAVITY)
    	fprintf(fp, " %s\n", CNTROFGRAV_KEY);

    /* Write out style information for curves and histograms */
    for (i=0, style=curveStyles; i<nCurveStyles; i++, style++) {
    	if (style->markerStyle != XY_NO_MARK)
    	    fprintf(fp, " MarkerStyle%d %d\n", i+1, style->markerStyle);
    	if (style->markerSize != XY_SMALL)
    	    fprintf(fp, " MarkerSize%d %d\n", i+1, style->markerSize);
    	if (style->lineStyle != i+1)
    	    fprintf(fp, " LineStyle%d %d\n", i+1, style->lineStyle);
    	if (style->markerPixel != black) {
    	    fprintf(fp, " MarkerColor%d ", i+1);
	    writeNonscaleColor(dpy, cmap, style->markerPixel, fp);
	    fprintf(fp, "\n");
	}
	if (style->linePixel != black) {
	    fprintf(fp, " LineColor%d ", i+1);
	    writeNonscaleColor(dpy, cmap, style->linePixel, fp);
	    fprintf(fp, "\n");
	}
    }
    for (i=0, hStyle=histStyles; i<nHistStyles; i++, hStyle++) {
    	if (hStyle->fillStyle != XY_NO_FILL)
    	    fprintf(fp, " FillStyle%d %s\n", i+1,
    	    	    fillStyleName(hStyle->fillStyle));
    	if (hStyle->lineStyle != i+1)
    	    fprintf(fp, " LineStyle%d %d\n", i+1, hStyle->lineStyle);
	if (hStyle->fillPixel != black) {
	    fprintf(fp, " FillColor%d ", i+1);
	    writeNonscaleColor(dpy, cmap, hStyle->fillPixel, fp);
	    fprintf(fp, "\n");
	}
	if (hStyle->linePixel != black) {
	    fprintf(fp, " LineColor%d ", i+1);
	    writeNonscaleColor(dpy, cmap, hStyle->linePixel, fp);
	    fprintf(fp, "\n");
	}
    }
}


/*
** Write additional window configuration information for "window" to file "fp"
** if multiWinID is > 0, this is part of a multi-plot window and "row" and
** "col" give the location in the multi-plot panel.
*/
static void writeWindowInfo(FILE *fp, windowInfo *window, int multiWinID,
	int overlayID, int row, int col)
{
    char geomString[MAX_GEOMETRY_LEN];
    int xLog, yLog, zLog;
    int xMinDefault, xMaxDefault, yMinDefault, yMaxDefault;
    int zMinDefault, zMaxDefault, angle1Default, angle2Default, angle3Default;
    double xMinLim, xMaxLim, yMinLim, yMaxLim, zMinLim, zMaxLim;
    double angle1, angle2, angle3;
    WidgetClass class;
    XmFontList font;
    XmString psFont;
    int psFontSize;
    int addLeft, addRight, addBottom, addTop;

    /* Write out the window title and position information (geometry if it's
       an individual window, row and column if it's part of a multi-plot) */
    if (multiWinID > 0) {
        fprintf(fp, " %s %d\n", INMULTIPLOT_KEY, multiWinID);
        fprintf(fp, " %s %d\n", ROW_KEY, row);
        fprintf(fp, " %s %d\n", COLUMN_KEY, col);
    } else {
    	char *windowTitle;
	createGeomString(window->shell, geomString);
	fprintf(fp, " %s %s\n", GEOMETRY_KEY, geomString);
	XtVaGetValues(window->shell, XmNtitle, &windowTitle, NULL);
	fprintf(fp, " %s %s\n", WINDOWNAME_KEY, windowTitle);
	if (window->windowID[0])
	    fprintf(fp, " %s %s\n", WINDOWLOC_KEY, window->windowID);
    }
    
    /* If this is an overlay window, write out the overlay id */
    if (window->nPlots > 1)
    {
       	fprintf(fp, " %s %d\n", OVERLAY_KEY, overlayID);
	if (window->xlabel[0])
	    fprintf(fp, " %s %s\n", XLABEL_KEY, window->xlabel);
	if (window->ylabel[0])
	    fprintf(fp, " %s %s\n", YLABEL_KEY, window->ylabel);
    }

    /* Write out information on auxilliary windows, if any are up */
    if (window->sliderWindow != NULL) {
    	createGeomString(XtParent(window->sliderWindow), geomString);
    	fprintf(fp, " %s %s\n", SLIDERWIN_KEY, geomString);
    }
    if (window->rebinWindow != NULL) {
    	createGeomString(XtParent(window->rebinWindow), geomString);
    	fprintf(fp, " %s %s\n", REBINWIN_KEY, geomString);
    }
    if (window->statsWindow != NULL) {
    	createGeomString(XtParent(window->statsWindow), geomString);
    	fprintf(fp, " %s %s\n", STATSWIN_KEY, geomString);
    }
    if (window->cellNormWindow != NULL) {
    	createGeomString(XtParent(window->cellNormWindow), geomString);
    	fprintf(fp, " %s %s\n", CELLNORMWIN_KEY, geomString);
    }
    
    /* Get the plot limit and rotation information.  If this isn't a plot
       (meaning it's a trigger or control), we're done. */
    if (!getPlotLimits(window->widget, &xMinDefault, &xMinLim, &xMaxDefault,
    	    &xMaxLim, &yMinDefault, &yMinLim, &yMaxDefault, &yMaxLim,
    	    &zMinDefault, &zMinLim, &zMaxDefault, &zMaxLim, &angle1Default,
    	    &angle1, &angle2Default, &angle2, &angle3Default, &angle3))
    	return;

    /* Write out the plot font information */
    XtVaGetValues(window->widget,
		  XmNfontList, &font,
		  XmNpsFont, &psFont,
		  XmNpsFontSize, &psFontSize,
		  NULL);
    if (font && psFont)
    {
	char *fontName = getFontName(XtDisplay(window->widget), font);
	if (fontName)
	{
	    fprintf(fp, " %s %s\n", FONT_KEY, fontName);
	    XFree(fontName);
	}
	XmStringGetLtoR(psFont, XmSTRING_DEFAULT_CHARSET, &fontName);
	fprintf(fp, " %s %s\n", PSFONT_KEY, fontName);
	XtFree(fontName);
	fprintf(fp, " %s %d\n", PSFONTSIZE_KEY, psFontSize);
    }
    if (psFont)
	XmStringFree(psFont);

    /* Write the additional margins */
    XtVaGetValues(window->widget,
		  XmNaddLeftMargin, &addLeft,
		  XmNaddRightMargin, &addRight,
		  XmNaddTopMargin, &addTop,
		  XmNaddBottomMargin, &addBottom,
		  NULL);
    if (addLeft)
	fprintf(fp, " %s %d\n", MARGINLEFT_KEY, addLeft);
    if (addRight)
	fprintf(fp, " %s %d\n", MARGINRIGHT_KEY, addRight);
    if (addTop)
	fprintf(fp, " %s %d\n", MARGINTOP_KEY, addTop);
    if (addBottom)
	fprintf(fp, " %s %d\n", MARGINBOTTOM_KEY, addBottom);

    /* Write the plot limit, rotation, log, and plot-specific settings */
    class = XtClass(window->widget);
    if (!xMinDefault)
    	fprintf(fp, " %s %g\n", XMINLIMIT_KEY, xMinLim);
    if (!xMaxDefault)
    	fprintf(fp, " %s %g\n", XMAXLIMIT_KEY, xMaxLim);
    if (!yMinDefault)
    	fprintf(fp, " %s %g\n", YMINLIMIT_KEY, yMinLim);
    if (!yMaxDefault)
    	fprintf(fp, " %s %g\n", YMAXLIMIT_KEY, yMaxLim);
    if (!zMinDefault)
    	fprintf(fp, " %s %g\n", ZMINLIMIT_KEY, zMinLim);
    if (!zMaxDefault)
    	fprintf(fp, " %s %g\n", ZMAXLIMIT_KEY, zMaxLim);
    if (!angle1Default && class == hist2DWidgetClass)
    	fprintf(fp, " %s %g\n", FI_KEY, angle1);
    if (!angle1Default && class == scat3DWidgetClass)
    	fprintf(fp, " %s %g\n", ALPHA_KEY, angle1);
    if (!angle2Default && class == hist2DWidgetClass)
    	fprintf(fp, " %s %g\n", PSI_KEY, angle2);
    if (!angle2Default && class == scat3DWidgetClass)
    	fprintf(fp, " %s %g\n", BETA_KEY, angle2);
    if (!angle3Default)
    	fprintf(fp, " %s %g\n", GAMMA_KEY, angle3);
    getLogSettings(window->widget, &xLog, &yLog, &zLog);
    if (xLog)
    	fprintf(fp, " %s\n", LOGX_KEY);
    if (yLog)
    	fprintf(fp, " %s\n", LOGY_KEY);
    if (zLog)
    	fprintf(fp, " %s\n", LOGZ_KEY);
    if (toggleSetting(window->thickenMenuItem, False))
    	fprintf(fp, " %s\n", DARKEN_KEY);
    if (toggleSetting(window->binEdgeMenuItem, False))
    	fprintf(fp, " %s\n", BINEDGE_KEY);
    if (toggleSetting(window->backplanesMenuItem, True))
    	fprintf(fp, " %s\n", BACKPLANES_KEY);
    if (toggleSetting(window->legendMenuItem, True))
    	fprintf(fp, " %s\n", HIDELEGEND_KEY);
    if (toggleSetting(window->growOnlyMenuItem, False))
    	fprintf(fp, " %s\n", GROWONLY_KEY);
    if (class == cellWidgetClass) {
    	if (window->cellNormMin != -FLT_MAX)
    	    fprintf(fp, " %s %g\n", NORMMIN_KEY, window->cellNormMin);
    	if (window->cellNormMax != FLT_MAX)
    	    fprintf(fp, " %s %g\n", NORMMAX_KEY, window->cellNormMax);
    }
}

/*
** Write the configuration information for multi-plot window "window" to
** file "fp", including the descriptions of each individual plot in the
** window via writeWindowConfig
*/
static void writeMultiWinConfig(FILE *fp, multWindow *window, int id,
    	int *overlayID)
{
    char geomString[MAX_GEOMETRY_LEN];
    int i, j;
    multiPlot *plot;
    char *windowTitle;

    /* Write out the information concerning the whole window */
    fprintf(fp, "%s\n", Headings[MULTIPLOT_HEADING]);
    XtVaGetValues(window->appShell, XmNtitle, &windowTitle, NULL);
    fprintf(fp, " %s %s\n", WINDOWNAME_KEY, windowTitle);
    createGeomString(window->appShell, geomString);
    fprintf(fp, " %s %s\n", GEOMETRY_KEY, geomString);
    fprintf(fp, " %s %d\n", WINDOWID_KEY, id);
    if (window->windowID[0])
	fprintf(fp, " %s %s\n", WINDOWLOC_KEY, window->windowID);
    fprintf(fp, " %s %d\n", ROWS_KEY, window->numRows);
    fprintf(fp, " %s %d\n", COLUMNS_KEY, window->numCols);
    if (!window->dispLabels)
    	fprintf(fp, " %s\n", HIDETITLES_KEY);

    /* Write out the information for each plot in the window */
    for (i = 0; i < window->numRows; ++i) {
    	for (j = 0; j < window->numCols; ++j) {
    	    plot = &window->plot[i*window->numCols+j];
    	    if (plot->wInfo != NULL) {
    	        writeWindowConfig(fp, plot->wInfo, id, *overlayID, i+1, j+1);
    	        if (plot->wInfo->nPlots > 1)
    	            (*overlayID)++;
		writeDrawingsAndComments(fp, plot->wInfo, window->windowID,
				    i+1, j+1);
    	    }
    	}
    }
}

/*
** Read a line containing a keyword and optional argument, and record the 
** argument value and the fact that the keyword was present in the structure
** "values".  On error, return False and report the error by returning a
** pointer to an error message string in errMsg;
*/
static int readKeyword(char *line, keywordValues *values, char **errMsg)    
{
    int i, index;
    
    /* Look up the keyword in the keyword list, and find it's index.  If
       the keyword doesn't match, return an error */
    for (i=0; i<NKEYWORDS; i++) {
    	if (compareKeyword(line, Keywords[i])) {
    	    index = i;
    	    break;
    	}
    }
    if (i == NKEYWORDS) {
    	*errMsg = "Unrecognized keyword";
    	return False;
    }
    
    /* Mark the keyword as read */
    values->specified[index] = True;

    /* Dispatch to the appropriate reader for the argument (if any) */
    switch (index) {
        case XLABEL_INDEX:
	    return readLabel(line, values->xlabel, errMsg);
        case YLABEL_INDEX:
	    return readLabel(line, values->ylabel, errMsg);
        case GEOMETRY_INDEX:
            return readGeom(line, values->geometry, errMsg);
        case CATEGORY_INDEX:
            return readCategory(line, values->category, errMsg);
        case UID_INDEX:
            return readIntArg(line, &values->uid, errMsg);
        case NAME_INDEX:
    	    return readName(line, values->name, errMsg);
        case HBOOKID_INDEX:
    	    return readIntArg(line, &values->hbookID, errMsg);
        case WINDOWNAME_INDEX:
    	    return readShortString(line, values->windowTitle,
				   HS_MAX_WINDOW_TITLE_LENGTH, errMsg);
    	case XMINLIMIT_INDEX:
    	    return readFloatArg(line, &values->scale.xMinLimit, errMsg);
    	case XMAXLIMIT_INDEX:
    	    return readFloatArg(line, &values->scale.xMaxLimit, errMsg);
    	case YMINLIMIT_INDEX:
    	    return readFloatArg(line, &values->scale.yMinLimit, errMsg);
    	case YMAXLIMIT_INDEX:
    	    return readFloatArg(line, &values->scale.yMaxLimit, errMsg);
    	case ZMINLIMIT_INDEX:
    	    return readFloatArg(line, &values->scale.zMinLimit, errMsg);
    	case ZMAXLIMIT_INDEX:
    	    return readFloatArg(line, &values->scale.zMaxLimit, errMsg);
    	case FI_INDEX:
    	case ALPHA_INDEX:
    	    return readFloatArg(line, &values->scale.angle1, errMsg);
    	case PSI_INDEX:
    	case BETA_INDEX:
    	    return readFloatArg(line, &values->scale.angle2, errMsg);
    	case GAMMA_INDEX:
    	    return readFloatArg(line, &values->scale.angle3, errMsg);
    	case PLOTTYPE_INDEX:
    	    return readPlotType(line, &values->plotType, errMsg);
    	case VAR_INDEX:
    	    return readVar(line, values, errMsg);
    	case EXPRNAME_INDEX:
    	    return readExprName(line, values, errMsg);
    	case EXPR_INDEX:
    	    return readExpr(line, values, errMsg);
    	case SLIDER_INDEX:
    	    return readSlider(line, values, errMsg);
    	case SLIDERWIN_INDEX:
    	    return readGeom(line, values->sliderWinGeom, errMsg);
    	case REBINWIN_INDEX:
    	    return readGeom(line, values->rebinWinGeom, errMsg);
    	case CELLNORMWIN_INDEX:
    	    return readGeom(line, values->cellNormWinGeom, errMsg);
    	case STATSWIN_INDEX:
    	    return readGeom(line, values->statsWinGeom, errMsg);
    	case SLIDERLT_INDEX:
    	    return readSliderThreshold(line, values, False, errMsg);
    	case SLIDERGT_INDEX:
    	    return readSliderThreshold(line, values, True, errMsg);
	case NBINS_INDEX:
	case NXBINS_INDEX:
	case BINLIMIT_INDEX:
	    return readIntArg(line, &values->nXBins, errMsg);
	case NYBINS_INDEX:
	    return readIntArg(line, &values->nYBins, errMsg);
	case NORMMIN_INDEX:
	    return readFloatArg(line, &values->normMin, errMsg);
	case NORMMAX_INDEX:
	    return readFloatArg(line, &values->normMax, errMsg);
	case MARKSTYLE_INDEX:
	    return readStyle(line, values->markStyles, errMsg);
	case LINESTYLE_INDEX:
	    return readStyle(line, values->lineStyles, errMsg);
	case FILLSTYLE_INDEX:
	    return readFillStyle(line, values->fillStyles, errMsg);
	case MARKSIZE_INDEX:
	    return readStyle(line, values->markSizes, errMsg);
	case MARKCOLOR_INDEX:
	    return readColor(line, &values->markColors[0][0], errMsg);
	case LINECOLOR_INDEX:
	    return readColor(line, &values->lineColors[0][0], errMsg);
	case FILLCOLOR_INDEX:
	    return readColor(line, &values->fillColors[0][0], errMsg);
	case INMULTIPLOT_INDEX:
	    return readIntArg(line, &values->multiPlotID, errMsg);
	case INOVERLAY_INDEX:
	    return readIntArg(line, &values->overlayID, errMsg);
	case ROWS_INDEX:
	    return readIntArg(line, &values->row, errMsg);
	case COLUMNS_INDEX:
	    return readIntArg(line, &values->col, errMsg);
	case ROW_INDEX:
	    return readIntArg(line, &values->row, errMsg);
	case COLUMN_INDEX:
	    return readIntArg(line, &values->col, errMsg);
	case WINDOWID_INDEX:
	    return readIntArg(line, &values->multiPlotID, errMsg);
	case WINDOWLOC_INDEX:
	    return readWinLoc(line, values->windowLoc, errMsg);
	case WINDOWACT_INDEX:
    	    return readShortString(line, values->windowAct,
				   MAX_CONFIG_LINE_LEN, errMsg);
	case OVERLAY_INDEX:
	    return readIntArg(line, &values->overlayID, errMsg);
        case NOCOMPLAIN_INDEX:
            return readIntArg(line, &values->noComplain, errMsg);
        case COLORSPACE_INDEX:
    	    return readShortString(line, values->colorSpace,
				   MAX_COLORSPACE_LEN, errMsg);
        case NCOLORS_INDEX:
            return readIntArg(line, &values->ncolors, errMsg);
        case STARTCOLOR_INDEX:
        case FGCOLOR_INDEX:
            return readShortString(line, values->startColor,
				   MAX_COLOR_LEN, errMsg);
        case ENDCOLOR_INDEX:
        case BGCOLOR_INDEX:
            return readShortString(line, values->endColor,
				   MAX_COLOR_LEN, errMsg);
        case UNDERCOLOR_INDEX:
            return readShortString(line, values->underflowColor,
				   MAX_COLOR_LEN, errMsg);
        case OVERCOLOR_INDEX:
            return readShortString(line, values->overflowColor,
				   MAX_COLOR_LEN, errMsg);
        case PIXELCOLOR_INDEX:
	    return readPixelData(line, values->pixelColors, errMsg);
        case COLORSCALENAME_INDEX:
	    return readColorScaleName(line, &values->clrScale, errMsg);
        case UPDATEFREQUENCY_INDEX:
	    return readIntArg(line, &values->updateFrequency, errMsg);
	case MINPSLINEWIDTH_INDEX:
    	    return readFloatArg(line, &values->minPSlineWidth, errMsg);
        case BUFFERGRAPHICS_INDEX:
	    return readIntArg(line, &values->bufferGraphics, errMsg);
        case THICKENSCAT_INDEX:
	    return readIntArg(line, &values->thickenPointsScat, errMsg);
	case THICKENSCAT3D_INDEX:
	    return readIntArg(line, &values->thickenPointsScat3D, errMsg);
        case AUTOMATICHELP_INDEX:
            return readIntArg(line, &values->autohelp, errMsg);
        case TASKNUMBER_INDEX:
            return readIntArg(line, &values->taskNumber, errMsg);
        case FONT_INDEX:
	    return readShortString(line, values->fontName,
				   MAX_FONTNAME_LEN, errMsg);
        case PSFONT_INDEX:
	    return readShortString(line, values->psFontName,
				   MAX_FONTNAME_LEN, errMsg);
        case PSFONTSIZE_INDEX:
	    return readIntArg(line, &values->psFontSize, errMsg);
        case ALIGNMENT_INDEX:
            return readAlignmentArg(line, &values->alignment, errMsg);
        case ARROWSTYLE_INDEX:
            return readIntArg(line, &values->arrowstyle, errMsg);
        case REFPOINT_INDEX:
            return readDrawingPoint(line, &values->refpoint, errMsg);
        case DRAWPOINT_INDEX:
            return readNumberedPoint(line, values, errMsg);
        case MARGINLEFT_INDEX:
	    return readIntArg(line, &values->addLeft, errMsg);
        case MARGINRIGHT_INDEX:
	    return readIntArg(line, &values->addRight, errMsg);
        case MARGINTOP_INDEX:
	    return readIntArg(line, &values->addTop, errMsg);
        case MARGINBOTTOM_INDEX:
	    return readIntArg(line, &values->addBottom, errMsg);
        case JUSTIFYTEXT_INDEX:
	    return readIntArg(line, &values->justify, errMsg);
        case MAGNIFICATION_INDEX:
	    return readFloatArg(line, &values->magnify, errMsg);
        case PRINTCOORDS_INDEX:
            return readIntArg(line, &values->printCoords, errMsg);
        case ERRMARKER_INDEX:
            return readFloatArg(line, &values->errMarkerRatio, errMsg);
        case LEFTARROWS_INDEX:
            return readIntArg(line, &values->enableLeftArr, errMsg);
        case RIGHTARROWS_INDEX:
            return readIntArg(line, &values->enableRightArr, errMsg);
        case TOPARROWS_INDEX:
            return readIntArg(line, &values->enableTopArr, errMsg);
        case BOTTOMARROWS_INDEX:
            return readIntArg(line, &values->enableBottomArr, errMsg);
    }

    /* No argument, just return true */
    return True;
}

/*
** Display an item of (probably) of the type specified in "headingType", using
** the keyword values read from the configuration file.  Return True if the
** item was read or skipped successfully, False to give up reading the file.
** headingLineNum is the line number to show the user if something goes wrong.
** multiPlotIDs is an array for mapping the multiPlot window IDs used to
** identify these window in the configuration file to the actual window
** in which to display the plot.
*/
static int displayPlot(Widget parent, int headingType, keywordValues *values,
		       int headingLineNum, multiPlotIDRec *multiPlotIDs)
{
    hsGeneral *item;
    windowInfo *window;
    readyCallBack *readycall;
    char *geometry, *winID, *filename, *comment, *s_end;
    const char *normalColor;
    int i, needPoints, row, column, taskNumber, styleChanged;
    int varList[MAX_DISP_VARS], sliderList[N_SLIDERS];
    char *keyUsed = values->specified;
    int overlayedType = -1, choice, rebin = False;
    char *colorName, *errVar, *errMsg = "";
    multWindow *multiPlotWin;
    extern windowInfo *WindowList;
    extern multWindow *MultWindowList;
    plotInfo *pInfo;
    colorScaleInfo csData;
    colorScaleInfo *csInfo;
    OverlayedObject *obj = NULL, *listPtr;
    XcmsColor exact_def, screen_def;
    XcmsColorFormat format = XcmsRGBiFormat;
    Display *dpy;
    Colormap cmap;
    XmFontListEntry fontEntry = NULL;
    widgetConfigInfo confInfo;
    Pixel *colorValue;
    XmString psFontResource = NULL;

    taskNumber = keyUsed[TASKNUMBER_INDEX] ? values->taskNumber : 0;
    winID = keyUsed[WINDOWLOC_INDEX] ? values->windowLoc : NULL;
    memset(&confInfo, 0, sizeof(confInfo));

#ifdef REQUIRE_PSFONT_SPEC
    /* Require that PostScript font is also specified
     * in case we have a font specification for something.
     */
    if (keyUsed[FONT_INDEX] && !keyUsed[PSFONT_INDEX])
    {
	choice = DialogF(DF_ERR, parent, 2,
	    "PostScript font not specified for\nheading \"%s\" at line %d",
	    "Cancel", "Continue", Headings[headingType], headingLineNum);
	ReportTaskCompletion(taskNumber, 1, "missing postscript font");
	return choice != 1;
    }
#endif /* REQUIRE_PSFONT_SPEC */

    /* Check that the font specification makes sense */
    if (keyUsed[FONT_INDEX])
    {
	fontEntry = getFontEntry(XtDisplay(parent), values->fontName);
	if (fontEntry == NULL)
	{
	    choice = DialogF(DF_ERR, parent, 2,
		"Can't load font \"%s\" for\nheading \"%s\" at line %d",
		"Cancel", "Continue", values->fontName,
		Headings[headingType], headingLineNum);
	    ReportTaskCompletion(taskNumber, 1, "unsupported font");
	    return choice != 1;
	}
    }

    if (headingType == WINACTION_HEADING)
    {
	char actionWord[32];
	strncpy(actionWord, values->windowAct, 31);
	actionWord[31] = '\0';
	for (i=0; i<31; ++i)
	    if (isspace(actionWord[i]))
	    {
		actionWord[i] = '\0';
		break;
	    }

	if (!(keyUsed[WINDOWLOC_INDEX] && keyUsed[WINDOWACT_INDEX]))
	{
	    choice = DialogF(DF_ERR, parent, 2,
	       "Both WindowName and Action are required for\nheading \"%s\" at line %d",
	       "Cancel", "Continue", Headings[headingType], headingLineNum);
	    ReportTaskCompletion(taskNumber, 1, "required argument is missing");
	    return choice != 1;
	}

	/* Check if this is the "CloseAll" command which ignores the window name */
	if (strcmp(actionWord, "CloseAll") == 0)
	    return CloseAll_cmd("CloseAll", taskNumber, 0, NULL, &errMsg);

	/* Lookup the window name */
	i = 0;
	if (winID[0]) {
	    for (multiPlotWin=MultWindowList; multiPlotWin!=NULL;
		 multiPlotWin=multiPlotWin->next) {
		if (multiPlotWin->windowID[0]) {
		    if (strcmp(winID, multiPlotWin->windowID) == 0) {
			i = 2;
			break;
		    }
		}
	    }
	    if (i == 0) {
		for (window = WindowList; window != NULL; window = window->next) {
		    if (window->windowID[0]) {
			if (strcmp(winID, window->windowID) == 0) {
			    i = 1;
			    break;
			}
		    }
		}
	    }
	}

	/* Check if we are asked whether the window still exists */
	if (strcmp(actionWord, "Exists") == 0)
	{
	    ReportTaskCompletion(taskNumber, 0, i ? "1" : "0");
	    return True;
	}

	if (i == 0) {
	    /* Window name not found */
	    if (keyUsed[NOCOMPLAIN_INDEX] && values->noComplain)
		ReportTaskCompletion(taskNumber, 0, NULL);
	    else
	    {
		fprintf(stderr, "Warning: invalid window name \"%s\"\n", winID);
		ReportTaskCompletion(taskNumber, 1, "invalid window name");
	    }
	    return False;
	}

	/* Figure out the window for the multiplot */
	if (keyUsed[ROW_INDEX])
	    row = values->row - 1;
	else
	    row = 0;
	if (keyUsed[COLUMN_INDEX])
	    column = values->col - 1;
	else
	    column = 0;
	if (i == 2)
	{
	    if (row < 0 || row >= multiPlotWin->numRows)
	    {
		choice = DialogF(DF_ERR, parent, 2,
		    "Row number is out of range for\nheading \"%s\" at line %d",
		    "Cancel", "Continue", Headings[headingType], headingLineNum);
		ReportTaskCompletion(taskNumber, 1, "row out of range");
		return choice != 1;
	    }
	    if (column < 0 || column >= multiPlotWin->numCols)
	    {
		choice = DialogF(DF_ERR, parent, 2,
		    "Column number is out of range for\nheading \"%s\" at line %d",
		    "Cancel", "Continue", Headings[headingType], headingLineNum);
		ReportTaskCompletion(taskNumber, 1, "column out of range");
		return choice != 1;
	    }
	    window = multiPlotWin->plot[row*multiPlotWin->numCols+column].wInfo;
	    if (window == NULL)
	    {
		choice = DialogF(DF_ERR, parent, 2,
		    "No plot at given row and column for\nheading \"%s\" at line %d",
		    "Cancel", "Continue", Headings[headingType], headingLineNum);
		ReportTaskCompletion(taskNumber, 1, "no plot at given row and column");
		return choice != 1;
	    }
	}

	/* Process commands which can be executed immediately */
	/* Check if we are requested to clear things */
	if (strcmp(actionWord, "Clear") == 0)
	{
	    int n;
	    if (keyUsed[CATEGORY_INDEX])
	    {
		overlayedType = stringToOverlayedType(values->category);
		if (overlayedType < 0)
		{
		    choice = DialogF(DF_ERR, parent, 2,
			"Invalid category for\nheading \"%s\" at line %d",
			"Cancel", "Continue", Headings[headingType], headingLineNum);
		    ReportTaskCompletion(taskNumber, 1, "invalid category");
		    return choice != 1;
		}
		n = destroyAllOverlayedOfType(
		    &window->decor, window->widget, overlayedType);
	    }
	    else
	    {
		n = destroyOverlayedObjectList(window->decor, window->widget);
		window->decor = NULL;
	    }
	    if (n) safeRedisplay(window);
	    ReportTaskCompletion(taskNumber, 0, NULL);
	    return True;
	}

	if (strcmp(actionWord, "Delete") == 0)
	{
	    int n;
	    if (keyUsed[CATEGORY_INDEX])
	    {
		overlayedType = stringToOverlayedType(values->category);
		if (overlayedType < 0)
		{
		    choice = DialogF(DF_ERR, parent, 2,
			"Invalid category for\nheading \"%s\" at line %d",
			"Cancel", "Continue", Headings[headingType], headingLineNum);
		    ReportTaskCompletion(taskNumber, 1, "invalid category");
		    return choice != 1;
		}
		n = destroyLastOverlayedOfType(
		    &window->decor, window->widget, overlayedType);
	    }
	    else
	    {
		n = destroyLastOverlayedObject(&window->decor, window->widget);
	    }
	    if (n) safeRedisplay(window);
	    ReportTaskCompletion(taskNumber, 0, NULL);
	    return True;
	}

	/* Check if this is the "Redraw" command */
	if (strcmp(actionWord, "Redraw") == 0)
	{
	    safeRedisplay(window);
	    ReportTaskCompletion(taskNumber, 0, NULL);
	    return True;
	}

	/* Check if this is one of the object drawing commands */
	overlayedType = stringToOverlayedType(actionWord);
	if (overlayedType >= 0)
	{
	    Pixel black = BlackPixelOfScreen(XtScreen(window->widget));
	    Pixel white = WhitePixelOfScreen(XtScreen(window->widget));
	    obj = (OverlayedObject *)calloc(1, sizeof(OverlayedObject));
	    if (obj == NULL)
	    {
		fprintf(stderr, "Fatal error in displayPlot: "
			"out of memory. Exiting.\n");
		exit(EXIT_FAILURE);
	    }
	    obj->type = overlayedType;

	    /* Check if this is the "Comment" command */
	    if (overlayedType == OVER_STRING)
	    {
		/* Initialize colors to avoid resource leak */
		obj->item.s.color = black;
		obj->item.s.background = white;

		comment = values->windowAct + strlen(overlayedTypeString(overlayedType));
		if (*comment)
		    /* Next character should be a space */
		    ++comment;
		if (*comment == '\0')
		{
		    /* Empty comment defaults to no-op */
		    ReportTaskCompletion(taskNumber, 0, NULL);
		    destroyOverlayedObjectList(obj, window->widget);
		    return True;
		}

		/* Check position */
		if (!values->pointsSpecified[0])
		{
		    choice = DialogF(DF_ERR, parent, 2,
				     "Coordinates are not specified for\nheading \"%s\" at line %d",
				     "Cancel", "Continue", Headings[headingType], headingLineNum);
		    ReportTaskCompletion(taskNumber, 1, "missing coordinates");
		    destroyOverlayedObjectList(obj, window->widget);
		    return choice != 1;
		}
		obj->item.s.position = values->points[0];

		/* Check reference */
		if (keyUsed[REFPOINT_INDEX])
		    obj->item.s.reference = values->refpoint;

		/* Check alignment */
		obj->item.s.alignment = DEFAULT_ALIGNMENT;
		if (keyUsed[ALIGNMENT_INDEX])
		    obj->item.s.alignment = values->alignment;

		/* Check the color */
#define processColorDefinition do {\
		if (strcmp(colorName, "black") == 0)\
                    *colorValue = black;\
		else if (strcmp(colorName, "white") == 0)\
                    *colorValue = white;\
		else\
		{\
		    normalColor = normalizeColorName(colorName,\
						     values->overflowColor);\
		    if (normalColor)\
		    {\
			dpy = XtDisplay(window->widget);\
			cmap = DefaultColormapOfScreen(XtScreen(window->widget));\
			if (XcmsLookupColor(dpy, cmap, normalColor, &exact_def,\
					    &screen_def, format) == XcmsFailure)\
			{\
			    choice = DialogF(DF_ERR, parent, 2,\
				"Bad color name \"%s\" for\nheading \"%s\" at line %d",\
				"Cancel", "Continue", values->startColor,\
				Headings[headingType], headingLineNum);\
			    ReportTaskCompletion(taskNumber, 1, "bad color name");\
                            destroyOverlayedObjectList(obj, window->widget);\
			    return choice != 1;\
			}\
			if (AllocSharedColor(dpy, cmap, &screen_def, format) == XcmsFailure)\
			{\
			    choice = DialogF(DF_ERR, parent, 2,\
				"Failed to allocate color \"%s\" for\nheading \"%s\" at line %d",\
				"Cancel", "Continue", values->startColor,\
				Headings[headingType], headingLineNum);\
			    ReportTaskCompletion(taskNumber, 1, "color allocation failed");\
                            destroyOverlayedObjectList(obj, window->widget);\
			    return choice != 1;\
			}\
			*colorValue = screen_def.pixel;\
		    }\
		}\
} while(0)
		if (keyUsed[FGCOLOR_INDEX])
		{
		    colorName = values->startColor;
		    colorValue = &obj->item.s.color;
		    processColorDefinition;
		}
		if (keyUsed[BGCOLOR_INDEX])
		{
		    colorName = values->endColor;
		    colorValue = &obj->item.s.background;
		    processColorDefinition;
		    obj->item.s.options |= XYSTRING_FILLMASK;
		}

		/* Draw rectangle border? */
		if (keyUsed[BORDER_INDEX])
		    obj->item.s.options |= XYSTRING_BORDERMASK;

		/* Text justification */
		if (keyUsed[JUSTIFYTEXT_INDEX])
		    obj->item.s.options |= 
			((values->justify << JUSTIFY_TEXT_SHIFT) & JUSTIFY_TEXT_MASK);

		/* PostScript font specified? */
		if (keyUsed[PSFONT_INDEX])
		    if ((obj->item.s.psfont = strdup(values->psFontName)) == NULL)
		    {
			fprintf(stderr, "Fatal error in displayPlot: "
				"out of memory. Exiting.\n");
			exit(EXIT_FAILURE);
		    }
		obj->item.s.psfontsize = values->psFontSize;

		/* Create Motif string */
		obj->item.s.string = XmStringCreateLtoR(
		    comment, XmSTRING_DEFAULT_CHARSET);
		obj->item.s.font = XmFontListAppendEntry(NULL, fontEntry);
		obj->item.s.text = strdup(comment);
		if (obj->item.s.string == NULL || obj->item.s.text == NULL)
		{
		    fprintf(stderr, "Fatal error in displayPlot: "
			    "out of memory. Exiting.\n");
		    exit(EXIT_FAILURE);
		}
	    }

	    /* Check if this is the "PSPixmap" or "Latex" command */
	    else if (overlayedType == OVER_PSPIXMAP || overlayedType == OVER_LATEX)
	    {
		obj->item.p.bordercolor = black;
		obj->item.p.background = white;
		obj->item.p.pixmap = XmUNSPECIFIED_PIXMAP;
		obj->item.p.scale = 1.f;
		obj->item.p.channel = -1;
		obj->item.p.addLeft = values->addLeft;
		obj->item.p.addRight = values->addRight;
		obj->item.p.addTop = values->addTop;
		obj->item.p.addBottom = values->addBottom;

		/* Trim the file name */
		for (filename = values->windowAct + strlen(overlayedTypeString(overlayedType));
		     *filename && isspace(*filename); ++filename);
		if (*filename)
		{
		    for (s_end = filename + strlen(filename) - 1;
			 isspace(*s_end) && (s_end > filename); --s_end)
			*s_end = '\0';
		}
		else
		{
		    /* Empty file name defaults to no-op */
		    ReportTaskCompletion(taskNumber, 0, NULL);
		    destroyOverlayedObjectList(obj, window->widget);
		    return True;
		}
		if ((obj->item.p.filename = strdup(filename)) == NULL)
		{
		    fprintf(stderr, "Fatal error in displayPlot: "
			    "out of memory. Exiting.\n");
		    exit(EXIT_FAILURE);
		}

		/* Check magnification */
		if (keyUsed[MAGNIFICATION_INDEX])
		{
		    if (values->magnify <= 0.f)
		    {
			choice = DialogF(DF_ERR, parent, 2,
					 "Invalid value for\nheading \"%s\" at line %d",
					 "Cancel", "Continue", Headings[headingType], headingLineNum);
			ReportTaskCompletion(taskNumber, 1, "invalid magnification value");
			destroyOverlayedObjectList(obj, window->widget);
			return choice != 1;
		    }
		    obj->item.p.scale = values->magnify;
		}

		/* Check border color */
		if (keyUsed[FGCOLOR_INDEX])
		{
		    colorName = values->startColor;
		    colorValue = &obj->item.p.bordercolor;
		    processColorDefinition;
		    obj->item.p.options |= XYSTRING_BORDERMASK;
		}

		/* Check background color. This option is not really
		 * supported yet because ghostscript seems to fill
		 * the pixmap with white no matter what.
		 */
		if (keyUsed[BGCOLOR_INDEX])
		{
		    colorName = values->endColor;
		    colorValue = &obj->item.p.background;
		    processColorDefinition;
		    obj->item.p.options |= XYSTRING_FILLMASK;
		}

		/* Check position */
		if (!values->pointsSpecified[0])
		{
		    choice = DialogF(DF_ERR, parent, 2,
				     "Coordinates are not specified for\nheading \"%s\" at line %d",
				     "Cancel", "Continue", Headings[headingType], headingLineNum);
		    ReportTaskCompletion(taskNumber, 1, "missing coordinates");
		    destroyOverlayedObjectList(obj, window->widget);
		    return choice != 1;
		}
		obj->item.p.position = values->points[0];

		/* Draw rectangle border? */
		if (keyUsed[BORDER_INDEX])
		    obj->item.p.options |= XYSTRING_BORDERMASK;

		/* Check reference */
		if (keyUsed[REFPOINT_INDEX])
		    obj->item.p.reference = values->refpoint;

		/* Check alignment */
		obj->item.p.alignment = DEFAULT_ALIGNMENT;
		if (keyUsed[ALIGNMENT_INDEX])
		    obj->item.p.alignment = values->alignment;

		/* Remove the file when drawing is done? */
		if (keyUsed[UNLINK_INDEX])
		    obj->item.p.options |= PSPIXMAP_UNLINKMASK;
	    }

	    /* Check if this is the "Draw" command */
	    else if (overlayedType == OVER_DRAW)
	    {
		obj->item.d.linecolor = black;
		obj->item.d.fillcolor = black;
		obj->item.d.linestyle = values->lineStyles[0];

		/* Check that the item to draw is specified */
		if (keyUsed[CATEGORY_INDEX])
		{
		    obj->item.d.category = drawingObjectType(values->category);
		    if (obj->item.d.category < 0)
		    {
			choice = DialogF(DF_ERR, parent, 2,
			    "Invalid category for\nheading \"%s\" at line %d",
			    "Cancel", "Continue", Headings[headingType], headingLineNum);
			ReportTaskCompletion(taskNumber, 1, "invalid category");
			destroyOverlayedObjectList(obj, window->widget);
			return choice != 1;
		    }
		    needPoints = drawingObjectMinPoints(obj->item.d.category);
		    if (obj->item.d.category == DRAW_LINE && keyUsed[ARROWSTYLE_INDEX])
		    {
			if (values->arrowstyle == 1)
			    obj->item.d.options |= FORWARD_ARROW_MASK;
			else if (values->arrowstyle == 2)
			    obj->item.d.options |= BACKWARD_ARROW_MASK;
			else if (values->arrowstyle == 3)
			    obj->item.d.options |= (FORWARD_ARROW_MASK | 
						    BACKWARD_ARROW_MASK);
		    }
		}
		else
		{
		    choice = DialogF(DF_ERR, parent, 2,
				     "Category is not specified for\nheading \"%s\" at line %d",
				     "Cancel", "Continue", Headings[headingType], headingLineNum);
		    ReportTaskCompletion(taskNumber, 1, "missing category");
		    destroyOverlayedObjectList(obj, window->widget);
		    return choice != 1;
		}
		for (i=0; i<MAX_DRAWING_POINTS; ++i)
		{
		    if (!values->pointsSpecified[i])
			break;
		    else
			++obj->item.d.npoints;
		}
		if (obj->item.d.npoints < needPoints)
		{
		    choice = DialogF(DF_ERR, parent, 2,
				     "Not all points specified for\nheading \"%s\" at line %d",
				     "Cancel", "Continue", Headings[headingType], headingLineNum);
		    ReportTaskCompletion(taskNumber, 1, "incomplete item specification");
		    destroyOverlayedObjectList(obj, window->widget);
		    return choice != 1;
		}
		obj->item.d.points = (DrawingPoint *)malloc(
		    obj->item.d.npoints*sizeof(DrawingPoint));
		if (obj->item.d.points == NULL)
		{
		    fprintf(stderr, "Fatal error in displayPlot: "
			    "out of memory. Exiting.\n");
		    exit(EXIT_FAILURE);
		}
		for (i=0; i<obj->item.d.npoints; ++i)
		    obj->item.d.points[i] = values->points[i];

		/* Check the reference point */
		if (keyUsed[REFPOINT_INDEX])
		    obj->item.d.reference = values->refpoint;

		/* Check item colors */
		if (keyUsed[FGCOLOR_INDEX])
		{
		    colorName = values->startColor;
		    colorValue = &obj->item.d.linecolor;
		    processColorDefinition;
		    obj->item.d.options |= XYSTRING_BORDERMASK;
		}
		if (keyUsed[BGCOLOR_INDEX])
		{
		    colorName = values->endColor;
		    colorValue = &obj->item.d.fillcolor;
		    processColorDefinition;
		    obj->item.d.options |= XYSTRING_FILLMASK;
		}
	    }

	    else
	    {
		/* Unknown overlayed type, This is a bug. */
		assert(0);
	    }

	    /* Add object to the list */
	    if (window->decor == NULL)
		window->decor = obj;
	    else
	    {
		listPtr = window->decor;
		while (listPtr->next)
		    listPtr = listPtr->next;
		listPtr->next = obj;
	    }
	    if (!keyUsed[NOREDRAW_INDEX])
		/* The next command will update the plot */
		safeRedisplay(window);
	    ReportTaskCompletion(taskNumber, 0, NULL);
	    return True;
	}

	/* Process commands which have to be executed in a callback */
	/* Prepare the callback */
	readycall = (readyCallBack *)XtMalloc(sizeof(readyCallBack));
	readycall->data[0] = '\0';
	readycall->data[511] = '\0';
	readycall->taskNumber = taskNumber;

	/* Do different things depending on the action */
	if (strcmp(actionWord, "Close") == 0)
	{
	    if (i == 1)
		readycall->callback = ClosePlotWindowById;
	    else
		readycall->callback = CloseMPlotWindowById;
	}
	else if (strcmp(actionWord, "Save") == 0)
	{
	    /* Trim the file name */
	    for (filename = values->windowAct + 4;
		 *filename && isspace(*filename); ++filename);
	    if (*filename)
	    {
		for (s_end = filename + strlen(filename) - 1;
		     isspace(*s_end) && (s_end > filename); --s_end)
		    *s_end = '\0';
		strncpy(readycall->data, filename, 511);
	    }
	    if (i == 1)
		readycall->callback = GeneratePlotPSById;
	    else
		readycall->callback = GenerateMPlotPSById;
	}
	else
	{
	    XtFree((char *)readycall);
	    choice = DialogF(DF_ERR, parent, 2,
	       "Unknown action \"%s\" is specified for\nheading \"%s\" at line %d",
	       "Cancel", "Continue", values->windowAct, Headings[headingType],
	       headingLineNum);
	    ReportTaskCompletion(taskNumber, 1, "invalid window action");
	    return choice != 1;
	}

	/* The action seems to be correct. Add the callback. */
	if (i == 1) {
	    add_to_list(window->readyCB, readycall);
	    RunRefreshCycle(window);
	} else {
	    add_to_list(multiPlotWin->readyCB, readycall);
	    MPlotRefreshCycle();
	}
	return True;
    }

    else if (headingType == CONFIG_HEADING)
    {
	/* Minimum PostScript line width */
	if (keyUsed[MINPSLINEWIDTH_INDEX])
	    PSSetMinimumLineWidth(values->minPSlineWidth);

	/* Update period */
	if (keyUsed[UPDATEFREQUENCY_INDEX])
	    if (values->updateFrequency > 0)
		SetUpdateFreq(values->updateFrequency);

	/* Buffer graphics to pixmaps? */
	if (keyUsed[BUFFERGRAPHICS_INDEX])
	    SetBufferGraphicsBtnState(values->bufferGraphics);

	/* Thicken scatter plot points? */
	if (keyUsed[THICKENSCAT_INDEX])
	    setThickenPointsScat(values->thickenPointsScat);

	/* Thicken 3d scatter plot points? */
	if (keyUsed[THICKENSCAT3D_INDEX])
	    setThickenPointsScat3D(values->thickenPointsScat3D);

	/* Show automatic plot help? */
	if (keyUsed[AUTOMATICHELP_INDEX])
	    SetAutoHelpBtnState(values->autohelp);

        /* Allow printing of mouse coordinates? */
        if (keyUsed[PRINTCOORDS_INDEX])
            AllowCoordPrint(values->printCoords);

        /* Change the error marker width? */
        if (keyUsed[ERRMARKER_INDEX])
            XYSetErrorMarkerRatio(values->errMarkerRatio);

        /* Enable drawing arrows on axes ? */
        if (keyUsed[LEFTARROWS_INDEX])
            enableLeftArrow(values->enableLeftArr);
        if (keyUsed[RIGHTARROWS_INDEX])
            enableRightArrow(values->enableRightArr);
        if (keyUsed[TOPARROWS_INDEX])
            enableTopArrow(values->enableTopArr);
        if (keyUsed[BOTTOMARROWS_INDEX])
            enableBottomArrow(values->enableBottomArr);

	/* Include plot titles into PostScript files? */
	if (keyUsed[NAME_INDEX])
	{
	    if (strcmp(values->name, "1") == 0)
		SetTitlePSOnOrOff(True);
	    else if (strcmp(values->name, "0") == 0)
		SetTitlePSOnOrOff(False);
	}

	/* Set default X-windows font for comments? */
	if (keyUsed[FONT_INDEX])
	{
	    XmFontList font = XmFontListAppendEntry(NULL, fontEntry);
	    setDefaultCommentFont(font, NULL, 0);
	    if (font)
		XmFontListFree(font);
	}

	/* Set default PostScript font for comments? This may also be
	   a special command which sets PS font for titles */
	if (keyUsed[PSFONT_INDEX])
	{
	    int special = 0;
	    if (keyUsed[NAME_INDEX]) {
		if (strcmp(values->name, "WinTitleFont") == 0) {
		    special = 1;
		    if (values->psFontName && values->psFontSize > 0) {
			SetTitlePSFont(values->psFontName);
			SetTitlePSFontSize(values->psFontSize);
		    }
		} else if (strcmp(values->name, "MplotTitleFont") == 0) {
		    special = 1;
		    if (values->psFontName && values->psFontSize > 0) {
			SetMplotTitlePSFont(values->psFontName);
			SetMplotTitlePSFontSize(values->psFontSize);
		    }
		}
	    }
	    if (!special) {
		setDefaultCommentFont(NULL, values->psFontName, values->psFontSize);
	    }
	}

	return True;
    }

    else if (headingType == COMMAND_HEADING)
    {
	if (!keyUsed[WINDOWACT_INDEX])
	{
	    choice = DialogF(DF_ERR, parent, 2,
	       "Action is required for\nheading \"%s\" at line %d",
	       "Cancel", "Continue", Headings[headingType], headingLineNum);
	    return choice != 1;
	}
	for (i=0; i<MAX_DISP_VARS; ++i)
	    if (!values->varsSpecified[i])
		break;
	if (executeHistoCommand(values->windowAct, taskNumber,
				i, values->vars, &errMsg) == False)
	{
	    if (errMsg[0] != '\0')
	    {
		choice = DialogF(DF_ERR, parent, 2, "%s\nfor heading %s at line %d",
				 "Cancel", "Continue", errMsg, Headings[headingType],
				 headingLineNum);
		return choice != 1;
	    }
	    else
		return False;
	}
	return True;
    }

    /* Make sure all of the required information has been gathered */
    if (!(keyUsed[UID_INDEX] || keyUsed[NAME_INDEX] || keyUsed[HBOOKID_INDEX])) {
        choice = DialogF(DF_ERR, parent, 2,
           "UID, HBOOKID, or Title are required for\nheading \"%s\" at line %d",
           "Cancel", "Continue", Headings[headingType], headingLineNum);
    	return choice != 1;
    }

    /* Make sure only UID or HBOOKID, but not both, have been specified */
    if ((keyUsed[UID_INDEX] && keyUsed[HBOOKID_INDEX])) {
        choice = DialogF(DF_ERR, parent, 2,
           "Both UID and HbookID specified for\nheading \"%s\" at line %d",
           "Cancel", "Continue", Headings[headingType], headingLineNum);
    	return choice != 1;
    }

    /* If the item is an overlay plot, a user error which could cause trouble
       below is specifying both overlayID and inOverlay for the same plot */
    if (keyUsed[INOVERLAY_INDEX] && keyUsed[OVERLAY_INDEX]) {
    	choice = DialogF(DF_ERR, parent, 2,
        "Both OverlayID and INOverlay specified for\nheading \"%s\" at line %d",
              "Cancel", "Continue", Headings[headingType], headingLineNum);
    	return choice != 1;
    }

    /* Axis labels should be used together with overlayID */
    if (!keyUsed[OVERLAY_INDEX] && (keyUsed[XLABEL_INDEX] || keyUsed[YLABEL_INDEX])) {
    	choice = DialogF(DF_ERR, parent, 2,
        "Axis label specified without OverlayID for\nheading \"%s\" at line %d",
              "Cancel", "Continue", Headings[headingType], headingLineNum);
    	return choice != 1;
    }    

    /* Get the histoscope data item and complain if it's missing or wrong */
    if (!keyUsed[CATEGORY_INDEX])
    	values->category[0] = '\0';
    if (keyUsed[UID_INDEX]) {
    	item = GetMPItemByUID(values->category, values->uid);
    	/* V3.1.1 wrote the HBOOK ID as a UID, so must try UID as an HbookID */
    	if (item == NULL)	/* to read V3.1.1 Configuration files */
    	    item = GetMPItemByHbookID(values->category, values->uid);
    }
    else if (keyUsed[HBOOKID_INDEX])
    	item = GetMPItemByHbookID(values->category, values->hbookID);
    else
    	item = GetMPItemByName(values->category, values->name);
    if (item == NULL) {
    	 choice = DialogF(DF_ERR, parent, 2,
    	 	"No item matched %s for heading \"%s\" at line %d", "Cancel",
    	 	"Continue", keyUsed[UID_INDEX]?"UID":"Title",
    	 	Headings[headingType], headingLineNum);
        return choice != 1;
    }
    if (item->type != HeadingItemTypes[headingType]) {
    	choice = DialogF(DF_ERR, parent, 3,
    	   "Heading (%s) does not agree\nwith type of item matched at line %d",
    	   "Cancel", "Skip", "Continue", Headings[headingType], headingLineNum);
    	if (choice == 1)
    	    return False;
    	else if (choice == 2)
    	    return True;
    }
    
    /* Load in the data for the item if it is not already loaded */
    if (!ItemHasData(item))
    	if (!LoadItemData(parent, item))
    	    return False;
    
    /* If the item is an ntuple, read in the variable list and convert it
       to a numeric list of variable indecies and of slider indecies */
    if (item->type == HS_NTUPLE) {
    	if (!convertVarNamesToIndecies((hsNTuple *)item, values, varList,
    		sliderList, &errVar)) {
    	    choice = DialogF(DF_ERR, parent, 2,
"Variable name %s not found for\nheading %s at line %d.  (User\n\
defined variables must be specified\nbefore they can be referenced)",
    	    	    "Cancel", "Continue", errVar, Headings[headingType],
    	    	    headingLineNum);
             return choice != 1;
    	}
    }

    /* If the plot is part of a multi-plot window, make sure that it exists,
       that the row and column are specified, and that they make sense */
    if (keyUsed[INMULTIPLOT_INDEX]) {
	multiPlotWin = lookUpMultiplot(multiPlotIDs, values->multiPlotID);
	if (multiPlotWin == NULL) {
    	    choice = DialogF(DF_ERR, parent, 2,
        	  "Multi-plot window %d not defined\nheading \"%s\" at line %d",
        	  "Cancel", "Continue", values->multiPlotID,
        	  Headings[headingType], headingLineNum);
    	    return choice != 1;
	}
	if (!keyUsed[ROW_INDEX] || !keyUsed[COLUMN_INDEX]) {
	    choice = DialogF(DF_ERR, parent, 2,
"Row and Column are required for\nplots included in multi-plot windows\n\
heading \"%s\" at line %d", "Cancel", "Continue", Headings[headingType],
		    headingLineNum);
	    return choice != 1;
	}
	if (values->row <= 0 || values->row > multiPlotWin->numRows ||
		values->col <= 0 || values->col > multiPlotWin->numCols) {
	    choice = DialogF(DF_ERR, parent, 2,
"Row and Column not appropriate\nfor Multi-plot window %d\n\
heading \"%s\" at line %d", "Cancel", "Continue", values->multiPlotID,
        	  Headings[headingType], headingLineNum);
    	    return choice != 1;
	}
    }

    /* Take care of the color scale information. Note overloaded
       use of ZMINLIMIT, ZMAXLIMIT, and LOGZ keywords. */
    if (keyUsed[COLORSCALENAME_INDEX])
	csData.colorScale = values->clrScale;
    else
	csData.colorScale = defaultColorScale();
    if (csData.colorScale)
    {
	csInfo = &csData;
	if (keyUsed[ZMINLIMIT_INDEX])
	    csData.colorMin = values->scale.zMinLimit;
	else
	    csData.colorMin = FLT_MAX;  /* Means recalculate fom data */
	if (keyUsed[ZMAXLIMIT_INDEX])
	    csData.colorMax = values->scale.zMaxLimit;
	else
	    csData.colorMax = -FLT_MAX; /* Means recalculate fom data */
	if (keyUsed[LOGZ_INDEX])
	    csData.colorIsLog = 1;
	else
	    csData.colorIsLog = 0;
	if (keyUsed[DYNAMICCOLOR_INDEX])
	    csData.rangeIsDynamic = 1;
	else
	    csData.rangeIsDynamic = 0;
    }
    else
	csInfo = NULL;

    /* Take care of the font information */
    if (keyUsed[FONT_INDEX])
    {
	XmFontList font = XmFontListAppendEntry(NULL, fontEntry);
	if (font)
	{
	    XtSetArg(confInfo.args[confInfo.nargs], XmNfontList, font);
	    ++confInfo.nargs;
	}
    }
    if (keyUsed[PSFONT_INDEX])
    {
	psFontResource = XmStringCreateSimple(values->psFontName);
	XtSetArg(confInfo.args[confInfo.nargs], XmNpsFont, psFontResource);
	++confInfo.nargs;
	XtSetArg(confInfo.args[confInfo.nargs], XmNpsFontSize, values->psFontSize);
	++confInfo.nargs;
    }

    /* Set additional plot margins */
    XtSetArg(confInfo.args[confInfo.nargs], XmNaddLeftMargin, values->addLeft);
    ++confInfo.nargs;
    XtSetArg(confInfo.args[confInfo.nargs], XmNaddRightMargin, values->addRight);
    ++confInfo.nargs;
    XtSetArg(confInfo.args[confInfo.nargs], XmNaddTopMargin, values->addTop);
    ++confInfo.nargs;
    XtSetArg(confInfo.args[confInfo.nargs], XmNaddBottomMargin, values->addBottom);
    ++confInfo.nargs;

    /* Put up the plot */
    assert(confInfo.nargs <= (int)(sizeof(confInfo.args)/sizeof(confInfo.args[0])));
    if (keyUsed[INOVERLAY_INDEX]) {
    	window = findOverlayWindow(values->overlayID);
    	OverlayPlot(window, item,
    	    	item->type == HS_1D_HISTOGRAM ? HIST1D : values->plotType,
    	    	varList, sliderList,
    		(keyUsed[NBINS_INDEX] || keyUsed[NXBINS_INDEX] ||
	    	keyUsed[BINLIMIT_INDEX]) ? values->nXBins : 0,
	    	keyUsed[NYBINS_INDEX] ? values->nYBins : 0,
	    	keyUsed[DATAERROR_INDEX] ? DATA_ERROR_BARS :
	    	(keyUsed[GAUSSERROR_INDEX] ? GAUSSIAN_ERROR_BARS :
	    	NO_ERROR_BARS), NULL, 0, NULL, csInfo, &confInfo);
	pInfo = window->pInfo[window->nPlots - 1];
    } else if (keyUsed[INMULTIPLOT_INDEX]) {
	if (item->type == HS_NTUPLE) {
    	    window = AddNtupleToMultiPlot(multiPlotWin, (hsNTuple *)item,
    	    	    values->row-1, values->col-1, values->plotType, varList,
    	    	    sliderList, csInfo, &confInfo, &errMsg);
     	} else /* non-ntuple */
    	    window = AddHistToMultiPlot(multiPlotWin, item, values->row-1,
			      values->col-1, NULL, headingType, 0, csInfo, &confInfo);
    	pInfo = window->pInfo[0];
    } else {
	geometry = keyUsed[GEOMETRY_INDEX] ? values->geometry : NULL;
    	if (item->type == HS_NTUPLE)
    	    window = ViewNtuplePlot(XtDisplay(parent), (hsNTuple *)item, winID,
				    values->plotType, varList, sliderList,
				    geometry, csInfo, &confInfo, &errMsg);
    	else
    	    window = ViewItem(XtDisplay(parent), item, winID, geometry,
			      headingType, csInfo, &confInfo);
        pInfo = window->pInfo[0];
    }
    if (psFontResource)
	XmStringFree(psFontResource);
    if (window == NULL) {
    	choice = DialogF(DF_ERR, parent, 2, "%s\nfor heading %s at line %d",
    	    	"Cancel", "Continue", errMsg, Headings[headingType],
    	    	headingLineNum);
        return choice != 1;
    }
    
    /* Set per-plot information: slider values if specified, binning for
       histograms */
    for (i=0; i<N_SLIDERS; i++) {
    	if (values->sliderThreshSpecified[i]) {
    	    if (values->sliderGT[i])
    	    	pInfo->sliderGTorLT[i] = SLIDER_GT;
    	    pInfo->sliderThresholds[i] = values->sliderThresh[i];
    	    RefreshItem(pInfo->id);
    	}
    }
    if (keyUsed[NBINS_INDEX] || keyUsed[NXBINS_INDEX] ||
	    keyUsed[BINLIMIT_INDEX]) {
    	if (pInfo->nXBins != values->nXBins) {
    	    pInfo->nXBins = values->nXBins;
    	    rebin = True;
    	}
    }
    if (keyUsed[NYBINS_INDEX]) {
    	if (pInfo->nYBins != values->nYBins) {
    	    pInfo->nYBins = values->nYBins;
    	    rebin = True;
    	}
    }
    
    /* Set cell normalization information */
    if (keyUsed[NORMMIN_INDEX]) {
    	window->cellNormMin = values->normMin;
    	rebin = True;
    }
    if (keyUsed[NORMMAX_INDEX]) {
    	window->cellNormMax = values->normMax;
    	rebin = True;
    }
    if (rebin)
        RedisplayPlotWindow(window, REBIN);
    
    /* Set Window Title if WindowTitle keyword was used */
    if (keyUsed[WINDOWNAME_INDEX] && values->windowTitle[0] != '\0')
    	SET_ONE_RSRC(window->shell, XmNtitle, values->windowTitle);
    
    /* Set miscelaneous toggle buttons */
    if (keyUsed[DARKEN_INDEX])
    	setMenuToggle(window->thickenMenuItem, True, True);
    if (keyUsed[BINEDGE_INDEX])
    	setMenuToggle(window->binEdgeMenuItem, True, True);
    if (keyUsed[BACKPLANES_INDEX])
    	setMenuToggle(window->backplanesMenuItem, False, True);
    if (keyUsed[HIDELEGEND_INDEX])
    	setMenuToggle(window->legendMenuItem, False, True);
    if (keyUsed[CNTROFGRAV_INDEX]) {
    	if (keyUsed[INOVERLAY_INDEX]) {
    	    pInfo->aHistBinStrategy = CENTER_OF_GRAVITY;
    	    RefreshItem(pInfo->id);
    	} else
    	    setMenuToggle(window->cntrOfGravMenuItem, True, True);
    }
    if (keyUsed[GROWONLY_INDEX])
    	setMenuToggle(window->growOnlyMenuItem, True, True);

    /* Set plot limits, rotation, and log scaling if requested.  This is not
       done immediately if the data is not loaded, because loading the data
       will then reset everything.  Instead, it is deferred until at least
       some of the data is available. */
    if (ItemHasData(item))
    	setPlotScale(window->widget, keyUsed, &values->scale);
    else
    	setPlotScaleDeferred(pInfo->id, window->widget, keyUsed,&values->scale);

    /* Set marker and line styles and colors for XY plots */
    styleChanged = False;
    if (XtClass(window->widget)==xyWidgetClass && window->curveStyles!=NULL) {
    	XYCurve *style, *styles = &((XYCurve *)window->curveStyles)
    	    	[window->nCurves-NCurvesInPlot(pInfo->plotType, pInfo->ntVars)];
    	for (i=0, style=styles; i<MAX_DISP_VARS; i++, style++) {
    	    if (values->markStyles[i] != (char)-1) {
    	    	style->markerStyle = values->markStyles[i];
    	    	styleChanged = True;
    	    }
    	    if (values->lineStyles[i] != (char)-1) {
    	    	style->lineStyle = values->lineStyles[i];
    	    	styleChanged = True;
    	    }
    	    if (values->markSizes[i] != (char)-1) {
    	    	style->markerSize = values->markSizes[i];
    	    	styleChanged = True;
    	    }
    	    /* Allocate color and set pixel value in curves structure */
    	    if (values->markColors[i][0] != '\0') {
		if (strcmp(values->markColors[i], "black")) {
		    style->markerPixel = allocColor(
			values->markColors[i],
			XtDisplay(window->widget),
			XScreenNumberOfScreen(XtScreen(window->widget)));
		    styleChanged = True;
		}
    	    }
    	    if (values->lineColors[i][0] != '\0') {
		if (strcmp(values->lineColors[i], "black")) {
		    style->linePixel = allocColor(
			values->lineColors[i],
			XtDisplay(window->widget),
			XScreenNumberOfScreen(XtScreen(window->widget)));
		    styleChanged = True;
		}
	    }
	}
    }
    if (XtClass(window->widget)==xyWidgetClass && window->histStyles!=NULL &&
    	    PlotIsHist(pInfo->plotType) != 0) {
    	XYHistogram *style = &((XYHistogram *)window->histStyles)
    	    	[window->nHists-1];
    	if (values->fillStyles[0] != -1) {
    	    style->fillStyle = values->fillStyles[0];
    	    styleChanged = True;
    	}
    	if (values->lineStyles[0] != (char)-1) {
    	    style->lineStyle = values->lineStyles[0];
    	    styleChanged = True;
    	}
    	/* Allocate color and set pixel value in curves structure */
    	if (values->fillColors[0][0] != '\0') {
	    if (strcmp(values->fillColors[0], "black")) {
		style->fillPixel = allocColor(
		    values->fillColors[0],
    	    	    XtDisplay(window->widget),
    	    	    XScreenNumberOfScreen(XtScreen(window->widget)));
		styleChanged = True;
	    }
    	}
    	if (values->lineColors[0][0] != '\0') {
	    if (strcmp(values->lineColors[0], "black")) {
		style->linePixel = allocColor(
		    values->lineColors[0],
		    XtDisplay(window->widget),
		    XScreenNumberOfScreen(XtScreen(window->widget)));
		styleChanged = True;
	    }
	}
    }
    if (styleChanged)
	RefreshItem(window->pInfo[0]->id);

    /* Add error bars if requested */
    if (keyUsed[DATAERROR_INDEX])
    	ShowErrorBars(window, DATA_ERROR_BARS, 0);
    if (keyUsed[GAUSSERROR_INDEX])
    	ShowErrorBars(window, GAUSSIAN_ERROR_BARS, 0);
    	
    /* Put up auxiliary windows */
    if (keyUsed[SLIDERWIN_INDEX]) {
    	if (window->sliderWindow == NULL)
    	    CreateSliderWindow(window, values->sliderWinGeom);
    	XtManageChild(window->sliderWindow);
    	XmToggleButtonSetState(window->sliderMenuItem, True, False);
    }
    if (keyUsed[REBINWIN_INDEX]) {    
    	if (window->rebinWindow == NULL) {
    	    if (values->plotType == ADAPTHIST1D ||
    	    	    values->plotType == ADAPTHIST2D)
    	    	CreateBinLimitWindow(window, values->rebinWinGeom);
    	    else if (values->plotType == HIST1D)
    	    	CreateRebinWindow(window, 1, values->rebinWinGeom);
    	    else
    	    	CreateRebinWindow(window, 2, values->rebinWinGeom);
    	}
    	XtManageChild(window->rebinWindow);
    	XmToggleButtonSetState(window->rebinMenuItem, True, False);
    }
    if (keyUsed[CELLNORMWIN_INDEX]) {    
    	if (window->cellNormWindow == NULL)
    	    CreateCellNormalizeWindow(window, values->cellNormWinGeom);
    	XtManageChild(window->cellNormWindow);
    	XmToggleButtonSetState(window->cellNormMenuItem, True, False);
    }
    if (keyUsed[STATSWIN_INDEX]) {    
    	if (window->statsWindow == NULL)
    	    CreateStatsWindow(window, values->statsWinGeom);
    	XtManageChild(window->statsWindow);
    	XmToggleButtonSetState(window->statsMenuItem, True, False);
    }

    /* If the window had an overlayID keyword, remember it for later
       plots to be overlaid on top */
    if (keyUsed[OVERLAY_INDEX])
    {
    	saveOverlayWindow(window, values->overlayID);
	if (keyUsed[XLABEL_INDEX])
	    strcpy(window->xlabel, values->xlabel);
	if (keyUsed[YLABEL_INDEX])
	    strcpy(window->ylabel, values->ylabel);
    }

    return True;
}

/*
** Display an empty multi-plot window, using the keyword values read from
** the configuration file.  "headingLineNum" is the line number to show the
** user if something goes wrong.  multiPlotIDs is an array for mapping the
** multiPlot window IDs used to identify these window in the configuration
** file to the actual window it corresponds to. Return True if the
** item was read or skipped successfully, False to give up reading the file.
*/
static int displayMultiPlotWindow(Widget parent, keywordValues *values,
	int headingLineNum, multiPlotIDRec *multiPlotIDs)
{
    char *geometry, *name, *winID;
    char *keyUsed = values->specified;
    int choice;
    multWindow *window;

    /* Make sure all required keywords are there and have reasonable values */
    if (!(keyUsed[ROWS_INDEX] && keyUsed[COLUMNS_INDEX] &&
    	    keyUsed[WINDOWID_INDEX])) {
    	choice = DialogF(DF_ERR, parent, 2,
             "Rows, Columns, and WindowID must\nbe "
             "specified for heading\n\"%s\" at line %d",
             "Cancel", "Continue", Headings[MULTIPLOT_HEADING], headingLineNum);
        return choice != 1;
    }
    if (values->row <= 0 || values->row > 50 ||
    	    values->col <= 0 || values->col > 50) {
    	choice = DialogF(DF_ERR, parent, 2,
	     "Bad Rows or Columns value under heading\n\"%s\" at line %d",
             "Cancel", "Continue", Headings[MULTIPLOT_HEADING], headingLineNum);
        return choice != 1;
    }
    
    /* Put up the window */
    geometry = keyUsed[GEOMETRY_INDEX] ? values->geometry : NULL;
    name = keyUsed[WINDOWNAME_INDEX] ? values->windowTitle : "Untitled";
    winID = keyUsed[WINDOWLOC_INDEX] ? values->windowLoc : NULL;
    window = CreateEmptyMultPlot(parent, name, winID, values->row,
				 values->col, geometry);
    if (keyUsed[HIDETITLES_INDEX])
    	window->dispLabels = False;
    	
    /* Associate the ID with the winndow */
    addMultiPlotID(multiPlotIDs, values->multiPlotID, window);
    return True;
}

/*
** Use the keyword values read from the configuration file (in the "values"
** parameter) to construct an ntuple extension describing user defined
** variables associated with a specific ntuple
*/
static int enterNtupleExtension(Widget parent, keywordValues *values,
	int headingLineNum)
{
    hsGeneral *item;
    hsNTuple *ntuple;
    char *errMsg, *errVar, *name, *expression;
    Program *program;
    int choice;
    int i, failedAt;
    ntupleExtension *ntExt;
    
    /* Make sure all of the required information has been gathered */
    if (!(values->specified[UID_INDEX] || values->specified[NAME_INDEX])) {
        choice = DialogF(DF_ERR, parent, 2,
           "Either UID or Title are required for\nntuple extensions (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;
    }

    /* Get the ntuple and complain if it's missing or wrong */
    if (!values->specified[CATEGORY_INDEX])
    	values->category[0] = '\0';
    if (values->specified[UID_INDEX])
    	item = GetMPItemByUID(values->category, values->uid);
    else
    	item = GetMPItemByName(values->category, values->name);
    if (item == NULL) {
    	choice = DialogF(DF_ERR, parent, 2,
    		"No item matched %s for ntuple extension at line %d", "Cancel",
    		"Continue", values->specified[UID_INDEX]?"UID":"Title",
		headingLineNum);
        return choice != 1;
    }
    if (item->type != HS_NTUPLE) {
    	choice = DialogF(DF_ERR, parent, 3,
    		 "Item must match an ntuple at line %d", "Cancel", "Skip",
    		 "Continue", headingLineNum);
    	if (choice == 1)
    	    return False;
    	else if (choice == 2)
    	    return True;
    }
    ntuple = (hsNTuple *)item;

    /* Make sure there's an expression for every name and visa versa */
    for (i=0; i<MAX_USER_VARS; i++) {
    	name = values->userVars[i].name;
    	expression = values->userVars[i].expression;
	if (name != NULL && expression == NULL) {
	    choice = DialogF(DF_ERR, parent, 3,
       "No expression corresponding to name\n%s in ntuple extension at line %d",
    	       	    "Cancel", "Skip Heading", "Continue", name, headingLineNum);
            if (choice == 1)
    	    	return False;
    	    else if (choice == 2)
    	    	return True;
    	    XtFree(name);
    	    values->userVars[i].name = NULL;
        } else if (name == NULL && expression != NULL) {
            choice = DialogF(DF_ERR, parent, 3,
       "No name corresponding to expression\n%s in ntuple extension at line %d",
    	       	    "Cancel", "Skip Heading", "Continue", expression,
    	       	    headingLineNum);
            if (choice == 1)
    	    	return False;
    	    else if (choice == 2)
    	    	return True;
    	    XtFree(expression);
    	    values->userVars[i].expression = NULL;
    	}
    }
    
    /* Check for existing names and warn if any will be overwritten */
    ntExt = GetNTupleExtension(ntuple->id);
    if (ntExt != NULL) {
	int var, replaceExisting = False, warnedExisting = False;
	for (i=0; i<MAX_USER_VARS; i++) {
	    if (values->userVars[i].name == NULL)
	        continue;
	    for (var=0; var<ntExt->nVars; var++) {
    		if (!strcmp(values->userVars[i].name, ntExt->vars[var].name)) {
    		    if (!warnedExisting) {
    			choice = DialogF(DF_WARN, parent, 2,
    		    		"Replace existing variables?", "No", "Yes");
    			replaceExisting = choice == 2;
    			warnedExisting = True;
    		    }
    		    if (!replaceExisting) {
    			XtFree(values->userVars[i].name);
    			values->userVars[i].name = NULL;
    			XtFree(values->userVars[i].expression);
    			values->userVars[i].expression = NULL;
		    }
		    break;
		}
	    }
	}
    }
    
    /* Make sure there are still variables left to process */
    for (i=0; i<MAX_USER_VARS; i++)
    	if (values->userVars[i].name != NULL)
    	    break;
    if (i >= MAX_USER_VARS)
    	return True;

    /* Create the user defined variables from the names and expressions stored
       by the reader.  Call the parser to check each expression and to create
       the program for the variable.  Note that the ownership of the memory
       allocated in the reader is now transferred to the extension manager */
    for (i=0; i<MAX_USER_VARS; i++) {
    	name = values->userVars[i].name;
    	expression = values->userVars[i].expression;
    	if (name == NULL || expression == NULL)
    	    continue;
    	program = ParseExpr(expression, &errMsg, &failedAt);
	if (program == NULL) {
            choice = DialogF(DF_ERR, parent, 3,
          "Error (%s) parsing expression\n  %s\nin ntuple extension at line %d",
    	       	    "Cancel", "Skip Heading", "Continue", errMsg,
    	       	    expression, headingLineNum);
            if (choice == 1)
    	    	return False;
    	    else if (choice == 2)
    	    	return True;
    	    else
    	    	continue;
	}
	AddUserVariable(ntuple, name, expression, program);
    }
    
    /* Once all of the derived variables are in place, resolve variable
       references.  No need to warn about undefined variables, since the
       expression is tested on the data with TestUserVariable below,
       and that complains about undefined variables too. */
    ntExt = GetNTupleExtension(ntuple->id);
    if (ntExt == NULL)
    	return True;
    if (!ItemHasData(item))
    	if (!LoadItemData(parent, item))
    	    return False;
    for (i=0; i<ntExt->nVars; i++)
    	ResolveVariableReferences(ntExt->vars[i].program, ntuple, ntExt);
    
    /* Check the new ntuple extension for circular references */
    while (CheckForCircularReferences(ntExt, &errVar)) {
    	DialogF(DF_ERR, parent, 1,
    	 "Variable %s in ntuple extension\nat line %d is circularly referenced",
    	 	"Dismiss", errVar, headingLineNum);
        DeleteUserVariable(ntuple, ntExt, errVar);
        ntExt = GetNTupleExtension(ntuple->id);
        if (ntExt == NULL)
            return True;
    }
    
    /* Test the expression on the current data and report arithmetic errors */
    for (i=0; i<ntExt->nVars; i++) {
    	if (!TestUserVariable(ntuple, ntExt, i, &errMsg)) {
    	    choice = DialogF(DF_ERR, parent, 1,
"At line %d, user defined variable\n%s fails with error:\n\n  %s\n\n\
(using currently loaded data)", "Dismiss", headingLineNum,
		    ntExt->vars[i].name, errMsg);
	}
    }
    
    /* Update displayed NTuple and variable panels with new variable names,
       and displayed plots with new data */
    UpdateNTuplePanelList(ntuple->id);
    UpdateVariablePanels(ntuple, ntExt);
    RefreshItem(ntuple->id);
    return True;
}

/*
** Get visible range and rotation angle for plots.  This data is maintained
** only within the plotting widgets.  Actually, figuring out whether the data
** represents a default setting or something the user has specifically set
** is more difficult than obtaining the settings themselves.  For the 3D
** widgets, this requires temporarily resetting the plots and then setting
** them back.  For the 2D widgets it is done by examining private widget data.
*/
static int getPlotLimits(Widget plotW, int *xMinDefault, double *xMinLim,
	int *xMaxDefault, double *xMaxLim, int *yMinDefault, double *yMinLim,
	int *yMaxDefault, double *yMaxLim, int *zMinDefault, double *zMinLim,
	int *zMaxDefault, double *zMaxLim, int *angle1Default, double *angle1,
	int *angle2Default, double *angle2, int *angle3Default, double *angle3)
{
    WidgetClass class = XtClass(plotW);
    
    *zMinDefault = *zMaxDefault = True;
    *angle2Default = *angle3Default = *angle1Default = True;
    if (class == scatWidgetClass) {
    	ScatPart *privData = &((ScatWidget)plotW)->scat;
    	ScatGetVisibleRange(plotW, xMinLim, yMinLim, xMaxLim, yMaxLim);
    	*xMinDefault = privData->minXData >= privData->minXLim;
    	*xMaxDefault = privData->maxXData <= privData->maxXLim;
    	*yMinDefault = privData->minYData >= privData->minYLim;
    	*yMaxDefault = privData->maxYData <= privData->maxYLim;
    } else if (class == h1DWidgetClass) {
    	H1DPart *privData = &((H1DWidget)plotW)->h1D;
    	H1DGetVisibleRange(plotW, xMinLim, yMinLim, xMaxLim, yMaxLim);
    	*xMinDefault = privData->minXData >= privData->minXLim;
    	*xMaxDefault = privData->maxXData <= privData->maxXLim;
    	*yMinDefault = privData->minYData >= privData->minYLim;
    	*yMaxDefault = privData->maxYData <= privData->maxYLim;
    } else if (class == xyWidgetClass) {
    	XYPart *privData = &((XYWidget)plotW)->xy;
    	XYGetVisibleRange(plotW, xMinLim, yMinLim, xMaxLim, yMaxLim);
    	*xMinDefault = privData->minXData >= privData->minXLim;
    	*xMaxDefault = privData->maxXData <= privData->maxXLim;
    	*yMinDefault = privData->minYData >= privData->minYLim;
    	*yMaxDefault = privData->maxYData <= privData->maxYLim;
    } else if (class == cellWidgetClass) {
    	CellPart *privData = &((CellWidget)plotW)->cell;
    	CellGetVisibleRange(plotW, xMinLim, yMinLim, xMaxLim, yMaxLim);
    	*xMinDefault = privData->minXData >= privData->minXLim;
    	*xMaxDefault = privData->maxXData <= privData->maxXLim;
    	*yMinDefault = privData->minYData >= privData->minYLim;
    	*yMaxDefault = privData->maxYData <= privData->maxYLim;
    } else if (class == hist2DWidgetClass) {
    	double defXMin, defXMax, defYMin, defYMax, defZMin, defZMax;
    	double defAngle1, defAngle2;
    	hist2DGetVisiblePart(plotW, xMinLim, xMaxLim, yMinLim, yMaxLim,
    		zMinLim, zMaxLim);
	/* Reset the widget to find out defaults and compare, then use
	   saved settings to restore.  This makes the widget flip back
	   and forth on the screen, which is unasthetic, but changing
	   the widget to figure out the defaults would be worse. */
	hist2DGetViewAngles(plotW, angle1, angle2);
	hist2DResetView(plotW);
	hist2DGetVisiblePart(plotW, &defXMin, &defXMax, &defYMin, &defYMax,
    		&defZMin, &defZMax);
    	hist2DGetViewAngles(plotW, &defAngle1, &defAngle2);
    	*xMinDefault = *xMinLim == defXMin;
    	*xMaxDefault = *xMaxLim == defXMax;
    	*yMinDefault = *yMinLim == defYMin;
    	*yMaxDefault = *yMaxLim == defYMax;
    	*zMinDefault = *zMinLim == defZMin;
    	*zMaxDefault = *zMaxLim == defZMax;
    	*angle1Default = *angle1 == defAngle1;
    	*angle2Default = *angle2 == defAngle2;
    	hist2DSetVisiblePart(plotW, *xMinLim, *xMaxLim, *yMinLim, *yMaxLim,
    		*zMinLim, *zMaxLim);
    	hist2DSetViewAngles(plotW, *angle1, *angle2);
    } else if (class == scat3DWidgetClass) {
    	float defXMin, defXMax, defYMin, defYMax, defZMin, defZMax;
    	double defAngle1, defAngle2, defAngle3;
    	float minX, maxX, minY, maxY, minZ, maxZ;
    	Scat3DGetVisiblePart(plotW, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);
    	*xMinLim = minX; *xMaxLim = maxX; *yMinLim = minY; *yMaxLim = maxY;
    		*zMinLim = minZ; *zMaxLim = maxZ;
	/* Reset the widget to find out defaults and compare, then use
	   saved settings to restore.  This makes the widget flip back
	   and forth on the screen, which is unasthetic, but changing
	   the widget to figure out the defaults would be worse. */
	Scat3DGetViewEulerAngles(plotW, angle1, angle2, angle3);
	Scat3DResetView(plotW);
	Scat3DGetVisiblePart(plotW, &defXMin, &defXMax, &defYMin, &defYMax,
    		&defZMin, &defZMax);
    	Scat3DGetViewEulerAngles(plotW, &defAngle1, &defAngle2, &defAngle3);
    	*xMinDefault = *xMinLim == defXMin;
    	*xMaxDefault = *xMaxLim == defXMax;
    	*yMinDefault = *yMinLim == defYMin;
    	*yMaxDefault = *yMaxLim == defYMax;
    	*zMinDefault = *zMinLim == defZMin;
    	*zMaxDefault = *zMaxLim == defZMax;
    	*angle1Default = *angle1 == defAngle1;
    	*angle2Default = *angle2 == defAngle2;
    	*angle3Default = *angle3 == defAngle3;
    	Scat3DSetVisiblePart(plotW, *xMinLim, *xMaxLim, *yMinLim, *yMaxLim,
    		*zMinLim, *zMaxLim);
    	Scat3DSetViewEulerAngles(plotW, *angle1, *angle2, *angle3);
    } else
    	return False;
    return True;
}

/*
** Find out if any axis of the plot widget "plotW" is set for log scaling.
*/
static void getLogSettings(Widget plotW, int *xLog, int *yLog, int *zLog)
{
    WidgetClass class = XtClass(plotW);

    *xLog = *yLog = *zLog = False;
    if (class == h1DWidgetClass)
    	XtVaGetValues(plotW, XmNlogScaling, yLog, NULL);
    else if (class == hist2DWidgetClass)
    	XtVaGetValues(plotW, XmNzLogScaling, zLog, NULL);
    else if (class == scat3DWidgetClass)
    	XtVaGetValues(plotW, XmNxLogScaling, xLog, XmNyLogScaling, yLog,
    	        XmNzLogScaling, zLog, NULL);
    else
    	XtVaGetValues(plotW, XmNxLogScaling, xLog, XmNyLogScaling, yLog, NULL);
}

/*
** Set the plot limits linear/log, and rotation for a plot widget.  Values for
** which the keyword was not specified are left to the widget's default
** setting.
*/
static void setPlotScale(Widget plotW, char *keyUsed, plotScaleInfo *scale)
{
    double origMinXLim, origMaxXLim, origMinYLim, origMaxYLim;
    double origMinZLim, origMaxZLim, origAngle1, origAngle2, origAngle3;
    WidgetClass class = XtClass(plotW);
    int xMinRead = keyUsed[XMINLIMIT_INDEX];
    int xMaxRead = keyUsed[XMAXLIMIT_INDEX];
    int yMinRead = keyUsed[YMINLIMIT_INDEX];
    int yMaxRead = keyUsed[YMAXLIMIT_INDEX];
    int zMinRead = keyUsed[ZMINLIMIT_INDEX];
    int zMaxRead = keyUsed[ZMAXLIMIT_INDEX];
    int angle1Read = keyUsed[FI_INDEX] || keyUsed[ALPHA_INDEX];
    int angle2Read = keyUsed[PSI_INDEX] || keyUsed[BETA_INDEX];
    int angle3Read = keyUsed[GAMMA_INDEX];
    
    /* Set log scaling */
    XtVaSetValues(plotW, XmNlogScaling, keyUsed[LOGY_INDEX],
    	    XmNxLogScaling, keyUsed[LOGX_INDEX],
    	    XmNyLogScaling, keyUsed[LOGY_INDEX],
    	    XmNzLogScaling, keyUsed[LOGZ_INDEX], NULL);

    /* If nothing new was specified, don't reset plot limits */
    if (!(xMinRead || xMaxRead || yMinRead || yMaxRead ||
    	    zMinRead || zMaxRead || angle1Read || angle2Read || angle3Read))
    	return;
    
    /* Get the default (original) plotting bounds the plot widget */
    if (class == scatWidgetClass) {
    	ScatGetVisibleRange(plotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    } else if (class == h1DWidgetClass) {
    	H1DGetVisibleRange(plotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    } else if (class == xyWidgetClass) {
    	XYGetVisibleRange(plotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    } else if (class == cellWidgetClass) {
    	CellGetVisibleRange(plotW, &origMinXLim, &origMinYLim,
    		&origMaxXLim, &origMaxYLim);
    } else if (class == hist2DWidgetClass) {
    	hist2DGetVisiblePart(plotW, &origMinXLim, &origMaxXLim,
    		&origMinYLim, &origMaxYLim, &origMinZLim, &origMaxZLim);
	hist2DGetViewAngles(plotW, &origAngle1, &origAngle2);
    } else if (class == scat3DWidgetClass) {
    	float minX, maxX, minY, maxY, minZ, maxZ;
    	Scat3DGetVisiblePart(plotW, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);
	Scat3DGetViewEulerAngles(plotW, &origAngle1, &origAngle2, &origAngle3);
	origMinXLim = minX; origMaxXLim = maxX; origMinYLim = minY;
	origMaxYLim = maxY; origMinZLim = minZ; origMaxZLim = maxZ;
    }
        
    /* If a limit was specified, replace the original one */
    if (xMinRead) origMinXLim = scale->xMinLimit;
    if (xMaxRead) origMaxXLim = scale->xMaxLimit;
    if (yMinRead) origMinYLim = scale->yMinLimit;
    if (yMaxRead) origMaxYLim = scale->yMaxLimit;
    if (zMinRead) origMinZLim = scale->zMinLimit;
    if (zMaxRead) origMaxZLim = scale->zMaxLimit;
    if (angle1Read) origAngle1 = scale->angle1;
    if (angle2Read) origAngle2 = scale->angle2;
    if (angle3Read) origAngle3 = scale->angle3;
    
    /* Set the new values in the widget */
    if (class == scatWidgetClass) {
    	ScatSetVisibleRange(plotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == h1DWidgetClass) {
    	H1DSetVisibleRange(plotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == xyWidgetClass) {
    	XYSetVisibleRange(plotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == cellWidgetClass) {
    	CellSetVisibleRange(plotW, origMinXLim, origMinYLim,
    		origMaxXLim, origMaxYLim);
    } else if (class == hist2DWidgetClass) {
    	hist2DSetVisiblePart(plotW, origMinXLim, origMaxXLim,
    		origMinYLim, origMaxYLim, origMinZLim, origMaxZLim);
	hist2DSetViewAngles(plotW, origAngle1, origAngle2);
    } else if (class == scat3DWidgetClass) {
    	Scat3DSetVisiblePart(plotW, origMinXLim, origMaxXLim,
    		origMinYLim, origMaxYLim, origMinZLim, origMaxZLim);
	Scat3DSetViewEulerAngles(plotW, origAngle1, origAngle2, origAngle3);
    }
}

/*
** readXXXX routines are readers for various argument types.  Readers
** for indexed keyword types (readVar, readSlider, readSliderThreshold)
** also look at the keyword itself to decide where to store their results.
*/
static int readGeom(char *line, char *geom, char **errMsg)
{
    char *arg;
    
    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, MAX_GEOMETRY_LEN, errMsg))
    	return False;
    
    /* Return the argument */
    strcpy(geom, arg);
    return True;
}

static int readLabel(char *line, char *label_string, char **errMsg)
{
    char *arg;

    /* Get the argument and check its length */
    if (!getArg(line, &arg, False, HS_MAX_LABEL_LENGTH, errMsg))
    	return False;

    /* Return the argument */
    strcpy(label_string, arg);
    return True;
}

static int readCategory(char *line, char *category, char **errMsg)
{    	
    char *arg;
    
    /* Get the argument and check its length */
    if (!getArg(line, &arg, False, HS_MAX_CATEGORY_LENGTH, errMsg))
    	return False;
    
    /* Return the argument */
    strcpy(category, arg);
    return True;
}

static int readShortString(char *line, char *name,
			   int len, char **errMsg)
{
    char *arg;

    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, len, errMsg))
    	return False;

    /* Return the argument */
    strcpy(name, arg);
    return True;
}

static int readName(char *line, char *name, char **errMsg)
{    	
    char *arg;

    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, HS_MAX_NAME_LENGTH, errMsg))
    	return False;
    
    /* Return the argument */
    strcpy(name, arg);
    return True;
}

static int readWinLoc(char *line, char *name, char **errMsg)
{    	
    char *arg;
    
    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, MAX_WINDOWID_LEN, errMsg))
    	return False;
    
    /* Return the argument */
    strcpy(name, arg);
    return True;
}

static int readPlotType(char *line, int *plotType, char **errMsg)
{
    char *arg;
    int i;
    
    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, MAX_PLOTTYPE_LEN, errMsg))
    	return False;
    	
    for (i=0; i<N_PLOT_TYPES; i++) {
    	if (!strcasecmp(PlotTypeNames[i], arg)) {
    	    *plotType = i;
    	    return True;
    	}
    }
    *errMsg = "Unknown plot type";
    return False;
}

static int readSliderThreshold(char *line, keywordValues *values,
			       int sliderGT, char **errMsg)
{
    float thresh;
    int index;
    
    if (!readFloatArg(line, &thresh, errMsg))
    	return False;
    
    index = getIndex(line, N_SLIDERS, errMsg);
    if (index == -1)
    	return False;
    	
    values->sliderThreshSpecified[index] = True;
    values->sliderThresh[index] = thresh;
    values->sliderGT[index] = sliderGT;
    return True;
}

static int readVar(char *line, keywordValues *values, char **errMsg)
{
    int index;
    char *arg;

    index = getIndex(line, MAX_DISP_VARS, errMsg);
    if (index == -1)
    	return False;

    if (!getArg(line, &arg, True, HS_MAX_NAME_LENGTH, errMsg))
    	return False;

    strcpy(values->vars[index], arg);
    values->varsSpecified[index] = True;
    return True;
}

static int readSlider(char *line, keywordValues *values, char **errMsg)
{
    int index;
    char *arg;
    
    index = getIndex(line, N_SLIDERS, errMsg);
    if (index == -1)
    	return False;
    
    if (!getArg(line, &arg, True, HS_MAX_NAME_LENGTH, errMsg))
    	return False;
    
    strcpy(values->sliders[index], arg);
    values->slidersSpecified[index] = True;
    return True;
}

static int readExprName(char *line, keywordValues *values, char **errMsg)
{
    int index;
    char *arg;
    
    index = getIndex(line, MAX_USER_VARS, errMsg);
    if (index == -1)
    	return False;
    
    if (!getArg(line, &arg, True, HS_MAX_NAME_LENGTH, errMsg))
    	return False;
    
    values->userVars[index].name = XtMalloc(sizeof(char)*(strlen(arg)+1));
    strcpy(values->userVars[index].name, arg);
    return True;
}

static int readExpr(char *line, keywordValues *values, char **errMsg)
{
    int index;
    char *arg;
    
    index = getIndex(line, MAX_USER_VARS, errMsg);
    if (index == -1)
    	return False;
    
    if (!getArg(line, &arg, True, INT_MAX, errMsg))
    	return False;

    values->userVars[index].expression = XtMalloc(sizeof(char)*(strlen(arg)+1));
    strcpy(values->userVars[index].expression, arg);
    return True;
}

static int readStyle(char *line, char *styleList, char **errMsg)
{
    int index, arg;
    
    index = getIndex(line, MAX_DISP_VARS, errMsg);
    if (index == -1)
    	return False;
    if (!readIntArg(line, &arg, errMsg))
    	return False;
    styleList[index] = arg;
    return True;
}

static int readFillStyle(char *line, int *styleList, char **errMsg)
{
    int index, i;
    char *arg;
    
    index = getIndex(line, MAX_DISP_VARS, errMsg);
    if (index == -1)
    	return False;
    if (!getArg(line, &arg, True, INT_MAX, errMsg))
    	return False;
    for (i=0; i<N_FILL_STYLES; i++)
    	if (!strcasecmp(HistFillStyles[i].name, arg))
    	    break;
    if (i == N_FILL_STYLES) {
    	*errMsg = "Unrecognized fill style name";
    	return False;
    }
    styleList[index] = HistFillStyles[i].style;
    return True;
}

static int readColor(char *line, char *colorList, char **errMsg)
{
    char *arg;
    int index;
    
    /* Get the argument for color */
    if (!getArg(line, &arg, True, HS_MAX_NAME_LENGTH, errMsg))
    	return False;
    index = getIndex(line, MAX_DISP_VARS, errMsg);
    if (index == -1)
    	return False;
    strcpy(&colorList[index*MAX_COLOR_LEN], arg);
/*printf("in readColor: index = %d, line = %s, arg = %s\n", index, line, arg);*/
    return True;
}

static int readFloatArg(char *line, float *result, char **errMsg)
{    	    
    char *arg;
    
    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, MAX_FLOAT_LEN, errMsg))
    	return False;
    
#ifdef VMS
    if (sscanf(arg, "%f", result) != 1) {
#else
    if (sscanf(arg, "%g", result) != 1) {
#endif /*VMS*/
    	*errMsg = "Argument must be a number";
    	return False;
    }
    return True;
}

static int readIntArg(char *line, int *value, char **errMsg)
{    	    
    char *arg;
    
    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, 20, errMsg))
    	return False;
    
    /* Convert text to integer argument */
    if (sscanf(arg, "%d", value) != 1) {
    	*errMsg = "Argument must be an integer";
    	return False;
    }
    return True;
}

/*
** Return a pointer to the beginning of the keyword argument on line "line".
** If there's no argument, or the maximum argument length is exceeded,
** return false, with a message in errMsg.  Note that trailing whitespace
** on arguments is removed by virtue of the getLine subroutine having
** trimmed it previously.
*/
static int getArg(char *line, char **argStart, int required,
		  int maxLength, char **errMsg)
{
    int lineIndex, length;

    lineIndex = strspn(line, " \t");
    lineIndex += strcspn(&line[lineIndex], " \t");
    lineIndex += strspn(&line[lineIndex], " \t");
    length = strlen(&line[lineIndex]);
    if (length == 0 && required) {
    	*errMsg = "Required argument not specified";
    	return False;
    }
    if (length >= maxLength) {
    	*errMsg = "Invalid argument";
    	return False;
    }
    *argStart = &line[lineIndex];
    return True;
}

static int readColorScaleName(char *line,
			      const ColorScale **cscale,
	        	      char **errMsg)
{
    char *arg;

    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, HS_MAX_TITLE_LENGTH, errMsg))
    	return False;
    if (strcmp(arg, DEFAULT_COLOR_SCALE_NAME))
    {
	if ((*cscale = findColorScale(arg)) == NULL)
	{
	    *errMsg = "Unknown color scale name";
	    return False;
	}
    }
    else
    {
	if ((*cscale = defaultColorScale()) == NULL)
	{
	    *errMsg = "Default color scale not found";
	    return False;
	}
    }
    return True;
}

/*
** Get the one or two digit index number of an indexed keyword (S1-S12 or
** V1-V14) from a line containing the keyword, if the maximum index
** "maxIndex" is exceeded, or the keyword contains no index, return -1
** with a message in errMsg.  Note that the index returned is zero based,
** ie. S1 -> 0, V2 -> 1.
*/
static int getIndex(char *line, int maxIndex, char **errMsg)
{
    int keyStart, keyEnd, result;
    char firstDigit, secondDigit;
    static char numbers[11] = "1234567890";

    keyStart = strspn(line, " \t");
    keyEnd = strcspn(&line[keyStart], " \t");
    secondDigit = line[keyEnd];
    firstDigit = line[keyEnd-1];
    if (strchr(numbers, secondDigit) == NULL) {
    	*errMsg = "Invalid keyword";
    	return -1;
    }
    if  (strchr(numbers, firstDigit) == NULL)
    	result = secondDigit - '0';
    else
    	result = (firstDigit - '0') * 10 + secondDigit - '0';
    
    if (result > maxIndex) {
    	*errMsg = "Keyword index to large";
    	return -1;
    }
    return result - 1;
}

/*
** Convert all of the plot variable and slider variable names into
** ntuple variable indecies returned in "varlist" and "sliderList".
** If a name is not found, return False and return a pointer to the
** unmatched name in errVar.
*/
static int convertVarNamesToIndecies(hsNTuple *ntuple, keywordValues *values,
	int *varList, int *sliderList, char **errVar)
{
    int i, index;
    ntupleExtension *ntExt = GetNTupleExtension(ntuple->id);
    
    for (i=0; i<MAX_DISP_VARS; i++) {
    	if (values->varsSpecified[i]) {
    	    index = lookupNtupleName(ntuple, ntExt, values->vars[i]);
    	    if (index == -1) {
    	    	*errVar = values->vars[i];
    	    	return False;
    	    }
    	    varList[i] = index;
    	} else {
    	    varList[i] = -1;
    	}
    }
    for (i=0; i<N_SLIDERS; i++) {
    	if (values->slidersSpecified[i]) {
    	    index = lookupNtupleName(ntuple, ntExt, values->sliders[i]);
    	    if (index == -1) {
    	    	*errVar = values->vars[i];
    	    	return False;
    	    }
    	    sliderList[i] = index;
    	} else {
    	    sliderList[i] = -1;
    	}
    }
    return True;
}

/*
** Find the index of the ntuple variable whose name matches "name".  Return
** -1 if not found.  Note that this depends heavily on routines elsewhere
** in histoscope having trimmed leading and trailing space from the ntuple
** names, since the configuration file syntax does not allow for leading
** or trailing space in names, and sloppy fortran users often leave these
** in their variable names and titles.
*/
static int lookupNtupleName(hsNTuple *ntuple, ntupleExtension *ntExt,
	char *name)
{
    int i;
    
    for (i=0; i<ntuple->nVariables; i++) {
    	if (!strcasecmp(name, ntuple->names[i]))
    	    return i;
    }
    if (ntExt != NULL) {
    	for (i=0; i<ntExt->nVars; i++) {
    	    if (!strcasecmp(name, ntExt->vars[i].name))
    	    	return i + ntuple->nVariables;
	}
    }
    return -1;
}

/*
** Version of XmToggleButtonSetState which checks for "w" being NULL
** first before trying to set the button state.  Used for safety so that
** if an improper keyword tries to set a button that doesn't exist, the
** program won't respond with a crash.
*/
static void setMenuToggle(Widget w, int buttonState, int notify)
{
    if (w != NULL)
    	XmToggleButtonSetState(w, buttonState, notify);
}

/*
** Look at a toggle button setting and decide whether the corresponding
** keyword should be written.  If the button exists in the menu, and is
** sensitive, return True if the button is not set to the default value
** passed in the parameter "default"
*/ 
static int toggleSetting(Widget w, int defaultValue)
{
    if (w == NULL)
    	return False;
    if (!XtIsSensitive(w))
    	return False;
    return XmToggleButtonGetState(w) != defaultValue;
}

/*
** Read a line and compare it against a keyword.  Keyword arguments in a
** Histo-Scope configuration file are indented and follow after a heading
** continuing the indent until the next heading.  Returns true if the
** keyword "keyword" is found properly indented at the beginning of the
** line.  Keywords ending in % match their equivalent with a 1 or 2 digit
** integer suffix.
*/
static int compareKeyword(char *line, const char *keyword)
{
    int white, nDigits;
    int keyLen = strlen(keyword);
    char afterKeyChar;
    
    white = strspn(line, " \t");
    if (white <= 0)
        return False;
    if (keyword[keyLen-1] == '%') {
    	if (strncasecmp(&line[white], keyword, keyLen-1))
    	    return False;
    	nDigits = strspn(&line[white+keyLen-1], "1234567890");
    	if (nDigits < 1 || nDigits > 2)
    	    return False;
    } else {
	if (strncasecmp(&line[white], keyword, keyLen))
    	    return False;
    	afterKeyChar = line[white+keyLen];
    	if (afterKeyChar != '\0' && afterKeyChar != ' ' &&
    	    	afterKeyChar != '\t')
    	    return False;
    }
    return True;
}

/*
** Save the position and size of a window (represented by the widget "shell")
** as an X standard geometry string.  A string of length MAX_GEOMETRY_LEN
** should be provided in the argument "geomString" to receive the result.
*/
static void createGeomString(Widget shell, char *geomString)
{
    int x, y;
    unsigned int width, height, dummyW, dummyH, bw, depth, nChild;
    Window parent, root, *child, window = XtWindow(shell);
    Display *dpy = XtDisplay(shell);
    
    /* Find the width and height from the window of the shell */
    XGetGeometry(dpy, window, &root, &x, &y, &width, &height, &bw, &depth);
    
    /* Find the top left corner (x and y) of the window decorations.  (This
       is what's required in the geometry string to restore the window to it's
       original position, since the window manager re-parents the window to
       add it's title bar and menus, and moves the requested window down and
       to the left.)  The position is found by traversing the window hier-
       archy back to the window to the last parent before the root window */
    for(;;) {
        XQueryTree(dpy, window, &root, &parent,  &child, &nChild);
        XFree((char*)child);
        if (parent == root)
            break;
        window = parent;
    }
    XGetGeometry(dpy, window, &root, &x, &y, &dummyW, &dummyH, &bw, &depth);
    
    /* Write the string */
    sprintf(geomString, "%dx%d+%d+%d", width, height, x, y);
}

/*
** Read a line of at most "maxLen" characters into the parameter
** "line" from file fp, ignoring comments (lines beginning with "#") and
** blank lines, and removing the terminating newline and any trailing space.
** Returns True if the line was read successfully, and False on end of file
** or error.  Increments lineNum.
*/
const char *getLine(char *line, const char *fp, int *lineNum, int maxLen)
{
    char *c, *inPtr, *outPtr;
    int lineLen;
    
    /* Read until line contains the next a non-blank, non-comment line */
    line[0] = '#'; line[1] = '\0';
    while (line[0] == '#' || lineIsBlank(line)) {
        if ((fp = sgets(line, maxLen, fp)) == NULL)
    	    return NULL;
    	(*lineNum)++;
    }
    
    /* Continue reading if line was continued with '\' */
    lineLen = strlen(line);
    if (line[lineLen-2] == '\\' && line[lineLen-1] == '\n')
        fp = getLine(&line[lineLen-2], fp, lineNum, maxLen - (lineLen-2));

    /* Convert escaped newline and backslash characters into real ones */
    for (inPtr=line, outPtr=line; *inPtr!='\0'; inPtr++) {
    	if (*inPtr == '\\' && *(inPtr+1) == 'n') {
    	    inPtr++;
    	    *outPtr++ = '\n';
	} else if (*inPtr == '\\' && *(inPtr+1) == '\\') {
	    inPtr++;
	    *outPtr++ = '\\';
	} else if (*inPtr == '\\') {
	    /* Hm, not sure what this thing is. Just ignore it. */
	    fprintf(stderr, "Warning: unknown escape "
		    "sequence '\%c' ignored\n", *++inPtr);
    	} else
    	    *outPtr++ = *inPtr;
    }
    *outPtr = '\0';

    /* Remove trailing newlines and whitespace */
    for (c = &line[strlen(line)-1]; ; c--) {
    	if (*c == '\n' || *c == '\t' || *c == ' ')
    	    *c = '\0';
    	else
    	    break;
    }
    
    return fp;
}

/*
** Return True if the string "line" represents a blank line
*/
static int lineIsBlank(char *line)
{
    char *c;
    
    for (c=line; *c!='\0'; c++) {
    	if (*c != ' '  &&  *c != '\t'  &&  *c != '\n')
    	    return False;
    }
    return True;
}

/*
** Look up the multi-plot window corresponding to id "id" in the list
** "multiPlotIDs"
*/
static multWindow *lookUpMultiplot(multiPlotIDRec *multiPlotIDs, int id)
{
    int i;
    
    for (i=0; i<MAX_MULTI_PLOT_WINDOWS && multiPlotIDs[i].id!=0; i++)
    	if (multiPlotIDs[i].id == id)
    	    return multiPlotIDs[i].window;
    return NULL;
}

/*
** Add a multi-plot window and its corresponding id "id" to the list
** "multiPlotIDs"
*/
static void addMultiPlotID(multiPlotIDRec *multiPlotIDs, int id,
	multWindow *window)
{
    int i;
    
    /* Find the end of the list */
    for (i=0; i<MAX_MULTI_PLOT_WINDOWS; i++)
    	if (multiPlotIDs[i].id == 0)
    	    break;
    if (i == MAX_MULTI_PLOT_WINDOWS)
    	fprintf(stderr, "Internal Error: out of multi-plot windows\n");
    
    /* add the id and corresponding window to the list */
    multiPlotIDs[i].id = id;
    multiPlotIDs[i].window = window;
    
    /* Mark the new end of the list with an id of zero */
    multiPlotIDs[i+1].id = 0;
}


static Pixel allocColor(char *colorName, Display *display, int screen_num)
{
    int default_depth;
/*    Visual *default_visual; */
    XColor exact_def;
    int visualClass = 5;
    XVisualInfo visual_info;
    static int warningPrinted = 0;
/*
 *     static char *visual_class[] = {
 * 	"StaticGray",
 * 	"GrayScale",
 * 	"StaticColor",
 * 	"PseudoColor",
 * 	"TrueColor",
 * 	"DirectColor"
 *     };
 */

    /* Try to allocate colors for PseudoColor, TrueColor, 
     * DirectColor, and StaticColor.  Use black and white
     * for StaticGray and GrayScale */

    default_depth = DefaultDepth(display, screen_num);
/*     default_visual = DefaultVisual(display, screen_num); */
    if (default_depth == 1) {
	/* must be StaticGray, use black and white */
	return(BlackPixel(display, screen_num));
    }

    while (!XMatchVisualInfo(display, screen_num, default_depth, 
	    visualClass--, &visual_info))
	    ;
    ++visualClass;
/*     printf("%s: found a %s class visual at default_depth.\n", progname, 
	    visual_class[visualClass]);
 */
    if (visualClass < 2) {
	/* No color visual available at default_depth.  Some applications
	 * might call XMatchVisualInfo here to try for a GrayScale
	 * visual if they can use gray to advantage, before  giving up
	 * and using black and white.
	 */
	return(BlackPixel(display, screen_num));
    }

    /* Otherwise, got a color visual at default_depth */

    /* The visual we found is not necessarily the default visual, and
     * therefore it is not necessarily the one we used to create our
     * window.  However, we now know for sure that color is supported,
     * so the following code will work (or fail in a controlled way).
     * Let's check just out of curiosity:
     */
/*     if (visual_info.visual != default_visual)
       printf("%s: PseudoColor visual at default depth is not default visual!\n\
	   Continuing anyway...\n", progname);
 */
    /* printf("allocating %s\n", ColorName[i]); */
    if (!XParseColor (display, DefaultColormap(display, screen_num), 
	    colorName, &exact_def)) {
	fprintf(stderr, "Color name %s not in database\n", colorName);
	return(BlackPixel(display, screen_num));
    }
/*  printf("The RGB values from the database are %d, %d, %d\n", 
	exact_def.red, exact_def.green, exact_def.blue);
*/
    if (!XAllocColor(display, DefaultColormap(display,screen_num),&exact_def)) {
	if (warningPrinted == 0) {
	    fprintf(stderr, 
		    "\nCan't allocate color %s. All colorcells allocated"
		    " and no matching cell found.\n", colorName);
	    warningPrinted = -1;
	}
	return(BlackPixel(display, screen_num));
    }
/*  printf("The RGB values actually allocated are %d, %d, %d\n", 
	exact_def.red, exact_def.green, exact_def.blue);
*/
    return(exact_def.pixel);
}

static windowInfo *findOverlayWindow(int overlayID)
{
    int i;
    
    for (i=0; i<NOverlays; i++)
    	if (overlayID == Overlays[i].id)
    	    return Overlays[i].window;
    return NULL;
}

static void saveOverlayWindow(windowInfo *window, int overlayID)
{
    int i;
    overlayIDRec *oldTable, *newTable;
    
    /* If the overlay id is already in the table, just overwrite it */
    for (i=0; i<NOverlays; i++) {
    	if (overlayID == Overlays[i].id) {
    	    Overlays[i].window = window;
    	    return;
    	}
    }
    
    /* Create a new entry */
    oldTable = Overlays;
    newTable = (overlayIDRec *)XtMalloc(sizeof(overlayIDRec) * (NOverlays+1));
    for (i=0; i<NOverlays; i++) {
    	newTable[i].id = oldTable[i].id;
    	newTable[i].window = oldTable[i].window;
    }
    newTable[i].id = overlayID;
    newTable[i].window = window;
    Overlays = newTable;
    NOverlays++;
    
    /* Free the old table */
    if (oldTable != NULL)
    	XtFree((char *)oldTable);
}

static void clearOverlayList(void)
{
    if (Overlays != NULL)
    {
    	XtFree((char *)Overlays);
	Overlays = NULL;
    }
    NOverlays = 0;
}

static char *fillStyleName(unsigned int style)
{
    int i;
    
    for (i=0; i<N_FILL_STYLES; i++)
    	if (HistFillStyles[i].style == style)
    	    return HistFillStyles[i].name;
    return "none";
}

/*
** Setting the scaling for a plot whose data we don't yet have, is not always
** possible.  The widgets need at least some data as a frame of reference
** to make the limits meaningful, and until they have that, they ignore
** scale information until they have some data to apply it to.  This routine
** collects the data and routine necessary for scaling and hands it off to
** the communication code to be executed when data from an attached process
** finally arrives.  The memory allocated here is freed when the callback
** has completed its scaling task.
*/
static void setPlotScaleDeferred(int id, Widget plotW, char *keyUsed,
    	plotScaleInfo *scale)
{
    deferredScaleInfo *scaleData;
    
    scaleData = (deferredScaleInfo *)XtMalloc(sizeof(deferredScaleInfo));
    scaleData->scale = *scale;
    scaleData->plotW = plotW;
    memcpy(scaleData->keysUsed, keyUsed, NKEYWORDS);
    ScheduleDataArrivedCallback(id, deferredPlotScalingCB, (void *)scaleData);
}

/*
** Callback routine for deferred scaling (see setPlotScaleDeferred above)
** This callback is invoked when data is actually available for the plot.
*/
static void deferredPlotScalingCB(void *cbData)
{
    deferredScaleInfo *scaleData = (deferredScaleInfo *)cbData;
    windowInfo *w;
    
    /* For the unlikely but real possibility the plot has been deleted,
       check first that the corresponding window, and thus the widget, 
       still exists. */
    w = GetWinfoFromWidget(scaleData->plotW);
    if (w == NULL) {
    	XtFree((char *)scaleData);
    	return;
    }
    
    /* Instead of allowing the refresh work proc in plotWindows.c to refresh
       the plot, do it here and mark the plot as no longer needing data, so it
       won't reset the widget.  Note that for overlay plots, we may be turning
       off w->needsData prematurely, but this should be safe, since each
       overlay will be processed here and thus will be properly REINITed */
    RedisplayPlotWindow(w, REINIT);
    w->needsData = False;
    
    /* Set the scale. */
    	setPlotScale(scaleData->plotW, scaleData->keysUsed, &scaleData->scale);
    XtFree((char *)scaleData);
}

const char * sgets(char *buffer, size_t size, const char *stream)
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

static void writeUsedColorScales(FILE *fp)
{
    int i, j, row, col, count = 0, nColorScales, newscale;
    const ColorScale **scales_written;
    windowInfo *w;
    multWindow *window;
    plotInfo *pInfo;
    multiPlot *plot;

    nColorScales = numUsedColorScales();
    if (nColorScales == 0)
	return;
    scales_written = (const ColorScale **)malloc(nColorScales*sizeof(ColorScale *));
    if (scales_written == NULL)
    {
	fprintf(stderr, "Error in writeUsedColorScales: out of memory!\n");
	return;
    }
    for (w=WindowList; w!=NULL; w=w->next)
    {
	if (!w->multPlot) 
	{
	    for (i=0; i<w->nPlots; i++)
	    {
		pInfo = w->pInfo[i];
		if (pInfo->csi.colorScale)
		{
		    /* Is this a color scale we haven't written yet? */
		    newscale = 1;
		    for (j=0; j<count; ++j)
			if (pInfo->csi.colorScale == scales_written[j])
			{
			    newscale = 0;
			    break;
			}
		    if (newscale)
		    {
			assert(count < nColorScales);
			writeColorScale(pInfo->csi.colorScale, fp);
			scales_written[count++] = pInfo->csi.colorScale;
		    }
		}
	    }
	}
    }
    for (window=MultWindowList; window != NULL; window=window->next) {
	for (row = 0; row < window->numRows; ++row) {
	    for (col = 0; col < window->numCols; ++col) {
		plot = &window->plot[row*window->numCols+col];
		if (plot->wInfo != NULL) {
		    w = plot->wInfo;
		    for (i=0; i<w->nPlots; i++)
		    {
			pInfo = w->pInfo[i];
			if (pInfo->csi.colorScale)
			{
			    /* Is this a color scale we haven't written yet? */
			    newscale = 1;
			    for (j=0; j<count; ++j)
				if (pInfo->csi.colorScale == scales_written[j])
				{
				    newscale = 0;
				    break;
				}
			    if (newscale)
			    {
				assert(count < nColorScales);
				writeColorScale(pInfo->csi.colorScale, fp);
				scales_written[count++] = pInfo->csi.colorScale;
			    }
			}
		    }
		}
	    }
	}
    }
    free(scales_written);
}

static void writeColorScale(const ColorScale *scale, FILE *fp)
{
    int i;

    fprintf(fp, "%s\n", Headings[COLORSCALE_HEADING]);
    fprintf(fp, " %s %s\n", NAME_KEY, scale->name);
    fprintf(fp, " %s %s\n", COLORSPACE_KEY, colorSpaceName(scale->colorspace));
    fprintf(fp, " %s %d\n", NCOLORS_KEY, scale->ncolors);

    fprintf(fp, " %s ", STARTCOLOR_KEY);
    writeOneScaleColor(scale, scale->startPixel, fp);
    fprintf(fp, "\n");
    fprintf(fp, " %s ", ENDCOLOR_KEY);
    writeOneScaleColor(scale, scale->endPixel, fp);
    fprintf(fp, "\n");
    fprintf(fp, " %s ", UNDERCOLOR_KEY);
    writeOneScaleColor(scale, scale->underflowPixel, fp);
    fprintf(fp, "\n");
    fprintf(fp, " %s ", OVERCOLOR_KEY);
    writeOneScaleColor(scale, scale->overflowPixel, fp);
    fprintf(fp, "\n");

    if (scale->isLinear)
	fprintf(fp, " %s\n", LINEARCOLOR_KEY);
    else
    {
	for (i=0; i<scale->ncolors; ++i)
	{
	    fprintf(fp, " %s %d ", PIXELCOLOR_KEY, i);
	    writeOneScaleColor(scale, scale->pixels[i], fp);
	    fprintf(fp, "\n");
	}
    }
}

static void writeOneScaleColor(const ColorScale *scale, Pixel pix, FILE *fp)
{
    char buf[MAX_COLOR_LEN];
    XcmsColor cmsColor;

    cmsColor.pixel = pix;
    XcmsQueryColor(scale->display, scale->colormap, &cmsColor, XcmsRGBiFormat);
    printColorName(scale->display, scale->colormap, buf, scale->colorspace,
		   cmsColor.spec.RGBi.red, cmsColor.spec.RGBi.green,
		   cmsColor.spec.RGBi.blue);
    fprintf(fp, "%s", buf);
}

static void writeNonscaleColor(Display *dpy, Colormap cmap, Pixel pix, FILE *fp)
{
    char buf[MAX_COLOR_LEN];
    XcmsColor cmsColor;

    cmsColor.pixel = pix;
    XcmsQueryColor(dpy, cmap, &cmsColor, XcmsRGBiFormat);
    printColorName(dpy, cmap, buf, RGBI,
		   cmsColor.spec.RGBi.red, cmsColor.spec.RGBi.green,
		   cmsColor.spec.RGBi.blue);
    fprintf(fp, "%s", buf);
}

static int prepareColorScale(Widget parent, keywordValues *values,
			     int headingLineNum)
{
    int i, choice, cspace;
    const ColorScale *scale;
    char buf[64];
    const char *color;
    Pixel *newpixels;
    int n_new_pixels = 0;
    XcmsColor exact_def, screen_def;
    Display *dpy = XtDisplay(parent);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(parent));
    char *keyUsed = values->specified;

    /* Check that we have all necessary info */
    if (values->name[0] == '\0') {
        choice = DialogF(DF_ERR, parent, 2,
           "Title is required for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (values->colorSpace[0] == '\0') {
        choice = DialogF(DF_ERR, parent, 2,
           "Color space is not specified for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    cspace = colorSpaceNumber(values->colorSpace);
    if (cspace < 0)
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Bad color space \"%s\" for\ncolor scale (line %d)",
           "Cancel", "Continue", values->colorSpace, headingLineNum);
        return choice != 1;	
    }
    if (values->ncolors == -12345)
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Number of colors is not specified for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (values->ncolors < 2 || values->ncolors > maxColorScaleColors())
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Number of colors is out of range for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (values->startColor[0] == '\0')
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Start color is not specified for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (values->endColor[0] == '\0')
    {
        choice = DialogF(DF_ERR, parent, 2,
           "End color is not specified for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (values->underflowColor[0] == '\0')
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Underflow color is not specified for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (values->overflowColor[0] == '\0')
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Overflow color is not specified for\ncolor scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (!keyUsed[LINEARCOLOR_INDEX])
	for (i=0; i<values->ncolors; ++i)
	    if (values->pixelColors[i][0] == '\0')
	    {
		choice = DialogF(DF_ERR, parent, 2,
			"Pixel %d color is not specified for\ncolor scale (line %d)",
			"Cancel", "Continue", i, headingLineNum);
		return choice != 1;
	    }
    scale = createLinearColorScale(
	values->name, dpy, cmap,
	cspace, values->ncolors,
	values->startColor, values->endColor,
	values->underflowColor, values->overflowColor);
    if (scale == NULL)
    {
        choice = DialogF(DF_ERR, parent, 2,
           "Failed to allocate\na color scale (line %d)",
           "Cancel", "Continue", headingLineNum);
        return choice != 1;	
    }
    if (keyUsed[COLORSCALEPERSIST_INDEX])
	incrColorScaleRefCount(scale);
    if (!keyUsed[LINEARCOLOR_INDEX])
    {
	newpixels = (Pixel *)malloc(values->ncolors*sizeof(Pixel));
	if (newpixels == NULL)
	{
	    fprintf(stderr, "Error in prepareColorScale: out of memory!\n");
	    decrColorScaleRefCount(scale);
	    return False;
	}
	for (i=0; i<values->ncolors; ++i)
	{
	    color = normalizeColorName(&values->pixelColors[i][0], buf);
	    if (XcmsLookupColor(dpy, cmap, color,
		&exact_def, &screen_def, XcmsRGBiFormat) == XcmsFailure)
	    {
		choice = DialogF(DF_ERR, parent, 2,
			"Bad color name \"%s\" for\ncolor scale (line %d)",
			"Cancel", "Continue", &values->pixelColors[i][0], headingLineNum);
		if (n_new_pixels > 0)
		    FreeSharedColors(dpy, cmap, newpixels, n_new_pixels, 0);
		free(newpixels);
		decrColorScaleRefCount(scale);
		return choice != 1;
	    }
	    if (AllocSharedColor(dpy, cmap, &screen_def, XcmsRGBiFormat) == XcmsFailure)
	    {
		choice = DialogF(DF_ERR, parent, 2,
			"Failed to allocate color \"%s\" for\ncolor scale (line %d)",
			"Cancel", "Continue", &values->pixelColors[i][0], headingLineNum);
		if (n_new_pixels > 0)
		    FreeSharedColors(dpy, cmap, newpixels, n_new_pixels, 0);
		free(newpixels);
		decrColorScaleRefCount(scale);
		return choice != 1;
	    }
	    newpixels[n_new_pixels++] = screen_def.pixel;
	}
	for (i=0; i<values->ncolors; ++i)
	    setScalePixel(scale, i, newpixels[i]);
	free(newpixels);
    }
    /* Success */
    RedisplayAllColoredWindows(scale);
    return True;
}

static int readPixelData(char *line, char pixelColors[][MAX_COLOR_LEN],
			 char **errMsg)
{
    char *arg;
    int pixel_number;
    char color[MAX_COLOR_LEN];

    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, MAX_COLOR_LEN+5, errMsg))
    	return False;

    /* Return the argument */
    if (sscanf(arg, "%d %s", &pixel_number, color) != 2)
	return False;
    if (pixel_number < 0 || pixel_number >= MAX_COLORS_HIGHCOLOR)
    {
	*errMsg = "Color number is out of range";
	return False;
    }
    strcpy(&pixelColors[pixel_number][0], color);
    return True;
}

static int CloseAll_cmd(char *cmd, int taskNumber, int argc,
			char argv[][HS_MAX_NAME_LENGTH], char **errMsg)
{
    /* See the comment inside the "closeAllCB" code in file mainMenu.c
       about the order in which the windows should be closed */
    CloseAllMultWindows();
    CloseAllPlotWindows();
    CloseAllNTupleWindows();
    UpdateWindowsMenu();
    ReportTaskCompletion(taskNumber, 0, NULL);
    return True;
}

static int executeHistoCommand(char *cmd, int taskNumber, int argc,
			       char argv[][HS_MAX_NAME_LENGTH], char **errMsg)
{
    int i;
    printf("Executing command \"%s\", task %d, ", cmd, taskNumber);
    if (argc > 0)
	printf("with arguments");
    else
	printf("without additional arguments");
    for (i=0; i<argc; ++i)
	printf(" %s", &argv[i][0]);
    printf("\n");
    fflush(stdout);
    for (i=0; i<(int)(sizeof(CommandList)/sizeof(CommandList[0])); ++i)
	if (strcmp(cmd, CommandList[i].name) == 0)
	    return CommandList[i].command(cmd, taskNumber, argc, argv, errMsg);
    ReportTaskCompletion(taskNumber, 1, "invalid command");
    *errMsg = "";
    return False;
}

static XmFontListEntry getFontEntry(Display *dpy, char *fontName)
{
    XmFontContext fontContext;
    XmFontListEntry fontEntry = NULL;
    static XmFontList fontList = NULL;

    if (XmFontListInitFontContext(&fontContext, fontList))
    {
	while ((fontEntry = XmFontListNextEntry(fontContext)))
	    if (strcmp(XmFontListEntryGetTag(fontEntry), fontName) == 0)
		break;
	XmFontListFreeFontContext(fontContext);
    }
    if (fontEntry == NULL)
    {
	fontEntry = XmFontListEntryLoad(
	    dpy, fontName, XmFONT_IS_FONT, fontName);
	fontList = XmFontListAppendEntry(fontList, fontEntry);
    }
    return fontEntry;
}

static int readAlignmentArg(char *line, int *value, char **errMsg)
{
    int i;
    char *arg;

    /* Get the argument and check its length */
    if (!getArg(line, &arg, True, 3, errMsg))
    	return False;

    /* Convert to an enum */
    arg[0] = toupper(arg[0]);
    arg[1] = toupper(arg[1]);
    for (i=0; i<N_ALIGNMENT_TYPES; ++i)
	if (strncmp(arg, alignmentNames[i], 2) == 0)
	{
	    *value = i;
	    return True;	    
	}
    *errMsg = "Bad alignment argument";
    return False;
}

static int readCoordsType(char *line, unsigned char *value, char **errMsg, int get)
{
    char *arg;
    int i;

    /* Get the argument and check its length */
    if (get)
    {
	if (!getArg(line, &arg, True, 8, errMsg))
	    return False;
    }
    else
	arg = line;
    
    for (i=0; i<N_COORD_TYPES; ++i)
	if (strcasecmp(arg, coordTypeNames[i]) == 0)
	{
	    *value = i;
	    return True;
	}
    *errMsg = "Invalid coordinate system type";
    return False;
}

static int readDrawingPoint(char *line, DrawingPoint *pt, char **errMsg)
{
    /* This function must be consistent
       with the "writeDrawingPoint" function */
    int i, j;
    char *cvalue, *endptr, *description;
    int status = True;

    if (!getArg(line, &description, True, HS_MAX_NAME_LENGTH, errMsg))
    	return False;

    for (j=0, cvalue = strtok(description, " \t");
	 cvalue; cvalue = strtok(NULL, " \t"), ++j)
    {
	switch (j)
	{
	case 0:
	    pt->x = strtod(cvalue, &endptr);
	    status = (cvalue != endptr);
	    break;
	case 1:
	    status = readCoordsType(
		cvalue, &pt->coordTypeX, &endptr, 0);
	    break;
	case 2:
	    for (i=0; i<N_POINT_REF_TYPES; ++i)
		if (strcasecmp(cvalue, pointRefTypeNames[i]) == 0)
		{
		    pt->refTypeX = i;
		    break;
		}
	    status = (i < N_POINT_REF_TYPES);
	    break;
	case 3:
	    pt->y = strtod(cvalue, &endptr);
	    status = (cvalue != endptr);
	    break;
	case 4:
	    status = readCoordsType(
		cvalue, &pt->coordTypeY, &endptr, 0);
	    break;
	case 5:
	    for (i=0; i<N_POINT_REF_TYPES; ++i)
		if (strcasecmp(cvalue, pointRefTypeNames[i]) == 0)
		{
		    pt->refTypeY = i;
		    break;
		}
	    status = (i < N_POINT_REF_TYPES);
	    break;
	default:
	    status = False;
	    break;
	}
	if (status == False)
	    break;
    }
    if (j < 6 || status == False)
    {
	*errMsg = "Invalid point specification";
	return False;
    }
    return True;
}

static int readNumberedPoint(char *line, keywordValues *values,
			     char **errMsg)
{
    int index = getIndex(line, MAX_DRAWING_POINTS, errMsg);
    if (index == -1)
    	return False;
    if (!readDrawingPoint(line, values->points + index, errMsg))
	return False;
    values->pointsSpecified[index] = 1;
    return True;
}

static void writeDrawingObject(Widget w, const DrawingObject *obj,
			       const char *winID, int row, int col, FILE *fp)
{
    int i;
    Display *display = XtDisplay(w);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(w));
    Pixel black = BlackPixelOfScreen(XtScreen(w));

    fprintf(fp, "%s\n", Headings[WINACTION_HEADING]);
    fprintf(fp, " %s %s\n", WINDOWLOC_KEY, winID);
    if (row >= 0 && col >= 0)
    {
	fprintf(fp, " %s %d\n", ROW_KEY, row);
	fprintf(fp, " %s %d\n", COLUMN_KEY, col);
    }
    fprintf(fp, " %s Draw\n", WINDOWACT_KEY);
    fprintf(fp, " %s %s\n", CATEGORY_KEY, drawingObjectCategory(obj->category));
    if (obj->options & XYSTRING_FILLMASK)
    {
	fprintf(fp, " %s ", BGCOLOR_KEY);
	if (obj->fillcolor == black)
	    fprintf(fp, "black");
	else
	    writeNonscaleColor(display, cmap, obj->fillcolor, fp);
	fprintf(fp, "\n");
    }
    if (obj->options & XYSTRING_BORDERMASK)
    {
	fprintf(fp, " %s ", FGCOLOR_KEY);
	if (obj->linecolor == black)
	    fprintf(fp, "black");
	else
	    writeNonscaleColor(display, cmap, obj->linecolor, fp);
	fprintf(fp, "\n");
    }
    if ((obj->options & FORWARD_ARROW_MASK) && (obj->options & BACKWARD_ARROW_MASK))
	i = 3;
    else if (obj->options & FORWARD_ARROW_MASK)
	i = 1;
    else if (obj->options & BACKWARD_ARROW_MASK)
	i = 2;
    else
	i = 0;	
    if (i)
	fprintf(fp, " %s %d\n", ARROWSTYLE_KEY, i);

    /* Line style */
    if (obj->linestyle >= 0 && obj->linestyle < XY_N_LINE_STYLES)
	fprintf(fp, " LineStyle1 %d\n", obj->linestyle);

    /* Write the reference point */
    fprintf(fp, " %s ", REFPOINT_KEY);
    writeDrawingPoint(&obj->reference, fp);
    fprintf(fp, "\n");

    /* Write all object points */
    for (i=0; i<obj->npoints; ++i)
    {
	fprintf(fp, " Point%d ", i+1);
	writeDrawingPoint(obj->points + i, fp);
	fprintf(fp, "\n");
    }
}

static void writeDrawingPoint(const DrawingPoint *point, FILE *fp)
{
    /* This function must be consistent
       with the "readDrawingPoint" function */
    fprintf(fp, "%g %s %s %g %s %s",
	    (double)point->x,
	    coordTypeNames[point->coordTypeX],
	    pointRefTypeNames[point->refTypeX],
	    (double)point->y,
	    coordTypeNames[point->coordTypeY],
	    pointRefTypeNames[point->refTypeY]);
}

static void writeDrawingsAndComments(FILE *fp, windowInfo *window,
				     const char *winID, int row, int col)
{
    OverlayedObject *obj = window->decor;

    if (winID[0] == '\0')
	return;
    while (obj)
    {
	switch (obj->type)
	{
	case OVER_STRING:
	    writeWindowComment(window->widget, &obj->item.s, winID, row, col, fp);
	    break;
	case OVER_DRAW:
	    writeDrawingObject(window->widget, &obj->item.d, winID, row, col, fp);
	    break;
	default:
	    assert(0);
	}
	obj = obj->next;
    }
}

static void writeWindowComment(Widget w, const XYString *string,
			       const char *winID, int row, int col, FILE *fp)
{
    Display *display = XtDisplay(w);
    Colormap cmap = DefaultColormapOfScreen(XtScreen(w));
    Pixel black = BlackPixelOfScreen(XtScreen(w));
    char *txt;

    fprintf(fp, "%s\n", Headings[WINACTION_HEADING]);
    fprintf(fp, " %s %s\n", WINDOWLOC_KEY, winID);
    if (row >= 0 && col >= 0)
    {
	fprintf(fp, " %s %d\n", ROW_KEY, row);
	fprintf(fp, " %s %d\n", COLUMN_KEY, col);
    }
    if (string->text)
	fprintf(fp, " %s Comment %s\n", WINDOWACT_KEY, string->text);
    else
    {
	XmStringGetLtoR(string->string, XmSTRING_DEFAULT_CHARSET, &txt);
	fprintf(fp, " %s Comment %s\n", WINDOWACT_KEY, txt);
	XtFree(txt);
    }
    if (string->font)
    {
	char *fontName = getFontName(XtDisplay(w), string->font);
	if (fontName)
	{
	    fprintf(fp, " %s %s\n", FONT_KEY, fontName);
	    XFree(fontName);
	}
    }
    if (string->psfont)
    {
	fprintf(fp, " %s %s\n", PSFONT_KEY, string->psfont);
	if (string->psfontsize)
	    fprintf(fp, " %s %d\n", PSFONTSIZE_KEY, string->psfontsize);
    }
    if (string->color != black)
    {
	fprintf(fp, " %s ", FGCOLOR_KEY);
	writeNonscaleColor(display, cmap, string->color, fp);
	fprintf(fp, "\n");
    }
    if (string->options & XYSTRING_FILLMASK)
    {
	fprintf(fp, " %s ", BGCOLOR_KEY);
	if (string->background == black)
	    fprintf(fp, "black");
	else
	    writeNonscaleColor(display, cmap, string->background, fp);
	fprintf(fp, "\n");
    }
    if (string->options & XYSTRING_BORDERMASK)
	fprintf(fp, " %s\n", BORDER_KEY);
    fprintf(fp, " %s %s\n", ALIGNMENT_KEY, alignmentNames[string->alignment]);
    fprintf(fp, " %s %d\n", JUSTIFYTEXT_KEY, 
	    (string->options & JUSTIFY_TEXT_MASK) >> JUSTIFY_TEXT_SHIFT);

    /* Reference point */
    fprintf(fp, " %s ", REFPOINT_KEY);
    writeDrawingPoint(&string->reference, fp);
    fprintf(fp, "\n");

    /* String position */
    fprintf(fp, " Point1 ");
    writeDrawingPoint(&string->position, fp);
    fprintf(fp, "\n");
}

static void safeRedisplay(windowInfo *window)
{
    /* Make sure we can call the redisplay command
       without referring to a non-existing item */
    int p;
    for (p=0; p<window->nPlots; p++)
   	if (GetMPItemByID(window->pInfo[p]->id) == NULL)
	    return;
    RedisplayPlotWindow(window, REFRESH);
}

static char *getFontName(Display *dpy, XmFontList font)
{
    XmFontContext fontContext;
    XmFontListEntry fontEntry;
    XFontStruct *fs = NULL;
    XmFontType type_return;
    unsigned long property;

    assert(dpy);
    if (font == NULL)
	return NULL;
    if (XmFontListInitFontContext(&fontContext, font))
    {
	while ((fontEntry = XmFontListNextEntry(fontContext)))
	{
	    fs = (XFontStruct *)XmFontListEntryGetFont(fontEntry, &type_return);
	    if (type_return == XmFONT_IS_FONT)
		break;
	    else
		fs = NULL;
	}
	XmFontListFreeFontContext(fontContext);
    }
    if (fs)
	if (XGetFontProperty(fs, XA_FONT, &property))
	    return (char *)XGetAtomName(dpy, (Atom)property);
    return NULL;
}
