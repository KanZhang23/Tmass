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

#include "MTM3_Init.h"

#include "level0_pt.c"

// #include "randomize_jet_energy.h"
// #include "angular_transfer_function.h"
#include "lhapdf_interface.h"
#include "tfs_2015.h"
#include "topmass_utils.h"
#include "twidth.h"
#include "wgrid.h"
#include "solve_top.h"
#include "quartic_lib.h"
#include "convolute_breit_wigner.h"
#include "transfer_function.h"
#include "johnson_random.h"
#include "random_jet_angles.h"
#include "linear_interpolator_1d.h"
#include "linear_interpolator_2d.h"
#include "p_struct_function.h"
#include "bw_polynomial.h"
#include "pi.h"
#include "n_d_random.h"
#include "single_jet_probs.h"
#include "scrambled_sobol.h"
#include "tf_interpolator.h"
#include "qqbar_deltaR_efficiency.h"
#include "ordered_tree_collection.h"
#include "ratio_tf_shift.h"
#include "sorter.h"
#include "tfs_2015.h"

#define HISTO_TF_PLOT

#ifdef HISTO_TF_PLOT
#include "ordered_tree_api.h"
#include "linear_interpolator_nd_api.h"
#include "cdf_2d_api.h"
#include "cdf_3d_api.h"
#include "histoscope.h"
#endif

#define MAX_TF_PARAMS 100
#define MAX_3D_NIDS 8

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PART_W = 0,
    PART_NU,
    PART_B,
    PART_WB,
    PART_MC
};

typedef struct {
    int i1;
    int i2;
} i_i_pair;

int solve_top_api_init(Tcl_Interp *interp);
int _hs_init(Tcl_Interp *interp);
void _hs_fini(void);

static const unsigned uncert_mode = 1U;
static char obsolete[] = "this function is obsolete";

static double hypot3(double x, double y, double z)
{
    return sqrt(x*x + y*y + z*z);
}
    
static int compare_double(const double *p1, const double *p2)
{
    if (*p1 < *p2)
        return -1;
    else if (*p1 > *p2)
        return 1;
    else
        return 0;
}

static int mc_leptonic_side_solver(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const int debug_level, const size_t max_iterations,
    lepton_side_solution solutions[4])
{
    const double minTopPz = -1000.0;
    const double maxTopPz = 1000.0;

    if (mb == 0.0)
        return solve_leptonic_byMW_approx(
            topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
            mt, mb, mwsq, 1.0, debug_level,
            minTopPz, maxTopPz, solutions);
    else
        return solve_leptonic_side_massiveb_brute(
            topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
            mt, mb, mwsq, debug_level, max_iterations,
            minTopPz, maxTopPz, solutions);
}

