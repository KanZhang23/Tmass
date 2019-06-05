#ifndef NPSTAT_STATUNCERTAINTIES_HH_
#define NPSTAT_STATUNCERTAINTIES_HH_

/*!
// \file statUncertainties.hh
//
// \brief Uncertainties of various sample statistics
//
// Author: I. Volobouev
//
// July 2016
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    //
    // The formulae below normally assume that all distribution moments
    // are estimated from the sample itself
    //

    /** Uncertainty of the mean estimate */
    double meanUncertainty(double sigmaEstimate, unsigned long n);

    /** 
    // Uncertainty of the variance (not standard deviation!) estimate.
    // Parameters "sigma" and "kurtosis" are population sigma and
    // kurtosis, not sample estimates.
    */
    double varianceUncertainty(double sigma, double kurtosis, unsigned long n);

    /** Uncertainty of a cdf value (binomial) */
    double cdfUncertainty(double cdf, unsigned long n);

    /** Uncertainty of a quantile value (asymptotic) */
    double quantileUncertainty(const AbsDistribution1D& distro,
                               double cdf, unsigned long n);

    /** Uncertainty of a difference between two quantile values (asymptotic) */
    double quantileDeltaUncertainty(const AbsDistribution1D& distro,
                                    double cdf1, double cdf2, unsigned long n);

    /** Uncertainty of the median estimate for the Gaussian distribution */
    double gaussianMedianUncertainty(double sigmaEstimate, unsigned long n);

    /**
    // Uncertainty of the variance (not standard deviation!)
    // estimate for the Gaussian distribution
    */
    double gaussianVarianceUncertainty(double sigmaEstimate, unsigned long n);

    /** 
    // Uncertainty of the standard deviation estimate
    // for the Gaussian distribution
    */
    double gaussianStdevUncertainty(double sigmaEstimate, unsigned long n);

    /** Skewness uncertainty for the Gaussian distribution */
    double gaussianSkewnessUncertainty(unsigned long n);

    /** Kurtosis uncertainty for the Gaussian distribution */
    double gaussianKurtosisUncertainty(unsigned long n);
}

#endif // NPSTAT_STATUNCERTAINTIES_HH_
