#include <cmath>
#include <cassert>
#include <stdexcept>

#include "npstat/stat/continuousDegreeTaper.hh"

namespace npstat {
    void continuousDegreeTaper(const double degree, std::vector<double>* taper)
    {
        if (degree < 0.0) throw std::invalid_argument(
            "In npstat::continuousDegreeTaper: "
            "degree argument must be non-negative");
        assert(taper);

        const unsigned long deg = floor(degree);
        const bool isInt = static_cast<double>(deg) == degree;
        const unsigned long sz = isInt ? deg + 1UL : deg + 2UL;

        taper->clear();
        taper->reserve(sz);
        for (unsigned long i=0; i<sz; ++i)
            taper->push_back(1.0);
        if (!isInt)
        {
            const double delta = degree - static_cast<double>(deg);
            (*taper)[sz - 1UL] = sqrt(delta);
        }
    }
}