static int c_reset_impossible_tf_bins(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_get_jet_pt_cutoff(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_get_min_pt_exceedance(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_random_angle(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_params_to_loose_parton(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_angle_width_factor(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_adjust_angular_tf_width(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_init_lhapdf(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_cleanup_lhapdf(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_alphas_lhapdf(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_solve_leptonic_side(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

/*
static int c_drteq4(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);
*/
    
static int c_quartic(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_cubic(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_convolute_bw(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_solve_hadronic_side(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_w_mass_range(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_nuz_local_peak(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_nuz_minmax(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_debug_level(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_print_wgrid(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_tf_parameters(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_get_tf_parameters(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_transfer_function(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_jet_scale_and_uncert_from_l5(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_bare_ratio_density(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_bare_ratio_efficiency(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_topwidth(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_uniform_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_normal_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_johnson_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_init_n_d_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_cleanup_n_d_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_next_n_d_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_sobol_scrambling_multiplier(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_reset_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_jet_tf_predictor(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_angular_tree_collection(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_ratio_shift_tree(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_integrated_angular_efficiency(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_load_tfs_2015(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_bare_transfer_function(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_bare_ptratio_exceedance(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_randomize_sampling_fromrand(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_bare_tf_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_single_parton_eff(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_uniform_random_setseed(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

#ifdef HISTO_TF_PLOT
static int c_angular_tf_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_angular_tf_random_fill(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_transfer_function_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_transfer_function_invcdf_scan_2(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_transfer_function_efficiency_scan_2(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_mc_eff_prob_to_loose_parton_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_linear_interpolate_1d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_linear_interpolate_2d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_bw_multipole_coeffs(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_tf_interpolate(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_load_tf_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_parton_eff_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_angular_eff_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_set_qqbar_deltaR_interpolator(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

static int c_generic_correction_uncertainty_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[]);

#endif

static int debug_level = 0;
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

#define verify_1d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_1D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 1d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define verify_2d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_2D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 2d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define verify_3d_histo(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_3D_HISTOGRAM)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a 3d histogram", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

#define verify_ntuple(id,objnum) do {\
    if (Tcl_GetIntFromObj(interp, objv[objnum], &id) != TCL_OK)\
	return TCL_ERROR;\
    if (hs_type(id) != HS_NTUPLE)\
    {\
	if (hs_type(id) == HS_NONE)\
	    Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not a valid Histo-Scope id", NULL);\
	else\
	    Tcl_AppendResult(interp, "item with id ",\
			     Tcl_GetStringFromObj(objv[objnum], NULL),\
			     " is not an ntuple", NULL);\
	return TCL_ERROR;\
    }\
} while(0);

int MTM3_Init(Tcl_Interp *interp)
{
    return _hs_init(interp);
}

int _hs_init(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.3", 0) == NULL) {
	return TCL_ERROR;
    }
#endif

    return solve_top_api_init(interp);
}

int solve_top_api_init(Tcl_Interp *interp)
{
    if (Tcl_CreateObjCommand(interp,
			     "get_jet_pt_cutoff",
			     c_get_jet_pt_cutoff,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "get_min_pt_exceedance",
			     c_get_min_pt_exceedance,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "random_angle",
			     c_random_angle,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_params_to_loose_parton",
			     c_set_params_to_loose_parton,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "reset_impossible_tf_bins",
			     c_reset_impossible_tf_bins,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_angle_width_factor",
			     c_set_angle_width_factor,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "adjust_angular_tf_width",
			     c_adjust_angular_tf_width,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "init_lhapdf",
			     c_init_lhapdf,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "cleanup_lhapdf",
			     c_cleanup_lhapdf,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "alphas_lhapdf",
			     c_alphas_lhapdf,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "solve_leptonic",
			     c_solve_leptonic_side,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "solve_hadronic",
			     c_solve_hadronic_side,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "w_mass_range",
			     c_w_mass_range,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "nuz_local_peak",
			     c_nuz_local_peak,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "nuz_minmax",
			     c_nuz_minmax,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "debug_level",
			     c_set_debug_level,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    /*
    if (Tcl_CreateObjCommand(interp,
			     "drteq4",
			     c_drteq4,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    */
    if (Tcl_CreateObjCommand(interp,
			     "quartic",
			     c_quartic,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "cubic",
			     c_cubic,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "convolute_bw",
			     c_convolute_bw,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "print_wgrid",
			     c_print_wgrid,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "transfer_function",
			     c_transfer_function,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "jet_scale_and_uncert_from_l5",
			     c_jet_scale_and_uncert_from_l5,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "bare_ratio_density",
			     c_bare_ratio_density,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "bare_ratio_efficiency",
			     c_bare_ratio_efficiency,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "topwidth",
			     c_topwidth,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "johnson_random",
			     c_johnson_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "uniform_random",
			     c_uniform_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
     if (Tcl_CreateObjCommand(interp,
			     "normal_random",
			     c_normal_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
   if (Tcl_CreateObjCommand(interp,
			     "reset_interpolators",
			     c_reset_interpolators,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "jet_tf_predictor",
			     c_jet_tf_predictor,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_angular_tree_collection",
			     c_set_angular_tree_collection,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_ratio_shift_tree",
			     c_set_ratio_shift_tree,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "integrated_angular_efficiency",
			     c_integrated_angular_efficiency,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;

    if (Tcl_CreateObjCommand(interp,
			     "load_tfs_2015",
			     c_load_tfs_2015,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "bare_transfer_function",
			     c_bare_transfer_function,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "bare_ptratio_exceedance",
			     c_bare_ptratio_exceedance,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "randomize_sampling_fromrand",
			     c_randomize_sampling_fromrand,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "bare_tf_random",
			     c_bare_tf_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "single_parton_eff",
			     c_single_parton_eff,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "uniform_random_setseed",
			     c_uniform_random_setseed,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    
#ifdef HISTO_TF_PLOT
    if (Tcl_CreateObjCommand(interp,
			     "generic_correction_uncertainty_scan",
			     c_generic_correction_uncertainty_scan,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "angular_tf_scan",
			     c_angular_tf_scan,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "angular_tf_random_fill",
			     c_angular_tf_random_fill,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "transfer_function_scan",
			     c_transfer_function_scan,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "transfer_function_invcdf_scan_2",
			     c_transfer_function_invcdf_scan_2,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "transfer_function_efficiency_scan_2",
			     c_transfer_function_efficiency_scan_2,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "mc_eff_prob_to_loose_parton_scan",
			     c_mc_eff_prob_to_loose_parton_scan,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "linear_interpolate_1d",
			     c_linear_interpolate_1d,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "linear_interpolate_2d",
			     c_linear_interpolate_2d,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "bw_multipole_coeffs",
			     c_bw_multipole_coeffs,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "tf_interpolate",
			     c_tf_interpolate,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "load_tf_interpolators",
			     c_load_tf_interpolators,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_parton_eff_interpolators",
			     c_set_parton_eff_interpolators,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_angular_eff_interpolators",
			     c_set_angular_eff_interpolators,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_qqbar_deltaR_interpolator",
			     c_set_qqbar_deltaR_interpolator,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;

    if (init_ordered_tree_api(interp) != TCL_OK)
        return TCL_ERROR;

    if (init_linear_interpolator_nd_api(interp) != TCL_OK)
        return TCL_ERROR;

    if (init_cdf_2d_api(interp) != TCL_OK)
        return TCL_ERROR;

    if (init_cdf_3d_api(interp) != TCL_OK)
        return TCL_ERROR;
#endif
    if (Tcl_CreateObjCommand(interp,
			     "set_tf_parameters",
			     c_set_tf_parameters,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "get_tf_parameters",
			     c_get_tf_parameters,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "set_sobol_scrambling_multiplier",
			     c_set_sobol_scrambling_multiplier,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "init_n_d_random",
			     c_init_n_d_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "cleanup_n_d_random",
			     c_cleanup_n_d_random,
			     (ClientData)NULL,
			     (Tcl_CmdDeleteProc *)NULL) == NULL)
	return TCL_ERROR;
    if (Tcl_CreateObjCommand(interp,
			     "next_n_d_random",
			     c_next_n_d_random,
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
	Tcl_DeleteCommand(commandInterp, "get_jet_pt_cutoff");
	Tcl_DeleteCommand(commandInterp, "get_min_pt_exceedance");
	Tcl_DeleteCommand(commandInterp, "random_angle");
	Tcl_DeleteCommand(commandInterp, "set_params_to_loose_parton");
	Tcl_DeleteCommand(commandInterp, "reset_impossible_tf_bins");
	Tcl_DeleteCommand(commandInterp, "set_angle_width_factor");
	Tcl_DeleteCommand(commandInterp, "adjust_angular_tf_width");
	Tcl_DeleteCommand(commandInterp, "init_lhapdf");
	Tcl_DeleteCommand(commandInterp, "cleanup_lhapdf");
	Tcl_DeleteCommand(commandInterp, "alphas_lhapdf");
	Tcl_DeleteCommand(commandInterp, "solve_leptonic");
	Tcl_DeleteCommand(commandInterp, "solve_hadronic");
	Tcl_DeleteCommand(commandInterp, "w_mass_range");
	Tcl_DeleteCommand(commandInterp, "nuz_local_peak");
	Tcl_DeleteCommand(commandInterp, "nuz_minmax");
	Tcl_DeleteCommand(commandInterp, "debug_level");
	/* Tcl_DeleteCommand(commandInterp, "drteq4"); */
	Tcl_DeleteCommand(commandInterp, "quartic");
	Tcl_DeleteCommand(commandInterp, "cubic");
	Tcl_DeleteCommand(commandInterp, "convolute_bw");
        Tcl_DeleteCommand(commandInterp, "print_wgrid");
        Tcl_DeleteCommand(commandInterp, "transfer_function");
        Tcl_DeleteCommand(commandInterp, "jet_scale_and_uncert_from_l5");
        Tcl_DeleteCommand(commandInterp, "bare_ratio_density");
        Tcl_DeleteCommand(commandInterp, "bare_ratio_efficiency");
        Tcl_DeleteCommand(commandInterp, "topwidth");
        Tcl_DeleteCommand(commandInterp, "johnson_random");
        Tcl_DeleteCommand(commandInterp, "uniform_random");
        Tcl_DeleteCommand(commandInterp, "normal_random");
        Tcl_DeleteCommand(commandInterp, "reset_interpolators");
        Tcl_DeleteCommand(commandInterp, "jet_tf_predictor");
        Tcl_DeleteCommand(commandInterp, "set_angular_tree_collection");
        Tcl_DeleteCommand(commandInterp, "set_ratio_shift_tree");
        Tcl_DeleteCommand(commandInterp, "integrated_angular_efficiency");

        Tcl_DeleteCommand(commandInterp, "load_tfs_2015");
        Tcl_DeleteCommand(commandInterp, "bare_transfer_function");
        Tcl_DeleteCommand(commandInterp, "bare_ptratio_exceedance");
        Tcl_DeleteCommand(commandInterp, "randomize_sampling_fromrand");
        Tcl_DeleteCommand(commandInterp, "bare_tf_random");
        Tcl_DeleteCommand(commandInterp, "single_parton_eff");
        Tcl_DeleteCommand(commandInterp, "uniform_random_setseed");

#ifdef HISTO_TF_PLOT
        Tcl_DeleteCommand(commandInterp, "generic_correction_uncertainty_scan");
        Tcl_DeleteCommand(commandInterp, "angular_tf_scan");
        Tcl_DeleteCommand(commandInterp, "angular_tf_random_fill");
        Tcl_DeleteCommand(commandInterp, "transfer_function_invcdf_scan_2");
        Tcl_DeleteCommand(commandInterp, "transfer_function_efficiency_scan_2");
        Tcl_DeleteCommand(commandInterp, "mc_eff_prob_to_loose_parton_scan");
        Tcl_DeleteCommand(commandInterp, "linear_interpolate_1d");
        Tcl_DeleteCommand(commandInterp, "linear_interpolate_2d");
        Tcl_DeleteCommand(commandInterp, "bw_multipole_coeffs");
        Tcl_DeleteCommand(commandInterp, "tf_interpolate");
        Tcl_DeleteCommand(commandInterp, "load_tf_interpolators");
        Tcl_DeleteCommand(commandInterp, "set_parton_eff_interpolators");
        Tcl_DeleteCommand(commandInterp, "set_angular_eff_interpolators");

        cleanup_ordered_tree_api();
        cleanup_linear_interpolator_nd_api();
        cleanup_cdf_2d_api();
        cleanup_cdf_3d_api();
#endif
        Tcl_DeleteCommand(commandInterp, "set_tf_parameters");
        Tcl_DeleteCommand(commandInterp, "get_tf_parameters");
        Tcl_DeleteCommand(commandInterp, "init_n_d_random");
        Tcl_DeleteCommand(commandInterp, "cleanup_n_d_random");
        Tcl_DeleteCommand(commandInterp, "next_n_d_random");
        Tcl_DeleteCommand(commandInterp, "set_sobol_scrambling_multiplier");
    }
}

static int c_adjust_angular_tf_width(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_set_ratio_shift_tree(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: set_ratio_shift_tree tree_num isB */
    int i, isB;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &i) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[2], &isB) != TCL_OK)
        return TCL_ERROR;
    set_ratio_shift_tree(get_ordered_tree_by_number(i), isB);
    return TCL_OK;
}

static int c_set_angular_tree_collection(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_set_angle_width_factor(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: set_angle_width_factor which isB factor
     *        "which" is a string "eta" or "phi"
     */
    char *which;
    int isB, isEta;
    double factor;

    tcl_require_objc(4);
    which = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcasecmp(which, "eta") == 0)
        isEta = 1;
    else if (strcasecmp(which, "phi") == 0)
        isEta = 0;
    else
    {
        Tcl_AppendResult(interp, "bad angle type \"", which, "\"", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetBooleanFromObj(interp, objv[2], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &factor) != TCL_OK)
        return TCL_ERROR;
    if (isEta)
        set_eta_width_factor(factor, isB);
    else
        set_phi_width_factor(factor, isB);
    return TCL_OK;
}

static int c_set_params_to_loose_parton(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: set_params_to_loose_parton pt_cut eta_cut max_efficiency jes_sigma_factor
     */
    double pt_cut, eta_cut, max_efficiency, jes_sigma_factor;

    tcl_require_objc(5);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &pt_cut) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &eta_cut) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &max_efficiency) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &jes_sigma_factor) != TCL_OK)
        return TCL_ERROR;

    set_params_to_loose_parton(pt_cut, eta_cut, max_efficiency, jes_sigma_factor);
    return TCL_OK;
}

static int c_get_jet_pt_cutoff(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    tcl_require_objc(1);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(get_jet_pt_cutoff()));
    return TCL_OK;
}

static int c_get_min_pt_exceedance(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    tcl_require_objc(1);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(get_min_pt_exceedance()));
    return TCL_OK;
}

static int c_random_angle(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: random_angle which isB pt
     *        "which" is a string "eta" or "phi"
     */
    char *which;
    int isB, isEta;
    double pt, r;

    tcl_require_objc(4);
    which = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcasecmp(which, "eta") == 0)
        isEta = 1;
    else if (strcasecmp(which, "phi") == 0)
        isEta = 0;
    else
    {
        Tcl_AppendResult(interp, "bad angle type \"", which, "\"", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetBooleanFromObj(interp, objv[2], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &pt) != TCL_OK)
        return TCL_ERROR;
    if (isEta)
    {
        if (isB)
            r = random_b_deta(pt);
        else
            r = random_q_deta(pt);
    }
    else
    {
        if (isB)
            r = random_b_dphi(pt);
        else
            r = random_q_dphi(pt);
    }
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(r));
    return TCL_OK;
}

static int c_cleanup_lhapdf(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    tcl_require_objc(1);
    cleanup_lhapdf();
    return TCL_OK;
}

static int c_init_lhapdf(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    double Q2fac = 1.0;
    char *pdfname;

    tcl_objc_range(2, 3);
    pdfname = Tcl_GetStringFromObj(objv[1], NULL);
    if (objc > 2)
        if (Tcl_GetDoubleFromObj(interp, objv[2], &Q2fac) != TCL_OK)
            return TCL_ERROR;
    if (Q2fac <= 0.0)
    {
        Tcl_SetResult(interp, "Q^2 factor must be positive",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }
    init_lhapdf(pdfname, Q2fac);
    return TCL_OK;
}

static int c_alphas_lhapdf(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    double Q2;

    tcl_require_objc(2);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &Q2) != TCL_OK)
	return TCL_ERROR;
    if (Q2 <= 0.0)
    {
        Tcl_SetResult(interp, "Q^2 must be positive",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(alphas_lhapdf(Q2)));
    return TCL_OK;
}

static int c_solve_leptonic_side(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: solve_leptonic particle double double ... */
    int nsols, i, index0;
    double a[11];
    char *particle;
    lepton_side_solution solutions[4];
    int (*myfunction)(
        const double, const double,
        const double, const double, const double,
        const double, const double, const double,
        const double, const double, const double,
        const int,    const size_t, lepton_side_solution *);
    static int (*functions[5])(
        const double, const double,
        const double, const double, const double,
        const double, const double, const double,
        const double, const double, const double,
        const int,    const size_t, lepton_side_solution *) = {
            solve_leptonic_side,
            solve_leptonic_byNuPz,
            solve_leptonic_byPb,
            solve_leptonic_byMWsq,
            mc_leptonic_side_solver
    };
    Tcl_Obj *results[4];

    tcl_require_objc(13);
    particle = Tcl_GetStringFromObj(objv[1], NULL);
    if (strcasecmp(particle, "w") == 0)
        index0 = PART_W;
    else if (strcasecmp(particle, "nu") == 0)
        index0 = PART_NU;
    else if (strcasecmp(particle, "b") == 0)
        index0 = PART_B;
    else if (strcasecmp(particle, "wb") == 0)
        index0 = PART_WB;
    else if (strcasecmp(particle, "mc") == 0)
        index0 = PART_MC;
    else
    {
        Tcl_AppendResult(interp, "bad particle \"", particle, "\"", NULL);
        return TCL_ERROR;
    }
    myfunction = functions[index0];
    for (i=0; i<11; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+2], a+i) != TCL_OK)
            return TCL_ERROR;
    nsols = myfunction(a[0], a[1], a[2], a[3], a[4], a[5],
                       a[6], a[7], a[8], a[9], a[10],
                       debug_level, 0, solutions);
    for (i=0; i<nsols; ++i)
    {
        double *fudge = (double *)(&solutions[i]);
        int j;

        results[i] = Tcl_NewListObj(0, NULL);
        for (j=0; j<11; ++j)
            Tcl_ListObjAppendElement(interp, results[i],
                                     Tcl_NewDoubleObj(fudge[j]));
        Tcl_ListObjAppendElement(interp, results[i],
                                 Tcl_NewIntObj(solutions[i].fail));
    }
    Tcl_SetObjResult(interp, Tcl_NewListObj(nsols, results));
    return TCL_OK;
}

static int c_set_debug_level(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    tcl_objc_range(1,2);
    if (objc > 1)
        if (Tcl_GetIntFromObj(interp, objv[1], &debug_level) != TCL_OK)
            return TCL_ERROR;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(debug_level));
    return TCL_OK;
}

static int c_w_mass_range(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* This command has 10 double arguments and a list of doubles */
    int i, listlen, ipoint=0;
    double a[10];
    double *initial_mw_points;
    Tcl_Obj **listObjElem;
    lepton_side_solution mwmin, mwmax;
    Tcl_Obj *result;

    tcl_require_objc(12);
    for (i=0; i<10; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], a+i) != TCL_OK)
            return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[11], 
			       &listlen, &listObjElem) != TCL_OK)
	return TCL_ERROR;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "empty list of trial W mass points",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }
    initial_mw_points = (double *)malloc(listlen*sizeof(double));
    for (i=0; i<listlen; ++i)
    {
        int j, point_is_duplicate = 0;
        if (Tcl_GetDoubleFromObj(interp, listObjElem[i],
                                 initial_mw_points+ipoint) != TCL_OK)
        {
            free(initial_mw_points);
            return TCL_ERROR;
        }
        if (initial_mw_points[ipoint] < 0.0)
        {
            Tcl_SetResult(interp, "can't use negative trial W mass",
                          TCL_VOLATILE);
            free(initial_mw_points);
            return TCL_ERROR;
        }
        for (j=0; j<ipoint; ++j)
            if (initial_mw_points[j] == initial_mw_points[ipoint])
            {
                point_is_duplicate = 1;
                break;
            }
        if (!point_is_duplicate)
            ++ipoint;
    }
    qsort(initial_mw_points, listlen, sizeof(double),
          (int (*)(const void *, const void *))compare_double);
    i = w_mass_range(a[0], a[1], a[2], a[3], a[4],
                     a[5], a[6], a[7], a[8], a[9],
                     debug_level, initial_mw_points, ipoint,
                     &mwmin, &mwmax);
    free(initial_mw_points);
    result = Tcl_NewListObj(0, NULL);
    if (i)
    {
        {
            lepton_side_solution *thisext = &mwmin;
            double *fudge = (double *)thisext;
            Tcl_Obj *relem = Tcl_NewListObj(0, NULL);
 
            Tcl_ListObjAppendElement(interp, relem, Tcl_NewIntObj(-1));
            for (i=0; i<11; ++i)
                Tcl_ListObjAppendElement(interp, relem,
                                         Tcl_NewDoubleObj(fudge[i]));
            Tcl_ListObjAppendElement(interp, relem, Tcl_NewIntObj(thisext->fail));
            Tcl_ListObjAppendElement(interp, result, relem);
        }
        {
            lepton_side_solution *thisext = &mwmax;
            double *fudge = (double *)thisext;
            Tcl_Obj *relem = Tcl_NewListObj(0, NULL);

            Tcl_ListObjAppendElement(interp, relem, Tcl_NewIntObj(1));
            for (i=0; i<11; ++i)
                Tcl_ListObjAppendElement(interp, relem,
                                         Tcl_NewDoubleObj(fudge[i]));
            Tcl_ListObjAppendElement(interp, relem, Tcl_NewIntObj(thisext->fail));
            Tcl_ListObjAppendElement(interp, result, relem);
        }
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_solve_hadronic_side(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* This command has 15 double arguments */
    int i;
    double a[15];
    hadron_side_solution solution;
    Tcl_Obj *result;

    tcl_require_objc(16);
    for (i=0; i<15; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], a+i) != TCL_OK)
            return TCL_ERROR;
    i = solve_hadronic_side(a[0], a[1], a[2], a[3], a[4], a[5], a[6],
                            a[7], a[8], a[9], a[10], a[11], a[12],
                            a[13], a[14], debug_level, &solution);
    result = Tcl_NewListObj(0, NULL);
    if (i)
    {
        /* Kids, don't do this at home */
        double *fudge = (double *)(&solution);
	assert(solution.is_valid);
        for (i=0; i<15; ++i)
            Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewDoubleObj(fudge[i]));
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_nuz_local_peak(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* This command has 12 double arguments */
    int i;
    double a[12];
    lepton_side_solution sol;
    Tcl_Obj *result;

    tcl_require_objc(13);
    for (i=0; i<12; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], a+i) != TCL_OK)
            return TCL_ERROR;

    if (a[10] <= 0.0)
    {
        Tcl_SetResult(interp, "initial pb must be positive", TCL_VOLATILE);
        return TCL_ERROR;
    }
    i = nuz_local_peak(a[0], a[1], a[2], a[3], a[4], a[5], a[6],
                       a[7], a[8], a[9], debug_level, a[10], a[11], &sol);
    result = Tcl_NewListObj(0, NULL);
    if (i)
    {
        double *fudge = (double *)(&sol);
        Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(i));
        for (i=0; i<10; ++i)
            Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewDoubleObj(fudge[i]));
        Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(sol.fail));
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_nuz_minmax(
    ClientData clientData,Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* This command has 13 double arguments */
    int i;
    double a[13];
    lepton_side_solution sol;
    Tcl_Obj *result;

    tcl_require_objc(14);
    for (i=0; i<13; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], a+i) != TCL_OK)
            return TCL_ERROR;
    if (a[11] == 0.0)
    {
        Tcl_SetResult(interp, "initial step can not be zero", TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (a[12] <= 0.0)
    {
        Tcl_SetResult(interp, "target precision must be positive", TCL_VOLATILE);
        return TCL_ERROR;
    }
    i = variable_step_nuz_minmax(a[0], a[1], a[2], a[3], a[4], a[5], a[6],
                                 a[7], a[8], a[9], debug_level, a[10],
                                 a[11], a[12], &sol);
    result = Tcl_NewListObj(0, NULL);
    if (i)
    {
        double *fudge = (double *)(&sol);
        Tcl_ListObjAppendElement(interp, result,
                                 Tcl_NewIntObj(a[11] < 0.0 ? -1 : 1));
        for (i=0; i<11; ++i)
            Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewDoubleObj(fudge[i]));
        Tcl_ListObjAppendElement(interp, result, Tcl_NewIntObj(sol.fail));
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

/*
static int c_drteq4(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int i;
    double coeffs[5];
    Tcl_Obj *result;

    tcl_require_objc(6);

    for (i=0; i<5; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], coeffs+i) != TCL_OK)
            return TCL_ERROR;
    if (coeffs[0] == 0.0)
    {
        Tcl_SetResult(interp, "leading equation coefficient "
                      "must not be 0", TCL_VOLATILE);
        return TCL_ERROR;
    }
    result = Tcl_NewListObj(0, NULL);

    {
        double quartic_roots[4], Z[8], DC = 0.0;
        int MT = 0, nroots = 0;
        double A = coeffs[1]/coeffs[0];
        double B = coeffs[2]/coeffs[0];
        double C = coeffs[3]/coeffs[0];
        double D = coeffs[4]/coeffs[0];

        drteq4_(&A, &B, &C, &D, Z, &DC, &MT);
        switch (MT)
        {
        case 1:
            // Four real roots
            quartic_roots[nroots] = Z[nroots*2];
            ++nroots;
            quartic_roots[nroots] = Z[nroots*2];
            ++nroots;
        case 3:
            // Two real roots
            quartic_roots[nroots] = Z[nroots*2];
            ++nroots;
            quartic_roots[nroots] = Z[nroots*2];
            ++nroots;
        case 2:
            // No real roots
            break;
        default:
            assert(0);
        }

        for (i=0; i<nroots; ++i)
            Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewDoubleObj(quartic_roots[i]));
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}
*/

static int c_cubic(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int i;
    double coeffs[4];
    Tcl_Obj *result;

    tcl_require_objc(5);

    for (i=0; i<4; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], coeffs+i) != TCL_OK)
            return TCL_ERROR;
    if (coeffs[0] == 0.0)
    {
        Tcl_SetResult(interp, "leading equation coefficient "
                      "must not be 0", TCL_VOLATILE);
        return TCL_ERROR;
    }
    result = Tcl_NewListObj(0, NULL);

    {
        double rts[4];
        double A = coeffs[1]/coeffs[0];
        double B = coeffs[2]/coeffs[0];
        double C = coeffs[3]/coeffs[0];
        int nroots = cubic(A, B, C, rts);

        for (i=0; i<nroots; ++i)
            Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewDoubleObj(rts[i]));
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_quartic(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int i;
    double coeffs[5];
    Tcl_Obj *result;

    tcl_require_objc(6);

    for (i=0; i<5; ++i)
        if (Tcl_GetDoubleFromObj(interp, objv[i+1], coeffs+i) != TCL_OK)
            return TCL_ERROR;
    if (coeffs[0] == 0.0)
    {
        Tcl_SetResult(interp, "leading equation coefficient "
                      "must not be 0", TCL_VOLATILE);
        return TCL_ERROR;
    }
    result = Tcl_NewListObj(0, NULL);

    {
        double rts[4];
        double A = coeffs[1]/coeffs[0];
        double B = coeffs[2]/coeffs[0];
        double C = coeffs[3]/coeffs[0];
        double D = coeffs[4]/coeffs[0];
        int nroots = quartic(A, B, C, D, rts);

        for (i=0; i<nroots; ++i)
            Tcl_ListObjAppendElement(interp, result,
                                     Tcl_NewDoubleObj(rts[i]));
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_convolute_bw(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    double hwhm;
    int i, listlen;
    Tcl_Obj **listObjElem;
    double *r;
    Tcl_Obj *result;

    tcl_require_objc(3);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &hwhm) != TCL_OK)
        return TCL_ERROR;
    if (hwhm <= 0.0)
    {
        Tcl_SetResult(interp, "width must be positive", TCL_VOLATILE);
        return TCL_ERROR;
    }

    if (Tcl_ListObjGetElements(interp, objv[2], 
                               &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen == 0)
    {
        Tcl_SetResult(interp, "empty list of top mass points",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }
    r = (double *)malloc(listlen*sizeof(double));
    if (r == NULL)
    {
        Tcl_SetResult(interp, "out of memory", TCL_VOLATILE);
        return TCL_ERROR;
    }
    for (i=0; i<listlen; ++i)
        if (Tcl_GetDoubleFromObj(interp, listObjElem[i], r+i) != TCL_OK)
        {
            free(r);
            return TCL_ERROR;
        }
    convolute_breit_wigner(r, listlen, hwhm);
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<listlen; ++i)
        Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(r[i]));
    free(r);
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_print_wgrid(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: print_wgrid n_intervals peak width coverage wtype min? max?
     *        print_wgrid filename
     *
     * "wtype" should be one of the letters r, s, or t. They stand for
     * rectangular, Simpson, or trapezoidal integration grid, respectively.
     */
    int n_intervals;
    double peak, width, coverage;
    char *cwtype;
    Wgrid_type wtype;
    wgrid g;
    double mWmin, mWmax;

    if (objc == 2)
    {
	if (read_w_grid(&g, Tcl_GetStringFromObj(objv[1], NULL)))
	    return TCL_ERROR;
    }
    else
    {
	tcl_objc_range(6,8);
	if (Tcl_GetIntFromObj(interp, objv[1], &n_intervals) != TCL_OK)
	    return TCL_ERROR;
	if (n_intervals <= 0)
	    goto fail;

	if (Tcl_GetDoubleFromObj(interp, objv[2], &peak) != TCL_OK)
	    return TCL_ERROR;

	if (Tcl_GetDoubleFromObj(interp, objv[3], &width) != TCL_OK)
	    return TCL_ERROR;
	if (width <= 0.0)
	    goto fail;

	if (Tcl_GetDoubleFromObj(interp, objv[4], &coverage) != TCL_OK)
	    return TCL_ERROR;
	if (coverage <= 0.0)
	    goto fail;

	cwtype = Tcl_GetStringFromObj(objv[5], NULL);
	if (strcasecmp(cwtype, "r") == 0)
	    wtype = WGRID_RECTANGULAR;
	else if (strcasecmp(cwtype, "t") == 0)
	    wtype = WGRID_TRAPEZOID;
	else if (strcasecmp(cwtype, "s") == 0)
	    wtype = WGRID_SIMPSON;
	else
	    goto fail;

	if (objc > 6)
	{
	    tcl_require_objc(8);
	    if (Tcl_GetDoubleFromObj(interp, objv[6], &mWmin) != TCL_OK)
		return TCL_ERROR;
	    if (mWmin <= 0.0)
		goto fail;
	    if (Tcl_GetDoubleFromObj(interp, objv[7], &mWmax) != TCL_OK)
		return TCL_ERROR;
	    if (mWmax <= mWmin)
		goto fail;
	}

	init_w_grid(&g, n_intervals, peak, width, coverage, wtype);
	if (objc > 6)
	    set_w_grid_edges(&g, mWmin, mWmax);
    }

    print_w_grid(&g, stdout);
    printf("\n");
    fflush(stdout);
    cleanup_w_grid(&g);

    return TCL_OK;

 fail:
    Tcl_SetResult(interp, "invalid argument value", TCL_STATIC);
    return TCL_ERROR;
}

static int c_set_tf_parameters(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_get_tf_parameters(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_bare_ratio_density(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_bare_ratio_efficiency(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_jet_scale_and_uncert_from_l5(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: jet_scale_and_uncert_from_l5 icorr ptJet eta
     */
    tcl_require_objc(4);

    int icorr;
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
	Tcl_SetResult(interp, "invalid corrector number", TCL_VOLATILE);
	return TCL_ERROR;
    }

    double ptJet, eta;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &ptJet) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &eta) != TCL_OK)
        return TCL_ERROR;

    const double pt0 = level0_pt(icorr, ptJet, 0.3f, eta);
    const double syserr = generic_correction_uncertainty(icorr, pt0, eta, uncert_mode);

    Tcl_Obj* result = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(ptJet/pt0));
    Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(syserr));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_transfer_function(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: transfer_function icorr ptParton ptJet eta isB partonMass deltaJES
     */
    double ptgen, ptJet, eta, phi = 0.5, px, py, pz;
    double bProb = 0.0, bFake = 0.0, hepgmass, deltaJES;
    int icorr, isB;
    jet_info jet;

    tcl_require_objc(8);
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
	Tcl_SetResult(interp, "invalid corrector number", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[2], &ptgen) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &ptJet) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &eta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[5], &isB) != TCL_OK)
        return TCL_ERROR;
    if (isB < 0 || isB > 2)
    {
	Tcl_SetResult(interp, "isB parameter is out of range", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[6], &hepgmass) != TCL_OK)
        return TCL_ERROR;
    if (hepgmass < 0.0)
    {
	Tcl_SetResult(interp, "mass can not be negative", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[7], &deltaJES) != TCL_OK)
        return TCL_ERROR;
    if (isB)
        bProb = 1.0;

    const double l5cut = get_jet_pt_cutoff();
    if (l5cut <= 0.0)
    {
	Tcl_SetResult(interp, "invalid jet Pt cutoff", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (ptJet < l5cut)
    {
	Tcl_SetResult(interp, "jet Pt is below cutoff", TCL_VOLATILE);
	return TCL_ERROR;
    }

    const double tCut = level0_pt(icorr, l5cut, 0.3f, eta);
    const double cuterr = generic_correction_uncertainty(icorr, tCut, eta, uncert_mode);

    const double pt0 = level0_pt(icorr, ptJet, 0.3f, eta);
    const double syserr = generic_correction_uncertainty(icorr, pt0, eta, uncert_mode);

    const double delta = ptJet*0.001;
    const double ptPlus0 = level0_pt(icorr, ptJet+delta, 0.3f, eta);
    const double ptMinus0 = level0_pt(icorr, ptJet-delta, 0.3f, eta);
    const double sysplus = generic_correction_uncertainty(icorr, ptPlus0, eta, uncert_mode);
    const double sysminus = generic_correction_uncertainty(icorr, ptMinus0, eta, uncert_mode);
    const double sysderi = (sysplus - sysminus)/2.0/delta;

    px = ptJet*cos(phi);
    py = ptJet*sin(phi);
    pz = ptJet*sinh(eta);

    fill_jet_info(&jet, px, py, pz, 0.0, bProb, bFake, eta,
                  syserr, sysderi, cuterr, 0, -1, 0);
    const double scale = 1.0 + deltaJES*syserr;
    double corr = deltaJES*sysderi*ptJet;
    double f = scale + corr;
    const double tf = f*transfer_function(
        particle(pt_eta_phi(ptgen, eta, phi), hepgmass), &jet, scale, isB);

    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(tf));
    return TCL_OK;
}

static int c_topwidth(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    double M_T, M_W, M_B;

    tcl_require_objc(4);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &M_T) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &M_W) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &M_B) != TCL_OK)
        return TCL_ERROR;
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(getTopWidth(M_T, M_W, M_B)));
    return TCL_OK;
}

static int c_jet_tf_predictor(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
#ifdef USE_ET_AS_TF_PREDICTOR
    Tcl_SetResult(interp, "Et", TCL_VOLATILE);
#else
    Tcl_SetResult(interp, "Pt", TCL_VOLATILE);
#endif
    return TCL_OK;
}

static int c_reset_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_uniform_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(uniform_random()));
    return TCL_OK;
}

static int c_normal_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(normal_random()));
    return TCL_OK;
}

static int c_johnson_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: johnson_random mean sigma skew kurt num_points? */
    double mean, sigma, skew, kurt, locat, width;
    int i, status, num_points = 1;
    double drand;
    double *rand = NULL;
    Tcl_Obj *result;

    tcl_objc_range(7, 8);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &mean) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &sigma) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &skew) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &kurt) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[5], &locat) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &width) != TCL_OK)
        return TCL_ERROR;
    if (objc > 7)
        if (Tcl_GetIntFromObj(interp, objv[7], &num_points) != TCL_OK)
            return TCL_ERROR;
    if (num_points < 0)
    {
        Tcl_SetResult(interp, "invalid arguments", TCL_VOLATILE);
        return TCL_ERROR;
    }
    else if (num_points == 0)
        return TCL_OK;
    else if (num_points == 1)
        rand = &drand;
    else
    {
        rand = (double *)malloc(num_points*sizeof(double));
        assert(rand);
    }
    status = johnson_random(mean, sigma, skew, kurt, locat,
			    width, rand, num_points);
    if (status)
    {
        if (num_points > 1)
            free(rand);
        Tcl_SetResult(interp, "invalid arguments", TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (num_points == 1)
        result = Tcl_NewDoubleObj(drand);
    else
    {
        result = Tcl_NewListObj(0, NULL);
        for (i=0; i<num_points; ++i)
            Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(rand[i]));
        free(rand);
    }
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_cleanup_n_d_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    tcl_require_objc(1);
    cleanup_n_d_random();
    return TCL_OK;
}

static int c_set_sobol_scrambling_multiplier(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    int i;

    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], &i) != TCL_OK)
        return TCL_ERROR;
    set_sobol_scrambling_multiplier((unsigned)i);
    return TCL_OK;
}

static int c_init_n_d_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: init_n_d_random method ndim param */
    int ndim, param;
    N_d_random_method randmeth;

    tcl_require_objc(4);
    randmeth = parse_n_d_random_method(Tcl_GetStringFromObj(objv[1], NULL));
    if (randmeth == N_D_RANDOM_INVALID)
    {
        Tcl_SetResult(interp, "invalid n-d random method", TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &ndim) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &param) != TCL_OK)
        return TCL_ERROR;
    if (ndim < 0)
    {
        Tcl_SetResult(interp, "number of dimensions can not be negative",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }
    init_n_d_random(randmeth, (unsigned)ndim, param);
    return TCL_OK;
}

static int c_next_n_d_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: next_n_d_random npoints */
    unsigned ndim;
    int i, npoints;
    float *data = NULL;

    tcl_require_objc(2);
    if (current_n_d_random_method() ==  N_D_RANDOM_INVALID)
    {
        Tcl_SetResult(interp, "please call init_n_d_random first",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }
    ndim = current_n_d_random_ndim();
    if (Tcl_GetIntFromObj(interp, objv[1], &npoints) != TCL_OK)
        return TCL_ERROR;
    if (npoints < 1)
    {
        Tcl_SetResult(interp, "must specify at least one point",
                      TCL_VOLATILE);
        return TCL_ERROR;
    }
    if (ndim > 0)
        if ((data = (float *)malloc(npoints*ndim*sizeof(float))) == NULL)
        {
            Tcl_SetResult(interp, "out of memory", TCL_VOLATILE);
            return TCL_ERROR;
        }
    for (i=0; i<npoints; ++i)
        next_n_d_random(data + i*ndim);
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((unsigned char *)data,
                                                 npoints*ndim*sizeof(float)));
    if (data)
        free(data);
    return TCL_OK;
}

static int c_integrated_angular_efficiency(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}


static int c_load_tfs_2015(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: load_tfs_2015 gssaFile minExceedance list_of_eta_bin_limits \
     *             partonMassSplit drStretchFactor jetPtCutoff interpolateParam
     */
    char* infile;
    double minExceedance, partonMassSplit, drStretch, jetPtCutoff;
    int i, listlen, interpolateParam;
    Tcl_Obj **listObjElem;
    double binLimits[64];

    tcl_require_objc(8);
    infile = Tcl_GetStringFromObj(objv[1], NULL);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &minExceedance) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_ListObjGetElements(interp, objv[3],
                               &listlen, &listObjElem) != TCL_OK)
        return TCL_ERROR;
    if (listlen > (int)(sizeof(binLimits)/sizeof(binLimits[0])))
    {
        Tcl_SetResult(interp, "too many bin limits", TCL_VOLATILE);
        return TCL_ERROR;
    }
    for (i=0; i<listlen; ++i)
        if (Tcl_GetDoubleFromObj(interp, listObjElem[i], &binLimits[i]) != TCL_OK)
            return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &partonMassSplit) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[5], &drStretch) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &jetPtCutoff) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[7], &interpolateParam) != TCL_OK)
        return TCL_ERROR;

    if (load_tfs_2015(infile, minExceedance, binLimits, listlen,
                      partonMassSplit, drStretch, jetPtCutoff, interpolateParam))
        return TCL_ERROR;
    else
        return TCL_OK;
}

static int c_bare_transfer_function(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: bare_transfer_function jetEta isB mparton partonPt \
     *                   dr ptRatio xCut
     *
     * xCut should be set to tf_pt_cutoff*jes_at_cut/partonPt
     */
    double jetEta;
    int isB;
    double mparton, partonPt, dr, ptRatio, xCut, tf;
    double x[2], predictors[2];

    tcl_require_objc(8);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &jetEta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[2], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &partonPt) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[5], &dr) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &ptRatio) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[7], &xCut) != TCL_OK)
        return TCL_ERROR;

    predictors[0] = mparton;
    predictors[1] = partonPt;
    x[0] = dr;
    x[1] = ptRatio;
    tf = bare_transfer_function(jetEta, isB, predictors, 2U, x, 2U, xCut);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(tf));
    return TCL_OK;
}

static int c_bare_ptratio_exceedance(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: bare_ptratio_exceedance jetEta isB mparton partonPt \
     *                   ptRatio xCut
     *
     * xCut should be set to tf_pt_cutoff*jes_at_cut/partonPt
     */
    double jetEta;
    int isB;
    double mparton, partonPt, ptRatio, xCut, tf;
    double predictors[2];

    tcl_require_objc(7);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &jetEta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[2], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &partonPt) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[5], &ptRatio) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &xCut) != TCL_OK)
        return TCL_ERROR;

    predictors[0] = mparton;
    predictors[1] = partonPt;
    tf = bare_ptratio_exceedance(jetEta, isB, predictors, 2U, ptRatio, xCut);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(tf));
    return TCL_OK;
}

