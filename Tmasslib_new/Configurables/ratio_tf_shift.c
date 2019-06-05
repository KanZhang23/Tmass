#include <math.h>
#include <limits.h>
#include <assert.h>

#include "ratio_tf_shift.h"
#include "ordered_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

static const Ordered_tree_node* ratio_tf_slope_q = NULL;
static const Ordered_tree_node* ratio_tf_slope_b = NULL;

void set_ratio_shift_tree(const Ordered_tree_node *tree, int isB)
{
    if (isB)
        ratio_tf_slope_b = tree;
    else
        ratio_tf_slope_q = tree;
}

static double unpack_shift(const OTPayload *pay,
                           const double parton_eta,
                           const double jet_eta)
{
    typedef struct {
        float slope;
        unsigned short min;
        unsigned short max;
    } Packed;
    const double bw = 2.0/65536;

    const Packed *pack = (const Packed *)pay;
    const double deta_min = pack->min*bw - 1.0;
    const double deta_max = pack->max*bw - 1.0;
    double deta = parton_eta < 0.0 ? parton_eta - jet_eta :
                                     jet_eta - parton_eta;
    assert(sizeof(OTPayload) == sizeof(Packed));
    if (deta < deta_min)
        deta = deta_min;
    if (deta > deta_max)
        deta = deta_max;
    return pack->slope * deta;
}

double ratio_tf_shift(const double predictor, const double m94,
                      const double parton_eta, const double jet_eta,
                      const int isB)
{
    float index[3];
    OTPayload *pslope;

    index[0] = predictor;
    index[1] = m94;
    index[2] = fabs(parton_eta);

    if (isB)
    {
        assert(ratio_tf_slope_b);
        pslope = ot_find_payload(ratio_tf_slope_b, index, INT_MAX);
    }
    else
    {
        assert(ratio_tf_slope_q);
        pslope = ot_find_payload(ratio_tf_slope_q, index, INT_MAX);
    }
    return unpack_shift(pslope, parton_eta, jet_eta);
}

#ifdef __cplusplus
}
#endif
