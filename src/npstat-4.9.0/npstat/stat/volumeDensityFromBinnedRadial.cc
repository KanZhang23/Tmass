#include <cmath>
#include <cassert>

#include "npstat/nm/MathUtils.hh"
#include "npstat/stat/volumeDensityFromBinnedRadial.hh"

namespace npstat {
    double volumeDensityFromBinnedRadial(const unsigned dim,
                                         const double binWidth,
                                         const double r,
                                         const double radialDensity)
    {
        double dens = radialDensity;
        switch (dim)
        {
        case 0U:
            break;

        case 1U:
            dens /= 2.0;
            break;

        default:
            assert(binWidth >= 0.0);
            assert(r >= 0.0);
            if (binWidth > 0.0)
            {
                const double rMin = floor(r/binWidth)*binWidth;
                const double vmin = pow(rMin, dim);
                const double vmax = pow(rMin + binWidth, dim);
                const double effArea = ndUnitSphereVolume(dim)*(vmax - vmin)/binWidth;
                dens /= effArea;
            }
            else
            {
                assert(r > 0.0);
                const double effArea = ndUnitSphereArea(dim)*pow(r, dim-1U);
                dens /= effArea;
            }
            break;
        }
        return dens;
    }
}