static int c_bare_tf_random(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: bare_tf_random mparton pt eta phi isB xcut maxtries */
    const int ngen = 3;
    int i, isB, maxtries;
    double mparton, pt, eta, phi, xcut;
    double predictors[2], generated[ngen];
    Tcl_Obj *result;

    tcl_require_objc(8);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &mparton) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &pt) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &eta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &phi) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[5], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &xcut) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[7], &maxtries) != TCL_OK)
        return TCL_ERROR;

    predictors[0] = mparton;
    predictors[1] = pt;
    if (bare_tf_random(eta, isB, xcut,
                       predictors, sizeof(predictors)/sizeof(predictors[0]),
                       (void*)0, maxtries, generated, generated+1, generated+2))
    {
        Tcl_SetResult(interp, "maximum number of tries exceeded", TCL_VOLATILE);
        return TCL_ERROR;
    }
    result = Tcl_NewListObj(0, NULL);
    for (i=0; i<ngen; ++i)
        Tcl_ListObjAppendElement(interp, result, Tcl_NewDoubleObj(generated[i]));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int c_randomize_sampling_fromrand(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: randomize_sampling_fromrand id mparton pt eta phi isB nfills id2? */
    int i, id, isB, nfills, id2 = -1;
    double mparton, pt, eta, phi, density;
    v3_obj rnd_mom, parton_mom;
    double* pd = 0;

    tcl_objc_range(8,9);
    verify_2d_histo(id, 1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &mparton) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &pt) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &eta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[5], &phi) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[6], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[7], &nfills) != TCL_OK)
        return TCL_ERROR;
    if (objc > 8)
        verify_2d_histo(id2, 8);

    parton_mom = pt_eta_phi(pt, eta, phi);
    if (id2 > 0)
        pd = &density;
    for (i=0; i<nfills; ++i)
    {
        rnd_mom = randomize_sampling_fromrand(parton_mom, isB, mparton,
                                              uniform_random(), uniform_random(), pd);
        {
            const float neweta = Eta(rnd_mom);
            const float newphi = Phi(rnd_mom);
            hs_fill_2d_hist(id, neweta, newphi, 1.f);
            if (id2 > 0)
            {
                /* hs_fill_2d_hist(id2, neweta, newphi, density); */
                int nx, ny, bx, by;
                float xmin, xmax, ymin, ymax, bwx, bwy;

                hs_2d_hist_num_bins(id2, &nx, &ny);
                hs_2d_hist_range(id2, &xmin, &xmax, &ymin, &ymax);
                bwx = (xmax - xmin)/nx;
                bwy = (ymax - ymin)/ny;
                bx = floor((neweta - xmin)/bwx);
                by = floor((newphi - ymin)/bwy);
                if (bx >= 0 && bx < nx && by >= 0 && by < ny)
                    hs_2d_hist_set_bin(id2, bx, by, density);
            }
        }
    }

    return TCL_OK;
}

