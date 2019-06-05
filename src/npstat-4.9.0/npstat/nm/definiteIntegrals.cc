#include <cmath>
#include <stdexcept>

#include "npstat/nm/definiteIntegrals.hh"

namespace npstat {
    double definiteIntegral_1(const double a, const double b,
                              const double xmin, const double xmax)
    {
        if (xmin < 0.0 || xmin > 1.0) throw std::invalid_argument(
            "In npstat::definiteIntegral_1: xmin out of range");
        if (xmax < 0.0 || xmax > 1.0) throw std::invalid_argument(
            "In npstat::definiteIntegral_1: xmax out of range");
        if (xmin == xmax)
            return 0.0;
        const double asinmax = asin(sqrt(xmax));
        const double asinmin = asin(sqrt(xmin));
        const double i1 = 2.0*(asinmax - asinmin);
        const double i2 = asinmax - sqrt((1.0 - xmax)*xmax);
        const double i3 = asinmin - sqrt((1.0 - xmin)*xmin);
        return a*(i2 - i3) + b*i1;
    }
}
