/*******************************************************************************
*									       *
* readHbookList.c -- Small and minimal interface between a RZ HBOOK file and   *
* the HistoScope Server Process.  Given a UNIX filename, this routine will fill*
* the Histo.h structure, excluding the Data field.                             * 
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
* April 27, 1992							       *
*									       *
* Written by Paul Lebrun						       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "hsTypes.h"
#include "histoUtil.h"
#include "readHbookList.h"

/*
**	Open the RZ file, which contains Histogram Data, and make a 
**	listing of all histograms
*/ 
int ReadHbookList(char* filename, int lrec, int *numRead, 
			hsGeneral ***addrItemTablePtr)
{ 
    return 0;
}
