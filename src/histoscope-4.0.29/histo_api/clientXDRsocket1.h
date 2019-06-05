/*******************************************************************************
*									       *
* clientXDRsocket1.h -- Include file for clientXDRsocket.c.  Used for all XDR  *
*		 and socket operations.  This separate module is put into its  *
*		 own shareable image to hide the Multinet bcopy routine from   *
*		 the client program.  Thus even users of YBOS can use Histo-   *
*		 Scope.							       *
*									       *
* Copyright (c) 1993 Universities Research Association, Inc.		       *
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
* November 5, 1993							       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modification History:							       *
*									       *
*									       *
*******************************************************************************/

#ifdef VMS

extern char * hs_gethostbyname(char *name);
extern int    hs_gethostname(char *name, unsigned int length);

#endif /*VMS*/
