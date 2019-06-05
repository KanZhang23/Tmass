/*******************************************************************************
*									       *
* hsdir.c -- HS File Directory Program for the Nirvana Histoscope tool	       *
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
* March 7, 1994								       *
*									       *
* Written by Joy Kyriakopulos				       		       *
*									       *
*******************************************************************************/

/*
* REQUIRED INCLUDE FILES
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <string.h>

#include "hsTypes.h"
#include "hsFile.h"
#include "histprotocol.h"
#include "histoUtil.h"

static void printDirectory(char *filename);

int main (int argc, char **argv)
{
    int i;
    
#ifdef VMS
    /* Convert VMS style command line to Unix style */
    ConvertVMSCommandLine((int *)&argc, &argv);
#endif /*VMS*/
    
    /* Process command line arguments: Hs files to display directories */
    for (i=1; i<argc; i++) {
#ifdef VMS
	int numFiles, j;
	char **nameList = NULL;
	/* Use VMS's LIB$FILESCAN for filename in argv[i] to process */
	/* wildcards and to obtain a full VMS file specification     */
	numFiles = VMSFileScan(argv[i], &nameList, NULL, EXCLUDE_FNF);
	/* for each expanded file name do: */
	for (j = 0; j < numFiles; ++j) {
	    printDirectory(nameList[j]);
	    free(nameList[j]);
	}
	if (nameList != NULL)
	    free(nameList);
#else
	printDirectory(argv[i]);
#endif /*VMS*/
    }
    
    /* If no file names were given on the command line, ask for one */
    
    if (argc <= 0) {
#ifdef VMS
	int numFiles, j;
	char **nameList = NULL;
#endif /*VMS*/
	char fileName[MAXPATHLEN];
    	printf("\nfilename? ");
    	scanf("%s", fileName);
#ifdef VMS
	/* Use VMS's LIB$FILESCAN for filename in argv[i] to process */
	/* wildcards and to obtain a full VMS file specification     */
	numFiles = VMSFileScan(argv[i], &nameList, NULL, EXCLUDE_FNF);
	/* for each expanded file name do: */
	for (j = 0; j < numFiles; ++j) {
	    printDirectory(nameList[j]);
	    free(nameList[j]);
	}
	if (nameList != NULL)
	    free(nameList);
#else
	printDirectory(argv[i]);
#endif /*VMS*/
    }

    exit(0);
}

static void printDirectory(char *filename)
{
    hsFile *hsfile;
    char *errString = NULL;
    int i, stat, numItems, flag;
    hsGeneral *item;
    static char *hsType[N_HS_DATA_TYPES] = {"1dHist", "2dHist", "Ntuple", "Indic", 
        "Ctrl", "Trig", "None", "NFit", "Group", "Config", "3dHist"};

    hsfile = OpenHsFile(filename, HS_READ, &errString);
    if (errString != NULL) {
    	fprintf(stderr, "\nhsdir: %s\n", errString);
    	free(errString);
    	errString = NULL;
    }
    if (hsfile == NULL) {
    	printf("\nNo items listed\n");
    	return;
    }
    
    stat = DirHsFile(&hsfile, &flag, &errString);
    if (errString != NULL) {
        fprintf(stderr, "\nhsdir: %s\n", errString);
        free(errString);
        errString = NULL;
    }
    if (hsfile == NULL) {
        printf("\nNo items listed\n");
        return;
    }

    numItems = hsfile->fileversion < HS_FILE_C ? stat : hsfile->numOfItems;
    
    if (flag == 0)
	printf("\nDirectory of Histo-Scope format file: %s\n\n", filename);
    else
	printf("\nDirectory of NFit format file: %s\n\n", filename);
    if (hsfile != NULL && stat > 0) {
    	printf(
"     uid  category                      title                          type\n\
     ---  --------                      -----                          ----\n");
    	for (i = 0; i < numItems && i < stat; ++i) {
    	    item = hsfile->locTblMem[i];
    	    printf("%8d  %-29s %-30s %-6s\n", item->uid, item->category,
    	    	item->title, hsType[item->type]);
    	    FreeItem(item);
    	}
    }
    if (errString != NULL)
    	printf("\n%s\n", errString);
    if (stat > 0 && hsfile != NULL)
    	if (hsfile->fileversion < HS_FILE_C)
    	    printf("\nTotal of %d items listed in file\n\n", stat);
    	else
    	    printf("\nTotal of %d items listed out of %d in file\n\n", stat, 
    		hsfile->numOfItems);
    else
    	printf("\nNo items listed\n");
    
    FreeHsFileStruct(&hsfile);
}
