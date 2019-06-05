#include <numeric>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <cfloat>

#include "npstat/nm/allocators.hh"
#include "npstat/nm/discretizedDistance.hh"
#include "npstat/nm/EquidistantSequence.hh"
#include "npstat/nm/goldenSectionSearch.hh"
#include "npstat/nm/DualAxis.hh"
#include "npstat/nm/MathUtils.hh"

#include "npstat/stat/UnfoldingBandwidthScanner1D.hh"
#include "npstat/stat/arrayStats.hh"
#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/AbsBinnedComparison1D.hh"
#include "npstat/stat/SmoothedEMUnfold1D.hh"
#include "npstat/stat/AbsNtuple.hh"

namespace npstat {
    namespace Private {
        class UnfoldingBandwidthScanner1DHelper : public Functor1<double,double>
        {
        public:
            inline explicit UnfoldingBandwidthScanner1DHelper(
                UnfoldingBandwidthScanner1D* scan)
                : scan_(scan) {assert(scan_);}

            inline double operator()(const double& bw) const
                {return scan_->getAICc(bw);}

        private:
            UnfoldingBandwidthScanner1D* scan_;
        };
    }
}

namespace npstat {
    UnfoldingBandwidthScanner1D::UnfoldingBandwidthScanner1D(
        AbsUnfold1D& unfold,
        const double* observed, const unsigned lenObserved,
        const Matrix<double>* observationCovariance,
        const double* oracle, const unsigned lenOracle,
        const int symbetaPower, const double maxLOrPEDegree,
        const double xMinUnfolded, const double xMaxUnfolded,
        const BoundaryHandling& filterBoundaryMethod,
        const double i_nDoFCorrectionFactor,
        const std::vector<const AbsBinnedComparison1D*>& foldedComparators,
        const std::vector<const AbsBinnedComparison1D*>& oracleComparators)
        : unfold_(unfold),
          observed_(observed, observed+lenObserved),
          oracle_(),
          maxLOrPEDegree_(maxLOrPEDegree),
          xmin_(xMinUnfolded),
          xmax_(xMaxUnfolded),
          binwidth_(0.0),
          nDoFCorr_(i_nDoFCorrectionFactor),
          nObserved_(std::accumulate(observed, observed+lenObserved, 0.0L)),
          obsNonZeroFraction_(getNonZeroFraction(observed_)),
          bm_(filterBoundaryMethod),
          foldedComparators_(foldedComparators),
          oracleComparators_(oracleComparators),
          symbetaPower_(symbetaPower),
          //
          bandwidth_(-2.0),
          useEntropicNDoFinAICc_(false),
          //
          unfoldedVec_(unfold_.responseMatrix().nColumns(), 0.0),
          unfoldedCovmat_(),
          foldedDistances_(foldedComparators.size(), -1.0),
          foldedPValues_(foldedComparators.size(), -1.0),
          oracleDistances_(oracleComparators.size(), -1.0),
          oraclePValues_(oracleComparators.size(), -1.0),
          smoothedOracleDistances_(oracleComparators.size(), -1.0),
          smoothedOraclePValues_(oracleComparators.size(), -1.0),
          //
          foldedSum_(-1.0),
          unfoldedSum_(-1.0),
          smoothedOracleSum_(-1.0),
          foldedLogli_(0.0),
          //
          unfoldedLogli_(0.0),
          unfoldedISE_(-1.0),
          unfoldedDiagChisq_(-1.0),
          //
          smoothedUnfoldedLogli_(0.0),
          smoothedUnfoldedISE_(-1.0),
          smoothedUnfoldedDiagChisq_(-1.0),
          //
          filterNDoFEntropic_(-1.0),
          filterNDoFTrace_(-1.0),
          foldingNDoFEntropic_(-1.0),
          foldingNDoFTrace_(-1.0),
          unfoldedNDoFEntropic_(-1.0),
          unfoldedNDoFTrace_(-1.0),
          modelNDoFEntropic_(-1.0),
          modelNDoFTrace_(-1.0),
          modelNDoF3_(-1.0),
          //
          AICcEntropic_(0.0),
          AICcTrace_(0.0),
          AICc3_(0.0),
          //
          smoothingNormfactor_(0.0),
          integratedVariance_(-1.0),
          nIterations_(0),
          unfoldingStatus_(false),
          //
          nVariables_(0),
          oldFilter_(unfold_.getFilter())
    {
        const unsigned lenUnfolded = unfoldedVec_.size();
        binwidth_ = (xMaxUnfolded - xMinUnfolded)/lenUnfolded;

        if (!(observed && lenObserved && lenUnfolded && maxLOrPEDegree >= 0.0 &&
              binwidth_ > 0.0)) throw std::invalid_argument(
                  "In npstat::UnfoldingBandwidthScanner1D constructor: "
                  "invalid arguments");

        if (observationCovariance)
        {
            if (observationCovariance->nRows() != lenObserved ||
                observationCovariance->nColumns() != lenObserved)
                throw std::invalid_argument(
                    "In npstat::UnfoldingBandwidthScanner1D constructor: "
                    "incompatible dimensions for the covariance "
                    "matrix of observations");
            observationCovariance_ = *observationCovariance;
        }

        const unsigned long nFoldedComp = foldedComparators_.size();
        for (unsigned long icomp=0; icomp<nFoldedComp; ++icomp)
            assert(foldedComparators_[icomp]);

        const unsigned long nOracleComp = oracleComparators_.size();
        for (unsigned long icomp=0; icomp<nOracleComp; ++icomp)
            assert(oracleComparators_[icomp]);

        if (lenOracle)
        {
            if (lenUnfolded != lenOracle) throw std::invalid_argument(
                "In npstat::UnfoldingBandwidthScanner1D constructor: "
                "incompatible length of the oracle array");
            assert(oracle);
            oracle_.resize(lenOracle);
            copyBuffer(&oracle_[0], oracle, lenOracle);
        }
    }

