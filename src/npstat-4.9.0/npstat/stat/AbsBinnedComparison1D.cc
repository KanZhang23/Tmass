#include <cmath>
#include <climits>
#include <stdexcept>

#include "npstat/rng/AbsRandomGenerator.hh"

#include "npstat/stat/AbsBinnedComparison1D.hh"
#include "npstat/stat/DiscreteDistributions1D.hh"

namespace npstat {
    unsigned long AbsBinnedComparison1D::realToULong(const long double d)
    {
        if (d < 0.0L) throw std::invalid_argument(
            "In npstat::AbsBinnedComparison1D::realToULong: "
            "input must not be negative");
        const long double f = roundl(d);
        if (f > static_cast<long double>(ULONG_MAX)) throw std::invalid_argument(
            "In npstat::AbsBinnedComparison1D::realToULong: input is too large");
        return static_cast<unsigned long>(f);
    }

    double AbsBinnedComparison1D::pvalueByPseudo(
        const AbsDiscreteDistribution1DDistance& calc, AbsRandomGenerator& rng,
        const DiscreteTabulated1D& data, const double dataSampleSize,
        const DiscreteTabulated1D& reference, const double refSampleSize,
        const double observedDistance, const unsigned nPseudo)
    {
        assert(nPseudo);
        const unsigned long len = data.probabilities().size();
        assert(reference.probabilities().size() == len);

        const unsigned long nData = dataSampleSize > 0.5 ?
                                    realToULong(dataSampleSize) : 0UL;
        const unsigned long nRef  = refSampleSize > 0.5 ?
                                    realToULong(refSampleSize) : 0UL;
        assert(nData || nRef);

        std::vector<unsigned long> countsVec(len);
        unsigned long* counts = &countsVec[0];
        unsigned nHigher = 0;
        long idx = 0;

        if (nData && nRef)
        {
            // This is a two-sample test.
            // Build a pooled distribution and sample that.
            const DiscreteTabulated1D& pooled = pooledDiscreteTabulated1D(
                data, dataSampleSize, reference, refSampleSize, 0L, len);

            for (unsigned ipseudo=0; ipseudo<nPseudo; ++ipseudo)
            {
                clearBuffer(counts, len);
                for (unsigned long i=0; i<nData; ++i)
                {
                    pooled.random(rng, &idx);
                    counts[idx]++;
                }
                DiscreteTabulated1D mc(0, counts, len);

                clearBuffer(counts, len);
                for (unsigned long i=0; i<nRef; ++i)
                {
                    pooled.random(rng, &idx);
                    counts[idx]++;
                }
                DiscreteTabulated1D mcref(0, counts, len);

                const double d = calc(mc, mcref, &pooled, 0L, len);
                if (d >= observedDistance)
                    ++nHigher;
            }
        }
        else
        {
            // Sample the precise distribution only
            const DiscreteTabulated1D& precise(nData ? reference : data);
            const unsigned long nEvents = nData + nRef;

            for (unsigned ipseudo=0; ipseudo<nPseudo; ++ipseudo)
            {
                clearBuffer(counts, len);
                for (unsigned long i=0; i<nEvents; ++i)
                {
                    precise.random(rng, &idx);
                    counts[idx]++;
                }
                DiscreteTabulated1D mc(0, counts, len);

                const double d = calc(mc, precise, 0, 0L, len);
                if (d >= observedDistance)
                    ++nHigher;
            }
        }

        return static_cast<double>(nHigher)/nPseudo;
    }
}
