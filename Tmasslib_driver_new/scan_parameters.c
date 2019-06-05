#include <limits.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "parameter_parser.h"
#include "topmass_utils.h"
#include "topmass_norm.h"
#include "random_jet_angles.h"
#include "ttbar_phase_space.h"
#include "mc_topmass_integrator.h"
#include "single_jet_probs.h"

/* Scan parameters */
static double parton_syserr_pt_cutoff;
static double sample_pole_mass;
static double prob_factor;
static int discard_excluded_jet;
static int topscan_points;
static int n_jes_points;
static int maxevents;
static int skipevents;
static int script_period;
static int fill_result_ntuple;
static char *category;
static char *periodic_script;

/* Static variables which persist between events */
static double *top_mass_values = NULL;
static unsigned n_top_mass_values = 0;
static double *jes_values = NULL;
static unsigned n_jes_values = 0;
static integ_parameters ipars;
static Mc_scan_parameters mpars;
static Prob_to_acquire_jet *prob_to_acquire_jet = NULL;
static Prob_to_loose_parton *prob_to_loose_parton = NULL;
static int excluded_jet_number = -1;

/* Some useful defaults */
static char *default_category = "Work";
static char *default_sobol = "sobol";
static char *default_none = "none";

#define check_parameter_range(name, minval, maxval) do {\
    if (name < minval || name > maxval)\
    {\
        Tcl_SetResult(interp,\
                      "parameter \"" #name "\" out of range",\
                      TCL_VOLATILE);\
        return TCL_ERROR;\
    }\
} while(0);

#define setup_required_parameter(parser_type, name, minval, maxval) do {\
    if (get_ ## parser_type ## _parameter(interp, some_string,\
                                          #name, &name) != TCL_OK)\
        return TCL_ERROR;\
    check_parameter_range(name, minval, maxval);\
} while(0);

#define setup_optional_parameter(parser_type, name, default_value, minval, maxval) do {\
    name = default_value;\
    if (get_ ## parser_type ## _parameter(interp, some_string,\
                                          #name, &name) != TCL_OK)\
        Tcl_ResetResult(interp);\
    check_parameter_range(name, minval, maxval);\
} while(0);

#define required_string_parameter(name) do {\
    name = get_string_parameter(interp, some_string, #name);\
    if (name == NULL)\
    {\
        Tcl_SetResult(interp,\
                      "missing parameter \"" #name "\"",\
                      TCL_VOLATILE);\
        return TCL_ERROR;\
    }\
} while(0);

#define optional_string_parameter(name, default_value) do {\
    name = get_string_parameter(interp, some_string, #name);\
    if (name == NULL)\
        name = default_value;\
} while(0);

#define free_string_parameter(name, default_value) do {\
    if (name && name != default_value)\
    {\
        free(name);\
        name = NULL;\
    }\
} while(0);

