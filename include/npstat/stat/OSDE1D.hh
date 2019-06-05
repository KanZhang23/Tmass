#ifndef NPSTAT_OSDE1D_HH_
#define NPSTAT_OSDE1D_HH_

/*!
// \file OSDE1D.hh
//
// \brief Orthogonal Series Density Estimation (OSDE) in one dimension
//
// Author: I. Volobouev
//
// May 2017
*/

#include <utility>

#include "npstat/nm/AbsClassicalOrthoPoly1D.hh"

namespace npstat {
    class OSDE1D
    {
    public:
        /**
        // Main constructor. The arguments are as follows:
        //
        //  poly1d -- classical orthogonal polynomial system to use
        //
        //  xmin   -- lower bound of the density support region
        //
        //  xmax   -- upper bound of the density support region
        */
        OSDE1D(const AbsClassicalOrthoPoly1D& poly1d,
               double xmin, double xmax);

        OSDE1D(const OSDE1D&);
        OSDE1D& operator=(const OSDE1D&);

        ~OSDE1D();

        //@{
        /** Basic inspection of object properties */
        inline const AbsClassicalOrthoPoly1D& clpoly() const {return *poly_;}
        inline double xmin() const {return xmin_;}
        inline double xmax() const {return xmax_;}
        double weight(double x) const;
        //@}

        /** Polynomial values */
        inline double poly(const unsigned deg, const double x) const
            {return poly_->poly(deg, (x - shift_)/scale_);}

        /** Polynomial series representing the fitted density */
        double series(const double* coeffs, unsigned maxdeg, double x) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the given function. The length of the array "coeffs"
        // should be at least maxdeg + 1. Note that the coefficients
        // are returned in the order of increasing degree (same order
        // is used by the "series" function).
        */
        template <class Functor, class Quadrature>
        void calculateCoeffs(const Functor& fcn, const Quadrature& quad,
                             double* coeffs, unsigned maxdeg) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the given sample of points (empirical density function).
        // The length of the array "coeffs" should be at least maxdeg + 1.
        // Note that the coefficients are returned in the order of
        // increasing degree (same order is used by the "series" function).
        */
        template <class Numeric>
        void sampleCoeffs(const Numeric* coords, unsigned long lenCoords,
                          double* coeffs, unsigned maxdeg) const;

        /*
        // Estimate variances of the coefficients obtained previously
        // with the "sampleCoeffs" method.
        */
        template <class Numeric>
        void sampleCoeffVars(const Numeric* coords, unsigned long lenCoords,
                             const double* coeffs, unsigned maxdeg,
                             double* variances) const;

        /**
        // Build the coefficients of the orthogonal polynomial series for
        // the given sample of weighted points (empirical density function).
        // The first element of the pair will be treated as the coordinate
        // and the second element will be treated as weight. Weights must
        // not be negative. The length of the array "coeffs" should be
        // at least maxdeg + 1. Note that the coefficients are returned
        // in the order of increasing degree (same order is used by the
        // "series" function).
        */
        template <class Pair>
        void weightedSampleCoeffs(const Pair* points, unsigned long numPoints,
                                  double* coeffs, unsigned maxdeg) const;

        /*
        // Estimate variances of the coefficients obtained previously
        // with the "weightedSampleCoeffs" method. This code assumes that
        // weights and coordinates are statistically independent from
        // each other.
        */
        template <class Pair>
        void weightedSampleCoeffVars(const Pair* points, unsigned long nPoints,
                                     const double* coeffs, unsigned maxdeg,
                                     double* variances) const;

        /**
        // Integrated squared error between the given function and the
        // polynomial series
        */
        template <class Functor, class Quadrature>
        double integratedSquaredError(const Functor& fcn,
                                      const Quadrature& quad,
                                      const double* coeffs,
                                      unsigned maxdeg) const;

        /**
        // Integrated squared error between the given function and the
        // polynomial series on an arbitrary interval. If the given
        // interval has regions beyond xmin() and xmax(), the integration
        // will assume that on those regions the value of the series is 0,
        // and the quadrature will be carried out there separately.
        */
        template <class Functor, class Quadrature>
        double integratedSquaredError(const Functor& fcn,
                                      const Quadrature& quad,
                                      const double* coeffs,
                                      unsigned maxdeg,
                                      double xmin, double xmax) const;

        /**
        // A helper function for choosing the density support interval if
        // it is unknown and has to be estimated from the sample. If the
        // limit is not from the sample (as indicated by the corresponding
        // boolean argument), it is left unchanged. The first element of
        // the returned pair will correspond to the lower limit of the
        // interval and the second element to the upper.
        */
        static std::pair<double,double> supportRegion(
            const AbsClassicalOrthoPoly1D& poly1d,
            double leftLimit, bool leftIsFromSample,
            double rightLimit, bool rightIsFromSample,
            unsigned long nPoints);

        /**
        // Optimal OSDE truncation degree according to the Hart scheme.
        // Minimizes Sum_{i=0}^M (k*variances[i] - coeffs[i]^2) over M.
        // The default value of k = 2 corresponds to the original scheme.
        // Argument arrays "coeffs" and "variances" should have at least
        // maxdeg + 1 elements and should be calculated in advance
        // with "sampleCoeffs"/"sampleCoeffVars" or with
        // "weightedSampleCoeffs"/"weightedSampleCoeffVars" methods.
        */
        static unsigned optimalDegreeHart(const double* coeffs,
                                          const double* variances,
                                          unsigned maxdeg, double k = 2.0);

    private:
        const AbsClassicalOrthoPoly1D* poly_;
        double xmin_;
        double xmax_;
        double shift_;
        double scale_;
    };
}

#include "npstat/stat/OSDE1D.icc"

#endif // NPSTAT_OSDE1D_HH_
