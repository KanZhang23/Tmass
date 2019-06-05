#ifndef EMSUNFOLD_SPARSEUNFOLDINGBANDWIDTHSCANNERND_HH_
#define EMSUNFOLD_SPARSEUNFOLDINGBANDWIDTHSCANNERND_HH_

/*!
// \file SparseUnfoldingBandwidthScannerND.hh
//
// \brief Characterization of multivariate unfolding performance vs. bandwidth
//        for unfolding classes that utilize sparse matrices
//
// Author: I. Volobouev
//
// July 2014
*/

#include <map>

#include "npstat/emsunfold/AbsSparseUnfoldND.hh"
#include "npstat/emsunfold/trlanEigensystem.hh"

#include "npstat/stat/SymbetaParams1D.hh"
#include "npstat/stat/MemoizingSymbetaFilterProvider.hh"

namespace emsunfold {
    template<class Matrix>
    class SparseUnfoldingBandwidthScannerND
    {
    public:
        typedef AbsSparseUnfoldND<Matrix> unfolding_type;
        typedef typename unfolding_type::response_matrix_type response_matrix_type;
        typedef typename unfolding_type::input_covariance_type input_covariance_type;
        typedef typename unfolding_type::output_covariance_type output_covariance_type;
        typedef typename unfolding_type::filter_type filter_type;

        /**
        // The constructor arguments are as follows:
        //
        //  unfold                 -- An instance of AbsSparseUnfoldND class.
        //
        //  filterParameters       -- Specifications how to build the
        //                            smoothing filters for each dimension
        //                            of the unfolded data. The corresponding
        //                            values will be eventually passed to the
        //                            "symbetaLOrPEFilter1D" call. The number
        //                            of elements in this vector must be equal
        //                            to the rank of the response matrix used
        //                            by the "unfold" argument.
        //
        //  trlanParameters        -- Parameters which steer determination
        //                            of covariance matrix eigenspectrum by
        //                            TRLAN.
        //
        //  minAbsoluteCorrelation -- Correlation coefficients of the unfolded
        //                            covariance matrix with absolute values
        //                            below this cutoff will be set to 0 in
        //                            an attempt to reduce the matrix size.
        //                            If this argument is 0 or negative, the
        //                            unfolded covariance matrix will not be
        //                            pruned.
        //
        //  observed               -- Observed data. Can be modified later
        //                            using "setObservedData" method. Array
        //                            shape, however, can not change.
        //
        //  nDoFCorrectionFactor   -- The correction factor to use for
        //                            determining the number of effective
        //                            parameters in the fit. If this argument
        //                            is positive, it will be multiplied
        //                            by the number of parameters determined
        //                            by the standard procedure (so set it to
        //                            1.0 in order not to apply any correction).
        //                            If this argument is 0 or negative, the
        //                            correction factor will be calculated
        //                            as the fraction of bins filled in the
        //                            observed data.
        //
        //  observationCovariance  -- Covariance matrix for the observed
        //                            values. If not NULL, this matrix should
        //                            be symmetric and positive-definite.
        //                            An internal copy will be made.
        //
        //  oracle                 -- The "correct" unfolded distribution
        //                            for use in various comparisons with
        //                            unfolded results. If oracle is NULL,
        //                            this distribution will be considered
        //                            unknown, and the corresponding
        //                            comparisons will not be made.
        */
        SparseUnfoldingBandwidthScannerND(
            unfolding_type& unfold,
            const std::vector<npstat::SymbetaParams1D>& filterParameters,
            const EigenParameters& trlanParameters,
            double minAbsoluteCorrelation,
            const npstat::ArrayND<double>& observed,
            double nDoFCorrectionFactor,
            const input_covariance_type* observationCovariance = 0,
            const npstat::ArrayND<double>* oracle = 0);

        virtual ~SparseUnfoldingBandwidthScannerND();

        /** Dimensionality of the unfolded space */
        inline unsigned unfoldedDim() const
            {return filterParameters_.size();}

        /** Dimensionality of the observed space */
        inline unsigned observedDim() const
            {return unfold_.getObservedShape().size();}

