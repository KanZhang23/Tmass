/*******************************************************************************
*									       *
* plotWindows.h -- Windows and menus for histogram/ntuple/indicator widgets    *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
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
* May 20, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Modified by Joy Kyriakopulos 5/12/93 to add slider > or < 		       *
*									       *
*******************************************************************************/

#include "ColorScale.h"
#include "drawOver.h"

#define N_SLIDERS 17
#define SLIDER_LT -1
#define SLIDER_GT 0
#define MAX_OVERLAID 16
#define MAX_WINDOWID_LEN 128

enum redisplayTypes {REINIT = 0, UPDATE, REBIN, ANIMATION,
                     RENORMALIZE, OVERLAY, REFRESH};
/** Note that values for errorBars are now exported to the client code.  Any */
/** changes here should also go into HistoClient.c and the user includes     */
enum errorBars {NO_ERROR_BARS = 0, DATA_ERROR_BARS, GAUSSIAN_ERROR_BARS};
enum binStrats {SPLIT_IN_HALF = 0, CENTER_OF_GRAVITY};
enum ntuplePlotTypes {TSPLOT = 0, TSPLOTERR, XYPLOT, XYPLOTERR, XYSORT,
        XYSORTERR, SCAT2D, SCAT3D, HIST1D, HIST2D, ADAPTHIST1D, ADAPTHIST2D,
        CELL, COLORSCAT2D, COLORCELL, N_PLOT_TYPES};
enum coordTypes {WINDOW_ABS_COORDS = 0, WINDOW_REL_COORDS,
                 PLOT_COORDS, N_COORD_TYPES};
enum referenceTypes {COORD_ORIGIN_REF = 0, LAST_POINT_REF, REF_POINT_REF,
                     N_POINT_REF_TYPES};
enum alignmentTypes {ALIGN_NW = 0, ALIGN_NC,   ALIGN_NE,
		     ALIGN_CW,     ALIGN_CC,   ALIGN_CE,
		     ALIGN_SW,     ALIGN_SC,   ALIGN_SE,
                     N_ALIGNMENT_TYPES};

typedef void (*redisplayProc)();
typedef void (readyCallBackProc)(void *wInfo, int taskNumber, char *c);

typedef struct _readyCallBack {
    struct _readyCallBack *next;
    readyCallBackProc *callback;
    int taskNumber;
    char data[512];
} readyCallBack;

typedef struct _colorScaleInfo {
    const ColorScale *colorScale;
    float colorMin;
    float colorMax;
    int colorIsLog;
    int rangeIsDynamic;
} colorScaleInfo;

typedef struct _widgetConfigInfo {
    Arg args[20];
    int nargs;
} widgetConfigInfo;

typedef struct _plotInfo {
    int id;				/* id for source of data */
    int plotType;			/* type of plot (for ambiguous item
    	    	    	    	    	   types like ntuple and 2DHist) */
    int ntVars[MAX_DISP_VARS];		/* if source is ntuple, this holds a
    					   list of the variables */
    int nXBins;				/* for histograms from ntuple data,
    					   number of bins set by user, for
    					   adaptive hists, bin limit */
    int nYBins;				/* "" # of bins along y (2D hist) */
    int nSliders;			/* number of active slider variables */
    int sliderVars[N_SLIDERS];		/* ntuple variable index for each sl. */
    float sliderThresholds[N_SLIDERS];	/* thresholds for slider variables 
    					   above which an ntuple element is
    					   not displayed.  Disabled by
    					   setting to FLT_MAX */
    int sliderGTorLT[N_SLIDERS];	/* flags the way slider affects plot */
    char errorBars;			/* type of error bars (or none) shown */
    char aHistBinStrategy;		/* binning strategy (adaptive hists) */
    char padding[2];
    colorScaleInfo csi;
} plotInfo;

