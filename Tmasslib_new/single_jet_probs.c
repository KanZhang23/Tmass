#include <assert.h>
#include <string.h>

#include "single_jet_probs.h"

#ifdef __cplusplus
extern "C" {
#endif

double unit_prob_to_acquire_jet(const jet_info *jets, unsigned njets,
                                unsigned which, double jes)
{
    return 1.0;
}

double invalid_prob_to_acquire_jet(const jet_info *jets, unsigned njets,
                                   unsigned which, double jes)
{
    assert(0);
}

double unit_prob_to_loose_parton(const particle_obj *particle,
                                 double jes_sigma, int isB)
{
    return 1.0;
}

double invalid_prob_to_loose_parton(const particle_obj *particle,
                                    double jes_sigma, int isB)
{
    assert(0);
}

Prob_to_acquire_jet* choose_prob_to_acquire_jet(const char *name)
{
    if (strcasecmp(name, "unit") == 0)
        return unit_prob_to_acquire_jet;
    else if (strcasecmp(name, "none") == 0)
        return 0;
    else
        return invalid_prob_to_acquire_jet;
}

Prob_to_loose_parton* choose_prob_to_loose_parton(const char *name)
{
    if (strcasecmp(name, "unit") == 0)
        return unit_prob_to_loose_parton;
    else if (strcasecmp(name, "tf_eff") == 0)
        return mc_eff_prob_to_loose_parton;
    else if (strcasecmp(name, "drop") == 0)
        return drop_prob_to_loose_parton;
    else if (strcasecmp(name, "single_parton_eff") == 0)
        return single_parton_eff_prob_to_loose_parton;
    else if (strcasecmp(name, "total") == 0)
        return total_prob_to_loose_parton;
    else if (strcasecmp(name, "none") == 0)
        return 0;
    else
        return invalid_prob_to_loose_parton;
}

#ifdef __cplusplus
}
#endif
