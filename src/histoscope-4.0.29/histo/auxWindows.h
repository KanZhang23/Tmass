/*******************************************************************************
*									       *
* auxWindows.h -- Slider and statistics windows for plots		       *
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
*******************************************************************************/
Widget CreateSliderWindow(windowInfo *wInfo, char *geometry);
Widget CreateRebinWindow(windowInfo *wInfo, int nDim, char *geometry);
Widget CreateBinLimitWindow(windowInfo *wInfo, char *geometry);
Widget CreateCellNormalizeWindow(windowInfo *wInfo, char *geometry);
Widget CreateStatsWindow(windowInfo *wInfo, char *geometry);
Widget CreateCoordsWindow(windowInfo *wInfo, char *geometry, int hasPlotCoords);
void WriteStatsPS(FILE *psFile, windowInfo *wInfo, char *fontName,
	int fontSize, int xPos, int yPos);
void UpdateSliderRange(windowInfo *wInfo);
void UpdateStatsWindow(windowInfo *wInfo);
void UpdateCellNormValueLabels(windowInfo *window);
int SliderNTRef(hsNTuple *ntuple, ntupleExtension *ntExt, int *sliders,
	int nSliders, float *thresholds, int index, int *sliderGTorLT);
