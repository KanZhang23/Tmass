#include <assert.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <float.h>

#include "mc_topmass_integrator.h"
#include "topmass_utils.h"
#include "tag_probability_weight.h"
#include "generate_ttbar_pt.h"
#include "tau_pt_spectrum.h"
#include "ttbar_phase_space.h"
#include "p_struct_function.h"
#include "transfer_function.h"
#include "matrel_mahlon_parke.h"
#include "matrel_kleiss_stirling.h"
#include "random_parton_mass.h"
#include "random_jet_angles.h"
#include "tfs_2015.h"
#include "single_parton_efficiency.h"
#include "sorter.h"
#include "topmass_norm.h"
#include "wgrid.h"
#include "leptonic_side_mask.h"
#include "event_efficiency.h"
#include "parton_syserr.h"
#include "twidth.h"

/* Comment out TCL_EVENTS_ALLOWED if you don't
 * want to link to the tcl library
 */
#define TCL_EVENTS_ALLOWED
#ifdef TCL_EVENTS_ALLOWED
#include "tcl.h"   /* for Tcl_DoOneEvent */
#endif

#define MIN_MT_MW_DIFFERENCE_HAD 0.5
#define MIN_MT_MW_DIFFERENCE_LEP 0.5
#define TAU_LOWER_LIMIT_FACTOR 0.95
#define MIN_MW_MPARTONS_DIFF 1.0
#define MAX_JES_POINTS 128
#define RELATIVE_EPS_MINWMASS 1.0e-12
#define TF_RATIO_CUTOFF 3.0
#define MIN_MW_BOUNDARY_DISTANCE 1.0

#ifdef __cplusplus
extern "C" {
#endif

/* Interface to LAPACK */
void dgetrf_(int *M, int *N, double *A, int *LDA, int *IPIV, int *INFO);
void dgetrs_(char *TRANS, int *N, int *NRHS, double *A, int *LDA, int *IPIV,
	     double *B, int *LDB, int *INFO, int lenTRANS);

typedef enum {
    SCAN_THREE_JETS = 0,
    SCAN_FOUR_JETS,
    SCAN_FIVE_JETS
} Scan_type;

typedef struct {
    unsigned iperm;
    unsigned worst_counts;
} Permutation_counter;
 
typedef struct {
    double value;
    double err;
    unsigned worst_perm;
} Likelihood_value;

typedef struct {
    double jes_weights[MAX_JES_POINTS];
    double jaco_mw;
    double jaco_pnuz;
    lepton_side_solution lsol;
} Lepton_solution_info;

typedef struct {
    Lepton_solution_info info[4];
    int nlepsols;
} W_point_info;

typedef struct {
    double had_tfprod[MAX_JES_POINTS];
    particle_obj l_94;
    particle_obj q_solved;
    particle_obj qbar_solved;
    particle_obj bhad_solved;
    particle_obj thad;
    particle_obj tree_q;
    particle_obj tree_qbar;
    particle_obj tree_bhad;
    v3_obj olddir_blep;
    double mtscan;
    double toplevel_weight;
    int estimate_blep_angular_tf;
    int lCharge;
} Common_leptonic_params;

typedef struct {
    double d;
    int i;
} d_i_pair;

const char* const scan_status_names[N_SCAN_STATUS_VALUES] = {
    "continue",
    "ok",
    "maxpoints",
    "zeroprob",
    "timelimit"
};

sort_struct_by_member_decr(Likelihood_value, value)
sort_struct_by_member_incr(lepton_side_solution, nuz)
sort_struct_by_member_incr(d_i_pair, d)
sort_struct_by_member_decr(Permutation_counter, worst_counts)

static void mc_permutation_scan(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const unsigned n_points_to_scan,
    const float *random_grid, const unsigned ndim,
    const int random_dim_map[N_MC_DIMS],
    const double *jes_points, const unsigned n_jes_points,
    const double *tmass_points, const unsigned n_tmass_points,
    jet_info *q, jet_info *qbar,
    jet_info *bhad, jet_info *blep,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double hepg_delta_mthad, const double hepg_delta_mtlep,
    const double hepg_mwhad, const double hepg_param,
    const double hepg_mwlep,
    const double ttbarPx_unscal, const double ttbarPy_unscal,
    const Leptonic_side_mask *lmask,
    Prob_to_loose_parton *prob_to_loose_parton,
    norm_accumulator *results);

static void mc_one_top_mass_scan(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const double *jes_points, unsigned n_jes_points,
    const double mwhad,
    const double param, const double mt, const unsigned itop,
    const jet_info *q, const jet_info *qbar,
    const jet_info *bhad, const jet_info *blep,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double hepg_delta_mthad, const double hepg_delta_mtlep,
    const double hepg_mwlep,
    const double ttbarPx_unscal, const double ttbarPy_unscal,
    const float *r, const int random_dim_map[N_MC_DIMS],
    double toplevel_weight,
    const Leptonic_side_mask *lmask,
    Prob_to_loose_parton *prob_to_loose_parton,
    norm_accumulator *mt_results);

static void mc_integrate_w_mass(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const double *jes_points, unsigned n_jes_points,
    const hadron_side_solution *hsol,
    const jet_info *q, const jet_info *qbar,
    const jet_info *bhad, const jet_info *blep,
    const jet_info *blep_orig, const unsigned is_blepdir_random,
    const double mtscan, const unsigned itop,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double fixed_mwlep,
    const double ttbarPx, const double ttbarPy, const double mtlep,
    const double toplevel_weight,
    Prob_to_loose_parton *prob_to_loose_parton,
    double *event_weight);

static int is_random_covered(
    const integ_parameters *ipars,
    const float r[N_MC_DIMS],
    const int random_dim_map[N_MC_DIMS]);

static void random_dir_jet(jet_info *newjet, double m,
                           int isB, float r1, float r2);

static Scan_status is_scan_complete(
    const Mc_scan_parameters *mpars,
    const unsigned max_points_integrated,
    const time_t integration_start_time,
    const unsigned n_jes_points,
    const unsigned n_tmass_points,
    const double perm_weights[5*N_JET_PERMUTATIONS][MAX_JES_POINTS],
    const unsigned n_points_integrated[5*N_JET_PERMUTATIONS],
    const norm_accumulator *perm_results,
    int current_perm_mask[5*N_JET_PERMUTATIONS]);

static unsigned init_dim_map(const unsigned mc_dim_mask,
                             int random_dim_map[N_MC_DIMS]);

static void slide_w_grid(wgrid *g, const double mwmax,
                         const double wlep_coverage)
{
    const double peak  = g->mW*g->mW;
    const double hwhm  = g->mW*g->widthW;
    const double cdf0  = atan((0.0 - peak)/hwhm)/M_PI + 0.5;
    const double cdfmax = atan((mwmax*mwmax - peak)/hwhm)/M_PI + 0.5;
    const double coverage_delta = (1.0 - wlep_coverage)/2.0;
    const double cdf_delta = (cdfmax - cdf0)*coverage_delta;
    const double mwsq_min = hwhm*tan(M_PI*(cdf0 + cdf_delta - 0.5)) + peak;
    const double mwsq_max = hwhm*tan(M_PI*(cdfmax - cdf_delta - 0.5)) + peak;
    assert(mwsq_min > 0.0);
    assert(mwsq_max > mwsq_min);
    set_w_grid_edges(g, sqrt(mwsq_min), sqrt(mwsq_max));
}

/* The following function determines the scan type
 * and verifies the argument consistency for that
 * scan type.
 */
static Scan_type determine_scan_type(
    const unsigned n_jets,
    Prob_to_acquire_jet *prob_to_acquire_jet,
    Prob_to_loose_parton *prob_to_loose_parton)
{
    Scan_type stype;

    switch (n_jets)
    {
    case 4:
        if (prob_to_loose_parton == NULL &&
            prob_to_acquire_jet == NULL)
            stype = SCAN_FOUR_JETS;
        else
        {
            assert(prob_to_acquire_jet);
            assert(prob_to_loose_parton);
            stype = SCAN_THREE_JETS;
        }
        break;

    case 5:
        assert(prob_to_acquire_jet);
        assert(prob_to_loose_parton == NULL);
        stype = SCAN_FIVE_JETS;
        break;

    default:
        assert(0);
    }
    return stype;
}

static unsigned n_scan_permutations(const Scan_type scan_type)
{
    switch (scan_type)
    {
    case SCAN_THREE_JETS:
        return 4*N_JET_PERMUTATIONS;

    case SCAN_FOUR_JETS:
        return N_JET_PERMUTATIONS;

    case SCAN_FIVE_JETS:
        return 5*N_JET_PERMUTATIONS;

    default:
        assert(0);
    }
}

static int find_extra_jet(const jet_info *jets, const unsigned n_jets)
{
    int n_dropped = 0, i_drop = -1;
    unsigned i;

    for (i=0; i<n_jets; ++i)
        if (jets[i].is_extra)
        {
            i_drop = i;
            ++n_dropped;
        }
    if (n_dropped == 1)
        return i_drop;
    else
        return -1;
}

static void normalize_perm_weights(
    const unsigned n_jes_points,
    double perm_weight[5*N_JET_PERMUTATIONS][MAX_JES_POINTS])
{
    unsigned iperm, ijes;

    for (ijes=0; ijes<n_jes_points; ++ijes)
    {
        double perm_w_sum = 0.0;
        for (iperm=0; iperm<5*N_JET_PERMUTATIONS; ++iperm)
        {
            assert(perm_weight[iperm][ijes] >= 0.0);
            perm_w_sum += perm_weight[iperm][ijes];
        }
        assert(perm_w_sum > 0.0);
        for (iperm=0; iperm<5*N_JET_PERMUTATIONS; ++iperm)
            perm_weight[iperm][ijes] /= perm_w_sum;
    }
}

static void determine_perm_weights_three(
    const integ_parameters *ipars,
    const jet_info *input_jets,
    const int leptonCharge,
    Prob_to_acquire_jet *prob_to_acquire_jet,
    const double *jes_points, const unsigned n_jes_points,
    double perm_weight[5*N_JET_PERMUTATIONS][MAX_JES_POINTS])
{
    memset(perm_weight, 0, 5*N_JET_PERMUTATIONS*MAX_JES_POINTS*sizeof(double));

    switch (ipars->permute_jets)
    {
    case 0:
        /* Do not permute. However, we must know which jet to drop. */
    {
        unsigned ijes;
        const int idrop = find_extra_jet(input_jets, 4);
        const unsigned iperm = idrop*N_JET_PERMUTATIONS + 11;
        assert(idrop >= 0);
        assert(idrop < 4);
        for (ijes=0; ijes<n_jes_points; ++ijes)
            perm_weight[iperm][ijes] = 1.0;
    }
    break;

    case 1:
        /* Permutations with correct b tag assignment get
         * positive weights, with incorrect assignment 0.
         * Also, we will not drop jets with b tags if
         * the number of b tags is 2 or less.
         */
    {
        unsigned idrop;
        unsigned all_btags = 0;

        for (idrop=0; idrop<4; ++idrop)
            all_btags += jet_has_btag(input_jets + idrop);

        for (idrop=0; idrop<4; ++idrop)
        {
            const int dropped_jet_has_btag = jet_has_btag(input_jets + idrop);
            if (!dropped_jet_has_btag || all_btags > 2)
            {
                unsigned iperm, ijes;
                double dropprob[MAX_JES_POINTS];
                const jet_info *currentPermutation[4];

                const unsigned nbtags = dropped_jet_has_btag ? 
                    all_btags - 1 : all_btags;

                for (ijes=0; ijes<n_jes_points; ++ijes)
                {
                    double scale = 1.0 + jes_points[ijes]*input_jets[idrop].syserr;
                    dropprob[ijes] = prob_to_acquire_jet(input_jets, 4, idrop, scale);
                }

                for (iperm=0; iperm<N_JET_PERMUTATIONS; ++iperm)
                {
                    const unsigned index = idrop*N_JET_PERMUTATIONS + iperm;

                    /* Permute jet assignments */
                    unsigned j, assigned_btags = 0;
                    for (j=0; j<4; ++j)
                        currentPermutation[allPermutations[iperm].index[j]] = 
                            &input_jets[j];

                    /* Check correctness of the b jet assignment */
                    if (currentPermutation[BHAD] != input_jets+idrop &&
                        jet_has_btag(currentPermutation[BHAD]))
                        ++assigned_btags;
                    if (currentPermutation[BLEP] != input_jets+idrop &&
                        jet_has_btag(currentPermutation[BLEP]))
                        ++assigned_btags;
                    if (assigned_btags < 2 && assigned_btags < nbtags)
                    {
                        /* We have not assigned all b-tagged jets
                         * to b quarks. This permutation gets 0 weight.
                         */
                        for (ijes=0; ijes<n_jes_points; ++ijes)
                            perm_weight[index][ijes] = 0.0;
                    }
                    else
                    {
                        for (ijes=0; ijes<n_jes_points; ++ijes)
                            perm_weight[index][ijes] = dropprob[ijes];
                    }
                }
            }
        }
    }
    break;

    default:
        /* Permutations are weighted according to the tag probability */
    {
        unsigned idrop;
        jet_info jets[4];
        const jet_info *currentPermutation[4];

        memcpy(jets, input_jets, 4*sizeof(jet_info));
        for (idrop=0; idrop<4; ++idrop)
            jets[idrop].is_extra = 0;

        for (idrop=0; idrop<4; ++idrop)
        {
            double dropprob[MAX_JES_POINTS];
            unsigned ijes, j, iperm;

            for (ijes=0; ijes<n_jes_points; ++ijes)
            {
                double scale = 1.0 + jes_points[ijes]*jets[idrop].syserr;
                dropprob[ijes] = prob_to_acquire_jet(jets, 4, idrop, scale);
            }

            if (idrop)
                jets[idrop-1].is_extra = 0;
            jets[idrop].is_extra = 1;

            for (iperm=0; iperm<N_JET_PERMUTATIONS; ++iperm)
            {
                double tagw;
                const unsigned index = idrop*N_JET_PERMUTATIONS + iperm;
                for (j=0; j<4; ++j)
                    currentPermutation[allPermutations[iperm].index[j]] = &jets[j];
                tagw = tag_probability_weight(currentPermutation, leptonCharge);
                for (ijes=0; ijes<n_jes_points; ++ijes)
                    perm_weight[index][ijes] = dropprob[ijes]*tagw;
            }
        }
    }
    break;
    }

    normalize_perm_weights(n_jes_points, perm_weight);
}

static void determine_perm_weights_four(
    const integ_parameters *ipars,
    const jet_info *input_jets,
    const int leptonCharge,
    Prob_to_acquire_jet *prob_to_acquire_jet,
    const double *jes_points, const unsigned n_jes_points,
    double perm_weight[5*N_JET_PERMUTATIONS][MAX_JES_POINTS])
{
    /* Can't have extra jets */
    assert(find_extra_jet(input_jets, 4) < 0);

    memset(perm_weight, 0, 5*N_JET_PERMUTATIONS*MAX_JES_POINTS*sizeof(double));

    switch (ipars->permute_jets)
    {
    case 0:
        /* Do not permute */
    {
        unsigned ijes;
        for (ijes=0; ijes<n_jes_points; ++ijes)
            perm_weight[11][ijes] = 1.0;
    }
    break;

    case 1:
        /* Permutations with correct b tag assignment get
         * weights 1, with incorrect assignment 0.
         */
    {
        unsigned iperm;
        unsigned nbtags = 0;

        for (iperm=0; iperm<4; ++iperm)
            nbtags += jet_has_btag(input_jets + iperm);

        for (iperm=0; iperm<N_JET_PERMUTATIONS; ++iperm)
        {
            /* Permute jet assignments */
            unsigned j, assigned_btags, ijes;
            const jet_info *currentPermutation[4];

            for (j=0; j<4; ++j)
                currentPermutation[allPermutations[iperm].index[j]] 
                    = &input_jets[j];

            /* Check correctness of the b jet assignment */
            assigned_btags = jet_has_btag(currentPermutation[BHAD]) +
                jet_has_btag(currentPermutation[BLEP]);
            if (assigned_btags < 2 && assigned_btags < nbtags)
            {
                /* We have not assigned all b-tagged jets
                 * to b quarks. This permutation gets 0 weight.
                 */
                for (ijes=0; ijes<n_jes_points; ++ijes)
                    perm_weight[iperm][ijes] = 0.0;
            }
            else
            {
                for (ijes=0; ijes<n_jes_points; ++ijes)
                    perm_weight[iperm][ijes] = 1.0;
            }
        }
    }
    break;

    default:
        /* Permutations are weighted according to the tag probability */
    {
        unsigned iperm;
        const jet_info *currentPermutation[4];

        for (iperm=0; iperm<N_JET_PERMUTATIONS; ++iperm)
        {
            unsigned j, ijes;
            double tagw;
            for (j=0; j<4; ++j)
                currentPermutation[allPermutations[iperm].index[j]] =
                    &input_jets[j];
            tagw = tag_probability_weight(currentPermutation, leptonCharge);
            for (ijes=0; ijes<n_jes_points; ++ijes)
                perm_weight[iperm][ijes] = tagw;
        }
    }
    break;
    }

    normalize_perm_weights(n_jes_points, perm_weight);
}

static void determine_perm_weights_five(
    const integ_parameters *ipars,
    const jet_info *input_jets,
    const int leptonCharge,
    Prob_to_acquire_jet *prob_to_acquire_jet,
    const double *jes_points, const unsigned n_jes_points,
    double perm_weight[5*N_JET_PERMUTATIONS][MAX_JES_POINTS])
{
    /* Can't have extra jets */
    assert(find_extra_jet(input_jets, 5) < 0);

    memset(perm_weight, 0, 5*N_JET_PERMUTATIONS*MAX_JES_POINTS*sizeof(double));

    switch (ipars->permute_jets)
    {
    case 0:
        /* Do not permute. Assume that the first four jets
         * are given in the correct order.
         */
    {
        unsigned ijes;
        for (ijes=0; ijes<n_jes_points; ++ijes)
            perm_weight[11][ijes] = 1.0;
    }
    break;

    case 1:
        /* Permutations with correct b tag assignment get
         * weights 1, with incorrect assignment 0.
         * Also, we will not drop jets with b tags if
         * the number of b tags is 2 or less.
         */
    {
        unsigned idrop;
        unsigned all_btags = 0;

        for (idrop=0; idrop<5; ++idrop)
            all_btags += jet_has_btag(input_jets + idrop);

        for (idrop=0; idrop<5; ++idrop)
        {
            const int dropped_jet_has_btag = jet_has_btag(input_jets + idrop);
            if (!dropped_jet_has_btag || all_btags > 2)
            {
                double dropprob[MAX_JES_POINTS];

                const unsigned nbtags = dropped_jet_has_btag ? 
                    all_btags - 1 : all_btags;

                /* Drop the jet */
                jet_info jets[4];
                unsigned j, iperm, ijes, icopy = 0;

                for (ijes=0; ijes<n_jes_points; ++ijes)
                {
                    double scale = 1.0 + jes_points[ijes]*input_jets[idrop].syserr;
                    dropprob[ijes] = prob_to_acquire_jet(input_jets, 5, idrop, scale);
                }

                for (j=0; j<5; ++j)
                    if (j != idrop)
                        memcpy(jets + icopy++, input_jets + j, sizeof(jet_info));
                assert(icopy == 4);

                for (iperm=0; iperm<N_JET_PERMUTATIONS; ++iperm)
                {
                    const unsigned index = idrop*N_JET_PERMUTATIONS + iperm;
                    unsigned assigned_btags;
                    const jet_info *currentPermutation[4];

                    for (j=0; j<4; ++j)
                        currentPermutation[allPermutations[iperm].index[j]] = 
                            &jets[j];

                    /* Check correctness of the b jet assignment */
                    assigned_btags = jet_has_btag(currentPermutation[BHAD]) +
                        jet_has_btag(currentPermutation[BLEP]);
                    if (assigned_btags < 2 && assigned_btags < nbtags)
                    {
                        /* We have not assigned all b-tagged jets
                         * to b quarks. This permutation gets 0 weight.
                         */
                        for (ijes=0; ijes<n_jes_points; ++ijes)
                            perm_weight[index][ijes] = 0.0;
                    }
                    else
                    {
                        for (ijes=0; ijes<n_jes_points; ++ijes)                        
                            perm_weight[index][ijes] = dropprob[ijes];
                    }
                }
            }
        }
    }
    break;

    default:
        /* Permutations are weighted according to the tag probability */
    {
        unsigned idrop;
        for (idrop=0; idrop<5; ++idrop)
        {
            /* Drop the jet */
            jet_info jets[4];
            unsigned j, iperm, ijes, icopy = 0;
            const jet_info *currentPermutation[4];
            double dropprob[MAX_JES_POINTS];

            for (ijes=0; ijes<n_jes_points; ++ijes)
            {
                double scale = 1.0 + jes_points[ijes]*input_jets[idrop].syserr;
                dropprob[ijes] = prob_to_acquire_jet(input_jets, 5, idrop, scale);
            }

            for (j=0; j<5; ++j)
                if (j != idrop)
                    memcpy(jets + icopy++, input_jets + j, sizeof(jet_info));
            assert(icopy == 4);

            for (iperm=0; iperm<N_JET_PERMUTATIONS; ++iperm)
            {
                const unsigned index = idrop*N_JET_PERMUTATIONS + iperm;
                double tagw;
                for (j=0; j<4; ++j)
                    currentPermutation[allPermutations[iperm].index[j]] =
                        &jets[j];
                tagw = tag_probability_weight(currentPermutation, leptonCharge);
                for (ijes=0; ijes<n_jes_points; ++ijes)
                    perm_weight[index][ijes] = dropprob[ijes]*tagw;
            }
        }
    }
    break;
    }

    normalize_perm_weights(n_jes_points, perm_weight);
}

static void fill_quasi_mc_grid(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const Scan_type scan_type,
    const jet_info *input_jets,
    const int random_dim_map[N_MC_DIMS],
    const unsigned ndims,
    float *random_grid)
{
    float r[N_MC_DIMS];
    unsigned idim, irand = 0;

    /* Check that the grid dimensions were meaningfully initialized */
    if (scan_type == SCAN_THREE_JETS)
    {
        if (ipars->permute_jets)
        {
            /* We must be able to integrate over all jet angles */
            assert(random_dim_map[MC_DIM_Q_ETA] >= 0);
            assert(random_dim_map[MC_DIM_Q_PHI] >= 0);
            assert(random_dim_map[MC_DIM_QBAR_ETA] >= 0);
            assert(random_dim_map[MC_DIM_QBAR_PHI] >= 0);
            assert(random_dim_map[MC_DIM_BLEP_ETA] >= 0);
            assert(random_dim_map[MC_DIM_BLEP_PHI] >= 0);
            assert(random_dim_map[MC_DIM_BHAD_ETA] >= 0);
            assert(random_dim_map[MC_DIM_BHAD_PHI] >= 0);
        }
        else
        {
            /* We must be able to integrate over the angles
             * of the missing parton
             */
            if (input_jets[Q].is_extra)
            {
                assert(random_dim_map[MC_DIM_Q_ETA] >= 0);
                assert(random_dim_map[MC_DIM_Q_PHI] >= 0);
            }
            if (input_jets[QBAR].is_extra)
            {
                assert(random_dim_map[MC_DIM_QBAR_ETA] >= 0);
                assert(random_dim_map[MC_DIM_QBAR_PHI] >= 0);
            }
            if (input_jets[BLEP].is_extra)
            {
                assert(random_dim_map[MC_DIM_BLEP_ETA] >= 0);
                assert(random_dim_map[MC_DIM_BLEP_PHI] >= 0);
            }
            if (input_jets[BHAD].is_extra)
            {
                assert(random_dim_map[MC_DIM_BHAD_ETA] >= 0);
                assert(random_dim_map[MC_DIM_BHAD_PHI] >= 0);
            }
        }
    }

    /* Fill out the grid values */
    do {
        int is_good_random = 1;
        next_n_d_random(r);
        for (idim=0; idim<ndims; ++idim)
            if (r[idim] <= 0.f || r[idim] >= 1.f)
                is_good_random = 0;
        if (is_good_random && is_random_covered(ipars, r, random_dim_map))
        {
            float *ptr = random_grid + ndims*irand++;
            for (idim=0; idim<ndims; ++idim)
                ptr[idim] = r[idim];
        }
    } while (irand < mpars->max_points);
}

static void randomize_jet_dir(const jet_info *old_jet,
                              const double correct_mom, const int isB,
                              const float rnd_eta, const float rnd_phi,
                              const int use_massive_tfs,
                              jet_info *new_jet, double *density)
{
    const double c = correct_mom/old_jet->p;
    const v3_obj old = v3(c*old_jet->px, c*old_jet->py, c*old_jet->pz);
    const v3_obj newj = randomize_sampling_fromrand(old, isB, old_jet->m,
                                                    rnd_eta, rnd_phi, density);
    assert(use_massive_tfs);
    fill_jet_info(new_jet, newj.x/c, newj.y/c, newj.z/c, old_jet->m,
                  old_jet->bProb, old_jet->bFake, old_jet->etaDet,
                  old_jet->syserr, old_jet->derr_dpt, old_jet->cuterr,
                  old_jet->isLoose, old_jet->ntracks, old_jet->is_extra);
}

static int mc_solve_leptonic_side(
    const double shift_magnitude, const int shift_dir,
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const int debug_level, const size_t max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4])
{
    const double mwsq_shift = mb > 0.0 ? 0.0 : shift_magnitude*shift_dir;

    const int nlepsols0 = mb > 0.0 ?
        solve_leptonic_side_massiveb_brute(
            topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
            mt, mb, mwsq,
            /* betalist, n_beta_points, */
            debug_level, max_iterations,
            minTopPz, maxTopPz, solutions) :
        solve_leptonic_byMW_approx(
            topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
            mt, mb, mwsq, 1.0, debug_level,
            minTopPz, maxTopPz, solutions);

    if (mwsq_shift)
    {
        lepton_side_solution solutions1[4];
        const int nlepsols1 = mb > 0.0 ?
            solve_leptonic_side_massiveb_brute(
                topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
                mt, mb, mwsq + mwsq_shift,
                /* betalist, n_beta_points, */
                debug_level, max_iterations,
                minTopPz, maxTopPz, solutions1) :
            solve_leptonic_byMW_approx(
                topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
                mt, mb, mwsq + mwsq_shift, 1.0, debug_level,
                minTopPz, maxTopPz, solutions1);
        if (nlepsols0 == nlepsols1)
            return nlepsols0;
        else
        {
            lepton_side_solution solutions2[4];
            const int nlepsols2 = mb > 0.0 ?
                solve_leptonic_side_massiveb_brute(
                    topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
                    mt, mb, mwsq + mwsq_shift/2.0,
                    debug_level, max_iterations,
                    minTopPz, maxTopPz, solutions2) :
                solve_leptonic_byMW_approx(
                    topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
                    mt, mb, mwsq + mwsq_shift/2.0, 1.0, debug_level,
                    minTopPz, maxTopPz, solutions2);
            if (nlepsols0 == nlepsols2)
                return nlepsols0;
            else if (nlepsols1 == nlepsols2)
            {
                memcpy(solutions, solutions2,
                       nlepsols2*sizeof(lepton_side_solution));
                return nlepsols2;
            }
            else
            {
                /* The coincidence scheme has failed.
                 * Not sure what is the best thing to do here.
                 */
                if (debug_level > 0)
                {
                    static int max_warnings_coincidence = 1000;
                    if (max_warnings_coincidence > 0)
                    {
                        printf("WARNING in mc_solve_leptonic_side: "
                               "coincidence scheme failure\n");
                        if (--max_warnings_coincidence == 0)
                            printf("Further occurrences of this message "
                                   "will be suppressed\n");
                        fflush(stdout);
                    }
                }
                return 0;
            }
        }
    }
    else
        return nlepsols0;
}

