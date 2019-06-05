/*******************************************************************************
*									       *
* GetHbookData.h -- Small and minimal interface between a RZ HBOOK file and    *
* the HistoScope Server Process.  Given a UNIX filename, this routine will fill*
* the Data Field of the Histo.h structure.                                     * 
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
* April 28, 1992							       *
*									       *
* Written by Paul Lebrun						       *
*									       *
*******************************************************************************/
int GetHbookDataF(char* filename, int lrec, hsGeneral *hsg);
int GetHbookData(hsGeneral *hsg, int fromTuple, int toTuple, int getErrFlg);
int hbneedtoupdate( hsGeneral* hsg, int updateLevel );

#define HBOOK_DATA_OK -1 
#define HBOOK_DATA_MISSING 1
#define HBOOK_DATA_TOO_BIG 3
#define HBOOK_DATA_CORRUPTED 5
#define HBOOK_DATA_FILE_OPEN_ERR 7
#define HBOOK_ERRORS_TOO_BIG 9
#define HBOOK_NTUPLE_IS_CIRCULAR -42
