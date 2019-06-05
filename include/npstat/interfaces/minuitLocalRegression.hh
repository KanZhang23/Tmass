#ifndef NPSI_MINUITLOCALREGRESSION_HH_
#define NPSI_MINUITLOCALREGRESSION_HH_

/*!
// \file minuitLocalRegression.hh
//
// \brief Local logistic regression using Minuit as a minimization engine
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/stat/LocalLogisticRegression.hh"

namespace npsi {
    //@{
    /**
    // High-level driver function for performing local logistic regression
    // fits using Minuit2 as a minimization engine. It is assumed that the
    // constant bandwidth is set up already, with the weight function which
    // was used to create the orthogonal polynomials. The weight function is
    // assumed to be symmetric in each dimension.
    */
    template <class Point, class Numeric, class BooleanFunctor,
              typename Num2, unsigned StackLen2, unsigned StackDim2>
    void minuitUnbinnedLogisticRegression(
        npstat::LogisticRegressionOnKDTree<Point,Numeric,BooleanFunctor>& reg,
        const unsigned maxdeg, npstat::ArrayND<Num2,StackLen2,StackDim2>* result,
        const npstat::BoxND<Numeric>& resultBox, unsigned reportProgressEvery = 0);


    template <typename Numeric, unsigned StackLen, unsigned StackDim,
              typename Num2, unsigned StackLen2, unsigned StackDim2>
    void minuitLogisticRegressionOnGrid(
        npstat::LogisticRegressionOnGrid<Numeric,StackLen,StackDim>& reg,
        const unsigned maxdeg, npstat::ArrayND<Num2,StackLen2,StackDim2>* result,
        unsigned reportProgressEvery = 0);
    //@}
}

#include "npstat/interfaces/minuitLocalRegression.icc"

#endif // NPSI_MINUITLOCALREGRESSION_HH_