static int c_single_parton_eff(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: single_parton_eff mparton pt eta phi isB deltaJES sigmaAtCut */
    int isB;
    double mparton, pt, eta, phi, deltaJES, sigmaAtCut, eff;

    tcl_require_objc(8);
    if (Tcl_GetDoubleFromObj(interp, objv[1], &mparton) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &pt) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &eta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &phi) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetBooleanFromObj(interp, objv[5], &isB) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &deltaJES) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[7], &sigmaAtCut) != TCL_OK)
        return TCL_ERROR;

    eff = single_parton_eff(particle(pt_eta_phi(pt, eta, phi), mparton),
                            isB, deltaJES, sigmaAtCut);
    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(eff));
    return TCL_OK;
}

static int c_uniform_random_setseed(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    unsigned seed;

    /* Usage: uniform_random_setseed seed */
    tcl_require_objc(2);
    if (Tcl_GetIntFromObj(interp, objv[1], (int *)(&seed)) != TCL_OK)
        return TCL_ERROR;
    seed = uniform_random_setseed(seed);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(*((int *)(&seed))));
    return TCL_OK;
}

#ifdef HISTO_TF_PLOT
static const Tf_interpolator *make_tf_interpolator_from_3d_histo(
    const int id_from, double (*transform)(double),
    double (*convert)(double),
    const float f_low, const float f_high)
{
    int hstype, nxbins, nybins, nzbins, nall;
    float xmin, xmax, ymin, ymax, zmin, zmax, xbw, ybw, zbw;
    float *data;
    const Tf_interpolator *tf = NULL;

    hstype = hs_type(id_from);
    assert(hstype == HS_3D_HISTOGRAM);

    hs_3d_hist_num_bins(id_from, &nxbins, &nybins, &nzbins);
    hs_3d_hist_range(id_from, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    xbw = (xmax - xmin)/nxbins;
    ybw = (ymax - ymin)/nybins;
    zbw = (zmax - zmin)/nzbins;
    nall = nxbins*nybins*nzbins;
    data = (float *)malloc(nall*sizeof(float));
    assert(data);
    hs_3d_hist_bin_contents(id_from, data);
    if (transform)
    {
        int i;
        for (i=0; i<nall; ++i)
            data[i] = transform(data[i]);
    }
    tf = create_tf_interpolator(convert, data, nxbins, nybins, nzbins,
                                xmin + xbw/2.0, xmax - xbw/2.0,
                                ymin + ybw/2.0, ymax - ybw/2.0,
                                zmin + zbw/2.0, zmax - zbw/2.0,
                                f_low, f_high);
    assert(tf);

    return tf;
}

static int c_linear_interpolate_1d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: linear_interpolate_1d histo_id x */
    static Interpolator_data_1d d = {0, 0, 0, 0};
    static int old_histo_id = 0;

    int histo_id;
    double x;

    tcl_require_objc(3);
    if (Tcl_GetIntFromObj(interp, objv[1], &histo_id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
        return TCL_ERROR;
    if (histo_id == 0)
    {
	old_histo_id = 0;
	return TCL_OK;
    }
    if (histo_id != old_histo_id)
    {
	int nxbins;
	float xmin, xmax, dx;

	if (hs_type(histo_id) != HS_1D_HISTOGRAM)
	{
	    if (hs_type(histo_id) == HS_NONE)
		Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
				 " is not a valid Histo-Scope id", NULL);
	    else
		Tcl_AppendResult(interp, "item with id ",
				 Tcl_GetStringFromObj(objv[1], NULL),
				 " is not a 1d histogram", NULL);
	    return TCL_ERROR;
	}
	nxbins = hs_1d_hist_num_bins(histo_id);
	if (nxbins < 2)
	{
	    Tcl_SetResult(interp, "must have at least 2 bins",
			  TCL_VOLATILE);
	    return TCL_ERROR;
	}
	hs_1d_hist_range(histo_id, &xmin, &xmax);
	dx = (xmax - xmin)/nxbins;

	d.nxpoints = nxbins;
	d.xmin = xmin + dx/2.0;
	d.xmax = xmax - dx/2.0;
	d.data = (float *)realloc((float *)d.data, nxbins*sizeof(float));
	assert(d.data);
	hs_1d_hist_bin_contents(histo_id, (float *)d.data);

	old_histo_id = histo_id;
    }

    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(linear_interpolate_1d(&d, x)));
    return TCL_OK;
}

