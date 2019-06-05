#ifndef NPSTAT_LOCALQUADRATICLEASTSQUARESND_HH_
#define NPSTAT_LOCALQUADRATICLEASTSQUARESND_HH_

/*!
// \file LocalQuadraticLeastSquaresND.hh
//
// \brief Local least squares fit of a quadratic polynomial to a set
//        of irregularly positioned points
//
// This code can properly take into account response uncertainties.
// The code is very general but it is rather slow in comparison with
// grid-based filters. Internal calculations are performed in double
// precision.
//
// Author: I. Volobouev
//
// October 2010
*/

#include <cassert>
#include <stdexcept>

#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    /**
    // Local quadratic least squares fitter. Template parameter
    // class Point must be subscriptable.
    */
    template<class Point, typename Numeric>
    class LocalQuadraticLeastSquaresND
    {
    public:
        /**
        // The "polyDegree" parameter must be 0, 1, or 2.
        //
        // All values in the "bandwidthValues" array must be positive
        // and their number, "nBandwidthValues", must be equal to
        // the dimensionality of the weight function, weight.dim().
        // The bandwidth values will be copied into an internal buffer.
        //
        // Make sure that the bandwidth values are sufficiently large
        // so that the number of points for which the weight function
        // is positive is always at least as large as the number of
        // polynomial terms fitted. This must be true for every future
        // "fit" call.
        //
        // This object will not copy or own "pointCoords", "values",
        // or "errors" arrays. The number of elements in all of
        // these arrays must be the same and equal to "nPoints". All
        // errors must be positive. The "errors" array can also be NULL
        // in which case all errors are assumed to be equal.
        */
        LocalQuadraticLeastSquaresND(
            unsigned polyDegree,
            const AbsDistributionND& weight,
            const double* bandwidthValues, unsigned nBandwidthValues,
            const Point* pointCoords, unsigned nPoints,
            const Numeric* values, const Numeric* errors = 0);
        ~LocalQuadraticLeastSquaresND();

        //@{
        /** Examine object properties */
        inline unsigned dim() const {return dim_;}
        inline unsigned polyDegree() const {return polyDegree_;}
        inline unsigned nPoints() const {return nPoints_;}
        inline unsigned nTermsFitted() const {return nBasisFcns_;}
        inline double getBandwidth(const unsigned bwNumber) const
        {
            if (bwNumber >= dim_) throw std::out_of_range(
                "In npstat::LocalQuadraticLeastSquaresND::getBandwidth: "
                "bandwidth index out of range");
            return bw_[bwNumber];
        }
        //@}

        /**
        // Change the point coordinates. Note that the number of new
        // coordinates must be exactly equal to the number of points
        // provided in the constructor. The array will not be copied.
        */
        inline void setPointCoords(const Point* newCoords)
            {assert(newCoords); points_ = newCoords;}

        /**
        // Change values for the point locations provided in the constructor.
        // The array must have at least "nPoints()" elements.
        */
        inline void setValues(const Numeric* newValues) 
            {assert(newValues); values_ = newValues;}

        /**
        // Change errors for the point locations provided in the constructor.
        // The array must have at least "nPoints()" elements (or it can be
        // NULL).
        */
        inline void setErrors(const Numeric* newErrors) {errors_ = newErrors;}

        /** Change bandwidth used for weight calculations */
        inline void setBandwidth(const unsigned bwNum, const double newBw)
        {
            if (bwNum >= dim_) throw std::out_of_range(
                "In npstat::LocalQuadraticLeastSquaresND::setBandwidth: "
                "bandwidth index out of range");
            if (newBw <= 0.0) throw std::invalid_argument(
                "In npstat::LocalQuadraticLeastSquaresND::setBandwidth: "
                "bandwidth must be positive");
            bw_[bwNum] = newBw;
        }

        /**
        // Perfom local quadratic polynomial fit with weight function
        // centered at "coordinate". In additional to the fitted value,
        // one can get the gradient and the Hessian at the fitted point,
        // as estimated by the fit.
        */
        template <typename Num2>
        double fit(const Num2* coordinate, unsigned coordinateDim,
                   double* gradient=0, unsigned lenGradient=0,
                   double* hessian=0, unsigned lenHessian=0) const;
    private:
        LocalQuadraticLeastSquaresND();
        LocalQuadraticLeastSquaresND(const LocalQuadraticLeastSquaresND&);
        LocalQuadraticLeastSquaresND& operator=(
            const LocalQuadraticLeastSquaresND&);

        double basisPoly(unsigned num, const double* coords) const;

        const unsigned polyDegree_;
        const unsigned nPoints_;
        const unsigned dim_;
        unsigned nBasisFcns_;
        const AbsDistributionND* weight_;
        const Point* points_;
        const Numeric* values_;
        const Numeric* errors_;
        const Numeric zero_;

        std::vector<double> bwBuf_;
        const double* bw_;
        double* delta_;

        std::vector<double> memBuf_;
        double* A_;
        double* b_;
        double* singularValues_;

        mutable std::vector<double> workBuf_;
        mutable std::vector<int> intBuf_;
    };
}

#include "npstat/stat/LocalQuadraticLeastSquaresND.icc"

#endif // NPSTAT_LOCALQUADRATICLEASTSQUARESND_HH_
