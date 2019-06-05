/*******************************************************************************
*									       *
* histoP.h -- Central include file for Histoscope routines			       *
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
*									       *
* Created 4/17/92							       *
*									       *
*******************************************************************************/

/* The maximum number of variables that can be displayed by any
   of the plotting widgets.  This is NOT the number of possible
   variables in an ntuple, which, conceptually, has no limit */
#define MAX_DISP_VARS 36

/* Name of preferences file saved in the users home directory */
#ifdef VMS
#define PREF_FILE_NAME ".HISTO"
#else
#define PREF_FILE_NAME ".histo"
#endif

/* Default block size for hbook files */
#define DEFAULT_HBOOK_BLOCK_SIZE 1024

#define MKSTRING(string) \
	XmStringCreateLtoR(string, XmSTRING_DEFAULT_CHARSET)
	
#define SET_ONE_RSRC(widget, name, newValue) \
{ \
    static Arg args[1] = {{name, (XtArgVal)0}}; \
    args[0].value = (XtArgVal)newValue; \
    XtSetValues(widget, args, 1); \
}	

#define GET_ONE_RSRC(widget, name, valueAddr) \
{ \
    static Arg args[1] = {{name, (XtArgVal)0}}; \
    args[0].value = (XtArgVal)valueAddr; \
    XtGetValues(widget, args, 1); \
}
