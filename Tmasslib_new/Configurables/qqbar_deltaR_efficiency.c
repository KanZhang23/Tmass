#include <assert.h>

#include "qqbar_deltaR_efficiency.h"

#ifdef __cplusplus
extern "C" {
#endif

static const Interpolator_data_nd* qqbar_deltaR_interp = 0;

void set_qqbar_deltaR_interpolator(const Interpolator_data_nd *in)
{
    qqbar_deltaR_interp = in;
}

double qqbar_deltaR_efficiency(double deltaR, double m1_ratio, double m2_ratio)
{
    float x[3];
    assert(qqbar_deltaR_interp);
    x[0] = deltaR;
    x[1] = m1_ratio;
    x[2] = m2_ratio;
    return linear_interpolate_nd(qqbar_deltaR_interp, x);
}

#ifdef __cplusplus
}
#endif
