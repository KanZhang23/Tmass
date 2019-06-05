#ifndef NPSI_MINUITSEMIPARAMETRICFITFCN1D_HH_
#define NPSI_MINUITSEMIPARAMETRICFITFCN1D_HH_

/*!
// \file MinuitSemiparametricFitFcn1D.hh
//
// \brief Fit of a density by maximum likelihood using parametric signal
//        model and nonparametric background
//
// Author: I. Volobouev
//
// October 2013
*/

#include <cmath>
#include <cfloat>
#include <iostream>

#include "npstat/interfaces/MinuitLOrPEBgCVFcn1D.hh"

#include "Minuit2/MnUserParameters.h"
#include "Minuit2/MnMinimize.h"
#include "Minuit2/FunctionMinimum.h"

namespace npsi {
    /**
    // Target minimization function adapter class for running maximum
    // likelihood density fits to histogrammed data by Minuit2 in which
    // background is estimated by LOrPE and signal by a parametric model.
    //
    // "Numeric" is the type of fitted histogram contents.
    //
    // "DensityConstructor" is a functor which creates the necessary
    // signal density function out of a vector of parameters.
    // Must have "operator()(const std::vector<double>&) const"
    // which returns an object (or a reference) of some class which
    // was derived from AbsDistribution1D. "ScalableDensityConstructor1D"
    // is an example of such a constructor class, appropriate in case
    // only the signal fraction, location and scale are to be fitted
    // but not the signal shape.
    //
    // "NumIn" is the type of the array elements used to provide initial
    // guess for the background density.
    //
    // Note the special order of Minuit parameters which must be used
    // with this class. See the comments to "operator()" for more detail.
    */
    template <typename Numeric, class DensityConstructor, typename NumIn=double>
    class MinuitSemiparametricFitFcn1D : public ROOT::Minuit2::FCNBase
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        //  histo  -- Naturally, the histogram to fit. It is assumed that the
        //            histogram bins are not scaled and contain the actual
        //            unweighted event counts.
        //
        //  densityMaker -- This object will generate AbsDistribution1D objects
        //            for the signal model from the vector of signal parameters.
        //
        //  fb     -- "Filter builder". This object will generate local
        //            polynomial filters using densities from the symmetric
        //            beta family as weights. A reference implementation
        //            of such a class is provided with the NPStat software:
        //            "SimpleSymbetaFilterProvider". It may be possible to
        //            construct more efficient implementations which will
        //            reuse filters constructed for different bandwidth values.
        //
        //  bm     -- Method for handling LOrPE weight function at the
        //            boundaries of density support region.
        //
        //  symmetricBetaPower -- This parameter determines the choice of
        //            the kernel. If it is negative, Gaussian kernel will
        //            be used, truncated at +- 12 sigma. If it is positive
        //            or 0, the kernel from the symmetric beta family will
        //            be used, proportional to (1 - x^2)^symmetricBetaPower.
        //            In my experience, most useful values of this parameter
        //            are -1 and 4.
        //
        //  minimumBgFeatureSize -- Minimum feature size (like sigma of the
        //            Gaussian) of the background distribution that the
        //            nonparametric component of the fit should attempt to
        //            represent in full detail. Together with other things
        //            such as the number of events, this parameter will affect
        //            the smallest LOrPE bandwidth considered in the fit.
        //
        //  initialApproximation -- Initial approximation for the background
        //            density (one array element per input histogram bin).
        //            Can be specified as NULL in which case the uniform
        //            density will be used as the initial approximation.
        //
        //  lenApproximation -- Length of the initial approximation array.
        //            If not 0 (and if initialApproximation is not NULL),
        //            the code will verify that it equals the number of
        //            histogram bins.
        //
        //  polyDegreeLimit -- The maximum polynomial degree to consider
        //            during cross validation (will be used as a limiting
        //            value of the corresponding MIGRAD parameter). Due to the
        //            round-off errors in calculating orthogonal polynomials,
        //            this number should not exceed 20 or so. The default
        //            value of -1 (and any other negative number) means that
        //            a reasonable limit estimate will be made automatically.
        //
        //  verbose -- If "true", some info will be printed on the standard
        //             output while the fit is in progress.
        //
        //  cvmode -- Mode for cross validation calculations. See comments
        //            to the "lorpeBackground1D" function for more detail
        //            on the meaning of this argument.
        //
        //  useLeastSquaresCV -- "true" means use least squares cross
        //            validation. "false" means use pseudo likelihood.
        //
        //  refitBandwidth -- If "true", the code will refit the bandwidth
        //            of the background model by cross validation for every
        //            value of signal parameters. In this case the user should
        //            fix the bandwidth parameter in the fit (it will be used
        //            as the initial guess only).
        //
        //  refitPolyDegree -- If "true", the code will refit the polynomial
        //            degree of the background model by cross validation
        //            for every value of signal parameters. In this case
        //            the user should fix the polynomial degree parameter
        //            in the fit (it will be used as the initial guess only).
        //
        //  regularizationParameter -- If this parameter is non-negative,
        //            the code will attempt to figure out the minimum
        //            reasonable value of the density estimate in cases
        //            that density is estimated to be zero at some point where
        //            data is present. This minimum density will be inversely
        //            proportional to pow(N, regularizationParameter).
        //            This will be applied during the likelihood calculation
        //            and in cross validation (limited to pseudo-likelihood
        //            CV, not done for least squares CV).
        //
        //  nIntegrationPoints -- How many points to use in order to integrate
        //            the parametric signal density across each histogram bin.
        //            If this argument is specified as 0 then the difference
        //            of cumulative densities at the bin edges will be used,
        //            otherwise Gauss-Legendre quadrature will be employed
        //            (so that the number of points must be either 1 or one
        //            of the numbers supported by the "GaussLegendreQuadrature"
        //            class). For properly implemented densities, 0 should be
        //            the best option.
        //
        //  bandwidthUpperLimitFactor -- This parameter determines the maximum
        //            kernel bandwidth which will be considered during cross
        //            validation. For Gaussian kernels, the maximum sigma
        //            will be set to the width of the histogram times this
        //            factor. For other kernels the maximum width will be
        //            adjusted accordingly (technically, the additional
        //            adjustment factor is determined by the "canonical
        //            bandwidth" ratio evaluated for the initial degree
        //            of the model polynomial).
        //
        //  pseudoFitCycles -- Sometimes, when the initial guess of the
        //            bandwidth and polynomial degree is far away from
        //            optimal, Minuit has problems navigating towards the
        //            minimum in this 2-d space (the "narrow valley" problem).
        //            In this case it may be helful to make a number of
        //            steps at the beginning optimizing each variable in turn
        //            while keeping the other variable fixed. This parameter
        //            determines how many pairs of such initial steps to make
        //            during cross validation.
        //
        //  convergenceEpsilon -- The background density is refitted
        //            iteratively by the relevant algorithm until convergence
        //            is achieved. The convergence criterion is that the L1
        //            distance between the background distributions obtained
        //            in two successive iterations is less than this number.
        //            Must be non-negative. A reasonable value will be
        //            selected automatically in case default value of
        //            0.0 is given.
        //
        //  pseudoFitPrecision -- The Minuit working precision for maximizing
        //            the cross validation pseudo likelihood. Should be
        //            higher than the convergenceEpsilon by about an order
        //            of magnitude or so. Note that you will need to adjust
        //            precision of _this_ fit as well. A reasonable value of
        //            "pseudoFitPrecision" will be selected automatically
        //            in case default value of 0.0 is given.
        //
        //  bandwidthScanPoints -- If Minuit fails to optimize the quantity
        //            used in cross validation, the optimum will be found by
        //            simply scanning the bandwidth and the polynomial degree.
        //            This parameter determines the number of bandwidth points
        //            to scan.
        //
        //  minlog -- This parameter limits the contribution of non-empty
        //            histogram bins into the log-likelihood in case the
        //            density was estimated to be 0 for that bin.
        //
        //  up     -- The Minuit "up" parameter which affects the definition
        //            of uncertainties. See the Minuit manual for details.
        //
        //  upRefit -- The "up" parameter to use in the cross validation step.
        //
        //  maxIterations -- The hard limit on the number of iterations that
        //            can be used for background determination. If convergence
        //            is not achieved after this number of iterations,
        //            an exception will be thrown.
        //
        // This class will not assume ownership of any references. However,
        // an internal copy will be made of the "initialApproximation" array
        // if this array is not NULL.
        */
        MinuitSemiparametricFitFcn1D(const npstat::HistoND<Numeric>& histo,
                                     const DensityConstructor& densityMaker,
                                     npstat::AbsSymbetaFilterProvider& fb,
                                     const npstat::BoundaryHandling& bm,
                                     const int symmetricBetaPower,
                                     const double minimumBgFeatureSize,
                                     const NumIn* initialApproximation = 0,
                                     const unsigned lenApproximation = 0U,
                                     const int polyDegreeLimit = -1,
                                     const bool verbose = true,
                                     const unsigned cvmode = 
                                             npstat::CV_MODE_LINEARIZED,
                                     const bool useLeastSquaresCV = true,
                                     const bool refitBandwidth = true,
                                     const bool refitPolyDegree = true,
                                     const double regularizationParameter = -1.0,
                                     const unsigned nIntegrationPoints = 0U,
                                     const double bandwidthUpperLimitFactor=5.0,
                                     const unsigned pseudoFitCycles = 0U,
                                     const double convergenceEpsilon = 0.0,
                                     const double pseudoFitPrecision = 0.0,
                                     const unsigned bandwidthScanPoints = 64U,
                                     const double minlog = log(DBL_MIN),
                                     const double up = 0.5,
                                     const double upRefit = 0.05,
                                     const unsigned maxIterations = 10000U)
            : signalDensityBuffer_(histo.nBins(), 0.0),
              bgDensityBuffer_(histo.nBins(), 0.0),
              histo_(histo),
              fbuilder_(fb),
              densityMaker_(densityMaker),
              minimumBgFeatureSize_(minimumBgFeatureSize),
              convergenceEpsilon_(convergenceEpsilon),
              pseudoFitPrecision_(pseudoFitPrecision),
              bandwidth_(0.0),
              maxDegree_(-1.0),
              signalFraction_(0.0),
              minlog_(minlog),
              up_(up),
              upNonparametric_(upRefit),
              regularizationParameter_(regularizationParameter),
              bandwidthLimitFactor_(bandwidthUpperLimitFactor),
              maxBgEventsInWindow_(-1.0),
              callCount_(0),
              nIntegrationPoints_(nIntegrationPoints),
              maxIterations_(maxIterations),
              nFitCycles_(pseudoFitCycles),
              polyDegreeLimit_(polyDegreeLimit),
              cvmode_(cvmode),
              bandwidthScanPoints_(bandwidthScanPoints),
              m_(symmetricBetaPower),
              numRegularized_(0U),
              bm_(bm),
              refitBandwidth_(refitBandwidth),
              refitPolyDegree_(refitPolyDegree),
              verbose_(verbose),
              useLeastSquaresCV_(useLeastSquaresCV)
        {
            if (histo_.dim() != 1U) throw std::invalid_argument(
                "In npsi::MinuitSemiparametricFitFcn1D constructor : "
                "only 1-d histograms can be used with this class");

            if (initialApproximation && lenApproximation)
            {
                initialApproximation_.reserve(lenApproximation);
                for (unsigned i=0; i<lenApproximation; ++i)
                    initialApproximation_.push_back(initialApproximation[i]);
            }

            if (convergenceEpsilon_ == 0.0)
                convergenceEpsilon_ = 10.0*DBL_EPSILON*histo.nBins();
            if (pseudoFitPrecision_ == 0.0)
                pseudoFitPrecision_ = 10.0*convergenceEpsilon_;

            if (polyDegreeLimit >= 0)
                if (static_cast<unsigned>(polyDegreeLimit) > 
                    npstat::maxFilterDegreeSupported())
                    throw std::invalid_argument(
                        "In npsi::MinuitSemiparametricFitFcn1D constructor: "
                        "polynomial degree limit outside of supported range");

            if (minimumBgFeatureSize_ < 0.0)
                throw std::invalid_argument(
                    "In npsi::MinuitSemiparametricFitFcn1D constructor: "
                    "minimum feature size can not be negative");

            nEvents_ = histo_.binContents().template sum<long double>();
        }

