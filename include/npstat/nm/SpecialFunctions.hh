#ifndef NPSTAT_SPECIALFUNCTIONS_HH_
#define NPSTAT_SPECIALFUNCTIONS_HH_

/*!
// \file SpecialFunctions.hh
//
// \brief Mathematical special functions
//
// A number of mathematical special functions needed by this package.
// The implementations are not optimized in any systematic way, and
// may be slow.
//
// Author: I. Volobouev
//
// November 2009
*/

namespace npstat {
    /** Inverse cumulative distribition function for 1-d Gaussian */
    double inverseGaussCdf(double cdf);

    /** Regularized incomplete beta function */
    double incompleteBeta(double a, double b, double x);

    /** Inverse regularized incomplete beta function */
    double inverseIncompleteBeta(double a, double b, double x);

    /** The gamma function for positive real arguments */
    double Gamma(double x);

    /** Incomplete gamma ratio */
    double incompleteGamma(double a, double x);

    /** Incomplete gamma ratio complement */
    double incompleteGammaC(double a, double x);

    /** Inverse incomplete gamma ratio */
    double inverseIncompleteGamma(double a, double x);

    /** Inverse incomplete gamma ratio complement */
    double inverseIncompleteGammaC(double a, double x);

    /** Dawson's integral  exp(-x^2) Int_0^x exp(t^2) dt */
    long double dawsonIntegral(long double x);

    /** Inverse of the integral  Int_0^x exp(t^2) dt */
    long double inverseExpsqIntegral(long double x);

    /** Order n derivative of Gaussian density with mean 0 and sigma 1 */
    long double normalDensityDerivative(unsigned n, long double x);

    /** Bivariate normal cumulative probability */
    double bivariateNormalIntegral(double rho, double x, double y);
}

#endif // NPSTAT_SPECIALFUNCTIONS_HH_
