#include <assert.h>
#include <math.h>
#include "random_lepton_momentum.h"
#include "topmass_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fudge factor to take into account the tails */
#define E_RESOLUTION_FUDGE 1.07

double mu_pt_error(double pt)
{
    if (pt < 10.0)
        pt = 10.0;
    return 0.275912672281 - 0.0050118858926*pt + 0.000509732344653*pt*pt;
}

double e_pt_bias(double pt)
{
    return -0.480267286301 + 0.0161927025765*pt;
}

double mu_pt_bias(double pt)
{
    if (pt < 25.0)
        pt = 25.0;
    if (pt > 135.0)
        pt = 135.0;
    return (((-1.45924614614e-08*pt + 4.13780617237e-06)*pt - 
             0.000340946135111)*pt + 0.00968152843416)*pt - 0.108470104635;
}

double e_pt_error(double pt)
{
    return E_RESOLUTION_FUDGE*(0.441608309746 + 0.0198304578662*pt);
}

double random_mu_dpt(double pt)
{
    return gauss_random(mu_pt_bias(pt), mu_pt_error(pt));
}

double random_e_dpt(double pt)
{
    return gauss_random(e_pt_bias(pt), e_pt_error(pt));
}

v3_obj randomize_lepton_momentum(v3_obj orig, int isMu)
{
    typedef double (*randomizer)(double);
    static const randomizer random_dpt[2] = {random_e_dpt, random_mu_dpt};

    const int index = isMu ? 1 : 0;
    const double pt = Pt(orig);
    double newpt;

    assert(pt > 0.0);
    do {
	newpt = pt + random_dpt[index](pt);
    } while (newpt <= 0.0);

    return mult3(orig, newpt/pt);
}

#ifdef __cplusplus
}
#endif
