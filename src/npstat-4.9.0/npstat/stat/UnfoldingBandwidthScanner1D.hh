#ifndef NPSTAT_UNFOLDINGBANDWIDTHSCANNER1D_HH_
#define NPSTAT_UNFOLDINGBANDWIDTHSCANNER1D_HH_

/*!
// \file UnfoldingBandwidthScanner1D.hh
//
// \brief Characterization of 1-d unfolding performance vs. filter bandwidth
//
// Author: I. Volobouev
//
// May 2014
*/

#include <map>
#include <string>

#include "npstat/nm/MinSearchStatus1D.hh"
#include "npstat/nm/Triple.hh"

#include "npstat/stat/AbsUnfold1D.hh"
#include "npstat/stat/BoundaryHandling.hh"
#include "npstat/stat/MemoizingSymbetaFilterProvider.hh"

namespace npstat {
    namespace Private {
        class UnfoldingBandwidthScanner1DHelper;
    }

    class AbsBinnedComparison1D;

    class UnfoldingBandwidthScanner1D
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        //  unfold                -- An instance of AbsUnfold1D class.
        //
        //  observed, lenObserved -- Observed data and its length. Can be
        //                           modified later using "setObservedData"
        //                           method.
        //
        //  observationCovariance -- Covariance matrix for the observed
        //                           values (can be NULL). If provided,
        //                           this matrix should be symmetric and
        //                           positive-definite. An internal copy
        //                           will be made.
        //
        //  oracle, lenOracle     -- The "correct" unfolded distribution
        //                           for use in various comparisons with
        //                           unfolded results. If lenOracle == 0,
        //                           this distribution will be considered
        //                           unknown (in this case "oracle" pointer
        //                           can be NULL), and the corresponding
        //                           comparisons will not be made.
        //
        //  symbetaPower          -- This code will create smoothing filters
        //  maxDegree                for the unfolded space using the
        //  xMinUnfolded             function "symbetaLOrPEFilter1D".
        //  xMaxUnfolded             These parameters will be passed to
        //  filterBoundaryMethod     the "symbetaLOrPEFilter1D" call.
        //
        //  nDoFCorrectionFactor  -- The correction factor to use for
        //                           determining the number of effective
        //                           parameters in the fit. If this argument
        //                           is positive, it will be multiplied
        //                           by the number of parameters determined
        //                           by the standard procedure (so set it to
        //                           1.0 in order not to apply any correction).
        //                           If this argument is 0 or negative, the
        //                           correction factor will be calculated as
        //                           the fraction of bins filled in the observed
        //                           data.
        //
        //  foldedComparators     -- The comparator objects to use in order
        //                           to compare observed data with its fit.
        //
        //  oracleComparators     -- The comparator objects to use in order
        //                           to compare unfolded data with the oracle
        //                           (in case the oracle is actually provided).
        */
        UnfoldingBandwidthScanner1D(
            AbsUnfold1D& unfold,
            const double* observed, unsigned lenObserved,
            const Matrix<double>* observationCovariance,
            const double* oracle, unsigned lenOracle,
            int symbetaPower, double maxDegree,
            double xMinUnfolded, double xMaxUnfolded,
            const BoundaryHandling& filterBoundaryMethod,
            double nDoFCorrectionFactor,
            const std::vector<const AbsBinnedComparison1D*>& foldedComparators,
            const std::vector<const AbsBinnedComparison1D*>& oracleComparators);

        virtual ~UnfoldingBandwidthScanner1D();

