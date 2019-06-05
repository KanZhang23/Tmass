#include <cmath>
#include <stdexcept>

#include "npstat/nm/PtrBufferHandle.hh"

#include "npstat/stat/BernsteinCopulaSmoother.hh"
#include "npstat/stat/BernsteinFilter1DBuilder.hh"

static std::vector<double> effectiveBernsteinBwValues(
    const npstat::Matrix<unsigned>& polyDegreesToUse)
{
    const unsigned nFilters = polyDegreesToUse.nRows();
    const unsigned dim = polyDegreesToUse.nColumns();
    std::vector<double> bwvec(nFilters);
    for (unsigned ifilt=0; ifilt<nFilters; ++ifilt)
    {
        long double h = 1.0L;
        for (unsigned idim=0; idim<dim; ++idim)
            h *= 1.0/(polyDegreesToUse(ifilt, idim) + 1U);
        bwvec[ifilt] = powl(h, 1.0/dim);
    }
    return bwvec;
}

namespace npstat {
    BernsteinCopulaSmoother::BernsteinCopulaSmoother(
        const unsigned* nBinsInEachDim, const unsigned dim,
        const double marginTolerance, const unsigned nNormCycles,
        const CVCalc* cvCalc, const bool assumeCvCalcOwnership,
        const Matrix<unsigned>& polyDegreesToUse)
        : CVCopulaSmoother<SequentialPolyFilterND>(
            nBinsInEachDim, dim, marginTolerance, nNormCycles,
            effectiveBernsteinBwValues(polyDegreesToUse),
            cvCalc, assumeCvCalcOwnership, true)
    {
        const unsigned nFilters = polyDegreesToUse.nRows();
        if (nFilters)
        {
            if (dim != polyDegreesToUse.nColumns()) throw std::invalid_argument(
                "In npstat::BernsteinCopulaSmoother constructor: "
                "incompatible matrix of polynomial degrees");

            // Generate all filters
            for (unsigned ifilt=0; ifilt<nFilters; ++ifilt)
            {
                std::vector<const LocalPolyFilter1D*> f1d(dim, 0);
                Private::PtrBufferHandle<const LocalPolyFilter1D> handle(&f1d[0], dim);

                for (unsigned idim=0; idim<dim; ++idim)
                {
                    const unsigned nbins = nBinsInEachDim[idim];
                    const unsigned deg = polyDegreesToUse(ifilt, idim);
                    if (deg+1U > nbins) throw std::invalid_argument(
                        "In npstat::BernsteinCopulaSmoother constructor: "
                        "polynomial degree is too large");
                    BernsteinFilter1DBuilder builder(deg, nbins, deg+1U == nbins);
                    f1d[idim] = new LocalPolyFilter1D(0, 0, builder, nbins);
                }

                // Generate the filter
                Filter* f = new SequentialPolyFilterND(&f1d[0], dim, true);
                handle.release();

                this->setFilter(ifilt, f);
            }
        }
    }
}
