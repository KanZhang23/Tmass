#include <assert.h>
#include <math.h>

#include "single_jet_probs.h"
#include "transfer_function.h"
#include "single_parton_efficiency.h"
// #include "angular_transfer_function.h"

#ifdef __cplusplus
extern "C" {
#endif

static double pt_cut_toloose = 10.0;
static double eta_cut_toloose = 2.0;
static double max_efficiency_toloose = 1.0;
static double jes_sigma_factor_toloose = -1.0;

void set_params_to_loose_parton(double pt_cut, double eta_cut,
                                double max_efficiency,
                                double jes_sigma_factor)
{
    assert(jes_sigma_factor > 0.0);

    pt_cut_toloose = pt_cut;
    eta_cut_toloose = eta_cut;
    max_efficiency_toloose = max_efficiency;
    jes_sigma_factor_toloose = jes_sigma_factor;
}

double drop_prob_to_loose_parton(const particle_obj *particle,
                                 double deltaJES, int isB)
{
    return 1.0 - mc_eff_prob_to_loose_parton(particle, deltaJES, isB);
}

double mc_eff_prob_to_loose_parton(const particle_obj *particle,
                                   double deltaJES, int isB)
{
    if (fabs(Eta(particle->p)) > eta_cut_toloose)
        return 1.0;
    else
    {
        assert(jes_sigma_factor_toloose > 0.0);

        const double jes = 1.0 + deltaJES*jes_sigma_factor_toloose;
        const double effective_cut = pt_cut_toloose*jes;
        /* The following makes an assumption that eta parton
         * is within the same bin as eta detector. This,
         * in principle, should be fixed.
         */
        const double eff = transfer_function_efficiency_2(*particle, Eta(particle->p),
                                                          isB, effective_cut);
        const double prob = 1.0 - max_efficiency_toloose*eff;
        assert(prob <= 1.0);
        return prob > 0.0 ? prob : 0.0;
    }
}

double single_parton_eff_prob_to_loose_parton(const particle_obj *particle,
                                              double deltaJES, int isB)
{
    if (fabs(Eta(particle->p)) > eta_cut_toloose)
        return 0.0;
    if (Pt(particle->p) < pt_cut_toloose)
        return 0.0;
    return single_parton_eff(*particle, isB, deltaJES, -1.0);
}

//double total_prob_to_loose_parton(const particle_obj *particle,
//                                  double deltaJES, int isB)
//{
//    const double eff = single_parton_eff(*particle, isB, deltaJES, -1.0)*
//                       angular_tf_efficiency(isB, particle->p, particle->m);
//    const double prob = 1.0 - max_efficiency_toloose*eff;
//    assert(prob <= 1.0);
//
//    /* It makes no sense here to return a probability
//     * less than machine epsilon or so (because we have
//     * just subtracted two large numbers).
//     */
//    return prob > 1.0e-15 ? prob : 1.0e-15;
//}

double total_prob_to_loose_parton(const particle_obj *particle,
                                  double deltaJES, int isB)
{
    assert(0);
    return 1.0e-15;
}

#ifdef __cplusplus
}
#endif
