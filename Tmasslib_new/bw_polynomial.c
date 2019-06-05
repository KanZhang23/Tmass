#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "bw_polynomial.h"

#define sz(x) (sizeof(x)/sizeof(x[0]) - 1)

#ifdef __cplusplus
extern "C" {
#endif

inline static double poly_sum(const double *a, int deg, const double x)
{
    /* Dimension of a should be at least deg+1 */
    register double sum = 0.0;
    for (; deg>=0; --deg)
    {
	sum *= x;
	sum += a[deg];
    }
    return sum;
}

double bw_polynomial(const double x0, const unsigned power)
{
    static const double a2[]  = {1};
    static const double a3[]  = {2};
    static const double a4[]  = {-1, 3};
    static const double a5[]  = {-4, 4};
    static const double a6[]  = {1, -10, 5};
    static const double a7[]  = {6, -20, 6};
    static const double a8[]  = {-1, 21, -35, 7};
    static const double a9[]  = {-8, 56, -56, 8};
    static const double a10[] = {1, -36, 126, -84, 9};
    static const double a11[] = {10, -120, 252, -120, 10};
    static const double a12[] = {-1, 55, -330, 462, -165, 11};
    static const double a13[] = {-12, 220, -792, 792, -220, 12};
    static const double a14[] = {1, -78, 715, -1716, 1287, -286, 13};
    static const double a15[] = {14, -364, 2002, -3432, 2002, -364, 14};
    static const double a16[] = {-1, 105, -1365, 5005, -6435, 3003, -455, 15};
    static const double a17[] = {-16, 560, -4368, 11440, -11440, 4368, -560, 16};
    static const double a18[] = {1, -136, 2380, -12376, 24310, -19448, 6188, -680, 17};
    static const double a19[] = {18, -816, 8568, -31824, 48620, -31824, 8568, -816, 18};
    static const double a20[] = {-1, 171, -3876, 27132, -75582, 92378, -50388, 11628, -969, 19};
    static const double *a[MAX_BW_POLY_DEGREE-1] = 
        {a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
         a12, a13, a14, a15, a16, a17, a18, a19, a20};
    static const size_t l[MAX_BW_POLY_DEGREE-1] = 
        {sz(a2), sz(a3), sz(a4), sz(a5), sz(a6), sz(a7), sz(a8),
         sz(a9), sz(a10), sz(a11), sz(a12), sz(a13), sz(a14),
         sz(a15), sz(a16), sz(a17), sz(a18), sz(a19), sz(a20)};
    double pvalue;

    if (power < 2)
        pvalue = 0.0;
    else if (power <= MAX_BW_POLY_DEGREE)
        pvalue = poly_sum(a[power-2], l[power-2], x0*x0);
    else
        assert(0);
    if (power % 2)
        return x0*pvalue;
    else
        return pvalue;
}

double bw_expansion(const double x, const double *coeffs,
                    const unsigned maxdegree)
{
    assert(maxdegree <= MAX_BW_POLY_DEGREE);
    if (fabs(x) < 1.0 || maxdegree < 2)
        return 0.0;
    else
    {
        const double invx = 1.0/x;
        return invx*invx*poly_sum(coeffs, maxdegree-2, invx);
    }
}

double bw_expansion_integral(const double x, const double *coeffs,
                             const unsigned maxdegree)
{
    assert(maxdegree <= MAX_BW_POLY_DEGREE);
    if (fabs(x) < 1.0 || maxdegree < 2)
        return 0.0;
    else
    {
        unsigned i;
        const unsigned imax = maxdegree-1;
        double integ_coeffs[MAX_BW_POLY_DEGREE-1];
        const double invx = 1.0/x;
        for (i=0; i<imax; ++i)
            integ_coeffs[i] = coeffs[i]/(i + 1);
        return -invx*poly_sum(integ_coeffs, maxdegree-2, invx);
    }
}

#ifdef __cplusplus
}
#endif
