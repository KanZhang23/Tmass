#include <stdexcept>

#include "npstat/stat/SymbetaParams1D.hh"

namespace npstat {
    SymbetaParams1D::SymbetaParams1D(const int i_symbetaPower,
                                     const double i_maxDegree,
                                     const double i_binWidth,
                                     const BoundaryHandling& bm)
        : maxDegree_(i_maxDegree), binWidth_(i_binWidth),
          symbetaPower_(i_symbetaPower), bm_(bm)
    {
        if (maxDegree_ < 0.0) throw std::invalid_argument(
            "In npstat::SymbetaParams1D constructor: "
            "maximum degree argument can not be negative");
        if (binWidth_ <= 0.0) throw std::invalid_argument(
            "In npstat::SymbetaParams1D constructor: "
            "bin width argument must be positive");
    }
}