static int setup_scan_parameters(Tcl_Interp *interp, const char *some_string)
{
    /* Parse parameters which are neither members of 
     * "integ_parameters" nor "Mc_scan_parameters"
     */
    int print_integration_parameters;
    double topscan_min, topscan_max, jes_min, jes_max;
    double eta_q_sigma_factor, eta_b_sigma_factor;
    double phi_q_sigma_factor, phi_b_sigma_factor;
    double nu_momentum_cutoff, q_momentum_cutoff, had_phase_space_cutoff;
    double detlepNu_cutoff, detlepW_cutoff;

    setup_optional_parameter(boolean, print_integration_parameters, 0, 0, 1);
    setup_optional_parameter(boolean, fill_result_ntuple, 1, 0, 1);

    setup_optional_parameter(double, parton_syserr_pt_cutoff, 21.0, 0.0,   1.e10);
    setup_optional_parameter(double, sample_pole_mass,    0.0,    0.0,     1.e10);
    setup_optional_parameter(boolean, discard_excluded_jet, 0,    0,       1);
    setup_optional_parameter(double, topscan_min,         157.0,  0.0,     1.e10);
    setup_optional_parameter(double, topscan_max,         188.0,  0.0,     1.e10);
    setup_optional_parameter(int,    topscan_points,      31,     1,       INT_MAX);

    setup_optional_parameter(double, jes_min,            -3.1,   -1.e100,  1.e100);
    setup_optional_parameter(double, jes_max,             3.1,   -1.e100,  1.e100);
    setup_optional_parameter(int,    n_jes_points,        31,     1,       128);

    setup_optional_parameter(double, prob_factor,    20.e8,   1.e-100, 1.e100);
    setup_optional_parameter(int,    maxevents,      INT_MAX, 0,       INT_MAX);
    setup_optional_parameter(int,    skipevents,     0,       0,       INT_MAX);
    setup_optional_parameter(int,    script_period,  1,       1,       INT_MAX);

    optional_string_parameter(category, default_category);
    optional_string_parameter(periodic_script, NULL);

    setup_optional_parameter(double, eta_q_sigma_factor,  1.059,  0.001,  1000.0);
    setup_optional_parameter(double, eta_b_sigma_factor,  1.147,  0.001,  1000.0);
    setup_optional_parameter(double, phi_q_sigma_factor,  1.059,  0.001,  1000.0);
    setup_optional_parameter(double, phi_b_sigma_factor,  1.132,  0.001,  1000.0);

    setup_optional_parameter(double, nu_momentum_cutoff,     4.0,    0.0,  1.0e100);
    setup_optional_parameter(double, q_momentum_cutoff,      1.0e-4, 0.0,  1.0e100);
    setup_optional_parameter(double, had_phase_space_cutoff, 1.0,    0.0,  1.0e100);
    setup_optional_parameter(double, detlepNu_cutoff,        0.1,    0.0,  1.0e100);
    setup_optional_parameter(double, detlepW_cutoff,         10.0,   0.0,  1.0e100);

    /* Generate the JES grid */
    if (n_jes_points)
    {
        int i;
        const double tstep = (jes_max - jes_min)/n_jes_points;

        get_static_memory((void **)&jes_values, sizeof(double),
                          &n_jes_values, n_jes_points);
        for (i=0; i<n_jes_points; ++i)
            jes_values[i] = jes_min + (i + 0.5)*tstep;
    }

    /* Generate the top mass grid */
    if (topscan_points)
    {
        int i;
        const double tstep = (topscan_max - topscan_min)/topscan_points;

        get_static_memory((void **)&top_mass_values, sizeof(double),
                          &n_top_mass_values, topscan_points);
        for (i=0; i<topscan_points; ++i)
            top_mass_values[i] = topscan_min + (i + 0.5)*tstep;
    }

    /* Setup correction factors for angular resolution modeling */
    set_eta_width_factor(eta_q_sigma_factor, 0);
    set_eta_width_factor(eta_b_sigma_factor, 1);
    set_phi_width_factor(phi_q_sigma_factor, 0);
    set_phi_width_factor(phi_b_sigma_factor, 1);

    /* Set up phase space factor limits */
    set_phase_space_limits(nu_momentum_cutoff, q_momentum_cutoff,
                           had_phase_space_cutoff, detlepNu_cutoff, detlepW_cutoff);

    /* Parse parameters which are members of "Mc_scan_parameters" */
    {
        int status;
        N_d_random_method rmethod, lside_rmethod;

        double mc_check_factor;
        double mc_mwsq_safety_margin;
        double mc_precision_fraction;
        double mc_precision_target;
        double mc_worst_perm_cutoff;
        double mc_nominal_q_mass, mc_nominal_b_mass;
        double mc_abs_param_min, mc_abs_param_max;
        double mc_rel_param_min, mc_rel_param_max;
        double mc_lside_pt_max;
        double mc_lside_nuz_min, mc_lside_nuz_max;
        double mc_lside_b_tailsize, mc_lside_b_conesize;
        int mc_had_mt_gen_type;
        int mc_lep_mt_gen_type;
        int mc_fudge_to_treelevel;
        int mc_min_points;
        int mc_max_points;
        int mc_max_zeroprob_pts;
        int mc_max_event_seconds;
        int mc_lside_mask_nPt, mc_lside_mask_nPhi;
        int mc_lside_max_points, mc_lside_max_seconds;
        int mc_efficiency_mask;
        int mc_random_gen_param, mc_lside_rnd_param;
        char *mc_rand_method, *mc_lside_rand_method;
        unsigned mc_dim_mask = 0x7FFFFFFFU;
        int mc_grid_mask_param, mc_grid_mask_whad;
        int mc_grid_mask_tau, mc_grid_mask_ttbarpt;
        int mc_grid_mask_toplep, mc_grid_mask_tophad;
        int mc_grid_mask_mq, mc_grid_mask_mqbar;
        int mc_grid_mask_mblep, mc_grid_mask_mbhad;
        int mc_grid_mask_angle_q, mc_grid_mask_angle_qbar;
        int mc_grid_mask_angle_bhad, mc_grid_mask_angle_blep;

        char *mc_prob_to_acquire_jet, *mc_prob_to_loose_parton, *mc_excluded_jet;
        double mc_pt_cut, mc_eta_cut, mc_max_efficiency, mc_jes_sigma_factor;

        setup_optional_parameter(double,  mc_check_factor,       2.0,     1.0,      1000.0);
        setup_optional_parameter(double,  mc_mwsq_safety_margin, 0.01,    0.0,      1.0e100);
        setup_optional_parameter(double,  mc_precision_fraction, 0.9,     0.0,      1.0);
        setup_optional_parameter(double,  mc_precision_target,   0.02,    0.0,      1.0);
        setup_optional_parameter(double,  mc_worst_perm_cutoff,  2.0,     0.0,      DBL_MAX);
        setup_optional_parameter(double,  mc_nominal_q_mass,     0.0,     0.0,      DBL_MAX);
        setup_optional_parameter(double,  mc_nominal_b_mass,     4.8,     0.0,      DBL_MAX);
        setup_optional_parameter(double,  mc_abs_param_min,     -4.0,    -1.0e100,  1.0e100);
        setup_optional_parameter(double,  mc_abs_param_max,      4.0,    -1.0e100,  1.0e100);
        setup_optional_parameter(double,  mc_rel_param_min,     -2.0,    -1.0e100,  1.0e100);
        setup_optional_parameter(double,  mc_rel_param_max,      2.0,    -1.0e100,  1.0e100);

        setup_optional_parameter(double,  mc_lside_pt_max,      500.0,    0.0,      1.0e100);
        setup_optional_parameter(double,  mc_lside_nuz_min,    -400.0,   -1.0e100,  1.0e100);
        setup_optional_parameter(double,  mc_lside_nuz_max,     400.0,   -1.0e100,  1.0e100);
        setup_optional_parameter(double,  mc_lside_b_tailsize,   0.01,    0.0,      0.5);
        setup_optional_parameter(double,  mc_lside_b_conesize,   0.7,     0.0,      M_PI);

        setup_optional_parameter(int,     mc_had_mt_gen_type,    0,       0,        INT_MAX);
        setup_optional_parameter(int,     mc_lep_mt_gen_type,    0,       0,        INT_MAX);
        setup_optional_parameter(int,     mc_fudge_to_treelevel, 0,       0,        INT_MAX);
        setup_optional_parameter(int,     mc_min_points,         1024,    1,        INT_MAX);
        setup_optional_parameter(int,     mc_max_points,         524288,  1,        INT_MAX);
        setup_optional_parameter(int,     mc_max_zeroprob_pts,  4*mc_min_points, 0, INT_MAX);
        setup_optional_parameter(int,     mc_max_event_seconds,  INT_MAX, 0,        INT_MAX);

        setup_optional_parameter(int,     mc_lside_mask_nPt,     64,      1,        INT_MAX);
        setup_optional_parameter(int,     mc_lside_mask_nPhi,    64,      1,        INT_MAX);
        setup_optional_parameter(int,     mc_lside_max_points,   524288,  1,        INT_MAX);
        setup_optional_parameter(int,     mc_lside_max_seconds,  0,       0,        INT_MAX);

        setup_optional_parameter(int,     mc_efficiency_mask,    0,       INT_MIN,  INT_MAX);

        setup_optional_parameter(int,     mc_random_gen_param,   0,       INT_MIN,  INT_MAX);
        setup_optional_parameter(int,     mc_lside_rnd_param,    0,       INT_MIN,  INT_MAX);

        setup_optional_parameter(boolean, mc_grid_mask_param,    1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_whad,     1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_tau,      0,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_ttbarpt,  1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_toplep,   1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_tophad,   1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_mq,       1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_mqbar,    1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_mbhad,    1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_mblep,    1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_angle_q,  1,       0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_angle_qbar, 1,     0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_angle_bhad, 1,     0,        1);
        setup_optional_parameter(boolean, mc_grid_mask_angle_blep, 1,     0,        1);
        optional_string_parameter(mc_rand_method, default_sobol );
        optional_string_parameter(mc_lside_rand_method, default_sobol );

        optional_string_parameter(mc_prob_to_acquire_jet, default_none );
        optional_string_parameter(mc_prob_to_loose_parton, default_none );
        optional_string_parameter(mc_excluded_jet, default_none );
        setup_optional_parameter(double,  mc_pt_cut,             20.0,    0.0,      1.0e100);
        setup_optional_parameter(double,  mc_eta_cut,            2.0,     0.0,      1.0e100);
        setup_optional_parameter(double,  mc_max_efficiency,     1.0,     0.0,      1.0);
        setup_optional_parameter(double,  mc_jes_sigma_factor,   0.03,    0.0,      1.0);

        rmethod = parse_n_d_random_method(mc_rand_method);
        if (rmethod == N_D_RANDOM_INVALID)
        {
            Tcl_AppendResult(interp, "unknown integration random gen method \"",
                             mc_rand_method, "\"", 0);
            return TCL_ERROR;
        }
        lside_rmethod = parse_n_d_random_method(mc_lside_rand_method);
        if (lside_rmethod == N_D_RANDOM_INVALID)
        {
            Tcl_AppendResult(interp, "unknown lepton side mask random gen method \"",
                             mc_lside_rand_method, "\"", 0);
            return TCL_ERROR;
        }

        if (!mc_grid_mask_param)
            mc_dim_mask &= ~(1U << MC_DIM_PARAM);
        if (!mc_grid_mask_whad)
            mc_dim_mask &= ~(1U << MC_DIM_WHAD);
        if (!mc_grid_mask_tau)
            mc_dim_mask &= ~(1U << MC_DIM_TAU);
        if (!mc_grid_mask_ttbarpt)
        {
            mc_dim_mask &= ~(1U << MC_DIM_TTBAR_PT_MAG);
            mc_dim_mask &= ~(1U << MC_DIM_TTBAR_PT_PHI);
        }
        if (!mc_grid_mask_toplep)
            mc_dim_mask &= ~(1U << MC_DIM_TLEP);
        if (!mc_grid_mask_tophad)
            mc_dim_mask &= ~(1U << MC_DIM_THAD);
        if (!mc_grid_mask_mq)
            mc_dim_mask &= ~(1U << MC_DIM_MQ);
        if (!mc_grid_mask_mqbar)
            mc_dim_mask &= ~(1U << MC_DIM_MQBAR);
        if (!mc_grid_mask_mbhad)
            mc_dim_mask &= ~(1U << MC_DIM_MBHAD);
        if (!mc_grid_mask_mblep)
            mc_dim_mask &= ~(1U << MC_DIM_MBLEP);
        if (!mc_grid_mask_angle_q)
        {
            mc_dim_mask &= ~(1U << MC_DIM_Q_ETA);
            mc_dim_mask &= ~(1U << MC_DIM_Q_PHI);
        }
        if (!mc_grid_mask_angle_qbar)
        {
            mc_dim_mask &= ~(1U << MC_DIM_QBAR_ETA);
            mc_dim_mask &= ~(1U << MC_DIM_QBAR_PHI);
        }
        if (!mc_grid_mask_angle_bhad)
        {
            mc_dim_mask &= ~(1U << MC_DIM_BHAD_ETA);
            mc_dim_mask &= ~(1U << MC_DIM_BHAD_PHI);
        }
        if (!mc_grid_mask_angle_blep)
        {
            mc_dim_mask &= ~(1U << MC_DIM_BLEP_ETA);
            mc_dim_mask &= ~(1U << MC_DIM_BLEP_PHI);
        }

        status = fill_mc_scan_parameters(&mpars, mc_check_factor, mc_mwsq_safety_margin,
                                         mc_precision_fraction, mc_precision_target,
                                         mc_worst_perm_cutoff,
                                         mc_nominal_q_mass, mc_nominal_b_mass,
                                         mc_abs_param_min, mc_abs_param_max,
                                         mc_rel_param_min, mc_rel_param_max,
                                         mc_lside_pt_max, mc_lside_nuz_min, mc_lside_nuz_max,
                                         mc_lside_b_tailsize, mc_lside_b_conesize,
                                         mc_had_mt_gen_type,
                                         mc_lep_mt_gen_type, mc_fudge_to_treelevel,
                                         mc_min_points, mc_max_points,
                                         mc_max_zeroprob_pts, mc_max_event_seconds,
                                         mc_lside_mask_nPt, mc_lside_mask_nPhi,
                                         mc_lside_max_points, mc_lside_max_seconds,
                                         mc_dim_mask, mc_efficiency_mask,
                                         mc_random_gen_param, mc_lside_rnd_param,
                                         rmethod, lside_rmethod);
        if (status)
        {
            char buf[128];
            sprintf(buf, "bad mc integration parameter, status %d", status);
            Tcl_SetResult(interp, buf, TCL_VOLATILE);
            return TCL_ERROR;
        }
        if (print_integration_parameters)
            print_mc_scan_parameters(&mpars, stdout);

        prob_to_acquire_jet = choose_prob_to_acquire_jet(mc_prob_to_acquire_jet);
        if (prob_to_acquire_jet == invalid_prob_to_acquire_jet)
        {
            Tcl_SetResult(interp, "invalid \"mc_prob_to_acquire_jet\" value", TCL_VOLATILE);
            return TCL_ERROR;
        }

        prob_to_loose_parton = choose_prob_to_loose_parton(mc_prob_to_loose_parton);
        if (prob_to_loose_parton == invalid_prob_to_loose_parton)
        {
            Tcl_SetResult(interp, "invalid \"mc_prob_to_loose_parton\" value", TCL_VOLATILE);
            return TCL_ERROR;
        }

        set_params_to_loose_parton(mc_pt_cut, mc_eta_cut,
                                   mc_max_efficiency, mc_jes_sigma_factor);

        excluded_jet_number = parse_jet_number(mc_excluded_jet);

        free_string_parameter(mc_rand_method, default_sobol );
        free_string_parameter(mc_lside_rand_method, default_sobol );
        free_string_parameter(mc_prob_to_acquire_jet, default_none );
        free_string_parameter(mc_prob_to_loose_parton, default_none );
        free_string_parameter(mc_excluded_jet, default_none );
    }

    /* Parse parameters which are members of "integ_parameters" */
    {
        int status;

        double nominal_w_mass;
        double nominal_w_width;
        double cms_energy;
        double light_jet_mass;
        double had_b_jet_mass;
        double lep_b_jet_mass;
        double whad_coverage;
        double wlep_coverage;
        double thad_coverage;
        double tlep_coverage;
        double top_width;
        int wlep_npoints;
        int debug_level;
        int matrel_code;
        int process_tcl_events;
        int permute_jets;
        int param_grid_absolute;

        setup_optional_parameter(double,  nominal_w_mass,       80.385, 1.0,  1000.0);
        setup_optional_parameter(double,  nominal_w_width,      2.085,  0.1,  100.0);
        setup_optional_parameter(double,  cms_energy,           1960.0, 1.0,  1.0e10);
        setup_optional_parameter(double,  light_jet_mass,       0.0,   -1.0e100, 100.0);
        setup_optional_parameter(double,  had_b_jet_mass,       4.8,   -1.0e100, 100.0);
        setup_optional_parameter(double,  lep_b_jet_mass,       4.8,   -1.0e100, 100.0);
        setup_optional_parameter(double,  whad_coverage,        0.998,  0.0,  1.0);
        setup_optional_parameter(double,  wlep_coverage,        0.99,   0.0,  1.0);
        setup_optional_parameter(double,  thad_coverage,        0.98,   0.0,  1.0);
        setup_optional_parameter(double,  tlep_coverage,        0.98,   0.0,  1.0);
	setup_optional_parameter(double,  top_width,            1.2,   -1.0e10, 1.0e10);
        setup_optional_parameter(int,     wlep_npoints,         7,      1,    INT_MAX);
        setup_optional_parameter(int,     debug_level,          10,  INT_MIN, INT_MAX);
        setup_optional_parameter(boolean, process_tcl_events,   0,      0,    1);
        setup_optional_parameter(boolean, param_grid_absolute,  0,      0,    1);
        setup_required_parameter(int,     matrel_code,                  0,    7);
        setup_required_parameter(int,     permute_jets,                 0,    2);

        status = fill_integ_parameters(&ipars, nominal_w_mass, nominal_w_width,
                                       cms_energy, light_jet_mass,
                                       had_b_jet_mass, lep_b_jet_mass,
                                       whad_coverage, wlep_coverage,
                                       thad_coverage, tlep_coverage,
                                       top_width, wlep_npoints,
				       debug_level, matrel_code,
                                       process_tcl_events, permute_jets,
				       param_grid_absolute);
        if (status)
        {
            char buf[128];
            sprintf(buf, "bad integration parameter, status %d", status);
            Tcl_SetResult(interp, buf, TCL_VOLATILE);
            return TCL_ERROR;
        }
        if (print_integration_parameters)
        {
            print_integ_parameters(&ipars, stdout);
            fflush(stdout);
        }
    }

    return TCL_OK;
}

static void cleanup_scan_parameters(void)
{
    cleanup_integ_parameters(&ipars);
    memset(&ipars, 0, sizeof(integ_parameters));

    if (top_mass_values)
    {
        free(top_mass_values);
        top_mass_values = NULL;
        n_top_mass_values = 0;
    }

    if (jes_values)
    {
        free(jes_values);
        jes_values = NULL;
        n_jes_values = 0;
    }

    free_string_parameter(category, default_category);
    free_string_parameter(periodic_script, NULL);
}
