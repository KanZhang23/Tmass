#include <cmath>
#include <cassert>

#include "npstat/nm/OrthoPoly1D.hh"

#include "WeightTableFilter1DBuilder.hh"

static const double doubleOne = 1.0;

static npstat::OrthoPoly1D* build(
    unsigned* filterCenter, const unsigned maxDegree,
    const double* weight, const unsigned len,
    const unsigned center, const double step)
{
    *filterCenter = center;
    return new npstat::OrthoPoly1D(maxDegree, weight, len, step);
}

namespace npstat {
    WeightTableFilter1DBuilder::WeightTableFilter1DBuilder(
        const double* weight, const unsigned weightLen,
        const unsigned char* exclusionMask, const unsigned exclusionMaskLen,
        const bool excludeCentralPoint)
    {
        if (!weightLen) throw std::invalid_argument(
            "In npstat::WeightTableFilter1DBuilder constructor: "
            "array of weights must not be empty");
        assert(weight);

        // Construct the full weight
        w_.resize(2*weightLen - 1);
        w_[weightLen - 1] = excludeCentralPoint ? 0.0 : weight[0];       
        for (unsigned i=1; i<weightLen; ++i)
        {
            w_[weightLen + i - 1] = weight[i];
            w_[weightLen - i - 1] = weight[i];
        }

        // Fill the exclusion mask
        if (exclusionMask && exclusionMaskLen)
        {
            exclusionMask_.resize(exclusionMaskLen);
            for (unsigned i=0; i<exclusionMaskLen; ++i)
                exclusionMask_[i] = exclusionMask[i];
            wexcl_.resize(w_.size());
        }
    }

    OrthoPoly1D* WeightTableFilter1DBuilder::makeOrthoPoly(
        const unsigned maxDeg, const unsigned i,
        const unsigned dataLen, unsigned* filterCenter) const
    {
        if (!dataLen) throw std::invalid_argument(
            "In npstat::WeightTableFilter1DBuilder::makeOrthoPoly: "
            "invalid data length parameter");
        assert(filterCenter);
        assert(i < dataLen);

        const double step = 1.0/sqrt(1.0*w_.size());
        const unsigned weightLen = w_.size();
        const unsigned n_half = weightLen/2;

        const bool hasExcl = !exclusionMask_.empty();
        if (hasExcl)
        {
            if (exclusionMask_.size() != dataLen)
                throw std::invalid_argument(
                    "In npstat::WeightTableFilter1DBuilder::makeOrthoPoly: "
                    "data length is not compatible with the exclusion mask");

            // Weight element number n_half corresponds to mask element i
            for (unsigned iw=0; iw<weightLen; ++iw)
            {
                wexcl_[iw] = w_[iw];
                const int imask = static_cast<int>(iw + i) -
                                  static_cast<int>(n_half);
                if (imask >= 0 && imask < static_cast<int>(dataLen))
                    if (exclusionMask_[imask])
                        wexcl_[iw] = 0.0;
            }
        }

        if (i < n_half)
        {
            // Create left boundary weight and polynomials
            unsigned n_use = n_half + i + 1;
            if (n_use > dataLen)
                n_use = dataLen;
            const double* wptr = hasExcl ? &wexcl_[n_half-i] : &w_[n_half-i];
            return build(filterCenter, maxDeg, wptr, n_use, i, step);
        }
        else if (i + n_half >= dataLen)
        {
            // Create right boundary weight and polynomials
            const unsigned n_use = dataLen + n_half - i;
            assert(n_use <= dataLen);
            const double* wptr = hasExcl ? &wexcl_[0] : &w_[0];
            return build(filterCenter, maxDeg, wptr, n_use, n_half, step);
        }
        else
        {
            // Symmetric weight
            const double* wptr = hasExcl ? &wexcl_[0] : &w_[0];
            return build(filterCenter, maxDeg, wptr, w_.size(), n_half, step);
        }
    }

    NonmodifyingFilter1DBuilder::NonmodifyingFilter1DBuilder()
        : WeightTableFilter1DBuilder(&doubleOne, 1U)
    {
    }

    OrthoPoly1D* NonmodifyingFilter1DBuilder::makeOrthoPoly(
        unsigned /* maxDegree */, const unsigned binnum,
        const unsigned datalen, unsigned* filterCenter) const
    {
        return WeightTableFilter1DBuilder::makeOrthoPoly(
            0U, binnum, datalen, filterCenter);
    }
}
