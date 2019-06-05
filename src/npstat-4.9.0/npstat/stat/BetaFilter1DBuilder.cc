#include <stdexcept>

#include "npstat/stat/BetaFilter1DBuilder.hh"
#include "npstat/nm/OrthoPoly1D.hh"

namespace npstat {
    BetaFilter1DBuilder::BetaFilter1DBuilder(
        const double polyDegree, const unsigned dataLen,
        const double effectiveDegreeOffset, const double effectiveDegreeCutoff)
        : betaSet_(polyDegree, dataLen),
          polyDegree_(polyDegree),
          effectiveOffset_(effectiveDegreeOffset),
          effectiveCutoff_(effectiveDegreeCutoff),
          dataLen_(dataLen)
    {
        if (dataLen < 2U) throw std::invalid_argument(
            "In npstat::BetaFilter1DBuilder constructor: "
            "invalid data length parameter");

        if (polyDegree < 0.0)
            throw std::invalid_argument(
                "In npstat::BetaFilter1DBuilder constructor: "
                "invalid polynomial degree parameter");

        if (effectiveOffset_ < -1.0)
            effectiveOffset_ = -1.0;
        if (effectiveOffset_ > 0.0)
            effectiveOffset_ = 0.0;

        if (effectiveCutoff_ < -1.0)
            effectiveCutoff_ = -1.0;
        if (effectiveCutoff_ > 0.0)
            effectiveCutoff_ = 0.0;
    }

    void BetaFilter1DBuilder::fillContinuousBeta(const unsigned binnum,
                                                 long double* f) const
    {
        const double w = (polyDegree_+1.0)/dataLen_;

        // Simpson integration across the bin interval
        const double weights[3] = {1.0/6.0, 4.0/6.0, 1.0/6.0};
        const unsigned nWeights = sizeof(weights)/sizeof(weights[0]);
        const double step = 1.0/(nWeights - 1U);

        for (unsigned istep=0; istep<nWeights; ++istep)
        {
            const double x = (binnum+istep*step)/dataLen_;
            double effDeg = x*(polyDegree_ - 2.0*effectiveOffset_) + 
                effectiveOffset_;
            if (effDeg < effectiveCutoff_)
                effDeg = effectiveCutoff_;
            if (effDeg > polyDegree_ - effectiveCutoff_)
                effDeg = polyDegree_ - effectiveCutoff_;
            const double effw = weights[istep]*w;
            for (unsigned i=0; i<dataLen_; ++i)
                f[i] += effw*betaSet_.poly(effDeg, i);
        }
    }

    PolyFilter1D* BetaFilter1DBuilder::makeFilter(
        const double* /* taper */, unsigned /* lenTaper */,
        const unsigned binnum, const unsigned datalen) const
    {
        if (datalen != dataLen_) throw std::invalid_argument(
            "In npstat::BetaFilter1DBuilder::makeFilter: "
            "inconsistent data length");
        if (binnum >= dataLen_) throw std::out_of_range(
            "In npstat::BetaFilter1DBuilder::makeFilter: "
            "bin number out of range");

        PolyFilter1D* filter = new PolyFilter1D(binnum);
        filter->resize(datalen);
        long double* f = &(*filter)[0];
        for (unsigned i=0; i<datalen; ++i)
            f[i] = 0.0L;
        fillContinuousBeta(binnum, f);
        return filter;
    }
}
