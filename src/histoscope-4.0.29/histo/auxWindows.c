/*******************************************************************************
*									       *
* auxWindows.c -- Slider and statistics windows for plots		       *
*									       *
* Copyright (c) 1991, 1993 Universities Research Association, Inc.	       *
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
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "../histo_util/histoUtil.h"
#include "MarginSlider.h"
#include "plotWindows.h"
#include "mainPanel.h"
#include "interpret.h"
#include "variablePanel.h"
#include "auxWindows.h"

#define SLIDER_MAX 1000		/* Arbitrary max value for animation sliders */
#define MIN_BINS 1		/* Minimum range of rebin sliders */
#define MAX_BINS_1D 10000	/* Maximum range of rebin sliders in 1d */
#define MAX_BINS_ND 1000        /* Maximum range of rebin sliders in 2d */
#define BIN_LIMIT_SLIDER_MAX 370/* Arbitrary max value for bin limit sliders */

static void sliderCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void sliderValueCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void sliderCloseCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void sliderRadioACB(Widget w, windowInfo *wInfo, 
			XmToggleButtonCallbackStruct *callData);
static void sliderRadioBCB(Widget w, windowInfo *wInfo, 
			XmToggleButtonCallbackStruct *callData);
static void binLimitCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellNormCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void rebinXCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void rebinYCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void rebinXValueCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void rebinYValueCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void binLimitValueCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellNormLowValueCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellNormUpValueCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellChangeScaleCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void rebinCloseCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void cellNormCloseCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void statsCloseCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void coordsCloseCB(Widget w, windowInfo *wInfo, caddr_t callData);
static XmString createStatText(windowInfo *wInfo);
static XmString createNTupleStatText(hsNTuple *item, plotInfo *pInfo);
static XmString create1DHistStatText(hs1DHist *item);
static XmString create2DHistStatText(hs2DHist *item);
static int sliderToBinLimit(int sliderValue);
static int binLimitToSlider(int limit);
static void updateSliderValueLabel(windowInfo *wInfo, int sliderNum);
static void updateSliderScalePosition(windowInfo *wInfo, int sliderNum);

