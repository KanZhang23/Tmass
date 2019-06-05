#ifndef NPSTAT_GRIDDEDROBUSTREGRESSION_HH_
#define NPSTAT_GRIDDEDROBUSTREGRESSION_HH_

/*!
// \file griddedRobustRegression.hh
//
// \brief A generic framework for local robust regression on regular grids
//
// Author: I. Volobouev
//
// December 2011
*/

#include <utility>

#include "npstat/stat/AbsLossCalculator.hh"
#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /**
    // A generic framework for local robust regression on regular grids.
    // It is designed, in particular, in order to implement local least trimmed
    // squares (LLTS) but can be used with other loss types as well. Works by
    // replacing (i.e., adjusting) grid values with the highest local loss
    // in the whole grid by values fitted from local surroundings using,
    // for example, local orthogonal polynomials.
    //
    // A two-run use is envisioned: during the first run, the user accumulates
    // the "history" of grid replacements. Then the history is analyzed
    // (typically, the local loss time series will exhibit a characteristic
    // "knee" when all outliers have been detected and adjusted). During the
    // second run, the regression is performed up to the desired history point.
    // Automatic history analyzers might be added in the future, but for now
    // it is best to just eyeball the history with the help of some convenient
    // ploting package.
    //
    // Compared to other statistical methods in this package, this function is
    // expected to be very slow (which is not atypical for robust techniques).
    //
    // Function template arguments are as follows. These parameters should
    // be the same as those in the provided loss calculator.
    //
    // MaxDim              -- Maximum dimensionality of the data grid.
    //                        The actual dimensionality must not exceed
    //                        this parameter.
    //
    // MaxReplace          -- Maximum number of grid point adjustments
    //                        performed in a single local adjustment cycle.
    //
    // Function arguments are as follows:
    //
    // in                  -- Input data for regression.
    //
    // slidingWindowSize   -- Size of the local window for use by the loss
    //                        calculator. The size in each dimension must
    //                        be odd and larger than 1.
    //
    // slidingWindowDim    -- Number of elements in the slidingWindowSize array
    //                        (and dimensionality of the predictor space).
    //                        Naturally, the code will check that the input data
    //                        dimensionality is compatible with this parameter.
    //
    // lossCalc            -- Local loss calculator. Will be called on data
    //                        in all possible local windows. Should calculate
    //                        local least trimmed squares or some other such
    //                        quantity. See "WeightedLTSLoss.hh" and
    //                        "TwoPointsLTSLoss.hh" for examples of such
    //                        calculators.
    //
    // stopCallback        -- The function stops when this returns "true".
    //                        The arguments will be the largest remaining
    //                        loss and the number of replacements made
    //                        so far. See the "GriddedRobustRegressionStop.hh"
    //                        header for a simple example of such a callback.
    //
    // observationCallback -- Callback to call with the current values
    //                        of data (to monitor progress). Can be NULL.
    //
    // observationFrequency -- How often to call the above callback. Set this
    //                        argument to 0 to disable these calls completely.
    //
    // out                 -- Replaced array (on exit). Can point to the
    //                        same area as "in" (naturally, "in" will be
    //                        destroyed in this case).
    //
    // replacementHistory  -- History of replacements. This argument can be
    //                        NULL in which case the history is not generated.
    //                        This function will not clear a previously filled
    //                        history but will append to it.
    //
    // verbose             -- If "true", will print the location of the local
    //                        window in which the replacements are made to
    //                        the standard output.
    //
    // This function returns the total number of replacements made.
    */
    template <unsigned MaxDim, unsigned MaxReplace>
    unsigned long griddedRobustRegression(
        const ArrayND<double>& in,
        const unsigned *slidingWindowSize, unsigned slidingWindowDim,
        const AbsLossCalculator<MaxDim,MaxReplace>& lossCalc,
        const Functor2<bool,LocalLoss,unsigned long>& stopCallback,
        const Functor1<void,ArrayND<double> >* observationCallback,
        unsigned observationFrequency,
        ArrayND<double>* out,
        std::vector<
           std::pair<LocalLoss,ReplacementBlock<MaxDim,MaxReplace> >
        >* replacementHistory, bool verbose = false);
}

#include "npstat/stat/griddedRobustRegression.icc"

#endif // NPSTAT_GRIDDEDROBUSTREGRESSION_HH_
