/*******************************************************************************
*									       *
* publish.h -- Routines for histoscope to find programs to monitor, and for    *
*	       programs to announce their presence to histoscope tools	       *
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
#define TIME_FIELD_LEN 10	/* size of day of week + hour & minute string */
#define MAX_USER_NAME_LEN 256	/* enforced maximum on user name lengths      */
#define MAX_LIST_ERROR_LEN 80	/* max length for errors from ListIDFiles     */
#define MAX_NODE_LEN 256        /* max length of a nodename	              */
#define MAX_ID_FILE_SIZE /* pid */ 20 + /* sock */ 20 + MAX_USER_NAME_LEN + \
	HS_MAX_IDENT_LENGTH + TIME_FIELD_LEN + /* spaces & quotes */ 10 + \
	MAX_NODE_LEN

typedef struct _idFileData {
    int pid;
    int socketNum;
    char userName[MAX_USER_NAME_LEN];
    char idString[HS_MAX_IDENT_LENGTH];
    char startTime[TIME_FIELD_LEN];
    char node[MAX_NODE_LEN];
} idFileData;

int CreateIDFile(int socketNum, const char *identString, char *publString);
void RemoveIDFile(void);
void CleanupOldIDFiles(void);
int ListIDFiles(char *remoteHostName, idFileData **list, char *errorMessage,
	char *userName, char *passwd);
