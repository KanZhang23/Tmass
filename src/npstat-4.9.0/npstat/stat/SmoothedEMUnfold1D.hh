#ifndef NPSTAT_SMOOTHEDEMUNFOLD1D_HH_
#define NPSTAT_SMOOTHEDEMUNFOLD1D_HH_

/*!
// \file SmoothedEMUnfold1D.hh
//
// \brief Expectation-maximization (a.k.a. D'Agostini) unfolding with smoothing
//
// Author: I. Volobouev
//
// May 2014
*/

#include "npstat/stat/AbsUnfold1D.hh"

namespace npstat {
    class SmoothedEMUnfold1D : public AbsUnfold1D
    {
    public:
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
        SmoothedEMUnfold1D(const Matrix<double>& responseMatrix,
                           const LocalPolyFilter1D& filter,
                           bool useConvolutions,
                           bool useMultinomialCovariance = false,
                           bool smoothLastIter = true,
                           double convergenceEpsilon = 1.0e-10,
                           unsigned maxIterations = 100000U);

        inline virtual ~SmoothedEMUnfold1D() {}

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

        /**
        // The main unfolding method. The structure of the covariance matrix
        // of observations (if provided) will be ignored for optimizing
        // the unfolded values -- it is only used to evaluate their covariance.
        // The "unfoldedCovarianceMatrix" parameter can be NULL in which case
        // the corresponding calculation is not performed (which saves some
        // CPU time).
        */
        virtual bool unfold(const double* observed, unsigned lenObserved,
                            const Matrix<double>* observationCovarianceMatrix,
                            double* unfolded, unsigned lenUnfolded,
                            Matrix<double>* unfoldedCovarianceMatrix);
    protected:
        /** Single expectation-maximization (a.k.a. D'Agostini) iteration */
        void update(const double* observed, unsigned lenObserved,
                    const double* prev, double* next, unsigned lenUnfolded,
                    bool performSmoothing) const;

        // Initial unfolding matrix a-la D'Agostini
        Matrix<double> unfoldingMatrix0(const double* unfolded,
                                        const double* yhat) const;

        // Correct error propagation matrix for D'Agostini's iterations
        Matrix<double> errorPropagationMatrix(
            const double* observed, unsigned lenObserved,
            const double* unfolded, unsigned lenUnfolded,
            const double* yHat, double norm, bool smoothLast, unsigned maxiter,
            double convergenceEps, unsigned* itersMade, bool* converged) const;

    private:
        // The following method can be overriden by derived classes
        // in case they want to realize a more sophisticated iteration
        // scheme in order to improve convergence. This function should
        // return the number of iterations performed so far. These
        // iterations will be counted towards the limit on the number
        // of iterations.
        inline virtual unsigned preIterate(
            const double* /* observed */, unsigned /* lenObserved */,
            double** /* prev */, double** /* next */, unsigned /* lenUnfold */)
                {return 0U;}

        double convergenceEpsilon_;
        mutable double lastNormfactor_;
        unsigned maxIterations_;
        unsigned lastIterations_;
        unsigned lastEPIterations_;
        bool useMultinomialCovariance_;
        bool smoothLast_;

        // Memory buffers
        std::vector<double> v1_, v2_;
        mutable std::vector<double> v3_, yhatBuf_;
    };
}

#endif // NPSTAT_SMOOTHEDEMUNFOLD1D_HH_
