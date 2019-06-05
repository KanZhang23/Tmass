#include <cmath>
#include <cassert>
#include <stdexcept>

#include "npstat/rng/convertToSphericalRandom.hh"
#include "npstat/nm/SpecialFunctions.hh"

namespace npstat {
    double convertToSphericalRandom(const double* rnd,
                                    const unsigned dim,
                                    double* direction,
                                    const bool getRadialRandom)
    {
        if (!dim) throw std::invalid_argument(
            "In npstat::convertToSphericalRandom: "
            "dimensionality must be positive");
        assert(rnd);
        assert(direction);

        bool onTheBoundary = false;
        for (unsigned i=0; i<dim; ++i)
        {
            const double r = rnd[i];
            if (r < 0.0 || r > 1.0) throw std::invalid_argument(
                "In npstat::convertToSphericalRandom: "
                "input out of [0, 1] range");
            if (r == 0.0 || r == 1.0)
                onTheBoundary = true;
        }

        double rrnd = -1.0;

        if (dim == 1U)
        {
            if (rnd[0] > 0.5)
                *direction = 1.0;
            else
                *direction = -1.0;
            if (getRadialRandom)
                rrnd = 2.0*fabs(rnd[0] - 0.5);
        }
        else
        {
            double sumsq = 0.0;
            for (unsigned i=0; i<dim; ++i)
            {
                const double x = inverseGaussCdf(rnd[i]);
                direction[i] = x;
                sumsq += x*x;
            }
            const double len = sqrt(sumsq);
            if (len)
            {
                for (unsigned i=0; i<dim; ++i)
                    direction[i] /= len;
            }
            else
            {
                direction[0] = 1.0;
                for (unsigned i=1U; i<dim; ++i)
                    direction[i] = 0.0;
            }
            if (getRadialRandom)
            {
                if (onTheBoundary)
                    rrnd = 1.0;
                else if (dim == 2U)
                    rrnd = 1.0 - exp(-sumsq/2.0);
                else
                    rrnd = incompleteGamma(dim/2.0, sumsq/2.0);
            }
        }
        return rrnd;
    }
}
