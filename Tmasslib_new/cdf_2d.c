#include <assert.h>
#include <math.h>

#include "cdf_2d.h"

#ifdef __cplusplus
extern "C" {
#endif

double cdf_2d_value(const Cdf_2d_data *cdf,
                    const double x, const double y)
{
    if (x <= cdf->xmin || y <= cdf->ymin)
        return 0.0;
    else if (x >= cdf->xmax && y >= cdf->ymax)
        return 1.0;
    else if (y >= cdf->ymax)
    {
        // Return cdf at x and ymax
        const int nybinsp1 = cdf->nybins + 1;
        const int ix = (int)((x - cdf->xmin)/cdf->bwx);
        const double dx = (x - cdf->xmin)/cdf->bwx - ix;
        const double z00 = cdf->data[ix*nybinsp1+cdf->nybins];
        const double z10 = cdf->data[(ix+1)*nybinsp1+cdf->nybins];
        return z00*(1.0 - dx) + z10*dx;
    }
    else if (x >= cdf->xmax)
    {
        // Return cdf at xmax and y
        const int nybinsp1 = cdf->nybins + 1;
        const int iy = (int)((y - cdf->ymin)/cdf->bwy);
        const double dy = (y - cdf->ymin)/cdf->bwy - iy;
        const double z00 = cdf->data[cdf->nxbins*nybinsp1+iy];
        const double z01 = cdf->data[cdf->nxbins*nybinsp1+iy+1];
        return z00*(1.0 - dy) + z01*dy;
    }
    else
    {
        // We are not on the boundary
        const int nybinsp1 = cdf->nybins + 1;
        const int ix = (int)((x - cdf->xmin)/cdf->bwx);
        const int iy = (int)((y - cdf->ymin)/cdf->bwy);
        const double dx = (x - cdf->xmin)/cdf->bwx - ix;
        const double dy = (y - cdf->ymin)/cdf->bwy - iy;
        const double z00 = cdf->data[ix*nybinsp1+iy];
        const double z01 = cdf->data[ix*nybinsp1+iy+1];
        const double z10 = cdf->data[(ix+1)*nybinsp1+iy];
        const double z11 = cdf->data[(ix+1)*nybinsp1+iy+1];
        return (dx - 1.0)*(dy - 1.0)*z00 + dx*z10 + dy*z01 + 
                dx*dy*(z11-z01-z10);
    }
}

double cdf_2d_rectangle_coverage(const Cdf_2d_data *cdf,
                                 const double xmin, const double ymin,
                                 const double xmax, const double ymax)
{
    const double ur = cdf_2d_value(cdf, xmax, ymax);
    const double lr = cdf_2d_value(cdf, xmax, ymin);
    const double ul = cdf_2d_value(cdf, xmin, ymax);
    const double ll = cdf_2d_value(cdf, xmin, ymin);
    const double d = ur - lr - (ul - ll);
    if (d > 0.0)
        return d;
    else
        return 0.0;
}

double cdf_2d_invcdf_x(const Cdf_2d_data *cdf,
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
            zval = cdf_2d_value(cdf, xmed, cdf->ymax);
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

double cdf_2d_invcdf_y(const Cdf_2d_data *cdf,
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
            zval = cdf_2d_value(cdf, cdf->xmax, ymed);
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

static void build_window(const Cdf_2d_data *cdf, 
                         const double x_cdf, const double y_cdf,
                         const double width, const double eps,
                         double *xmin, double *ymin,
                         double *xmax, double *ymax)
{
    double x_cdf_lo, x_cdf_hi, y_cdf_lo, y_cdf_hi;

    covering_cdf_interval(x_cdf, width, &x_cdf_lo, &x_cdf_hi);
    covering_cdf_interval(y_cdf, width, &y_cdf_lo, &y_cdf_hi);
    *xmin = cdf_2d_invcdf_x(cdf, x_cdf_lo, eps);
    *xmax = cdf_2d_invcdf_x(cdf, x_cdf_hi, eps);
    *ymin = cdf_2d_invcdf_y(cdf, y_cdf_lo, eps);
    *ymax = cdf_2d_invcdf_y(cdf, y_cdf_hi, eps);
}

void cdf_2d_optimal_window(const Cdf_2d_data *cdf,
                           const double x_center, const double y_center,
                           const double coverage, const double eps,
                           double *xmin, double *ymin,
                           double *xmax, double *ymax)
{
    assert(coverage >= 0.0);
    assert(eps > 0.0);

    if (coverage == 0.0)
    {
        *xmin = *xmax = x_center;
        *ymin = *ymax = y_center;
    }
    else if (coverage >= 1.0)
    {
        *xmin = cdf->xmin;
        *xmax = cdf->xmax;
        *ymin = cdf->ymin;
        *ymax = cdf->ymax;
    }
    else
    {
        const double x_cdf = cdf_2d_value(cdf, x_center, cdf->ymax);
        const double y_cdf = cdf_2d_value(cdf, cdf->xmax, y_center);
        double try_width = sqrt(coverage);
        double width_min = 0.0;
        double cov_min = 0.0;
        double width_max = 1.0;
        double cov_max = 1.0;
        double try_coverage, new_width;
        do {
            build_window(cdf, x_cdf, y_cdf, try_width,
                         eps, xmin, ymin, xmax, ymax);
            try_coverage = cdf_2d_rectangle_coverage(
                cdf, *xmin, *ymin, *xmax, *ymax);
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
