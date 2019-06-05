#include <math.h>
#include <assert.h>

#include "linear_interpolator_1d.h"

#ifdef __cplusplus
extern "C" {
#endif

float linear_interpolate_1d(const Interpolator_data_1d *d, const float x)
{
    const float bwx = (d->xmax - d->xmin)/(d->nxpoints - 1);
    const int ix = x < d->xmin ? -1 : (int)floorf((x - d->xmin)/bwx);

    assert(d->nxpoints > 1);

    /* Process all possible conditions */
    if (ix < 0)
    {
	return d->data[0];
    }
    else if ((unsigned)ix < d->nxpoints - 1)
    {
	const float z0 = d->data[ix];
	return z0 + (d->data[ix+1] - z0)*((x - (d->xmin + ix * bwx))/bwx);
    }
    else
    {
	return d->data[d->nxpoints - 1];
    }
}

#ifdef __cplusplus
}
#endif