/*
** Create a slider window, positioning it under the left edge of the plot
** window, unless otherwise directed by a non-null "geometry" argument.
**
** Sliders currently work only on the "original" plot in in overlaid plot
** window (the plot described by wInfo->pInfo[0]).
*/
Widget CreateSliderWindow(windowInfo *wInfo, char *geometry)
{
    int *sliders = wInfo->pInfo[0]->sliderVars;
    Widget dialog, form, scale, name, value, topWidget, radioA, radioB,
    	radioBox;
    hsNTuple *ntuple;
    int i;
    short shellX, shellY, shellWidth, shellHeight;
    XmString s1;
    Widget parent = wInfo->widget;
     	
    /* if the window has no sliders, return NULL */
    if (sliders[0] == -1 && sliders[1] == -1 && sliders[2] == -1)
    	return NULL;

    /* return NULL if item not an ntuple, fetch ntuple for variable names */
    ntuple = (hsNTuple *)GetMPItemByID(wInfo->pInfo[0]->id);
    if (ntuple->type != HS_NTUPLE)
    	return NULL;

    /* create a dialog shell and set its position relative to the
       plot window, or according to specified geometry */
    if (geometry != NULL) {
    	dialog = XtVaCreateWidget("Animation Sliders", xmDialogShellWidgetClass,
    	    	parent, XmNgeometry, geometry, NULL);
    } else {
	XtVaGetValues(wInfo->shell, XmNx, &shellX, XmNy, &shellY,
    		XmNwidth, &shellWidth, XmNheight, &shellHeight, NULL);    
	dialog = XtVaCreateWidget("Animation Sliders", xmDialogShellWidgetClass,
    		parent, XmNx, shellX, XmNy, shellY + shellHeight, NULL);
    }
    AddMotifCloseCallback(dialog, (XtCallbackProc)sliderCloseCB, wInfo);

    /* create a form widget to hold everything */
    form = XtVaCreateWidget("sliders", xmFormWidgetClass, dialog,
    	    XmNdefaultPosition, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    wInfo->sliderWindow = form;
    
    /* calculate the limits of the slider ranges (must be done before
       updating the scale positions and value labels) */
    UpdateSliderRange(wInfo);

    /* create the contents of the slider window */
    topWidget = NULL;
    for (i=0; i<N_SLIDERS; i++) {
    	if (sliders[i] != -1) {
	    radioBox = XtVaCreateManagedWidget("radioBox",
	    	    xmRowColumnWidgetClass, form,
	    	    XmNradioBehavior, True,
	    	    XmNradioAlwaysOne, True,
	    	    XmNorientation, XmHORIZONTAL,
	    	    /*XmNresizeWidth, False,
	    	    XmNresizeHeight, False,
    		    XmNrecomputeSize, False,*/
    		    XmNleftAttachment, XmATTACH_POSITION,
    		    XmNtopAttachment, topWidget==NULL ? 
    			    XmATTACH_FORM : XmATTACH_WIDGET,
    		    XmNleftPosition, 1,
    		    XmNtopWidget, topWidget, NULL);
	    radioA = XtVaCreateManagedWidget("radioA",xmToggleButtonWidgetClass,
	    	    radioBox, 
	    	    XmNset, (wInfo->pInfo[0]->sliderGTorLT[i] == SLIDER_LT),
    		    XmNlabelString,s1=XmStringCreateSimple("<"),
                    XmNuserData, (XtPointer)((long)i), NULL);
	    XmStringFree(s1);
	    XtAddCallback(radioA, XmNvalueChangedCallback,
		    (XtCallbackProc)sliderRadioACB, (caddr_t)wInfo);
	    radioB = XtVaCreateManagedWidget("radioB",xmToggleButtonWidgetClass,
	    	    radioBox, 
	    	    XmNset, (wInfo->pInfo[0]->sliderGTorLT[i] == SLIDER_GT),
    		    XmNlabelString,s1=XmStringCreateSimple(">"),
    		    XmNuserData, (XtPointer)((long)i), NULL);
	    XmStringFree(s1);
	    XtAddCallback(radioB, XmNvalueChangedCallback,
		    (XtCallbackProc)sliderRadioBCB, (caddr_t)wInfo);
	    scale = XtVaCreateManagedWidget("scale", xmScaleWidgetClass, form,
    		    XmNorientation, XmHORIZONTAL,
    		    XmNminimum, 0,
    		    XmNmaximum, SLIDER_MAX,
    		    XmNvalue, SLIDER_MAX,
    		    XmNrightAttachment, XmATTACH_POSITION,
    		    XmNtopAttachment, topWidget==NULL ? 
    			    XmATTACH_FORM : XmATTACH_WIDGET,
    		    XmNleftAttachment, XmATTACH_WIDGET,
    		    XmNrightPosition, 99,
    		    XmNtopWidget, topWidget, 
    		    XmNleftWidget, radioBox, 
    		    XmNtopOffset, 3,
                    XmNuserData, (XtPointer)((long)i), NULL);
	    XtAddCallback(scale, XmNdragCallback,
	    	    (XtCallbackProc)sliderCB, (caddr_t)wInfo);
	    XtAddCallback(scale, XmNvalueChangedCallback,
	    	    (XtCallbackProc)sliderCB, (caddr_t)wInfo);
	    wInfo->sliderScales[i] = scale;
	    value = XtVaCreateManagedWidget("value", xmTextWidgetClass, form, 
    		    XmNcolumns, 10,
    		    XmNvalue, "All",
    		    XmNuserData, (XtPointer)((long)i),
    		    XmNmarginHeight, 0,
    		    XmNmarginWidth, 1,
    		    XmNshadowThickness, 0,
    		    XmNleftAttachment, XmATTACH_WIDGET,
    		    XmNrightAttachment, XmATTACH_POSITION,
    		    XmNtopAttachment, XmATTACH_WIDGET,
    		    XmNleftWidget, radioBox,
    		    XmNrightPosition, 62,
    		    XmNtopWidget, scale, NULL);
	    XtAddCallback(value, XmNactivateCallback,
	    	    (XtCallbackProc)sliderValueCB, (caddr_t)wInfo);
	    wInfo->sliderLabels[i] = value;
	    name = XtVaCreateManagedWidget("name", xmLabelWidgetClass, form, 
    		    XmNlabelString, s1=XmStringCreateSimple(
    		    	ExtNTVarName(ntuple, sliders[i])),
    		    XmNalignment, XmALIGNMENT_END,
    		    XmNleftAttachment, XmATTACH_POSITION,
    		    XmNrightAttachment, XmATTACH_POSITION,
    		    XmNtopAttachment, XmATTACH_WIDGET,
    		    XmNleftPosition, 62,
    		    XmNrightPosition, 98,
    		    XmNtopWidget, scale, NULL);
	    XmStringFree(s1);

    	    updateSliderValueLabel(wInfo, i);
    	    updateSliderScalePosition(wInfo, i);
	    topWidget = name;
	}
    }
    return form;
}

Widget CreateRebinWindow(windowInfo *wInfo, int nDim, char *geometry)
{
    Widget dialog, form, scale, name, value, topWidget;
    hsNTuple *ntuple;
    short shellX, shellY, shellWidth, shellHeight;
    char nameStr[HS_MAX_NAME_LENGTH + 6];
    int i;
    XmString s1;
    Widget parent = wInfo->widget;
    int max_bins;
    	
    /* return NULL if item not an ntuple, fetch ntuple for variable names */
    ntuple = (hsNTuple *)GetMPItemByID(wInfo->pInfo[0]->id);
    if (ntuple->type != HS_NTUPLE)
    	return NULL;

    /* create a dialog shell and set its position relative to the
       plot window, or according to specified geometry */
    if (geometry != NULL) {
    	dialog = XtVaCreateWidget("Rebin Histogram", xmDialogShellWidgetClass,
    		parent, XmNgeometry, geometry, NULL);
    } else {
	XtVaGetValues(wInfo->shell, XmNx, &shellX, XmNy, &shellY,
    		XmNwidth, &shellWidth, XmNheight, &shellHeight, NULL);    
	dialog = XtVaCreateWidget("Rebin Histogram", xmDialogShellWidgetClass,
    	 	parent, XmNx, shellX+shellWidth/2, XmNy, shellY+shellHeight, NULL);
    }
    AddMotifCloseCallback(dialog, (XtCallbackProc)rebinCloseCB, wInfo);
    
    /* create the contents of the rebin slider window */
    form = XtVaCreateWidget("rebin", xmFormWidgetClass, dialog,
    	    XmNdefaultPosition, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    topWidget = NULL;
    if (nDim == 1)
	max_bins = MAX_BINS_1D;
    else
	max_bins = MAX_BINS_ND;
    for (i=0; i<nDim; i++) {
	scale = XtVaCreateManagedWidget("scale", xmScaleWidgetClass, form,
    		XmNorientation, XmHORIZONTAL,
    		XmNminimum, MIN_BINS,
    		XmNmaximum, max_bins,
    		XmNscaleMultiple, 10,
    		XmNvalue, i==0 ? wInfo->pInfo[0]->nXBins : wInfo->pInfo[0]->nYBins,
    		XmNwidth, 200,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, topWidget==NULL ? 
    			XmATTACH_FORM : XmATTACH_WIDGET,
    		XmNleftPosition, 1,
    		XmNrightPosition, 99,
    		XmNtopWidget, topWidget,
    		XmNtopOffset, 3, NULL);
	XtAddCallback(scale, XmNdragCallback,
		(XtCallbackProc)(i==0 ? rebinXCB : rebinYCB), (caddr_t)wInfo);
	XtAddCallback(scale, XmNvalueChangedCallback,
		(XtCallbackProc)(i==0 ? rebinXCB : rebinYCB), (caddr_t)wInfo);
	wInfo->rebinScales[i] = scale;
	sprintf(nameStr, "%s bins", ExtNTVarName(ntuple, wInfo->pInfo[0]->ntVars[i]));
	value = XtVaCreateManagedWidget("value", xmTextWidgetClass, form, 
    		XmNcolumns, 5,
    		XmNmarginHeight, 0,
    		XmNmarginWidth, 1,
    		XmNshadowThickness, 0,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftPosition, 1,
    		XmNrightPosition, 40,
    		XmNtopWidget, scale, NULL);
	XtAddCallback(value, XmNactivateCallback, (XtCallbackProc)
		(i==0 ? rebinXValueCB : rebinYValueCB), (caddr_t)wInfo);
	wInfo->rebinLabels[i] = value;
	name = XtVaCreateManagedWidget("name", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple(nameStr),
    		XmNalignment, XmALIGNMENT_END,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNleftPosition, 41,
    		XmNrightPosition, 99,
    		XmNtopWidget, scale, NULL);
	XmStringFree(s1);
	SetIntText(value, i==0 ? wInfo->pInfo[0]->nXBins : wInfo->pInfo[0]->nYBins);
	topWidget = name;
    }
    XtVaCreateManagedWidget("msg", xmLabelWidgetClass, form, 
    	    XmNlabelString, s1=XmStringCreateSimple(
    		    "Use arrow keys for fine adjustments"),
    	    XmNalignment, XmALIGNMENT_CENTER,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 99,
    	    XmNtopWidget, topWidget, NULL);
    XmStringFree(s1);
    wInfo->rebinWindow = form;
    return form;
}

Widget CreateBinLimitWindow(windowInfo *wInfo, char *geometry)
{
    Widget dialog, form, scale, name, value;
    short shellX, shellY, shellWidth, shellHeight;
    XmString s1;
    Widget parent = wInfo->widget;
    	
    /* create a dialog shell and set its position relative to the
       plot window, or according to specified geometry */
    if (geometry != NULL) {
    	dialog = XtVaCreateWidget("Bin Limits", xmDialogShellWidgetClass,
    		parent, XmNgeometry, geometry, NULL);
    } else {
	XtVaGetValues(wInfo->shell, XmNx, &shellX, XmNy, &shellY,
    		XmNwidth, &shellWidth, XmNheight, &shellHeight, NULL);    
	dialog = XtVaCreateWidget("Bin Limits", xmDialogShellWidgetClass,
    	     	parent, XmNx, shellX+shellWidth/2, XmNy, shellY+shellHeight, NULL);
    }
    AddMotifCloseCallback(dialog, (XtCallbackProc)rebinCloseCB, wInfo);
    	 		  
    /* create the contents of the bin limits slider window */
    form = XtVaCreateWidget("rebin", xmFormWidgetClass, dialog,
    	    XmNdefaultPosition, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    scale = XtVaCreateManagedWidget("scale", xmScaleWidgetClass, form,
    	    XmNorientation, XmHORIZONTAL,
    	    XmNminimum, 1,
    	    XmNmaximum, BIN_LIMIT_SLIDER_MAX,
    	    XmNscaleMultiple, 10,
    	    XmNvalue, binLimitToSlider(wInfo->pInfo[0]->nXBins),
    	    XmNwidth, 200,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 99,
    	    XmNtopOffset, 3, NULL);
    XtAddCallback(scale, XmNdragCallback, (XtCallbackProc)binLimitCB,
    	    (caddr_t)wInfo);
    XtAddCallback(scale, XmNvalueChangedCallback, (XtCallbackProc)binLimitCB,
    	    (caddr_t)wInfo);
    value = XtVaCreateManagedWidget("value", xmTextWidgetClass, form, 
    	    XmNcolumns, 5,
    	    XmNmarginHeight, 0,
    	    XmNmarginWidth, 1,
    	    XmNshadowThickness, 0,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 40,
    	    XmNtopWidget, scale, NULL);
    XtAddCallback(value, XmNactivateCallback,
	    (XtCallbackProc)binLimitValueCB, (caddr_t)wInfo);
    name = XtVaCreateManagedWidget("name", xmLabelWidgetClass, form, 
    	    XmNlabelString, s1=XmStringCreateSimple("Max Counts/Bin"),
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 41,
    	    XmNrightPosition, 99,
    	    XmNtopWidget, scale, NULL);
    XmStringFree(s1);
    XtVaCreateManagedWidget("msg", xmLabelWidgetClass, form, 
    	    XmNlabelString, s1=XmStringCreateSimple(
    		    "Use arrow keys for fine adjustments"),
    	    XmNalignment, XmALIGNMENT_CENTER,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 99,
    	    XmNtopWidget, name, NULL);
    XmStringFree(s1);
    
    /* Store the information about the bin limit slider window as if it
       were a rebin slider.  Other than appearance and limit values, the
       bin limits slider is the same as a rebin slider and uses the same
       callbacks and storage in the windowInfo structure */
    wInfo->rebinWindow = form;
    wInfo->rebinLabels[0] = value;
    wInfo->rebinScales[0] = scale;
    SetIntText(value, wInfo->pInfo[0]->nXBins);
    return form;
}

Widget CreateCellNormalizeWindow(windowInfo *wInfo, char *geometry)
{
    Widget dialog, form, margin, lowValue, upValue;
    Widget aLogRadio, cellLinA;
    short shellX, shellY, shellWidth, shellHeight;
    XmString s1;
    Widget parent = wInfo->widget;
    	
    /* create a dialog shell and set its position relative to the
       plot window, or according to specified geometry */
    if (geometry != NULL) {
    	dialog = XtVaCreateWidget("Normalization", xmDialogShellWidgetClass,
    		parent, XmNgeometry, geometry, NULL);
    } else {
	XtVaGetValues(wInfo->shell, XmNx, &shellX, XmNy, &shellY,
    		XmNwidth, &shellWidth, XmNheight, &shellHeight, NULL);    
	dialog = XtVaCreateWidget("Normalization", xmDialogShellWidgetClass,
    		parent, XmNx, shellX+shellWidth/2, XmNy, shellY+shellHeight, NULL);
    }
    AddMotifCloseCallback(dialog, (XtCallbackProc)cellNormCloseCB, wInfo);
    	 		  
    /* create the contents of the cell normalization slider window */
    form = XtVaCreateWidget("cellNorm", xmFormWidgetClass, dialog,
    	    XmNdefaultPosition, False, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    wInfo->cellNormWindow = form;
    margin = XtVaCreateManagedWidget("scale", marginSliderWidgetClass, form,
    	    XmNorientation, XmHORIZONTAL,
    	    XmNminimum, 1,
    	    XmNmaximum, BIN_LIMIT_SLIDER_MAX,
    	    XmNscaleMultiple, 10,
    	    XmNvalue, binLimitToSlider(wInfo->pInfo[0]->nXBins),
    	    XmNwidth, 200,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 99,
    	    XmNtopOffset, 3, NULL);
    XtAddCallback(margin, XmNmarginDragCallback, (XtCallbackProc)cellNormCB,
    	    (caddr_t)wInfo);
    XtAddCallback(margin, XmNmarginChangedCallback, (XtCallbackProc)cellNormCB,
    	    (caddr_t)wInfo);
    wInfo->cellNormScale = margin;
    lowValue = XtVaCreateManagedWidget("lowValue", xmTextWidgetClass, form, 
    	    XmNcolumns, 10,
    	    XmNmarginHeight, 0,
    	    XmNmarginWidth, 1,
    	    XmNshadowThickness, 0,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 40,
    	    XmNtopWidget, margin, NULL);
    XtAddCallback(lowValue, XmNactivateCallback,
	    (XtCallbackProc)cellNormLowValueCB, (caddr_t)wInfo);
    wInfo->cellNormLowLabel = lowValue;
    upValue = XtVaCreateManagedWidget("upValue", xmTextWidgetClass, form, 
    	    XmNcolumns, 10,
    	    XmNmarginHeight, 0,
    	    XmNmarginWidth, 1,
    	    XmNshadowThickness, 0,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 60,
    	    XmNrightPosition, 99,
    	    XmNtopWidget, margin, NULL);
    XtAddCallback(upValue, XmNactivateCallback,
	    (XtCallbackProc)cellNormUpValueCB, (caddr_t)wInfo);
    wInfo->cellNormUpLabel = upValue;
    
    /* display the initial slider values */
    UpdateCellNormValueLabels(wInfo);
    /* Add a set of 2 radio buttons to control Lin vs Log scaling
    **  P.L. Feb. 96 */
    aLogRadio = XtVaCreateManagedWidget("aLogRadio",xmRowColumnWidgetClass,form,
    		XmNradioBehavior, True,
    		XmNleftAttachment, XmATTACH_FORM,
    		XmNtopAttachment, XmATTACH_WIDGET,
	    	XmNorientation, XmHORIZONTAL,
    		XmNtopWidget, lowValue,
    		XmNtopOffset, 10, NULL);
    cellLinA = XtVaCreateManagedWidget("linY", xmToggleButtonWidgetClass,
                aLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Linear Scaling"),
    		XmNset, True,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    XtAddCallback(cellLinA,  XmNvalueChangedCallback,
		    (XtCallbackProc)cellChangeScaleCB, (caddr_t) wInfo);
    wInfo->cellLogA = XtVaCreateManagedWidget("logY", xmToggleButtonWidgetClass,
                aLogRadio,
    		XmNlabelString, s1=XmStringCreateSimple("Log. Scaling"),
    		XmNset, False,
    		XmNmarginHeight, 0, NULL);
    XmStringFree(s1);
    XtAddCallback(wInfo->cellLogA,  XmNvalueChangedCallback,
		    (XtCallbackProc)cellChangeScaleCB, (caddr_t) wInfo);
    
    return form;
}


Widget CreateStatsWindow(windowInfo *wInfo, char *geometry)
{
    Widget dialog, label;
    short shellX, shellY, shellWidth, shellHeight;
    XmString statText;
    Widget parent = wInfo->widget;
    	
    /* generate statistics text */
    statText = createStatText(wInfo);

    /* create a dialog shell and set its position relative to the
       plot window, or according to specified geometry */
    if (geometry != NULL) {
    	dialog = XtVaCreateWidget("Rebin Histogram", xmDialogShellWidgetClass,
    		parent, XmNgeometry, geometry, NULL);
    } else {
	XtVaGetValues(wInfo->shell, XmNx, &shellX, XmNy, &shellY,
    		XmNwidth, &shellWidth, XmNheight, &shellHeight, NULL);    
	dialog = XtVaCreateWidget("Statistics", xmDialogShellWidgetClass,
    		parent, XmNx, shellX+shellWidth, XmNy, shellY, NULL);
    }
    AddMotifCloseCallback(dialog, (XtCallbackProc)statsCloseCB, wInfo);
    	 		  
    /* create the label to hold the statistics text */
    label = XtVaCreateWidget("stats", xmLabelWidgetClass, dialog,
    	    XmNlabelString, statText,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginLeft, 5,
    	    XmNmarginRight, 5,
    	    XmNmarginTop, 5,
    	    XmNmarginBottom, 5, NULL);
    XmStringFree(statText);
    wInfo->statsWindow = label;
    return label;
}

Widget CreateCoordsWindow(windowInfo *wInfo, char *geometry, int hasPlotCoords)
{
    Widget dialog, form, label;
    Widget labs, lrel, lplot = NULL;
    Widget dispabs, disprel, dispplot = NULL;
    short shellX, shellY, shellWidth, shellHeight;
    XmString labelText;
    Widget parent = wInfo->widget;
    hsGeneral *item;
    const char *message = "To view point coordinates\n"
                          "right-click with Shift\n"
                          "inside the plot area";
    char title[HS_MAX_TITLE_LENGTH + 100];

    /* create a dialog shell */
    XtVaGetValues(wInfo->shell, XmNx, &shellX, XmNy, &shellY,
                  XmNwidth, &shellWidth, XmNheight, &shellHeight, NULL);    
    dialog = XtVaCreateWidget("Coordinates", xmDialogShellWidgetClass,
                              parent, XmNx, shellX+shellWidth, XmNy, shellY, NULL);
    AddMotifCloseCallback(dialog, (XtCallbackProc)coordsCloseCB, wInfo);

    /* create a top level form child of the dialog */
    form = XmCreateForm(dialog, "coordsform", NULL, 0);

    /* create the label to hold the prompt text */
    item = GetMPItemByID(wInfo->pInfo[0]->id);
    sprintf(title, "%s\n\n%s", item->title, message);
    labelText = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
    label = XtVaCreateManagedWidget(
        "label", xmLabelWidgetClass, form,
        XmNrightAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        XmNlabelString, labelText,
        XmNalignment, XmALIGNMENT_CENTER,
        XmNmarginLeft, 5,
        XmNmarginRight, 5,
        XmNmarginTop, 5,
        XmNmarginBottom, 5,
        NULL);
    XmStringFree(labelText);

    /* Absolute window coords */
    labelText = XmStringCreateLtoR(" Absolute: ", XmSTRING_DEFAULT_CHARSET);
    labs = XtVaCreateManagedWidget(
        "labs", xmLabelWidgetClass, form,
        XmNlabelString, labelText,
        XmNalignment, XmALIGNMENT_END,
        XmNmarginTop, 5,
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, label,
        NULL);
    XmStringFree(labelText);
    dispabs = XtVaCreateManagedWidget(
        "abs", xmTextFieldWidgetClass, form,
        XmNeditable, False,
        XmNblinkRate, 0,
        XmNcursorPositionVisible, False,
        XmNverifyBell, False,
        XmNhighlightThickness, 0,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 5,
        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
        XmNtopWidget, labs,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, labs,
        NULL);

    /* Relative window coords */
    labelText = XmStringCreateLtoR(" Relative: ", XmSTRING_DEFAULT_CHARSET);
    lrel = XtVaCreateManagedWidget(
        "lrel", xmLabelWidgetClass, form,
        XmNlabelString, labelText,
        XmNalignment, XmALIGNMENT_END,
        XmNmarginTop, 5,
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, dispabs,
        XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
        XmNrightWidget, labs,
        NULL);
    XmStringFree(labelText);
    disprel = XtVaCreateManagedWidget(
        "rel", xmTextFieldWidgetClass, form,
        XmNeditable, False,
        XmNblinkRate, 0,
        XmNcursorPositionVisible, False,
        XmNverifyBell, False,
        XmNhighlightThickness, 0,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 5,
        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
        XmNtopWidget, lrel,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, labs,
        NULL);

    /* Plot coords */
    if (hasPlotCoords)
    {
        labelText = XmStringCreateLtoR(" Plot: ", XmSTRING_DEFAULT_CHARSET);
        lplot = XtVaCreateManagedWidget(
            "lplot", xmLabelWidgetClass, form,
            XmNlabelString, labelText,
            XmNalignment, XmALIGNMENT_END,
            XmNmarginTop, 5,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, disprel,
            XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
            XmNrightWidget, labs,
            NULL);
        XmStringFree(labelText);
        dispplot = XtVaCreateManagedWidget(
            "plot", xmTextFieldWidgetClass, form,
            XmNeditable, False,
            XmNblinkRate, 0,
            XmNcursorPositionVisible, False,
            XmNverifyBell, False,
            XmNhighlightThickness, 0,
            XmNrightAttachment, XmATTACH_FORM,
            XmNrightOffset, 5,
            XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
            XmNtopWidget, lplot,
            XmNleftAttachment, XmATTACH_WIDGET,
            XmNleftWidget, labs,
            NULL);
    }

    /* Insert a dummy frame to make some space on the bottom */
    {
        Widget dummy = XtVaCreateManagedWidget(
            "space", xmBulletinBoardWidgetClass, form,
            XmNheight, 4,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNrightOffset, 5,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_WIDGET,
            NULL);
        if (hasPlotCoords)
            XtVaSetValues(dummy, XmNtopWidget, dispplot, NULL);
        else
            XtVaSetValues(dummy, XmNtopWidget, disprel, NULL);
    }

    wInfo->coordsWindow = form;
    wInfo->coAbs = dispabs;
    wInfo->coRel = disprel;
    wInfo->coPlot = dispplot;
    return form;
}

/*
** Write PostScript commands for drawing the statistics window to a file.
*/
void WriteStatsPS(FILE *psFile, windowInfo *wInfo, char *fontName,
	int fontSize, int xPos, int yPos)
{
    char *statsStr, *c;
    XmString statsMStr;
    int lineStart;
    
    /* Get the stats text and convert it to a null terminated string */
    statsMStr = createStatText(wInfo);
    statsStr = GetXmStringText(statsMStr);
    XmStringFree(statsMStr);

    /* Set up the font and the initial position */
    fprintf(psFile, "/%s findfont %04d scalefont setfont\n", fontName,
	    fontSize);
    lineStart = yPos - fontSize;
    fprintf(psFile, "%d %d moveto\n", xPos, lineStart);
    
    /* Break the string into lines and output each line to psFile in
       the form of PostScript string drawing commands */
    fputc('(', psFile);
    for (c=statsStr; *c!='\0'; c++) {
    	if (*c == '\n') {
    	    lineStart -= fontSize + 1;
    	    fprintf(psFile, ") show\n%d %d moveto\n(", xPos, lineStart);
    	} else
    	    fputc(*c, psFile);
    }
    fprintf(psFile, ") show\n");   
    XtFree(statsStr); 
}

/*
** Update the statistics window for "wInfo" if it is displayed
*/
void UpdateStatsWindow(windowInfo *wInfo)
{
    XmString statText;
    
    /* check if stats window is displayed.  return if it's not */
    if (wInfo->statsWindow == NULL)
    	return;
    if (!XtIsManaged(wInfo->statsWindow))
    	return;
    	
    statText = createStatText(wInfo);
    XtVaSetValues(wInfo->statsWindow, XmNlabelString, statText, NULL);
    XmStringFree(statText);
}

/*
** refresh the minimum and maximum values used by the animation sliders
*/
void UpdateSliderRange(windowInfo *wInfo)
{
    ntupleExtension *ntExt;
    hsNTuple *firstItem;
    plotInfo *pInfo;
    int i;

    /* save some calculation if sliders have not been displayed yet */
    if (wInfo->sliderWindow == NULL)
    	return;
    
    /* get the first plot item, and make sure it's an ntuple */
    pInfo = wInfo->pInfo[0];
    firstItem = (hsNTuple *)GetMPItemByID(pInfo->id);
    if (firstItem->type != HS_NTUPLE)
    	return;
    ntExt = GetNTupleExtension(pInfo->id);
    
    /* recalculate ranges for each slider variable */
    for (i=0; i<N_SLIDERS; i++) {
	if (pInfo->sliderVars[i] != -1)
	    ExtCalcNTVarRange(firstItem, ntExt, pInfo->sliderVars[i],
	    	    &wInfo->sliderMin[i], &wInfo->sliderMax[i]);
    }
}

static void sliderCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int sliderNum, sliderValue;
    float thresh;
    XtPointer userData;
    
    /* Find out which slider this is and where it's positioned */
    XtVaGetValues(w, XmNuserData, &userData,
                  XmNvalue, &sliderValue, NULL);
    sliderNum = (long)userData;

    /* Compute the new slider threshold */
    if (sliderValue == SLIDER_MAX)
    	thresh = FLT_MAX;
    else if (sliderValue == 0)
    	thresh = -FLT_MAX;
    else
    	thresh = wInfo->sliderMin[sliderNum] + sliderValue *
    	 (wInfo->sliderMax[sliderNum] - wInfo->sliderMin[sliderNum])/SLIDER_MAX;

    /* Do nothing if it hasn't changed */
    if (thresh == wInfo->pInfo[0]->sliderThresholds[sliderNum])
    	return;
    	
    /* Display the new threshold value in the slider window */
    wInfo->pInfo[0]->sliderThresholds[sliderNum] = thresh;
    updateSliderValueLabel(wInfo, sliderNum);
    
    /* redisplay the data using the new threshold */
    RedisplayPlotWindow(wInfo, ANIMATION);
    UpdateStatsWindow(wInfo);
}

static void sliderValueCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int sliderNum;
    double thresh;
    char *valueText;
    XtPointer userData;
    
    /* Find out which slider label this is */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    sliderNum = (long)userData;
    
    /* Get the value from it */
    valueText = XmTextGetString(w);
    if (!strcasecmp(valueText, "all"))
    	thresh = FLT_MAX;
    else if (!strcasecmp(valueText, "none"))
    	thresh = -FLT_MAX;
    else if (GetFloatTextWarn(w, &thresh, "value", True) != TEXT_READ_OK)
    	return;
    XtFree(valueText);

    /* Do nothing if it hasn't changed */
    if (thresh == wInfo->pInfo[0]->sliderThresholds[sliderNum])
    	return;
    
    /* Move the slider to correspond with the new value */
    wInfo->pInfo[0]->sliderThresholds[sliderNum] = thresh;
    updateSliderScalePosition(wInfo, sliderNum);
    
    /* Redisplay the data using the new threshold */
    RedisplayPlotWindow(wInfo, ANIMATION);
    UpdateStatsWindow(wInfo);
}

static void sliderCloseCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XmToggleButtonSetState(wInfo->sliderMenuItem, False, True);
}

static void sliderRadioACB(Widget w, windowInfo *wInfo, 
			XmToggleButtonCallbackStruct *callData)
{
    int sliderNum;
    XtPointer userData;

    /* respond only when toggled on */
    if (!callData->set)
    	return;
    	
    /* find out which slider this is */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    sliderNum = (long)userData;

    /* change the slider polarity to < */
    wInfo->pInfo[0]->sliderGTorLT[sliderNum] = SLIDER_LT;

    /* if label is All or None, switch it to the other */
    updateSliderValueLabel(wInfo, sliderNum);

    /* redisplay the data */
    RedisplayPlotWindow(wInfo, ANIMATION);
    UpdateStatsWindow(wInfo);
}

static void sliderRadioBCB(Widget w, windowInfo *wInfo, 
			XmToggleButtonCallbackStruct *callData)
{
    int sliderNum;
    XtPointer userData;

    /* respond only when toggled on */
    if (!callData->set)
    	return;
    	
    /* Find out which slider this is */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    sliderNum = (long)userData;

    /* Change the slider polarity to > */
    wInfo->pInfo[0]->sliderGTorLT[sliderNum] = SLIDER_GT;

    /* if label is All or None, switch it to the other */
    updateSliderValueLabel(wInfo, sliderNum);

    /* redisplay the data */
    RedisplayPlotWindow(wInfo, ANIMATION);
    UpdateStatsWindow(wInfo);
}

static void binLimitCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int sliderValue, limit;
    
    /* get the slider position and see if the number of bins has changed */
    XtVaGetValues(w, XmNvalue, &sliderValue, NULL);
    limit = sliderToBinLimit(sliderValue);
    if (limit == wInfo->pInfo[0]->nXBins)
    	return;
    
    /* redisplay the data using the new number of bins */
    wInfo->pInfo[0]->nXBins = limit;
    SetIntText(wInfo->rebinLabels[0], limit);
    RedisplayPlotWindow(wInfo, REBIN);
}

static void cellNormCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    RedisplayPlotWindow(wInfo, RENORMALIZE);
}

