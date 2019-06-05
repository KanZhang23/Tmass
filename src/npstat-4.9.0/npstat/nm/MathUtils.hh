#ifndef NPSTAT_MATHUTILS_HH_
#define NPSTAT_MATHUTILS_HH_

/*!
// \file MathUtils.hh
//
// \brief Various simple mathematical utilities which did not end up inside
//        dedicated headers
//
// Author: I. Volobouev
//
// March 2010
*/

namespace npstat {
    /**
    // Solve the quadratic equation x*x + b*x + c == 0
    // in a numerically sound manner. Return the number of roots.
    */
    unsigned solveQuadratic(double b, double c,
                            double *x1, double *x2);

    /**
    // Find the real roots of the cubic:
    //   x**3 + p*x**2 + q*x + r = 0
    // The number of real roots is returned, and the roots are placed
    // into the "v3" array. Original code by Don Herbison-Evans (see
    // his article "Solving Quartics and Cubics for Graphics" in the
    // book "Graphics Gems V", page 3), with minimal adaptation for
    // this package by igv.
    */
    unsigned solveCubic(double p, double q, double r, double v3[3]);

    /**
    // Find an extremum of a parabola passing through the three given points.
    // The returned value is "true" for minimum and "false" for maximum.
    // std::invalid_argument will be thrown in case some of the x values
    // coincide or if the coordinates describe a straight line.
    */
    bool parabolicExtremum(double x0, double y0, double x1, double y1,
                           double x2, double y2, double* extremumCoordinate,
                           double* extremumValue);

    /**
    // Volume of the n-dimensional sphere of unit radius. Should be
    // multuplied by R^n to get the volume of the sphere with arbitrary radius.
    */
    double ndUnitSphereVolume(unsigned n);

    /**
    // Area of the sphere of unit radius embedded in the n-dimensional space.
    // Should be multuplied by R^(n-1) to get the area of the sphere with
    // arbitrary radius.
    */
    double ndUnitSphereArea(unsigned n);

    /**
    // Sum of polynomial series. The length of the
    // array of coefficients should be at least degree+1.
    // The highest degree coefficient is assumed to be
    // the last one in the "coeffs" array (0th degree
    // coefficient comes first).
    */
    template<typename Numeric>
    long double polySeriesSum(const Numeric *coeffs,
                              unsigned degree, long double x);

    /**
    // Sum and derivative of polynomial series. The length of the
    // array of coefficients should be at least degree+1.
    */
    template<typename Numeric>
    void polyAndDeriv(const Numeric *coeffs, unsigned degree,
                      long double x, long double *value, long double *deriv);

    /**
    // Series using Legendre polynomials. 0th degree coefficient
    // comes first. Although any value of x can be specified, the result
    // is not going to be terribly meaningful in case |x| > 1.
    */
    template<typename Numeric>
    long double legendreSeriesSum(const Numeric *coeffs,
                                  unsigned degree, long double x);

    /**
    // Series for Hermite polynomials orthogonal with weight exp(-x*x/2).
    // These are sometimes called the "probabilists' Hermite polynomials".
    */
    template<typename Numeric>
    long double hermiteSeriesSumProb(const Numeric *coeffs,
                                     unsigned degree, long double x);

    /**
    // Series for Hermite polynomials orthogonal with weight exp(-x*x). These
    // are sometimes called the "physicists' Hermite polynomials". The weight
    // exp(-x*x) is also used in the "GaussHermiteQuadrature" class.
    */
    template<typename Numeric>
    long double hermiteSeriesSumPhys(const Numeric *coeffs,
                                     unsigned degree, long double x);

    /** Series for Gegenbauer polynomials. "lambda" is the parameter. */
    template<typename Numeric>
    long double gegenbauerSeriesSum(const Numeric *coeffs, unsigned degree, 
                                    long double lambda, long double x);

    /** Series for the Chebyshev polynomials of the first kind */
    template<typename Numeric>
    long double chebyshevSeriesSum(const Numeric *coeffs,
                                   unsigned degree, long double x);

    /**
    // Generate Chebyshev series coefficients for a given functor on the
    // interval [xmin, xmax]. The array of coefficients must be at least
    // degree+1 long. The functor will be given a long double argument.
    */
    template<typename Functor, typename Numeric>
    void chebyshevSeriesCoeffs(const Functor& f,
                               long double xmin, long double xmax,
                               unsigned degree, Numeric *coeffs);
}

#include "npstat/nm/MathUtils.icc"

#endif // NPSTAT_MATHUTILS_HH_
