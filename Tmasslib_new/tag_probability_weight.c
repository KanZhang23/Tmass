#include <assert.h>
#include "tag_probability_weight.h"
#include "permutations.h"

#ifdef __cplusplus
extern "C" {
#endif

double tag_probability_weight(const jet_info* jets[4],
                              const int leptonCharge)
{
    unsigned j;
    double tag_probs[4], weight = 1.0;

    /* Note that the following might not be correct for Pythia.
     * In Herwig we will have q set to ubar or cbar, qbar set to d or s.
     * That is, q and qbar meaning is swapped due to specific ordering
     * of the decay daughters. See also "mahlon_parke_weight" function.
     */
    tag_probs[Q] = 0.5*(svx_tag_prob(jets[Q], 'q') + svx_tag_prob(jets[Q], 'c'));
    tag_probs[QBAR] = svx_tag_prob(jets[QBAR], 'q');
    tag_probs[BLEP] = svx_tag_prob(jets[BLEP], 'b');
    tag_probs[BHAD] = svx_tag_prob(jets[BHAD], 'b');

    for (j=0; j<4; ++j)
        if (!jets[j]->is_extra)
        {
            if (jet_has_btag(jets[j]))
                weight *= tag_probs[j];
            else
                weight *= (1.0 - tag_probs[j]);
        }

    return weight;
}

#ifdef __cplusplus
}
#endif