    UnfoldingBandwidthScanner1D::~UnfoldingBandwidthScanner1D()
    {
        unfold_.setFilter(oldFilter_);
    }

    double UnfoldingBandwidthScanner1D::getNonZeroFraction(
        const std::vector<double>& v)
    {
        double f = 1.0;
        const unsigned long sz = v.size();
        if (sz)
        {
            unsigned long cnt = 0;
            const double* data = &v[0];
            for (unsigned long i=0; i<sz; ++i)
                if (*data++)
                    ++cnt;
            f = cnt*1.0/sz;
        }
        return f;
    }

    bool UnfoldingBandwidthScanner1D::process(const double bandwidth)
    {
        if (bandwidth < 0.0) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::process: "
            "bandwidth must not be negative");
        if (nObserved_ <= 0.0) throw std::runtime_error(
            "In npstat::UnfoldingBandwidthScanner1D::process: "
            "insufficient number of observed events");

        bandwidth_ = bandwidth;
        lastFilter_ = filterProvider_.provideFilter(
            symbetaPower_, bandwidth_, maxLOrPEDegree_,
            unfoldedVec_.size(), binwidth_, bm_, UINT_MAX, false);
        unfold_.setFilter(lastFilter_.get());

        return performUnfolding();
    }

    void UnfoldingBandwidthScanner1D::setBias(
        const double* unfoldingBias, const unsigned lenUnfolded)
    {
        if (lenUnfolded != unfoldedVec_.size()) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::setBias: "
            "incompatible input length");
        assert(unfoldingBias);
        if (biasData_.size() != lenUnfolded)
            biasData_.resize(lenUnfolded);
        copyBuffer(&biasData_[0], unfoldingBias, lenUnfolded);
    }

    void UnfoldingBandwidthScanner1D::setObservedData(
        const double* observed, const unsigned len,
        const Matrix<double>* observationCovariance)
    {
        if (len != observed_.size()) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::setObservedData: "
            "incompatible input array length");

        assert(observed);
        const double dsum = std::accumulate(observed, observed+len, 0.0L);
        if (dsum <= 0.0) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::setObservedData: "
            "input array sum is not positive");

        if (observationCovariance)
        {
            if (observationCovariance->nRows() != len ||
                observationCovariance->nColumns() != len)
                throw std::invalid_argument(
                    "In npstat::UnfoldingBandwidthScanner1D::setObservedData: "
                    "incompatible dimensions for the covariance "
                    "matrix of observations");
            observationCovariance_ = *observationCovariance;
        }
        else
            observationCovariance_.uninitialize();
        observationErr_.uninitialize();

        copyBuffer(&observed_[0], observed, len);
        nObserved_ = dsum;
        obsNonZeroFraction_ = getNonZeroFraction(observed_);
    }

    MinSearchStatus1D UnfoldingBandwidthScanner1D::processAICcBandwidth(
        const double bwmin, const double bwmax, const unsigned nsteps,
        const double startBw, const double startingFactor,
        const bool useEntropicNDoF)
    {
        if (startBw <= 0.0) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::processAICcBandwidth: "
            "initial bandwidth must be positive");
        if (nsteps < 3U) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::processAICcBandwidth: "
            "insufficient number of grid cells");
        if (nObserved_ <= 0.0) throw std::runtime_error(
            "In npstat::UnfoldingBandwidthScanner1D::processAICcBandwidth: "
            "insufficient number of observed events");

        useEntropicNDoFinAICc_ = useEntropicNDoF;
        EquidistantInLogSpace grid(bwmin, bwmax, nsteps);
        DualAxis axis(grid, true);
        unsigned i0 = axis.getInterval(startBw).first;
        unsigned delta = 1U;
        if (startingFactor > 1.0)
        {
            const unsigned i1 = axis.getInterval(startBw*startingFactor).first;
            delta = i1 - i0;
        }
        double fm1 = 0., f0 = 0., fp1 = 0.;
        Private::UnfoldingBandwidthScanner1DHelper helper(this);
        MinSearchStatus1D status = goldenSectionSearchOnAGrid(
            helper, axis, i0, delta, &i0, &fm1, &f0, &fp1);

        double bw = i0 < nsteps ? axis.coordinate(i0) : -1.0;
        if (status == MIN_SEARCH_OK)
        {
            const double x1 = axis.coordinate(i0 - 1U);
            const double x2 = axis.coordinate(i0);
            const double x3 = axis.coordinate(i0 + 1U);
            double aic = 0.;
            if (!parabolicExtremum(x1, fm1, x2, f0, x3, fp1, &bw, &aic))
                throw std::runtime_error(
                    "In npstat::UnfoldingBandwidthScanner1D::processAICcBandwidth:"
                    " failed to find the parabolic extremum");
        }

        if (bw < 0.0)
        {
            // "goldenSectionSearchOnAGrid" found a maximum.
            // This is not absolutely impossible far away from the minimum.
            // Perform the complete scan. This should be very infrequent.
            std::vector<double> fullScan(nsteps);
            unsigned minpos = nsteps;
            double minval = DBL_MAX;
            for (unsigned iscan=0; iscan<nsteps; ++iscan)
            {
                fullScan[iscan] = helper(axis.coordinate(iscan));
                if (fullScan[iscan] < minval)
                {
                    minval = fullScan[iscan];
                    minpos = iscan;
                }
            }
            if (minpos < nsteps)
            {
                if (minpos == 0U)
                {
                    bw = axis.coordinate(minpos);
                    status = MIN_ON_LEFT_EDGE;
                }
                else if (minpos == nsteps - 1U)
                {
                    bw = axis.coordinate(minpos);
                    status = MIN_ON_RIGHT_EDGE;
                }
                else
                {
                    const double x1 = axis.coordinate(minpos - 1U);
                    const double x2 = axis.coordinate(minpos);
                    const double x3 = axis.coordinate(minpos + 1U);
                    double aic = 0.;
                    if (!parabolicExtremum(x1, fullScan[minpos - 1U],
                                           x2, fullScan[minpos],
                                           x3, fullScan[minpos + 1U],
                                           &bw, &aic))
                        throw std::runtime_error(
                            "In npstat::UnfoldingBandwidthScanner1D::processAICcBandwidth:"
                            " no parabolic extremum found by the complete bandwidth scan");
                    status = MIN_SEARCH_OK;
                }
            }
        }

        if (bw >= 0.0)
        {
            const bool ism = filterProvider_.isMemoizing();
            filterProvider_.stopMemoizing();
            process(bw);
            if (ism)
                filterProvider_.startMemoizing();
        }
        else
        {
            status = MIN_SEARCH_FAILED;
            unfoldingStatus_ = false;
            bandwidth_ = -1.0;
        }

        return status;
    }

    double UnfoldingBandwidthScanner1D::getAICc(const double bandwidth)
    {
        const double* observed = &observed_[0];
        const unsigned lenObs = observed_.size();
        double* unfolded = &unfoldedVec_[0];
        const unsigned lenUnfolded = unfoldedVec_.size();
        const Matrix<double>& K(unfold_.responseMatrix());

        lastFilter_ = filterProvider_.provideFilter(
            symbetaPower_, bandwidth, maxLOrPEDegree_,
            unfoldedVec_.size(), binwidth_, bm_, UINT_MAX, false);
        unfold_.setFilter(lastFilter_.get());

        double ndof1 = 0., ndof2 = 0., ndof3 = 0.;
        getModelNDoF(&ndof1, &ndof2, &ndof3, bandwidth);
        // const double ndof = useEntropicNDoFinAICc_ ? ndof1 : ndof2;
        const double ndof = useEntropicNDoFinAICc_ ? ndof1 : ndof3;

        unfold_.unfold(observed, lenObs, 0, unfolded, lenUnfolded, 0);
        if (memBuf_.size() < lenObs)
            memBuf_.resize(lenObs);
        double* folded = &memBuf_[0];
        K.timesVector(unfolded, lenUnfolded, folded, lenObs);
        const double logli = foldedLogLikelihood(folded, observed, lenObs);

        return aicc(ndof, logli, nObserved_);
    }

    bool UnfoldingBandwidthScanner1D::performUnfolding()
    {
        // Useful variables
        double* unfolded = &unfoldedVec_[0];
        const unsigned lenUnfolded = unfoldedVec_.size();
        const double* observed = &observed_[0];
        const unsigned lenObserved = observed_.size();
        const double* oracle = oracle_.empty() ? (double*)0 : &oracle_[0];
        const Matrix<double>& K(unfold_.responseMatrix());

        // Smooth the oracle
        const double* smoothedOracle = 0;
        if (oracle)
        {
            if (smoothedOracleVec_.size() != lenUnfolded)
                smoothedOracleVec_.resize(lenUnfolded);
            smoothTheOracle(oracle, &smoothedOracleVec_[0]);
            smoothedOracle = &smoothedOracleVec_[0];
            if (!biasData_.empty())
            {
                double* sm = &smoothedOracleVec_[0];
                const double* bias = &biasData_[0];
                for (unsigned i=0; i<lenUnfolded; ++i)
                    sm[i] += bias[i];
            }
            smoothedOracleSum_ = std::accumulate(
                smoothedOracle, smoothedOracle+lenUnfolded, 0.0L);
        }

        // Figure out the number of degrees of freedom for the model
        // in the folded space
        getModelNDoF(&modelNDoFEntropic_, &modelNDoFTrace_, &modelNDoF3_, bandwidth_);

        // Perform the unfolding
        unfoldingStatus_ = unfold_.unfold(
            observed, lenObserved,
            observationCovariance_.nRows() ? &observationCovariance_ : 0,
            unfolded, lenUnfolded, &unfoldedCovmat_);
        unfoldedSum_ = std::accumulate(unfolded, unfolded+lenUnfolded, 0.0L);
        integratedVariance_ = unfoldedCovmat_.tr()*binwidth_;

        SmoothedEMUnfold1D* dago = 
            dynamic_cast<SmoothedEMUnfold1D*>(&unfold_);
        if (dago)
        {
            nIterations_ = dago->lastNIterations();
            smoothingNormfactor_ = dago->lastSmoothingNormfactor();
        }

        // Fitted distribution in the folded space
        if (memBuf_.size() < lenObserved)
            memBuf_.resize(lenObserved);
        double* folded = &memBuf_[0];
        K.timesVector(unfolded, lenUnfolded, folded, lenObserved);
        foldedSum_ = std::accumulate(folded, folded+lenObserved, 0.0L);

        // Run comparisons in the folded space
        const unsigned nFoldedComp = foldedComparators_.size();
        for (unsigned icomp=0; icomp<nFoldedComp; ++icomp)
            foldedComparators_[icomp]->compare(
                observed, folded, lenObserved,
                &foldedDistances_[icomp], &foldedPValues_[icomp]);

        // Run comparisons in the unfolded space
        if (oracle)
        {
            const unsigned nOracleComp = oracleComparators_.size();
            for (unsigned long icomp=0; icomp<nOracleComp; ++icomp)
            {
                oracleComparators_[icomp]->compare(
                    unfolded, oracle, lenUnfolded,
                    &oracleDistances_[icomp], &oraclePValues_[icomp]);
                oracleComparators_[icomp]->compare(
                    unfolded, smoothedOracle, lenUnfolded,
                    &smoothedOracleDistances_[icomp],
                    &smoothedOraclePValues_[icomp]);
            }
        }

        // Calculate fit likelihood in the folded space
        foldedLogli_ = foldedLogLikelihood(folded, observed, lenObserved);

        // Calculate folded model AICc
        AICcEntropic_ = aicc(modelNDoFEntropic_, foldedLogli_, nObserved_);
        AICcTrace_ = aicc(modelNDoFTrace_, foldedLogli_, nObserved_);
        AICc3_ = aicc(modelNDoF3_, foldedLogli_, nObserved_);

        // Calculate likelihood in the unfolded space
        if (oracle)
        {
            unfoldedLogli_ = poissonLogLikelihood(
                oracle, unfolded, lenUnfolded);
            smoothedUnfoldedLogli_ = poissonLogLikelihood(
                smoothedOracle, unfolded, lenUnfolded);
        }

        // Calculate unfolded ISE
        if (oracle)
        {
            unfoldedISE_ = discretizedL2(oracle, lenUnfolded, unfolded,
                                         lenUnfolded, binwidth_, true);
            smoothedUnfoldedISE_ = discretizedL2(smoothedOracle, lenUnfolded,
                                      unfolded, lenUnfolded, binwidth_, true);
        }

        // Unfolded diagonal "chi-square"
        if (oracle)
        {
            long double dsum = 0.0L, dsum2 = 0.0L;
            for (unsigned i=0; i<lenUnfolded; ++i)
            {
                const double d1 = oracle[i] - unfolded[i];
                const double d2 = smoothedOracle[i] - unfolded[i];
                const double sigmaSq = unfoldedCovmat_[i][i];
                dsum += d1*d1/sigmaSq;
                dsum2 += d2*d2/sigmaSq;
            }
            unfoldedDiagChisq_ = dsum;
            smoothedUnfoldedDiagChisq_ = dsum2;
        }

        // Eigenvector comparison
        if (oracle)
        {
            if (covEigenValues_.size() != lenUnfolded)
            {
                covEigenValues_.resize(lenUnfolded);
                eigenDeltas_.resize(lenUnfolded);
            }

            Matrix<double> eigenvec;
            unfoldedCovmat_.symEigen(&covEigenValues_[0], lenUnfolded,
                                     &eigenvec, EIGEN_D_AND_C);

            double* delta = &eigenDeltas_[0];
            for (unsigned i=0; i<lenUnfolded; ++i)
                delta[i] = unfolded[i] - smoothedOracle[i];

            const Matrix<double>& edeltaMat = 
                eigenvec.timesVector(delta, lenUnfolded);
            const double* edelta = edeltaMat.data();

            unsigned ev = 0;
            for (long i=lenUnfolded-1; i>=0; --i, ++ev)
            {
                if (covEigenValues_[i] > 0.0)
                {
                    delta[ev] = edelta[i]/sqrt(covEigenValues_[i]);
                    if (delta[ev] > FLT_MAX)
                        delta[ev] = FLT_MAX;
                    else if (delta[ev] < -FLT_MAX)
                        delta[ev] = -FLT_MAX;
                }
                else
                    delta[ev] = FLT_MAX;
            }

            std::sort(covEigenValues_.begin(), covEigenValues_.end(),
                      std::greater<double>());
        }

        // Various numbers of degrees of freedom
        std::pair<double,double> ndof = unfold_.smoothingNDoF();
        filterNDoFEntropic_ = ndof.first;
        filterNDoFTrace_ = ndof.second;
        ndof = unfold_.smoothedResponseNDoF();
        foldingNDoFEntropic_ = ndof.first;
        foldingNDoFTrace_ = ndof.second;
        unfoldedNDoFEntropic_ = unfoldedCovmat_.symPSDefEffectiveRank(
            0.0, EIGEN_D_AND_C, &unfoldedNDoFTrace_);

        return unfoldingStatus_;
    }

    std::vector<std::string> UnfoldingBandwidthScanner1D::variableNames() const
    {
        static const char* simpleNames[] = {
            "bandwidth",
            "foldedSum",
            "unfoldedSum",
            "smoothedOracleSum",
            "foldedLogli",

            "unfoldedLogli",
            "unfoldedISE",
            "unfoldedDiagChisq",

            "smoothedUnfoldedLogli",
            "smoothedUnfoldedISE",
            "smoothedUnfoldedDiagChisq",

            "filterNDoFEntropic",
            "filterNDoFTrace",
            "foldingNDoFEntropic",
            "foldingNDoFTrace",
            "unfoldedNDoFEntropic",
            "unfoldedNDoFTrace",
            "modelNDoFEntropic",
            "modelNDoFTrace",
            "modelNDoF3",

            "AICcEntropic",
            "AICcTrace",
            "AICc3",

            "smoothingNormfactor",
            "integratedVariance",
            "nIterations",
            "unfoldingStatus"
        };

        std::vector<std::string> names(ntupleColumns(
             simpleNames, sizeof(simpleNames)/sizeof(simpleNames[0])));

        const unsigned nFoldedComp = foldedComparators_.size();
        addNamesWithPrefix("foldedDistance_", nFoldedComp, &names);
        addNamesWithPrefix("foldedPValue_", nFoldedComp, &names);

        const unsigned nOracleComp = oracleComparators_.size();
        addNamesWithPrefix("oracleDistance_", nOracleComp, &names);
        addNamesWithPrefix("oraclePValue_", nOracleComp, &names);
        addNamesWithPrefix("smoothedOracleDistance_", nOracleComp, &names);
        addNamesWithPrefix("smoothedOraclePValue_", nOracleComp, &names);

        nVariables_ = names.size();
        return names;
    }

    void UnfoldingBandwidthScanner1D::addNamesWithPrefix(
        const char* prefix, const unsigned count,
        std::vector<std::string>* names)
    {
        for (unsigned i=0; i<count; ++i)
        {
            std::ostringstream os;
            os << prefix << i;
            names->push_back(os.str());
        }
    }

    unsigned UnfoldingBandwidthScanner1D::variableCount() const
    {
        if (!nVariables_)
            UnfoldingBandwidthScanner1D::variableNames();
        return nVariables_;
    }

    unsigned UnfoldingBandwidthScanner1D::ntuplize(
        double* buf, const unsigned len) const
    {
        if (!nVariables_)
            UnfoldingBandwidthScanner1D::variableNames();
        if (len < nVariables_) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScanner1D::ntuplize: "
            "insufficient buffer length");
        assert(buf);

        unsigned i = 0;
        buf[i++] = bandwidth_;
        buf[i++] = foldedSum_;
        buf[i++] = unfoldedSum_;
        buf[i++] = smoothedOracleSum_;
        buf[i++] = foldedLogli_;

        buf[i++] = unfoldedLogli_;
        buf[i++] = unfoldedISE_;
        buf[i++] = unfoldedDiagChisq_;

        buf[i++] = smoothedUnfoldedLogli_;
        buf[i++] = smoothedUnfoldedISE_;
        buf[i++] = smoothedUnfoldedDiagChisq_;

        buf[i++] = filterNDoFEntropic_;
        buf[i++] = filterNDoFTrace_;
        buf[i++] = foldingNDoFEntropic_;
        buf[i++] = foldingNDoFTrace_;
        buf[i++] = unfoldedNDoFEntropic_;
        buf[i++] = unfoldedNDoFTrace_;
        buf[i++] = modelNDoFEntropic_;
        buf[i++] = modelNDoFTrace_;
        buf[i++] = modelNDoF3_;

        buf[i++] = AICcEntropic_;
        buf[i++] = AICcTrace_;
        buf[i++] = AICc3_;

        buf[i++] = smoothingNormfactor_;
        buf[i++] = integratedVariance_;
        buf[i++] = nIterations_;
        buf[i++] = unfoldingStatus_;

        copyBuffer(buf + i, &foldedDistances_[0], foldedDistances_.size());
        i += foldedDistances_.size();

        copyBuffer(buf + i, &foldedPValues_[0], foldedPValues_.size());
        i += foldedPValues_.size();

        copyBuffer(buf + i, &oracleDistances_[0], oracleDistances_.size());
        i += oracleDistances_.size();

        copyBuffer(buf + i, &oraclePValues_[0], oraclePValues_.size());
        i += oraclePValues_.size();

        copyBuffer(buf + i, &smoothedOracleDistances_[0],
                   smoothedOracleDistances_.size());
        i += smoothedOracleDistances_.size();

        copyBuffer(buf + i, &smoothedOraclePValues_[0],
                   smoothedOraclePValues_.size());
        i += smoothedOraclePValues_.size();

        assert(i == nVariables_);
        return i;
    }

    void UnfoldingBandwidthScanner1D::smoothTheOracle(
        const double* oracle, double* smoothed) const
    {
        const unsigned lenUnfolded = unfoldedVec_.size();
        const double sum = std::accumulate(oracle, oracle+lenUnfolded, 0.0L);
        const LocalPolyFilter1D* filter = unfold_.getFilter(true);
        if (unfold_.usingConvolutions())
            filter->convolve(oracle, lenUnfolded, smoothed);
        else
            filter->filter(oracle, lenUnfolded, smoothed);
        normalizeArrayAsDensity(smoothed, lenUnfolded, 1.0/sum);
    }

    void UnfoldingBandwidthScanner1D::getModelNDoF(double* ndof1, double* ndof2,
                                                   double* ndof3, const double bw)
    {
        Triple<double,double,double> n;
        std::map<double,Triple<double,double,double> >::iterator it =
            ndofMap_.find(bw);
        if (it == ndofMap_.end())
        {
            const Matrix<double>& K(unfold_.responseMatrix());
            {
                const unsigned lenObserved = K.nRows();
                const unsigned lenUnfolded = unfoldedVec_.size();
                std::vector<double> uniform(lenObserved, 1.0);
                if (!unfold_.unfold(&uniform[0], lenObserved, 0,
                                    &unfoldedVec_[0], lenUnfolded,
                                    &unfoldedCovmat_))
                {
                    std::ostringstream os;
                    os << "In npstat::UnfoldingBandwidthScanner1D::getModelNDoF:"
                       << " unfolding failed for bandwidth " << bw;
                    throw std::runtime_error(os.str());
                }
            }
            const Matrix<double>& KT(unfold_.transposedResponseMatrix());
            const Matrix<double>& cmat = K*unfoldedCovmat_*KT;
            n.first = cmat.symPSDefEffectiveRank(
                0.0, EIGEN_D_AND_C, &n.second);
            n.third = cmat.tr();
            ndofMap_.insert(std::make_pair(bw, n));
        }
        else
            n = it->second;

        const double corr = nDoFCorr_ > 0.0 ? nDoFCorr_ : obsNonZeroFraction_;
        *ndof1 = n.first * corr;
        *ndof2 = n.second * corr;        
        *ndof3 = n.third * corr;        
    }

    double UnfoldingBandwidthScanner1D::foldedLogLikelihood(
        const double* fitted, const double* counts, const unsigned long len)
    {
        if (observationCovariance_.nRows())
        {
            if (!observationErr_.nRows())
                observationErr_ = observationCovariance_.symPDInv();
            if (deltaBuf_.size() < len)
                deltaBuf_.resize(len);
            double* delta = &deltaBuf_[0];
            for (unsigned long i=0; i<len; ++i)
                delta[i] = counts[i] - fitted[i];
            return -0.5*observationErr_.bilinear(delta, len);
        }
        else
            return poissonLogLikelihood(fitted, counts, len);
    }
}
