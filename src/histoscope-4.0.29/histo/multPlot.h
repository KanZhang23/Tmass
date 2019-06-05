/*******************************************************************************
*                                        				       *
* multPlot.h -- Multiple plot include file for Nirvana Histoscope tool	       *
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
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* April 10, 1995							       *
*									       *
* Written by Joy Kyriakopulos						       *
* 									       *
*******************************************************************************/

typedef struct _multiPlot {
    windowInfo *wInfo;			/* ptr to window Info structure  */
    Widget frame;			/* frame for an empty plot	 */
    Widget label;			/* label for plot title		 */
    Widget plotWidget;			/* plot widget created           */
} multiPlot;

typedef struct _multWindow {		/* Multiple plot window information */
    struct _multWindow *next;		/* pointer to next in list    */
    char windowID[MAX_WINDOWID_LEN];    /* window id string */
    char geometry[32];
    Widget appShell;			/* application shell parent   */
    Widget multWidget;			/* form containing all plots  */
    int numRows;
    int numCols;
    int dispLabels;			/* display plot Titles?	      */
    multiPlot *plot;			/* ptr to table of multiplots */
    struct _readyCallBack *readyCB;
} multWindow;

extern multWindow *MultWindowList;

multWindow *CreateMultPlotWin(void);
multWindow *CreateMultPlotFromList(Widget parent, char *windowTitle, char *winID,
	       int numRows, int numCols, int numPlots, int *ids, int *errsDisp);
multWindow *CreateEmptyMultPlot(Widget parent, char *windowTitle, char *winID,
               int numRows, int numCols, char *geometry);
windowInfo *AddHistToMultiPlot(multWindow *w, hsGeneral *item, int row,int col,
	       windowInfo *ntupleSourceWinfo, int headingType, int dispErrs,
               colorScaleInfo *csInfo, widgetConfigInfo *confInfo);
windowInfo *AddNtupleToMultiPlot(multWindow *w, hsNTuple *item, int row,
	       int col, int plotType, int *varList, int *sliderList,
	       colorScaleInfo *csInfo, widgetConfigInfo *confInfo, char **errMsg);
int CountMultWindows(void);
void GetMplotWindowsAndTitles(Window *windows, char **titles);
void CloseAllMultWindows(void);
int CloseMiniPlot(windowInfo *wInfo);
int SetLabelsOnMultPlot(windowInfo *wInfo);
int SetNoMultPlotLabels(windowInfo *wInfo);
multWindow *GetMPlotInfoFromFrame(Widget frame, int *row, int *col); 
void CloseMPlotFromWinfo(windowInfo *wInfo);
void PrintMultiPlotWin(Widget shell, char *fileName);
Boolean InqMiniTitleState(Widget shell);
void DefaultMultiPlotGeom(int numRows, int numCols, int *width, int *height);
Widget GetMiniPlotLabel(windowInfo *wInfo);
int ReFillMultiPlot(windowInfo *wInfo, int numRows, int numCols);
int GetStatsForMultiPlot(windowInfo *wInfo, int *numRows, int *numCols,
			 int *numPlots, int *numUnused);
void CloseMPlotWindowById(void *wInfo, int taskNumber, char *c);
void GenerateMPlotPSById(void *wInfo, int taskNumber, char *c);
void RedisplayAllColoredWindows(const ColorScale *scale);
/* The following function returns True if
   more updates needs to be done in the future*/
Boolean MPlotRefreshCycle(void);

void SetMplotTitlePSFont(const char *font);
void SetMplotTitlePSFontSize(int size);
void SetMplotTitlePSOnOrOff(Boolean state);
const char *GetMplotTitlePSFont(void);
int GetMplotTitlePSFontSize(void);
Boolean GetMplotTitlePSOnOrOff(void);