        inline virtual ~MinuitSemiparametricFitFcn1D() {}

        inline double Up() const {return up_;}

        inline double lastBandwidthUsed() const {return bandwidth_;}

        inline double lastPolyDegreeUsed() const {return maxDegree_;}

        /**
        // The number of bins for which the background density was
        // actually adjusted during the most recent run
        */
        inline unsigned lastRegularized() const {return numRegularized_;}

        inline double lastSignalFractionUsed() const {return signalFraction_;}

        inline const std::vector<double>& lastBackgroundFitted() const
            {return bgDensityBuffer_;}

        inline const std::vector<double>& lastSignalFitted() const
            {return signalDensityBuffer_;}

        inline unsigned long callCount() const {return callCount_;}

        inline double crossValidationPrecision() const
            {return pseudoFitPrecision_;}

        inline int symbetaPower() const {return m_;}

        inline double maxBgEventsInGaussWindow() const
            {return maxBgEventsInWindow_;}

        inline npstat::AbsSymbetaFilterProvider& getFilterProvider() const
            {return fbuilder_;}

        inline std::vector<double> lastDensityFitted() const
        {
            const double bgfrac = 1.0 - signalFraction_;
            const unsigned nbins = histo_.nBins();
            std::vector<double> dvec(nbins);
            for (unsigned i=0; i<nbins; ++i)
                dvec[i] = signalFraction_*signalDensityBuffer_[i] +
                          bgfrac*bgDensityBuffer_[i];
            return dvec;
        }

