#include "topmass_integrator.h"
#include "norm_accumulator.h"
#include "n_d_random.h"
#include "permutations.h"
#include "single_jet_probs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SCAN_STATUS_CONTINUE = 0,
    SCAN_STATUS_OK,
    SCAN_STATUS_MAXPOINTS,
    SCAN_STATUS_ZEROPROB,
    SCAN_STATUS_TIMELIMIT,
    N_SCAN_STATUS_VALUES
} Scan_status;

extern const char* const scan_status_names[N_SCAN_STATUS_VALUES];

enum {
    MC_DIM_PARAM = 0,
    MC_DIM_WHAD,
    MC_DIM_THAD,
    MC_DIM_TTBAR_PT_MAG,
    MC_DIM_TTBAR_PT_PHI,
    MC_DIM_TLEP,
    MC_DIM_TAU,
    MC_DIM_MQ,
    MC_DIM_MQBAR,
    MC_DIM_MBHAD,
    MC_DIM_MBLEP,
    MC_DIM_Q_ETA,
    MC_DIM_Q_PHI,
    MC_DIM_QBAR_ETA,
    MC_DIM_QBAR_PHI,
    MC_DIM_BHAD_ETA,
    MC_DIM_BHAD_PHI,
    MC_DIM_BLEP_ETA,
    MC_DIM_BLEP_PHI,
    N_MC_DIMS
};

typedef struct {
    double check_factor;        /* Defines how often the integration
                                 * code checks for completion.
                                 */
    double mwsq_safety_margin;  /* Must be a small positive number.
                                 * Used to wiggle the kinematic points.
                                 */
    double precision_fraction;
    double precision_target;    /* Scan precision target. Currently used
                                 * as the relative likelihood precision
                                 * which should be reached by all points
                                 * in the likelihood curve which make
                                 * up "precision_fraction" fraction of
                                 * the total probability.
                                 */
    double worst_perm_cutoff;   /* Controls which permutations are used in
				 * the permutation mask for MC integration.
				 * One permutation is worst for the 
				 * greatest number of unsatisfied points.
				 * If the number of points for which that 
				 * permutation is worst is n0, then all 
				 * permutations which are worst for at least 
				 * n0/worst_perm_cutoff points are included 
				 * in the mask.
                                 */
    double nominal_q_mass;      /* Nominal light quark mass used in converting
                                 * the event kinematics to tree level.
                                 */
    double nominal_b_mass;      /* Nominal b quark mass used in converting
                                 * the event kinematics to tree level.
                                 */
    double abs_param_min;       /* Integration limits for log(pq/pqbar),   */
    double abs_param_max;       /* for relative and absolute grids. Note   */
    double rel_param_min;       /* that the grid will be absolute in the   */
    double rel_param_max;       /* 3-jet integration mode.                 */

    double lside_pt_max;        /* These parameters are used to create the */
    double lside_nuz_min;       /* leptonic side masks.                    */
    double lside_nuz_max;
    double lside_b_tailsize;
    double lside_b_conesize;

    unsigned had_mt_gen_type;   /* These parameters describe the choice    */
    unsigned lep_mt_gen_type;   /* of inverse cdf used for top masses      */

    unsigned fudge_to_treelevel;/* Tells whether to convert the events
                                 * with massive jets to tree level before
                                 * applying the matrix element weight.
                                 */
    unsigned min_points;        /* Minimum number of random points to use   */
    unsigned max_points;        /* Maximum number of random points to use   */
    unsigned max_zeroprob_pts;  /* Maximum number of random points to use
                                 * in case probabilities for all points are 0
                                 */
    unsigned max_event_seconds; /* Maximum integration time per event       */

    unsigned lside_mask_nPt;    /* To use the leptonic side masks, _all_    */
    unsigned lside_mask_nPhi;   /* of the following conditions must be      */
    unsigned lside_max_points;  /* satisfied:                               */
    unsigned lside_max_seconds; /*     lside_mask_nPt > 0                   */
                                /*     lside_mask_nPhi > 0                  */
                                /*     lside_max_seconds > 0                */
                                /*     mc_dim_mask bit MC_DIM_TAU is 0      */
                                /* The "lside_max_points" parameter can be  */
                                /* set to 0. In this case the masks will    */
                                /* be populated until lside_max_seconds     */
                                /* seconds expire.                          */

    unsigned mc_dim_mask;       /* Set each of the N_MC_DIMS bits as needed */

    unsigned efficiency_mask;   /* Look at the "event_efficiency.h" header  */
                                /* to see the meaning of bits in this mask. */

    int random_gen_param;       /* Passed to the "init_n_d_random" function
                                 * as a multipurpose parameter.
                                 */
    int lside_rnd_param;        /* Same thing, for generating leptonic side
                                 * masks
                                 */
    N_d_random_method random_method; /* Type of the random generator to use
                                      * for integration
                                      */
    N_d_random_method lside_random;  /* Type of the random generator to use
                                      * for building leptonic side masks
                                      */
} Mc_scan_parameters;

