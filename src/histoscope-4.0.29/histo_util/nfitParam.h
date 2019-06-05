/*******************************************************************************
*									       *
* nfitParam.h -- Include file for NFit minuitParam structure		       *
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
* Jan 24, 1994								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

/* Maximum number of parameters allowed by MINUIT.  Must agree with the
   parameters defined for the MINUIT library that NFit is linked with */
#define MAX_PARAMS 100
#define MAX_VARIABLE_PARAMS 50
#define NFIT_ITEM_VERSION 0

typedef struct {
    char active;		/* is included in expression or compiled fn */
    char nameBlank;		/* flags to indicate blank fields */
    char valueBlank;
    char initValueBlank;
    char lowLimBlank;
    char upLimBlank;
    char stepBlank;
    char fixed;			/* fixed in minuit */
    char name[11];
    double value;
    double initValue;
    double lowLim;
    double upLim;
    double step;
    double parabolicError;
    double minosError[2];
    double globcc;
} minuitParam;