        /**
        // This method returns the negative log likelihood.
        //
        // Parameters must be given in a specific order:
        //
        //   x[0] -- Value of the LOrPE bandwidth. Should be fixed
        //           in the fit if "refitBandwidth" was set "true"
        //           in the constructor.
        //
        //   x[1] -- Degree of the LOrPE polynomial. Should be fixed
        //           in the fit if "refitPolyDegree" was set "true"
        //           in the constructor.
        //
        //   x[2] -- Signal fraction. Can be either fixed or fitted.
        //
        //   x[3...] -- These parameters will be passed to the
        //              density maker for building the signal density.
        */
        virtual double operator()(const std::vector<double>& x) const
        {
            const unsigned npara = x.size();
            if (npara < 3U) throw std::invalid_argument(
                "In npsi::MinuitSemiparametricFitFcn1D::operator(): "
                "insufficient number of parameters, need at least 3");

            bandwidth_ = x[0];
            maxDegree_ = x[1];
            signalFraction_ = x[2];

            if (!refitBandwidth_ && bandwidth_ <= 0.0)
                throw std::invalid_argument(
                    "In npsi::MinuitSemiparametricFitFcn1D::operator(): "
                    "if not refitted, the bandwidth must be positive");

            if (!refitPolyDegree_ && maxDegree_ < 0.0)
                throw std::invalid_argument(
                    "In npsi::MinuitSemiparametricFitFcn1D::operator(): "
                    "if not refitted, polynomial degree can not be negative");

            if (signalParameters_.size() != npara - 3U)
                signalParameters_.resize(npara - 3U);
            for (unsigned i=3U; i<npara; ++i)
                signalParameters_[i - 3U] = x[i];
            const npstat::AbsDistribution1D& signal = 
                densityMaker_(signalParameters_);

            const unsigned lenApprox = initialApproximation_.size();
            const NumIn* initApprox = 0;
            if (lenApprox)
                initApprox = &initialApproximation_[0];

            // Estimate the max number of bg events in a window
            // of relevant size. We will later use this number
            // for things like determining the maximum degree
            // of the LOrPE polynomial.
            //
            // Note that this calculation is performed only once,
            // using the initial signal settings. If we redo this
            // calculation every time signal parameters change,
            // memoizing the filters becomes hopeless.
            //
            double winBgEvents = 1.0;
            if (refitBandwidth_)
            {
                if (maxBgEventsInWindow_ <= 0.0)
                {
                    maxBgEventsInWindow_ = 1.0;
                    const double oneSigmaCoverage = 0.682689492137085897;
                    const double nbg2 = npstat::maxBgPointsInWindow1D(
                        histo_, signal, signalFraction_, nIntegrationPoints_,
                        2.0*minimumBgFeatureSize_, initApprox, lenApprox,
                        workspace_)/oneSigmaCoverage;
                    if (nbg2 > maxBgEventsInWindow_)
                        maxBgEventsInWindow_ = nbg2;
                }
                winBgEvents = maxBgEventsInWindow_;
            }

            if (refitBandwidth_ || refitPolyDegree_)
            {
                // Figure out optimal LOrPE parameters by cross validation
                const unsigned beVerboseFor = 3U;
                const bool verboseRefit = verbose_ && callCount_ < beVerboseFor;
                if (verbose_ && beVerboseFor && callCount_ == beVerboseFor)
                    std::cout << "**** MinuitLOrPEBgCVFcn1D appears to work "
                              << "fine and will shut up now" << std::endl;

                MinuitLOrPEBgCVFcn1D<Numeric,NumIn> bgFcn(
                    histo_, fbuilder_, signal, signalFraction_,
                    nIntegrationPoints_, initApprox, lenApprox,
                    m_, minimumBgFeatureSize_, winBgEvents,
                    convergenceEpsilon_, maxIterations_,
                    bm_, useLeastSquaresCV_,
                    verboseRefit, !refitBandwidth_, cvmode_,
                    regularizationParameter_, minlog_, upNonparametric_);

                // If the initial values of bandwidth and/or poly degree
                // are negative (also 0 in case of the bandwidth) we must
                // find a good initial approximation by scanning
                const bool scanBandwidth = refitBandwidth_ && bandwidth_ <= 0.0;
                const bool scanPolyDeg = refitPolyDegree_ && maxDegree_ < 0.0;
                double bwstep = 0.0, degstep = 0.0;

                if (scanBandwidth && scanPolyDeg)
                    scanBandwidthAndDegree(bgFcn, bandwidthScanPoints_,
                                           winBgEvents, &bandwidth_, &bwstep,
                                           &maxDegree_, &degstep);
                else if (scanBandwidth)
                    scanBandwidthOnly(bgFcn, bandwidthScanPoints_,
                                      winBgEvents, maxDegree_,
                                      &bandwidth_, &bwstep);
                else if (scanPolyDeg)
                    scanPolyDegOnly(bgFcn, winBgEvents,
                                    bandwidth_, &maxDegree_, &degstep);

                if (!refitBandwidthAndDegree(bgFcn, nFitCycles_, winBgEvents,
                                             !refitBandwidth_, !refitPolyDegree_,
                                             &bandwidth_, bwstep,
                                             &maxDegree_, degstep))
                {
                    // The Minuit fit has to converge if we are looking
                    // into just 1-d optimization. If it doesn't converge,
                    // something must be really wrong with the problem setup.
                    if (!(refitBandwidth_ && refitPolyDegree_))
                        throw std::runtime_error(
                            "In npsi::MinuitSemiparametricFitFcn1D::operator()"
                            ": Minuit2 failed to optimize cross-validation "
                            "bandwidth or poly degree");

                    if (!(scanBandwidth && scanPolyDeg))
                    {
                        // Try to determine a better starting point by 
                        // scanning the bandwidth and the degree in wide
                        // ranges of possible values
                        scanBandwidthAndDegree(bgFcn, bandwidthScanPoints_,
                                               winBgEvents,
                                               &bandwidth_, &bwstep,
                                               &maxDegree_, &degstep);

                        // Refit again using the new starting point.
                        // If this fit fails again then just leave
                        // the bandwidth and degree parameters at their
                        // values determined by the scan.
                        refitBandwidthAndDegree(bgFcn, nFitCycles_, winBgEvents,
                                                !refitBandwidth_,
                                                !refitPolyDegree_,
                                                &bandwidth_, bwstep,
                                                &maxDegree_, degstep);
                    }
                }
            }

            const unsigned niter = npstat::lorpeBackground1D(
                histo_, fbuilder_, bm_, signal, signalFraction_,
                nIntegrationPoints_, initApprox, lenApprox, m_,
                bandwidth_, maxDegree_,
                convergenceEpsilon_, maxIterations_,
                &signalDensityBuffer_[0], signalDensityBuffer_.size(),
                &bgDensityBuffer_[0], bgDensityBuffer_.size(), workspace_);

            if (niter >= maxIterations_) throw std::runtime_error(
                "In npsi::MinuitSemiparametricFitFcn1D::operator(): "
                "background determination algorithm failed to converge");

            if (regularizationParameter_ >= 0.0)
            {
                const double nBg = nEvents_*(1.0 - signalFraction_);
                const double minBgDensity1 = npstat::symbetaWeightAt0(
                    m_, bandwidth_)/nBg/std::pow(nBg, regularizationParameter_);
                numRegularized_ = npstat::lorpeRegularizeBgDensity1D(
                    histo_, signalFraction_,
                    &signalDensityBuffer_[0], signalDensityBuffer_.size(),
                    &bgDensityBuffer_[0], bgDensityBuffer_.size(),
                    minBgDensity1);
            }

            const double logli = npstat::lorpeBgLogli1D(
                histo_, signalFraction_,
                &signalDensityBuffer_[0], signalDensityBuffer_.size(),
                &bgDensityBuffer_[0], bgDensityBuffer_.size(), minlog_);

            if (verbose_)
            {
                std::cout << "C " << callCount_
                          << ", bw " << bandwidth_
                          << ", deg " << maxDegree_
                          << ", sf " << signalFraction_;
                const npstat::AbsScalableDistribution1D* s =
                    dynamic_cast<const npstat::AbsScalableDistribution1D*>(
                        &signal);
                if (s)
                    std::cout << ", m " << s->location()
                              << ", s " << s->scale();
                std::cout << ", ll " << logli;
                if (regularizationParameter_ >= 0.0)
                    std::cout << ", adj " << numRegularized_;
                std::cout << '\n';
                std::cout.flush();
            }

            ++callCount_;
            return -logli;
        }

