#include <climits>
#include <cassert>
#include <stdexcept>

#include "npstat/nm/rectangleQuadrature.hh"
#include "npstat/nm/GaussLegendreQuadrature.hh"

// The following integration function will be called recursively
static long double loopIntegral1(const npstat::AbsMultivariateFunctor& f,
                                 const double* rectangleCenter,
                                 const double* rectangleSize,
                                 const unsigned integdim,
                                 const unsigned halfpoints,
                                 const long double* abscissae,
                                 const long double* weights,
                                 double* coords,
                                 const unsigned mydim)
{
    const double unit = rectangleSize[mydim]/2.0;
    const double midpoint = rectangleCenter[mydim];
    const bool isLastDim = mydim == integdim - 1U;

    long double sum = 0.0L;
    for (unsigned i=0; i<halfpoints; ++i)
    {
        coords[mydim] = midpoint + unit*abscissae[i];
        long double value = isLastDim ? f(coords, integdim) : 
            loopIntegral1(f, rectangleCenter, rectangleSize, integdim,
                          halfpoints, abscissae, weights, coords, mydim+1U);
        sum += weights[i]*value;
        coords[mydim] = midpoint - unit*abscissae[i];
        value = isLastDim ? f(coords, integdim) : 
            loopIntegral1(f, rectangleCenter, rectangleSize, integdim,
                          halfpoints, abscissae, weights, coords, mydim+1U);
        sum += weights[i]*value;
    }
    return sum*unit;
}

namespace npstat {
    double rectangleIntegralCenterAndSize(const AbsMultivariateFunctor& f,
                                          const double* rectangleCenter,
                                          const double* rectangleSize,
                                          const unsigned dim,
                                          const unsigned integrationPoints)
    {
        // Check that we can deal with "dim"
        const unsigned maxdim = CHAR_BIT*sizeof(unsigned long);
        if (!(dim && dim <= maxdim)) throw std::invalid_argument(
            "In npstat::rectangleIntegralCenterAndSize: "
            "unsupported rectangle dimensionality");

        // Check that we can deal with "integrationPoints"
        const unsigned maxpoints = 256;
        if (!(integrationPoints && integrationPoints <= maxpoints))
            throw std::invalid_argument(
                "In npstat::rectangleIntegralCenterAndSize: "
                "unsupported number of integration points");

        // Check the input pointers
        assert(rectangleCenter);
        assert(rectangleSize);

        GaussLegendreQuadrature quad(integrationPoints);
        assert(quad.npoints() == integrationPoints);

        // Get integration abscissae and weights
        long double abscissae[maxpoints/2], weights[maxpoints/2];
        quad.getAbscissae(abscissae, maxpoints/2);
        quad.getWeights(weights, maxpoints/2);

        // Enter the recursive integration loop
        double coords[maxdim];
        return loopIntegral1(f, rectangleCenter, rectangleSize, dim,
                             integrationPoints/2, abscissae, weights,
                             coords, 0);
    }
}
