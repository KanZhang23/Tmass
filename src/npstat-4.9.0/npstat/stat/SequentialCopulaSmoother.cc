#include <stdexcept>

#include "npstat/nm/PtrBufferHandle.hh"

#include "npstat/stat/SequentialCopulaSmoother.hh"

namespace npstat {
    SequentialCopulaSmoother::SequentialCopulaSmoother(
        const unsigned* nBinsInEachDim, const unsigned dim,
        const double marginTolerance, const unsigned nNormCycles,
        const int symbetaPower, const double maxFilterDegree,
        const BoundaryHandling& boundaryMethod,
        const double initialBw, const double* bwCoeffs,
        const CVCalc* cvCalc, const bool assumeCvCalcOwnership,
        const double cvFactor, const unsigned nCV, const bool useConvolute,
        const bool doublyStochastic)
        : CVCopulaSmoother<SequentialPolyFilterND>(
            nBinsInEachDim, dim, marginTolerance, nNormCycles, initialBw,
            cvCalc, assumeCvCalcOwnership, cvFactor, nCV, useConvolute)
    {
        static const double doublyStochasticTolerance = 1.0e-8;
        static const unsigned doublyStochasticMaxIter = 1000000;

        const std::vector<double>& bwVec = bandwidthValues();
        const unsigned nFilters = bwVec.size();

        // Generate all filters
        for (unsigned ifilt=0; ifilt<nFilters; ++ifilt)
        {
            std::vector<const LocalPolyFilter1D*> f1d(dim, 0);
            Private::PtrBufferHandle<const LocalPolyFilter1D> handle(&f1d[0], dim);

            for (unsigned idim=0; idim<dim; ++idim)
            {
                const unsigned nbins = nBinsInEachDim[idim];
                const double bandwidth = bwCoeffs ? 
                    bwVec[ifilt]*bwCoeffs[idim] : bwVec[ifilt];

                f1d[idim] = symbetaLOrPEFilter1D(
                    symbetaPower, bandwidth, maxFilterDegree,
                    nbins, 0.0, 1.0, boundaryMethod).release();
                if (doublyStochastic)
                {
                    CPP11_auto_ptr<LocalPolyFilter1D> ptr = 
                        f1d[idim]->doublyStochasticFilter(
                            doublyStochasticTolerance,
                            doublyStochasticMaxIter);
                    delete f1d[idim];
                    f1d[idim] = ptr.release();
                    if (!f1d[idim])
                        throw std::runtime_error(
                            "In npstat::SequentialCopulaSmoother constructor:"
                            " failed to construct doubly stochastic filter");
                }
            }

            // Generate the multivariate filter
            Filter* f = new SequentialPolyFilterND(&f1d[0], dim, true);
            handle.release();
            this->setFilter(ifilt, f);
        }
    }
}