static int c_linear_interpolate_2d(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: linear_interpolate_2d histo_id x y */
    static Interpolator_data_2d d = {0, 0, 0, 0, 0, 0, 0};
    static int old_histo_id = 0;

    int histo_id;
    double x, y;

    tcl_require_objc(4);
    if (Tcl_GetIntFromObj(interp, objv[1], &histo_id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[2], &x) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &y) != TCL_OK)
        return TCL_ERROR;
    if (histo_id == 0)
    {
	old_histo_id = 0;
	return TCL_OK;
    }
    if (histo_id != old_histo_id)
    {
	int nxbins, nybins;
	float xmin, xmax, ymin, ymax, dx, dy;

	if (hs_type(histo_id) != HS_2D_HISTOGRAM)
	{
	    if (hs_type(histo_id) == HS_NONE)
		Tcl_AppendResult(interp, Tcl_GetStringFromObj(objv[1], NULL),
				 " is not a valid Histo-Scope id", NULL);
	    else
		Tcl_AppendResult(interp, "item with id ",
				 Tcl_GetStringFromObj(objv[1], NULL),
				 " is not a 2d histogram", NULL);
	    return TCL_ERROR;
	}
	hs_2d_hist_num_bins(histo_id, &nxbins, &nybins);
	if (nxbins < 2 || nybins < 2)
	{
	    Tcl_SetResult(interp, "must have at least 2 bins in each axis",
			  TCL_VOLATILE);
	    return TCL_ERROR;
	}
	hs_2d_hist_range(histo_id, &xmin, &xmax, &ymin, &ymax);
	dx = (xmax - xmin)/nxbins;
	dy = (ymax - ymin)/nybins;

	d.nxpoints = nxbins;
	d.nypoints = nybins;
	d.xmin = xmin + dx/2.0;
	d.xmax = xmax - dx/2.0;
	d.ymin = ymin + dy/2.0;
	d.ymax = ymax - dy/2.0;
	d.data = (float *)realloc((float *)d.data,
				  nxbins*nybins*sizeof(float));
	assert(d.data);
	hs_2d_hist_bin_contents(histo_id, (float *)d.data);

	old_histo_id = histo_id;
    }

    Tcl_SetObjResult(interp, Tcl_NewDoubleObj(linear_interpolate_2d(&d, x, y)));
    return TCL_OK;
}