        /** 
        // Names of the variables calculated by calling the "process" method.
        // The meaning of the variables is described below. Variables which
        // have * in front of their names will have meaningful values only
        // in case the known "oracle" distribution was provided in the
        // constructor.
        //
        //  bandwidth_N                -- With N = 0, 1, ..., up to
        //                                filterParameters.size() - 1.
        //                                Last bandwidth set processed. All
        //                                these values will be set to -2.0 if
        //                                "ntuplize" was called before the
        //                                first call to "process".
        //
        //  unprunedNonZeroFrac        -- Fraction of non-zero entries in the
        //                                unfolding covariance matrix before
        //                                pruning of small correlations.
        //
        //  nonZeroFrac                -- Fraction of non-zero entries in the
        //                                unfolding covariance matrix after
        //                                pruning of small correlations.
        //
        //  foldedSum                  -- Sum of the Poisson means fitted to
        //                                the observations.
        //
        //  unfoldedSum                -- Sum of the unfolded values.
        //
        //  *smoothedOracleSum         -- Sum of the smoothed oracle data.
        //
        //  foldedLogli                -- Poisson log-likelihood for the
        //                                observations assuming that the
        //                                Poisson means are given by the fit.
        //                                If the covariance matrix of the
        //                                observations was provided by the
        //                                user, this variable will be set to
        //                                -chi^2/2 instead.
        //
        //  *unfoldedLogli             -- Poisson log-likelihood for the
        //                                unfolded distribution assuming that
        //                                the Poisson means are given by the
        //                                oracle data.
        //
        //  *unfoldedISE               -- Integrated squared error for the
        //                                unfolded distribution w.r.t. the
        //                                oracle (both distributions are
        //                                normalized to 1). This variable is
        //                                the easiest one to check in order
        //                                to find out whether an oracle was
        //                                provided (its value will be negative
        //                                if it wasn't).
        //
        //  *unfoldedDiagChisq         -- Chi-squared of the unfolded
        //                                distribution w.r.t. the oracle
        //                                using only the diagonal elements
        //                                of the unfolding covariance matrix.
        //
        //  *smoothedUnfoldedLogli     -- Poisson log-likelihood for the
        //                                unfolded distribution assuming that
        //                                the Poisson means are given by the
        //                                smoothed racle data.
        //
        //  *smoothedUnfoldedISE       -- Integrated squared error for the
        //                                unfolded distribution w.r.t. the
        //                                smoothed oracle (both distributions
        //                                are normalized to 1).
        //
        //  *smoothedUnfoldedDiagChisq -- Chi-squared of the unfolded
        //                                distribution w.r.t. the smoothed
        //                                oracle using only the diagonal
        //                                elements of the covariance matrix.
        //
        //  modelNDoFEntropic          -- Effective NDoF for the matrix
        //  modelNDoFTrace                H = K*E*(K*E)^T which plays a role
        //                                similar to the hat matrix in
        //                                regression. Here, E is the error
        //                                propagation matrix for unfolding
        //                                uniform observed distribution.
        //                                "Entropic" means that the exponent of
        //                                the eigenspectrum entropy is used to
        //                                define effective NDoF, and "Trace"
        //                                means that the ratio of the matrix
        //                                trace to the largest eigenvalue is
        //                                used.
        //
        //  AICcEntropic               -- AIC (Akaike information criterion)
        //  AICcTrace                     with a correction for the finite
        //                                sample size. Calculated using
        //                                "foldedLogli" and corresponding
        //                                "modelNDoF".
        //
        //  smoothingNormfactor        -- Normalization factor applied during
        //                                the last smoothing procedure.
        //
        //  integratedVariance         -- Product of the covariance matrix
        //                                trace and the bin width in the
        //                                unfolded space.
        //
        //  nIterations                -- Number of iterations used to process
        //                                this bandwidth (e.g., by the
        //                                expectation-maximization method).
        //
        //  unfoldingStatus            -- Status returned by the "unfold" call
        //                                of the unfolding object.
        //
        //  trlan_*                    -- Multiple variables which provide
        //                                TRLAN diagnostic information. See
        //                                the description of TrlanDiagnostics
        //                                class.
        */
        virtual std::vector<std::string> variableNames() const;

        /**
        // Number of variables calculated by the "process" method.
        // See the description of "variableNames" method for details.
        */
        virtual unsigned variableCount() const;

        /**
        // Write out produced variables into a common buffer.
        // The order of the values will be consistent with the
        // names returned by the "variableNames" method.
        // The function returns the number of variables filled.
        */
        virtual unsigned ntuplize(double* buf, unsigned len) const;

        /** 
        // Perform unfolding with the given bandwidth values. This method
        // returns "true" on success and "false" on failure.
        */
        bool process(const std::vector<double>& bandwidthValues);

        /** Last bandwidth values processed */
        inline const std::vector<double>& lastBandwidthValues() const
            {return bandwidthValues_;}

        /**
        // Change the array of observations. Array dimensions must be
        // compatible with those given in the constructor. The pointer
        // to the covariance matrix of observations can be NULL. If provided,
        // this matrix should be symmetric and positive-definite.
        */
        virtual void setObservedData(
            const npstat::ArrayND<double>& observed,
            const input_covariance_type* observationCovarianceMatrix);

        /** Return the observed data */
        inline const npstat::ArrayND<double>& getObservedData() const
            {return observed_;}

        /**
        // Set the bias (this is useful for various studies of uncertainties).
        // If set, this bias will be added to the smoothed oracle data. The
        // dimensions must be compatible with those of the unfolded result.
        */
        virtual void setBias(const npstat::ArrayND<double>& bias);

        /** Return the bias data provided by the last "setBias" call */
        inline const npstat::ArrayND<double>& getBias() const {return bias_;}

        /** Clear bias data */
        inline virtual void clearBias() {bias_.uninitialize();}

        /** Status of the last unfolding call ("true" means success) */
        inline virtual bool lastSparseUnfoldingStatus() const
            {return unfoldingStatus_;}

