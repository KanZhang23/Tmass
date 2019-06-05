/*******************************************************************************
*									       *
* plotMenus.h -- Popup menus for histoscope and nplot plot windows	       *
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
void CreateTSPlotMenu(windowInfo *wInfo);
void CreateXYPlotMenu(windowInfo *wInfo);
void CreateXYSortMenu(windowInfo *wInfo);
void Create2DScatMenu(windowInfo *wInfo);
void Create3DScatMenu(windowInfo *wInfo);
void Create1DHistMenu(windowInfo *wInfo, int fromNtuple);
void Create2DHistMenu(windowInfo *wInfo, int fromNtuple);
void Create1DAdaptHistMenu(windowInfo *wInfo);
void Create2DAdaptHistMenu(windowInfo *wInfo);
void CreateOverlayMenu(windowInfo *wInfo, int fromNtuple);
void CreateCellMenu(windowInfo *wInfo, int fromNtuple);
void CreateColorCellMenu(windowInfo *wInfo, int fromNtuple);
void CreateIndicatorMenu(windowInfo *wInfo);
void CreateControlMenu(windowInfo *wInfo);
void CreateTriggerMenu(windowInfo *wInfo);
void RemovePlotMenu(windowInfo *wInfo);
