#include <cstring>
#include <sstream>
#include <cstdio>

#include "tcl.h"
#include "loadResultsFromHS.hh"
#include "makeResultHisto.hh"

static int c_load_results_as_histos(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static Tcl_Interp *commandInterp = 0;

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

#ifdef __cplusplus
extern "C" {
#endif

int _hs_init(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "load_results_as_histos",
			     c_load_results_as_histos,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;

    commandInterp = interp;
    return TCL_OK;
}

void _hs_fini(void)
{
    if (commandInterp)
    {
	Tcl_DeleteCommand(commandInterp, "load_results_as_histos");
    }
}

#ifdef __cplusplus
}
#endif

static int c_load_results_as_histos(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    // Usage: load_results_as_histos infile categ prefix make_1d_only
    //             scale useQmcUncertainties
    tcl_require_objc(7);

    char* infile = Tcl_GetStringFromObj(objv[1], NULL);
    char* categ = Tcl_GetStringFromObj(objv[2], NULL);
    char* prefix = Tcl_GetStringFromObj(objv[3], NULL);
    int make_1d_only;
    if (Tcl_GetBooleanFromObj(interp, objv[4], &make_1d_only) != TCL_OK)
        return TCL_ERROR;

    double scale;
    if (Tcl_GetDoubleFromObj(interp, objv[5], &scale) != TCL_OK)
        return TCL_ERROR;

    int useQmc;
    if (Tcl_GetBooleanFromObj(interp, objv[6], &useQmc) != TCL_OK)
        return TCL_ERROR;

    std::vector<JesIntegResult> results;
    loadResultsFromHS(infile, categ, &results);

    Tcl_Obj* reslist = Tcl_NewListObj(0, NULL);
    const unsigned nRes = results.size();
    if (nRes)
    {
        const unsigned lenPrefix = strlen(prefix);
        const unsigned lenCateg = strlen(categ);
        std::ostringstream cats;
        if (lenPrefix && lenCateg)
            cats << prefix << '/' << categ;
        else if (lenPrefix)
            cats << prefix;
        else if (lenCateg)
            cats << categ;
        const std::string& ocateg = cats.str();
        char title[100];
        for (unsigned ires=0; ires<nRes; ++ires)
        {
            sprintf(title, "Result %d", results[ires].uid);
            const int id = makeResultHisto(results[ires].uid, title,
                                           ocateg.c_str(), results[ires],
                                           make_1d_only, scale, useQmc);
            if (id >= 0)
                Tcl_ListObjAppendElement(interp, reslist, Tcl_NewIntObj(id));
        }
    }
    Tcl_SetObjResult(interp, reslist);
    return TCL_OK;
}

/*
  Compile and load this code using the following commands:

  hs::sharedlib_compile load_results_as_histos.cc ./load_results_as_histos.so
  set dlltoken [hs::sharedlib open ./load_results_as_histos.so]

  When you are done using this command, unload it with

  hs::sharedlib close $dlltoken
*/