static int c_angular_tf_random_fill(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_generic_correction_uncertainty_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: generic_correction_uncertainty_scan icorr id
     *
     * id should refer to a 2d histogram with eta on x axis and pt on y axis
     */
    tcl_require_objc(3);

    int icorr;
    if (Tcl_GetIntFromObj(interp, objv[1], &icorr) != TCL_OK)
        return TCL_ERROR;
    if (!corrector_valid(icorr))
    {
	Tcl_SetResult(interp, "invalid corrector number", TCL_VOLATILE);
	return TCL_ERROR;
    }

    int id;
    verify_2d_histo(id, 2);

    float etamin, etamax, ptmin, ptmax;
    hs_2d_hist_range(id, &etamin, &etamax, &ptmin, &ptmax);

    int netabins, nptbins;
    hs_2d_hist_num_bins(id, &netabins, &nptbins);

    const float dx = (etamax - etamin)/netabins;
    const float dy = (ptmax - ptmin)/nptbins;

    int ix = 0;
    for (; ix<netabins; ++ix)
    {
        const float eta = etamin + (ix + 0.5f)*dx;
        int iy = 0;
        for (; iy<nptbins; ++iy)
        {
            const float pt = ptmin + (iy + 0.5f)*dy;
            const float value = generic_correction_uncertainty(icorr, pt, eta, uncert_mode);
            hs_2d_hist_set_bin(id, ix, iy, value);
        }
    }

    return TCL_OK;
}

static int c_angular_tf_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: angular_tf_scan id pt eta mparton isB
     *
     * id should refer to a 2d histogram
     */
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_OK;
}

