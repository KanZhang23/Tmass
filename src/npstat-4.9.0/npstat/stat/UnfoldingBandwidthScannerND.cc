#include <cfloat>
#include <climits>
#include <stdexcept>

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/discretizedDistance.hh"

#include "npstat/stat/UnfoldingBandwidthScannerND.hh"
#include "npstat/stat/UnfoldingBandwidthScanner1D.hh"
#include "npstat/stat/SmoothedEMUnfoldND.hh"
#include "npstat/stat/SequentialPolyFilterND.hh"
#include "npstat/stat/StatUtils.hh"
#include "npstat/stat/arrayStats.hh"

inline static void printDoubleVec(std::ostream& os,
                                  const std::vector<double>& v)
{
    os << '{';
    const unsigned sz = v.size();
    for (unsigned i=0; i<sz; ++i)
    {
        if (i) os << ", ";
        os << v[i];
    }
    os << '}';
}

namespace npstat {
    UnfoldingBandwidthScannerND::UnfoldingBandwidthScannerND(
        AbsUnfoldND& i_unfold,
        const std::vector<SymbetaParams1D>& i_filterParameters,
        const ArrayND<double>& i_observed,
        const double i_nDoFCorrectionFactor,
        const Matrix<double>* i_observationCovariance,
        const ArrayND<double>* i_oracle)
        : unfold_(i_unfold),
          filterParameters_(i_filterParameters),
          observed_(i_observed),
          binVolume_(1.0),
          nDoFCorr_(i_nDoFCorrectionFactor),
          nObserved_(observed_.sum<long double>()),
          obsNonZeroFraction_(getNonZeroFraction(observed_)),
          //
          bandwidthValues_(filterParameters_.size(), -2.0),
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
          modelNDoFEntropic_(-1.0),
          modelNDoFTrace_(-1.0),
          AICcEntropic_(0.0),
          AICcTrace_(0.0),
          //
          smoothingNormfactor_(0.0),
          integratedVariance_(-1.0),
          nIterations_(0),
          unfoldingStatus_(false),
          //
          nVariables_(0),
          oldFilter_(unfold_.getFilter()),
          localFilter_(0),
          localFilterBuf_(filterParameters_.size())
    {
        const unsigned dim = filterParameters_.size();
        if (unfold_.responseMatrix().rank() != dim)
            throw std::invalid_argument(
                "In npstat::UnfoldingBandwidthScannerND constructor: "
                "incompatible number of filter parameters");
        unfold_.validateObservedShape(observed_);

        for (unsigned idim=0; idim<dim; ++idim)
            binVolume_ *= filterParameters_[idim].binWidth();

        if (i_observationCovariance)
        {
            const unsigned nrows = i_observationCovariance->nRows();
            const unsigned ncols = i_observationCovariance->nColumns();
            const unsigned long len = observed_.length();
            if (len != nrows || len != ncols) throw std::invalid_argument(
                "In npstat::UnfoldingBandwidthScannerND constructor: "
                "incompatible dimensions of the input covariance matrix");
            if (nrows*1UL*ncols > UINT_MAX) throw std::invalid_argument(
                "In npstat::UnfoldingBandwidthScannerND constructor: "
                "covariance matrix of observations is too large");
            observationCovariance_ = *i_observationCovariance;
        }

        if (i_oracle)
        {
            unfold_.validateUnfoldedShape(*i_oracle);
            oracle_ = *i_oracle;
        }
    }

    UnfoldingBandwidthScannerND::~UnfoldingBandwidthScannerND()
    {
        unfold_.setFilter(oldFilter_);
        delete localFilter_;
    }

    double UnfoldingBandwidthScannerND::getNonZeroFraction(
        const ArrayND<double>& a)
    {
        double f = 1.0;
        const unsigned long sz = a.length();
        if (sz)
        {
            unsigned long cnt = 0;
            const double* data = a.data();
            for (unsigned long i=0; i<sz; ++i)
                if (*data++)
                    ++cnt;
            f = cnt*1.0/sz;
        }
        return f;
    }

    bool UnfoldingBandwidthScannerND::process(
        const std::vector<double>& bandwidthValues)
    {
        const unsigned dim = filterParameters_.size();
        if (bandwidthValues.size() != dim) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScannerND::process: "
            "incompatible number of bandwidth values");
        for (unsigned i=0; i<dim; ++i)
            if (bandwidthValues[i] < 0.0) throw std::invalid_argument(
                "In npstat::UnfoldingBandwidthScannerND::process: "
                "bandwidth values must not be negative");
        if (nObserved_ <= 0.0) throw std::runtime_error(
            "In npstat::UnfoldingBandwidthScannerND::process: "
            "insufficient number of observed events");

