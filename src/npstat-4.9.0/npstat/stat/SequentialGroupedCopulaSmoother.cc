#include <stdexcept>

#include "npstat/nm/PtrBufferHandle.hh"

#include "npstat/stat/SequentialGroupedCopulaSmoother.hh"

namespace npstat {
    SequentialGroupedCopulaSmoother::SequentialGroupedCopulaSmoother(
        const unsigned* nBinsInEachDim, const unsigned dim,
        const double marginTolerance, const unsigned nNormCycles,
        const int symbetaPower, const double maxFilterDegree,
        const BoundaryHandling& boundaryMethod,
        const double initialBw, const double* bwCoeffs,
        const GCVCalc* cvCalc, const bool assumeCvCalcOwnership,
        const double cvFactor, const unsigned nCV, const bool useConvolute)
        : GCVCopulaSmoother<SequentialPolyFilterND>(
            nBinsInEachDim, dim, marginTolerance, nNormCycles, initialBw,
            cvCalc, assumeCvCalcOwnership, cvFactor, nCV, useConvolute)
    {
        const std::vector<double>& bwVec = bandwidthValues();
        const unsigned nFilters = bwVec.size();

        // Generate all filters
        for (unsigned ifilt=0; ifilt<nFilters; ++ifilt)
        {
            std::vector<const LocalPolyFilter1D*> f1d(dim, 0);
            Private::PtrBufferHandle<const LocalPolyFilter1D> handle(&f1d[0], dim);
            std::vector<const LocalPolyFilter1D*> f1d2(dim, 0);
            Private::PtrBufferHandle<const LocalPolyFilter1D> handle2(&f1d2[0], dim);

            for (unsigned idim=0; idim<dim; ++idim)
            {
                const unsigned nbins = nBinsInEachDim[idim];
                const double bandwidth = bwCoeffs ? 
                    bwVec[ifilt]*bwCoeffs[idim] : bwVec[ifilt];

                f1d[idim] = symbetaLOrPEFilter1D(
                    symbetaPower, bandwidth, maxFilterDegree,
                    nbins, 0.0, 1.0, boundaryMethod).release();
                f1d2[idim] = symbetaLOrPEFilter1D(
                    symbetaPower, bandwidth, maxFilterDegree,
                    nbins, 0.0, 1.0, boundaryMethod, 0, true).release();
            }

            // Generate the multivariate filter
            CPP11_auto_ptr<Filter> f(new SequentialPolyFilterND(&f1d[0], dim, true));
            handle.release();
            CPP11_auto_ptr<Filter> f2(new SequentialPolyFilterND(&f1d2[0], dim, true));
            handle2.release();

            this->setFilter(ifilt, f.release(), f2.release());
        }
    }
}
