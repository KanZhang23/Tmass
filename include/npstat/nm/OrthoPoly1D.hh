#ifndef NPSTAT_ORTHOPOLY1D_HH_
#define NPSTAT_ORTHOPOLY1D_HH_

/*!
// \file OrthoPoly1D.hh
//
// \brief Discrete orthogonal polynomial series in one dimension
//
// These series are intended for use in local (or global) filtering.
//
// Author: I. Volobouev
//
// October 2009
*/

namespace npstat {
    class OrthoPoly1D
    {
    public:
        OrthoPoly1D();
        OrthoPoly1D(const OrthoPoly1D&);

        /*
        // Main constructor. The arguments are as follows:
        //
        //  maxDegree -- Maximum degree of the polynomials.
        //
        //  weight    -- Tabulated weight function, in equidistant
        //               steps, in the order of increasing x.
        //               All values must be non-negative.
        //
        //  weightLen -- Length of the "weight" array. This length
        //               must be larger than "maxDegree".
        //
        //  step      -- The x distance between two points in the table
        //               of weights. A good choice of "step" helps the
        //               polynomial values to remain within the dynamic
        //               range of double precision numbers.
        */
        OrthoPoly1D(unsigned maxDegree, const double* weight,
                    unsigned weightLen, double step);

        ~OrthoPoly1D();

        /** The assignment operator */
        OrthoPoly1D& operator=(const OrthoPoly1D&);

        //@{
        /** A simple inspector of object properties */
        inline unsigned length() const {return nw_;}
        inline unsigned maxDegree() const {return maxdeg_;}
        inline double step() const {return step_;}
        //@}

        //@{
        /** Compare for equality */
        bool operator==(const OrthoPoly1D& r) const;
        bool operator!=(const OrthoPoly1D& r) const;
        //@}

        /** Return the weight function value for the given index */
        double weight(unsigned index) const;

        /** Return the value of one of the polynomials for the given index */
        double poly(unsigned deg, unsigned index) const;

        /**
        // Product of the weight function and one of the polynomials
        // for the given index
        */
        double polyTimesWeight(unsigned deg, unsigned index) const;

        /**
        // Return the values of the orthogonal polynomial series
        // at the point with the given index
        */
        double series(const double *coeffs,
                      unsigned maxdeg, unsigned index) const;

        /**
        // Generate a linear filter for the point with the given index.
        // "coeffs" are the taper function coefficients. The length of
        // the result should be equal to the length of the weight given
        // in the constructor.
        */
        template <typename Numeric>
        void linearFilter(const double *coeffs, unsigned maxdeg, unsigned index,
                          Numeric *result, unsigned lenResult) const;

        /**
        // Generate a global filter (OSDE-style).
        // "coeffs" are the taper function coefficients. The length of
        // the "result" array should be equal to the square of the length
        // of the weight given in the constructor. It will be assumed that
        // "result" will later be treated as a square matrix which will
        // be multiplied by a column of input data.
        */
        template <typename Numeric>
        void globalFilter(const double *coeffs, unsigned maxdeg,
                          Numeric *result, unsigned long lenResult) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the given function. "dataLen" should be equal to "weightLen"
        // provided in the constructor. The length of the array "coeffs"
        // should be at least "maxdeg" + 1. Note that the coefficients
        // are returned in the order of increasing degree (same order
        // is used by the "series" function).
        */
        void calculateCoeffs(const double *data, unsigned dataLen,
                             double *coeffs, unsigned maxdeg) const;

        /**
        // This method is useful for testing the numerical precision
        // of the orthonormalization procedure. It returns the scalar
        // products between various polynomials.
        */
        double empiricalKroneckerDelta(unsigned deg1, unsigned deg2) const;

        /**
        // Orthogonal series for the weight function itself.
        // The length of the array "coeffs" should be at least maxdeg + 1.
        */
        void weightExpansionCoeffs(double *coeffs, unsigned maxdeg) const;

        /**
        // Covariance matrix between weight expansion coefficients in the
        // assumption that the weight function is a probability that is
        // being sampled. The covariance is calculated for a single
        // point sample. If the sample has N points, the covariance matrix
        // should be divided by N (this assumes that the weight expansion
        // coefficients are determined for a normalized EDF).
        */
        double weightExpansionCovariance(unsigned deg1, unsigned deg2) const;

        /** Scalar products between polynomials without the weight */
        double unweightedPolyProduct(unsigned deg1, unsigned deg2) const;

    private:
        long double *weight_;
        long double *poly_;   // this is an array dimensioned [maxdeg+1][nw]
        long double step_;
        unsigned nw_;
        unsigned maxdeg_;

        long double unweightedProduct(const long double *x,
                                      const long double *y) const;
        long double scalarProduct(const long double *x,
                                  const long double *y) const;
        long double scalarProduct(const double *x,
                                  const long double *y) const;
        void gramSchmidt(unsigned startingDegree);
    };
}

#include "npstat/nm/OrthoPoly1D.icc"

#endif // NPSTAT_ORTHOPOLY1D_HH_
