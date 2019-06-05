#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include "linear_interpolator_nd.h"

/* Note that the number of points must be at least 2^MAXDIM,
 * so it does not make much sense to use more than 31 dimensions
 * or so. The current code implementation does not allow to
 * increase the number of dimensions beyond (the number of bits
 * in an unsigned integer) - 1.
 */
#define MAXDIM 31

#ifdef __cplusplus
extern "C" {
#endif

Interpolator_data_nd* create_interpolator_nd(const Interpolator_axis* axes,
                                             unsigned n_axes, const float *data,
                                             unsigned assume_data_ownership)
{
    Interpolator_data_nd *d;
    unsigned i;
    int j;

    assert(axes);
    assert(n_axes);
    assert(n_axes <= MAXDIM);
    assert(data);

    d = (Interpolator_data_nd *)malloc(sizeof(Interpolator_data_nd));
    assert(d);

    d->axes = (Interpolator_axis *)malloc(n_axes*sizeof(Interpolator_axis));
    assert(d->axes);
    d->dim = n_axes;

    for (i=0; i<n_axes; ++i)
    {
        assert(axes[i].xmin < axes[i].xmax);
        assert(axes[i].npoints > 1);
        d->axes[i] = axes[i];
    }

    d->strides = (unsigned *)malloc(n_axes*sizeof(unsigned));
    assert(d->strides);
    d->strides[n_axes-1] = 1;
    for (j=(int)n_axes-2; j>=0; --j)
        d->strides[j] = d->strides[j+1]*axes[j+1].npoints;

    d->data = data;
    d->owns_data = assume_data_ownership;

    return d;
}

void destroy_interpolator_nd(Interpolator_data_nd *d)
{
    if (d)
    {
        free(d->strides);
        free(d->axes);
        if (d->owns_data)
            free((float *)d->data);
        free(d);
    }
}

float linear_interpolate_nd(const Interpolator_data_nd *d,
                            const float *coords)
{
    const unsigned maxcycle = 1U << d->dim;
    int ix[MAXDIM];
    float dx[MAXDIM];
    unsigned i, icycle;
    double sum = 0.0;

    for (i=0; i<d->dim; ++i)
    {
        const float x = coords[i];
        const float xmin = d->axes[i].xmin;
        const float xmax = d->axes[i].xmax;
        const unsigned npoints = d->axes[i].npoints;

        if (x <= xmin)
        {
            ix[i] = 0;
            dx[i] = 0.f;
        }
        else
        {
            const float bwx = (xmax - xmin)/(npoints - 1);
            ix[i] = (int)floorf((x - xmin)/bwx);
            if ((unsigned)ix[i] >= npoints - 1)
            {
                ix[i] = npoints - 2;
                dx[i] = 1.f;
            }
            else
                dx[i] = (x - (xmin + ix[i] * bwx))/bwx;
        }
    }

    for (icycle=0; icycle<maxcycle; ++icycle)
    {
        double w = 1.0;
        unsigned icell = 0;
        for (i=0; i<d->dim; ++i)
        {
            if (icycle & (1U << i))
            {
                w *= dx[i];
                icell += d->strides[i]*(ix[i] + 1);
            }
            else
            {
                w *= (1.0 - dx[i]);
                icell += d->strides[i]*ix[i];
            }
        }
        sum += w*d->data[icell];
    }

    return sum;
}

#ifdef __cplusplus
}
#endif