static void rebinXCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int nBins;
    
    /* get the slider position and see if the number of bins has changed */
    XtVaGetValues(w, XmNvalue, &nBins, NULL);
    if (nBins == wInfo->pInfo[0]->nXBins)
    	return;
    
    /* redisplay the data using the new number of bins */
    wInfo->pInfo[0]->nXBins = nBins;
    SetIntText(wInfo->rebinLabels[0], nBins);
    RedisplayPlotWindow(wInfo, REBIN);
}

static void rebinYCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int nBins;
    
    /* get the slider position and see if the number of bins has changed */
    XtVaGetValues(w, XmNvalue, &nBins, NULL);
    if (nBins == wInfo->pInfo[0]->nYBins)
    	return;
    
    /* redisplay the data using the new number of bins */
    wInfo->pInfo[0]->nYBins = nBins;
    SetIntText(wInfo->rebinLabels[1], nBins);
    RedisplayPlotWindow(wInfo, REBIN);
}

static void rebinXValueCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int nBins, max_bins;
    
    /* Get the value from the widget */
    if (GetIntTextWarn(w, &nBins, "number of bins", True) != TEXT_READ_OK)
    	return;
    if (nBins <= 0) {
    	DialogF(DF_WARN, w, 1,"Number of bins must\nbe greater than zero","OK");
    	return;
    }
    
    /* Move the slider to correspond with the new value */
    XtVaGetValues(wInfo->rebinScales[0], XmNmaximum, &max_bins, NULL);
    XtVaSetValues(wInfo->rebinScales[0], XmNvalue,
		  nBins > max_bins ? max_bins : nBins, NULL);
    
    /* redisplay the data using the new number of bins */
    wInfo->pInfo[0]->nXBins = nBins;
    RedisplayPlotWindow(wInfo, REBIN);
}