    protected:
        // Get the maximum bandwidth according to the limit provided
        // in the constructor
        inline double estimateMaxBandwidth() const
        {
            const double bwfactor = npstat::symbetaBandwidthRatio(m_, 0);
            const npstat::HistoAxis& axis = histo_.axis(0);
            return bwfactor*(axis.max() - axis.min())*bandwidthLimitFactor_;
        }

        //
        // Get a reasonable estimate for the maximum degree of the LOrPE
        // filter from the plugin bandwidth theory
        //
        inline unsigned maxFilterDegree1D(const double nbg) const
        {
            if (polyDegreeLimit_ >= 0)
                return polyDegreeLimit_;

            unsigned lim = m_ >= 0 ? npstat::amisePluginDegreeSymbeta(m_,nbg) :
                                     npstat::amisePluginDegreeGauss(nbg);
            lim += (lim/2U + 1U);
            if (lim < 3U)
                lim = 3U;
            else if (lim > npstat::maxFilterDegreeSupported())
                lim = npstat::maxFilterDegreeSupported();
            return lim;
        }

    private:
        void scanPolyDegOnly(const MinuitLOrPEBgCVFcn1D<Numeric,NumIn>& bgFcn,
                             const double effBgEvents, const double bw,
                             double* bestDegree, double* stepSize) const
        {
            const unsigned nIntervalsOnUnit = 4U;

            assert(bestDegree);
            assert(stepSize);
            assert(bw > 0.0);

            const unsigned maxDeg = maxFilterDegree1D(effBgEvents);
            const double degstep = 1.0/nIntervalsOnUnit;

            std::vector<double> arg(2);
            arg[0] = bw;

            bgFcn.getFilterProvider().startMemoizing();

            double minFCN = DBL_MAX, bestdeg = 0.0;
            for (unsigned ideg=0U; ideg<=maxDeg; ++ideg)
            {
                const unsigned nInt = ideg == maxDeg ? 1U : nIntervalsOnUnit;
                for (unsigned iint=0U; iint<nInt; ++iint)
                {
                    arg[1] = iint*degstep + ideg;
                    const double f = bgFcn(arg);
                    if (f < minFCN)
                    {
                        minFCN = f;
                        bestdeg = arg[1];
                    }
                }
            }

            bgFcn.getFilterProvider().stopMemoizing();

            *bestDegree = bestdeg;
            *stepSize = degstep;
        }

