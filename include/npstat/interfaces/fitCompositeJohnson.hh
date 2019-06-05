#ifndef NPSI_FITCOMPOSITEJOHNSON_HH_
#define NPSI_FITCOMPOSITEJOHNSON_HH_

/*!
// \file fitCompositeJohnson.hh
//
// \brief Nonparametric density estimation utilizing Johnson transformation
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/LocalPolyFilter1D.hh"

namespace npsi {
    /**
    // Density estimation by the transformation method using the following
    // sequence of steps:
    //
    // 1. Johnson system is fitted to the input sample between quantiles
    //    that correspond to parameters "qmin" and "qmax". Typical values
    //    of these parameters are 0.05 and 0.95.
    //
    // 2. The sample is transformed according to the cumulative distribution
    //    of the fitted Johnson system.
    //
    // 3. The transformed sample is smoothed with a bunch of filters with
    //    different bandwidth values. The best filter (bandwidh) is then
    //    chosen using pseudo-likelihood cross-vaidation.
    //
    // 4. BinnedCompositeJohnson density is made using the results of these
    //    fits. This density is scanned into the "smoothedCurve" array.
    //
    // Function arguments are as follows:
    //
    // input, nInput           -- Array of input data points (typically
    //                            floats or doubles) and the number of
    //                            points in this array.
    //
    // nBins                   -- Number of bins for the histogram which
    //                            will be used for fitting parameters of
    //                            the Johnson system.
    //
    // xmin, xmax              -- Range (support) of the estimated density.
    //
    // qmin, qmax, minlog      -- Parameters passed to the JohnsonFit class.
    //
    // filters, nFilters       -- A collection of smoothers to try on the
    //                            transformed density. All of them will be
    //                            used and the smoother with the best
    //                            cross-validation pseudo-likelihood will
    //                            be chosen to build the final result.
    //
    // smoothedCurve, lenCurve -- The array in which the smoothed values
    //                            will be stored. The coordinates correspond
    //                            to the bin centers of a histogram with
    //                            "lenCurve" bins between "xmin" and "xmax".
    //
    // intitialFitConverged    -- Can be used to find out whether the initial
    //                            Johnson system fit converged successfully.
    //                            This parameter can also be NULL.
    //
    // filterUsed              -- On output, will contain the number of the
    //                            best filter from "filters" (or can be NULL).
    */
    template<typename InputData, typename OutputData>
    void fitCompositeJohnson(
        const InputData* input, unsigned long nInput,
        unsigned nBins, double xmin, double xmax,
        double qmin, double qmax, double minlog,
        const npstat::LocalPolyFilter1D* const* filters, unsigned nFilters,
        OutputData* smoothedCurve, unsigned lenCurve,
        bool* intitialFitConverged, unsigned* filterUsed);
}

#include "npstat/interfaces/fitCompositeJohnson.icc"

#endif // NPSI_FITCOMPOSITEJOHNSON_HH_
