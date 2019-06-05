#ifndef SIMPLE_TCL_API_H
#define SIMPLE_TCL_API_H

#include "tcl.h"

/* In the next definition Hs_api_ prefix may be changed to
 * any other string to avoid function names collisions in C
 */
#define tcl_c_name(name) (Hs_api_ ## name)

/* 
 * The next macro is convenient to use because all
 * Histo-Scope C API functions have the same prefix
 */
#define histo_api_name(name) (hs_ ## name)

#define tcl_routine(name) EXTERN int tcl_c_name(name)\
                                   (ClientData clientData,\
                                    Tcl_Interp *interp,\
                                    int objc, Tcl_Obj *CONST objv[])

#define tcl_new_command(namespace, name) if ((Tcl_CreateObjCommand(interp,\
      #namespace"::"#name, tcl_c_name(name), (ClientData)NULL,\
      (Tcl_CmdDeleteProc *)NULL)) == NULL)\
      return TCL_ERROR

#define tcl_require_objc(N) do {\
  if (objc != N)\
  {\
    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                     " : wrong # of arguments", NULL);\
    return TCL_ERROR;\
  }\
} while(0);

#define tcl_objc_range(N,M) do {\
  if (objc < N || objc > M)\
  {\
    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[0],NULL),\
                     " : wrong # of arguments", NULL);\
    return TCL_ERROR;\
  }\
} while(0);

#define obj_2_dstring(objname, dsname) do {\
    Tcl_UtfToExternalDString(NULL, Tcl_GetStringFromObj(objname, NULL), -1, &dsname);\
} while(0);

#define VOID_FUNCT_WITH_VOID_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  tcl_require_objc(1);\
  histo_api_name(c_api_funct) ();\
  return TCL_OK;\
}

#define VOID_FUNCT_WITH_ONE_CHAR_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  tcl_require_objc(2);\
  histo_api_name(c_api_funct) (Tcl_GetStringFromObj(objv[1], NULL));\
  return TCL_OK;\
}

#define VOID_FUNCT_WITH_ONE_INT_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  int i;\
  tcl_require_objc(2);\
  if (Tcl_GetIntFromObj(interp, objv[1], &i) != TCL_OK)\
    return TCL_ERROR;\
  histo_api_name(c_api_funct) (i);\
  return TCL_OK;\
}

#define VOID_FUNCT_WITH_ONE_BOOLEAN_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  int i;\
  tcl_require_objc(2);\
  if (Tcl_GetBooleanFromObj(interp, objv[1], &i) != TCL_OK)\
    return TCL_ERROR;\
  histo_api_name(c_api_funct) (i);\
  return TCL_OK;\
}

#define INT_FUNCT_WITH_VOID_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  tcl_require_objc(1);\
  Tcl_SetObjResult(interp, Tcl_NewIntObj((int) histo_api_name(c_api_funct) ()));\
  return TCL_OK;\
}

#define BOOL_FUNCT_WITH_VOID_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  tcl_require_objc(1);\
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj((int) histo_api_name(c_api_funct) ()));\
  return TCL_OK;\
}

#define INT_FUNCT_WITH_ONE_CHAR_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  tcl_require_objc(2);\
  Tcl_SetObjResult(interp, Tcl_NewIntObj((int) histo_api_name(c_api_funct)\
        (Tcl_GetStringFromObj(objv[1], NULL))));\
  return TCL_OK;\
}

#define BOOL_FUNCT_WITH_ONE_CHAR_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  tcl_require_objc(2);\
  Tcl_SetObjResult(interp, Tcl_NewBooleanObj((int) histo_api_name(c_api_funct)\
        (Tcl_GetStringFromObj(objv[1], NULL))));\
  return TCL_OK;\
}

#define INT_FUNCT_WITH_ONE_INT_ARG(c_api_funct) tcl_routine(c_api_funct)\
{\
  int i;\
  tcl_require_objc(2);\
  if (Tcl_GetIntFromObj(interp, objv[1], &i) != TCL_OK)\
    return TCL_ERROR;\
  Tcl_SetObjResult(interp, Tcl_NewIntObj((int) histo_api_name(c_api_funct) (i)));\
  return TCL_OK;\
}

/* Next definition is useful for prototyping. Use with care. */
#define FUNCT_NOT_IMPLEMENTED(c_api_funct) tcl_routine(c_api_funct)\
{\
  Tcl_AppendResult(interp, "Command \"", Tcl_GetStringFromObj(objv[0],NULL),\
  "\" is not implemented", NULL);\
  return TCL_ERROR;\
}

#endif /* not SIMPLE_TCL_API_H */
