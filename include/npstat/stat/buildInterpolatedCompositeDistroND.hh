#ifndef NPSTAT_BUILDINTERPOLATEDCOMPOSITEDISTROND_HH_
#define NPSTAT_BUILDINTERPOLATEDCOMPOSITEDISTROND_HH_

/*!
// \file buildInterpolatedCompositeDistroND.hh
//
// \brief Multivariate density estimation in the regression context
//
// Author: I. Volobouev
//
// June 2015
*/

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/stat/GridInterpolatedDistribution.hh"
#include "npstat/stat/AbsCompositeDistroBuilder.hh"

namespace npstat {
    /*
    // Arguments of this function are as follows:
    //
    // data                  -- The data points. The "Point" class should
    //                          be subscriptable and should publish
    //                          "value_type" typedef (std::array works).
    //
    // dimPredictors         -- These arguments define which dimensions of the
    // nPredictors              data points should be treated as predictors.
    //
    // predictorNames        -- Names of the predictor variables. This argument
    //                          can be 0 in which case the predictor grid axes
    //                          will not have any labels.
    //
    // predictorNumBins      -- Number of bins to use in each predictor
    //                          dimension. These bins will be defined in
    //                          the predictor cdf space.
    //
    // predictorSymbetaPower -- This parameter defines the type of the
    //                          symmetric beta or Gaussian kernel to use
    //                          in the predictor cdf space.
    //
    // effectiveEventsPerBin -- Approximate number of effective events to use
    //                          per predictor bin in the middle of the range.
    //
    // stretchPredKernels    -- Should the kernels be stretched near the
    //                          boundaries in the predictor space?
    //
    // dimResponses          -- These arguments define which dimensions of the
    // nResponseVars            data points should be treated as responses.
    //
    // builder               -- Builder of multivariate distributions using
    //                          data points weighted by kernels in the
    //                          predictor cdf space.
    //
    // interpolateCopulas    -- Initial interpolation method setting for
    //                          GridInterpolatedDistribution. If "true",
    //                          copula densities will be interpolated,
    //                          otherwise the inverse Rosenblatt transformation
    //                          will be interpolated.
    //
    // reportFrequency       -- The code will print a message to the standard
    //                          output periodically, after processing this
    //                          number of bins. Set this parameter to 0
    //                          in order to disable such messages.
    */
    template <class Point>
    CPP11_auto_ptr<GridInterpolatedDistribution>
    buildInterpolatedCompositeDistroND(
        const std::vector<Point>& data,
        const unsigned* dimPredictors, unsigned nPredictors,
        const std::string* predictorNames,
        const unsigned* predictorNumBins, int predictorSymbetaPower,
        double effectiveEventsPerBin, bool stretchPredKernels,
        const unsigned* dimResponses, unsigned nResponseVars,
        const AbsCompositeDistroBuilder<Point>& builder,
        bool interpolateCopulas, unsigned reportFrequency = 0);
}

#include "npstat/stat/buildInterpolatedCompositeDistroND.icc"

#endif // NPSTAT_BUILDINTERPOLATEDCOMPOSITEDISTROND_HH_
