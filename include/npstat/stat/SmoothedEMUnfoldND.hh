#ifndef NPSTAT_SMOOTHEDEMUNFOLDND_HH_
#define NPSTAT_SMOOTHEDEMUNFOLDND_HH_

/*!
// \file SmoothedEMUnfoldND.hh
//
// \brief Multivariate expectation-maximization unfolding with smoothing
//
// Author: I. Volobouev
//
// June 2014
*/

#include "npstat/stat/AbsUnfoldND.hh"

namespace npstat {
    class SmoothedEMUnfoldND : public AbsUnfoldND
    {
    public:
        typedef AbsUnfoldND Base;

        /**
        // The constructor arguments are:
        //
        // responseMatrix           -- Naturally, the problem response matrix.
        //
        // filter                   -- The filter to use for smoothing the
        //                             unfolded values. This object will not
        //                             make a copy of the filter. It is
        //                             a responsibility of the caller to ensure
        //                             that the argument filter exists while
        //                             this object is in use.
        //
        // useConvolutions          -- If "true", the code will call the
        //                             "convolve" method of the filter rather
        //                             than its "filter" method.
        //
        // useMultinomialCovariance -- Specifies whether we should use
        //                             multinomial distribution to estimate
        //                             covariance of fitted observations
        //                             (otherwise Poisson assumption is used).
        //
        // smoothLastIter           -- If "false", smoothing will not be
        //                             applied after the last iteration.
        //                             Setting this parameter to "false" is
        //                             not recommended for production results
        //                             because it is unclear how to compare
        //                             such results with models.
        //
        // convergenceEpsilon       -- Convergence criterion parameter for
        //                             various iterations.
        //
        // maxIterations            -- Maximum number of iterations allowed
        //                             (both for the expectation-maximization
        //                             iterations and for the code estimating
        //                             the error propagation matrix).
        */
        SmoothedEMUnfoldND(const ResponseMatrix& responseMatrix,
                           const AbsUnfoldingFilterND& filter,
                           bool useConvolutions,
                           bool useMultinomialCovariance = false,
                           bool smoothLastIter = true,
                           double convergenceEpsilon = 1.0e-10,
                           unsigned maxIterations = 100000U);

        inline virtual ~SmoothedEMUnfoldND() {}

        /** Change maximum number of allowed iterations */
        inline void setMaxIterations(const unsigned u) {maxIterations_ = u;}

        /** Switch between multinomial/Poisson covariance for observed space */
        inline void useMultinomialCovariance(const bool b)
            {useMultinomialCovariance_ = b;}

        /** Switch between smoothing/not smoothing the last iteration */
        inline void smoothLastIteration(const bool b) {smoothLast_ = b;}

        /** Change the convergence criterion */
        void setConvergenceEpsilon(double eps);

        //@{
        /** Simple inspector of object properties */
        inline double convergenceEpsilon() const {return convergenceEpsilon_;}
        inline unsigned maxIterations() const {return maxIterations_;}
        inline bool usingMultinomialCovariance() const
            {return useMultinomialCovariance_;}
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
        virtual bool unfold(const ArrayND<double>& observed,
                            const Matrix<double>* observationCovarianceMatrix,
                            ArrayND<double>* unfolded,
                            Matrix<double>* unfoldedCovarianceMatrix);
    protected:
        /** Single expectation-maximization (a.k.a. D'Agostini) iteration */
        void update(const ArrayND<double>& observed,
                    const ArrayND<double>* prev, ArrayND<double>* next,
                    bool performSmoothing) const;

    private:
        // Error propagation matrix for expectation-maximization iterations
        Matrix<double> errorPropagationMatrix(
            const double* observed, unsigned lenObserved,
            const double* unfolded, unsigned lenUnfolded,
            const double* yHat, double norm, bool smoothLast, unsigned maxiter,
            double convergenceEps, unsigned* itersMade, bool* converged) const;

        double convergenceEpsilon_;
        mutable double lastNormfactor_;
        unsigned maxIterations_;
        unsigned lastIterations_;
        unsigned lastEPIterations_;
        bool useMultinomialCovariance_;
        bool smoothLast_;

        // Memory buffers
        ArrayND<double> v1_, v2_;
        mutable ArrayND<double> yhatBuf_, unfoldedBuf_;
    };
}

#endif // NPSTAT_SMOOTHEDEMUNFOLDND_HH_
