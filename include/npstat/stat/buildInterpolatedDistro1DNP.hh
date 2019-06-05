#ifndef NPSTAT_BUILDINTERPOLATEDDISTRO1DNP_HH_
#define NPSTAT_BUILDINTERPOLATEDDISTRO1DNP_HH_

/*!
// \file buildInterpolatedDistro1DNP.hh
//
// \brief Univariate density estimation in the regression context
//
// Author: I. Volobouev
//
// June 2015
*/

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/stat/InterpolatedDistro1DNP.hh"
#include "npstat/stat/AbsDistro1DBuilder.hh"

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
    // responseDimToUse      -- Which dimension of the data points should be
    //                          treated as the response variable.
    //
    // builder               -- Builder of univariate distributions using
    //                          data points weighted by kernels in the
    //                          predictor cdf space.
    //
    // interpolateVertically -- Initial interpolation method setting for
    //                          InterpolatedDistro1DNP. If "true", densities
    //                          will be interpolated linearly, otherwise
    //                          quantile functions will be interpolated
    //                          linearly.
    //
    // reportFrequency       -- The code will print a message to the standard
    //                          output periodically, after processing this
    //                          number of bins. Set this parameter to 0
    //                          in order to disable such messages.
    */
    template <class Point>
    CPP11_auto_ptr<InterpolatedDistro1DNP>
    buildInterpolatedDistro1DNP(
        const std::vector<Point>& data,
        const unsigned* dimPredictors, unsigned nPredictors,
        const std::string* predictorNames,
        const unsigned* predictorNumBins, int predictorSymbetaPower,
        double effectiveEventsPerBin, bool stretchPredKernels,
        unsigned responseDimToUse,
        const AbsDistro1DBuilder<Point>& builder,
        bool interpolateVertically = false, unsigned reportFrequency = 0);
}

#include "npstat/stat/buildInterpolatedDistro1DNP.icc"

#endif // NPSTAT_BUILDINTERPOLATEDDISTRO1DNP_HH_
