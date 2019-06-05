/*******************************************************************************
*									       *
* communications.h -- Communications include file for Nirvana Histoscope tool  *
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
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* May 11, 1992								       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
*******************************************************************************/

/* 	ConnectToClient Return Values:	*/

#define COM_OK 0

#define COM_CTC_ERROR_HADDR 1	/* Error getting host address          */
#define COM_CTC_ERROR_OPEN  2	/* Error opening connection            */
#define COM_CTC_ERROR_CONN  3	/* Error connecting to client process  */
#define COM_CTC_ERROR_SEND  4	/* Error sending msg to client process */

/* Return values from RequestErrors() and SetControl(): */
/* COM_OK: No error */
#define COM_ERR_V1CLIENT   1	/* CLIENT IS V1, no errors */
#define COM_ERR_NOCONNECT -1	/* Client is not connected */
#define COM_ERR_ERR	  -2	/* Item doesn't exist      */

/* 	Value of ComFD when there's no connection:    */

#define NO_CONNECTION -1

typedef void (*deferProc)();

/*
 *	Function Prototypes:
 */

void RequestUpdates(int id);
void EndUpdates(int id);
int RequestErrors(int id);
int ConnectToClient (char * on_host, int portNum);
int CaptiveConnect(char *setenvVar);
void DisconnectProcess(void);
int ReadClientCom (Widget parent);
void DisplayWaitMsg (Widget parent, Boolean cancelBtn);
void UndisplayWaitMsg (void);
int ReadHistoFile(char *file);
int SetControl(int id, float value);
int SetTrigger(int id);
int GetUpdateFreq(void);
void SetUpdateFreq(int updFreq);
void SetConfigFileToLoad(char *cfgFileToLoad);
void ScheduleDataArrivedCallback(int itemID, deferProc deferredCB, 
	void *dataPtr);
int ReportTaskCompletion(int taskNumber, int status, const char *result);