        bandwidthValues_ = bandwidthValues;
        for (unsigned i=0; i<dim; ++i)
        {
            const SymbetaParams1D& p(filterParameters_[i]);
            localFilterBuf_[i] = filterProvider_.provideFilter(
                p.symbetaPower(), bandwidthValues_[i], p.maxDegree(),
                unfold_.responseMatrix().span(i), p.binWidth(),
                p.boundaryMethod(), UINT_MAX, false).get();
        }

        CPP11_auto_ptr<const SequentialPolyFilterND> sfilter(
            new SequentialPolyFilterND(&localFilterBuf_[0], dim, false));
        delete localFilter_; localFilter_ = 0;
        localFilter_ = new UnfoldingFilterND<SequentialPolyFilterND>(
            sfilter.get(), true);
        sfilter.release();
        unfold_.setFilter(localFilter_);

        return performUnfolding();
    }

    void UnfoldingBandwidthScannerND::setBias(const ArrayND<double>& bias)
    {
        unfold_.validateUnfoldedShape(bias);
        bias_ = bias;
    }

    void UnfoldingBandwidthScannerND::setObservedData(
        const ArrayND<double>& observed,
        const Matrix<double>* i_observationCovariance)
    {
        unfold_.validateObservedShape(observed);

        const double dsum = observed.sum<long double>();
        if (dsum <= 0.0) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScannerND::setObservedData: "
            "input array sum is not positive");

        if (i_observationCovariance)
        {
            const unsigned nrows = i_observationCovariance->nRows();
            const unsigned ncols = i_observationCovariance->nColumns();
            const unsigned long len = observed_.length();
            if (len != nrows || len != ncols) throw std::invalid_argument(
                "In npstat::UnfoldingBandwidthScannerND::setObservedData: "
                "incompatible dimensions of the input covariance matrix");
            observationCovariance_ = *i_observationCovariance;
        }
        else
            observationCovariance_.uninitialize();
        observationErr_.uninitialize();

        observed_ = observed;
        nObserved_ = dsum;
        obsNonZeroFraction_ = getNonZeroFraction(observed_);
    }

    bool UnfoldingBandwidthScannerND::performUnfolding()
    {
        const bool oracleExists = oracle_.isShapeKnown();

        if (oracleExists)
        {
            // Smooth the oracle
            const long double sum = oracle_.sum<long double>();
            const AbsUnfoldingFilterND* filter = unfold_.getFilter(true);
            smoothedOracle_.reshape(oracle_);
            if (unfold_.usingConvolutions())
                filter->convolve(oracle_, &smoothedOracle_);
            else
                filter->filter(oracle_, &smoothedOracle_);
            normalizeArrayAsDensity(const_cast<double*>(smoothedOracle_.data()),
                                    smoothedOracle_.length(), 1.0L/sum);
            if (bias_.isShapeKnown())
                smoothedOracle_ += bias_;
            smoothedOracleSum_ = smoothedOracle_.sum<long double>();
        }

        // Figure out the number of degrees of freedom for the model
        // in the folded space
        getModelNDoF(&modelNDoFEntropic_, &modelNDoFTrace_, bandwidthValues_);

        // Perform the unfolding
        unfoldingStatus_ = unfold_.unfold(
            observed_,
            observationCovariance_.nRows() ? &observationCovariance_ : 0,
            &unfolded_, &unfoldedCovmat_);
        unfoldedSum_ = unfolded_.sum<long double>();
        integratedVariance_ = unfoldedCovmat_.tr()*binVolume_;

        SmoothedEMUnfoldND* dago = 
            dynamic_cast<SmoothedEMUnfoldND*>(&unfold_);
        if (dago)
        {
            nIterations_ = dago->lastNIterations();
            smoothingNormfactor_ = dago->lastSmoothingNormfactor();
        }

        // Fitted distribution in the folded space
        const ResponseMatrix& K = unfold_.responseMatrix();
        K.timesVector(unfolded_, &folded_);
        foldedSum_ = folded_.sum<long double>();

        // Calculate fit likelihood in the folded space
        foldedLogli_ = foldedLogLikelihood(folded_, observed_);

        // Calculate folded model AICc
        AICcEntropic_ = aicc(modelNDoFEntropic_, foldedLogli_, nObserved_);
        AICcTrace_ = aicc(modelNDoFTrace_, foldedLogli_, nObserved_);

        // Calculate various oracle-dependent quantities
        if (oracleExists)
        {
            const unsigned long lenUnfolded = unfolded_.length();
            const double* unfolded = unfolded_.data();
            const double* oracle = oracle_.data();
            const double* smoothedOracle = smoothedOracle_.data();

            // Likelihood in the unfolded space
            unfoldedLogli_ = poissonLogLikelihood(
                oracle, unfolded, lenUnfolded);
            smoothedUnfoldedLogli_ = poissonLogLikelihood(
                smoothedOracle, unfolded, lenUnfolded);

            // Unfolded ISE
            unfoldedISE_ = discretizedL2(oracle, lenUnfolded, unfolded,
                                         lenUnfolded, binVolume_, true);
            smoothedUnfoldedISE_ = discretizedL2(smoothedOracle, lenUnfolded,
                                                 unfolded, lenUnfolded,
                                                 binVolume_, true);
            // Unfolded diagonal "chi-square"
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

            // Eigenvector comparison
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

        return unfoldingStatus_;
    }

    void UnfoldingBandwidthScannerND::getModelNDoF(
        double* ndof1, double* ndof2,
        const std::vector<double>& bwValues)
    {
        std::pair<double,double> n;
        std::map<std::vector<double>, std::pair<double,double> >::iterator it =
            ndofMap_.find(bwValues);
        if (it == ndofMap_.end())
        {
            {
                ArrayND<double> uniform(unfold_.responseMatrix().observedShape());
                uniform.constFill(1.0);
                if (!unfold_.unfold(uniform, 0, &unfolded_, &unfoldedCovmat_))
                {
                    std::ostringstream os;
                    os << "In npstat::UnfoldingBandwidthScannerND::getModelNDoF:"
                       << " unfolding failed for bandwidth ";
                    printDoubleVec(os, bwValues);
                    throw std::runtime_error(os.str());
                }
            }
            const Matrix<double>& K(unfold_.responseMatrix().denseMatrix());
            const Matrix<double>& cmat = K*unfoldedCovmat_*K.T();
            n.first = cmat.symPSDefEffectiveRank(
                0.0, EIGEN_D_AND_C, &n.second);
            ndofMap_.insert(std::make_pair(bwValues, n));
        }
        else
            n = it->second;

        const double corr = nDoFCorr_ > 0.0 ? nDoFCorr_ : obsNonZeroFraction_;
        *ndof1 = n.first * corr;
        *ndof2 = n.second * corr;
    }

    double UnfoldingBandwidthScannerND::foldedLogLikelihood(
        const ArrayND<double>& fitted, const ArrayND<double>& counts)
    {
        const unsigned long len = fitted.length();
        assert(len == counts.length());
        const double* pcounts = counts.data();
        const double* pfitted = fitted.data();

        if (observationCovariance_.nRows())
        {
            if (!observationErr_.nRows())
                observationErr_ = observationCovariance_.symPDInv();
            if (deltaBuf_.size() < len)
                deltaBuf_.resize(len);
            double* delta = &deltaBuf_[0];
            for (unsigned long i=0; i<len; ++i)
                delta[i] = pcounts[i] - pfitted[i];
            return -0.5*observationErr_.bilinear(delta, len);
        }
        else
            return poissonLogLikelihood(pfitted, pcounts, len);
    }

    std::vector<std::string> UnfoldingBandwidthScannerND::variableNames() const
    {
        static const char* simpleNames[] = {
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

            "modelNDoFEntropic",
            "modelNDoFTrace",
            "AICcEntropic",
            "AICcTrace",

            "smoothingNormfactor",
            "integratedVariance",
            "nIterations",
            "unfoldingStatus"
        };

        std::vector<std::string> names;

        const unsigned dim = filterParameters_.size();
        names.reserve(dim + sizeof(simpleNames)/sizeof(simpleNames[0]));
        UnfoldingBandwidthScanner1D::addNamesWithPrefix(
            "bandwidth_", dim, &names);

        for (unsigned i=0; i<sizeof(simpleNames)/sizeof(simpleNames[0]); ++i)
            names.push_back(std::string(simpleNames[i]));

        nVariables_ = names.size();
        return names;
    }

    unsigned UnfoldingBandwidthScannerND::variableCount() const
    {
        if (!nVariables_)
            UnfoldingBandwidthScannerND::variableNames();
        return nVariables_;
    }

    unsigned UnfoldingBandwidthScannerND::ntuplize(
        double* buf, const unsigned len) const
    {
        if (!nVariables_)
            UnfoldingBandwidthScannerND::variableNames();
        if (len < nVariables_) throw std::invalid_argument(
            "In npstat::UnfoldingBandwidthScannerND::ntuplize: "
            "insufficient buffer length");
        assert(buf);

        unsigned i = 0;

        copyBuffer(buf + i, &bandwidthValues_[0], bandwidthValues_.size());
        i += bandwidthValues_.size();

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

        buf[i++] = modelNDoFEntropic_;
        buf[i++] = modelNDoFTrace_;
        buf[i++] = AICcEntropic_;
        buf[i++] = AICcTrace_;

        buf[i++] = smoothingNormfactor_;
        buf[i++] = integratedVariance_;
        buf[i++] = nIterations_;
        buf[i++] = unfoldingStatus_;

        assert(i == nVariables_);
        return i;
    }
}