        /** 
        // Names of the variables calculated by calling the "process" method.
        // The meaning of the variables is described below. Variables which
        // have * in front of their names will have meaningful values only
        // in case the known "oracle" distribution was provided in the
        // constructor.
        //
        //  bandwidth                  -- Last bandwidth processed. Will be
        //                                set to -1.0 if processing has failed
        //                                and to -2.0 if "ntuplize" was called
        //                                before the first call to "process".
        //                                In case bandwidth is negative, values
        //                                of all other variables are undefined
        //                                (might simply remain unchanged from
        //                                the previous "process" call). Always
        //                                check that the bandwidth is
        //                                non-negative before looking at other
        //                                variables.
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
        //  filterNDoFEntropic         -- Effective number of degrees of
        //  filterNDoFTrace               freedom for the smoothing matrix,
        //                                (defined for S * S^T). "Entropic"
        //                                means that the exponent of the
        //                                eigenspectrum entropy is used to
        //                                define effective NDoF, and "Trace"
        //                                means that the ratio of the matrix
        //                                trace to the largest eigenvalue is
        //                                used.
        //
        //  foldingNDoFEntropic        -- Effective NDoF for the "folding"
        //  foldingNDoFTrace              matrix F = K*S (defined for F * F^T.
        //                                K is the response matrix here).
        //                                F "folds" the unsmoothed distribution
        //                                in the expectation-maximization
        //                                iterations.
        //
        //  unfoldedNDoFEntropic       -- Effective NDoF for the unfolding
        //  unfoldedNDoFTrace             covariance matrix.
        //
        //  modelNDoFEntropic          -- Effective NDoF for the matrix
        //  modelNDoFTrace                H = K*E*(K*E)^T which plays a role
        //                                similar to the hat matrix in
        //                                regression. Here, E is the error
        //                                propagation matrix for unfolding
        //                                uniform observed distribution.
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
        //  foldedDistance_N           -- With N = 0, 1, ..., up to
        //  foldedPValue_N                foldedComparators.size() - 1.
        //                                Distances and p-values between the
        //                                observations and their fitted values,
        //                                calculated by distribution comparators
        //                                provided in the "foldedComparators"
        //                                argument of the constructor.
        //
        //  *oracleDistance_N          -- With N = 0, 1, ..., up to
        //  *oraclePValue_N               oracleComparators.size() - 1.
        //                                Distances and p-values between oracle
        //                                data and unfolded values, calculated
        //                                by distribution comparators provided
        //                                in the "oracleComparators" argument
        //                                of the constructor.
        //
        //  *smoothedOracleDistance_N  -- With N = 0, 1, ..., up to
        //  *smoothedOraclePValue_N       oracleComparators.size() - 1.
        //                                Distances and p-values between
        //                                smoothed oracle data and unfolded
        //                                values, calculated by distribution
        //                                comparators provided in the
        //                                "oracleComparators" argument of
        //                                the constructor.
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
        // Perform unfolding with the given bandwidth. This method
        // returns "true" on success and "false" on failure.
        */
        bool process(double bandwidth);

        /**
        // Find the optimal bandwidth according to the AICc criterion and
        // process that bandwidth. The arguments "bwmin", "bwmax", and
        // "nsteps" specify the initial grid in the log space on which
        // bandwidth search is performed ("nsteps" must be larger than 2).
        // "startBw" is the bandwidth from which to start the search and
        // "startingFactor" (>1.0) is an aproximate stepping factor for
        // the beginning of the search. The argument "useEntropicNDoF"
        // specifies whether the code should calculate effective NDoF
        // for AICc using eigenspectrum entropy (if "true") or the trace
        // (if "false") of the relevant matrix. Correspondingly, the code
        // will minimize AICcEntropic or AICcTrace.
        //
        // As the intermediate results are memoized (filters, degrees of
        // freedom, etc), it is best to call this function on different
        // observed data with the same values of "bwmin", "bwmax", and
        // "nsteps" arguments.
        //
        // This function returns the status of the search for the minimum AICc.
        // The results of the "process" call for the corresponding bandwidth
        // can be retrieved in the usual manner, using methods "ntuplize",
        // "lastUnfoldingStatus", "lastBandwidth", "unfoldedResult", etc.
        */
        MinSearchStatus1D processAICcBandwidth(double bwmin, double bwmax,
                                               unsigned nsteps, double startBw,
                                               double startingFactor,
                                               bool useEntropicNDoF = true);

        /** Last bandwidth processed */
        inline double lastBandwidth() const {return bandwidth_;}

        /**
        // Change the vector of observations. Array size must be compatible
        // with that given in the constructor. The pointer to the covariance
        // matrix of observations can be NULL. If provided, this matrix
        // should be symmetric and positive-definite.
        */
        virtual void setObservedData(const double* observed, unsigned len,
                                     const Matrix<double>* observCovariance);

        /**
        // Set the bias (this is useful for various studies of uncertainties).
        // If set, this bias will be added to the smoothed oracle data. The
        // length must be compatible with the length of the unfolded result.
        */
        virtual void setBias(const double* unfoldingBias, unsigned len);

        /** Return the bias data provided by the last "setBias" call */
        inline const std::vector<double>& getBias() const
            {return biasData_;}

        /** Clear bias data */
        inline virtual void clearBias() {biasData_.clear();}

        /** Length of the observed data vector */
        inline unsigned observedSize() const {return observed_.size();}

        /** Length of the unfolded data vector */
        inline unsigned unfoldedSize() const {return unfoldedVec_.size();}

        /** Status of the last unfolding call ("true" means success) */
        inline virtual bool lastUnfoldingStatus() const
            {return unfoldingStatus_;}

        /** Last unfolded distribution */
        inline const std::vector<double>& unfoldedResult() const
            {return unfoldedVec_;}

        /** Last unfolded covariance matrix */
        inline const Matrix<double>& unfoldedCovariance() const
            {return unfoldedCovmat_;}

        /** Response matrix */
        inline const Matrix<double>& responseMatrix() const
            {return unfold_.responseMatrix();}

        /**
        // Oracle data. Empty vector is returned if the oracle
        // was not provided in the constructor.
        */
        inline const std::vector<double>& getOracleData() const 
            {return oracle_;}