Scan_status mc_top_jes_scan(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const double *jes_points, const unsigned n_jes_points,
    const double *tmass_points, const unsigned n_tmass_points,
    jet_info *input_jets, const unsigned n_jets,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double hepg_delta_mthad,
    const double hepg_delta_mtlep,
    const double hepg_mwhad,
    const double hepg_param,
    const double hepg_mwlep,
    const double ttbarPx_unscal,
    const double ttbarPy_unscal,
    Prob_to_acquire_jet *prob_to_acquire_jet,
    Prob_to_loose_parton *prob_to_loose_parton,
    norm_accumulator *event_results)
{
    static norm_accumulator* result_buf = 0;
    static unsigned result_buf_length = 0;

    static float *random_grid = 0;
    static unsigned random_grid_size = 0;
    static unsigned ndims = 0;
    static int random_dim_map[N_MC_DIMS];
    static unsigned mc_dim_mask = 0;
    static N_d_random_method random_method = 0;
    static int random_gen_param = 0;
    static unsigned max_integ_points = 0;
    static Leptonic_side_mask lep_masks[5];
    static unsigned lep_masks_initialized = 0;

    const Scan_type scan_type = determine_scan_type(
        n_jets, prob_to_acquire_jet, prob_to_loose_parton);
    const unsigned res_points = n_tmass_points*n_jes_points;
    const time_t integration_start_time = time(NULL);
    const unsigned n_jet_permutations = n_scan_permutations(scan_type);

    unsigned next_cycle_points = mpars->min_points;
    Scan_status scan_status = SCAN_STATUS_CONTINUE;
    unsigned n_points_integrated[5*N_JET_PERMUTATIONS] = {0,};
    int permutation_mask[5*N_JET_PERMUTATIONS] = {0,};
    double perm_weight[5*N_JET_PERMUTATIONS][MAX_JES_POINTS];
    int use_lepside_mask;
    jet_info jets[4];

    /* Check that we will be able to go through with the integration */
    assert(ipars);
    assert(ipars->nominal_w_mass > 0.0);
    /* assert(ipars->top_width > 0.0); */

    assert(mpars);
    assert(mpars->min_points > 0);
    assert(mpars->max_points >= mpars->min_points);
 
    assert(jes_points);
    assert(n_jes_points);
    assert(n_jes_points <= MAX_JES_POINTS);
    assert(tmass_points);
    assert(n_tmass_points);
    assert(input_jets);
    assert(event_results);

    /* Set b quark mass as a singleton */
    setBQuarkMass(mpars->nominal_b_mass);

    /* (Re)build the quasi-random grid.
     * This is normally done for the first event only.
     */
    if (mpars->max_points != max_integ_points ||
        mpars->mc_dim_mask != mc_dim_mask ||
        mpars->random_method != random_method ||
        mpars->random_gen_param != random_gen_param)
    {
        ndims = init_dim_map(mpars->mc_dim_mask, random_dim_map);
        init_n_d_random(mpars->random_method, ndims, mpars->random_gen_param);

        get_static_memory((void **)&random_grid, sizeof(float),
                          &random_grid_size, ndims*mpars->max_points);
        fill_quasi_mc_grid(ipars, mpars, scan_type, input_jets,
                           random_dim_map, ndims, random_grid);

        max_integ_points = mpars->max_points;
        mc_dim_mask = mpars->mc_dim_mask;
        random_method = mpars->random_method;
        random_gen_param = mpars->random_gen_param;
    }

    /* Limit the scan types for now */
    assert(scan_type == SCAN_FOUR_JETS);

    /* Figure out the permutation weights */
    switch (scan_type)
    {
    case SCAN_THREE_JETS:
        determine_perm_weights_three(
            ipars, input_jets, leptonCharge,
            prob_to_acquire_jet,
            jes_points, n_jes_points,
            perm_weight);
        break;

    case SCAN_FOUR_JETS:
        determine_perm_weights_four(
            ipars, input_jets, leptonCharge,
            prob_to_acquire_jet,
            jes_points, n_jes_points,
            perm_weight);
        break;

    case SCAN_FIVE_JETS:
        determine_perm_weights_five(
            ipars, input_jets, leptonCharge,
            prob_to_acquire_jet,
            jes_points, n_jes_points,
            perm_weight);
        break;

    default:
        assert(0);
    }

    /* Allocate and reset the permutation results buffer */
    {
        unsigned i;
        get_static_memory((void **)&result_buf, sizeof(norm_accumulator),
                          &result_buf_length, res_points*n_jet_permutations);
        for (i=0; i<res_points*n_jet_permutations; ++i)
            norm_reset(result_buf + i);
    }

    /* Set the initial permutation mask */
    {
        unsigned iperm;
        for (iperm=0; iperm<n_jet_permutations; ++iperm)
            if (perm_weight[iperm][n_jes_points/2] > 0.0)
                permutation_mask[iperm] = 1;
    }

    /* Generate leptonic side masks if requested,
     * and if this is not a tau scan (leptonic side
     * masks do not work with tau scans).
     *
     * Note that the leptonic side masks are initialized
     * when the first event is encountered. They will
     * not be reinitialized later even if parameters
     * change.
     */
    use_lepside_mask = (random_dim_map[MC_DIM_TAU] < 0 &&
                        mpars->lside_mask_nPt &&
                        mpars->lside_mask_nPhi &&
                        mpars->lside_max_seconds);
    if (use_lepside_mask)
    {
        double b_cone = 0.0;
        unsigned i, lside_randomize_mb = 0;

        if (!lep_masks_initialized)
        {
            double mt_min, mt_max;

            if (n_tmass_points > 1)
            {
                const double lodelta = tmass_points[1] - tmass_points[0];
                const double hidelta = tmass_points[n_tmass_points-1] - 
                                       tmass_points[n_tmass_points-2];
                mt_min = tmass_points[0] - lodelta/2.0;
                mt_max = tmass_points[n_tmass_points-1] + hidelta/2.0;
            }
            else
            {
                mt_min = tmass_points[0] - 1.0;
                mt_max = tmass_points[0] + 1.0;
            }

            for (i=0; i<sizeof(lep_masks)/sizeof(lep_masks[0]); ++i)
                init_leptonic_side_mask(lep_masks+i, mt_min, mt_max,
                                        mpars->lside_pt_max, n_tmass_points,
                                        mpars->lside_mask_nPt,
                                        mpars->lside_mask_nPhi);
            lep_masks_initialized = 1;
        }

        if (random_dim_map[MC_DIM_BLEP_ETA] >= 0 &&
            random_dim_map[MC_DIM_BLEP_PHI] >= 0)
            b_cone = mpars->lside_b_conesize;
        if (random_dim_map[MC_DIM_MBLEP] >= 0)
            lside_randomize_mb = 1;

        for (i=0; i<n_jets; ++i)
        {
            const jet_info *bjet = input_jets + i;
            const double m = ipars->lep_b_jet_mass >= 0.0 ? 
                             ipars->lep_b_jet_mass : bjet->m;
            const time_t t_start = time(NULL);
            const unsigned npt = populate_leptonic_side_mask(
                lep_masks+i, mpars->lside_random, mpars->lside_rnd_param,
                lPx, lPy, lPz, bjet->px, bjet->py, bjet->pz, m, bjet->cuterr,
                mpars->lside_nuz_min, mpars->lside_nuz_max,
                ipars->nominal_w_mass, ipars->nominal_w_width,
                mpars->lside_b_tailsize, b_cone,
                lside_randomize_mb, mpars->lside_max_points,
                mpars->lside_max_seconds);
            if (ipars->debug_level > 0)
            {
                const double fillfrac = leptonic_side_masked_fraction(lep_masks+i);
                printf("Jet %u leptonic side mask: "
                       "%u tries in %d sec, masked fraction %g\n",
                       i, npt, (int)(time(NULL) - t_start), fillfrac);
            }
            input_jets[i].perm_info = lep_masks+i;
        }
    }

    /* Make a local copy of the jets */
    memcpy(jets, input_jets, 4*sizeof(jet_info));

    /* Run the integration cycle */
    while (scan_status == SCAN_STATUS_CONTINUE)
    {
        unsigned superperm, max_points_integrated = 0;
	for (superperm=0; superperm<n_jet_permutations; ++superperm)
            if (permutation_mask[superperm] == 1)
            {
                unsigned j;
                jet_info *currentPermutation[4];
                Leptonic_side_mask *lmask;

                const unsigned thisperm = superperm % N_JET_PERMUTATIONS;
                const unsigned iextra = superperm / N_JET_PERMUTATIONS;

                /* Calculate a pointer into the quasi-random
                 * set for this particular permutation
                 */
                const float *rand_buf = random_grid + 
                    n_points_integrated[superperm]*ndims;

                assert(perm_weight[superperm][n_jes_points/2] > 0.0);

                if (scan_type == SCAN_FIVE_JETS)
                {
                    /* Need to copy the whole jet array into
                     * the local buffer omitting the extra jet
                     */
                    unsigned icopy = 0;
                    for (j=0; j<5; ++j)
                        if (j != iextra)
                            memcpy(jets + icopy++, input_jets + j, sizeof(jet_info));
                    assert(icopy == 4);
                }
                else
                {
                    /* Just restore the original jet mass assignments */
                    for (j=0; j<4; ++j)
                        jets[j].m = input_jets[j].m;
                }

                /* Mark the extra jet for the three-jet scan */
                for (j=0; j<4; ++j)
                    jets[j].is_extra = 0;
                if (scan_type == SCAN_THREE_JETS)
                {
                    assert(iextra < 5);
                    jets[iextra].is_extra = 1;
                }

                /* Permute jet assignments */
                for (j=0; j<4; ++j)
                    currentPermutation[allPermutations[thisperm].index[j]] = &jets[j];

                /* Assign jet masses according to permutation */
                if (ipars->light_jet_mass >= 0.0)
                {
                    currentPermutation[Q]->m    = ipars->light_jet_mass;
                    currentPermutation[QBAR]->m = ipars->light_jet_mass;
                }
                if (ipars->lep_b_jet_mass >= 0.0)
                    currentPermutation[BLEP]->m = ipars->lep_b_jet_mass;
                if (ipars->had_b_jet_mass >= 0.0)
                    currentPermutation[BHAD]->m = ipars->had_b_jet_mass;

                /* Check whether leptonic side mask usage is requested */
                if (use_lepside_mask && !currentPermutation[BLEP]->is_extra)
                    lmask = (Leptonic_side_mask *)(currentPermutation[BLEP]->perm_info);
                else
                    lmask = 0;

                /* Scan this permutation */
                mc_permutation_scan(
                    ipars, mpars, next_cycle_points,
                    rand_buf, ndims, random_dim_map,
                    jes_points, n_jes_points,
                    tmass_points, n_tmass_points,
                    currentPermutation[Q], currentPermutation[QBAR],
                    currentPermutation[BHAD], currentPermutation[BLEP],
                    lPx, lPy, lPz, leptonCharge, isMu,
                    hepg_delta_mthad, hepg_delta_mtlep,
                    hepg_mwhad, hepg_param, hepg_mwlep,
                    ttbarPx_unscal, ttbarPy_unscal,
                    lmask, prob_to_loose_parton,
                    result_buf + superperm*res_points);
                n_points_integrated[superperm] += next_cycle_points;
                if (n_points_integrated[superperm] > max_points_integrated)
                    max_points_integrated = n_points_integrated[superperm];
            }

        assert(max_points_integrated);

        /* Figure out how many points to scan in the next cycle */
        {       
            unsigned nextcheck = max_points_integrated*mpars->check_factor;
            assert(nextcheck > max_points_integrated);
            if (nextcheck > mpars->max_points)
                nextcheck = mpars->max_points;
            next_cycle_points = nextcheck - max_points_integrated;
        }
        scan_status = is_scan_complete(mpars, max_points_integrated,
                                       integration_start_time,
                                       n_jes_points, n_tmass_points,
                                       (const double (*)[MAX_JES_POINTS])perm_weight,
                                       n_points_integrated,
                                       result_buf, permutation_mask);
    }

    /* Sum up the permutations */
    {
        unsigned ires, iperm;

        for (ires=0; ires<res_points; ++ires)
            norm_reset(event_results + ires);

        for (iperm=0; iperm<n_jet_permutations; ++iperm)
            if (perm_weight[iperm][n_jes_points/2] > 0.0)
            {
                norm_accumulator *result_buffer = result_buf + iperm*res_points;
                for (ires=0; ires<res_points; ++ires)
                {
                    const unsigned ijes = ires % n_jes_points;
                    norm_scale(result_buffer+ires, perm_weight[iperm][ijes]);
                    norm_add_nocorr(event_results+ires, result_buffer+ires);
                }
            }
    }

    return scan_status;
}

