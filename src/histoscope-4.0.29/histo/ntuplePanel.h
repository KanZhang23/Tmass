/*******************************************************************************
*									       *
* ntuplePanel.h -- Histoscope ntuple window, and NPlot main window	       *
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
* July 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
/* define the number of possible fields in the ntuple panel as the maximum #
   of variables that can be displayed by any plotting widget (from histoP.h) */
#define N_FIELDS MAX_DISP_VARS

typedef struct _ntWidgets {
    Widget form;
    Widget list;
    Widget typeMenu;
    Widget plotBtn;
    Widget modifyBtn;
    Widget createBtn;
    Widget dismissBtn;
    Widget helpBtn;
    Widget assignBtns[N_FIELDS];
    Widget fieldLabels[N_FIELDS];
    Widget fields[N_FIELDS];
    int plotType;
    int id;
    short initialHeight;
} ntWidgets;

void CreateNTupleWindow(Display *display, hsNTuple *item, char *geometry);
void CreateNTupleWindowContents(Widget parent, ntWidgets *widgets);
void FillNTupleWindow(ntWidgets *widgets, hsNTuple *item);
void UpdateNTupleWindow(ntWidgets *widgets, hsNTuple *item);
void ClearNTupleWindow(ntWidgets *widgets);
int CountNtupleWindows(void);
void GetNtupleWindowsAndTitles(Window *windows, char **titles);
void FindAndUpdateNTuplePanelList(int id);
void InvalidateNTupleWindow(ntWidgets *widgets);
void InvalidateAllNTupleWindows(void);
void InvalidateNTuple(int id);
void ChangeNtuplePanelItemID(int oldID, int newID);
int NtupleIsDisplayed(int id);
void CloseAllNTupleWindows(void);
windowInfo *ViewNtuplePlot(Display *display, hsNTuple *nTuple, char *winID, 
    int plotType, int *varList, int *sliderList, char *geometry,
    colorScaleInfo *csInfo, widgetConfigInfo *confInfo, char **errMsg);
windowInfo *ViewMiniNtuplePlot(Widget parent, hsNTuple *nTuple, int plotType, 
		int *varList, int *sliderList, char **errMsg, Widget appShell,
		char **ntupleTitle, colorScaleInfo *csInfo,
                widgetConfigInfo *confInfo);
int NCurvesInPlot(int plotType, int *vars);
int PlotIsHist(int plotType);
