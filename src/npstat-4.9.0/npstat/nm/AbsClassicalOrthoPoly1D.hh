#ifndef NPSTAT_ABSCLASSICALORTHOPOLY1D_HH_
#define NPSTAT_ABSCLASSICALORTHOPOLY1D_HH_

/*!
// \file AbsClassicalOrthoPoly1D.hh
//
// \brief Base class for classical continuous orthonormal polynomials
//
// Author: I. Volobouev
//
// May 2017
*/

#include <utility>
#include <algorithm>
#include <climits>

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/Matrix.hh"
#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    class StorablePolySeries1D;

    class AbsClassicalOrthoPoly1D
    {
    public:
        inline virtual ~AbsClassicalOrthoPoly1D() {}

        /** Virtual copy constructor */
        virtual AbsClassicalOrthoPoly1D* clone() const = 0;

        /** The weight function should be normalized */
        virtual long double weight(long double x) const = 0;

        //@{
        /** Support of the polynomial */
        virtual double xmin() const = 0;
        virtual double xmax() const = 0;
        //@}

        /** Maximum polynomial degree supported */
        inline virtual unsigned maxDegree() const {return UINT_MAX - 1U;}

        /** Polynomial values */
        long double poly(unsigned deg, long double x) const;

        /** 
        // Values of two orthonormal polynomials.
        // Faster than calling "poly" two times.
        */
        std::pair<long double,long double> twopoly(
            unsigned deg1, unsigned deg2, long double x) const;

        /**
        // Values of all orthonormal polynomials up to some degree.
        // Faster than calling "poly" multiple times. The size of
        // the "values" array should be at least maxdeg + 1.
        */
        void allpoly(long double x, long double* values, unsigned maxdeg) const;

        /** Polynomial series */
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

        /**
        // Estimate the variances of the coefficients returned by
        // the "sampleCoeffs" function. The "coeffs" array should be
        // at least maxdeg + 1 elements long and should be filled
        // by a previous call to "sampleCoeffs". The "variances" array
        // should have at least maxdeg + 1 elements. It will contain
        // the respective variances upon return.
        */
        template <class Numeric>
        void sampleCoeffVars(const Numeric* coords, unsigned long lenCoords,
                             const double* coeffs, unsigned maxdeg,
                             double* variances) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the given sample of points (empirical density function).
        // The length of the array "coeffs" should be at least maxdeg + 1.
        // Note that the coefficients are returned in the order of
        // increasing degree (same order is used by the "series" function).
        // Before calculating the coefficients, the coordinates are shifted
        // and scaled according to x_new = (x_original - location)/scale.
        // The resulting coefficients are also divided by scale.
        */
        template <class Numeric>
        void sampleCoeffs(const Numeric* coords, unsigned long lenCoords,
                          Numeric location, Numeric scale,
                          double* coeffs, unsigned maxdeg) const;

        /**
        // Estimate the variances of the coefficients returned by
        // the "sampleCoeffs" function. The "coeffs" array should be
        // at least maxdeg + 1 elements long and should be filled
        // by a previous call to "sampleCoeffs" with the same location
        // and scale. The "variances" array should have at least maxdeg + 1
        // elements. It will contain the respective variances upon return.
        */
        template <class Numeric>
        void sampleCoeffVars(const Numeric* coords, unsigned long lenCoords,
                             Numeric location, Numeric scale,
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

        /**
        // Estimate the variances of the coefficients returned by the
        // "weightedSampleCoeffs" function. The "coeffs" array should
        // be at least maxdeg + 1 elements long and should be filled
        // by a previous call to "weightedSampleCoeffs". The "variances"
        // array should have at least maxdeg + 1 elements. It will contain
        // the respective variances upon return. This code assumes that
        // weights and coordinates are statistically independent from
        // each other.
        */
        template <class Pair>
        void weightedSampleCoeffVars(const Pair* points, unsigned long nPoints,
                                     const  double* coeffs, unsigned maxdeg,
                                     double* variances) const;

        /**
        // Build the coefficients of the orthogonal polynomial series for
        // the given sample of weighted points (empirical density function).
        // The first element of the pair will be treated as the coordinate
        // and the second element will be treated as weight. Weights must
        // not be negative. The length of the array "coeffs" should be
        // at least maxdeg + 1. Note that the coefficients are returned
        // in the order of increasing degree (same order is used by the
        // "series" function). Before calculating the coefficients, the
        // coordinates are shifted and scaled according to
        // x_new = (x_original - location)/scale. The resulting coefficients
        // are also divided by scale.
        */
        template <class Pair>
        void weightedSampleCoeffs(const Pair* points, unsigned long numPoints,
                                  typename Pair::first_type location,
                                  typename Pair::first_type scale,
                                  double* coeffs, unsigned maxdeg) const;

        /**
        // Estimate the variances of the coefficients returned by the
        // "weightedSampleCoeffs" function. The "coeffs" array should
        // be at least maxdeg + 1 elements long and should be filled
        // by a previous call to "weightedSampleCoeffs" with the same
        // location and scale. The "variances" array should have at least
        // maxdeg + 1 elements. It will contain the respective variances
        // upon return. This code assumes that weights and coordinates
        // are statistically independent from each other.
        */
        template <class Pair>
        void weightedSampleCoeffVars(const Pair* points, unsigned long nPoints,
                                     typename Pair::first_type location,
                                     typename Pair::first_type scale,
                                     const double* coeffs, unsigned maxdeg,
                                     double* variances) const;

        /**
        // This method is useful for testing the numerical precision
        // of the orthonormalization procedure. It returns the scalar
        // products between various polynomials.
        */
        template <class Quadrature>
        double empiricalKroneckerDelta(const Quadrature& quad,
                                       unsigned deg1, unsigned deg2) const;

        /**
        // A measure-weighted average of a product of four orthonormal
        // poly values
        */
        template <class Quadrature>
        double jointAverage(const Quadrature& q, unsigned deg1, unsigned deg2,
                            unsigned deg3, unsigned deg4) const;

        /**
        // A measure-weighted average of a product of multiple orthonormal
        // poly values. "checkedForZeros" argument should be set to "true"
        // if we are sure that there are no zero elements in the "degrees"
        // array.
        */
        template <class Quadrature>
        double jointAverage(const Quadrature& q,
                            const unsigned* degrees, unsigned nDegrees,
                            bool checkedForZeros = false) const;

        /**
        // An unweighted integral of the orthonormal polynomial with the
        // given degree to some power. For this method, it is assumed
        // that the polynomials are supported on a closed interval (without
        // such an assumption unweighted integrals do not make much sense)
        // and that Gauss-Legendre quadratures can be used.
        */
        double integratePoly(unsigned degree, unsigned power) const;

        /**
        // An unweighted integral of a product of multiple orthonormal
        // polynomials. For this method, it is assumed that the polynomials
        // are supported on a closed interval (without such an assumption
        // unweighted integrals do not make much sense) and that Gauss-Legendre
        // quadratures can be used.
        */
        double jointIntegral(const unsigned* degrees, unsigned nDegrees) const;

        /**
        // Unweighted averages of the polynomial values for the given sample.
        // The length of array "averages" should be at least maxdeg + 1.
        */
        template <class Numeric>
        void sampleAverages(const Numeric* coords, unsigned long lenCoords,
                            double* averages, unsigned maxdeg) const;

        /**
        // Unweighted averages of the pairwise products of the polynomial
        // values for the given sample. The returned matrix will be symmetric
        // and will have the dimensions (maxdeg + 1) x (maxdeg + 1).
        */
        template <class Numeric>
        Matrix<double> sampleProductAverages(const Numeric* coords,
                                             unsigned long lenCoords,
                                             unsigned maxdeg) const;

        /**
        // Unweighted average of a product of polynomial values for the
        // given sample. "degrees" is the collection of polynomial degrees.
        // Polynomials of these degrees will be included in the product.
        */
        template <class Numeric>
        double jointSampleAverage(const Numeric* coords,
                                  unsigned long lenCoords,
                                  const unsigned* degrees,
                                  unsigned lenDegrees) const;

        // Similar to the previous method but calculates two averages
        // simultaneously
        template <class Numeric>
        std::pair<double, double> twoJointSampleAverages(
            const Numeric* coords, unsigned long lenCoords,
            const unsigned* degrees1, unsigned lenDegrees1,
            const unsigned* degrees2, unsigned lenDegrees2) const;

#ifndef SWIG
        CPP11_auto_ptr<StorablePolySeries1D> makeStorablePolySeries(
            unsigned maxPolyDeg,
            const double *coeffs=0, unsigned maxdeg=0) const;
#endif

    protected:
        inline static int kdelta(const unsigned i, const unsigned j)
            {return i == j ? 1 : 0;}

        // Recurrence relationship function should return alpha
        // as the first element of the pair and the square root
        // of beta as the second
        virtual std::pair<long double,long double>
        recurrenceCoeffs(unsigned deg) const = 0;

    private:
        // Helper classes
        class ProdFcn : public Functor1<long double, long double>
        {
        public:
            inline ProdFcn(const AbsClassicalOrthoPoly1D& p,
                           const unsigned d1,
                           const unsigned d2)
                : poly(p), deg1(d1), deg2(d2) {}

            inline long double operator()(const long double& x) const
            {
                const std::pair<long double,long double>& p =
                    poly.twopoly(deg1, deg2, x);
                return p.first*p.second*poly.weight(x);
            }

        private:
            const AbsClassicalOrthoPoly1D& poly;
            unsigned deg1;
            unsigned deg2;
        };

        class MultiProdFcn;
        friend class MultiProdFcn;

        class MultiProdFcn : public Functor1<long double, long double>
        {
        public:
            inline MultiProdFcn(const AbsClassicalOrthoPoly1D& p,
                                const unsigned* degrees, unsigned lenDegrees,
                                const bool includeWeight=true)
                : poly(p),
                  degs(degrees, degrees+lenDegrees),
                  maxdeg(0),
                  weighted(includeWeight)
            {
                if (lenDegrees)
                    maxdeg = *std::max_element(degrees, degrees+lenDegrees);
            }

            inline long double operator()(const long double& x) const
            {
                long double w = 1.0L, p = 1.0L;
                if (weighted)
                    w = poly.weight(x);
                if (maxdeg)
                    p = poly.normpolyprod(&degs[0], degs.size(), maxdeg, x);
                return w*p;
            }

        private:
            const AbsClassicalOrthoPoly1D& poly;
            std::vector<unsigned> degs;
            unsigned maxdeg;
            bool weighted;
        };

        // For calling the function below, maxdeg should be calculated as
        // *std::max_element(degrees, degrees+nDegrees)
        long double normpolyprod(const unsigned* degrees, unsigned nDegrees,
                                 unsigned maxdeg, long double x) const;
#ifdef SWIG
    public:
        inline double weight2(const double x) const
            {return weight(x);}

        inline double poly2(const unsigned deg, const double x) const
            {return poly(deg, x);}
#endif
    };

    /**
    // A functor for the weight function of the given ortho poly system.
    // The poly system is not copied, only a reference is used. It is
    // a responsibility of the user to make sure that the lifetime of
    // the poly system object exceeds the lifetime of the functor.
    */
    class OrthoPoly1DWeight : public Functor1<long double, long double>
    {
    public:
        inline explicit OrthoPoly1DWeight(const AbsClassicalOrthoPoly1D& fcn,
                                          const long double normfactor=1.0L)
            : fcn_(fcn), norm_(normfactor) {}

        inline virtual ~OrthoPoly1DWeight() {}

        inline virtual long double operator()(const long double& a) const
            {return norm_*fcn_.weight(a);}

    private:
        OrthoPoly1DWeight();
        const AbsClassicalOrthoPoly1D& fcn_;
        const long double norm_;
    };
}

#include "npstat/nm/AbsClassicalOrthoPoly1D.icc"

#endif // NPSTAT_ABSCLASSICALORTHOPOLY1D_HH_
