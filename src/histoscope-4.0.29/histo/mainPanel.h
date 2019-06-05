/*******************************************************************************
*									       *
* mainPanel.h -- Main panel for Nirvana Histoscope tool			       *
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
* April 20, 1992							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
enum fileFormat {HISTO_FORMAT, HBOOK_FORMAT};

extern Widget MainPanelW;

Widget CreateMainWindow(Display *display, int captiveMode);
void OpenNewFile(int format);
int OpenInitialHistoFile(char *filename);
int OpenInitialHbookFile(char *filename, int blockSize);
int GetCurrentCategoryHists(char *currentCategory, int **histIDs);
void RereadFile(void);
void OpenNewConnection(void);
void CloseFileOrConnection(void);
void RedisplayHistogramList(int resetCurrentCategory);
void AddItemToMainPanel(hsGeneral *item);
void DeleteItemFromMainPanel(int id);
void SetMainPanelConnected(char *title);
void CloseMainPanel(void);
void SetAllControlsInSensitive(void);
int LoadItemData(Widget parent, hsGeneral *item);
windowInfo *ViewItem(Display *display, hsGeneral *item, char *winID,
	char *geometry, int headingType, colorScaleInfo *csInfo,
	widgetConfigInfo *confInfo);
hsGeneral *GetMPItemByID(int id);
hsGeneral *GetMPItemByIDnp(int id);
hsGeneral *GetMPItemByUID(char *category, int uid);
hsGeneral *GetMPItemByName(char *category, char *name);
hsGeneral *GetMPItemByHbookID(char *category, int hbookID);
void UpdateNTuplePanelList(int id);
