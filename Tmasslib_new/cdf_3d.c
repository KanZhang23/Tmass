#include <assert.h>
#include <math.h>

#include "cdf_3d.h"

#ifdef __cplusplus
extern "C" {
#endif

double cdf_3d_value(const Cdf_3d_data *cdf,
                    const double x, const double y, const double z)
{
    if (x <= cdf->xmin || y <= cdf->ymin || z <= cdf->zmin)
        return 0.0;
    else if (x >= cdf->xmax && y >= cdf->ymax && z >= cdf->zmax)
        return 1.0;
    else
    {
        const int nybinsp1 = cdf->nybins + 1;
        const int nzbinsp1 = cdf->nzbins + 1;

        double dx, dy, dz, d;
        int ix, iy, iz;

        d = (x - cdf->xmin)/cdf->bwx;
        ix = d;
        dx = d - ix;
        d = (y - cdf->ymin)/cdf->bwy;
        iy = d;
        dy = d - iy;
        d = (z - cdf->zmin)/cdf->bwz;
        iz = d;
        dz = d - iz;

        if (ix >= cdf->nxbins)
        {
            ix = cdf->nxbins - 1;
            dx = 1.0;
        }
        if (iy >= cdf->nybins)
        {
            iy = cdf->nybins - 1;
            dy = 1.0;
        }
        if (iz >= cdf->nzbins)
        {
            iz = cdf->nzbins - 1;
            dz = 1.0;
        }

        {
            const double *data = cdf->data;
            const double v000 = data[(ix*nybinsp1 + iy)*nzbinsp1 + iz];
            const double v001 = data[(ix*nybinsp1 + iy)*nzbinsp1 + iz + 1];
            const double v010 = data[(ix*nybinsp1 + iy + 1)*nzbinsp1 + iz];
            const double v011 = data[(ix*nybinsp1 + iy + 1)*nzbinsp1 + iz + 1];
            const double v100 = data[((ix+1)*nybinsp1 + iy)*nzbinsp1 + iz];
            const double v101 = data[((ix+1)*nybinsp1 + iy)*nzbinsp1 + iz + 1];
            const double v110 = data[((ix+1)*nybinsp1 + iy + 1)*nzbinsp1 + iz];
            const double v111 = data[((ix+1)*nybinsp1 + iy + 1)*nzbinsp1 + iz + 1];

            return v000*(1.0-dx)*(1.0-dy)*(1.0-dz) +
                v001*(1.0-dx)*(1.0-dy)*dz + 
                v010*(1.0-dx)*dy*(1.0-dz) +
                v011*(1.0-dx)*dy*dz + 
                v100*dx*(1.0-dy)*(1.0-dz) +
                v101*dx*(1.0-dy)*dz + 
                v110*dx*dy*(1.0-dz) +
                v111*dx*dy*dz;
        }
    }
}

double cdf_3d_rectangle_coverage(const Cdf_3d_data *cdf,
                                 const double xmin,
                                 const double ymin,
                                 const double zmin,
                                 const double xmax,
                                 const double ymax,
                                 const double zmax)
{
    const double urlz = cdf_3d_value(cdf, xmax, ymax, zmin);
    const double lrlz = cdf_3d_value(cdf, xmax, ymin, zmin);
    const double ullz = cdf_3d_value(cdf, xmin, ymax, zmin);
    const double lllz = cdf_3d_value(cdf, xmin, ymin, zmin);
    const double dlz  = urlz - lrlz - (ullz - lllz);
    const double urhz = cdf_3d_value(cdf, xmax, ymax, zmax);
    const double lrhz = cdf_3d_value(cdf, xmax, ymin, zmax);
    const double ulhz = cdf_3d_value(cdf, xmin, ymax, zmax);
    const double llhz = cdf_3d_value(cdf, xmin, ymin, zmax);
    const double dhz  = urhz - lrhz - (ulhz - llhz);
    const double d = dhz - dlz;
    if (d > 0.0)
        return d;
    else
        return 0.0;
}

double cdf_3d_invcdf_x(const Cdf_3d_data *cdf,
                       const double cdfvalue,
                       const double eps)
{
    assert(eps > 0.0);

    if (cdfvalue <= 0.0)
        return cdf->xmin;
    else if (cdfvalue >= 1.0)
        return cdf->xmax;
    else
    {
        double xmin = cdf->xmin;
        double xmax = cdf->xmax;
        double zmin = 0.0;
        double zmax = 1.0;
        double zval, xmed;
        do {
            xmed = (xmin + xmax)/2.0;
            zval = cdf_3d_value(cdf, xmed, cdf->ymax, cdf->zmax);
            if (zval > cdfvalue)
            {
                xmax = xmed;
                zmax = zval;
            }
            else if (zval < cdfvalue)
            {
                xmin = xmed;
                zmin = zval;
            }
            else
                break;
        } while (zmax - zmin > eps);
        return xmed;
    }
}

double cdf_3d_invcdf_y(const Cdf_3d_data *cdf,
                       const double cdfvalue,
                       const double eps)
{
    assert(eps > 0.0);

    if (cdfvalue <= 0.0)
        return cdf->ymin;
    else if (cdfvalue >= 1.0)
        return cdf->ymax;
    else
    {
        double ymin = cdf->ymin;
        double ymax = cdf->ymax;
        double zmin = 0.0;
        double zmax = 1.0;
        double zval, ymed;
        do {
            ymed = (ymin + ymax)/2.0;
            zval = cdf_3d_value(cdf, cdf->xmax, ymed, cdf->zmax);
            if (zval > cdfvalue)
            {
                ymax = ymed;
                zmax = zval;
            }
            else if (zval < cdfvalue)
            {
                ymin = ymed;
                zmin = zval;
            }
            else
                break;
        } while (zmax - zmin > eps);
        return ymed;
    }
}

double cdf_3d_invcdf_z(const Cdf_3d_data *cdf,
                       const double cdfvalue,
                       const double eps)
{
    assert(eps > 0.0);

    if (cdfvalue <= 0.0)
        return cdf->zmin;
    else if (cdfvalue >= 1.0)
        return cdf->zmax;
    else
    {
        double zmin = cdf->zmin;
        double zmax = cdf->zmax;
        double vmin = 0.0;
        double vmax = 1.0;
        double vval, zmed;
        do {
            zmed = (zmin + zmax)/2.0;
            vval = cdf_3d_value(cdf, cdf->xmax, cdf->ymax, zmed);
            if (vval > cdfvalue)
            {
                zmax = zmed;
                vmax = vval;
            }
            else if (vval < cdfvalue)
            {
                zmin = zmed;
                vmin = vval;
            }
            else
                break;
        } while (vmax - vmin > eps);
        return zmed;
    }
}

static void covering_cdf_interval(const double x, const double width,
                                  double *xmin, double *xmax)
{
    const double halfwidth = width/2.0;
    assert(width > 0.0 && width < 1.0);
    *xmin = x - halfwidth;
    if (*xmin < 0.0)
    {
        *xmin = 0.0;
        *xmax = width;
    }
    else
    {
        *xmax = x + halfwidth;
        if (*xmax > 1.0)
        {
            *xmax = 1.0;
            *xmin = *xmax - width;
        }
    }
}

static void build_window(const Cdf_3d_data *cdf, 
                         const double x_cdf, const double y_cdf,
                         const double z_cdf,
                         const double width, const double eps,
                         double *xmin, double *ymin, double *zmin,
                         double *xmax, double *ymax, double *zmax)
{
    double x_cdf_lo, x_cdf_hi, y_cdf_lo, y_cdf_hi, z_cdf_lo, z_cdf_hi;

    covering_cdf_interval(x_cdf, width, &x_cdf_lo, &x_cdf_hi);
    covering_cdf_interval(y_cdf, width, &y_cdf_lo, &y_cdf_hi);
    covering_cdf_interval(z_cdf, width, &z_cdf_lo, &z_cdf_hi);
    *xmin = cdf_3d_invcdf_x(cdf, x_cdf_lo, eps);
    *xmax = cdf_3d_invcdf_x(cdf, x_cdf_hi, eps);
    *ymin = cdf_3d_invcdf_y(cdf, y_cdf_lo, eps);
    *ymax = cdf_3d_invcdf_y(cdf, y_cdf_hi, eps);
    *zmin = cdf_3d_invcdf_z(cdf, z_cdf_lo, eps);
    *zmax = cdf_3d_invcdf_z(cdf, z_cdf_hi, eps);
}

void cdf_3d_optimal_window(const Cdf_3d_data *cdf,
                           const double x_center, const double y_center,
                           const double z_center,
                           const double coverage, const double eps,
                           double *xmin, double *ymin, double *zmin,
                           double *xmax, double *ymax, double *zmax)
{
    assert(coverage >= 0.0);
    assert(eps > 0.0);

    if (coverage == 0.0)
    {
        *xmin = *xmax = x_center;
        *ymin = *ymax = y_center;
        *zmin = *zmax = z_center;
    }
    else if (coverage >= 1.0)
    {
        *xmin = cdf->xmin;
        *xmax = cdf->xmax;
        *ymin = cdf->ymin;
        *ymax = cdf->ymax;
        *zmin = cdf->zmin;
        *zmax = cdf->zmax;
    }
    else
    {
        const double x_cdf = cdf_3d_value(cdf, x_center, cdf->ymax, cdf->zmax);
        const double y_cdf = cdf_3d_value(cdf, cdf->xmax, y_center, cdf->zmax);
        const double z_cdf = cdf_3d_value(cdf, cdf->xmax, cdf->ymax, z_center);
        double try_width = pow(coverage, 1.0/3.0);
        double width_min = 0.0;
        double cov_min = 0.0;
        double width_max = 1.0;
        double cov_max = 1.0;
        double try_coverage, new_width;
        do {
            build_window(cdf, x_cdf, y_cdf, z_cdf, try_width,
                         eps/10.0, xmin, ymin, zmin, xmax, ymax, zmax);
            try_coverage = cdf_3d_rectangle_coverage(
                cdf, *xmin, *ymin, *zmin, *xmax, *ymax, *zmax);
            if (try_coverage > coverage)
            {
                width_max = try_width;
                cov_max = try_coverage;
            }
            else if (try_coverage < coverage)
            {
                width_min = try_width;
                cov_min = try_coverage;
            }
            else
                break;
            new_width = (width_min + width_max)/2.0;
            if (new_width > 2.0*try_width)
                try_width *= 2.0;
            else
                try_width = new_width;
        } while (cov_max - cov_min > eps);
    }
}

#ifdef __cplusplus
}
#endif
