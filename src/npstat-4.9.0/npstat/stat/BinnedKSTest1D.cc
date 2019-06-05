#include <cmath>
#include <climits>
#include <sstream>
#include <numeric>

#include "kstest/kstest.hh"

#include "npstat/nm/allocators.hh"
#include "npstat/rng/AbsRandomGenerator.hh"
#include "npstat/stat/BinnedKSTest1D.hh"
#include "npstat/stat/DiscreteDistributions1D.hh"

namespace {
    struct DiscreteKSDistance : public npstat::AbsDiscreteDistribution1DDistance
    {
        double operator()(const npstat::AbsDiscreteDistribution1D& d1,
                          const npstat::AbsDiscreteDistribution1D& d2,
                          const npstat::AbsDiscreteDistribution1D* /* pooled */,
                          const long first, const long last) const
        {
            double d = 0.0, maxd = 0.0;
            bool useExceedance = false;

            for (long i=first; i<last; ++i)
            {
                // Try to avoid subtractive cancellation by
                // switching to exceedance at high x
                if (useExceedance)
                    d = fabs(d1.exceedance(i) - d2.exceedance(i));
                else
                {
                    const double cdf1 = d1.cdf(i);
                    const double cdf2 = d2.cdf(i);
                    useExceedance = (cdf1 + cdf2 > 1.0);
                    d = fabs(cdf1 - cdf2);
                }
                if (d > maxd)
                    maxd = d;
            }
            return maxd;
        }
    };
}

namespace npstat {
    BinnedKSTest1D::BinnedKSTest1D(const bool useTwoSampleTest,
                                   AbsRandomGenerator* rng,
                                   const unsigned mcSamplesForPValue)
        : rng_(rng),
          mcSamples_(mcSamplesForPValue),
          twoSampleTest_(useTwoSampleTest)
    {
    }

    const char* BinnedKSTest1D::name() const
    {
        if (name_.empty())
        {
            std::ostringstream os;
            if (twoSampleTest_)
                os << "Two-sample ";
            os << "Kolmogorov-Smirnov";
            if (rng_ && mcSamples_)
                os << " by MC";
            name_ = os.str();
        }
        return name_.c_str();
    }

    void BinnedKSTest1D::compareD(const double* pdata, const double* preference,
                                  const unsigned len, double* distance,
                                  double* pvalue) const
    {
        DiscreteTabulated1D data(0, pdata, len);
        DiscreteTabulated1D ref(0, preference, len);

        DiscreteKSDistance calc;
        const double maxd = calc(data, ref, 0, 0L, static_cast<long>(len));
        if (distance)
            *distance = maxd;

        if (pvalue)
        {
            const double dData = std::accumulate(pdata, pdata+len, 0.0L);
            double dRef = 0.0;
            if (twoSampleTest_)
                dRef = std::accumulate(preference, preference+len, 0.0L);

            if (rng_ && mcSamples_)
            {
                // Calculate the p-value by MC
                *pvalue = pvalueByPseudo(calc, *rng_, data, dData,
                                         ref, dRef, maxd, mcSamples_);
            }
            else
            {
                const double effEvents = twoSampleTest_ ?
                    dData*dRef/(dData + dRef) : dData;
                const unsigned long n = realToULong(effEvents);
                if (n > UINT_MAX) throw std::invalid_argument(
                    "In npstat::BinnedKSTest1D::compareD: "
                    "sample size is too large");
                *pvalue = kstest::KSfbar(n, maxd);
            }
        }
    }
}
