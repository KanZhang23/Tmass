/*******************************************************************************
*									       *
* preferences.c -- Dialogs for HistoScope preferences menu		       *
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
* Nov 10, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/DialogS.h>
#include <Xm/SelectioB.h>
#include "../util/misc.h"
#include "../util/prefFile.h"
#include "../util/DialogF.h"
#include "histoP.h"
#include "../histo_util/hsTypes.h"
#include "communications.h"
#include "preferences.h"
#include "plotWindows.h"

static void createUpdateFreqPanel(Widget parent);
static void freqSliderCB(Widget w, caddr_t clientData, caddr_t callData);
static void freqCloseCB(Widget w, caddr_t clientData, caddr_t callData);
static void freqApplyCB(Widget w, caddr_t clientData, caddr_t callData);
static void freqOkCB(Widget w, caddr_t clientData, caddr_t callData);
static void freqCancelCB(Widget w, caddr_t clientData, caddr_t callData);
static void setFreqPanelValue(int freq);
static void setFreqLabel(int freq);

struct prefData PrefData;

static PrefDescripRec PrefDescrip[] = {
    {"bufferGraphics", "BufferGraphics", PREF_BOOLEAN, "False",
    	&PrefData.bufferGraphics, NULL, True},
    {"updateFrequency", "UpdateFrequency", PREF_INT, "3000",
    	&PrefData.updateFreq, NULL, True},
    {"autoPlotHelp", "AutoPlotHelp", PREF_BOOLEAN, "True",
    	&PrefData.plotAutoHelp, NULL, True},
    {"remapDeleteKey", "RemapDeleteKey", PREF_BOOLEAN, "True",
    	&PrefData.mapDelete, NULL, False},
};

static XrmOptionDescRec OpTable[] = {
    {"-buffer", ".bufferGraphics", XrmoptionNoArg, (caddr_t)"True"},
    {"-nobuffer", ".bufferGraphics", XrmoptionNoArg, (caddr_t)"False"},
    {"-freq", ".updateFrequency", XrmoptionSepArg, (caddr_t)NULL},
};

static char HeaderText[] = "\
# Preferences file for HistoScope or NPlot\n\
#\n\
# This file is overwritten by the \"Save Preferences...\" command and serves\n\
# only the interactively setable options presented in the HistoScope or NPlot\n\
# \"Preferences\" menu.  To modify other options, such as background colors\n\
# and key bindings, use the .Xdefaults file in your home directory (or\n\
# the X resource specification method appropriate to your system).  The\n\
# contents of this file can be moved into an X resource file, but since\n\
# resources in this file override their corresponding X resources, either\n\
# this file should be deleted or individual resource lines in the file\n\
# should be deleted for the moved lines to take effect.\n";

#define N_FREQS 21
static int Freqs[N_FREQS] = {
   0, 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
   15000, 20000, 30000, 45000, 60000, 120000, 300000, 600000
};
static Widget UpdateFreqPanel = NULL;
static Widget UpdateFreqLabel;
static Widget UpdateFreqSlider;
static int CurrentSliderFreq;		/* for ok */
static int OldSliderFreq;		/* for cancel */

void ShowUpdateFreqPanel(Widget parent)
{
    int updateFreq;

    if (UpdateFreqPanel == NULL)
    	createUpdateFreqPanel(parent);
    updateFreq = GetUpdateFreq();
    setFreqPanelValue(updateFreq);
    CurrentSliderFreq = updateFreq;
    OldSliderFreq = updateFreq;
    if (!XtIsManaged(UpdateFreqPanel))
    	XtManageChild(UpdateFreqPanel);
    else
    	XRaiseWindow(XtDisplay(UpdateFreqPanel),
    		     XtWindow(XtParent(UpdateFreqPanel)));
}  

void SaveHistoPreferences(Widget parent, char *prefFileName)
{
    /* slimy hack to suppress histo specific preferences in .nplot file */
    if (!strcmp(prefFileName, ".nplot") || !strcmp(prefFileName, ".NPLOT"))
    	PrefDescrip[1].save = False;
    	
    /* fill in preference data which is maintained elsewhere */
    PrefData.updateFreq = GetUpdateFreq();
    PrefData.bufferGraphics = GetGraphicsBuffering();
    
    /* save the file and tell the user whether the save succeeded or not */
    if (SavePreferences(XtDisplay(parent), prefFileName, HeaderText,
    	    PrefDescrip, XtNumber(PrefDescrip)))
    	DialogF(DF_INF, parent, 1, 
#ifdef VMS
		"Default preferences saved in SYS$LOGIN:%s\n\
The program automatically loads this file\neach time it is started.", "OK",
#else
    		"Default preferences saved in $HOME/%s\n\
The program automatically loads this file\neach time it is started.", "OK",
#endif /*VMS*/
		prefFileName);
    else
    	DialogF(DF_WARN, parent, 1,
#ifdef VMS
    		"Unable to save preferences in SYS$LOGIN:%s", "OK",
#else
    		"Unable to save preferences in $HOME/%s", "OK",
#endif /*VMS*/
		 prefFileName);
}

