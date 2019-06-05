/*******************************************************************************
*									       *
* controls.c -- Non-plot histoscope item types: indicators, controls, and      *
*		and triggers						       *
*									       *
* Copyright (c) 1991, 1993 Universities Research Association, Inc.	       *
* All rights reserved.		c					       *
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
* June, 1993								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Modifications:							       *
*									       *
* 	JMK - Nov 3, 1993 - Added underflow/overflow of indicator values       *
*									       *
*******************************************************************************/
#include <Xm/Xm.h>
#include <Xm/ScrollBar.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include "../util/help.h"
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../histo_util/hsTypes.h"
#include "histoP.h"
#include "plotWindows.h"
#include "plotMenus.h"
#include "mainPanel.h"
#include "communications.h"
#include "controls.h"
#include "help.h"

#define SLIDER_MAX 10000
#define SLIDER_WIDTH 650

static void redisplayIndicator(windowInfo *window, int mode);
static void redisplayControl(windowInfo *window, int mode);
static void redisplayTrigger(windowInfo *window, int mode);
static void setBtnCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void sliderCB(Widget w, windowInfo *wInfo, caddr_t callData);
static void triggerBtnCB(Widget w, windowInfo *wInfo, caddr_t callData);

/*
** CreateIndicatorWindow, CreateTriggerWindow, CreateControlWindow
**
** Creation routines for indicators, triggers, and controls.  This module
** has the same structure as PlotWindows.c: each item type has a creation
** routine and a redisplay routine.  The redisplay routine is attached
** to the plot-window structure so the item can be refreshed without
** knowlege of the details of the particular item type.
*/
windowInfo *CreateIndicatorWindow(Widget parent, hsGeneral *item,
	int *ntVars, int *sliderVars, char *title, Widget shell)
{
    Widget scale, form, frame, minLabel, maxLabel, valueLabel;
    windowInfo *wInfo;
    XmString s1;
    Pixel foreground, background;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xmFormWidgetClass,
    	    redisplayIndicator, title, NULL, shell, False, -1, NULL);

    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  IndicatorHelpCB, NULL);

    /* indicators are different from the other plotting windows, which simply
       enclose plotting widgets.  Above we told createGenericWindow to create
       a window with a Motif form as the plotting widget, below we fill in
       other widgets as children of the form to do the actual drawing.
       Background and foreground colors are manually inherited so the
       color specification in the fallback resources can be overridden
       without requiring the user to specify each component individually */
    form = wInfo->widget;
    XtVaGetValues(form, XmNforeground,&foreground, XmNbackground,&background, NULL);
    XtVaSetValues(wInfo->shell, XmNallowShellResize, True, NULL);
 
    frame = XtVaCreateManagedWidget("valueFrame", xmFrameWidgetClass, form,
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 34,
    		XmNrightPosition, 66,
    		XmNbottomPosition, 94, NULL);
    valueLabel = XtVaCreateManagedWidget("valueLabel", xmLabelWidgetClass,frame, 
    		XmNlabelString, s1=XmStringCreateSimple("-1.234567e-89"),
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, NULL);
    XmStringFree(s1);
    scale = XtVaCreateManagedWidget("sb", xmScrollBarWidgetClass, form,
    		XmNorientation, XmHORIZONTAL,
    		XmNsliderSize, 1,
    		XmNshowArrows, False,
    		XmNsensitive, False,
    		XmNminimum, 0,
    		XmNmaximum, SLIDER_MAX,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_WIDGET,
    		XmNleftPosition, 1,
    		XmNrightPosition, 99,
    		XmNtopPosition, 8,
    		XmNbottomOffset, 2,
    		XmNbottomWidget, frame, NULL);
    minLabel = XtVaCreateManagedWidget("minLabel", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("-1.234567e-89"),
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNalignment, XmALIGNMENT_BEGINNING,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNleftPosition, 1,
    		XmNrightPosition, 34,
    		XmNtopWidget, frame, NULL);
    XmStringFree(s1);
    maxLabel = XtVaCreateManagedWidget("maxLabel", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("-1.234567e-89"),
    		XmNrecomputeSize, False,
    		XmNalignment, XmALIGNMENT_END,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNleftPosition, 65,
    		XmNrightPosition, 99,
    		XmNtopWidget, frame, NULL);
    XmStringFree(s1);
    
    CreateIndicatorMenu(wInfo);
    
    wInfo->widget = valueLabel;	/* note: this replaces the form */
    wInfo->scale = scale;
    wInfo->minLabel = minLabel;
    wInfo->maxLabel = maxLabel;
    wInfo->setValue = NULL;

    /* Set the current values for the slider, value text, and limits */
    redisplayIndicator(wInfo, REINIT);

    /* Show the widget */
    XtRealizeWidget(wInfo->shell);
    
    return wInfo;
}

