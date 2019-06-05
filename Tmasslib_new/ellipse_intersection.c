#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "sorter.h"
#include "quartic_lib.h"
#include "simple_kinematics.h"
#include "ellipse_intersection.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double d1;
    double d2;
} d_d_pair;

sort_struct_by_member_incr(d_d_pair, d1)

static int checked_quartic_roots(double a, double b, double c,
                                 double d, double e, double rts[4])
{
    double A, B, C, D;
    double quartic_roots[4];
    int iroot, nroots, npb = 0;

    assert(a != 0.0);
    A = b/a;
    B = c/a;
    C = d/a;
    D = e/a;

    nroots = quartic(A, B, C, D, quartic_roots);

    for (iroot = 0; iroot < nroots; ++iroot)
    {
        /* Check the solution validity */
        const double x0     = quartic_roots[iroot];
        const double f0     = x0*(x0*(x0*(x0 + A) + B) + C) + D;
        const double deriv1 = x0*(x0*(4.0*x0 + 3.0*A) + 2.0*B) + C;
        const double deriv2 = x0*(12.0*x0 + 6.0*A) + 2.0*B;
        const double deriv3 = 24.0*x0 + 6.0*A;
        double newroot = DBL_MAX;

        if (deriv3 != 0.0)
        {
            /* Approximate using cubic */
            double x[4];
            const int ncubroots = cubic(
                deriv2/deriv3*3.0, deriv1/deriv3*6.0,
                f0/deriv3*6.0, x);
            const int require_roots = deriv1*deriv1 > 2.0*deriv2*f0 ? 1 : 3;
            if (ncubroots >= require_roots)
            {
                double mindiff = 1.0;
                int i;
                for (i=0; i<ncubroots; ++i)
                {
                    const double absdiff = fabs(x[i]);
                    if (absdiff < mindiff)
                    {
                        mindiff = absdiff;
                        newroot = x0 + x[i];
                    }
                }
            }
        }
        else if (deriv1*deriv1 > 2.0*deriv2*f0)
        {
            /* Looks like the solution is real.
             * Improve the precision.
             */
            if (deriv2 == 0.0)
                newroot = x0 - f0/deriv1;
            else
            {
                double x1, x2;
                if (solve_quadratic(2.0*deriv1/deriv2,
                                    2.0*f0/deriv2, &x1, &x2))
                {
                    const double dx = fabs(x1) < fabs(x2) ? x1 : x2;
                    if (fabs(dx) < 1.0)
                        newroot = x0 + dx;
                }
            }
        }

        if (newroot < DBL_MAX*0.999)
            rts[npb++] = newroot;
    }

    return npb;
}

static double best_match(double x[2][2])
{
    d_d_pair p[4];
    p[0].d1 = fabs(x[0][0] - x[1][0]);
    p[1].d1 = fabs(x[0][0] - x[1][1]);
    p[2].d1 = fabs(x[0][1] - x[1][0]);
    p[3].d1 = fabs(x[0][1] - x[1][1]);
    p[0].d2 = 0.5*(x[0][0] + x[1][0]);
    p[1].d2 = 0.5*(x[0][0] + x[1][1]);
    p[2].d2 = 0.5*(x[0][1] + x[1][0]);
    p[3].d2 = 0.5*(x[0][1] + x[1][1]);
    qsort(p, 4, sizeof(d_d_pair), sort_d_d_pair_by_d1_incr);
    return p[0].d2;
}

int ellipse_intersection(const Ellipse_coeffs e[2],
                         double xout[4], double yout[4])
{
    const double v0 = 2.0*(e[0].a00*e[1].a01 - e[1].a00*e[0].a01);
    const double v1 = e[0].a00*e[1].a11 - e[1].a00*e[0].a11;
    const double v2 = e[0].a00*e[1].b0 - e[1].a00*e[0].b0;
    const double v3 = e[0].a00*e[1].b1 - e[1].a00*e[0].b1;
    const double v4 = e[0].a00*e[1].c - e[1].a00*e[0].c;
    const double v5 = 2.0*(e[0].a01*e[1].a11 - e[1].a01*e[0].a11);
    const double v6 = 2.0*(e[0].a01*e[1].b1 - e[1].a01*e[0].b1);
    const double v7 = 2.0*(e[0].a01*e[1].c - e[1].a01*e[0].c);
    const double v8 = e[0].a11*e[1].b0 - e[1].a11*e[0].b0;
    const double v9 = e[0].b0*e[1].b1 - e[1].b0*e[0].b1;
    const double v10 = e[0].b0*e[1].c - e[1].b0*e[0].c;
    const double u0 = v2*v10 - v4*v4;
    const double u1 = v0*v10 + v2*(v7 + v9) - 2.0*v3*v4;
    const double u2 = v0*(v7 + v9) + v2*(v6 - v8) - v3*v3 - 2.0*v1*v4;
    const double u3 = v0*(v6 - v8) + v2*v5 - 2.0*v1*v3;
    const double u4 = v0*v5 - v1*v1;

    double rts[4];
    const int nroots = checked_quartic_roots(u4, u3, u2, u1, u0, rts);

    int i, iel, ireal = 0;
    for (i=0; i<nroots; ++i)
    {
        const double y = rts[i];
        double x[2][2];
        int n2[2];

        for (iel=0; iel<2; ++iel)
        {
            const double a = e[iel].a00;
            const double b = 2*e[iel].a01*y + e[iel].b0;
            const double c = (e[iel].a11*y + e[iel].b1)*y + e[iel].c;

            assert(a);
            n2[iel] = solve_quadratic(b/a, c/a, &x[iel][0], &x[iel][1]);
        }

        if (n2[0] && n2[1])
        {
            xout[ireal] = best_match(x);
            yout[ireal] = y;
            ++ireal;
        }
    }

    for (i=ireal; i<4; ++i)
    {
        xout[i] = 0.0;
        yout[i] = 0.0;
    }

    return ireal;
}

#ifdef __cplusplus
}
#endif