static void rebinYValueCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int nBins, max_bins;
    
    /* Get the value from the widget */
    if (GetIntTextWarn(w, &nBins, "number of bins", True) != TEXT_READ_OK)
    	return;
    if (nBins <= 0) {
    	DialogF(DF_WARN, w, 1,"Number of bins must\nbe greater than zero","OK");
    	return;
    }
    
    /* Move the slider to correspond with the new value */
    XtVaGetValues(wInfo->rebinScales[1], XmNmaximum, &max_bins, NULL);
    XtVaSetValues(wInfo->rebinScales[1], XmNvalue,
		  nBins > max_bins ? max_bins : nBins, NULL);
    
    /* redisplay the data using the new number of bins */
    wInfo->pInfo[0]->nYBins = nBins;
    RedisplayPlotWindow(wInfo, REBIN);
}

static void binLimitValueCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    int binLimit, sliderValue;
    
    /* Get the value from the widget */
    if (GetIntTextWarn(w, &binLimit, "bin limit", True) != TEXT_READ_OK)
    	return;
    if (binLimit <= 0) {
    	DialogF(DF_WARN, w, 1, "Bin limit must be\ngreater than zero", "OK");
    	return;
    }
    
    /* Move the slider to correspond with the new value */
    sliderValue = binLimitToSlider(binLimit);
    if (sliderValue > BIN_LIMIT_SLIDER_MAX)
    	sliderValue = BIN_LIMIT_SLIDER_MAX;
    XtVaSetValues(wInfo->rebinScales[0], XmNvalue, sliderValue, NULL);
    
    /* redisplay the data using the new number of bins */
    wInfo->pInfo[0]->nXBins = binLimit;
    RedisplayPlotWindow(wInfo, REBIN);
}