/* Don't fill the above structure by hand, use the function below instead.
 * This way any change in the inteface will be noticed by the compiler.
 * The function returns 0 if all parameter values make sense, otherwise
 * it returns something else.
 */
int fill_mc_scan_parameters(Mc_scan_parameters *mpars,
                            double check_factor,
                            double mwsq_safety_margin,
                            double precision_fraction,
                            double precision_target,
                            double worst_perm_cutoff,
                            double nominal_q_mass,
                            double nominal_b_mass,
                            double abs_param_min,
                            double abs_param_max,
                            double rel_param_min,
                            double rel_param_max,
                            double lside_pt_max,
                            double lside_nuz_min,
                            double lside_nuz_max,
                            double lside_b_tailsize,
                            double lside_b_conesize,
                            unsigned had_mt_gen_type,
                            unsigned lep_mt_gen_type,
                            unsigned fudge_to_treelevel,
                            unsigned min_points,
                            unsigned max_points,
                            unsigned max_zeroprob_pts, 
                            unsigned max_event_seconds,
                            unsigned lside_mask_nPt,
                            unsigned lside_mask_nPhi,
                            unsigned lside_max_points,
                            unsigned lside_max_seconds,
                            unsigned mc_dim_mask,
                            unsigned efficiency_mask,
                            int random_gen_param,
                            int lside_rnd_param,
                            N_d_random_method rmethod,
                            N_d_random_method lside_rmethod);

void print_mc_scan_parameters(const Mc_scan_parameters *mpars, FILE *stream);

/* Function used to do the ttbar->l+jets integration by MC.
 * "results" array must have n_tmass_points rows and n_jes_points columns.
 * Note that the number of rows and columns here is transposed wrt
 * the "top_jes_scan" function.
 *
 * "hepg_delta_mthad" is the difference between the actual top quark mass
 * on the hadronic side used by HEPG in this particular event and the
 * pole top mass. This parameter is used only when the integration over
 * the hadronic top mass is not turned on. It should be set to 0 for data.
 * "hepg_delta_mtlep" has the same meaning for the leptonic side top.
 */
Scan_status mc_top_jes_scan(const integ_parameters *ipars,
                const Mc_scan_parameters *mpars,
                const double *jes_points, unsigned n_jes_points,
                const double *tmass_points, unsigned n_tmass_points,
                jet_info *jets, unsigned n_jets,
                double lPx, double lPy, double lPz,
                int leptonCharge, int isMu,
                double hepg_delta_mthad, double hepg_delta_mtlep,
                double hepg_mwhad, double hepg_param, double hepg_mwlep,
                double ttbarPx_unscal, double ttbarPy_unscal,
                Prob_to_acquire_jet *prob_to_acquire_jet,
                Prob_to_loose_parton *prob_to_loose_parton,
                norm_accumulator *results);

#ifdef __cplusplus
}
#endif
