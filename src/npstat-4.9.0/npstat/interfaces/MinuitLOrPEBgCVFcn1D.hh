#ifndef NPSI_MINUITLORPEBGCVFCN1D_HH_
#define NPSI_MINUITLORPEBGCVFCN1D_HH_

/*!
// \file MinuitLOrPEBgCVFcn1D.hh
//
// \brief Fit of LOrPE bandwidth and maximum poly degree in the mixed
//        signal/background model
//
// Author: I. Volobouev
//
// October 2013
*/

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>

#include "npstat/stat/lorpeBackground1D.hh"
#include "npstat/interfaces/MinuitSemiparametricFitUtils.hh"

#include "Minuit2/FCNBase.h"

namespace npsi {
    template <class Numeric, class NumIn=double>
    class MinuitLOrPEBgCVFcn1D : public ROOT::Minuit2::FCNBase
    {
    public:
        /**
        // Most arguments of this constructor are the same as the arguments
        // of the npstat::lorpeBackground1D function. See the description
        // of that function for more detail. The two arguments not of this
        // kind are:
        //
        //   up      -- This parameter regulates Minuit convergence.
        //              See Minuit manual for details.
        //
        //   minlog  -- Infrequently, LOrPE creates a density estimate
        //              which is exactly 0 for some histogram bin which
        //              is not empty; the value of this parameter limits
        //              the contribution of such bins into the overall
        //              likelihood.
        //
        // An internal copy of "initialApproximation" will be made
        // if such an approximation is provided (initialApproximation
        // can also be specified as NULL to use the uniform background
        // density as a starting point for iterations).
        //
        // This class will not assume ownership of any references
        // or pointers
        */
        MinuitLOrPEBgCVFcn1D(const npstat::HistoND<Numeric>& histo,
                             npstat::AbsSymbetaFilterProvider& fbuilder,
                             const npstat::AbsDistribution1D& signal,
                             const double signalFraction,
                             const unsigned nIntegrationPoints,
                             const NumIn* initialApproximation,
                             const unsigned lenApproximation,
                             const int symmetricBetaPower,
                             const double minimumBgFeatureSize,
                             const double effectiveNumBgEvents,
                             const double convergenceEpsilon,
                             const unsigned maxIterations,
                             const npstat::BoundaryHandling& bm,
                             const bool useLeastSquaresCV,
                             const bool verbose,
                             const bool bandwidthIsAbsolute,
                             const unsigned cvmode,
                             const double regularizationParameter,
                             const double minlog,
                             const double up)
            : histo_(histo),
              fbuilder_(fbuilder),
              signal_(signal),
              initialApproximation_(initialApproximation),
              signalFraction_(signalFraction),
              minlog_(minlog),
              up_(up),
              convergenceEpsilon_(convergenceEpsilon),
              regularizationParameter_(regularizationParameter),
              minimumBgFeatureSize_(minimumBgFeatureSize),
              effectiveNumBgEvents_(effectiveNumBgEvents),
              callCount_(0),
              lenApproximation_(lenApproximation),
              nIntegrationPoints_(nIntegrationPoints),
              maxIterations_(maxIterations),
              cvmode_(cvmode),
              m_(symmetricBetaPower),
              bm_(bm),
              verbose_(verbose),
              bandwidthIsAbsolute_(bandwidthIsAbsolute),
              useLeastSquaresCV_(useLeastSquaresCV)
        {
            densityBuffer_.resize(3*histo_.nBins());
        }

        inline virtual ~MinuitLOrPEBgCVFcn1D() {}

        inline unsigned long callCount() const {return callCount_;}

        inline npstat::AbsSymbetaFilterProvider& getFilterProvider() const
            {return fbuilder_;}

