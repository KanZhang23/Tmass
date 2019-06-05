#include <cassert>
#include "npstat/stat/ProductSymmetricBetaNDCdf.hh"

namespace npstat {
    ProductSymmetricBetaNDCdf::ProductSymmetricBetaNDCdf(
        const double* location, const double* scale,
        const double* power, const int* direction,
        const unsigned dim)
        : directions_(dim),
          dim_(dim)
    {
        if (dim)
        {
            assert(location);
            assert(scale);
            assert(power);
            assert(direction);

            marginals_.reserve(dim);
            for (unsigned i=0; i<dim; ++i)
            {
                directions_[i] = direction[i];
                if (direction[i])
                    marginals_.push_back(SymmetricBeta1D(
                                             location[i], scale[i], power[i]));
                else
                    marginals_.push_back(SymmetricBeta1D(0.0, 1.0, 0.0));
            }
        }
    }

    double ProductSymmetricBetaNDCdf::operator()(const double* point,
                                                 const unsigned dim) const
    {
        if (dim)
            assert(point);
        if (dim != dim_)
            throw std::invalid_argument(
                "In npstat::ProductSymmetricBetaNDCdf::operator(): "
                "incompatible point dimensionality");
        double result = 1.0;
        for (unsigned i=0; i<dim; ++i)
        {
            const int dir = directions_[i];
            if (dir)
            {
                if (dir > 0)
                    result *= marginals_[i].exceedance(point[i]);
                else
                    result *= marginals_[i].cdf(point[i]);
            }
        }
        return result;
    }
}
