#ifndef EMSUNFOLD_SMOOTHEDEMSPARSEUNFOLDND_HH_
#define EMSUNFOLD_SMOOTHEDEMSPARSEUNFOLDND_HH_

/*!
// \file SmoothedEMSparseUnfoldND.hh
//
// \brief Multivariate expectation-maximization unfolding with smoothing
//        using sparse matrices
//
// Author: I. Volobouev
//
// July 2014
*/

#include "npstat/emsunfold/AbsSparseUnfoldND.hh"

namespace emsunfold {
    template<class Matrix>
    class SmoothedEMSparseUnfoldND : public AbsSparseUnfoldND<Matrix>
    {
    public:
        typedef AbsSparseUnfoldND<Matrix> Base;
        typedef typename Base::response_matrix_type response_matrix_type;
        typedef typename Base::input_covariance_type input_covariance_type;
        typedef typename Base::output_covariance_type output_covariance_type;
        typedef typename Base::filter_type filter_type;

        /**
        // The constructor arguments are:
        //
        // responseMatrix     -- Naturally, the problem response matrix.
        //
        // filter             -- The filter to use for smoothing the
        //                       unfolded values. This object will not
        //                       make a copy of the filter. It is
        //                       a responsibility of the caller to ensure
        //                       that the argument filter exists while
        //                       this object is in use.
        //
        // observedShape      -- Expected shape of the observed data.
        //                       The array of observations provided in
        //                       all subsequent calls of the "unfold"
        //                       method must have this shape.
        //
        // useConvolutions    -- If "true", the code will call the
        //                       "convolve" method of the filter rather
        //                       than its "filter" method.
        //
        // smoothLastIter     -- If "false", smoothing will not be
        //                       applied after the last iteration.
        //                       Setting this parameter to "false" is
        //                       not recommended for production results
        //                       because it is unclear how to compare
        //                       such results with models.
        //
        // convergenceEpsilon -- Convergence criterion parameter for
        //                       various iterations.
        //
        // maxIterations      -- Maximum number of iterations allowed
        //                       (both for the expectation-maximization
        //                       iterations and for the code estimating
        //                       the error propagation matrix).
        */
        SmoothedEMSparseUnfoldND(const response_matrix_type& responseMatrix,
                                 const filter_type& filter,
                                 const npstat::ArrayShape& observedShape,
                                 bool useConvolutions,
                                 bool smoothLastIter = true,
                                 double convergenceEpsilon = 1.0e-10,
                                 unsigned maxIterations = 100000U);

        inline virtual ~SmoothedEMSparseUnfoldND() {}

        /** Change maximum number of allowed iterations */
        inline void setMaxIterations(const unsigned u) {maxIterations_ = u;}

        /**
        // This method is included for compatibility with the
        // npstat::SmoothedEMUnfoldND class. The call will be ignored.
        */
        inline void useMultinomialCovariance(bool) {}

        /** Switch between smoothing/not smoothing the last iteration */
        inline void smoothLastIteration(const bool b) {smoothLast_ = b;}

        /** Change the convergence criterion */
        void setConvergenceEpsilon(double eps);

        //@{
        /** Simple inspector of object properties */
        inline double convergenceEpsilon() const {return convergenceEpsilon_;}
        inline unsigned maxIterations() const {return maxIterations_;}
        inline bool usingMultinomialCovariance() const {return false;}
        inline bool smoothingLastIteration() const {return smoothLast_;}
        //@}

        /** 
        // Returns the last number of iterations used to calculate the unfolded
        // results. This number will be filled after each "unfold" call.
        */
        inline unsigned lastNIterations() const {return lastIterations_;}

        /**
        // The last number of iterations used to calculate the error
        // propagation matrix
        */
        inline unsigned lastEPIterations() const {return lastEPIterations_;}

        /** The normalization factor applied during the last smoothing step */
        inline double lastSmoothingNormfactor() const {return lastNormfactor_;}

        /** The main unfolding method */
        virtual bool unfold(
            const npstat::ArrayND<double>& observed,
            const input_covariance_type* observationCovarianceMatrix,
            npstat::ArrayND<double>* unfolded,
            output_covariance_type* unfoldedCovarianceMatrix);

    protected:
        /** Single expectation-maximization (a.k.a. D'Agostini) iteration */
        void update(const npstat::ArrayND<double>& observed,
                    const npstat::ArrayND<double>* prev,
                    npstat::ArrayND<double>* next,
                    bool performSmoothing) const;

    private:
        // Error propagation matrix for the expectation-maximization iterations
        Matrix errorPropagationMatrix(
            const double* observed, unsigned lenObserved,
            const double* unfolded, unsigned lenUnfolded,
            const double* yHat, double norm, bool smoothLast, unsigned maxiter,
            double convergenceEps, unsigned* itersMade, bool* converged) const;

        double convergenceEpsilon_;
        mutable double lastNormfactor_;
        unsigned maxIterations_;
        unsigned lastIterations_;
        unsigned lastEPIterations_;
        bool smoothLast_;

        // Memory buffers
        npstat::ArrayND<double> v1_, v2_;
        mutable npstat::ArrayND<double> yhatBuf_, unfoldedBuf_;
    };
}

#include "npstat/emsunfold/SmoothedEMSparseUnfoldND.icc"

#endif // EMSUNFOLD_SMOOTHEDEMSPARSEUNFOLDND_HH_
