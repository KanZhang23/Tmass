#include <stdexcept>
#include "npstat/rng/RegularSampler1D.hh"

namespace npstat {
    void RegularSampler1D::reset()
    {
        intervalWidth_ = 2.0L;
        nIntervals_ = 0;
        interval_ = 0;
        nCalls_ = 0;
    }

    double RegularSampler1D::operator()()
    {
        if (interval_ == nIntervals_)
        {
            interval_ = 0;
            nIntervals_ = nCalls_ + 1ULL;
            intervalWidth_ /= 2.0L;
        }
        ++nCalls_;
        return (0.5L + interval_++)*intervalWidth_;
    }

    unsigned long long RegularSampler1D::uniformCount(const unsigned level)
    {
        if (level >= 63U) throw std::invalid_argument(
            "In npstat::RegularSampler1D::uniformCount: "
            "level number is out of range");
        return (1ULL << (level + 1U)) - 1ULL;
    }
}