static void cellNormLowValueCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    double value;
    
    /* Get the value from the widget */
    if (GetFloatTextWarn(w, &value, "minimum", True) != TEXT_READ_OK)
    	return;
    wInfo->cellNormMin = value;
    
    /* Slider position should be updated, but it's just too complicated,
       and this is not a very important part of histoscope */
    
    /* redisplay the data using the new cellNormMin */
    RedisplayPlotWindow(wInfo, REBIN);
}

static void cellNormUpValueCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    double value;
    
    /* Get the value from the widget */
    if (GetFloatTextWarn(w, &value, "maximum", True) != TEXT_READ_OK)
    	return;
    wInfo->cellNormMax = value;
    
    /* Slider position should be updated, but it's just too complicated,
       and this is not a very important part of histoscope */
    
    /* redisplay the data using the new cellNormMax */
    RedisplayPlotWindow(wInfo, REBIN);
}

static void cellChangeScaleCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    /* redisplay the data using the new cellNormMax */
    RedisplayPlotWindow(wInfo, REINIT);
}

static void rebinCloseCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XmToggleButtonSetState(wInfo->rebinMenuItem, False, True);
}

static void cellNormCloseCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XmToggleButtonSetState(wInfo->cellNormMenuItem, False, True);
}

static void statsCloseCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XmToggleButtonSetState(wInfo->statsMenuItem, False, True);
}