static void mc_permutation_scan(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const unsigned n_points_to_scan,
    const float *random_grid, const unsigned ndims,
    const int random_dim_map[N_MC_DIMS],
    const double *jes_points, const unsigned n_jes_points,
    const double *tmass_points, const unsigned n_tmass_points,
    jet_info * const q_in, jet_info * const qbar_in,
    jet_info * const bhad_in, jet_info * const blep_in,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double hepg_delta_mthad, const double hepg_delta_mtlep,
    const double hepg_mwhad, const double hepg_param,
    const double hepg_mwlep,
    const double ttbarPx_unscal, const double ttbarPy_unscal,
    const Leptonic_side_mask *lmask,
    Prob_to_loose_parton *prob_to_loose_parton,
    norm_accumulator *results)
{
    const int randomize_param = random_dim_map[MC_DIM_PARAM] >= 0;
    const int randomize_mwhad = random_dim_map[MC_DIM_WHAD] >= 0;
    const int randomize_mq    = random_dim_map[MC_DIM_MQ] >= 0;
    const int randomize_mqbar = random_dim_map[MC_DIM_MQBAR] >= 0;
    const int randomize_mbhad = random_dim_map[MC_DIM_MBHAD] >= 0;
    const int randomize_mblep = random_dim_map[MC_DIM_MBLEP] >= 0;
    const int randomize_q     = random_dim_map[MC_DIM_Q_ETA] >= 0 &&
                                random_dim_map[MC_DIM_Q_PHI] >= 0;
    const int randomize_qbar  = random_dim_map[MC_DIM_QBAR_ETA] >= 0 &&
                                random_dim_map[MC_DIM_QBAR_PHI] >= 0;
    const int extra_w_dau = q_in->is_extra || qbar_in->is_extra;
    const int param_absolute = ipars->param_grid_absolute || extra_w_dau;
    const double param_lolim = param_absolute ? 
        mpars->abs_param_min : mpars->rel_param_min;
    const double param_uplim = param_absolute ? 
        mpars->abs_param_max : mpars->rel_param_max;
    const double paramrange = param_uplim - param_lolim;
    const double param_offset = param_absolute ?
        param_lolim : param_lolim + log(q_in->p/qbar_in->p);
    const int use_massive_tf_q    = randomize_mq    || ipars->light_jet_mass < 0.0;
    const int use_massive_tf_qbar = randomize_mqbar || ipars->light_jet_mass < 0.0;
    const double w_peak = ipars->nominal_w_mass*ipars->nominal_w_mass;
    const double w_hwhm = ipars->nominal_w_mass*ipars->nominal_w_width;
    const double mw_max = ipars->cms_energy/2.0;

    unsigned icycle;

    /* Check the arguments */
    if (!q_in->is_extra)
        assert(q_in->p > 0.0);
    if (!qbar_in->is_extra)
        assert(qbar_in->p > 0.0);
    if (ipars->light_jet_mass < 0.0)
        assert(randomize_mq && randomize_mqbar);
    if (ipars->had_b_jet_mass < 0.0)
        assert(randomize_mbhad);
    if (ipars->lep_b_jet_mass < 0.0)
        assert(randomize_mblep);

    /* Don't do 3-jet integration for now */
    assert(!extra_w_dau);

    /* Cycle over the sequence of pseudo-random numbers */
    for (icycle=0; icycle<n_points_to_scan; ++icycle)
    {
        /* printf("cycle %d\n", icycle); */
        const float *r = random_grid + icycle*ndims;

        int top_scan_called = 0;
        double param, mwsq, min_allowed_mw;

        if (randomize_param)
            param = param_offset + r[random_dim_map[MC_DIM_PARAM]]*paramrange;
        else
            param = hepg_param;

        /* Generate parton masses. They are assumed to be
         * independent from everything else.
         */
        if (randomize_mq)
            q_in->m = random_q_mass_fromrand(r[random_dim_map[MC_DIM_MQ]]);
        if (randomize_mqbar)
            qbar_in->m = random_q_mass_fromrand(r[random_dim_map[MC_DIM_MQBAR]]);
        min_allowed_mw = q_in->m + qbar_in->m + MIN_MW_MPARTONS_DIFF;

        /* Generate the hadronic W mass */
        assert(randomize_mwhad);
        mwsq = w_hwhm*tan(M_PI*(r[random_dim_map[MC_DIM_WHAD]] - 0.5)) + w_peak;

        /*
        if (randomize_mwhad)
            mwsq = w_hwhm*tan(M_PI*(r[random_dim_map[MC_DIM_WHAD]] - 0.5)) + w_peak;
        else
            mwsq = hepg_mwhad*hepg_mwhad;
        */

        if (mwsq >= min_allowed_mw*min_allowed_mw && mwsq < mw_max*mw_max)
        {
            /* Generate the angles for the extra parton */
            const jet_info *q = q_in;
            const jet_info *qbar = qbar_in;
            double q_randomization_weight = 1.0/cauchy_density(mwsq, w_peak, w_hwhm);
            jet_info missing_parton;

            if (q_in->is_extra)
            {
                assert(randomize_q);
                random_dir_jet(&missing_parton, q_in->m, 0,
                               r[random_dim_map[MC_DIM_Q_ETA]],
                               r[random_dim_map[MC_DIM_Q_PHI]]);
                q = &missing_parton;
            }
            else if (qbar_in->is_extra)
            {
                assert(randomize_qbar);
                random_dir_jet(&missing_parton, qbar_in->m, 0,
                               r[random_dim_map[MC_DIM_QBAR_ETA]],
                               r[random_dim_map[MC_DIM_QBAR_PHI]]);
                qbar = &missing_parton;
            }
            if (extra_w_dau)
            {
                /* The phase space contains (sin(theta))^2 for conversion
                 * from d cos(theta) into d eta. Since we are integrating
                 * over d cos(theta) now, we should remove that factor.
                 */
                const double sintheta = missing_parton.pt/missing_parton.p;
                assert(sintheta > 0.0);
                q_randomization_weight /= (sintheta*sintheta);
            }

            /* We can now proceed with randomization
             * of the remaining hadronic W daughter angles.
             * b jet angles will have to be randomized
             * for each top mass separately.
             */
            {
                double pq, pqbar;
                const int nsols = solve_hadronic_w(
                    q->px, q->py, q->pz, q->m,
                    qbar->px, qbar->py, qbar->pz, qbar->m,
                    mwsq, param, &pq, &pqbar);
                if (nsols == 1)
                {
                    const int redo_q = randomize_q && !q_in->is_extra;
                    const int redo_qbar = randomize_qbar && !qbar_in->is_extra;
                    jet_info newq, newqbar;
                    double pqnew, pqbarnew;

                    if (redo_q)
                    {
                        double density = 0.0;
                        randomize_jet_dir(q, pq, 0,
                                          r[random_dim_map[MC_DIM_Q_ETA]],
                                          r[random_dim_map[MC_DIM_Q_PHI]],
                                          use_massive_tf_q, &newq, &density);
                        assert(density > 0.0);
                        q_randomization_weight /= density;
                    }
                    else
                        newq = *q;

                    if (redo_qbar)
                    {
                        double density = 0.0;
                        randomize_jet_dir(qbar, pqbar, 0,
                                          r[random_dim_map[MC_DIM_QBAR_ETA]],
                                          r[random_dim_map[MC_DIM_QBAR_PHI]],
                                          use_massive_tf_qbar, &newqbar, &density);
                        assert(density > 0.0);
                        q_randomization_weight /= density;
                    }
                    else
                        newqbar = *qbar;

                    if (redo_q || redo_qbar)
                    {
                        const int newsols = solve_hadronic_w(
                            newq.px, newq.py, newq.pz, newq.m,
                            newqbar.px, newqbar.py, newqbar.pz, newqbar.m,
                            mwsq, param, &pqnew, &pqbarnew);
                        if (newsols == 1)
                        {;}
                        else
                        {
                            assert(newsols == 0);
                            q_randomization_weight = 0.0;
                        }
                    }
                    else
                    {
                        pqnew = pq;
                        pqbarnew = pqbar;
                    }

                    /* WARNING!!!! The following code works only
                     * when the transfer functions are p/E transfer
                     * functions!!!
                     */
                    if (q_randomization_weight > 0.0)
                    {
                        if (!q_in->is_extra)
                        {
                            const double Eq = sqrt(pqnew*pqnew + newq.m*newq.m);
                            if (newq.p/Eq > TF_RATIO_CUTOFF)
                                 q_randomization_weight = 0.0;
                        }
                        if (!qbar_in->is_extra)
                        {
                            const double Eqbar = sqrt(pqbarnew*pqbarnew +
                                                      newqbar.m*newqbar.m);
                            if (newqbar.p/Eqbar > TF_RATIO_CUTOFF)
                                 q_randomization_weight = 0.0;
                        }
                    }

                    if (q_randomization_weight > 0.0)
                    {
                        /*
                        // In the "param" variable we are using uniform sampling
                        // density, with density equal to 1/paramrange. The weight
                        // should be divided by that density, i.e., multiplied by
                        // paramrange.
                        */
                        const double weight_so_far = paramrange*q_randomization_weight;
                        const double mw = sqrt(mwsq);
                        unsigned itop;

                        /* Randomize the b masses */
                        if (randomize_mbhad)
                            bhad_in->m = random_b_mass_fromrand(
                                r[random_dim_map[MC_DIM_MBHAD]]);
                        if (randomize_mblep)
                            blep_in->m = random_b_mass_fromrand(
                                r[random_dim_map[MC_DIM_MBLEP]]);

                        for (itop=0; itop<n_tmass_points; ++itop)
                            mc_one_top_mass_scan(
                                ipars, mpars, jes_points, n_jes_points,
                                mw, param, tmass_points[itop], itop,
                                &newq, &newqbar, bhad_in, blep_in,
                                lPx, lPy, lPz, leptonCharge, isMu,
                                hepg_delta_mthad, hepg_delta_mtlep,
                                hepg_mwlep, ttbarPx_unscal, ttbarPy_unscal,
                                r, random_dim_map, weight_so_far,
                                lmask, prob_to_loose_parton,
                                results + itop*n_jes_points);
                        top_scan_called = 1;
                    }
                }
                else
                    assert(nsols == 0);
            }
        }

        if (!top_scan_called)
        {
            const unsigned res_points = n_tmass_points*n_jes_points;
            unsigned ires;
            for (ires=0; ires<res_points; ++ires)
                norm_accumulate(results+ires, 0.0);
        }

#ifdef TCL_EVENTS_ALLOWED
        /* Process Tcl events. We need to do it somewhere,
         * but not too often. This place looks good...
         */
        if (ipars->process_tcl_events)
            while (Tcl_DoOneEvent(TCL_FILE_EVENTS |
                                  TCL_TIMER_EVENTS |
                                  TCL_DONT_WAIT)) {;}
#endif
    }
}