        /** Last unfolded distribution */
        inline const npstat::ArrayND<double>& unfoldedResult() const
            {return unfolded_;}

        /** Last unfolded covariance matrix */
        inline const output_covariance_type& unfoldedCovariance() const
            {return unfoldedCovmat_;}

        /** Response matrix */
        inline const response_matrix_type& responseMatrix() const
            {return unfold_.responseMatrix();}

        /**
        // Return oracle data smoothed with the last processed bandwidth.
        // If the bias was set, it was added to this data.
        */
        inline const npstat::ArrayND<double>& smoothedOracle() const 
            {return smoothedOracle_;}

        /** Return eigenvector differences divided by sigma */
        inline const std::vector<double>& eigenDeltas() const 
            {return eigenDeltas_;}

        /** Return covariance matrix eigenvalues (in the decreasing order) */
        inline const std::vector<double>& covEigenValues() const 
            {return covEigenValues_;}

        /** Are we using convolutions with our filters? */
        inline bool usingConvolutions() const
            {return unfold_.usingConvolutions();}

        /** Set the initial approximation to the unfolded solution */
        inline void setInitialApproximation(const npstat::ArrayND<double>& a)
            {unfold_.setInitialApproximation(a);}

        /** Clear the initial approximation to the unfolded solution */
        inline void clearInitialApproximation()
            {unfold_.clearInitialApproximation();}

        /** Return the initial approximation to the unfolded solution */
        inline const npstat::ArrayND<double>& getInitialApproximation() const
            {return unfold_.getInitialApproximation();}

        /** Correction factor for the number of degrees of freedom */
        inline double nDoFCorrectionFactor() const {return nDoFCorr_;}
        
        /** Set the correction factor for the number of degrees of freedom */
        inline void setNDoFCorrectionFactor(const double f) {nDoFCorr_ = f;}

    protected:
        // Return "true" on success
        virtual bool performUnfolding();

        // Variables set by the constructor
        unfolding_type& unfold_;
        std::vector<npstat::SymbetaParams1D> filterParameters_;
        EigenParameters trlanParameters_;
        double minAbsoluteCorrelation_;
        npstat::ArrayND<double> observed_;
        npstat::ArrayND<double> oracle_;
        input_covariance_type observationCovariance_;
        bool haveObservationCovariance_;
        double binVolume_;
        double nDoFCorr_;
        double nObserved_;
        double obsNonZeroFraction_;

        // Bandwidth is set by the "process" method
        std::vector<double> bandwidthValues_;

        // Unfolding results (calculated by the "performUnfolding" method)
        npstat::ArrayND<double> unfolded_;
        output_covariance_type unfoldedCovmat_;
        npstat::ArrayND<double> smoothedOracle_;
        std::vector<double> eigenDeltas_;
        std::vector<double> covEigenValues_;
        TrlanDiagnostics diagnose_;

    private:
        // Disable copy constructor an assignment operator
        SparseUnfoldingBandwidthScannerND();
        SparseUnfoldingBandwidthScannerND(const SparseUnfoldingBandwidthScannerND&);
        SparseUnfoldingBandwidthScannerND& operator=(
            const SparseUnfoldingBandwidthScannerND&);

        void getModelNDoF(double* ndof1, double* ndof2,
                          const std::vector<double>& bwValues);

        double foldedLogLikelihood(const npstat::ArrayND<double>& fitted,
                                   const npstat::ArrayND<double>& observed);

        bool unfoldAndPrune(const npstat::ArrayND<double>& observed,
                            const input_covariance_type* observationCovMat,
                            double* initialNonzeroFraction);

        static double getNonZeroFraction(const npstat::ArrayND<double>&);

        // Folded distribution estimate
        npstat::ArrayND<double> folded_;

        // Bias
        npstat::ArrayND<double> bias_;

        double unprunedNonZeroFrac_;
        double nonZeroFrac_;

        double foldedSum_;
        double unfoldedSum_;
        double smoothedOracleSum_;
        double foldedLogli_;

        double unfoldedLogli_;
        double unfoldedISE_;
        double unfoldedDiagChisq_;

        double smoothedUnfoldedLogli_;
        double smoothedUnfoldedISE_;
        double smoothedUnfoldedDiagChisq_;

        double modelNDoFEntropic_;
        double modelNDoFTrace_;
        double AICcEntropic_;
        double AICcTrace_;

        double smoothingNormfactor_;
        double integratedVariance_;
        unsigned nIterations_;
        bool unfoldingStatus_;

        // Variable set by "variableNames" call
        mutable unsigned nVariables_;

        const filter_type* oldFilter_;

        npstat::MemoizingSymbetaFilterProvider filterProvider_;
        std::map<std::vector<double>, std::pair<double,double> > ndofMap_;

        const filter_type* localFilter_;

        std::vector<const npstat::LocalPolyFilter1D*> localFilterBuf_;
        std::vector<double> deltaBuf_;
    };
}

#include "npstat/emsunfold/SparseUnfoldingBandwidthScannerND.icc"

#endif // EMSUNFOLD_SPARSEUNFOLDINGBANDWIDTHSCANNERND_HH_