static void coordsCloseCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    XmToggleButtonSetState(wInfo->coordsMenuItem, False, True);
}

/*
** Create text for the statistics window to display
**
** ... Note: this still needs work, it only does stats for the first item
**  	     in an overlay plot.
*/
static XmString createStatText(windowInfo *wInfo)
{
    hsGeneral *item = GetMPItemByID(wInfo->pInfo[0]->id);
    
    if (item->type == HS_NTUPLE)
    	return createNTupleStatText((hsNTuple *)item, wInfo->pInfo[0]);
    else if (item->type == HS_1D_HISTOGRAM)
    	return create1DHistStatText((hs1DHist *)item);
    else if (item->type == HS_2D_HISTOGRAM)
    	return create2DHistStatText((hs2DHist *)item);
    fprintf(stderr, "type error createStatText\n");
    return NULL;
}
   
static XmString createNTupleStatText(hsNTuple *item, plotInfo *pInfo)
{
    int *ntVars = pInfo->ntVars;
    char statString[512], hbookString[30];
    XmString accumString, temp, statMString;
    float value, min, max;
    double sum, mean, sumDev, stdDev;
    int var, i, nPoints, nShown, errors, n = item->n, firstVar = True;
    ntupleExtension *ntExt = GetNTupleExtension(item->id);
    
    /* if item has an hbook id, include it in the string */
    if (item->hbookID != 0)
    	sprintf(hbookString, "HBOOK ID %d\n", item->hbookID);
    else
    	*hbookString = '\0';
    
    /* start the string with name, optional hbook id, and number of entries */
    sprintf(statString, "%s\n%s%d Entries", item->title, hbookString, n);
    accumString = XmStringCreateLtoR(statString, XmSTRING_DEFAULT_CHARSET);
    
    /* loop over variables designated for plot */
    for (var=0; var<MAX_DISP_VARS; var++) {
    	if (ntVars[var] != -1) {
    	    
    	    /* calculate the min, max, sum, and number of points displayed */
    	    min = FLT_MAX; max = -FLT_MAX;
    	    sum = 0.;
    	    nPoints = 0;
    	    nShown = 0;
    	    errors = 0;
	    for (i=0; i<n; i++) {
		if (SliderNTRef(item, ntExt, pInfo->sliderVars, pInfo->nSliders,
			pInfo->sliderThresholds, i, pInfo->sliderGTorLT)) {
    		    nShown++;
    		    if (!ExtNTRef(item, ntExt, ntVars[var], i, &value)) {
    		        errors++;
    		    } else {
    			sum += value;
    			nPoints++;
    			if (value < min) min = value;
    			if (value > max) max = value;
    		    }
    		}
	    }

    	    /* if this is the first variable, report the number of points */
    	    if (firstVar && pInfo->nSliders != 0) {
    	    	sprintf(statString,"\n%d Shown", nShown);
    	    	statMString = XmStringCreateLtoR(statString,
    	    	    	XmSTRING_DEFAULT_CHARSET);
    	    	temp = XmStringConcat(accumString, statMString);
    	    	XmStringFree(accumString);
    	    	accumString = temp;
    	    	XmStringFree(statMString);
    	    	firstVar = False;
    	    }
    	    
    	    /* don't bother with rest of statistics if no points are shown */
    	    if (nShown == 0)
    	    	break;
    	    
    	    /* if variable is all bad, don't bother with statistics */
    	    if (nPoints == 0) {
    	    	sprintf(statString,
    	    		"\n%s, No Stats\n Variable not useable\n due to errors",
    	    		ExtNTVarName((hsNTuple *)item, ntVars[var]));
    	    	statMString = XmStringCreateLtoR(statString,
    	    	    	XmSTRING_DEFAULT_CHARSET);
    	    	temp = XmStringConcat(accumString, statMString);
    	    	XmStringFree(accumString);
    	    	accumString = temp;
    	    	XmStringFree(statMString);
    	    	continue;
    	    }    	    
	    
	    /* calculate the mean and standard deviation */
	    mean = sum/nPoints;
	    sumDev = 0.;
	    for (i=0; i<n; i++) {
    		if (SliderNTRef(item, ntExt, pInfo->sliderVars, pInfo->nSliders,
			pInfo->sliderThresholds, i, pInfo->sliderGTorLT)) {
		    if (ExtNTRef(item, ntExt, ntVars[var], i, &value))
		    	sumDev += pow(value - mean, 2.);
		}
	    }
	    stdDev = sqrt(sumDev/nPoints);

    	    /* append the statistics for this variable to the string */
    	    if (errors != 0) {
    		sprintf(statString,
    		"\n%s (Excl %d Errors)\n Mean %g\n Std Dev %g\n Min %g, Max %g",
    	    		ExtNTVarName((hsNTuple *)item, ntVars[var]), errors, 
    	    		mean, stdDev, min, max);
    	    } else {
    		sprintf(statString,
    			"\n%s\n Mean %g\n Std. Dev. %g\n Min %g, Max %g",
    	    		ExtNTVarName((hsNTuple *)item, ntVars[var]),
    	    		mean, stdDev, min, max);
    	    }
    	    statMString = XmStringCreateLtoR(statString,
    	    	    XmSTRING_DEFAULT_CHARSET);
    	    temp = XmStringConcat(accumString, statMString);
    	    XmStringFree(accumString);
    	    accumString = temp;
    	    XmStringFree(statMString);
    	}
    }
    return accumString;
}