        void scanBandwidthOnly(
            const MinuitLOrPEBgCVFcn1D<Numeric,NumIn>& bgFcn,
            const unsigned bwScanPoints, const double effBgEvents,
            const double maxdeg, double* bestBandwidth, double* logStep) const
        {
            assert(bwScanPoints > 1U);
            assert(maxdeg >= 0.0);
            assert(bestBandwidth);
            assert(logStep);

            const double maxRatio = estimateMaxBandwidth()/
                minHistoBandwidth1D(histo_, minimumBgFeatureSize_,
                                    0.0, effBgEvents, m_);
            const double logstep = log(maxRatio)/(bwScanPoints - 1U);
            std::vector<double> arg(2);
            arg[1] = maxdeg;

            bgFcn.getFilterProvider().startMemoizing();

            double minFCN = DBL_MAX, bestbw = 0.0;
            for (unsigned ibw=0U; ibw<bwScanPoints; ++ibw)
            {
                arg[0] = ibw*logstep;
                const double f = bgFcn(arg);
                if (f < minFCN)
                {
                    minFCN = f;
                    bestbw = arg[0];
                }
            }

            bgFcn.getFilterProvider().stopMemoizing();

            *bestBandwidth = bgFcn.getActualBandwidth(bestbw, maxdeg);
            *logStep = logstep;
        }

