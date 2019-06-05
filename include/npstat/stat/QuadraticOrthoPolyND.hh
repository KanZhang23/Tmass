#ifndef NPSTAT_QUADRATICORTHOPOLYND_HH_
#define NPSTAT_QUADRATICORTHOPOLYND_HH_

/*!
// \file QuadraticOrthoPolyND.hh
//
// \brief Multivariate quadratic orthogonal polynomials with arbitrary 
//        weight functions
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/nm/BoxND.hh"
#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    /**
    // Class for figuring out the coefficients of multivariate quadratic
    // orthogonal polynomials generated with an arbitrary weight functions
    */
    class QuadraticOrthoPolyND
    {
    public:
        /**
        // "weight" is the weigh function used to build the orthogonal
        // polynomials. "nSteps" is the number of integration intervals
        // in each dimension (numerical integration is used for
        // orthogonalization of the basis polynomials and for subsequent
        // construction of polynomial series). "nStepsDim" is the number
        // of elements in the "nSteps" array and also the dimensionality
        // of the coordinate space. The integration region is defined by
        // the "boundingBox" argument whose dimensionality must be consistent
        // with "nStepsDim".
        //
        // Integration will be performed by simply evaluating the functions
        // at the interval centers. Use a sufficiently large number of
        // integration points if high precision is desired.
        */
        QuadraticOrthoPolyND(const AbsDistributionND& weight,
                             const BoxND<double>& boundingBox,
                             const unsigned* nSteps, unsigned nStepsDim);
        ~QuadraticOrthoPolyND();

        // Copy constructor and the assignment operator
        QuadraticOrthoPolyND(const QuadraticOrthoPolyND&);
        QuadraticOrthoPolyND& operator=(const QuadraticOrthoPolyND&);

        //@{
        /** Inspect object properties */
        inline unsigned dim() const {return dim_;}
        inline unsigned nTerms() const {return ((dim_+1U)*(dim_+2U))/2U;}
        inline const AbsDistributionND* weight() const {return weight_;}
        inline const BoxND<double>& boundingBox() const {return box_;}
        inline const std::vector<unsigned>& nSteps() const {return numSteps_;}
        //@}

        /**
        // Coefficient in the expansion of the orthogonal series
        // polynomial in the monomial basis. The order of the monomials
        // is 1, x0, x1, ..., xN, x0^2, x0*x1, ..., x0*xN, x1^2, x1*x2, ...
        // The number of the polynomials is, of course, the same as the
        // number of monomials and equals "nTerms()".
        */
        double monomialCoefficient(unsigned iPoly, unsigned iMono) const;

        /**
        // Value of a certain polynomial at the given coordinate "x".
        // The argument "polyN" is the polynomial number which
        // should be less than "nTerms()". Note that the code does not
        // check whether the coordinate is inside the original box.
        */
        double value(unsigned polyN, const double* x, unsigned lenX) const;

        /**
        // Series expansion at the given coordinate "x". The number
        // of coefficients provided, nCoeffs, is allowed to take the
        // following values:
        //
        // 1          -- just return the constant term
        // dim() + 1  -- use linear terms only
        // nTerms()   -- use the full expansion
        //
        // Note that the code does not check whether the coordinate
        // is inside the original box.
        //
        // If the "individualPolynomials" array is not 0, the values
        // of the individual polynomials will be stored there. They
        // represent the gradient of the series expansion at the given
        // point with respect to the series coefficients. There must be
        // at least nCoeffs elements in the "individualPolynomials" array.
        */
        double series(const double* x, unsigned lenX,
                      const double* coeffs, unsigned nCoeffs,
                      double* individualPolynomials=0) const;

        /**
        // The gradient of the series expansion. The size of the result
        // array must be equal to "dim()". Note that the code does not
        // check whether the coordinate is inside the original box.
        */
        void gradient(const double* x, unsigned lenX,
                      const double* coeffs, unsigned nCoeffs,
                      double* result, unsigned lenResult) const;

        /**
        // The hessian of the series expansion. The size of the
        // result array must be equal to "dim()*(dim()+1)/2".
        // Hessian matrix is returned in the upper-trangular form.
        // The result is, of course, coordinate-independent.
        */
        void hessian(const double* coeffs, unsigned nCoeffs,
                     double* result, unsigned lenResult) const;

        /**
        // The following method fits orthogonal polynomial series
        // to an object which has its own method with signature
        // "Numeric method(const double* x, unsigned dimX) const".
        // The arguments which will be fed into this method will be
        // shifted and scaled from the box coordinates according to
        // x[i] = (boxCoordinate[i] - location[i])/scale[i].
        // It must be possible to perform a static cast from
        // "Numeric" to double.
        //
        // "nCoeffs" can have multiple values, as in the "series"
        // method. The bounding box and the number of integration
        // intervals are assumed to be the same as in the constructor.
        */
        template <class T, typename Numeric>
        void fit(const T& obj, Numeric (T::*)(const double*, unsigned) const,
                 const double* location, const double* scale, unsigned nScales,
                 double* coeffs, unsigned nCoeffs) const;
    private:
        QuadraticOrthoPolyND();
        
        void integrationLoop(unsigned level, long double *gram);
        void gramSchmidt(const long double *gram, unsigned startingMono);

        template <class T, typename Numeric>
        void fitLoop(const T& obj,
                     Numeric (T::*)(const double*, unsigned) const,
                     unsigned level,
                     const double* location, const double* scale,
                     double* methodCoords,
                     long double *sum, unsigned nCoeffs) const;

        AbsDistributionND* weight_;
        BoxND<double> box_;
        std::vector<unsigned> numSteps_;
        std::vector<double> coeffM_;
        mutable std::vector<double> work_;
        mutable std::vector<double> buf_;
        mutable std::vector<long double> sprod_;
        long double cellSize_;
        unsigned dim_;
    };
}

#include "npstat/stat/QuadraticOrthoPolyND.icc"

#endif // NPSTAT_QUADRATICORTHOPOLYND_HH_