static XmString create1DHistStatText(hs1DHist *item)
{
    char statString[512], hbookString[30];
    int nBins = item->nBins;
    float yMin = FLT_MAX, yMax = -FLT_MAX, min = item->min, max = item->max;
    double binWidth = (max-min)/nBins;
    double binValue, sum, sumDev, mean, stdDev, totalFills;
    int i;
    
    /* if item has an hbook id, include it in the string */
    if (item->hbookID != 0)
    	sprintf(hbookString, "HBOOK ID %d\n", item->hbookID);
    else
    	*hbookString = '\0';
    
    /* if the histogram is still empty, don't try to generate stats */
    if (item->bins == NULL) {
    	sprintf(statString,"%s\n%sWaiting for Data", item->title, hbookString);
    	return XmStringCreateLtoR(statString, XmSTRING_DEFAULT_CHARSET);
    }
    
    /* calculate y min and max, and sum */
    sum = 0.;
    totalFills = 0.;
    for (i=0; i<nBins; i++) {
    	binValue = item->bins[i];
    	if (binValue < yMin) yMin = binValue;
    	if (binValue > yMax) yMax = binValue;
    	totalFills += binValue;
    	sum += (min + binWidth*(i+.5)) * binValue;
    }
    
    /* calculate mean (using bin centers) and standard deviation */
    if (totalFills < FLT_MIN) {
    	mean = stdDev = 0.;
    	yMin = yMax = 0.;
    } else {
	mean = sum/totalFills;
	sumDev = 0.;
	for (i=0; i<nBins; i++)
    	    sumDev += pow((min+binWidth*(i+.5)) - mean, 2.) * item->bins[i];
	stdDev = sqrt(sumDev/totalFills);
    }
    
    /* generate the string presenting the statistics */
    sprintf(statString,"%s\n%s%d Fills\nSum %g\nMean %g\nStd. Dev. %g\n\
Overflow %g\nUnderflow %g\nX Min %g, Max %g\nY Min %g, Max %g",
	    item->title, hbookString, item->count, totalFills, mean, stdDev,
	    item->overflow, item->underflow, min, max, yMin, yMax);
    return XmStringCreateLtoR(statString, XmSTRING_DEFAULT_CHARSET);
}