static Scan_status is_scan_complete(
    const Mc_scan_parameters *mpars,
    const unsigned max_points_integrated,
    const time_t integration_start_time,
    const unsigned n_jes_points,
    const unsigned n_tmass_points,
    const double perm_weights[5*N_JET_PERMUTATIONS][MAX_JES_POINTS],
    const unsigned n_points_integrated[5*N_JET_PERMUTATIONS],
    const norm_accumulator *perm_results,
    int current_perm_mask[5*N_JET_PERMUTATIONS])
{
    static norm_accumulator *results = 0;
    static unsigned n_results = 0;

    static unsigned *worst_perm = 0;
    static unsigned n_worst_perm = 0;

    static double *worst_precision = 0;
    static unsigned n_worst_precision = 0;

    const unsigned res_points = n_tmass_points*n_jes_points;
    unsigned ires, iperm;

    if (max_points_integrated < mpars->min_points)
        return SCAN_STATUS_CONTINUE;

    /* Allocate and clear the current result space and other memory buffers */
    get_static_memory((void **)&results, sizeof(norm_accumulator),
                      &n_results, res_points);
    get_static_memory((void **)&worst_perm, sizeof(unsigned),
                      &n_worst_perm, res_points);
    get_static_memory((void **)&worst_precision, sizeof(double),
                      &n_worst_precision, res_points);
    for (ires=0; ires<res_points; ++ires)
    {
        norm_reset(results + ires);
	worst_perm[ires] = 5*N_JET_PERMUTATIONS;
	worst_precision[ires] = 0.0;
    }

    /* Build the current result and calculate precision for each permutation
       We track the permutation with the worst precision for each 
       result point.
    */
    for (iperm=0; iperm<5*N_JET_PERMUTATIONS; ++iperm)
	if (perm_weights[iperm][n_jes_points/2] > 0.0)
	{
	    const norm_accumulator *result_buffer = 
		perm_results + iperm*res_points;
	    const double* w = &perm_weights[iperm][0];
            const unsigned n_integ = n_points_integrated[iperm];

	    for (ires=0; ires<res_points; ++ires)
            {
                const unsigned ijes = ires % n_jes_points;
                norm_accumulator tmp = result_buffer[ires];
                assert(tmp.ntries == n_integ);
                norm_scale(&tmp, w[ijes]);
                norm_add_nocorr(results+ires, &tmp);
                {
                    const double err = norm_error(&tmp);
                    if (err > worst_precision[ires])
                    {
                        worst_perm[ires] = iperm;
                        worst_precision[ires] = err;
                    }
                }
            }
        }

    /* The following code checks whether
     * the precision target is satisfied.
     * Figure out which
     * permutations are most important,
     * and turn the others off...
     */
    if (mpars->precision_target > 0.0 &&
        mpars->precision_fraction > 0.0)
    {
        static Likelihood_value *likelihoods = 0;
        static unsigned n_likelihoods = 0;
	
        unsigned n_non_0 = 0;
	
	get_static_memory((void **)&likelihoods, sizeof(Likelihood_value),
			  &n_likelihoods, res_points);

	for (ires=0; ires<res_points; ++ires)
	{
	    const norm_accumulator *r = results + ires;
	    const double thisvalue = norm_value(r);
	    assert(thisvalue >= 0.0);
	    if (thisvalue > 0.0)
	    {
		likelihoods[n_non_0].value = thisvalue;
		likelihoods[n_non_0].err = norm_error(r);
		likelihoods[n_non_0].worst_perm = worst_perm[ires];
		++n_non_0;
	    }
	}

	if (n_non_0)
	{
	    int i, satisfied = 1;
	    double total_likeli = 0.0, likeli_cdf = 0.0;
            Permutation_counter worst_permutation_counts[5*N_JET_PERMUTATIONS];

            for (iperm=0; iperm<5*N_JET_PERMUTATIONS; iperm++)
            {
                worst_permutation_counts[iperm].iperm = iperm;
                worst_permutation_counts[iperm].worst_counts = 0;
            }

	    qsort(likelihoods, n_non_0, sizeof(Likelihood_value),
		  sort_Likelihood_value_by_value_decr);
	    for (i=n_non_0-1; i>=0; --i)
		total_likeli += likelihoods[i].value;
	    total_likeli *= mpars->precision_fraction;

	    for (ires=0; ires<n_non_0 && 
		         likeli_cdf < total_likeli; ++ires)
	    {
		if (likelihoods[ires].err/likelihoods[ires].value >
		    mpars->precision_target)
		{
                    const unsigned worst_perm = likelihoods[ires].worst_perm;
                    assert(worst_perm < 5*N_JET_PERMUTATIONS);
		    worst_permutation_counts[worst_perm].worst_counts++;
		    satisfied = 0;
		}
		likeli_cdf += likelihoods[ires].value;
	    }

	    if (satisfied)
		return SCAN_STATUS_OK;
            else
            {
                unsigned worst_counts;

                qsort(worst_permutation_counts, 5*N_JET_PERMUTATIONS,
                      sizeof(Permutation_counter),
                      sort_Permutation_counter_by_worst_counts_decr);
                worst_counts = worst_permutation_counts[0].worst_counts;
                assert(worst_counts);

                memset(current_perm_mask, 0, 5*N_JET_PERMUTATIONS*sizeof(int));
                current_perm_mask[worst_permutation_counts[0].iperm] = 1;
                for (iperm=1; iperm < 5*N_JET_PERMUTATIONS; ++iperm) 
                {
                    const double n_worst = worst_permutation_counts[iperm].worst_counts;
                    if (n_worst > 0.0 && worst_counts/n_worst < mpars->worst_perm_cutoff)
                        current_perm_mask[worst_permutation_counts[iperm].iperm] = 1;
                }
	    }
	}
        else
            if (max_points_integrated >= mpars->max_zeroprob_pts)
                return SCAN_STATUS_ZEROPROB;
    }
    else
    {
        /* Just check that this event has at least one
         * non-zero probability point
         */
        unsigned ires, n_non_0 = 0;
        for (ires=0; ires<res_points && !n_non_0; ++ires)
        {
            const norm_accumulator *r = results + ires;
            const double thisvalue = norm_value(r);
            assert(thisvalue >= 0.0);
            if (thisvalue > 0.0)
                ++n_non_0;
        }
        if (n_non_0 == 0)
            if (max_points_integrated >= mpars->max_zeroprob_pts)
                return SCAN_STATUS_ZEROPROB;
    }
    
    if (max_points_integrated >= mpars->max_points)
        return SCAN_STATUS_MAXPOINTS;
    
    if ((unsigned)(time(NULL) - integration_start_time) >
        mpars->max_event_seconds)
        return SCAN_STATUS_TIMELIMIT;

    return SCAN_STATUS_CONTINUE;
}

static unsigned init_dim_map(const unsigned mc_dim_mask,
                             int random_dim_map[N_MC_DIMS])
{
    unsigned idim, n_used = 0;

    for (idim=0; idim<N_MC_DIMS; ++idim)
    {
        if (mc_dim_mask & (1U << idim))
            random_dim_map[idim] = n_used++;
        else
            random_dim_map[idim] = -1;
    }
    return n_used;
}

