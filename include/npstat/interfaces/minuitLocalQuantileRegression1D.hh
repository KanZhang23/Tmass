#ifndef NPSI_MINUITLOCALQUANTILEREGRESSION1D_HH_
#define NPSI_MINUITLOCALQUANTILEREGRESSION1D_HH_

/*!
// \file minuitLocalQuantileRegression1D.hh
//
// \brief Local quantile regression with single predictor
//
// Author: I. Volobouev
//
// April 2011
*/

#include <vector>
#include <utility>

namespace npsi {
    /**
    // High-level driver functions for performing local 1-d quantile
    // regression fits using Minuit2 as a minimization engine
    //
    // The arguments are as follows:
    //
    // inputPoints  -- are the points for which the regression should be
    //                 performed. Predictor is the first member of the pair
    //                 and response is the second. As a side effect of this
    //                 function, the input points will be sorted in the
    //                 increasing order. This is why the vector of input
    //                 points is non-const.
    //
    // symbetaPower -- the power parameter for "SymmetricBeta1D". 3 and 4
    //                 are good values to try.
    //
    // bandwidthInCDFSpace -- Approximate fraction of sample points which
    //                 will participate in each fit. Due to robustness
    //                 requirements (obtaining limited bandwidth in
    //                 coordinate space), the bandwidth in the CDF space
    //                 must be less than 0.5 (and, of course, positive).
    //
    // polyDegree   -- this defines the degree of the polynomial that
    //                 will be fitted to the quantile curve. It does not
    //                 make much sense to go beyond 3 here.
    //
    // cdfValue     -- which quantile to use in the regression
    //
    // xmin, xmax   -- the result will be calculated between xmin and xmax
    //                 in equidistant steps
    //
    // result       -- array where the result will be stored
    //
    // nResultPoints -- number of coordinate points to use to build the
    //                 result. The interval (xmin, xmax) will be split
    //                 into "nResultPoints" bins. The coordinates at which
    //                 the fits are performed are taken from the middle
    //                 of those bins (as in a histogram). Naturally, array
    //                 "result" must have at least "nResultPoints" elements.
    //
    // verbose      -- this switch  can be turned on for debugging purposes
    */
    template<class Numeric1, class Numeric2>
    void minuitLocalQuantileRegression1D(
        std::vector<std::pair<Numeric1,Numeric1> > inputPoints,
        double symbetaPower, double bandwidthInCDFSpace,
        unsigned polyDegree, double cdfValue, double xmin, double xmax,
        Numeric2* result, unsigned nResultPoints, bool verbose=false);
}

#include "npstat/interfaces/minuitLocalQuantileRegression1D.icc"

#endif // NPSI_MINUITLOCALQUANTILEREGRESSION1D_HH_