        inline double getActualBandwidth(const double bwParameter,
                                         const double maxDegree) const 
        {
            double bandwidth = bwParameter;
            if (!bandwidthIsAbsolute_)
            {
                // The bandwidth parameter is actually the logarithm
                // of the ratio between the actual bandwidth and the
                // minimum acceptable bandwidth
                const double minbw = minHistoBandwidth1D(
                    histo_, minimumBgFeatureSize_, maxDegree,
                    effectiveNumBgEvents_, m_);
                bandwidth = minbw*exp(bwParameter);
            }
            return bandwidth;
        }

        /**
        // This method returns either the negative log likelihood or
        // the cross validation approximation of MISE.
        //
        // It is assumed that bandwidth is the first parameter while
        // degree of the LOrPE polynomial is the second.
        */
        virtual double operator()(const std::vector<double>& x) const
        {
            if (x.size() < 2U) throw std::invalid_argument(
                "In npsi::MinuitLOrPEBgCVFcn1D::operator() : "
                "at least two parameters needed");

            const double maxDegree = x[1];
            const double bandwidth = getActualBandwidth(x[0], maxDegree);

            const unsigned nbins = histo_.nBins();
            double* signalDensity = &densityBuffer_[0];
            double* bgDensity = signalDensity + nbins;
            double* densityMinusOne = bgDensity + nbins;

            double convergenceDelta = 0.0;
            const unsigned niter = npstat::lorpeBackground1D(
                    histo_, fbuilder_, bm_, signal_, signalFraction_,
                    nIntegrationPoints_,
                    initialApproximation_, lenApproximation_,
                    m_, bandwidth, maxDegree,
                    convergenceEpsilon_, maxIterations_,
                    signalDensity, nbins, bgDensity, nbins,
                    workspace_, densityMinusOne, nbins,
                    cvmode_, regularizationParameter_, &convergenceDelta);

            if (niter >= maxIterations_) 
            {
                std::ostringstream os;
                os << "In npsi::MinuitLOrPEBgCVFcn1D::operator() : "
                   << "background determination algorithm failed to converge.";
                if (fabs(convergenceDelta) > convergenceEpsilon_)
                    os << " Requested divergence <= " << convergenceEpsilon_
                       << ", achieved only " << convergenceDelta << '.';
                throw std::runtime_error(os.str());
            }

            double cv = 0.0;
            if (useLeastSquaresCV_)
                cv = npstat::lorpeBgCVLeastSquares1D(
                    histo_, signalFraction_,
                    signalDensity, nbins, bgDensity, nbins,
                    densityMinusOne, nbins);
            else
                cv = -npstat::lorpeBgCVPseudoLogli1D(
                    histo_, signalFraction_,
                    signalDensity, nbins, bgDensity, nbins,
                    densityMinusOne, nbins, minlog_);

            if (verbose_)
            {
                std::cout << "MinuitLOrPEBgCVFcn1D " << callCount_
                          << ", bw " << bandwidth
                          << ", deg " << maxDegree;
                if (useLeastSquaresCV_)
                    std::cout << ", lscv " << cv;
                else
                    std::cout << ", pll " << -cv;
                std::cout << '\n';
                std::cout.flush();
            }

            ++callCount_;
            return cv;
        }

        inline double Up() const {return up_;}

    private:
        mutable std::vector<double> densityBuffer_;
        mutable std::vector<double> workspace_;

        const npstat::HistoND<Numeric>& histo_;
        npstat::AbsSymbetaFilterProvider& fbuilder_;
        const npstat::AbsDistribution1D& signal_;
        const NumIn* initialApproximation_;

        double signalFraction_;
        double minlog_;
        double up_;
        double convergenceEpsilon_;
        double regularizationParameter_;
        double minimumBgFeatureSize_;
        double effectiveNumBgEvents_;

        mutable unsigned long callCount_;
        unsigned lenApproximation_;
        unsigned nIntegrationPoints_;
        unsigned maxIterations_;
        unsigned cvmode_;
        int m_;

        npstat::BoundaryHandling bm_;

        bool verbose_;
        bool bandwidthIsAbsolute_;
        bool useLeastSquaresCV_;
    };
}

#endif // NPSI_MINUITLORPEBGCVFCN1D_HH_
