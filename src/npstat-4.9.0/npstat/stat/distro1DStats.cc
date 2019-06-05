#include <cmath>
#include <vector>
#include <stdexcept>

#include "npstat/stat/distro1DStats.hh"

namespace npstat {
    void distro1DStats(const AbsDistribution1D& distro,
                       const double xmin, const double xmax,
                       const unsigned nPointsToUse,
                       double* pmean, double* pstdev,
                       double* pskewness, double* pkurtosis)
    {
        if (pmean || pstdev || pskewness || pkurtosis)
        {
            if (nPointsToUse < 2) throw std::invalid_argument(
                "In npstat::distro1DStats: not enough points");
            const double dx = (xmax - xmin)/nPointsToUse;
            long double sum = 0.0L, wsum = 0.0L;
            for (unsigned i=0; i<nPointsToUse; ++i)
            {
                const double x = xmin + (i + 0.5)*dx;
                const double dens = distro.density(x);
                sum += x*dens;
                wsum += dens;
            }
            if (wsum <= 0.0L) throw std::invalid_argument(
                "In npstat::distro1DStats: the support of the density "
                "is inconsistent with the requested interval sampling");
            const double mean = sum/wsum;
            if (pmean)
                *pmean = mean;
            if (pstdev || pskewness || pkurtosis)
            {
                long double sum2 = 0.0L, sum3 = 0.0L, sum4 = 0.0L;
                for (unsigned i=0; i<nPointsToUse; ++i)
                {
                    const double x = xmin + (i + 0.5)*dx;
                    const double dens = distro.density(x);
                    const double delta = x - mean;
                    const double delta2 = delta*delta;
                    const double delta3 = delta2*delta;
                    const double delta4 = delta2*delta2;
                    sum2 += delta2*dens;
                    sum3 += delta3*dens;
                    sum4 += delta4*dens;
                }
                const double mu2 = sum2/wsum;
                if (mu2 <= 0.0) throw std::invalid_argument(
                    "In npstat::distro1DStats: standard deviation "
                    "is zero with the requested interval sampling");
                const double stdev = sqrt(mu2);
                if (pstdev)
                    *pstdev = stdev;
                if (pskewness)
                {
                    const double mu3 = sum3/wsum;
                    *pskewness = mu3/(mu2*stdev);
                }
                if (pkurtosis)
                {
                    const double mu4 = sum4/wsum;
                    *pkurtosis = mu4/mu2/mu2;
                }
            }
        }
    }
}
