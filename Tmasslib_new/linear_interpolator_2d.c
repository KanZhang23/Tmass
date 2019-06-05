#include <math.h>
#include <assert.h>

#include "linear_interpolator_2d.h"

#ifdef __cplusplus
extern "C" {
#endif

static float lin_interpolate_1d(float x0, float x1,
				float z0, float z1, float x)
{
    return z0 + (z1 - z0)*((x - x0)/(x1 - x0));
}

float linear_interpolate_2d(const Interpolator_data_2d *d,
			    const float x,
			    const float y)
{
    const float bwx = (d->xmax - d->xmin)/(d->nxpoints - 1);
    const float bwy = (d->ymax - d->ymin)/(d->nypoints - 1);
    const int ix = x < d->xmin ? -1 : (int)floorf((x - d->xmin)/bwx);
    const int iy = y < d->ymin ? -1 : (int)floorf((y - d->ymin)/bwy);
    const float xlow = d->xmin + ix * bwx;
    const float xhi  = xlow + bwx;
    const float ylow = d->ymin + iy * bwy;
    const float yhi  = ylow + bwy;

    assert(d->nxpoints > 1);
    assert(d->nypoints > 1);

    /* Process all possible conditions */
    if (ix < 0)
    {
	if (iy < 0)
	{
	    return d->data[0];
	}
	else if ((unsigned)iy < d->nypoints - 1)
	{
	    return lin_interpolate_1d(ylow, yhi,
				      d->data[iy],
				      d->data[iy + 1],
				      y);
	}
	else
	{
	    return d->data[0*d->nxpoints + d->nypoints - 1];
	}
    }
    else if ((unsigned)ix < d->nxpoints - 1)
    {
	if (iy < 0)
	{
	    return lin_interpolate_1d(xlow, xhi,
				      d->data[ix*d->nypoints + 0],
				      d->data[(ix + 1)*d->nypoints + 0],
				      x);
	}
	else if ((unsigned)iy < d->nypoints - 1)
	{
	    /* This is the case where we really need 2d interpolation */
	    const float z00 = d->data[ix*d->nypoints+iy];
	    const float z10 = d->data[(ix+1)*d->nypoints+iy];
	    const float z01 = d->data[ix*d->nypoints+iy+1];
	    const float z11 = d->data[(ix+1)*d->nypoints+iy+1];
	    const float dx = (x - xlow)/bwx;
	    const float dy = (y - ylow)/bwy;

	    return (dx - 1.0)*(dy - 1.0)*z00 + dx*z10 + dy*z01 + 
		    dx*dy*(z11-z01-z10);
	}
	else
	{
	    return lin_interpolate_1d(xlow, xhi,
				      d->data[ix*d->nypoints + d->nypoints - 1],
				      d->data[(ix + 1)*d->nypoints + d->nypoints - 1],
				      x);
	}
    }
    else
    {
	if (iy < 0)
	{
	    return d->data[(d->nxpoints - 1)*d->nypoints];
	}
	else if ((unsigned)iy < d->nypoints - 1)
	{
	    return lin_interpolate_1d(ylow, yhi,
				      d->data[(d->nxpoints - 1)*d->nypoints + iy],
				      d->data[(d->nxpoints - 1)*d->nypoints + iy + 1],
				      y);
	}
	else
	{
	    return d->data[d->nxpoints*d->nypoints - 1];
	}
    }
}

#ifdef __cplusplus
}
#endif