static int c_mc_eff_prob_to_loose_parton_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: mc_eff_prob_to_loose_parton_scan id pt_cut mparton eta isB 
     *             max_efficiency jes_sigma_factor
     * 
     * id must refer to a 2d histo. On the x axis it should have
     * the values for parton pt, the y axis represents JES sigma.
     */
    int i, j, id, isB, nxbins, nybins;
    double pt_cut, mparton, eta, max_efficiency, jes_sigma_factor;
    float xmin, xmax, ymin, ymax;
    double xbw, ybw;

    tcl_require_objc(8);
    verify_2d_histo(id, 1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &pt_cut) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
        return TCL_ERROR;
    if (mparton < 0.0)
    {
	Tcl_SetResult(interp, "mparton parameter is out of range", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[4], &eta) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[5], &isB) != TCL_OK)
        return TCL_ERROR;
    if (isB < 0 || isB > 2)
    {
	Tcl_SetResult(interp, "isB parameter is out of range", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (Tcl_GetDoubleFromObj(interp, objv[6], &max_efficiency) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[7], &jes_sigma_factor) != TCL_OK)
        return TCL_ERROR;

    set_params_to_loose_parton(pt_cut, 2.0, max_efficiency, jes_sigma_factor);

    hs_2d_hist_num_bins(id, &nxbins, &nybins);
    hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
    xbw = (double)(xmax - xmin)/nxbins;
    ybw = (double)(ymax - ymin)/nybins;
    for (i=0; i<nxbins; ++i)
    {
        const double pt = xmin + xbw/2.0 + i*xbw;
        const particle_obj parton = particle(pt_eta_phi(pt, eta, 0.0), mparton);
        for (j=0; j<nybins; ++j)
        {
            const double jes_sigma = ymin + ybw/2.0 + j*ybw;
            const double prob = mc_eff_prob_to_loose_parton(&parton, jes_sigma, isB);
            hs_2d_hist_set_bin(id, i, j, (float)prob);
        }
    }
    return TCL_OK;
}