XrmDatabase CreateHistoPrefDB(char *prefFileName, char *appName,
	unsigned int *argcInOut, char **argvInOut)
{
    return CreatePreferencesDatabase(prefFileName, appName, 
	    OpTable, XtNumber(OpTable), argcInOut, argvInOut);
}

void RestoreHistoPrefs(XrmDatabase prefDB, XrmDatabase appDB, char *appName,
	char *appClass)
{
    RestorePreferences(prefDB, appDB, appName,
    	    appClass, PrefDescrip, XtNumber(PrefDescrip));
    BufferGraphics(PrefData.bufferGraphics);
    SetUpdateFreq(PrefData.updateFreq);
    SetDeleteRemap(PrefData.mapDelete);
}

static void createUpdateFreqPanel(Widget parent)
{
    Widget selBox, form, scale, value;
    XmString s1;

    selBox = XmCreatePromptDialog(parent, "updateFreqBox", NULL, 0);
    XtAddCallback(selBox, XmNapplyCallback,
                 (XtCallbackProc)  freqApplyCB, NULL);
    XtAddCallback(selBox, XmNokCallback,
                  (XtCallbackProc)  freqOkCB, NULL);
    XtAddCallback(selBox, XmNcancelCallback,
                  (XtCallbackProc)  freqCancelCB, NULL);
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_TEXT));
    XtManageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_APPLY_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_SELECTION_LABEL));
    XtVaSetValues(XtParent(selBox), XmNtitle, "Update Frequency", NULL);
    AddMotifCloseCallback(XtParent(selBox), (XtCallbackProc)freqCloseCB, NULL);

    /* create the slider and labels */
    form = XtVaCreateManagedWidget("rebin", xmFormWidgetClass, selBox, NULL);
    scale = XtVaCreateManagedWidget("scale", xmScaleWidgetClass, form,
    	    XmNorientation, XmHORIZONTAL,
    	    XmNminimum, 0,
    	    XmNmaximum, N_FREQS-1,
    	    XmNscaleMultiple, 2,
    	    XmNwidth, 200,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 99,
    	    XmNtopOffset, 3, NULL);
    XtAddCallback(scale, XmNdragCallback, (XtCallbackProc)freqSliderCB, NULL);
    XtAddCallback(scale, XmNvalueChangedCallback,
    	    (XtCallbackProc)freqSliderCB, NULL);
    XtVaCreateManagedWidget("name", xmLabelWidgetClass, form, 
    	    XmNlabelString, s1=XmStringCreateSimple("Update Frequency"),
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 1,
    	    XmNrightPosition, 60,
    	    XmNtopWidget, scale, NULL);
    XmStringFree(s1);
    value = XtVaCreateManagedWidget("value", xmLabelWidgetClass, form, 
    	    XmNlabelString,s1=XmStringCreateSimple("45 sec."),
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNrecomputeSize, False,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftPosition, 61,
    	    XmNrightPosition, 99,
    	    XmNtopWidget, scale, NULL);
    XmStringFree(s1);
    UpdateFreqLabel = value;
    UpdateFreqSlider = scale;
    UpdateFreqPanel = selBox;
}

static void freqSliderCB(Widget w, caddr_t clientData, caddr_t callData)
{
    int sliderValue;
    int freq;
    
    /* get the slider position */
    XtVaGetValues(w, XmNvalue, &sliderValue, NULL);
    
    /* convert the slider position to a frequency in milliseconds & set label */
    freq = Freqs[sliderValue];
    if (freq != CurrentSliderFreq) {
	setFreqLabel(freq);
	CurrentSliderFreq = freq;
    }
}

static void freqCloseCB(Widget w, caddr_t clientData, caddr_t callData)
{
    freqCancelCB(w, clientData, callData);
}

static void freqApplyCB(Widget w, caddr_t clientData, caddr_t callData)
{
    SetUpdateFreq(CurrentSliderFreq);
}

static void freqOkCB(Widget w, caddr_t clientData, caddr_t callData)
{
    SetUpdateFreq(CurrentSliderFreq);
    XtUnmanageChild(UpdateFreqPanel);
}

static void freqCancelCB(Widget w, caddr_t clientData, caddr_t callData)
{
    SetUpdateFreq(OldSliderFreq);
    XtUnmanageChild(UpdateFreqPanel);
}

static void setFreqPanelValue(int freq)
{
    int i;
    
    for (i=0; i<N_FREQS; i++) {
    	if (Freqs[i] >= freq) {
    	    XtVaSetValues(UpdateFreqSlider, XmNvalue, i, NULL);
    	    break;
    	}
    }
    setFreqLabel(freq);
}

static void setFreqLabel(int freq)
{
    char *units, labelString[25];
    XmString s1;
    
    if (freq == 0) {
    	sprintf(labelString, "Continuous");
    } else {
	if (freq >= 60000) {
    	    units = "min.";
    	    freq /= 60000;
	} else if (freq >= 1000) {
    	    units = "sec.";
    	    freq /= 1000;
    	} else
    	    units = "msec.";
	sprintf(labelString, "%d %s", freq, units);
    }
    s1=XmStringCreateSimple(labelString);
    XtVaSetValues(UpdateFreqLabel, XmNlabelString, s1, NULL);
    XmStringFree(s1);
}