        void scanBandwidthAndDegree(
            const MinuitLOrPEBgCVFcn1D<Numeric,NumIn>& bgFcn,
            const unsigned bwScanPoints, const double effBgEvents,
            double* p_bandwidth, double* p_bwstep,
            double* p_maxDegree, double* p_degstep) const
        {
            const unsigned nIntervalsOnUnit = 4U;

            assert(p_bandwidth);
            assert(p_bwstep);
            assert(p_maxDegree);
            assert(p_degstep);
            assert(bwScanPoints > 1U);
            assert(refitBandwidth_);

            const unsigned maxDeg = maxFilterDegree1D(effBgEvents);
            const double degstep = 1.0/nIntervalsOnUnit;
            const double maxRatio = estimateMaxBandwidth()/
                minHistoBandwidth1D(histo_, minimumBgFeatureSize_,
                                    0.0, effBgEvents, m_);
            const double logstep = log(maxRatio)/(bwScanPoints - 1U);
            std::vector<double> arg(2);

            bgFcn.getFilterProvider().startMemoizing();

            double minFCN = DBL_MAX;
            for (unsigned ideg=0U; ideg<=maxDeg; ++ideg)
            {
                const unsigned nInt = ideg == maxDeg ? 1U : nIntervalsOnUnit;
                for (unsigned iint=0U; iint<nInt; ++iint)
                {
                    arg[1] = iint*degstep + ideg;
                    for (unsigned ibw=0U; ibw<bwScanPoints; ++ibw)
                    {
                        arg[0] = ibw*logstep;
                        const double f = bgFcn(arg);
                        if (f < minFCN)
                        {
                            minFCN = f;
                            *p_bandwidth = arg[0];
                            *p_maxDegree = arg[1];
                        }
                    }
                }
            }

            bgFcn.getFilterProvider().stopMemoizing();

            *p_bandwidth = bgFcn.getActualBandwidth(*p_bandwidth, *p_maxDegree);
            *p_bwstep = logstep;
            *p_degstep = degstep;
        }

