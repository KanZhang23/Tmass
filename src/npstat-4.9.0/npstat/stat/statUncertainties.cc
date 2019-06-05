#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "npstat/stat/statUncertainties.hh"
#include "npstat/stat/Distributions1D.hh"

#define SQR2 1.414213562373095
#define SQRPIOVER2 1.25331413731550025

namespace npstat {
    double meanUncertainty(const double sigmaEstimate, const unsigned long n)
    {
        double u = 0.0;
        if (n)
            u = sigmaEstimate/std::sqrt(static_cast<double>(n));
        else
            throw std::invalid_argument("In npstat::meanUncertainty: "
                                        "sample size must be positive");
        return u;
    }

    double gaussianMedianUncertainty(const double sigmaEstimate, const unsigned long n)
    {
        return SQRPIOVER2*meanUncertainty(sigmaEstimate, n);
    }

    double quantileUncertainty(const AbsDistribution1D& g,
                               const double cdf, const unsigned long n)
    {
        double u = 0.0;
        if (n)
        {
            if (cdf > 0.0 && cdf < 1.0)
                u = sqrt(cdf*(1.0-cdf)/n)/g.density(g.quantile(cdf));
            else
                throw std::domain_error("In npstat::quantileUncertainty: "
                                        "input cdf value is out of range");
        }
        else
            throw std::invalid_argument("In npstat::quantileUncertainty: "
                                        "sample size must be positive");
        return u;
    }

    double varianceUncertainty(const double sigma, const double kurtosis,
                               const unsigned long n)
    {
        double u = 0.0;
        if (n > 1)
            u = sigma*sigma*std::sqrt((kurtosis - 3.0 + n*2.0/(n - 1))/n);
        else
            throw std::invalid_argument("In npstat::varianceUncertainty: "
                                        "sample size must be larger than 1");
        return u;
    }

    double gaussianVarianceUncertainty(const double sigmaEstimate,
                                       const unsigned long n)
    {
        double u = 0.0;
        if (n > 1)
            u = sigmaEstimate*sigmaEstimate*SQR2/std::sqrt(static_cast<double>(n-1));
        else
            throw std::invalid_argument("In npstat::gaussianVarianceUncertainty: "
                                        "sample size must be larger than 1");
        return u;
    }

    double gaussianStdevUncertainty(const double sigmaEstimate,
                                    const unsigned long n)
    {
        double u = 0.0;
        if (n > 1)
            u = sigmaEstimate/SQR2/std::sqrt(static_cast<double>(n-1));
        else
            throw std::invalid_argument("In npstat::gaussianStdevUncertainty: "
                                        "sample size must be larger than 1");
        return u;
    }

    // The formulae below come from D. N. Joanes and C. A. Gill,
    // "Comparing measures of sample skewness and kurtosis",
    // The Statistician, 47, pp 183-189 (1998). The article
    // has a typo for the variance of skewness which is fixed
    // in the code.
    double gaussianSkewnessUncertainty(const unsigned long un)
    {
        double u = 0.0;
        if (un > 2)
        {
            const double n = un;
            u = sqrt(6*n*(n - 1)/((n - 2)*(n + 1)*(n + 3)));
        }
        else
            throw std::invalid_argument("In npstat::gaussianSkewnessUncertainty: "
                                        "sample size must be larger than 2");
        return u;
    }

    double gaussianKurtosisUncertainty(const unsigned long un)
    {
        double u = 0.0;
        if (un > 3)
        {
            const double n = un;
            u = sqrt(24*(n - 1)*(n - 1)*n/((n - 3)*(n - 2)*(3 + n)*(5 + n)));
        }
        else
            throw std::invalid_argument("In npstat::gaussianKurtosisUncertainty: "
                                        "sample size must be larger than 3");
        return u;
    }

    double cdfUncertainty(const double cdf, const unsigned long n)
    {
        double u = 0.0;
        if (n)
        {
            if (cdf >= 0.0 && cdf <= 1.0)
                u = sqrt(cdf*(1.0-cdf)/n);
            else
                throw std::domain_error("In npstat::cdfUncertainty: "
                                        "input cdf value is out of range");
        }
        else
            throw std::invalid_argument("In npstat::cdfUncertainty: "
                                        "sample size must be positive");
        return u;
    }

    double quantileDeltaUncertainty(const AbsDistribution1D& g,
                                    const double cdf1, const double cdf2,
                                    const unsigned long n)
    {
        double u = 0.0;
        if (n)
        {
            if (cdf1 > 0.0 && cdf1 < 1.0 &&
                cdf2 > 0.0 && cdf2 < 1.0)
            {
                if (cdf1 != cdf2)
                {
                    double p1 = 1.0 - cdf1;
                    double p2 = 1.0 - cdf2;
                    const double s1 = sqrt(cdf1*p1/n)/g.density(g.quantile(cdf1));
                    const double s2 = sqrt(cdf2*p2/n)/g.density(g.quantile(cdf2));

                    // The following estimate of the quantile correlation
                    // coefficient comes from the monograph by G. Udny Yule,
                    // "An introduction to the theory of statistics" (1911).
                    if (p2 > p1)
                        std::swap(p1, p2);
                    double r = sqrt((p2*(1.0 - p1))/(p1*(1.0 - p2)));
                    if (r > 1.0)
                        r = 1.0;
                    u = sqrt(s1*s1 + s2*s2 - 2.0*r*s1*s2);
                }
            }
            else
                throw std::domain_error("In npstat::quantileDeltaUncertainty: "
                                        "input cdf value out of range");
        }
        else
            throw std::invalid_argument("In npstat::quantileDeltaUncertainty: "
                                        "sample size must be positive");
        return u;
    }
}
