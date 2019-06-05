/*******************************************************************************
*									       *
* preferences.h -- Dialogs for HistoScope preferences menu		       *
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
* Nov 10, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

/* note that the preferences for graphics buffering and update frequency are
   actually maintained elsewhere.  Use Set(Get)GraphicsBuffering and Set(Get)
   UpdateFrequency to set or get these preferences.  The remaining preferences
   can reliably be set and read from this structure */
struct prefData {
    int bufferGraphics;		/* turn on graphics buffering initially */
    int updateFreq;		/* initial setting for update frequency */
    int mapDelete;		/* whether to map delete to backspace */
    int plotAutoHelp;		/* automatic help for plot controls on/off */
};
extern struct prefData PrefData;

void ShowUpdateFreqPanel(Widget parent);
void SaveHistoPreferences(Widget parent, char *prefFileName);
XrmDatabase CreateHistoPrefDB(char *prefFileName, char *appName,
	unsigned int *argcInOut, char **argvInOut);
void RestoreHistoPrefs(XrmDatabase prefDB, XrmDatabase appDB, char *appName,
	char *appClass);
