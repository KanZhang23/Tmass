#ifndef NPSTAT_MULTISCALEEMUNFOLD1D_HH_
#define NPSTAT_MULTISCALEEMUNFOLD1D_HH_

/*!
// \file MultiscaleEMUnfold1D.hh
//
// \brief Expectation-maximization unfolding with multiscale smoothing
//
// Author: I. Volobouev
//
// May 2014
*/

#include "npstat/nm/EquidistantSequence.hh"

#include "npstat/stat/SmoothedEMUnfold1D.hh"
#include "npstat/stat/MemoizingSymbetaFilterProvider.hh"

namespace npstat {
    /**
    // In the situations with narrow response matrix, this class might
    // be able to speed up convergence of the expectation-maximization
    // iterations (in comparison with SmoothedEMUnfold1D). This is done
    // by performing initial iterations with filters that have wider
    // bandwidth. The maximum bandwidth of such filters should be
    // comparable with the size of typical structures in the unfolded
    // distribution, while the minimum bandwidth should be just slightly
    // above the bandwidth of the final filter. The bandwidth is
    // progressively reduced in the iteration process.
    //
    // Note that construction of additional filters is itself rather
    // CPU-intensive. Once constructed, the filters are memoized. It makes
    // sense to use this code instead of SmoothedEMUnfold1D only if you
    // plan to unfold multiple distributions.
    */
    class MultiscaleEMUnfold1D : public SmoothedEMUnfold1D
    {
    public:
        /**
        // The constructor arguments are:
        //
        // responseMatrix           -- Naturally, the problem response matrix.
        //
        // filter                   -- The filter to use for smoothing the
        //                             unfolded values. This object will not
        //                             make a copy of the filter. It is
        //                             a responsibility of the caller to ensure
        //                             that the argument filter exists while
        //                             this object is in use.
        //
        // symbetaPower             -- Before running the standard expectation
        // maxDegree                   maximization iterations with the filter
        // xMinUnfolded                provided by the previous argument, the
        // xMaxUnfolded                code will run iterations with a family
        // filterBoundaryMethod        of filters created by the
        //                             "symbetaLOrPEFilter1D" function. These
        //                             parameters will be passed to the
        //                             "symbetaLOrPEFilter1D" call.
        //
        // minBandwidth             -- Minimum bandwidth for the extra filters.
        //
        // maxBandwidth             -- Maximum bandwidth for the extra filters.
        //
        // nFilters                 -- Number of extra filters. Logarithms
        //                             of the bandwidth values of these filters
        //                             will be equidistant.
        //
        // itersPerFilter           -- Number of expectation-maximization
        //                             iterations to perform per extra filter.
        //                             These additional filters will be used
        //                             sequentially, in the order of decreasing
        //                             bandwidth.
        //
        // useConvolutions          -- If "true", the code will call the
        //                             "convolve" method of the filter rather
        //                             than its "filter" method.
        //
        // useMultinomialCovariance -- Specifies whether we should use
        //                             multinomial distribution to estimate
        //                             covariance of fitted observations
        //                             (otherwise Poisson assumption is used).
        //
        // smoothLastIter           -- If "false", smoothing will not be
        //                             applied after the last iteration.
        //                             Setting this parameter to "false" is
        //                             not recommended for production results
        //                             because it is unclear how to compare
        //                             such results with models.
        //
        // convergenceEpsilon       -- Convergence criterion parameter for
        //                             various iterations.
        //
        // maxIterations            -- Maximum number of iterations allowed
        //                             (both for the expectation-maximization
        //                             iterations and for the code estimating
        //                             the error propagation matrix).
        */
        MultiscaleEMUnfold1D(const Matrix<double>& responseMatrix,
                             const LocalPolyFilter1D& filter,
                             int symbetaPower, double maxDegree,
                             double xMinUnfolded, double xMaxUnfolded,
                             const BoundaryHandling& filterBoundaryMethod,
                             double minBandwidth, double maxBandwidth,
                             unsigned nFilters, unsigned itersPerFilter,
                             bool useConvolutions,
                             bool useMultinomialCovariance = false,
                             bool smoothLastIter = true,
                             double convergenceEpsilon = 1.0e-10,
                             unsigned maxIterations = 100000U);

        inline virtual ~MultiscaleEMUnfold1D() {}

    private:
        virtual unsigned preIterate(
            const double* observed, unsigned lenObserved,
            double** prev, double** next, unsigned lenUnfolded);

        double maxLOrPEDegree_;
        double binwidth_;
        int symbetaPower_;
        unsigned itersPerFilter_;
        BoundaryHandling filterBoundaryMethod_;
        EquidistantInLogSpace bwset_;
        MemoizingSymbetaFilterProvider filterProvider_;
    };
}

#endif // NPSTAT_MULTISCALEEMUNFOLD1D_HH_
