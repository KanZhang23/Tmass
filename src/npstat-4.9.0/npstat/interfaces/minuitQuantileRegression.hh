#ifndef NPSI_MINUITQUANTILEREGRESSION_HH_
#define NPSI_MINUITQUANTILEREGRESSION_HH_

/*!
// \file minuitQuantileRegression.hh
//
// \brief Local quantile regression with multiple predictors
//
// Author: I. Volobouev
//
// October 2011
*/

#include "npstat/stat/LocalQuantileRegression.hh"

namespace npsi {
    /**
    // High-level driver function for performing local quantile regression
    // fits using Minuit2 as a minimization engine. The weight function is
    // assumed to be symmetric in each dimension.
    //
    // Function arguments are as follows:
    //
    //   qrb                 -- Naturally, an instance of the
    //                          npstat::QuantileRegressionBase template.
    //                          Carries the information about the dataset,
    //                          the kernel, the bandwidth, and the quantile
    //                          to fit for. For more details, look at the
    //                          LocalQuantileRegression.hh header.
    //
    //   polyDegree          -- Degree of the local polynomial to fit.
    //                          Can be 0, 1 (local linear regression),
    //                          or 2 (local quadratic regression).
    //
    //   result              -- Grid which will hold the results on exit.
    //                          It defines the number of points in each
    //                          dimension and provides the storage space.
    //
    //   resultBox           -- Coordinates of the grid boundaries. The
    //                          points for which the regression is performed
    //                          will be positioned inside this box just like
    //                          histogram bin centers.
    //
    //   reportProgressEvery -- Print out a message about the number of
    //                          grid points processed to the standard output
    //                          every "reportProgressEvery" points. The
    //                          default value of 0 means that such printouts
    //                          are disabled.
    //
    //   upFactor            -- A factor for the Minuit UP parameter, to
    //                          multiply by the value estimated internally.
    //                          Don't change the default unless you really
    //                          understand what you are doing.
    //
    // For this function, it is assumed that the constant bandwidth is 
    // set up already, with the weight function which was used to create
    // the orthogonal polynomials.
    */
    template <typename Numeric,
              typename Num2, unsigned StackLen2, unsigned StackDim2>
    void minuitQuantileRegression(
        npstat::QuantileRegressionBase<Numeric>& qrb, unsigned polyDegree,
        npstat::ArrayND<Num2,StackLen2,StackDim2>* result,
        const npstat::BoxND<Numeric>& resultBox,
        unsigned reportProgressEvery = 0, double upFactor = 1.0);

    /**
    // High-level driver function for performing local quantile regression
    // fits using Minuit2 as a minimization engine. The weight function is
    // assumed to be symmetric in each dimension.
    //
    // This function is similar to minuitQuantileRegression. However,
    // it sometimes automatically increases the bandwidth: it makes sure
    // that the regression box has at least the minimal fraction of
    // points inside it, as specified by the "minimalSampleFraction"
    // parameter. The fraction is calculated from the "predictorHisto"
    // histogram whose dimensionality and axis order should coincide
    // with the regression predictors. It is expected that this histogram
    // will contain the predictor variables for the sample actually used
    // in the regression.
    //
    // "minimalSampleFraction" must be <= 1.0. 0 or negative values
    // will result in the constant bandwidth use, just like in the
    // minuitQuantileRegression function.
    */
    template 
    <
        typename Numeric,
        typename Num2, unsigned StackLen2, unsigned StackDim2,
        typename NumHisto
    >
    void minuitQuantileRegressionIncrBW(
        npstat::QuantileRegressionBase<Numeric>& qrb, unsigned polyDegree,
        npstat::ArrayND<Num2,StackLen2,StackDim2>* result,
        const npstat::BoxND<Numeric>& resultBox,
        const npstat::HistoND<NumHisto>& predictorHisto,
        double minimalSampleFraction,
        unsigned reportProgressEvery = 0, double upFactor = 1.0);
}

#include "npstat/interfaces/minuitQuantileRegression.icc"

#endif // NPSI_MINUITQUANTILEREGRESSION_HH_
