#include <cmath>
#include <stdexcept>
#include <sstream>

#include "npstat/stat/PearsonsChiSquared.hh"
#include "npstat/stat/Distributions1D.hh"

namespace npstat {
    PearsonsChiSquared::PearsonsChiSquared(double minCountToUse,
                                           double subtractForNDoFCalculation,
                                           bool forUncertaintyUseReferenceOnly)
        : minCount_(minCountToUse),
          subtractForNDoF_(subtractForNDoFCalculation),
          referenceOnly_(forUncertaintyUseReferenceOnly)
    {
    }

    const char* PearsonsChiSquared::name() const
    {
        if (name_.empty())
        {
            std::ostringstream os;
            os << "Pearson's chi-squared, " << minCount_ << " min counts";
            if (referenceOnly_)
                os << ", reference only";
            name_ = os.str();
        }
        return name_.c_str();
    }

    void PearsonsChiSquared::compareD(const double* data,
                                      const double* reference,
                                      const unsigned len,
                                      double* distance, double* pvalue) const
    {
        long double sum = 0.0L;
        unsigned npass = 0;

        for (unsigned i=0; i<len; ++i)
        {
            const double denom = referenceOnly_ ? reference[i] :
                                 reference[i] + data[i];
            if (denom >= minCount_)
            {
                ++npass;
                const double d = data[i] - reference[i];
                if (d)
                {
                    if (denom <= 0.0) throw std::invalid_argument(
                        "In npstat::PearsonsChiSquared::compareD: "
                        "uncertainty in the denominator is not positive");
                    sum += d*d/denom;
                }
            }
        }

        const double ndof = npass - subtractForNDoF_;
        if (ndof <= 0.0) throw std::invalid_argument(
            "In npstat::PearsonsChiSquared::compareD: zero degrees of freedom");

        if (distance)
            *distance = sum;

        if (pvalue)
        {
            Gamma1D g(0.0, 2.0, ndof/2.0);
            *pvalue = g.exceedance(sum);
        }
    }
}
