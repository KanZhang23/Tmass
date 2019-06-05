#ifndef NPSTAT_ORTHOPOLYND_HH_
#define NPSTAT_ORTHOPOLYND_HH_

/*!
// \file OrthoPolyND.hh
//
// \brief Discrete orthogonal polynomial series of arbitrary dimensionality
//        in hyperrectangular domains
//
// These series are intended for use in local filtering.
//
// Author: I. Volobouev
//
// October 2009
*/

#include <vector>

#include "npstat/nm/ArrayND.hh"

namespace npstat {
    /**
    // This class is templated on the maximum polynomial degree
    // that can be used
    */
    template <unsigned MaxDeg>
    class OrthoPolyND
    {
    public:
        /**
        // Main constructor. The "polyDegree" parameter must not exceed
        // the "MaxDeg" template parameter.
        //
        // "weight" is an arbitrary weight function, specified by its
        // values on a grid with equidistant steps in each dimension.
        //
        // "steps" is the array of steps. It can be specified as NULL.
        // In such a case the width of the integration region in each
        // dimension is assumed to be 1.0 and the step in each dimension
        // D is calculated as 1.0/weight.span(D).
        */
        template <unsigned StackLen, unsigned StackDim>
        OrthoPolyND(unsigned polyDegree,
                    const ArrayND<double,StackLen,StackDim>& weight,
                    const double* steps = 0, unsigned nSteps = 0);

        /** Copy constructor */
        OrthoPolyND(const OrthoPolyND&);

        // Destructor
        ~OrthoPolyND();

        /** The assignment operator */
        OrthoPolyND& operator=(const OrthoPolyND&);

        /** What is the dimensionality of the coordinate space? */
        inline unsigned dim() const {return weight_.rank();}

        /** Maximum polynomial degree */
        inline unsigned maxDegree() const {return maxdeg_;}

        /** How many terms the series will have? */
        inline unsigned nTerms() const {return poly_.size();}

        /** What is the degree of the given term in the series? */
        inline unsigned degree(const unsigned termNumber) const
            {return degs_.at(termNumber);}

        /** The step size in the given dimension */
        inline double step(const unsigned nd) const {return steps_.at(nd);}

        /** Get the table of weights */
        inline const ArrayND<double>& weight() const {return weight_;}

        /**
        // Return the value of one of the polynomials. "ix" is the
        // multivariate index of the array point for which the polynomial
        // value should be returned.
        */
        inline double poly(const unsigned termNumber, const unsigned* ix,
                           const unsigned lenIx) const
            {return poly_.at(termNumber)->valueAt(ix, lenIx);}

        //@{
        /** Compare for equality */
        bool operator==(const OrthoPolyND& r) const;
        bool operator!=(const OrthoPolyND& r) const;
        //@}

        /**
        // Series at a single point. The first coefficient refers to
        // the lowest degree polynomial (the constant term). The number
        // of coefficients provided must be equal to the number of terms
        // in the series (as returned by the "nTerms" method).
        */
        double series(const double* coeffs, unsigned lenCoeffs,
                      const unsigned* ix, unsigned lenIx) const;

        /** Series on the whole hyperrectangle */
        ArrayND<double> arraySeries(const double* coeffs,
                                    unsigned lenCoeffs) const;

        /**
        // Generate a linear filter for the given point. Note that
        // this filter is generated on the heap and the user must
        // eventually call the "delete" operator on the result
        // (or just use a smart pointer to hold it).
        */
        ArrayND<double>* linearFilter(
            const double* coeffs, unsigned lenCoeffs,
            const unsigned* ix, unsigned lenIx) const;

        /**
        // Build the series for the given gridded data.
        // "shift" is the offset with which the sliding window
        // of the weight will be placed inside "gridData".
        // The "shift" array can be NULL in which case it will be
        // assumed that all shift values are 0.
        */
        template <unsigned StackLen, unsigned StackDim>
        void calculateCoeffs(const ArrayND<double,StackLen,StackDim>& gridData,
                             const unsigned* shift, unsigned lenShift,
                             double *coeffs, unsigned lenCoeffs) const;
    private:
        OrthoPolyND();

        class Monomial
        {
        public:
            Monomial(unsigned mxdim, unsigned deg, unsigned long imono);

            unsigned degree() const;
            bool operator<(const Monomial& r) const;
            bool operator==(const Monomial& r) const;
            bool operator!=(const Monomial& r) const;
            double operator()(const double *x, unsigned xlen) const;

        private:
            Monomial();

            unsigned dims[MaxDeg];
            unsigned nDims;
        };

        class GridMonomial
        {
        public:
            GridMonomial(const Monomial& m, const double* steps,
                         double* xwork, unsigned nSteps);
            double operator()(const unsigned* ind, const unsigned len) const;

        private:
            const Monomial& mono_;
            const double* steps_;
            double* xwork_;
            const unsigned nSteps_;
        };

        ArrayND<double> weight_;
        std::vector<ArrayND<double>*> poly_;
        std::vector<double> steps_;
        std::vector<unsigned> degs_;
        double cellVolume_;
        unsigned maxdeg_;

        void generateMonomialSet(unsigned mxDeg, std::vector<Monomial>*) const;
        double scalarProduct(const ArrayND<double>&,
                             const ArrayND<double>&) const;
        void gramSchmidt(unsigned startingMono);

        template <unsigned StackLen, unsigned StackDim>
        long double windowProductLoop(
            unsigned level,
            const ArrayND<double,StackLen,StackDim>& data,
            const ArrayND<double>& poly,
            const unsigned* polyShift,
            unsigned long idxData, unsigned long idxPoly) const;
    };
}

#include "npstat/nm/OrthoPolyND.icc"

#endif // NPSTAT_ORTHOPOLYND_HH_
