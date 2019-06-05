#include <assert.h>
#include "poly_3_2d.h"

#define SQRT3  1.7320508075688772935
#define SQRT5  2.2360679774997896964
#define SQRT7  2.6457513110645905905
#define SQRT15 3.8729833462074168852

#ifdef __cplusplus
extern "C" {
#endif

static double standard_poly_value(const double x, const double y,
                                  const double coeffs[10])
{
    const double xsq = x*x;
    const double ysq = y*y;
    int i;
    double poly[10], sum = 0.0;

    poly[0] = 0.5;
    poly[1] = SQRT3/2.0*x;
    poly[2] = SQRT3/2.0*y;
    poly[3] = SQRT5/4.0*(3.0*xsq - 1.0);
    poly[4] = SQRT5/4.0*(3.0*ysq - 1.0);
    poly[5] = 1.5*x*y;
    poly[6] = SQRT7/4.0*x*(5.0*xsq - 3.0);
    poly[7] = SQRT7/4.0*y*(5.0*ysq - 3.0);
    poly[8] = SQRT15/4.0*y*(3.0*xsq - 1.0);
    poly[9] = SQRT15/4.0*x*(3.0*ysq - 1.0);
    for (i=0; i<10; ++i)
        sum += poly[i]*coeffs[i];
    return sum;
}

static double d_standard_poly_dx(const double x, const double y,
                                 const double coeffs[10])
{
    int i;
    double dpoly[10], sum = 0.0;

    dpoly[0] = 0.0;
    dpoly[1] = SQRT3/2.0;
    dpoly[2] = 0.0;
    dpoly[3] = 1.5*SQRT5*x;
    dpoly[4] = 0.0;
    dpoly[5] = 1.5*y;
    dpoly[6] = 3.0*SQRT7/4.0*(5.0*x*x - 1.0);
    dpoly[7] = 0.0;
    dpoly[8] = 1.5*SQRT15*x*y;
    dpoly[9] = SQRT15/4.0*(3.0*y*y - 1.0);
    for (i=0; i<10; ++i)
        sum += dpoly[i]*coeffs[i];
    return sum;
}

static double d_standard_poly_dy(const double x, const double y,
                                 const double coeffs[10])
{
    int i;
    double dpoly[10], sum = 0.0;

    dpoly[0] = 0.0;
    dpoly[1] = 0.0;
    dpoly[2] = SQRT3/2.0;
    dpoly[3] = 0.0;
    dpoly[4] = 1.5*SQRT5*y;
    dpoly[5] = 1.5*x;
    dpoly[6] = 0.0;
    dpoly[7] = 3.0*SQRT7/4.0*(5.0*y*y - 1.0);
    dpoly[8] = SQRT15/4.0*(3.0*x*x - 1.0);
    dpoly[9] = 1.5*SQRT15*x*y;
    for (i=0; i<10; ++i)
        sum += dpoly[i]*coeffs[i];
    return sum;
}

double poly_3_2d(const double x_in, const double y_in,
                 const double xmin, const double xmax,
                 const double ymin, const double ymax,
                 const double coeffs[10])
{
    const double x = 2.0*(x_in - xmin)/(xmax - xmin) - 1.0;
    const double y = 2.0*(y_in - ymin)/(ymax - ymin) - 1.0;
    const int ix = x < -1.0 ? 0 : x < 1.0 ? 1 : 2;
    const int iy = y < -1.0 ? 0 : y < 1.0 ? 1 : 2;

    switch (ix)
    {
    case 0:
        switch (iy)
        {
        case 0:
            return standard_poly_value(-1.0, -1.0, coeffs) + 
                d_standard_poly_dx(-1.0, -1.0, coeffs)*(x + 1.0) +
                d_standard_poly_dy(-1.0, -1.0, coeffs)*(y + 1.0);
        case 1:
            return standard_poly_value(-1.0, y, coeffs) +
                d_standard_poly_dx(-1.0, y, coeffs)*(x + 1.0);
        case 2:
            return standard_poly_value(-1.0, 1.0, coeffs) + 
                d_standard_poly_dx(-1.0, 1.0, coeffs)*(x + 1.0) +
                d_standard_poly_dy(-1.0, 1.0, coeffs)*(y - 1.0);
        default:
            assert(0);
        }
    case 1:
        switch (iy)
        {
        case 0:
            return standard_poly_value(x, -1.0, coeffs) +
                d_standard_poly_dy(x, -1.0, coeffs)*(y + 1.0);
        case 1:
            return standard_poly_value(x, y, coeffs);
        case 2:
            return standard_poly_value(x, 1.0, coeffs) +
                d_standard_poly_dy(x, 1.0, coeffs)*(y - 1.0);
        default:
            assert(0);
        }
    case 2:
        switch (iy)
        {
        case 0:
            return standard_poly_value(1.0, -1.0, coeffs) + 
                d_standard_poly_dx(1.0, -1.0, coeffs)*(x - 1.0) +
                d_standard_poly_dy(1.0, -1.0, coeffs)*(y + 1.0);
        case 1:
            return standard_poly_value(1.0, y, coeffs) + 
                d_standard_poly_dx(1.0, y, coeffs)*(x - 1.0);
        case 2:
            return standard_poly_value(1.0, 1.0, coeffs) + 
                d_standard_poly_dx(1.0, 1.0, coeffs)*(x - 1.0) +
                d_standard_poly_dy(1.0, 1.0, coeffs)*(y - 1.0);
        default:
            assert(0);
        }
    default:
        assert(0);
    }
}

double poly_3_2d_explicit(double x, double y,
                          double xmin, double xmax,
                          double ymin, double ymax,
                          double a, double b,
                          double c, double d,
                          double e, double f,
                          double g, double h,
                          double i, double j)
{
    double coeffs[10];
    coeffs[0] = a;
    coeffs[1] = b;
    coeffs[2] = c;
    coeffs[3] = d;
    coeffs[4] = e;
    coeffs[5] = f;
    coeffs[6] = g;
    coeffs[7] = h;
    coeffs[8] = i;
    coeffs[9] = j;
    return poly_3_2d(x, y, xmin, xmax, ymin, ymax, coeffs);
}

#ifdef __cplusplus
}
#endif
