#ifndef NPSTAT_QUANTILEREGRESSION1D_HH_
#define NPSTAT_QUANTILEREGRESSION1D_HH_

/*!
// \file QuantileRegression1D.hh
//
// \brief Local or global quantile regression with single predictor
//
// Author: I. Volobouev
//
// April 2011
*/

namespace npstat {
    /**
    // This functor simply evaluates the function to be minimized;
    // the minimization itself is up to some external optimization
    // engine. The modeling of the regression curve is performed
    // by either Legendre or Gegenbauer polynomials. The former
    // is appropriate for global models, while the latter is useful for
    // local models in which the weights are assigned by a symmetric
    // beta function.
    //
    // There is a high-level driver function for this class which uses
    // Minuit as the optimization engine: npsi::minuitLocalQuantileRegression1D
    */
    template <typename Pair, typename Wnum=double>
    class QuantileRegression1D
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //   points     -- The data to regress. Predictor is the first
        //                 member of the pair and response is the second.
        //
        //   weights    -- Localizing weights. Can be NULL in which case
        //                 it is assumed that each weight is 1.0 (this is
        //                 appropriate for global regression).
        //
        //   nPoints    -- Number of elements in the "points" array and,
        //                 if provided, in the "weights" array as well.
        //
        //   location   -- Can be used to shift all points.
        //
        //   scale      -- Can be used to scale distances between all points.
        //                 The coordinates at which the regression polynomials
        //                 will be evaluated will be transformed according to
        //                 x_poly = (points[i].first - location)/scale.
        //
        //   quantile   -- The target quantile.
        //
        //   polyDegree -- Maximum polynomial degree in the fit.
        //
        //   useGegenbauerPolys -- Set to "true" to use Gegenbauer polynomials
        //                 (appropriate for local fit), and to "false" to use
        //                 Legendre polynomials (appropriate for global
        //                 regression).
        //
        //   gegenbauerLambda -- set this to 0.5 + "power" parameter of the
        //                  SymmetricBeta1D if you are using Gegenbauer
        //                  polynomials for regression. This parameter is
        //                  ignored in case Legendre polynomials are used.
        //
        // This class will not make an internal copy and will not own
        // the provided data and weights.
        */
        QuantileRegression1D(const Pair* points,
                             const Wnum* weights, unsigned long nPoints,
                             double location, double scale, double quantile,
                             unsigned polyDegree, bool useGegenbauerPolys,
                             double gegenbauerLambda);

        /**
        // This operator takes the polynomial coefficients and their number
        // as arguments (the number of the coefficients must be equal to
        // polyDegree + 1). 0th degree coefficient comes first.
        */
        template <typename Numeric2>
        double operator()(const Numeric2 *coeffs, unsigned lenCoeffs) const;

    private:
        QuantileRegression1D();
        QuantileRegression1D(const QuantileRegression1D&);
        QuantileRegression1D& operator=(const QuantileRegression1D&);

        const Pair* points_;
        const Wnum* weights_;
        const unsigned long nPoints_;
        const long double location_;
        const long double scale_;
        const long double quantile_;
        const long double onemq_;
        const long double lambda_;
        const unsigned degree_;
        const bool useGegen_;
    };
}

#include "npstat/stat/QuantileRegression1D.icc"

#endif // NPSTAT_QUANTILEREGRESSION1D_HH_
