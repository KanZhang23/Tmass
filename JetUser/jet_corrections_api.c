#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tcl.h"
#include "JetCorrectionsInterface.hh"

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

static int c_corrector_level(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int icorr;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
        Tcl_SetResult(interp, "corrector number is invalid", TCL_STATIC);
        return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(corrector_level(icorr)));
    return TCL_OK;
}

static int c_set_sys_total_correction(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int icorr, systype;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
        Tcl_SetResult(interp, "corrector number is invalid", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &systype) != TCL_OK)
        return TCL_ERROR;
    set_sys_total_correction(icorr, systype);
    return TCL_OK;
}

static int c_init_generic_corrections(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int icorr, level, nvtx, conesize, version, syscode, nrun, mode;

    tcl_require_objc(9);
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (icorr < 0 || icorr >= N_JET_CORRECTORS)
    {
        Tcl_SetResult(interp, "corrector number is out of range", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &level) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &nvtx) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &conesize) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[5], &version) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[6], &syscode) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[7], &nrun) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[8], &mode) != TCL_OK)
        return TCL_ERROR;
    init_generic_corrections(icorr, level, nvtx, conesize,
                             version, syscode, nrun, mode);
    return TCL_OK;
}

static int c_generic_correction_uncertainty(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    double scale, ptin, detectorEta;
    int icorr, mode;

    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
        Tcl_SetResult(interp, "corrector number is invalid", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[2], &ptin) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &detectorEta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &mode) != TCL_OK)
        return TCL_ERROR;

    double u = generic_correction_uncertainty(icorr, ptin, detectorEta, mode);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(u));
    return TCL_OK;
}

static int c_generic_correction_scale(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    double scale, ptin, emf, detectorEta;
    float f_emf;
    int icorr;
    
    tcl_require_objc(5);
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
        Tcl_SetResult(interp, "corrector number is invalid", TCL_STATIC);
        return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[2], &ptin) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &emf) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &detectorEta) != TCL_OK)
        return TCL_ERROR;

    f_emf = emf;
    scale = generic_correction_scale(icorr, ptin, &f_emf, detectorEta);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(scale));
    return TCL_OK;
}

int _hs_init(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "set_sys_total_correction",
			     c_set_sys_total_correction,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "corrector_level",
			     c_corrector_level,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "init_generic_corrections",
			     c_init_generic_corrections,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "generic_correction_scale",
			     c_generic_correction_scale,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "generic_correction_uncertainty",
			     c_generic_correction_uncertainty,
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
	Tcl_DeleteCommand(commandInterp, "set_sys_total_correction");
	Tcl_DeleteCommand(commandInterp, "init_generic_corrections");
	Tcl_DeleteCommand(commandInterp, "generic_correction_scale");
	Tcl_DeleteCommand(commandInterp, "generic_correction_uncertainty");
	Tcl_DeleteCommand(commandInterp, "corrector_level");
    }
}

double jetcorr(double x, double y, double z, int mode,
               const double *pars, int *ierr)
{
    /* Input arguments:
     *   x     -- the first variable
     *   y     -- the second variable (may be ignored)
     *   z     -- the third variable (may be ignored)
     *   mode  -- an external parameter which will not
     *            be changed by Minuit (may be ignored)
     *   pars  -- array of fitted parameters
     *
     * Output:
     *   *ierr -- error status. 0 means everything is OK.
     *            If something goes wrong, the function is expected
     *            to print an informative error message to stdout
     *            and set *ierr to a non-zero value.
     * Returns:
     *   The function value at given coordinates with given parameters.
     */
    int icorr = pars[0];
    float detectorEta = pars[1];
    float emf = pars[2];

    if (!corrector_valid(icorr))
        return 0.0;
    if (x > 0.0 && emf >= 0.f && emf <= 1.f)
        return x*generic_correction_scale(icorr, x, &emf, detectorEta);
    else
        return 0.0;
}

#include "level0_pt.c"

double l0pt(double x, double y, double z, int mode,
            const double *pars, int *ierr)
{
    /* Input arguments:
     *   x     -- the first variable
     *   y     -- the second variable (may be ignored)
     *   z     -- the third variable (may be ignored)
     *   mode  -- an external parameter which will not
     *            be changed by Minuit (may be ignored)
     *   pars  -- array of fitted parameters
     *
     * Output:
     *   *ierr -- error status. 0 means everything is OK.
     *            If something goes wrong, the function is expected
     *            to print an informative error message to stdout
     *            and set *ierr to a non-zero value.
     * Returns:
     *   The function value at given coordinates with given parameters.
     */
    int icorr = pars[0];
    float detectorEta = pars[1];
    float emf = pars[2];

    if (!corrector_valid(icorr))
        return 0.0;
    if (x > 0.0 && emf >= 0.f && emf <= 1.f)
        return level0_pt(icorr, x, emf, detectorEta);
    else
        return 0.0;
}

double invcheck(double x, double y, double z, int mode,
                const double *pars, int *ierr)
{
    return l0pt(jetcorr(x, y, z, mode, pars, ierr),
                y, z, mode, pars, ierr);
}

/*
  Compile and load this code using the following commands:

  hs::sharedlib_compile jet_corrections_api.c ./jet_corrections_api.so
  set dlltoken [hs::sharedlib open ./jet_corrections_api.so]

  When you are done using this command, unload it with

  hs::sharedlib close $dlltoken
*/
