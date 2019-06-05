#include <stdexcept>
#include "npstat/rng/EquidistantSampler1D.hh"

namespace npstat {
    EquidistantSampler1D::EquidistantSampler1D(
        const unsigned long long nIntervals,
        const bool increasingOrder)
        : intervalWidth_(1.0L/nIntervals),
          nIntervals_(nIntervals),
          nCalls_(0),
          increasing_(increasingOrder)
    {
        if (!nIntervals) throw std::invalid_argument(
            "In npstat::EquidistantSampler1D constructor: "
            "number of intervals must be positive");
    }

    void EquidistantSampler1D::reset()
    {
        nCalls_ = 0;
    }

    double EquidistantSampler1D::operator()()
    {
        if (nCalls_ >= nIntervals_) throw std::runtime_error(
            "In npstat::EquidistantSampler1D::operator(): "
            "too many calls");
        const double value = ((increasing_ ? nCalls_ : 
                               nIntervals_ - nCalls_ - 1ULL)
                              +0.5L)*intervalWidth_;
        ++nCalls_;
        return value;
    }
}