static void mc_one_top_mass_scan(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const double *jes_points, unsigned n_jes_points,
    const double mwhad,
    const double param, const double mt, const unsigned itop,
    const jet_info *q, const jet_info *qbar,
    const jet_info *bhad_in_0, const jet_info *blep_in_0,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double hepg_delta_mthad, const double hepg_delta_mtlep,
    const double hepg_mwlep,
    const double ttbarPx_unsc, const double ttbarPy_unsc,
    const float *r, const int random_dim_map[N_MC_DIMS],
    double weight, const Leptonic_side_mask *lmask,
    Prob_to_loose_parton *prob_to_loose_parton,
    norm_accumulator *mt_results)
{
    const double mwsq = mwhad*mwhad;
    const double mthad_min = mwhad + bhad_in_0->m + MIN_MT_MW_DIFFERENCE_HAD;
    const double mt_max = ipars->cms_energy/2.0 - 0.5;
    const double t_width = getTopWidth(mt, ipars->nominal_w_mass, getBQuarkMass());
    double mthadsq;
    double event_weight[MAX_JES_POINTS];
    unsigned ijes;

    assert(weight > 0.0);

    /* Reset the result */
    assert(n_jes_points <= MAX_JES_POINTS);
    for (ijes=0; ijes<n_jes_points; ++ijes)
        event_weight[ijes] = 0.0;

    /* Get the hadronic top mass point */
    assert(random_dim_map[MC_DIM_THAD] >= 0);
    mthadsq = mt*(t_width*tan(M_PI*(r[random_dim_map[MC_DIM_THAD]] - 0.5)) + mt);
    /* 
    if (random_dim_map[MC_DIM_THAD] >= 0)
        mthadsq = mt*(t_width*tan(M_PI*(r[random_dim_map[MC_DIM_THAD]] - 0.5)) + mt);
    else
        mthadsq = (mt + hepg_delta_mthad)*(mt + hepg_delta_mthad);
    */

    if (mthadsq >= mthad_min*mthad_min && mthadsq < mt_max*mt_max)
    {
        hadron_side_solution hsols[2];
        int ihsol, nsols;
        jet_info missing_b;

        const double mthad = sqrt(mthadsq);
        weight /= cauchy_density(mthadsq, mt*mt, mt*t_width);

        /* Shut up some stupid compiler messages */
        missing_b.pt = 0.0;
        missing_b.p = 0.0;

        const int randomize_bhad_dir = 
            random_dim_map[MC_DIM_BHAD_ETA] >= 0 &&
            random_dim_map[MC_DIM_BHAD_PHI] >= 0;
        const int randomize_blep_dir = 
            random_dim_map[MC_DIM_BLEP_ETA] >= 0 &&
            random_dim_map[MC_DIM_BLEP_PHI] >= 0;

        const jet_info *bhad_in = bhad_in_0;
        const jet_info *blep_in = blep_in_0;

        /* If the hadronic b is an extra jet, we need to generate
         * its direction first. Otherwise, solve the hadronic side
         * equations and pick a reasonable angular distribution prior.
         */
        if (bhad_in->is_extra)
        {
            assert(0);

            assert(randomize_bhad_dir);
            random_dir_jet(&missing_b, bhad_in->m, 1,
                           r[random_dim_map[MC_DIM_BHAD_ETA]],
                           r[random_dim_map[MC_DIM_BHAD_PHI]]);
            bhad_in = &missing_b;
        }

        nsols = solve_hadronic_side(
            q->px, q->py, q->pz, q->m,
            qbar->px, qbar->py, qbar->pz, qbar->m,
            bhad_in->px, bhad_in->py, bhad_in->pz, bhad_in->m,
            mthad, mwsq, param, ipars->debug_level, hsols);

        if (nsols > 0 && blep_in->is_extra)
        {
            assert(0);

            /* Now we know that we have to generate the leptonic b direction */
            assert(randomize_blep_dir);
            assert(!bhad_in->is_extra);
            random_dir_jet(&missing_b, blep_in->m, 2,
                           r[random_dim_map[MC_DIM_BLEP_ETA]],
                           r[random_dim_map[MC_DIM_BLEP_PHI]]);
            blep_in = &missing_b;
        }

        if (nsols > 0 && (bhad_in->is_extra || blep_in->is_extra))
        {
            /* The integration is now over d cos(theta) instead of d eta.
             * Correct the phase space accordingly.
             */
             const double sintheta = missing_b.pt/missing_b.p;
             assert(0);
             assert(sintheta > 0.0);
             weight /= (sintheta*sintheta);
        }

        if (nsols > 0)
        {
            v3_obj ttbar_pt;
            double mtlep, mtlepsq;

            /* We can now randomize the b directions */
            const int redo_bhad = randomize_bhad_dir && !bhad_in->is_extra;
            const int redo_blep = randomize_blep_dir && !blep_in->is_extra;
            jet_info newbhad_store, newblep_store;

            const jet_info *bhad = (redo_bhad && weight > 0.0) ? &newbhad_store : bhad_in;
            const jet_info *blep = (redo_blep && weight > 0.0) ? &newblep_store : blep_in;

            if (redo_bhad && weight > 0.0)
            {
                const int rand_mbhad = random_dim_map[MC_DIM_MBHAD] >= 0 ||
                    ipars->had_b_jet_mass < 0.0;
                int newsols;
                double density = 0.0;

                assert(nsols == 1);
                randomize_jet_dir(bhad_in, hsols[0].bP, 1,
                                  r[random_dim_map[MC_DIM_BHAD_ETA]],
                                  r[random_dim_map[MC_DIM_BHAD_PHI]],
                                  rand_mbhad, &newbhad_store, &density);
                assert(density > 0.0);
                weight /= density;

                newsols = solve_hadronic_side(
                    q->px, q->py, q->pz, q->m,
                    qbar->px, qbar->py, qbar->pz, qbar->m,
                    bhad->px, bhad->py, bhad->pz, bhad->m,
                    mthad, mwsq, param,
                    ipars->debug_level, hsols);
                if (newsols == 1)
                {;}
                else
                {
                    assert(newsols == 0);
                    weight = 0.0;
                }
            }

            if (redo_blep && weight > 0.0)
            {
                /* The formula below simply uses reconstructed
                 * leptonic b momentum. The hope is that it is
                 * not going to be too different from the solutions,
                 * so that the ratios are not going to have extremely
                 * large variance. The evaluation of the density
                 * centered at the new point must be delayed
                 * until the complete event kinematics is known
                 * because we need to know the parton Pt.
                 */
                const int rand_mblep = random_dim_map[MC_DIM_MBLEP] >= 0 ||
                    ipars->lep_b_jet_mass < 0.0;
                double density = 0.0;
                randomize_jet_dir(blep_in, blep_in->p, 2,
                                  r[random_dim_map[MC_DIM_BLEP_ETA]],
                                  r[random_dim_map[MC_DIM_BLEP_PHI]],
                                  rand_mblep, &newblep_store, &density);
                assert(density > 0.0);
                weight /= density;
            }

            /* Randomize the ttbar Pt */
            if (random_dim_map[MC_DIM_TTBAR_PT_MAG] >= 0 &&
                random_dim_map[MC_DIM_TTBAR_PT_PHI] >= 0 &&
                weight > 0.0)
            {
                ttbar_pt = generate_ttbar_pt_fromrand(
                    r[random_dim_map[MC_DIM_TTBAR_PT_MAG]],
                    r[random_dim_map[MC_DIM_TTBAR_PT_PHI]]);
                weight *= ttbar_pt_weight(mt, r[random_dim_map[MC_DIM_TTBAR_PT_MAG]]);
            }
            else
                ttbar_pt = v3(ttbarPx_unsc, ttbarPy_unsc, 0.0);

            /* Randomize the leptonic top mass */
            if (random_dim_map[MC_DIM_TLEP] >= 0 && weight > 0.0)
                mtlepsq = mt*(t_width*tan(M_PI*(r[random_dim_map[MC_DIM_TLEP]] - 0.5)) + mt);
            else
                mtlepsq = (mt + hepg_delta_mtlep)*(mt + hepg_delta_mtlep);
            if (mtlepsq <= blep->m*blep->m || mtlepsq >= mt_max*mt_max)
            {
                mtlep = blep->m;
                weight = 0.0;
            }
            else
            {
                mtlep = sqrt(mtlepsq);
                if (mtlep <= blep->m)
                    weight = 0.0;
                else
                    weight /= cauchy_density(mtlepsq, mt*mt, mt*t_width);
            }

            /* Cycle over the hadronic side solutions */
            for (ihsol=0; ihsol<nsols; ++ihsol)
            {
                double solweight = weight;
                v3_obj lp = v3(lPx, lPy, lPz);
                const hadron_side_solution *hsol = hsols + ihsol;

                /* Check the leptonic side mask */
                if (lmask && solweight > 0.0)
                {
                    const double tlepPx = ttbar_pt.x - 
                        (hsol->qPx + hsol->qbarPx + hsol->bPx);
                    const double tlepPy = ttbar_pt.y - 
                        (hsol->qPy + hsol->qbarPy + hsol->bPy);
                    const int passes = leptonic_side_mask_value(
                        lmask, mtlep, hypot(tlepPx, tlepPy),
                        atan2(tlepPy, tlepPx));

                    /* Stop processing if the mask is not set */
                    if (!passes)
                        solweight = 0.0;
                }

                /* Special treatment for tau+jets events */
                if (random_dim_map[MC_DIM_TAU] >= 0 && solweight > 0.0)
                {
                    const double leptonPt = Pt(lp);
                    const double leptonMom = mom(lp);
                    const double dp_over_dpt = leptonMom/leptonPt;
                    const double topPx = ttbar_pt.x -
                        (hsol->qPx + hsol->qbarPx + hsol->bPx);
                    const double topPy = ttbar_pt.y - 
                        (hsol->qPy + hsol->qbarPy + hsol->bPy);
                    const double maxTauPt = max_lepton_pt(
                        topPx, topPy, mtlep*mtlep, lp.x, lp.y, lp.z);
                    const double minTauPt = leptonPt*TAU_LOWER_LIMIT_FACTOR;
                    const double tauWeight = maxTauPt - minTauPt;
                    const double tauPt = minTauPt + tauWeight*
                        r[random_dim_map[MC_DIM_TAU]];
                    const double tauMom = tauPt * dp_over_dpt;
                    const double tauEnergy = hypot(tauMom, TAU_LEPTON_MASS);
                    const double y = leptonMom/tauEnergy;

                    /* Hardwired constant... Be afraid! */
                    /* const double leptonPtCut = 20.0; */
                    /* const double ycut = leptonPtCut*dp_over_dpt/tauEnergy;*/

                    /* The dp_over_dpt factor is included below because
                     * the grid integrates over tau pt instead of p.
                     *
                     * Additional 1/tauEnergy factor is included because 
                     * "tau_daughter_spectrum" is a density in y,
                     * not in lepton momentum.
                     */
                    const double taufactor = 
                        tau_daughter_spectrum(y, mt, isMu ? 1 : 0) *
                        tauWeight * dp_over_dpt / tauEnergy;
                    solweight *= taufactor;

                    /* We now need to set "lp" to the value of tau
                     * momentum. The best way to do this is not obvious:
                     * lepton mass is ignored in various kinematic
                     * calculations. Should we set the magnitude of "lp"
                     * to tau momentum or tau energy? Setting this to
                     * tau energy can be done like this:
                     *
                     * lp = div3(lp, y);
                     */
                    lp = div3(lp, leptonMom/tauMom);
                }

                /* We have now randomized everything except
                 * leptonic W mass (or neutrino Pz). Go for it.
                 */
                if (solweight > 0.0)
                    mc_integrate_w_mass(
                        ipars, mpars, jes_points, n_jes_points,
                        hsol, q, qbar, bhad, blep, blep_in,
                        redo_blep, mt, itop,
                        lp.x, lp.y, lp.z, leptonCharge, isMu,
                        hepg_mwlep, ttbar_pt.x, ttbar_pt.y, mtlep,
                        solweight, prob_to_loose_parton, event_weight);
            }
        }
    }

    /* Fill the result accumulator */
    for (ijes=0; ijes<n_jes_points; ++ijes)
        norm_accumulate(mt_results+ijes, event_weight[ijes]);
}

int fill_mc_scan_parameters(Mc_scan_parameters *mpars,
                            const double check_factor,
                            const double mwsq_safety_margin,
                            const double precision_fraction,
                            const double precision_target,
                            const double worst_perm_cutoff,
                            const double nominal_q_mass,
                            const double nominal_b_mass,
                            const double abs_param_min,
                            const double abs_param_max,
                            const double rel_param_min,
                            const double rel_param_max,
                            const double lside_pt_max,
                            const double lside_nuz_min,
                            const double lside_nuz_max,
                            const double lside_b_tailsize,
                            const double lside_b_conesize,
                            const unsigned had_mt_gen_type,
                            const unsigned lep_mt_gen_type,
                            const unsigned fudge_to_treelevel,
                            const unsigned min_points,
                            const unsigned max_points,
                            const unsigned max_zeroprob_pts, 
                            const unsigned max_event_seconds,
                            const unsigned lside_mask_nPt,
                            const unsigned lside_mask_nPhi,
                            const unsigned lside_max_points,
                            const unsigned lside_max_seconds,
                            const unsigned mc_dim_mask,
                            const unsigned efficiency_mask,
                            const int random_gen_param,
                            const int lside_rnd_param,
                            const N_d_random_method rmethod,
                            const N_d_random_method lside_rmethod)
{
    memset(mpars, 0, sizeof(Mc_scan_parameters));

    /* Check arguments for validity */
    if (check_factor <= 1.0)                                  return 1;
    if (mwsq_safety_margin <= 0.0)                            return 2;
    if (max_points == 0)                                      return 3;
    if ((unsigned)(check_factor*min_points) == min_points)    return 4;
    if (max_event_seconds == 0)                               return 5;
    if (precision_fraction < 0.0 || precision_fraction > 1.0) return 6;
    if (nominal_b_mass < 0.0)                                 return 7;
    if (abs_param_max <= abs_param_min)                       return 8;
    if (rel_param_max <= rel_param_min)                       return 9;
    if (nominal_q_mass < 0.0)                                 return 10;
    if (rmethod == N_D_RANDOM_INVALID)                        return 11;

    mpars->check_factor = check_factor;
    mpars->mwsq_safety_margin = mwsq_safety_margin;
    mpars->precision_fraction = precision_fraction;
    mpars->precision_target = precision_target;
    mpars->worst_perm_cutoff = worst_perm_cutoff;
    mpars->nominal_q_mass = nominal_q_mass;
    mpars->nominal_b_mass = nominal_b_mass;
    mpars->abs_param_min = abs_param_min;
    mpars->abs_param_max = abs_param_max;
    mpars->rel_param_min = rel_param_min;
    mpars->rel_param_max = rel_param_max;

    mpars->lside_pt_max = lside_pt_max;
    mpars->lside_nuz_min = lside_nuz_min;
    mpars->lside_nuz_max = lside_nuz_max;
    mpars->lside_b_tailsize = lside_b_tailsize;
    mpars->lside_b_conesize = lside_b_conesize;

    mpars->had_mt_gen_type = had_mt_gen_type;
    mpars->lep_mt_gen_type = lep_mt_gen_type;
    mpars->fudge_to_treelevel = fudge_to_treelevel;
    mpars->min_points = min_points;
    mpars->max_points = max_points;
    mpars->max_zeroprob_pts = max_zeroprob_pts;
    mpars->max_event_seconds = max_event_seconds;

    mpars->lside_mask_nPt = lside_mask_nPt;
    mpars->lside_mask_nPhi = lside_mask_nPhi;
    mpars->lside_max_points = lside_max_points;
    mpars->lside_max_seconds = lside_max_seconds;

    mpars->mc_dim_mask = mc_dim_mask;
    mpars->efficiency_mask = efficiency_mask;
    mpars->random_gen_param = random_gen_param;
    mpars->lside_rnd_param = lside_rnd_param;

    mpars->random_method = rmethod;
    mpars->lside_random = lside_rmethod;

    return 0;
}