        /**
        // Return oracle data smoothed with the last processed bandwidth.
        // If the bias was set, it was added to this data.
        */
        inline const std::vector<double>& smoothedOracleData() const 
            {return smoothedOracleVec_;}

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
        inline void setInitialApproximation(const double* approx,
                                            unsigned lenApprox)
            {unfold_.setInitialApproximation(approx, lenApprox);}

        /** Clear the initial approximation to the unfolded solution */
        inline void clearInitialApproximation()
            {unfold_.clearInitialApproximation();}

        /** Return the initial approximation to the unfolded solution */
        inline const std::vector<double>& getInitialApproximation() const
            {return unfold_.getInitialApproximation();}

        /** Correction factor for the number of degrees of freedom */
        inline double nDoFCorrectionFactor() const {return nDoFCorr_;}
        
        /** Set the correction factor for the number of degrees of freedom */
        inline void setNDoFCorrectionFactor(const double f) {nDoFCorr_ = f;}

        //@{
        /** Function forwarded from AbsUnfold1D */
        inline std::pair<double, double> smoothingNDoF() const
            {return unfold_.smoothingNDoF();}
        inline std::pair<double, double> responseNDoF() const
            {return unfold_.responseNDoF();}
        inline std::pair<double, double> smoothedResponseNDoF() const
            {return unfold_.smoothedResponseNDoF();}
        //@}

        /** Add a number of names to the given vector of strings */
        static void addNamesWithPrefix(const char* prefix, unsigned count,
                                       std::vector<std::string>* names);
    protected:
        // Return "true" on success
        virtual bool performUnfolding();

        // Variables set by the constructor
        AbsUnfold1D& unfold_;
        std::vector<double> observed_;
        std::vector<double> oracle_;
        double maxLOrPEDegree_;
        double xmin_;
        double xmax_;
        double binwidth_;
        double nDoFCorr_;
        double nObserved_;
        double obsNonZeroFraction_;
        BoundaryHandling bm_;
        std::vector<const AbsBinnedComparison1D*> foldedComparators_;
        std::vector<const AbsBinnedComparison1D*> oracleComparators_;
        int symbetaPower_;
        Matrix<double> observationCovariance_;
        Matrix<double> observationErr_;

        // Bandwidth is set by the "process" method
        double bandwidth_;

        // useEntropicNDoFinAICc from the "processAICcBandwidth" method
        bool useEntropicNDoFinAICc_;

        // Unfolding results (calculated by the "performUnfolding" method)
        std::vector<double> unfoldedVec_;
        Matrix<double> unfoldedCovmat_;
        std::vector<double> smoothedOracleVec_;
        std::vector<double> eigenDeltas_;
        std::vector<double> covEigenValues_;

    private:
        friend class Private::UnfoldingBandwidthScanner1DHelper;

        // Disable copy constructor an assignment operator
        UnfoldingBandwidthScanner1D();
        UnfoldingBandwidthScanner1D(const UnfoldingBandwidthScanner1D&);
        UnfoldingBandwidthScanner1D& operator=(
            const UnfoldingBandwidthScanner1D&);

        void smoothTheOracle(const double* oracle, double* smoothed) const;

        void getModelNDoF(double* ndof1, double* ndof2, double* ndof3, double bw);

        double getAICc(const double bw);

        double foldedLogLikelihood(const double* fitted,
                                   const double* counts,
                                   unsigned long len);

        static double getNonZeroFraction(const std::vector<double>&);

        // Bias
        std::vector<double> biasData_;

        // Variables calculated by the "performUnfolding" method
        std::vector<double> foldedDistances_;
        std::vector<double> foldedPValues_;
        std::vector<double> oracleDistances_;
        std::vector<double> oraclePValues_;
        std::vector<double> smoothedOracleDistances_;
        std::vector<double> smoothedOraclePValues_;

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

        double filterNDoFEntropic_;
        double filterNDoFTrace_;
        double foldingNDoFEntropic_;
        double foldingNDoFTrace_;
        double unfoldedNDoFEntropic_;
        double unfoldedNDoFTrace_;
        double modelNDoFEntropic_;
        double modelNDoFTrace_;
        double modelNDoF3_;

        double AICcEntropic_;
        double AICcTrace_;
        double AICc3_;

        double smoothingNormfactor_;
        double integratedVariance_;
        unsigned nIterations_;
        bool unfoldingStatus_;

        // Variable set by "variableNames" call
        mutable unsigned nVariables_;

        const LocalPolyFilter1D* oldFilter_;

        MemoizingSymbetaFilterProvider filterProvider_;
        std::map<double,Triple<double,double,double> > ndofMap_;
        CPP11_shared_ptr<const LocalPolyFilter1D> lastFilter_;

        std::vector<double> memBuf_, deltaBuf_;
    };
}

#endif // NPSTAT_UNFOLDINGBANDWIDTHSCANNER1D_HH_
