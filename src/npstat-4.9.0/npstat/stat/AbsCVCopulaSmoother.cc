#include <stdexcept>

#include "npstat/nm/allocators.hh"
#include "npstat/nm/EquidistantSequence.hh"

#include "npstat/stat/AbsCVCopulaSmoother.hh"

namespace npstat {
    AbsCVCopulaSmoother::AbsCVCopulaSmoother(
        const unsigned* nBinsInEachDim, const unsigned dim,
        const double marginTolerance, const unsigned maxNormCycles,
        const double initialBw, double cvFactor, const unsigned nCV,
        const bool useConvolve)
        : AbsCopulaSmootherBase(nBinsInEachDim, dim,
                                marginTolerance, maxNormCycles),
          density_(nBinsInEachDim, dim),
          bestFilt_(0),
          useConvolute_(useConvolve)
    {
        if (initialBw <= 0.0) throw std::invalid_argument(
            "In npstat::AbsCVCopulaSmoother constructor: "
            "initial bandwidth must be positive");
        const unsigned nFilters = nCV % 2U ? nCV : nCV + 1U;
        if (nFilters == 1)
            cvFactor = 1.0;
        if (cvFactor <= 0.0) throw std::invalid_argument(
            "In npstat::AbsCVCopulaSmoother constructor: "
            "cross validation scale factor must be positive");
        bandwidthValues_ = EquidistantInLogSpace(
            initialBw/cvFactor, initialBw*cvFactor, nFilters);
        cvValues_.resize(nFilters);
        clearBuffer(&cvValues_[0], nFilters);
        regFractions_.resize(nFilters);
        clearBuffer(&regFractions_[0], nFilters);
    }

    AbsCVCopulaSmoother::AbsCVCopulaSmoother(
        const unsigned* nBinsInEachDim, const unsigned dim,
        const double marginTolerance, const unsigned maxNormCycles,
        const std::vector<double>& i_bandwidthValues,
        const bool useConvolve)
        : AbsCopulaSmootherBase(nBinsInEachDim, dim,
                                marginTolerance, maxNormCycles),
          bandwidthValues_(i_bandwidthValues),
          cvValues_(i_bandwidthValues.size(), 0.0),
          regFractions_(i_bandwidthValues.size(), 0.0),
          density_(nBinsInEachDim, dim),
          bestFilt_(0),
          useConvolute_(useConvolve)
    {
        if (bandwidthValues_.empty()) throw std::invalid_argument(
            "In npstat::AbsCVCopulaSmoother constructor: "
            "must provide at least one bandwidth value");
    }

    void AbsCVCopulaSmoother::smoothHisto(HistoND<double>& histo,
                                          const double effectiveSampleSize,
                                          double* bandwidthUsed,
                                          const bool isSampleWeighted)
    {
        bestFilt_ = 0;
        double bestCV = 0.0;
        bool isPositiveAndNormalized = false;

        const unsigned nFilters = bandwidthValues_.size();
        for (unsigned ifil=0; ifil<nFilters; ++ifil)
        {
            const double cv = smoothAndCV(
                histo, effectiveSampleSize, isSampleWeighted,
                ifil, nFilters > 1U,
                &density_, &regFractions_[ifil], &isPositiveAndNormalized);
            if (ifil)
            {
                if (cv > bestCV)
                {
                    bestCV = cv;
                    bestFilt_ = ifil;
                }
            }
            else
                bestCV = cv;
            cvValues_[ifil] = cv;
        }
        if (bestFilt_ != nFilters - 1U)
        {
            double dummy = 0.0;
            smoothAndCV(histo, effectiveSampleSize, isSampleWeighted,
                        bestFilt_, false,
                        &density_, &dummy, &isPositiveAndNormalized);
        }

        if (!isPositiveAndNormalized)
        {
            density_.makeNonNegative();
            const unsigned long nBinsTotal = density_.length();
            const double sum = density_.sum<long double>();
            assert(sum > 0.0);
            density_ *= (nBinsTotal/sum);
        }
        histo.setBinContents(density_.data(), density_.length());
        if (bandwidthUsed)
            *bandwidthUsed = bandwidthValues_[bestFilt_];
    }
}