        // The following function must return "true" on success
        // and "false" on failure. *p_bandwidth and *p_maxDegree
        // will be used both as initial approximations and to
        // hold the output.
        //
        bool refitBandwidthAndDegree(
            const MinuitLOrPEBgCVFcn1D<Numeric,NumIn>& bgFcn,
            const unsigned ncycles, const double effectiveBgEvents,
            const bool fixBandwidth, const bool fixPolyDegree,
            double* p_bandwidth, const double inBwstep,
            double* p_maxDegree, const double inDegstep) const
        {
            assert(p_bandwidth);
            assert(p_maxDegree);

            double bw = *p_bandwidth;
            double deg = *p_maxDegree;

            // Bring the initial polynomial degree within limits
            const double degreeLimit = maxFilterDegree1D(effectiveBgEvents);
            if (!fixPolyDegree)
            {
                if (deg < 0.0)
                    deg = 0.0;
                if (deg > degreeLimit)
                    deg = degreeLimit;
            }

            // Calculate the bandwidth limits
            const double minBandwidth = minHistoBandwidth1D(
                histo_, minimumBgFeatureSize_, deg, effectiveBgEvents, m_);
            const double maxRatio = estimateMaxBandwidth()/
                minHistoBandwidth1D(histo_, minimumBgFeatureSize_,
                                    0.0, effectiveBgEvents, m_);

            // Bring the initial bandwidth within limits
            if (!fixBandwidth)
            {
                if (maxRatio <= 1.0)
                    throw std::invalid_argument(
                        "In npsi::MinuitSemiparametricFitFcn1D::"
                        "refitBandwidthAndDegree: minimum bandwidth is too "
                        "large (or maximum is too small)");

                const double maxBandwidth = minBandwidth*maxRatio;
                const double limFactor = 1.00001;

                if (bw < minBandwidth*limFactor)
                    bw = minBandwidth*limFactor;
                if (bw > maxBandwidth/limFactor)
                    bw = maxBandwidth/limFactor;

                bw = log(bw/minBandwidth);
            }

            // Try the cross validation fit several times
            // with different precision
            const double precision = crossValidationPrecision();
            for (unsigned itry=0; itry<5U; ++itry)
            {
                const double errfactor = pow(1.4, itry);
                const double precfactor = pow(10.0, itry);

                // Initialize Minuit parameters
                ROOT::Minuit2::MnUserParameters upar;
                if (fixBandwidth)
                    upar.Add("bandwidth", bw, 0.1);
                else
                {
                    const double bwstep = inBwstep > 0.0 ?
                                          inBwstep : 0.1*errfactor;
                    upar.Add("bandwidth", bw, bwstep, 0.0, log(maxRatio));
                }
                if (fixPolyDegree)
                    upar.Add("degree", deg, 0.1);
                else
                {
                    const double degstep = inDegstep > 0.0 ?
                                           inDegstep : 0.1*errfactor;
                    upar.Add("degree", deg, degstep, 0.0, degreeLimit);
                }

                // Create the minimization engine
                ROOT::Minuit2::MnMinimize mini(bgFcn, upar);
                mini.SetPrecision(precision*precfactor);

                if (fixBandwidth)
                    mini.Fix(0U);
                else if (fixPolyDegree)
                    mini.Fix(1U);
                else
                {
                    // Minuit is not good at fitting functions with strongly
                    // correlated parameters. Because of this, we might want
                    // to perform a few fix/release cycles.
                    for (unsigned icycle=0; icycle<ncycles; ++icycle)
                    {
                        mini.Fix(1U);
                        mini();
                        mini.Release(1U);

                        mini.Fix(0U);
                        mini();
                        mini.Release(0U);
                    }
                }

                ROOT::Minuit2::FunctionMinimum fit(mini());
                if (fit.IsValid())
                {
                    *p_maxDegree = mini.Value(1U);
                    if (fixBandwidth)
                        *p_bandwidth = mini.Value(0U);
                    else
                        *p_bandwidth = bgFcn.getActualBandwidth(mini.Value(0U),
                                                                *p_maxDegree);
                    return true;
                }
            }

            return false;
        }

