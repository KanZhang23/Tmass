#include <cassert>
#include <climits>
#include <stdexcept>
#include <cmath>

#include "npstat/rng/HOSobolGenerator.hh"


static inline void set_bit(unsigned long long& number, const unsigned bin,
                           const bool value)
{
    if (value)
        number |= (1LL << bin);
    else
        number &= ~(1LL << bin);
}


static inline bool get_bit(const long long& number, const unsigned bin)
{
    return number & (1LL << bin);
}


namespace npstat {
    HOSobolGenerator::HOSobolGenerator(const unsigned indim,
                                       const unsigned interlacingFactor,
                                       const unsigned maxPowerOfTwo,
                                       const unsigned nSkip)
        : SobolGenerator(indim*interlacingFactor, maxPowerOfTwo, nSkip),
          normfactor_(1.0L/(powl(2.0L, maxPowerOfTwo*interlacingFactor))),
          dim_(indim),
          d_(interlacingFactor)
    {
        if (!interlacingFactor) throw std::invalid_argument(
            "In npstat::HOSobolGenerator constructor: "
            "interlacing factor must be positive");
        if (interlacingFactor > 1U)
            if (!(maxPowerOfTwo*interlacingFactor <= 
                  CHAR_BIT*sizeof(unsigned long long)))
                throw std::invalid_argument(
                    "In npstat::HOSobolGenerator constructor: "
                    "interlacing factor is too large");
    }

    void HOSobolGenerator::run(double* buf, const unsigned bufSize,
                               const unsigned nPoints)
    {
        if (d_ == 1U)
        {
            // No scrambling. Just run the vanilla Sobol generator.
            SobolGenerator::run(buf, bufSize, nPoints);
        }
        else if (nPoints)
        {
            if (bufSize < dim_*nPoints) throw std::invalid_argument(
                "In npstat::HOSobolGenerator::run: insufficient buffer length");
            assert(buf);

            const unsigned maxp2 = maxPowerOfTwo();
            for (unsigned ipt=0; ipt<nPoints; ++ipt)
            {
                nextBitSet(bitset_, DIM_MAX);
                for (unsigned i=0; i<dim_; ++i)
                    newset_[i] = 0ULL;
                for (unsigned j=0; j<dim_; ++j)
                    for (unsigned i=1; i<=maxp2; ++i)
                        for (unsigned k=1; k<=d_; ++k)
                            set_bit(newset_[j], maxp2*d_-k-(i-1)*d_,
                                    get_bit(bitset_[j*d_+k-1], maxp2-i));
                for (unsigned i=0; i<dim_; ++i)
                    *buf++ = newset_[i] * normfactor_;
            }
        }
    }
}