static int c_transfer_function_invcdf_scan_2(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_transfer_function_scan(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: transfer_function_scan id et mparton etaDet isB jes
     *
     * If id refers to 3d histo then et and mparton arguments
     * are ignored. Et is taken from the x axis of the 3d histogram
     * and mparton is taken from the y axis. If id refers to 2d
     * histo then et argument is ignored.
     */
    int i, j, k, nxbins, nybins, nzbins, id, isB;
    jet_info jet;
    double p, E, et, pt = 0.0, etaDet, sinhEtaDet, coshEtaDet;
    double ptrat, xbw, ybw, zbw, px, py = 0.0, pz;
    float xmin, xmax, ymin, ymax, zmin, zmax, fval;
    double bProb = 0.0, bFake = 0.0, mparton, jes;

    tcl_require_objc(7);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &etaDet) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[5], &isB) != TCL_OK)
        return TCL_ERROR;
    if (isB < 0 || isB > 2)
    {
	Tcl_SetResult(interp, "isB parameter is out of range", TCL_VOLATILE);
	return TCL_ERROR;
    }
    if (isB)
        bProb = 1.0;
    if (Tcl_GetDoubleFromObj(interp, objv[6], &jes) != TCL_OK)
        return TCL_ERROR;
    if (jes < 0.5 || jes > 2.0)
    {
	Tcl_SetResult(interp, "jes parameter is out of range", TCL_VOLATILE);
	return TCL_ERROR;
    }
    sinhEtaDet = sinh(etaDet);
    coshEtaDet = cosh(etaDet);

    switch (hs_type(id))
    {
    case HS_1D_HISTOGRAM:
        if (Tcl_GetDoubleFromObj(interp, objv[2], &et) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
            return TCL_ERROR;
        pt = et*et - mparton*mparton/coshEtaDet/coshEtaDet;
        if (pt > 0.0)
            pt = sqrt(pt);
        else
            pt = 0.0;
        nxbins = hs_1d_hist_num_bins(id);
        hs_1d_hist_range(id, &xmin, &xmax);
        xbw = (double)(xmax - xmin)/nxbins;
        px = pt;
        pz = pt*sinhEtaDet;
        p = hypot3(px, py, pz);
        E = hypot(p, mparton);
        if (p == 0.0)
            for (i=0; i<nxbins; ++i)
                hs_1d_hist_set_bin(id, i, 0.f);
        else
            for (i=0; i<nxbins; ++i)
            {
                ptrat = (xmin + xbw/2.0 + i*xbw)*E/p;
                fill_jet_info(&jet, ptrat*px, ptrat*py, ptrat*pz,
                              0.0, bProb, bFake, etaDet,
                              0.0, 0.0, 0.0, 0, -1, 0);
                scale_jet(&jet, jes, &jet);
                fval = jes*transfer_function(particle(v3(px, py, pz), mparton),
                                             &jet, jes, isB);
                hs_1d_hist_set_bin(id, i, (float)(E*fval));
            }
        break;

    case HS_2D_HISTOGRAM:
        if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
            return TCL_ERROR;
        hs_2d_hist_num_bins(id, &nxbins, &nybins);
        hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
        xbw = (double)(xmax - xmin)/nxbins;
        ybw = (double)(ymax - ymin)/nybins;
        for (i=0; i<nxbins; ++i)
        {
            et = xmin + xbw/2.0 + i*xbw;
            pt = et*et - mparton*mparton/coshEtaDet/coshEtaDet;
            if (pt > 0.0)
                pt = sqrt(pt);
            else
                pt = 0.0;
            px = pt;
            pz = pt*sinhEtaDet;
            p = hypot3(px, py, pz);
            E = hypot(p, mparton);
            if (p == 0.0)
                for (j=0; j<nybins; ++j)
                    hs_2d_hist_set_bin(id, i, j, 0.f);
            else
                for (j=0; j<nybins; ++j)
                {
                    ptrat = (ymin + ybw/2.0 + j*ybw)*E/p;
                    fill_jet_info(&jet, ptrat*px, ptrat*py, ptrat*pz,
                                  0.0, bProb, bFake, etaDet,
                                  0.0, 0.0, 0.0, 0, -1, 0);
                    scale_jet(&jet, jes, &jet);
                    fval = jes*transfer_function(particle(v3(px, py, pz), mparton),
                                                 &jet, jes, isB);
                    hs_2d_hist_set_bin(id, i, j, (float)(E*fval));
                }
        }
        break;

    case HS_3D_HISTOGRAM:
        hs_3d_hist_num_bins(id, &nxbins, &nybins, &nzbins);
        hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        xbw = (double)(xmax - xmin)/nxbins;
        ybw = (double)(ymax - ymin)/nybins;
        zbw = (double)(zmax - zmin)/nzbins;
        for (j=0; j<nybins; ++j)
        {
            mparton = ymin + ybw/2.0 + j*ybw;
            for (i=0; i<nxbins; ++i)
            {
                et = xmin + xbw/2.0 + i*xbw;
                pt = et*et - mparton*mparton/coshEtaDet/coshEtaDet;
                if (pt > 0.0)
                    pt = sqrt(pt);
                else
                    pt = 0.0;
                px = pt;
                pz = pt*sinhEtaDet;
                p = hypot3(px, py, pz);
                if (p == 0.0)
                    for (k=0; k<nzbins; ++k)
                        hs_3d_hist_set_bin(id, i, j, k, 0.f);
                else
                {
                    E = hypot(p, mparton);
                    for (k=0; k<nzbins; ++k)
                    {
                        ptrat = (zmin + zbw/2.0 + k*zbw)*E/p;
                        fill_jet_info(&jet, ptrat*px, ptrat*py, ptrat*pz,
                                      0.0, bProb, bFake, etaDet,
                                      0.0, 0.0, 0.0, 0, -1, 0);
                        scale_jet(&jet, jes, &jet);
                        fval = jes*transfer_function(particle(v3(px, py, pz), mparton),
                                                     &jet, jes, isB);
                        hs_3d_hist_set_bin(id, i, j, k, (float)(E*fval));
                    }
                }
            }
        }
        break;

    default:
        Tcl_AppendResult(interp, "item with id ",
                         Tcl_GetStringFromObj(objv[1],NULL),
                         " is not a histogram", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

static int c_reset_impossible_tf_bins(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: reset_impossible_tf_bins id etaDet */
    int i, j, k, id, nxbins, nybins, nzbins;
    double etaDet, et, mparton, coshEtaDet;
    float xmin, xmax, ymin, ymax, zmin, zmax, xbw, ybw;

    tcl_require_objc(3);
    verify_3d_histo(id, 1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &etaDet) != TCL_OK)
        return TCL_ERROR;
    coshEtaDet = cosh(etaDet);

    hs_3d_hist_num_bins(id, &nxbins, &nybins, &nzbins);
    hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    xbw = (double)(xmax - xmin)/nxbins;
    ybw = (double)(ymax - ymin)/nybins;
    for (j=0; j<nybins; ++j)
    {
        mparton = ymin + ybw/2.0 + j*ybw;
        for (i=0; i<nxbins; ++i)
        {
            et = xmin + xbw/2.0 + i*xbw;
            if (et*et - mparton*mparton/coshEtaDet/coshEtaDet <= 0.0)
                for (k=0; k<nzbins; ++k)
                    hs_3d_hist_set_bin(id, i, j, k, 0.f);
        }
    }

    return TCL_OK;
}

static int c_transfer_function_efficiency_scan_2(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: transfer_function_efficiency_scan_2 id et mparton etaDet isB
     *
     * If id refers to 3d histo then et and mparton arguments
     * are ignored. Et is taken from the x axis of the 3d histogram
     * and mparton is taken from the y axis. If id refers to 2d
     * histo then et argument is ignored.
     */
    int i, j, k, nxbins, nybins, nzbins, id, isB;
    double p, pt = 0.0, et, etaDet, sinhEtaDet, coshEtaDet, ptrat;
    double xbw, ybw, zbw, px, py = 0.0, pz, mparton;
    float xmin, xmax, ymin, ymax, zmin, zmax, fval;

    tcl_require_objc(6);
    if (Tcl_GetIntFromObj(interp, objv[1], &id) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &etaDet) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[5], &isB) != TCL_OK)
        return TCL_ERROR;
    if (isB < 0 || isB > 2)
    {
	Tcl_SetResult(interp, "isB parameter is out of range", TCL_VOLATILE);
	return TCL_ERROR;
    }
    sinhEtaDet = sinh(etaDet);
    coshEtaDet = cosh(etaDet);

    switch (hs_type(id))
    {
    case HS_1D_HISTOGRAM:
        if (Tcl_GetDoubleFromObj(interp, objv[2], &et) != TCL_OK)
            return TCL_ERROR;
        if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
            return TCL_ERROR;
        pt = et*et - mparton*mparton/coshEtaDet/coshEtaDet;
        if (pt > 0.0)
            pt = sqrt(pt);
        else
            pt = 0.0;
        nxbins = hs_1d_hist_num_bins(id);
        hs_1d_hist_range(id, &xmin, &xmax);
        xbw = (double)(xmax - xmin)/nxbins;
        px = pt;
        pz = pt*sinhEtaDet;
        p = hypot3(px, py, pz);
        if (p == 0.0)
            for (i=0; i<nxbins; ++i)
                hs_1d_hist_set_bin(id, i, 0.f);
        else
            for (i=0; i<nxbins; ++i)
            {
                ptrat = xmin + xbw/2.0 + i*xbw;
                fval = transfer_function_efficiency_2(particle(v3(px, py, pz), mparton),
                                                      etaDet, isB, et*ptrat);
                hs_1d_hist_set_bin(id, i, (float)fval);
            }
        break;

    case HS_2D_HISTOGRAM:
        if (Tcl_GetDoubleFromObj(interp, objv[3], &mparton) != TCL_OK)
            return TCL_ERROR;
        hs_2d_hist_num_bins(id, &nxbins, &nybins);
        hs_2d_hist_range(id, &xmin, &xmax, &ymin, &ymax);
        xbw = (double)(xmax - xmin)/nxbins;
        ybw = (double)(ymax - ymin)/nybins;
        for (i=0; i<nxbins; ++i)
        {
            et = xmin + xbw/2.0 + i*xbw;
            pt = et*et - mparton*mparton/coshEtaDet/coshEtaDet;
            if (pt > 0.0)
                pt = sqrt(pt);
            else
                pt = 0.0;
            px = pt;
            pz = pt*sinhEtaDet;
            p = hypot3(px, py, pz);
            if (p == 0.0)
                for (j=0; j<nybins; ++j)
                    hs_2d_hist_set_bin(id, i, j, 0.f);
            else
                for (j=0; j<nybins; ++j)
                {
                    ptrat = ymin + ybw/2.0 + j*ybw;
                    fval = transfer_function_efficiency_2(
                        particle(v3(px, py, pz), mparton),
                        etaDet, isB, et*ptrat);
                    hs_2d_hist_set_bin(id, i, j, (float)fval);
                }
        }
        break;
        
    case HS_3D_HISTOGRAM:
        hs_3d_hist_num_bins(id, &nxbins, &nybins, &nzbins);
        hs_3d_hist_range(id, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
        xbw = (double)(xmax - xmin)/nxbins;
        ybw = (double)(ymax - ymin)/nybins;
        zbw = (double)(zmax - zmin)/nzbins;
        for (j=0; j<nybins; ++j)
        {
            mparton = ymin + ybw/2.0 + j*ybw;
            for (i=0; i<nxbins; ++i)
            {
                et = xmin + xbw/2.0 + i*xbw;
                pt = et*et - mparton*mparton/coshEtaDet/coshEtaDet;
                if (pt > 0.0)
                    pt = sqrt(pt);
                else
                    pt = 0.0;
                px = pt;
                pz = pt*sinhEtaDet;
                p = hypot3(px, py, pz);
                if (p == 0.0)
                    for (k=0; k<nzbins; ++k)
                        hs_3d_hist_set_bin(id, i, j, k, 0.f);
                else
                {
                    for (k=0; k<nzbins; ++k)
                    {
                        ptrat = zmin + zbw/2.0 + k*zbw;
                        fval = transfer_function_efficiency_2(
                            particle(v3(px, py, pz), mparton),
                            etaDet, isB, et*ptrat);
                        hs_3d_hist_set_bin(id, i, j, k, (float)fval);
                    }
                }
            }
        }
        break;

    default:
        Tcl_AppendResult(interp, "item with id ",
                         Tcl_GetStringFromObj(objv[1],NULL),
                         " is not a histogram", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

static int c_bw_multipole_coeffs(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: bw_multipole_coeffs histo_id peak hwhm */
    const int maxpower = MAX_BW_POLY_DEGREE - 1;
    int i, ipow, histo_id, nbins;
    float xmin, xmax;
    float *hcontents = NULL;
    double peak, hwhm, binwidth, dmin, sum[maxpower];
    Tcl_Obj *results[maxpower];

    tcl_require_objc(4);
    verify_1d_histo(histo_id, 1);
    if (Tcl_GetDoubleFromObj(interp, objv[2], &peak) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &hwhm) != TCL_OK)
        return TCL_ERROR;
    if (hwhm <= 0.0)
    {
        Tcl_SetResult(interp, "hwhm must be positive", TCL_VOLATILE);
        return TCL_ERROR;
    }

    nbins = hs_1d_hist_num_bins(histo_id);
    hs_1d_hist_range(histo_id, &xmin, &xmax);

    hcontents = (float *)malloc(nbins*sizeof(float));
    assert(hcontents);
    hs_1d_hist_bin_contents(histo_id, hcontents);

    memset(sum, 0, maxpower*sizeof(double));
    binwidth = (double)(xmax - xmin)/nbins;
    dmin = xmin + binwidth/2.0;
    for (i=0; i<nbins; ++i)
        if (hcontents[i] != 0.f)
        {
            const double weight = binwidth*hcontents[i]/hwhm/PI;
            const double bincenter = (dmin + i*binwidth - peak)/hwhm;
            for (ipow=0; ipow<maxpower; ++ipow)
                sum[ipow] += weight*bw_polynomial(bincenter, ipow+2);
        }
    free(hcontents);

    for (ipow=0; ipow<maxpower; ++ipow)
    {
        results[ipow] = Tcl_NewDoubleObj(sum[ipow]);
        assert(results[ipow]);
    }
    Tcl_SetObjResult(interp, Tcl_NewListObj(maxpower, results));
    return TCL_OK;
}

static int c_tf_interpolate(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    /* Usage: tf_interpolate id_3d_from id_3d_to f_low f_high */
    int id_from, id_to, nxbins, nybins, nzbins, i, j, k;
    float xmin, xmax, ymin, ymax, zmin, zmax, xbw, ybw, zbw;
    float x, y, z, f;
    double f_low, f_high;
    const Tf_interpolator *tf = NULL;

    tcl_require_objc(5);
    verify_3d_histo(id_from, 1);
    verify_3d_histo(id_to, 2);
    if (Tcl_GetDoubleFromObj(interp, objv[3], &f_low) != TCL_OK)
        return TCL_ERROR;
    if (Tcl_GetDoubleFromObj(interp, objv[4], &f_high) != TCL_OK)
        return TCL_ERROR;

    /* Create the interpolator */
    tf = make_tf_interpolator_from_3d_histo(id_from, NULL, NULL, f_low, f_high);

    /* Scan the output histogram */
    hs_reset(id_to);
    hs_3d_hist_num_bins(id_to, &nxbins, &nybins, &nzbins);
    hs_3d_hist_range(id_to, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    xbw = (xmax - xmin)/nxbins;
    ybw = (ymax - ymin)/nybins;
    zbw = (zmax - zmin)/nzbins;
    for (i=0; i<nxbins; ++i)
    {
        x = xmin + xbw/2.0 + i*xbw;
        for (j=0; j<nybins; ++j)
        {
            y = ymin + ybw/2.0 + j*ybw;
            for (k=0; k<nzbins; ++k)
            {
                z = zmin + zbw/2.0 + k*zbw;
                f = tf_interpolate(tf, x, y, z);
                hs_fill_3d_hist(id_to, x, y, z, f);
            }
        }
    }

    destroy_tf_interpolator(tf);
    return TCL_OK;
}

#define reset_q_interp do {\
    for (i=0; i<2; ++i)\
        for (j=0; j<4; ++j)\
            q_interp[i][j] = 0;\
} while (0);

static int c_load_tf_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int get_nd_interpolator_from_cmd(Tcl_Interp *interp, Tcl_Obj *obj,
                                        Interpolator_data_nd **ptr)
{
    Tcl_CmdInfo cmdinfo;
    char *cmd = Tcl_GetStringFromObj(obj, NULL);
    if (strncmp(cmd, "linear_interp_nd_", 17))
    {
        Tcl_AppendResult(interp, "invalid linear_interpolator_nd handle \"",
                         cmd, "\"", NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetCommandInfo(interp, cmd, &cmdinfo) != 1)
        return TCL_ERROR;
    *ptr = (Interpolator_data_nd *)cmdinfo.objClientData;
    assert(*ptr);
    return TCL_OK;
}

static int c_set_angular_eff_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_set_parton_eff_interpolators(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Tcl_SetResult(interp, obsolete, TCL_STATIC);
    return TCL_ERROR;
}

static int c_set_qqbar_deltaR_interpolator(
    ClientData clientData, Tcl_Interp *interp,
    int objc, Tcl_Obj *CONST objv[])
{
    Interpolator_data_nd *in;

    tcl_require_objc(2);
    if (get_nd_interpolator_from_cmd(interp, objv[1], &in) != TCL_OK)
        return TCL_ERROR;
    set_qqbar_deltaR_interpolator(in);
    return TCL_OK;
}

#endif /* HISTO_TF_PLOT */

#ifdef __cplusplus
}
#endif
