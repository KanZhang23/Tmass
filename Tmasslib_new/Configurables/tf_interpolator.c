#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "tf_interpolator.h"

#ifdef __cplusplus
extern "C" {
#endif

inline static float lin_interpolate_1d(const float x0, const float x1,
                                       const float z0, const float z1,
                                       const float x)
{
    return z0 + (z1 - z0)*((x - x0)/(x1 - x0));
}

inline static float q_interpolate_1d(const float x1, const float bwx,
                                     const float z0, const float z1,
                                     const float z2, const float x)
{
    const float d1 = x - x1;
    const float d0 = d1 + bwx;
    const float d2 = d1 - bwx;
    return (d2*(d1*z0 - d0*z1) + d0*(d1*z2 - d2*z1))/2.f/bwx/bwx;
}

const Tf_interpolator* create_tf_interpolator(
    double (*convert)(double), float *data,
    unsigned nxpoints, unsigned nypoints, unsigned n_fcn_points,
    float xmin, float xmax, float ymin, float ymax,
    float fcn_arg_min, float fcn_arg_max, float fcn_low, float fcn_high)
{
    Tf_interpolator *tf = malloc(sizeof(Tf_interpolator));
    assert(tf);

    assert(data);
    assert(nxpoints > 1);
    assert(nypoints > 1);
    assert(n_fcn_points > 3);
    assert(xmax > xmin);
    assert(ymax > ymin);
    assert(fcn_arg_max > fcn_arg_min);

    tf->convert = convert;
    tf->data = data;
    tf->nxpoints = nxpoints;
    tf->nypoints = nypoints;
    tf->n_fcn_points = n_fcn_points;
    tf->xmin = xmin;
    tf->xmax = xmax;
    tf->ymin = ymin;
    tf->ymax = ymax;
    tf->fcn_arg_min = fcn_arg_min;
    tf->fcn_arg_max = fcn_arg_max;
    tf->fcn_low = fcn_low;
    tf->fcn_high = fcn_high;

    tf->bwx = (xmax - xmin)/(nxpoints - 1);
    tf->bwy = (ymax - ymin)/(nypoints - 1);
    tf->bwarg = (fcn_arg_max - fcn_arg_min)/(n_fcn_points - 1);

    return tf;
}

void destroy_tf_interpolator(const Tf_interpolator *tf_in)
{
    Tf_interpolator *tf = (Tf_interpolator *)tf_in;
    if (tf)
    {
        if (tf->data)
            free(tf->data);
        free(tf);
    }
}

float tf_interpolate(const Tf_interpolator *tf, const float x,
                     const float y, const float arg)
{
    assert(tf);
    if (arg < tf->fcn_arg_min)
        return tf->fcn_low;
    else if (arg > tf->fcn_arg_max)
        return tf->fcn_high;
    else
    {
        /* We will normally need 4 points for this interpolation */
        int ipt, point_nums[4];
        float arg_points[4];

        const int ix = x < tf->xmin ? -1 : (int)floorf((x - tf->xmin)/tf->bwx);
        const int iy = y < tf->ymin ? -1 : (int)floorf((y - tf->ymin)/tf->bwy);
        const int iarg = (arg - tf->fcn_arg_min)/tf->bwarg;
        const int narg = tf->n_fcn_points;
        const float xlow = tf->xmin + ix * tf->bwx;
        const float xhi  = xlow + tf->bwx;
        const float ylow = tf->ymin + iy * tf->bwy;
        const float yhi  = ylow + tf->bwy;

        if (iarg == 0)
        {
            point_nums[0] = 0;
            point_nums[1] = 1;
            point_nums[2] = 2;
            point_nums[3] = 3;
        }
        else if (iarg >= (int)(tf->n_fcn_points - 2))
        {
            point_nums[0] = tf->n_fcn_points - 4;
            point_nums[1] = tf->n_fcn_points - 3;
            point_nums[2] = tf->n_fcn_points - 2;
            point_nums[3] = tf->n_fcn_points - 1;
        }
        else
        {
            point_nums[0] = iarg-1;
            point_nums[1] = iarg;
            point_nums[2] = iarg+1;
            point_nums[3] = iarg+2;
        }

        /* Perform linear interpolation in x and y */
        for (ipt=0; ipt<4; ++ipt)
        {
            const int idx = point_nums[ipt];
            float fcn;

            if (ix < 0)
            {
                if (iy < 0)
                {
                    fcn = tf->data[idx];
                }
                else if ((unsigned)iy < tf->nypoints - 1)
                {
                    fcn = lin_interpolate_1d(ylow, yhi,
                                             tf->data[idx + narg*(iy)],
                                             tf->data[idx + narg*(iy + 1)],
                                             y);
                }
                else
                {
                    fcn = tf->data[idx + narg*(0*tf->nxpoints + tf->nypoints - 1)];
                }
            }
            else if ((unsigned)ix < tf->nxpoints - 1)
            {
                if (iy < 0)
                {
                    fcn = lin_interpolate_1d(xlow, xhi,
                                             tf->data[idx + narg*(ix*tf->nypoints + 0)],
                                             tf->data[idx + narg*((ix + 1)*tf->nypoints + 0)],
                                             x);
                }
                else if ((unsigned)iy < tf->nypoints - 1)
                {
                    /* This is the case where we really need 2d interpolation */
                    const float z00 = tf->data[idx + narg*(ix*tf->nypoints+iy)];
                    const float z10 = tf->data[idx + narg*((ix+1)*tf->nypoints+iy)];
                    const float z01 = tf->data[idx + narg*(ix*tf->nypoints+iy+1)];
                    const float z11 = tf->data[idx + narg*((ix+1)*tf->nypoints+iy+1)];
                    const float dx = (x - xlow)/tf->bwx;
                    const float dy = (y - ylow)/tf->bwy;

                    fcn = (dx - 1.0)*(dy - 1.0)*z00 + dx*z10 + dy*z01 + 
                        dx*dy*(z11-z01-z10);
                }
                else
                {
                    fcn = lin_interpolate_1d(xlow, xhi,
                                             tf->data[idx + narg*(ix*tf->nypoints + tf->nypoints - 1)],
                                             tf->data[idx + narg*((ix + 1)*tf->nypoints + tf->nypoints - 1)],
                                             x);
                }
            }
            else
            {
                if (iy < 0)
                {
                    fcn = tf->data[idx + narg*((tf->nxpoints - 1)*tf->nypoints)];
                }
                else if ((unsigned)iy < tf->nypoints - 1)
                {
                    fcn = lin_interpolate_1d(ylow, yhi,
                                             tf->data[idx + narg*((tf->nxpoints - 1)*tf->nypoints + iy)],
                                             tf->data[idx + narg*((tf->nxpoints - 1)*tf->nypoints + iy + 1)],
                                             y);
                }
                else
                {
                    fcn = tf->data[idx + narg*(tf->nxpoints*tf->nypoints - 1)];
                }
            }

            arg_points[ipt] = fcn;
        }

        /* Perform weighted quadratic interpolation in arg */
        {
            const float x1_1 = tf->fcn_arg_min + point_nums[1]*tf->bwarg;
            const float x1_2 = tf->fcn_arg_min + point_nums[2]*tf->bwarg;
            const float i1 = q_interpolate_1d(x1_1, tf->bwarg, arg_points[0],
                                              arg_points[1], arg_points[2], arg);
            const float i2 = q_interpolate_1d(x1_2, tf->bwarg, arg_points[1],
                                              arg_points[2], arg_points[3], arg);
            float w1, w2, r;
            if (arg <= x1_1)
            {
                w1 = 1.f;
                w2 = 0.f;
            }
            else if (arg >= x1_2)
            {
                w1 = 0.f;
                w2 = 1.f;
            }
            else
            {
                w1 = (x1_2 - arg)/tf->bwarg;
                w2 = (arg - x1_1)/tf->bwarg;

                /* The following weights may be uncommented to use
                 * cubic interpolation instead of weighted quadratic.
                 * Cubic interpolation will work better if the function
                 * we are intrerpolating looks like a polynomial.
                 */
                /* w1 = (x1_2 + tf->bwarg - arg)/3.0/tf->bwarg; */
                /* w2 = (arg + tf->bwarg - x1_1)/3.0/tf->bwarg; */
            }
            r = w1*i1 + w2*i2;
            if (tf->convert)
                return (tf->convert)(r);
            else
                return r;
        }
    }
}

#ifdef __cplusplus
}
#endif