windowInfo *CreateTriggerWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, Widget shell)
{
    windowInfo *wInfo;
    XmString s1;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL,
    	    xmPushButtonWidgetClass, redisplayTrigger, title, NULL,
            shell, False, -1, NULL);

    /* to establish the size of the window, give the label an initial
       string as long as the longest it will ever hold */
    XtVaSetValues(wInfo->widget, XmNlabelString,
    	    s1=XmStringCreateSimple("999 Pending"), NULL);
    XmStringFree(s1);

    /* add the button and help callbacks */
    XtAddCallback(wInfo->widget, XmNactivateCallback,
    	    (XtCallbackProc)triggerBtnCB, (char *)wInfo);
    /* XtAddCallback(wInfo->widget, XmNhelpCallback, TriggerHelpCB, NULL); */

    /* create the plot menu */
    CreateTriggerMenu(wInfo);

    /* start the count of pending trigger operations at 0 */
    wInfo->triggerCount = 0;

    /* display the widget */
    XtRealizeWidget(wInfo->shell);
    
    /* display the correct initial button label */
    redisplayTrigger(wInfo, REINIT);
    
    return wInfo;
}

windowInfo *CreateControlWindow(Widget parent, hsGeneral *item, int *ntVars,
	int *sliderVars, char *title, Widget shell)
{
    Widget scale, form, frame, minLabel, maxLabel, valueLabel;
    Widget slider, valueSet, setBtn;
    windowInfo *wInfo;
    XmString s1;
    Pixel foreground, background;

    /* create the window */
    wInfo = CreateGenericWindow(parent, item, ntVars, NULL, xmFormWidgetClass,
    	    redisplayControl, title, NULL, shell, False, -1, NULL);

    /* Add the help callback */
    XtAddCallback(wInfo->widget, XmNhelpCallback,
                  (XtCallbackProc)  ControlHelpCB, NULL);

    /* controls, like indicators are different from the other plotting windows,
       which simply enclose plotting widgets.  Above we told createGenericWindow
       to create a window with a Motif form as the plotting widget, below we
       fill in other widgets as children of the form to do the actual drawing.
       Background and foreground colors are manually inherited so the
       color specification in the fallback resources can be overridden
       without requiring the user to specify each component individually */
    form = wInfo->widget;
    XtVaGetValues(form, XmNforeground,&foreground, XmNbackground,&background,NULL);
    XtVaSetValues(wInfo->shell, XmNallowShellResize, True, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
 
    /* create the remainder of the contents */
    frame = XtVaCreateManagedWidget("valueFrame", xmFrameWidgetClass, form,
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, 8,
    		XmNleftPosition, 66,
    		XmNrightPosition, 99, NULL);
    valueLabel = XtVaCreateManagedWidget("valueLabel", xmLabelWidgetClass,frame, 
    		XmNlabelString, s1=XmStringCreateSimple("-1.234567e-89"),
    		XmNalignment, XmALIGNMENT_BEGINNING,
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, NULL);
    XmStringFree(s1);
    valueSet = XtVaCreateManagedWidget("valueSet", xmTextWidgetClass, form,
    		XmNcolumns, 13,
    		XmNmaxLength, 20,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, frame,
    		XmNleftPosition, 66,
    		XmNrightPosition, 99, NULL);
    RemapDeleteKey(valueSet);
    setBtn = XtVaCreateManagedWidget("valueFrame", xmPushButtonWidgetClass,form,
    		XmNlabelString, s1=XmStringCreateSimple("Set"),
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNbottomAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 26,
    		XmNrightPosition, 40,
    		XmNbottomPosition, 97, NULL);
    XmStringFree(s1);
    XtAddCallback(setBtn, XmNactivateCallback, (XtCallbackProc)setBtnCB, 
    		(char *)wInfo);
    scale = XtVaCreateManagedWidget("sb", xmScrollBarWidgetClass, form,
    		XmNorientation, XmHORIZONTAL,
    		XmNsliderSize, SLIDER_WIDTH,
    		XmNheight, 8,
    		XmNshowArrows, False,
    		XmNsensitive, False,
    		XmNminimum, 0,
    		XmNmaximum, SLIDER_MAX,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 1,
    		XmNrightPosition, 65,
    		XmNtopPosition, 8,
    		XmNbottomOffset, 2, NULL);
    slider = XtVaCreateManagedWidget("sl", xmScrollBarWidgetClass, form,
    		XmNorientation, XmHORIZONTAL,
    		XmNsliderSize, SLIDER_WIDTH,
    		XmNshowArrows, False,
    		XmNminimum, 0,
    		XmNmaximum, SLIDER_MAX,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNbottomAttachment, XmATTACH_WIDGET,
    		XmNleftPosition, 1,
    		XmNrightPosition, 65,
    		XmNtopWidget, scale,
    		XmNbottomOffset, 2,
    		XmNbottomWidget, setBtn, NULL);
    XtAddCallback(slider, XmNdragCallback, (XtCallbackProc)sliderCB,
    	    	(char *)wInfo);
    minLabel = XtVaCreateManagedWidget("minLabel", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("-1.234e-89"),
    		XmNrecomputeSize, False,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNalignment, XmALIGNMENT_BEGINNING,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNleftPosition, 1,
    		XmNrightPosition, 30,
    		XmNtopWidget, setBtn, NULL);
    XmStringFree(s1);
    maxLabel = XtVaCreateManagedWidget("maxLabel", xmLabelWidgetClass, form, 
    		XmNlabelString, s1=XmStringCreateSimple("-1.234e-89"),
    		XmNrecomputeSize, False,
    		XmNalignment, XmALIGNMENT_END,
    		XmNforeground, foreground,
    		XmNbackground, background, 
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNleftPosition, 36,
    		XmNrightPosition, 65,
    		XmNtopWidget, setBtn, NULL);
    XmStringFree(s1);
    XtVaSetValues(form, XmNdefaultButton, setBtn, NULL);
    XtVaSetValues(setBtn, XmNshowAsDefault, False,
    	XmNdefaultButtonShadowThickness, 0, False, NULL);
    
    /* add the (small) background plot menu */
    CreateControlMenu(wInfo);
    
    /* save the widget id's of widgets which must be accessed later */
    wInfo->widget = valueLabel;	/* note: this replaces the form */
    wInfo->scale = scale;
    wInfo->slider = slider;
    wInfo->minLabel = minLabel;
    wInfo->maxLabel = maxLabel;
    wInfo->setValue = valueSet;
    wInfo->setBtn = setBtn;

    /* Set the current values for the slider, value text, and limits */
    redisplayControl(wInfo, REINIT);

    /* Showtime */
    XtRealizeWidget(wInfo->shell);
    
    return wInfo;
}

/*
** Set controls and triggers insensitive (dimmed)
*/ 
void SetControlInsensitive(hsGeneral *item)
{
    windowInfo *wInfo;
    
    wInfo = FirstDisplayedItem(item->id);
    if (wInfo == NULL)
    	return;
    if (item->type == HS_CONTROL) {
    	XtVaSetValues(wInfo->setBtn, XmNsensitive, False, NULL);
        XtVaSetValues(wInfo->slider, XmNsensitive, False, NULL);
        XtVaSetValues(wInfo->widget, XmNsensitive, False, NULL);
    } else if (item->type == HS_TRIGGER) {
    	XtVaSetValues(wInfo->widget, XmNsensitive, False, NULL);
    }
}

/*
** Process the acknowledgement for a set-trigger request by updating
** the pending-requests count displayed on the button.
*/ 
void AcknowledgeTrigger(int id)
{
    windowInfo *wInfo;
    	
    /* find the window with the trigger (there should be only one) */
    wInfo = FirstDisplayedItem(id);
    if (wInfo == NULL)
    	return;
    	
    /* decrement the count of pending trigger requests and redisplay
       the trigger button to reflect the new count */
    wInfo->triggerCount--;
    if (wInfo->triggerCount < 0)
	wInfo->triggerCount = 0;
    redisplayTrigger(wInfo, REINIT);
}    

/*
** Reset a trigger, clearing the pending request count.
*/ 
void ResetTrigger(hsTrigger *trigger)
{
    windowInfo *wInfo;
    
    if (trigger->type != HS_TRIGGER)
    	return;
    	
    wInfo = FirstDisplayedItem(trigger->id);
    if (wInfo == NULL)
    	return;
    	
    /* set the count of pending trigger operations to 0  and re-display */
    wInfo->triggerCount = 0;
    redisplayTrigger(wInfo, REINIT);
}

/*
** Switch indicators and controls between their fully expanded states
** with range and graphical indicators shown, and their compact states with
** only numerical values shown.
*/
void ShowRange(windowInfo *wInfo, int rangeShown)
{
    Widget wList[4];
    
    if (wInfo->setValue == NULL) {
	
	/* Window is an indicator */
	wList[0]=wInfo->minLabel; wList[1]=wInfo->maxLabel;
	wList[2]=wInfo->scale;
	if (rangeShown) {
    	    XtVaSetValues(XtParent(wInfo->widget),
    		    XmNleftPosition, 34, XmNrightPosition, 66, NULL);
    	    XtManageChildren(wList, 3);
	} else {
    	    XtUnmanageChildren(wList, 3);
    	    XtVaSetValues(XtParent(wInfo->widget),
    		    XmNleftPosition, 1, XmNrightPosition, 99, NULL);
	}
    } else {
    	
    	/* Window is a control */
    	wList[0]=wInfo->minLabel; wList[1]=wInfo->maxLabel;
	wList[2]=wInfo->scale; wList[3]=wInfo->slider;
	if (rangeShown) {
    	    XtVaSetValues(XtParent(wInfo->widget),
    		    XmNleftPosition, 66, XmNrightPosition, 99, NULL);
    	    XtVaSetValues(wInfo->setBtn,
    		    XmNleftPosition, 25, XmNrightPosition, 40, NULL);
    	    XtVaSetValues(wInfo->setValue,
    		    XmNleftPosition, 66, XmNrightPosition, 99, NULL);
    	    XtManageChildren(wList, 4);
	} else {
    	    XtUnmanageChild(wInfo->setBtn);
    	    XtUnmanageChildren(wList, 4);
    	    XtVaSetValues(wInfo->setBtn,
    		    XmNleftPosition, 1, XmNrightPosition, 25, NULL);
    	    XtVaSetValues(XtParent(wInfo->widget),
    		    XmNleftPosition, 2, XmNrightPosition, 98, NULL);
    	    XtVaSetValues(wInfo->setValue,
    		    XmNleftPosition, 26, XmNrightPosition, 99, NULL);
    	    XtManageChild(wInfo->setBtn);
	}
    }
}

static void redisplayIndicator(windowInfo *window, int mode)
{
    hsIndicator *ind = (hsIndicator *)GetMPItemByID(window->pInfo[0]->id); 
    XmString s1;
    
    switch (ind->valueSet) {
    	case VALUE_UNDERFLOW:
	    s1=XmStringCreateSimple("* underflow *");
	    SET_ONE_RSRC(window->widget, XmNlabelString, s1)
	    XmStringFree(s1);
	    SetFloatLabel(window->widget, ind->value);
	    SET_ONE_RSRC(window->scale, XmNvalue, 0);
	    break;
    	case VALUE_OVERFLOW:
	    s1=XmStringCreateSimple("* overflow *");
	    SET_ONE_RSRC(window->widget, XmNlabelString, s1)
	    XmStringFree(s1);
	    SetFloatLabel(window->widget, ind->value);
	    SET_ONE_RSRC(window->scale, XmNvalue, (int)(SLIDER_MAX));
	    break;
    	case VALUE_SET:
	    SetFloatLabel(window->widget, ind->value);
	    SET_ONE_RSRC(window->scale, XmNvalue, (int)(SLIDER_MAX * 
    		(ind->value - ind->min)/(ind->max - ind->min) - .5));
	    break;
    	case VALUE_NOT_SET:
	    s1=XmStringCreateSimple("* not set *");
	    SET_ONE_RSRC(window->widget, XmNlabelString, s1)
	    XmStringFree(s1);
    	    SET_ONE_RSRC(window->scale, XmNvalue, SLIDER_MAX/2); /* middle */
	    break;
	default:
	    printf("Internal Error: indicator->valueSet incorrect\n");
    }
    if (mode == REINIT) {
	SetFloatLabel(window->minLabel, ind->min);
	SetFloatLabel(window->maxLabel, ind->max);
    }
}

static void redisplayControl(windowInfo *window, int mode)
{
    hsControl *ctrl = (hsControl *)GetMPItemByID(window->pInfo[0]->id);
    int sliderValue;
    
    SetFloatLabel(window->widget, ctrl->value);
    sliderValue = (int)((SLIDER_MAX - SLIDER_WIDTH) *
    	    (ctrl->value - ctrl->min)/(ctrl->max - ctrl->min) - .5);
    XtVaSetValues(window->scale, XmNvalue, sliderValue, NULL);
    if (mode == REINIT) {
	SetFloatLabel(window->minLabel, ctrl->min);
	SetFloatLabel(window->maxLabel, ctrl->max);
	XtVaSetValues(window->slider, XmNvalue, sliderValue, NULL);
	SetFloatText(window->setValue, ctrl->value);
    }
}

static void redisplayTrigger(windowInfo *window, int mode)
{
    char labelString[20];
    XmString s1;
    
    /* Reset the trigger button label (all modes) to reflect the current
       count of pending operations */
    if (window->triggerCount == 0)
	sprintf(labelString, " ");
    else if (window->triggerCount == 1)
	sprintf(labelString, "Pending");
    else
	sprintf(labelString, "%d Pending", window->triggerCount);
    XtVaSetValues(window->widget, XmNlabelString,
	    s1=XmStringCreateSimple(labelString), NULL);
    XmStringFree(s1);
}

static void setBtnCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    double value;
    float min, max;
    hsControl *ctrl;
    
    if (GetFloatTextWarn(wInfo->setValue, &value, "new setting", True))
    	return;
    ctrl = (hsControl *)GetMPItemByID(wInfo->pInfo[0]->id);
    min = ctrl->min;
    max = ctrl->max;
    if (value <= max && value >= min)
    	SetControl(wInfo->pInfo[0]->id, value);
    else
    	DialogF(DF_ERR, w, 1, "Value is not within range", "Acknowledged");
}

static void sliderCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    double value, min, max;
    int sliderValue;
    hsControl *ctrl;
    
    ctrl = (hsControl *)GetMPItemByID(wInfo->pInfo[0]->id);
    min = ctrl->min;
    max = ctrl->max;
    XtVaGetValues(wInfo->slider, XmNvalue, &sliderValue, NULL);
    value = sliderValue * (max - min)/(SLIDER_MAX - SLIDER_WIDTH) + min;
    SetFloatText(wInfo->setValue, value);
    	return;
}

static void triggerBtnCB(Widget w, windowInfo *wInfo, caddr_t callData)
{
    wInfo->triggerCount++;
    redisplayTrigger(wInfo, REINIT);
    SetTrigger(wInfo->pInfo[0]->id);
}
