/*
 *  rdl.h --
 *      
 *    This is the standard header file for the readline Tcl extension
 */

#ifndef _RDL_H
#define _RDL_H

#ifndef RDL_VERSION
    #define RDL_VERSION "1.1"
#endif

#include "tcl.h"

EXTERN int Rdl_Init(Tcl_Interp *interp);

#endif /* _RDL_H */
