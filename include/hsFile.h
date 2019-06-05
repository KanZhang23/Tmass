/*******************************************************************************
*									       *
* hsFile.h -- Include file for the Nirvana Histoscope tool, HS File access     *
*									       *
* Copyright (c) 1993, 1994 Universities Research Association, Inc.	       *
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
* March 3, 1994								       *
*									       *
* Written by Joy Kyriakopulos				       		       *
*									       *
*******************************************************************************/

#ifndef HS_FILE_H_
#define HS_FILE_H_

#define HS_READ 0
#define HS_WRITE 1

/* Definitions for Histo-Scope format save file */

#define HS_ID_STRING_A "HISTOSCOPE FILE FMT A"
#define HS_ID_STRING_B "HISTOSCOPE FILE FMT B"
#define HS_ID_STRING_C "HISTOSCOPE FILE FMT C"
#define HS_ID_STRING_D "HISTOSCOPE FILE FMT D"
#define HS_FILE_A 1		     /*  HS_ID_STRING_A  */
#define HS_FILE_B 2		     /*  HS_ID_STRING_B  */
#define HS_FILE_C 3		     /*  HS_ID_STRING_C  */
#define HS_FILE_D 4		     /*  HS_ID_STRING_D  */

#define MAX_HS_ID_STRING_LEN 65

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _hsFile {
    char *filename;
    FILE *filePtr;
    XDR *xdrs;
    int xdrEnOrDeCode;
    int fileversion;
    int numOfItems;
    int *locTblFile;
    int *uidTable;
    char **categoryTable;
    hsGeneral **locTblMem;
    int *dataPos;
} hsFile;

/*
** Function Prototypes 
**/

hsFile    *OpenHsFile (const char *filename, int readOrWrite, char **errorString);
int        ReadHsFile (hsFile **hsfilePtr, char **errorString);
int        WriteHsFile (hsFile **hsfilePtr, hsGeneral **locTbl, int numOfItems,
			 char **errorString);
hsGeneral *ReadHsItem (hsFile **hsfilePtr, int index, char **errorString);
int        DirHsFile (hsFile **hsfilePtr, int *nfitFlag, char **errorString);
void       FreeHsFileStruct (hsFile **hsfilePtr);
int        ReadHsItemData (hsFile **hsfilePtr, int index, char **errorString);

#ifdef __cplusplus
}
#endif

#endif /* not HS_FILE_H_ */