static XmString create2DHistStatText(hs2DHist *item)
{
    char statString[512], hbookString[30];
    int nXBins = item->nXBins, nYBins = item->nYBins;
    float xMin = item->xMin, xMax = item->xMax;
    float yMin = item->yMin, yMax = item->yMax;
    float zMin = FLT_MAX, zMax = -FLT_MAX;
    double xBinWidth = (xMax-xMin)/nXBins;
    double yBinWidth = (yMax-yMin)/nYBins;
    double xSum, xSumDev, xMean, xStdDev, ySum, ySumDev, yMean, yStdDev;
    double binCenter, binValue, totalFills;
    int i, j;
    
    /* if item has an hbook id, include it in the string */
    if (item->hbookID != 0)
    	sprintf(hbookString, "HBOOK ID %d\n", item->hbookID);
    else
    	*hbookString = '\0';
    
    /* if the histogram is still empty, don't try to generate stats */
    if (item->bins == NULL) {
    	sprintf(statString,"%s\n%sWaiting for Data", item->title, hbookString);
    	return XmStringCreateLtoR(statString, XmSTRING_DEFAULT_CHARSET);
    }
    
    /* calculate the x and y sums using bin center values */
    xSum = 0.;
    totalFills = 0.;
    for (i=0; i<nXBins; i++) {
    	binCenter = xMin + xBinWidth*(i+.5);
    	for (j=0; j<nYBins; j++) {
    	    binValue = item->bins[i*nYBins + j];
    	    if (binValue < zMin) zMin = binValue;
    	    if (binValue > zMax) zMax = binValue;
    	    totalFills += binValue;
    	    xSum += binCenter * binValue;
    	}
    }
    ySum = 0.;
    for (i=0; i<nYBins; i++) {
    	binCenter = yMin + yBinWidth*(i+.5);
    	for (j=0; j<nXBins; j++) {
    	    binValue = item->bins[j*nYBins + i];
    	    ySum += binCenter * binValue;
    	}
    }

    
    /* calculate means and standard deviations along X and y axes */
    if (totalFills < FLT_MIN) {
    	xMean = yMean = xStdDev = yStdDev = 0.;
    	zMin = zMax = 0.;
    } else {
	xMean = xSum/totalFills;
	yMean = ySum/totalFills;
	xSumDev = 0.;
	for (i=0; i<nXBins; i++) {
    	    binCenter = xMin + xBinWidth*(i+.5);
    	    for (j=0; j<nYBins; j++) {
    		xSumDev += pow(binCenter - xMean, 2.) * item->bins[i*nYBins+j];
    	    }
	}
	xStdDev = sqrt(xSumDev/totalFills);
	ySumDev = 0.;
	for (i=0; i<nYBins; i++) {
    	    binCenter = yMin + yBinWidth*(i+.5);
    	    for (j=0; j<nXBins; j++) {
    		ySumDev += pow(binCenter - yMean, 2.) * item->bins[j*nYBins+i];
    	    }
	}
	yStdDev = sqrt(ySumDev/totalFills);
    }
    
    /* generate the string presenting the statistics */
    sprintf(statString, "%s\n%s%d Fills\nSum %g\nX Mean %g\nY Mean %g\n\
X Std. Dev. %g\nY Std. Dev. %g\nOverflows\n -X +Y %g\n -X %g\n\
 -X -Y %g\n +Y %g\n -Y %g\n +X +Y %g\n +X %g\n +X -Y %g\nX Min %g, Max %g\n\
Y Min %g, Max %g\nZ Min %g, Max %g", item->title, hbookString, item->count,
	    totalFills, xMean, yMean, xStdDev, yStdDev,
	    item->overflow[0][0], item->overflow[0][1], item->overflow[0][2],
	    item->overflow[1][0], 			item->overflow[1][2],
	    item->overflow[2][0], item->overflow[2][1], item->overflow[2][2],
    	    xMin, xMax, yMin, yMax, zMin, zMax);
    return XmStringCreateLtoR(statString, XmSTRING_DEFAULT_CHARSET);
}

/*
** sliderToBinLimit and binLimitToSlider implement a non-linear scale for
** the bin limit slider so users can select reasonable values ranging
** from 1 to 1,000,000.  The actual slider values range from 1 to 370
*/
static int sliderToBinLimit(int sliderValue)
{
    if (sliderValue <= 100)
    	return sliderValue;
    if (sliderValue <= 190)
    	return (sliderValue - 100) * 10 + 100;
    if (sliderValue <= 280)
    	return (sliderValue - 190) * 100 + 1000;
    return (sliderValue - 280) * 1000 + 10000;
}
static int binLimitToSlider(int limit)
{
    if (limit <= 100)
    	return limit;
    if (limit <= 1000)
    	return 100 + limit/10 - 10;
    if (limit <= 10000)
    	return 190 + limit/100 - 10;
    return 280 + limit/1000 - 10;
}

static void updateSliderValueLabel(windowInfo *wInfo, int sliderNum)
{
    Widget label = wInfo->sliderLabels[sliderNum];
    int gtOrLT = wInfo->pInfo[0]->sliderGTorLT[sliderNum];
    float value = wInfo->pInfo[0]->sliderThresholds[sliderNum];
    float sliderMin = wInfo->sliderMin[sliderNum];
    float sliderMax = wInfo->sliderMax[sliderNum];

    if ((value >= sliderMax && gtOrLT == SLIDER_LT) ||
    	    (value <= sliderMin && gtOrLT == SLIDER_GT))
    	XmTextSetString(label, "All");
    else if ((value <= sliderMin && gtOrLT == SLIDER_LT) ||
    	    (value >= sliderMax && gtOrLT == SLIDER_GT))
    	XmTextSetString(label, "None");
    else
    	SetFloatText(label, value);
}

static void updateSliderScalePosition(windowInfo *wInfo, int sliderNum)
{
    int sliderValue;
    double thresh = wInfo->pInfo[0]->sliderThresholds[sliderNum];
    
    sliderValue = SLIDER_MAX * (thresh - wInfo->sliderMin[sliderNum]) /
    	    (wInfo->sliderMax[sliderNum] - wInfo->sliderMin[sliderNum]);
    if (sliderValue > SLIDER_MAX) sliderValue = SLIDER_MAX;
    if (sliderValue < 0) sliderValue = 0;
    XtVaSetValues(wInfo->sliderScales[sliderNum], XmNvalue, sliderValue, NULL);
}

void UpdateCellNormValueLabels(windowInfo *window)
{
    if (window->cellNormMin == -FLT_MAX)
	XmTextSetString(window->cellNormLowLabel, "Min");
    else
	SetFloatText(window->cellNormLowLabel, window->cellNormMin);
    if (window->cellNormMax == FLT_MAX)
	XmTextSetString(window->cellNormUpLabel, "Max");
    else
	SetFloatText(window->cellNormUpLabel, window->cellNormMax);
}

/*
** Returns true if ntuple element should be displayed.  Returns false if
** a slider is set below (<) or above (>) the value of the variable 
** that it is animating (or there is an interpreter error in evaluating
** any of the slider variables.
*/
int SliderNTRef(hsNTuple *ntuple, ntupleExtension *ntExt, int *sliders,
	int nSliders, float *thresholds, int index, int *sliderGTorLT)
{
    int i;
    float value;
    
    /* Note: Keep this routine efficient, it is called for every element
       of an ntuple at every refresh. */
    for (i=0; i<nSliders; i++) {
    	if (!ExtNTRef(ntuple, ntExt, sliders[i], index, &value))
    	    return False;
    	if (sliderGTorLT[i] == SLIDER_LT && value > thresholds[i])
    	    return False;
    	else if (sliderGTorLT[i] == SLIDER_GT && value < thresholds[i])
    	    return False;
    }
    return True;
}