void print_mc_scan_parameters(const Mc_scan_parameters *mpars, FILE *stream)
{
    fprintf(stream,
            "MC scan parameters: "
            "check_factor = %g, "
            "mwsq_safety_margin = %g, "
            "precision_fraction = %g, "
            "precision_target = %g, "
            "worst_perm_cutoff = %g, "
            "nominal_q_mass = %g, "
            "nominal_b_mass = %g, "
            "abs_param_min = %g, "
            "abs_param_max = %g, "
            "rel_param_min = %g, "
            "rel_param_max = %g, "

            "lside_pt_max = %g, "
            "lside_nuz_min = %g, "
            "lside_nuz_max = %g, "
            "lside_b_tailsize = %g, "
            "lside_b_conesize = %g, "

            "had_mt_gen_type = %u, "
            "lep_mt_gen_type = %u, "
            "fudge_to_treelevel = %u, "
            "min_points = %u, "
            "max_points = %u, "
            "max_zeroprob_pts = %u, "
            "max_event_seconds = %u, "

            "lside_mask_nPt = %u, "
            "lside_mask_nPhi = %u, "
            "lside_max_points = %u, "
            "lside_max_seconds = %u, "

            "mc_dim_mask = %u, "
            "efficiency_mask = %u, "
            "random_gen_param = %d, "
            "lside_rnd_param = %d, "
            "random_method = %u, "
            "lside_random = %u",

            mpars->check_factor,
            mpars->mwsq_safety_margin,
            mpars->precision_fraction,
            mpars->precision_target,
            mpars->worst_perm_cutoff,
            mpars->nominal_q_mass,
            mpars->nominal_b_mass,
            mpars->abs_param_min,
            mpars->abs_param_max,
            mpars->rel_param_min,
            mpars->rel_param_max,

            mpars->lside_pt_max,
            mpars->lside_nuz_min,
            mpars->lside_nuz_max,
            mpars->lside_b_tailsize,
            mpars->lside_b_conesize,

            mpars->had_mt_gen_type,
            mpars->lep_mt_gen_type,
            mpars->fudge_to_treelevel,
            mpars->min_points,
            mpars->max_points,
            mpars->max_zeroprob_pts,
            mpars->max_event_seconds,

            mpars->lside_mask_nPt,
            mpars->lside_mask_nPhi,
            mpars->lside_max_points,
            mpars->lside_max_seconds,

            mpars->mc_dim_mask,
            mpars->efficiency_mask,
            mpars->random_gen_param,
            mpars->lside_rnd_param,
            (unsigned)mpars->random_method,
            (unsigned)mpars->lside_random);
    fprintf(stream, "\n");
}

#define check_coverage(dim, coverage) do {\
    if (random_dim_map[dim] >= 0)\
    {\
        const float fmin = (1.0 - coverage)/2.0;\
        if (r[random_dim_map[dim]] < fmin || \
            r[random_dim_map[dim]] > 1.f - fmin)\
            return 0;\
    }\
} while(0);

static int is_random_covered(
    const integ_parameters *ipars,
    const float r[N_MC_DIMS],
    const int random_dim_map[N_MC_DIMS])
{
    check_coverage(MC_DIM_WHAD, ipars->whad_coverage);
    check_coverage(MC_DIM_THAD, ipars->thad_coverage);
    check_coverage(MC_DIM_TLEP, ipars->tlep_coverage);

    return 1;
}

static void fill_lepton_solution_info(
    Lepton_solution_info *info,
    const lepton_side_solution *sol,
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const Common_leptonic_params *params,
    const double *jes_points, const unsigned n_jes_points,
    const jet_info *blep_orig, Prob_to_loose_parton *prob_to_loose_parton)
{
    const particle_obj blep_solved = particle(v3(sol->bPx, sol->bPy,
                                                 sol->bPz), sol->mb);
    const particle_obj nu_solved   = particle(v3(sol->nux, sol->nuy,
                                                 sol->nuz), 0.0);
    particle_obj pblep, nu, parl;
    double matrel = 1.0, matrel_gg = 0.0;
    double structfun = 1.0, structfun_gg = 0.0;
    double point_weight = params->toplevel_weight;
    int status = 1;
    unsigned ijes;

    /* Figure out the weight due to the b angular transfer function */
    if (params->estimate_blep_angular_tf)
    {
        if ((mpars->mc_dim_mask & (1U << MC_DIM_MBLEP)) ||
            ipars->lep_b_jet_mass < 0.0)
        {
            /*
             * point_weight *= angular_transfer_function(
             *   2, params->olddir_blep, blep_solved.p, blep_solved.m);
             */
        }
        else
        {
            assert(0);
        }
    }

    /* Convert the leptonic side to tree-level if requested */
    if (mpars->fudge_to_treelevel && blep_solved.m != mpars->nominal_b_mass)
        status = redecay_top_quark(params->l_94, nu_solved, blep_solved,
                                   0.0,          0.0,       mpars->nominal_b_mass,
                                   &parl,        &nu,       &pblep);
    if (status)
    {
        parl  = params->l_94;
        nu    = nu_solved;
        pblep = blep_solved;
    }

    if (ipars->debug_level >= 30)
    {
        const particle_obj whad0 = sum4(params->tree_q, params->tree_qbar);
        const particle_obj thad  = sum4(whad0, params->tree_bhad);
        const particle_obj wlep0 = sum4(parl, nu);
        const particle_obj tlep  = sum4(wlep0, pblep);
        const particle_obj ttbar = sum4(tlep, thad);
        printf("  q    : %s\n", string4(params->tree_q));
        printf("  qbar : %s\n", string4(params->tree_qbar));
        printf("  bhad : %s\n", string4(params->tree_bhad));
        printf("  whad : %s\n", string4(whad0));
        printf("  thad : %s\n", string4(thad));
        printf("  blep : %s\n", string4(pblep));
        printf("  l    : %s\n", string4(parl));
        printf("  nu   : %s\n", string4(nu));
        printf("  wlep : %s\n", string4(wlep0));
        printf("  tlep : %s\n", string4(tlep));
        printf("  ttbar: %s\n", string4(ttbar));
    }

    /* Phase space weights */
    ttbar_phase_space(parl, nu, pblep,
                      params->q_solved, params->qbar_solved,
                      params->bhad_solved,
                      &info->jaco_mw, &info->jaco_pnuz);

    /* Matrix element weights */
    if (ipars->matrel_code & MASK_KLEISS_STIRLING)
        kleiss_stirling_weights(
            params->tree_q, params->tree_qbar, params->tree_bhad,
            pblep, parl, nu, params->lCharge,
            ipars->nominal_w_mass, ipars->nominal_w_width,
            params->mtscan, &matrel, &matrel_gg);
    else if (ipars->matrel_code & MASK_MAHLON_PARKE)
        matrel = mahlon_parke_weight(
            params->tree_q, params->tree_qbar, params->tree_bhad,
            pblep, parl, nu, params->lCharge,
            ipars->nominal_w_mass, ipars->nominal_w_width,
            params->mtscan);

    /* Parton distribution function weights */
    if (ipars->matrel_code & MASK_PROTON_STRUCT)
        pdfterm_qqbar(params->thad,
                      sum4(blep_solved, sum4(params->l_94, nu_solved)),
                      ipars->cms_energy, &structfun, &structfun_gg);
    point_weight *= (matrel*structfun + matrel_gg*structfun_gg);

    /* Scan the JES in the transfer function.
     * Note the use of ID94 leptonic b.
     */
    if (blep_orig->is_extra)
    {
        assert(0);

        /* printf("In fill_lepton_solution_info: extra blep\n"); */
        const double syserr = parton_syserr(&blep_solved, 2);
        assert(prob_to_loose_parton);
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*syserr;
            info->jes_weights[ijes] = point_weight*params->had_tfprod[ijes]*
                prob_to_loose_parton(&blep_solved, scale, 2);
        }
    }
    else
    {
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            double lep_tf = 0.0;
            const double jes = 1.0 + jes_points[ijes]*blep_orig->syserr;
            const double corr = jes_points[ijes]*blep_orig->derr_dpt*blep_orig->pt;
            const double f = jes + corr;

            if (f > 0.0)
            {
                lep_tf = f*transfer_function(blep_solved, blep_orig, jes, 2);
                if (lep_tf > 0.0)
                    lep_tf *= single_parton_eff(blep_solved, 2, jes_points[ijes], blep_orig->cuterr);
                else
                    lep_tf = 0.0;
            }
            info->jes_weights[ijes] = point_weight*lep_tf*params->had_tfprod[ijes];
        }
    }

    info->lsol = *sol;
}