typedef struct _windowInfo {
    char windowID[MAX_WINDOWID_LEN];    /* window id string */
    char xlabel[HS_MAX_LABEL_LENGTH];   /* x axis label */
    char ylabel[HS_MAX_LABEL_LENGTH];   /* y axis label */

    struct _windowInfo *next;		/* pointer to next in list */
    plotInfo **pInfo;	    	    	/* per-plot information (overlaying
    	    	    	    	    	   allows multiple plots per window) */
    int nPlots;     	    	    	/* number of plots overlaid here */
    char update;			/* set to false to turn off updating */
    char needsUpdate;			/* item has new data, needs refresh */
    char needsData;			/* window was created from empty item */
    char titleStarred;			/* title has "*" ind undisplayed data */
    char growOnly;			/* never shrink plot scale on updates */
    void *curveStyles;			/* (TS and XY) mark & line style info */
    void *histStyles;			/* histogram line and fill style info */
    int nCurves;			/* (2D plots) number of curves */
    int nHists;	    			/* (2D plots) number of histograms */
    int triggerCount;			/* (triggers only) # of pending trigger
    					   button presses not yet acknowleged */
    int multPlot;			/* !=0 if plot in a multi-plot window */
    float sliderMin[N_SLIDERS];		/* min values for each slider */
    float sliderMax[N_SLIDERS];		/* max values for each slider */
    float cellNormMax;			/* (cell only) full rectangle thres. */
    float cellNormMin;			/* (cell only) empty rectangle thresh */
    redisplayProc redisplay;		/* procedure for redisplaying */
    Widget rebinMenuItem;		/* rebin slider toggle button in menu */
    Widget cellNormMenuItem;		/* cell normalization tog btn in menu */
    Widget sliderMenuItem;		/* show sliders toggle button in menu */
    Widget statsMenuItem;		/* show stats toggle button in menu */
    Widget coordsMenuItem;		/* show coords toggle button in menu */
    Widget autoUpMenuItem;		/* auto update toggle button in menu */
    Widget updateMenuItem;		/* update item in menu */ 
    Widget growOnlyMenuItem;		/* grow only toggle button in menu */
    Widget gaussErrorMenuItem;		/* gaussian err toggle in hist menus */
    Widget backplanesMenuItem;
    Widget binEdgeMenuItem;
    Widget thickenMenuItem;
    Widget legendMenuItem;
    Widget errorDataMenuItem;		/* error data toggle in hist menus */
    Widget splitHalfMenuItem;		/* adaptive hist strategy toggle */
    Widget cntrOfGravMenuItem;		/* adaptive hist strategy toggle */
    Widget showTitlesMenuItem;		/* mini-plot show titles toggle */
    Widget sliderWindow;		/* slider dialog top non-shell widget */
    Widget sliderLabels[N_SLIDERS];	/* value label for each slider */
    Widget sliderScales[N_SLIDERS];	/* scale widget for each slider */
    Widget rebinWindow;			/* rebin dialog top non-shell widget */
    Widget cellNormWindow;		/* cell norm dialog widget */
    Widget cellLogA;			/* The cell plot log mode Btn */
    Widget statsWindow;			/* stats dialog top non-shell widget */
    Widget coordsWindow;                /* coordinate display widget */
    Widget coAbs, coRel, coPlot;        /* coordinate display text fields */
    Widget rebinLabels[2];		/* value labels for rebin sliders */
    Widget rebinScales[2];		/* scale widgets for rebin sliders */
    Widget cellNormLowLabel;		/* value label for cellNorm slider */
    Widget cellNormUpLabel;		/* value label for cellNorm slider */
    Widget cellNormScale;		/* scale widget for cellNorm slider */
    Widget shell;			/* application shell for window */
    Widget widget;			/* plotting widget */
    Widget menu;			/* popup menu */
    Widget axisSettings;		/* axis settings dialog shell */
    Widget scale;			/* (ind./control) scroll bar widget */
    Widget slider;			/* (control only) set value slider */
    Widget setValue;			/* (control only) text value widget */
    Widget setBtn;			/* (control only) set value button */
    Widget minLabel;			/* (ind./control) minLabel widget */
    Widget maxLabel;			/* (ind./control) maxLabel widget */
    Widget setRowColDlg;    	    	/* set num rows/cols dialog shell */
    Widget curveStyleDlg;   	    	/* shell for mark line style dialog */
    Widget histStyleDlg;   	    	/* shell for histogram style dialog */
    void *colorDialog;                  /* color adjustment dialog shell */
    readyCallBack *readyCB;
    OverlayedObject *decor;
} windowInfo;

/* The list of all displayed plot windows */
extern windowInfo *WindowList;

void RefreshItem(int id);
void ReinitItem(int id);
void RedisplayPlotWindow(windowInfo *window, int redisplayMode);
void OverlayPlot(windowInfo *window, hsGeneral *item, int plotType, int *ntVars,
    	int *sliderVars, int nXBins, int nYBins, int errorBars,
    	void *curveStyles, int nCurves, void *histStyle, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
void BufferGraphics(int state);
int GetGraphicsBuffering(void);
void ChangeWindowItemID(int oldID, int newID);
int CountPlotWindows(void);
void GetPlotWindowsAndTitles(Window *windows, char **titles);
void InvalidateItem(int id);
void InvalidateAllWindows(void);
windowInfo *Create1DHistWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
	widgetConfigInfo *confInfo);
windowInfo *Create2DHistWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *Create1DAdaptHistWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, char *winID, Widget shell,
        int isMiniPlot, colorScaleInfo *csInfo, widgetConfigInfo *confInfo);
windowInfo *Create2DAdaptHistWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateTSPlotWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateXYPlotWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateXYSortWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateTSPlotErrWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateXYPlotErrWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateXYSortErrWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *Create2DScatWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateColor2DScatWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *Create3DScatWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateCellWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateColorCellWindow(Widget parent, hsGeneral *item,
        int *ntVars, int *sliderVars, char *title, char *winID,
        Widget shell, int isMiniPlot, colorScaleInfo *csInfo,
        widgetConfigInfo *confInfo);
windowInfo *CreateGenericWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, WidgetClass plotWidgetClass,
	redisplayProc redisplay, char *title, char *winID, Widget shell,
	int isMiniPlot, int plotType, widgetConfigInfo *confInfo);
void ClosePlotWindow(windowInfo *wInfo);
void CloseAllPlotWindows(void);
void RemoveFromWindowList(windowInfo *window);
void StarWindowTitle(windowInfo *wInfo, int state);
int ItemIsDisplayed(int id);
int ShowErrorBars(windowInfo *wInfo, int errorBarState, int calledFromMenu);
windowInfo *FirstDisplayedItem(int id);
windowInfo *GetWinfoFromWidget(Widget w);
void CloseAuxWinsForPlot(windowInfo *wInfo);
readyCallBackProc ClosePlotWindowById;
readyCallBackProc GeneratePlotPSById;
void RunRefreshCycle(windowInfo *w);
void RemoveOverlayed(windowInfo *window, int index);
void setThickenPointsScat(int state);
void setThickenPointsScat3D(int state);
int getThickenPointsScat();
int getThickenPointsScat3D();
void AllowCoordPrint(int state);
int coordPrintAllowed(void);
void ClearOverlayedObjects(windowInfo *wInfo);
int existsInWindowList(windowInfo *window);

void SetTitlePSFont(const char *font);
void SetTitlePSFontSize(int size);
void SetTitlePSOnOrOff(Boolean state);
const char *GetTitlePSFont(void);
int GetTitlePSFontSize(void);
Boolean GetTitlePSOnOrOff(void);