        mutable std::vector<double> signalParameters_;
        mutable std::vector<double> signalDensityBuffer_;
        mutable std::vector<double> bgDensityBuffer_;
        mutable std::vector<double> workspace_;

        const npstat::HistoND<Numeric>& histo_;
        npstat::AbsSymbetaFilterProvider& fbuilder_;
        const DensityConstructor& densityMaker_;
        std::vector<NumIn> initialApproximation_;

        double minimumBgFeatureSize_;
        double convergenceEpsilon_;
        double pseudoFitPrecision_;
        mutable double bandwidth_;
        mutable double maxDegree_;
        mutable double signalFraction_;
        double minlog_;
        double up_;
        double upNonparametric_;
        double regularizationParameter_;
        double bandwidthLimitFactor_;
        double nEvents_;
        mutable double maxBgEventsInWindow_;

        mutable unsigned long callCount_;
        unsigned nIntegrationPoints_;
        unsigned maxIterations_;
        unsigned nFitCycles_;
        int polyDegreeLimit_;
        unsigned cvmode_;
        unsigned bandwidthScanPoints_;
        int m_;
        mutable unsigned numRegularized_;

        npstat::BoundaryHandling bm_;

        bool refitBandwidth_;
        bool refitPolyDegree_;
        bool verbose_;
        bool useLeastSquaresCV_;
  };
}

#endif // NPSI_MINUITSEMIPARAMETRICFITFCN1D_HH_