static int mc_w_mass_range(
    const double mwsq_safety_margin,
    const double i_topPx, const double i_topPy,
    const double i_lPx, const double i_lPy, const double ePz,
    const double i_bPx, const double i_bPy, const double i_bPz,
    const double mt, const double mb,
    const int max_iterations, const int debug_level,
    const W_point_info *start_info, const double mwsq_nosol,
    const double tpzmin, const double tpzmax,
    lepton_side_solution *mwmin)
{
    /* At the mW extrema the phase space Jacobian will go to 0.
     * If b is massive, finding this point analytically seems
     * to be hopeless. We'll just try the gradient descent instead.
     * Unfortunately, the W mass minimum is not the only reason
     * why the Jacobian can become 0.
     *
     * It is convenient to rotate the coordinate system so that
     * the b direction lies in the yz plane. Then the x component
     * of the neutrino momentum can be found immediately, and all
     * subsequent calculations are simplified.
     */

    /* Rotate the coordinates */
    const v3_obj zAxis      = v3(0.0, 0.0, 1.0);
    const v3_obj yPrimeAxis = direction3(v3(i_bPx, i_bPy, 0.0));
    const v3_obj xPrimeAxis = vprod3(yPrimeAxis, zAxis);

    /* Calculate various things in the new coordinate system */
    const v3_obj i_topPt   = v3(i_topPx, i_topPy, 0.0);
    const v3_obj i_l       = v3(i_lPx, i_lPy, ePz);
    const double tPx       = sprod3(xPrimeAxis, i_topPt);
    const double tPyGoal   = sprod3(yPrimeAxis, i_topPt);
    const double ePx       = sprod3(xPrimeAxis, i_l);
    const double ePy       = sprod3(yPrimeAxis, i_l);
    const double ee        = mom(i_l);
    const double i_bPt     = hypot(i_bPx, i_bPy);
    const double i_Pb      = hypot(i_bPt, i_bPz);
    const double cby       = i_bPt/i_Pb;
    const double cbz       = i_bPz/i_Pb;
    const double nuPx      = tPx - ePx;
    const double wPx       = tPx;
    const double mbsquared = mb*mb;
    const double mtsqGoal  = mt*mt;

    const double mwsq_start = start_info->info[0].lsol.mwsq;

    int isol;
    double b[3], equation_matrix[3][3];
    double minWmassSq = DBL_MAX;

    assert(start_info->nlepsols);
    memset(mwmin, 0, sizeof(lepton_side_solution));

    /* Go down the slope */
    for (isol=0; isol<start_info->nlepsols; ++isol)
    {
        const lepton_side_solution *startsol = &start_info->info[isol].lsol;
        double pb   = startsol->pblep;
        double nuPy = startsol->nuy;
        double nuPz = startsol->nuz;
        double mwsq = startsol->mwsq;

        int niter;
        for (niter=0; niter<max_iterations; ++niter)
        {
            int ifail = 0;
            int nrows1 = 3;
            int IPIV[3];
            int NRHS = 1;

            const double wPy = nuPy + ePy;
            const double wPz = nuPz + ePz;
            const double tPy = wPy + cby*pb;
            const double tPz = wPz + cbz*pb;
            const double enu = sqrt(nuPx*nuPx + nuPy*nuPy + nuPz*nuPz);
            const double eb = sqrt(pb*pb + mbsquared);
            const double et = ee + enu + eb;
            const double ebcubed = eb*eb*eb;
            const double beta = pb/eb;
            const double c0 = enu*et*(ee*nuPz - ePz*enu);
            const double c1 = enu*(cby*(ePz*nuPy - ePy*nuPz)*beta + 
                                   beta*beta*(ee*nuPz - ePz*enu) + 
                                   cbz*(-(ee*(cby*nuPy + cbz*nuPz)) + 
                                        (cby*ePy + cbz*ePz)*enu));
            const double dpb = (c1 + c0*mbsquared/ebcubed);
            const double d0 = -(nuPz*(ee*(cbz*nuPz*tPz + 
                                          cby*(ePy*nuPz + nuPy*nuPz + cbz*nuPy*pb)) + 
                                      cby*(ePy*nuPz - ePz*nuPy)*eb -
                                      ee*nuPz*beta*(ee + eb)));
            const double d1 = 2*nuPz*(cbz*ePz*tPz + 
                                      cby*(ePz*nuPy + ePy*(tPz - nuPz))) - 
                (ee*(cby*(ePy + nuPy) + cbz*(tPz + nuPz)) + cby*ePy*eb)*
                enu + cbz*ePz*enu*enu + 
                beta*(2*ee*nuPz*(-ePz + nuPz) + (ee*ee - 3*ePz*nuPz)*enu + 
                      ee*enu*enu + eb*(-2*ePz*nuPz + ee*enu));
            const double dnuPz = (d1 + d0/enu);
            const double e0 = -(nuPy*(ee*(cbz*nuPz*tPz + 
                                          cby*(ePy*nuPz + nuPy*nuPz + cbz*nuPy*pb)) + 
                                      cby*(ePy*nuPz - ePz*nuPy)*eb -
                                      ee*nuPz*beta*(ee + eb)));
            const double e1 = 2*nuPy*(cbz*ePz*tPz + 
                                      cby*(ePy*ePz + ePz*nuPy + cbz*ePy*pb)) - 
                cby*(ee*(tPz - ePz) - ePz*eb)*enu + 
                cby*ePz*enu*enu - nuPy*beta*(2*ee*ePz - 2*ee*nuPz + 
                                             2*ePz*eb + 3*ePz*enu);
            const double dnuPy = (e1 + e0/enu);

            const double jaco = enu*(beta*(-(enu*ePz*et) + ee*et*nuPz) + 
                                     cbz*(enu*ePz - ee*nuPz)*tPz + 
                                     cby*(ePz*et*nuPy - ePy*et*nuPz + 
                                          enu*ePy*tPz - ee*nuPy*tPz));
            const double mtsq = et*et - tPx*tPx - tPy*tPy - tPz*tPz;

            const double dmtsqdpb   = -2.0*(cby*tPy + cbz*tPz - beta*et);
            const double dmtsqdnuPz = 2.0*(nuPz*(ee + eb)/enu - ePz - cbz*pb);
            const double dmtsqdnuPy = 2.0*(nuPy*(ee + eb)/enu - ePy - cby*pb);

            const double dtPydpb   = cby;
            const double dtPydnuPz = 0.0;
            const double dtPydnuPy = 1.0;

            assert(enu > 0.0);
            mwsq = (ee+enu)*(ee+enu) - wPx*wPx - wPy*wPy - wPz*wPz;

            /* Fortran storage convention */
            equation_matrix[0][0] = dpb;
            equation_matrix[1][0] = dnuPz;
            equation_matrix[2][0] = dnuPy;
            equation_matrix[0][1] = dmtsqdpb;
            equation_matrix[1][1] = dmtsqdnuPz;
            equation_matrix[2][1] = dmtsqdnuPy;
            equation_matrix[0][2] = dtPydpb;
            equation_matrix[1][2] = dtPydnuPz;
            equation_matrix[2][2] = dtPydnuPy;

            b[0] = -jaco;
            b[1] = mtsqGoal - mtsq;
            b[2] = tPyGoal - tPy;

            /* Note that dgetrf_ destroys the original equation
             * matrix and stores its factors in its place
             */
            dgetrf_(&nrows1, &nrows1, &equation_matrix[0][0],
                    &nrows1, IPIV, &ifail);
            assert(ifail >= 0);
            if (ifail > 0)
            {
                /* Matrix is singular. Will deal with this
                 * case later, if it actually occurs
                 */
                assert(0);
            }
            nrows1 = 3;
            dgetrs_("N", &nrows1, &NRHS, &equation_matrix[0][0],
                    &nrows1, IPIV, b, &nrows1, &ifail, 1);
            assert(ifail == 0);

            /* Update the current values */
            pb   += b[0];
            nuPz += b[1];    
            nuPy += b[2];

            if (debug_level >= 30)
            {
                printf("mc_w_mass_range: after iteration %d, "
                       "pb = %g, nuPz = %g, nuPy = %g\n",
                       niter, pb, nuPz, nuPy);
                printf("Corresponding increments are %g, %g, and %g\n",
                       b[0], b[1], b[2]);
            }
 
            /* Break if the increments are small enough */
            if (fabs(b[0])/(fabs(pb) + 1.0) < RELATIVE_EPS_MINWMASS &&
                fabs(b[1])/(fabs(nuPz) + 1.0) < RELATIVE_EPS_MINWMASS &&
                fabs(b[2])/(fabs(nuPy) + 1.0) < RELATIVE_EPS_MINWMASS)
                break;
        }

        /* Check the number of iterations */
        if (niter >= max_iterations && debug_level >= 20)
        {
            static int max_warnings_iter = 1000;
            if (max_warnings_iter > 0)
            {
                printf("WARNING in mc_w_mass_range: iteration limit exceeded.\n");
                printf("Relative solution precisions are %g %g %g, requested %g.\n",
                       fabs(b[0])/(fabs(pb) + 1.0),
                       fabs(b[1])/(fabs(nuPz) + 1.0),
                       fabs(b[2])/(fabs(nuPy) + 1.0),
                       RELATIVE_EPS_MINWMASS);
                if (--max_warnings_iter == 0)
                    printf("Further occurrences of this message "
                           "will be suppressed\n");
                fflush(stdout);
            }
        }

        if (niter < max_iterations && pb > 0.0 && mwsq < minWmassSq)
        {
            const v3_obj out_nuT = sum3(mult3(xPrimeAxis,nuPx),
                                        mult3(yPrimeAxis,nuPy));
            mwmin->mwsq    = mwsq;
            mwmin->pblep   = pb;
            mwmin->bPx     = pb*i_bPx/i_Pb;
            mwmin->bPy     = pb*i_bPy/i_Pb;
            mwmin->bPz     = pb*i_bPz/i_Pb;
            mwmin->mb      = mb;
            mwmin->nux     = out_nuT.x;
            mwmin->nuy     = out_nuT.y;
            mwmin->nuz     = nuPz;
            mwmin->tlepz   = mwmin->bPz + mwmin->nuz + ePz;
            mwmin->mt      = mt;
            mwmin->fail    = 0;

            minWmassSq = mwsq;
        }
    }

    if (minWmassSq < mwsq_start)
    {
        /* This is probably the real minimum.
         * Make sure that the solution exists here.
         */
        const double delta_mwsq = mwsq_safety_margin > 0.0 ?
            mwsq_safety_margin : 0.01;
        lepton_side_solution lepsols[4];
        int i;

        const int nlsols = solve_leptonic_byNuPz(
            tPx, tPyGoal, ePx, ePy, ePz,
            0.0, i_bPt, i_bPz, mt, mb,
            mwmin->nuz, debug_level,
            max_iterations, lepsols);

        for (i=0; i<nlsols; ++i)
            if (fabs(lepsols[i].mwsq - minWmassSq) < delta_mwsq)
                return 1;
    }

    /* At this point we are in trouble. Will attempt to
     * find the minimum W mass point by trial and error.
     */
    if (debug_level >= 20)
    {
        static int max_warnings_noconv = 1000;
        if (max_warnings_noconv > 0)
        {
            printf("WARNING in mc_w_mass_range: iterations "
                   "did not converge to a true minimum.\n");
            printf("Switching to bisections.\n");
            if (--max_warnings_noconv == 0)
                printf("Further occurrences of this message "
                       "will be suppressed\n");
            fflush(stdout);
        }
    }
    {
        const double need_precision = mwsq_safety_margin > 0.0 ?
            mwsq_safety_margin*4.0 : 0.01;
        double lower_mwsq = mwsq_nosol;
        double upper_mwsq = mwsq_start;
        int nsolsupper = start_info->nlepsols;
        lepton_side_solution pbupper[4];

        for (isol=0; isol<nsolsupper; ++isol)
            pbupper[isol] = start_info->info[isol].lsol;

        while (upper_mwsq - lower_mwsq > need_precision)
        {
            const double halfmwsq = 0.5*(lower_mwsq + upper_mwsq);

            lepton_side_solution lepsols[4];
            const int nlsols = mc_solve_leptonic_side(
                mwsq_safety_margin, 1,
                tPx, tPyGoal, ePx, ePy, ePz,
                0.0, i_bPt, i_bPz,
                mt, mb, halfmwsq, debug_level,
                max_iterations, tpzmin, tpzmax, lepsols);
            if (nlsols > 0)
            {
                nsolsupper = nlsols;
                upper_mwsq = halfmwsq;
                memcpy(pbupper, lepsols, nlsols*sizeof(lepton_side_solution));
            }
            else
                lower_mwsq = halfmwsq;
        }

        mwmin->mwsq = upper_mwsq;
        mwmin->mt   = mt;
        mwmin->mb   = mb;
        mwmin->fail = 0;
        if (nsolsupper == 1)
        {
            mwmin->pblep   = pbupper->pblep;
            mwmin->nux     = pbupper->nux;
            mwmin->nuy     = pbupper->nuy;
            mwmin->nuz     = pbupper->nuz;
            mwmin->tlepz   = pbupper->tlepz;
        }
        else
        {
            /* First, try to solve using nuZ points in between.
             * If one of these solutions results in a better mass,
             * choose that solution.
             */
            int lowsol;
            lepton_side_solution trysols[4];
            lepton_side_solution bestsol = {0,0,0,0,0,0,0,0,0,0,0,0};
            bestsol.mwsq = DBL_MAX;

            for (lowsol=0; lowsol<nsolsupper-1; ++lowsol)
            {
                const double tryNuz = 0.5*(pbupper[lowsol].nuz +
                                           pbupper[lowsol+1].nuz);
                const int n_new = solve_leptonic_byNuPz(
                    i_topPx, i_topPy, i_lPx, i_lPy, ePz,
                    i_bPx, i_bPy, i_bPz, mt, mb, tryNuz,
                    debug_level, max_iterations, trysols);
                if (n_new > 0)
                    if (trysols[0].mwsq < bestsol.mwsq)
                        bestsol = trysols[0];
            }

            if (bestsol.mwsq < upper_mwsq)
            {
                *mwmin = bestsol;
                return 1;
            }
            else
            {
                /* Choose that original solution which
                 * provides the best pb match
                 */
                int i, index;
                const int blep_is_extra = fabs(i_Pb - 1.0) < 1.0e-3;
                d_i_pair sortem[4];

                if (debug_level > 0 && nsolsupper > 2)
                {
                    printf("WARNING in mc_w_mass_range: %d solutions "
                           "near the phase space boundary\n", nsolsupper);
                    fflush(stdout);
                }

                for (i=0; i<nsolsupper; ++i)
                {
                    if (blep_is_extra)
                        sortem[i].d = pb_distance(pbupper[i].pblep, 70.0);
                    else
                        sortem[i].d = pb_distance(pbupper[i].pblep, i_Pb);
                    sortem[i].i = i;
                }
                qsort(sortem, nsolsupper, sizeof(d_i_pair),
                      sort_d_i_pair_by_d_incr);
                index = sortem[0].i;
                mwmin->pblep   = pbupper[index].pblep;
                mwmin->nux     = pbupper[index].nux;
                mwmin->nuy     = pbupper[index].nuy;
                mwmin->nuz     = pbupper[index].nuz;
                mwmin->tlepz   = pbupper[index].tlepz;            
            }
        }

        /* Transform the solution back into
         * the original coordinate system
         */
        {
            const v3_obj out_nuT = sum3(mult3(xPrimeAxis,mwmin->nux),
                                        mult3(yPrimeAxis,mwmin->nuy));

            mwmin->bPx = mwmin->pblep*i_bPx/i_Pb;
            mwmin->bPy = mwmin->pblep*i_bPy/i_Pb;
            mwmin->bPz = mwmin->pblep*i_bPz/i_Pb;
            mwmin->nux = out_nuT.x;
            mwmin->nuy = out_nuT.y;
        }
        return 1;
    }
}

static double mc_split_mwsq_cdf(
    const double peak, const double hwhm,
    const double m1sq, const double m2sq,
    double *mwsq_binwidth)
{
    /* cdf value for m1sq */
    const double cdf1 = atan((m1sq-peak)/hwhm)/M_PI + 0.5;

    /* cdf value for m2sq */
    const double cdf2 = atan((m2sq-peak)/hwhm)/M_PI + 0.5;

    if (mwsq_binwidth)
        *mwsq_binwidth = fabs(cdf2 - cdf1)/2.0;

    /* inverse cdf value for the middle point */
    return hwhm*tan(M_PI*((cdf1 + cdf2)/2.0 - 0.5)) + peak;
}

static void mc_nuz_interval_integ(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const Lepton_solution_info *info1,
    const Lepton_solution_info *info2,
    double *jes_results, const unsigned n_jes_points)
{
    /*
    const double peak = ipars->nominal_w_mass*ipars->nominal_w_mass;
    const double hwhm = ipars->nominal_w_mass*ipars->nominal_w_width;
    const double bw1 = cauchy_density(info1->lsol.mwsq, peak, hwhm);
    const double bw2 = cauchy_density(info2->lsol.mwsq, peak, hwhm);
    */
    const double nuz_interval = fabs(info1->lsol.nuz - info2->lsol.nuz);
    const double *jes_weights1 = info1->jes_weights;
    const double *jes_weights2 = info2->jes_weights;

    assert(info1->lsol.mwsq <= info2->lsol.mwsq);

    {
        unsigned ijes;
        for (ijes=0; ijes<n_jes_points; ++ijes)
            jes_results[ijes] += nuz_interval*
                (info1->jaco_pnuz*jes_weights1[ijes] +
                 info2->jaco_pnuz*jes_weights2[ijes])/2.0;
    }
}

static void mc_add_mwsq_point(
    const integ_parameters *ipars,
    const Lepton_solution_info *info,
    const double weight,
    double *jes_results, const unsigned n_jes_points)
{
    const double peak = ipars->nominal_w_mass*ipars->nominal_w_mass;
    const double hwhm = ipars->nominal_w_mass*ipars->nominal_w_width;
    const double cd = cauchy_density(info->lsol.mwsq, peak, hwhm);
    const double w = info->jaco_mw*weight/cd;
    const double *jes_weights = info->jes_weights;

    unsigned ijes;
    for (ijes=0; ijes<n_jes_points; ++ijes)
        jes_results[ijes] += jes_weights[ijes]*w;
}

static void mc_integrate_w_mass(
    const integ_parameters *ipars,
    const Mc_scan_parameters *mpars,
    const double *jes_points, unsigned n_jes_points,
    const hadron_side_solution *hsol,
    const jet_info *q, const jet_info *qbar,
    const jet_info *bhad, const jet_info *blep,
    const jet_info *blep_orig, const unsigned is_blepdir_random,
    const double mtscan, const unsigned itop,
    const double lPx, const double lPy, const double lPz,
    const int leptonCharge, const int isMu,
    const double fixed_mwlep,
    const double ttbarPx, const double ttbarPy, const double mtlep,
    const double toplevel_weight,
    Prob_to_loose_parton *prob_to_loose_parton,
    double *event_weight)
{
    const size_t max_iterations = 0;
    const int w_mass_range_maxiter = 100;

    double tlepPx, tlepPy, tpzmin, tpzmax;
    int tree_status = 1, points_with_sols = 0;
    int ipt, max_point = -10, min_point = INT_MAX-20;
    unsigned idir, npoints;
    double w_reco_eff = 1.0;
    Common_leptonic_params params;

    static W_point_info *w_data = 0;
    static unsigned n_w_data = 0;

    /* Generate the W mass grid */
    static wgrid wmass_grid;
    static unsigned wgrid_intervals = 0;
    static double min_mwsq_cutoff = 0.0;
    static double old_mtlep = -1.0;

    if (ipars->debug_level >= 20)
        printf("mc_integrate_w_mass: mtscan = %g, toplevel_weight = %g\n",
               mtscan, toplevel_weight);

    assert(ipars->wlep_npoints > 0);
    if (wgrid_intervals != ipars->wlep_npoints)
    {
        if (wgrid_intervals > 0)
            cleanup_w_grid(&wmass_grid);
        init_w_grid(&wmass_grid, ipars->wlep_npoints, ipars->nominal_w_mass,
                    ipars->nominal_w_width, 0.98, WGRID_TRAPEZOID);
        wgrid_intervals = ipars->wlep_npoints;
        old_mtlep = -1.0;
    }

    assert(mtlep >= 0.0);
    if (mtlep != old_mtlep)
    {
        if (ipars->wlep_npoints > 1)
            slide_w_grid(&wmass_grid, mtlep, ipars->wlep_coverage);
        min_mwsq_cutoff = wmass_grid.points[0]*wmass_grid.points[0]/4.0;
        old_mtlep = mtlep;
    }

    /* Memory for the results at every point */
    npoints = w_grid_npoints(&wmass_grid);
    get_static_memory((void **)&w_data, sizeof(W_point_info),
                      &n_w_data, npoints);
    memset(w_data, 0, npoints*sizeof(W_point_info));

    /* Do the JES tf scan on the hadronic side.
     * Note that we use the original (ID94 level)
     * event in this scan.
     */
    hadronic_side_tf_product(jes_points, n_jes_points, hsol,
                             q, qbar, bhad, prob_to_loose_parton,
                             params.had_tfprod);

    /* Build the info common to every MW point */
    params.estimate_blep_angular_tf = is_blepdir_random;
    params.lCharge = leptonCharge;
    params.l_94 = particle(v3(lPx, lPy, lPz), 0.0);
    params.q_solved = particle(v3(hsol->qPx, hsol->qPy,
                                  hsol->qPz), hsol->mq);
    params.qbar_solved = particle(v3(hsol->qbarPx, hsol->qbarPy,
                                     hsol->qbarPz), hsol->mqbar);
    params.bhad_solved = particle(v3(hsol->bPx, hsol->bPy,
                                     hsol->bPz), hsol->mb);
    params.thad = sum4(params.bhad_solved, sum4(params.q_solved,
                                                params.qbar_solved));
    params.olddir_blep = v3(blep_orig->px, blep_orig->py, blep_orig->pz);
    params.mtscan = mtscan;

    /* Apply the W reconstruction efficiency */
    if (mpars->efficiency_mask & W_RECO_EFF_BIT)
        if (!q->is_extra && !qbar->is_extra)
            w_reco_eff = w_reco_efficiency(params.q_solved, params.qbar_solved);

    params.toplevel_weight = toplevel_weight * w_reco_eff;

    /* If requested, fudge hadronic side to tree level */
    if (mpars->fudge_to_treelevel &&
        (params.q_solved.m != mpars->nominal_q_mass ||
         params.qbar_solved.m != mpars->nominal_q_mass ||
         params.bhad_solved.m != mpars->nominal_b_mass))
        tree_status = redecay_top_quark(
            params.q_solved,       params.qbar_solved,    params.bhad_solved,
            mpars->nominal_q_mass, mpars->nominal_q_mass, mpars->nominal_b_mass,
            &params.tree_q,        &params.tree_qbar,     &params.tree_bhad);
    if (tree_status)
    {
        params.tree_q    = params.q_solved;
        params.tree_qbar = params.qbar_solved;
        params.tree_bhad = params.bhad_solved;
    }

    /* Estimate the limits for top Pz */
    leptonic_top_z_range(ipars->cms_energy, ttbarPx, ttbarPy,
                         params.thad.p.x, params.thad.p.y,
                         params.thad.p.z, params.thad.m,
                         mtlep, &tpzmin, &tpzmax);
    tlepPx = ttbarPx - params.thad.p.x;
    tlepPy = ttbarPy - params.thad.p.y;

    /* Process the special case when the leptonic W mass is specified */
    if (ipars->wlep_npoints == 1)
    {
        if (fixed_mwlep + blep->m + MIN_MT_MW_DIFFERENCE_LEP <= mtlep)
        {
            const double mwsq = fixed_mwlep*fixed_mwlep;
            lepton_side_solution lepsols[4];

            const int nlepsols = mc_solve_leptonic_side(
                mpars->mwsq_safety_margin, 1,
                tlepPx, tlepPy, lPx, lPy, lPz,
                blep->px, blep->py, blep->pz, mtlep,
                blep->m, mwsq, ipars->debug_level,
                max_iterations, tpzmin, tpzmax, lepsols);

            if (nlepsols > 0)
            {
                int isol;
                Lepton_solution_info *sol_info = w_data[0].info;
                w_data[0].nlepsols = nlepsols;

                qsort(lepsols, nlepsols, sizeof(lepton_side_solution),
                      sort_lepton_side_solution_by_nuz_incr);

                if (ipars->debug_level >= 30)
                    printf("\n*** In mc_integrate_w_mass, single point branch ***\n");
                for (isol=0; isol<nlepsols; ++isol)
                {
                    if (ipars->debug_level >= 30)
                        printf("Solution %d:\n", isol);
                    fill_lepton_solution_info(
                        sol_info+isol, lepsols+isol,
                        ipars, mpars, &params, jes_points,
                        n_jes_points, blep_orig, prob_to_loose_parton);
                }
                if (ipars->debug_level >= 30)
                    fflush(stdout);

                /* We have to make sure that we do not have
                 * a problem with the phase space Jacobian
                 */
                {
                    lepton_side_solution mwmin;
                    Lepton_solution_info mwmin_info;
                    double mwmin_value;

                    int status = mc_w_mass_range(mpars->mwsq_safety_margin,
                                                 tlepPx, tlepPy, lPx, lPy, lPz,
                                                 blep->px, blep->py, blep->pz,
                                                 mtlep, blep->m, w_mass_range_maxiter,
                                                 ipars->debug_level, w_data,
                                                 0.0, tpzmin, tpzmax, &mwmin);
                    assert(status);
                    mwmin_value = mwmin.mwsq > 0.0 ? sqrt(mwmin.mwsq) : 0.0;
                    if (ipars->debug_level >= 20)
                        printf("mc_integrate_w_mass: lowest MW is %g\n", mwmin_value);

                    fill_lepton_solution_info(&mwmin_info, &mwmin,
                                              ipars, mpars, &params, jes_points,
                                              n_jes_points, blep_orig,
                                              prob_to_loose_parton);
                    if (fixed_mwlep - mwmin_value > MIN_MW_BOUNDARY_DISTANCE)
                    {
                        for (isol=0; isol<nlepsols; ++isol)
                            mc_add_mwsq_point(ipars, sol_info+isol, 1.0,
                                              event_weight, n_jes_points);
                    }
                    else
                    {
                        /* Integrate the lowest MW^2 point.
                         * Find which points we should use.
                         */
                        int isol_min = 0, isol_max = nlepsols;

                        if (nlepsols > 2)
                        {
                            if (mwmin_info.lsol.nuz <= w_data[0].info[0].lsol.nuz)
                                isol_max = 2;
                            else if (mwmin_info.lsol.nuz >= 
                                     w_data[0].info[nlepsols-1].lsol.nuz)
                                isol_min = nlepsols - 2;
                            else
                            {
                                int place_found = 0;
                                for (isol=1; isol<nlepsols; ++isol)
                                    if (mwmin_info.lsol.nuz <= 
                                        w_data[0].info[isol].lsol.nuz)
                                    {
                                        isol_min = isol-1;
                                        isol_max = isol+1;
                                        place_found = 1;
                                        break;
                                    }
                                assert(place_found);
                            }
                        }

                        for (isol=isol_min; isol<isol_max; ++isol)
                            mc_nuz_interval_integ(
                                ipars, mpars, &mwmin_info,
                                w_data[0].info+isol,
                                event_weight, n_jes_points);
                    }
                }
            }
        }
    }
    else
    {
        /* Go over the grid points and solve the kinematic equations */
        for (idir=0; idir<2; ++idir)
        {
            int i, istart, iend, istep;
            if (idir)
            {
                /* Second cycle */
                istart = npoints/2-1;
                iend = -1;
                istep = -1;
            }
            else
            {
                /* First cycle */
                istart = npoints/2;
                iend = npoints;
                istep = 1;
            }
            for (i=istart; i!=iend; i+=istep)
            {
                const double mw = wmass_grid.points[i];
                if (mw + blep->m + MIN_MT_MW_DIFFERENCE_LEP <= mtlep)
                {
                    const double mwsq = mw*mw;
                    lepton_side_solution lepsols[4];

                    const int nlepsols = mc_solve_leptonic_side(
                        mpars->mwsq_safety_margin, istep,
                        tlepPx, tlepPy, lPx, lPy, lPz,
                        blep->px, blep->py, blep->pz, mtlep,
                        blep->m, mwsq, ipars->debug_level,
                        max_iterations, tpzmin, tpzmax, lepsols);
                    if (nlepsols)
                    {
                        int isol;
                        if (nlepsols > 1)
                            qsort(lepsols, nlepsols, sizeof(lepton_side_solution),
                                  sort_lepton_side_solution_by_nuz_incr);
                        for (isol=0; isol<nlepsols; ++isol)
                            fill_lepton_solution_info(
                                w_data[i].info+isol, lepsols+isol,
                                ipars, mpars, &params, jes_points,
                                n_jes_points, blep_orig, prob_to_loose_parton);
                        w_data[i].nlepsols = nlepsols;

                        if (i > max_point)
                            max_point = i;
                        if (i < min_point)
                            min_point = i;

                        ++points_with_sols;
                    }
                    else
                    {
                        /* Stop if we are going down and we have
                         * already seen at least one solution
                         */
                        if (max_point >= 0 && istep < 0)
                            break;
                    }
                }
            }
        }
        max_point += 1;

        /* CHECK THIS!!! */
        /* Should have at least two MW^2 points with solutions */
        if (points_with_sols >= 2)
        {
            const double mwsq_peak = wmass_grid.mW*wmass_grid.mW;
            const double mwsq_hwhm = wmass_grid.mW*wmass_grid.widthW;
            const int range_continuous = max_point - min_point == points_with_sols;

            double first_bin_binwidth = 0.0, previous_mwsq_binwidth = 0.0;
            int lowest_mwsq_integrated = 0, allow_lowest_point;

            /* Find the minimum W mass */
            lepton_side_solution mwmin;

            const double mw_nosol = min_point > 0 ? 
                wmass_grid.points[min_point-1] : 0.0;
            int status = mc_w_mass_range(mpars->mwsq_safety_margin,
                                         tlepPx, tlepPy, lPx, lPy, lPz,
                                         blep->px, blep->py, blep->pz,
                                         mtlep, blep->m, w_mass_range_maxiter,
                                         ipars->debug_level,
                                         w_data+min_point, mw_nosol*mw_nosol,
                                         tpzmin, tpzmax, &mwmin);
            assert(status);
            if (ipars->debug_level >= 20)
                printf("mc_integrate_w_mass: lowest MW is %g\n", 
                       mwmin.mwsq > 0.0 ? sqrt(mwmin.mwsq) : 0.0);

            /* Check if we are in danger of having a non-square-integrable
             * contribution into the integral from the phase space Jacobian
             */
            if (mwmin.mwsq > min_mwsq_cutoff)
            {
                Lepton_solution_info mwmin_info;

                fill_lepton_solution_info(&mwmin_info, &mwmin,
                                          ipars, mpars, &params, jes_points,
                                          n_jes_points, blep_orig, prob_to_loose_parton);

                /* Throw away the lowest MW^2 point for which there are
                 * solutions and split the interwal between mwmin and
                 * the next MW^2 point in half in MW^2 cdf. This makes
                 * sure that we are not going to have  a problem with
                 * a large contribution from the MW^2 phase space term.
                 */
                if (range_continuous)
                    assert(w_data[min_point+1].nlepsols);
                else if (ipars->debug_level > 0)
                {
                    static int max_warnings_noncont = 1000;
                    if (max_warnings_noncont > 0)
                    {
                        printf("WARNING in mc_integrate_w_mass: "
                               "leptonic side solutions are discontinuous in MW\n");
                        printf(" tlepPx %.17g\n", tlepPx);
                        printf(" tlepPy %.17g\n", tlepPy);
                        printf(" lPx %.17g\n", lPx);
                        printf(" lPy %.17g\n", lPy);
                        printf(" lPz %.17g\n", lPz);
                        printf(" bPx %.17g\n", blep->px);
                        printf(" bPy %.17g\n", blep->py);
                        printf(" bPz %.17g\n", blep->pz);
                        printf(" mt %.17g\n", mtlep);
                        printf(" mb %.17g\n", blep->m);
                        printf(" Estimated min MW is %.17g\n", sqrt(mwmin.mwsq));
                        if (--max_warnings_noncont == 0)
                            printf("Further occurrences of this message "
                                   "will be suppressed\n");
                        fflush(stdout);
                    }
                }

                if (w_data[min_point+1].nlepsols)
                {
                    lepton_side_solution lepsols[4];

                    /* Create a new mwsq point between the lowest possible mwsq
                     * value and the second lowest mwsq point for which a kinematic
                     * solution can be found
                     */
                    const double first_bin_mwsq = mc_split_mwsq_cdf(
                        mwsq_peak, mwsq_hwhm,
                        mwmin.mwsq, w_data[min_point+1].info[0].lsol.mwsq,
                        &first_bin_binwidth);

                    /* Fill out "lepsols" for the new mwsq */
                    const int nlepsols = mc_solve_leptonic_side(
                        mpars->mwsq_safety_margin, 1,
                        tlepPx, tlepPy, lPx, lPy, lPz,
                        blep->px, blep->py, blep->pz, mtlep,
                        blep->m, first_bin_mwsq, ipars->debug_level,
                        max_iterations, tpzmin, tpzmax, lepsols);

                    if (nlepsols)
                    {
                        int isol, isol_min = 0, isol_max = nlepsols;

                        if (nlepsols > 1)
                            qsort(lepsols, nlepsols, sizeof(lepton_side_solution),
                                  sort_lepton_side_solution_by_nuz_incr);
                        for (isol=0; isol<nlepsols; ++isol)
                            fill_lepton_solution_info(
                                w_data[min_point].info+isol, lepsols+isol,
                                ipars, mpars, &params, jes_points,
                                n_jes_points, blep_orig, prob_to_loose_parton);
                        w_data[min_point].nlepsols = nlepsols;

                        /* Integrate the lowest MW^2 point.
                         * Find which points we should use.
                         */
                        if (nlepsols > 2)
                        {
                            if (mwmin_info.lsol.nuz <= w_data[min_point].info[0].lsol.nuz)
                                isol_max = 2;
                            else if (mwmin_info.lsol.nuz >= 
                                     w_data[min_point].info[nlepsols-1].lsol.nuz)
                                isol_min = nlepsols - 2;
                            else
                            {
                                int place_found = 0;
                                for (isol=1; isol<nlepsols; ++isol)
                                    if (mwmin_info.lsol.nuz <= 
                                        w_data[min_point].info[isol].lsol.nuz)
                                    {
                                        isol_min = isol-1;
                                        isol_max = isol+1;
                                        place_found = 1;
                                        break;
                                    }
                                assert(place_found);
                            }
                        }

                        for (isol=isol_min; isol<isol_max; ++isol)
                            mc_nuz_interval_integ(
                                ipars, mpars, &mwmin_info,
                                w_data[min_point].info+isol,
                                event_weight, n_jes_points);
                        lowest_mwsq_integrated = 1;
                    }
                }
            }
        
            /* Go over all points and accumulate the integral.
             * 
             * If we have successfully integrated the lowest MW^2 point
             * then we should start integrating from the point with
             * the "min_point" index because this point has been
             * replaced by the cumulative probability density splitting
             * procedure. Otherwise we should skip "min_point"
             * because we are still in danger of having a very large
             * phase space term.
             */
            allow_lowest_point = lowest_mwsq_integrated ||
                mwmin.mwsq <= min_mwsq_cutoff;
            for (ipt=min_point; ipt<max_point; ++ipt)
            {
                double cdf_binwidth, cdf_weight;

                if (ipt == min_point)
                {
                    if (lowest_mwsq_integrated)
                        cdf_binwidth = first_bin_binwidth;
                    else
                        cdf_binwidth = wmass_grid.coverage/wmass_grid.nbins;
                }
                else if (ipt == max_point - 1)
                    cdf_binwidth = 0.0;
                else
                    cdf_binwidth = wmass_grid.coverage/wmass_grid.nbins;
                cdf_weight = (previous_mwsq_binwidth + cdf_binwidth)/2.0;

                if (ipt > min_point || (ipt == min_point && allow_lowest_point))
                {
                    /* WARNING!!! The following code uses at most
                     * two solutions out of all possible ones!
                     * Otherwise we can get into the infinite
                     * Jacobian problem again!
                     */ 
                    if (w_data[ipt].nlepsols > 0)
                        mc_add_mwsq_point(ipars, w_data[ipt].info+0, cdf_weight,
                                          event_weight, n_jes_points);
                    if (w_data[ipt].nlepsols > 1)
                    {
                        const int isol = w_data[ipt].nlepsols - 1;
                        mc_add_mwsq_point(ipars, w_data[ipt].info+isol, cdf_weight,
                                          event_weight, n_jes_points);
                    }
                }

                previous_mwsq_binwidth = cdf_binwidth;
            }
        }
    }
}

/* The function below must return a jet with momentum set to 1.
 * This is later used by various "solve_top" routines to recognize
 * that this jet is an "extra" one.
 */
static void random_dir_jet(jet_info *newjet, const double m,
                           const int isB, const float r1, const float r2)
{
    const double costheta = r1*2.0 - 1.0;
    const double sinthetasq = 1.0 - costheta*costheta;
    const double sintheta = sinthetasq > 1.0e-20 ? sqrt(sinthetasq) : 1.0e-10;
    const double phi      = 2.0*M_PI*r2;
    const double bProb    = isB ? 1.0 : 0.0;
    const double eta      = asinh(costheta/sintheta);

    fill_jet_info(newjet, sintheta*cos(phi), sintheta*sin(phi),
                  costheta, m, bProb, 0.0, eta, 0.0, 0.0, 0.0, 0, -1, 1);
}

#ifdef __cplusplus
}
#endif
