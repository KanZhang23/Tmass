#ifndef NPSTAT_LORPEBACKGROUND1D_HH_
#define NPSTAT_LORPEBACKGROUND1D_HH_

/*!
// \file lorpeBackground1D.hh
//
// \brief A driver function for 1-d density estimation by LOrPE in a composite
//        signal plus background model
//
// Author: I. Volobouev
//
// October 2013
*/

#include <cfloat>

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/stat/AbsSymbetaFilterProvider.hh"

namespace npstat {
    /** Possible modes for cross validation calculations */
    enum {
        CV_MODE_FAST = 0,
        CV_MODE_MINUSBIN,
        CV_MODE_MINUSONE,
        CV_MODE_LINEARIZED,
        N_CV_MODES
    };

    /**
    // A driver function for density estimation from histograms using LOrPE
    // in a composite signal plus background model in which signal is
    // represented by a parametric distribution. The function arguments are:
    //
    //  histo      -- Naturally, the histogram to fit. It is assumed that the
    //                histogram bins are not scaled and contain the actual
    //                unweighted event counts.
    //
    //  fbuilder   -- This object will generate local polynomial filters using
    //                densities from the symmetric beta family as weights.
    //                This argument is not a LocalPolyFilter1D already so that
    //                some memoization is allowed (this might be useful for
    //                speeding up the calculation).
    //
    //  bm         -- Boundary handling method.
    //
    //  signal     -- Distribution to use for modeling the signal. Will
    //                be internally renormalized to that it integrates
    //                to 1 on the histogram support interval.
    //
    //  signalFraction -- Fraction of the signal in the sample. Must belong
    //                    to the (-1, 1) interval.
    //
    //  nIntegrationPoints -- How many points to use in order to integrate
    //                the parametric signal density across each bin. If this
    //                is specified as 0 then the difference of cumulative
    //                densities at the bin edges will be used, otherwise
    //                Gauss-Legendre quadrature will be employed (so that the
    //                number of points must be either 1 or one of the numbers
    //                supported by the "GaussLegendreQuadrature" class).
    //                For properly implemented densities, 0 should be the
    //                best option.
    //
    //  initialApproximation -- Initial approximation for the background
    //                density (one array element per input histogram bin).
    //                Can be specified as NULL in which case the uniform
    //                density is used as the initial approximation.
    //
    //  lenApproximation -- Length of the initial approximation array.
    //                Must be equal to the number of histogram bins.
    //
    //  m          -- Choose the kernel from the symmetric beta family
    //                proportional to (1 - x^2)^m. If m is negative,
    //                Gaussian kernel will be used, truncated at +- 12 sigma.
    //
    //  bandwidth  -- The bandwidth for the kernel used to generate the LOrPE
    //                polynomials.
    //
    //  maxDegree  -- Degree of the LOrPE polynomial. Interpretation of
    //                non-integer arguments is by the "continuousDegreeTaper"
    //                function.
    //
    //  convergenceEpsilon -- We will postulate that the iterations have
    //                converged if the L1 distance between the background
    //                distributions obtained in two successive iterations
    //                is less than this number. Must be non-negative.
    //
    //  maxIterations -- Maximum number of iterations allowed.
    //
    //  signalDensity -- Buffer for storing the signal density integrated
    //                over histogram bins. This result will be normalized
    //                to 1 on the histogram support interval. Can be
    //                specified as NULL if not needed.
    //
    //  lenSignalDensity -- Length of the "signalDensity" buffer.
    //
    //  bgDensity  -- Buffer for storing the background density estimate.
    //                This result will be normalized to 1 on the histogram
    //                support interval. Can be specified as NULL if not
    //                needed.
    //
    //  lenBgDensity -- Length of the "bgDensity" buffer.
    //
    //  workspace  -- If this function is called many times on the same
    //                histogram, it is recommended to reuse the same
    //                workspace buffer. This will save a few memory
    //                allocation calls for various internal needs.
    //
    //  densityMinusOne -- If provided, a buffer for storing the results
    //                in which the background density is estimated for every
    //                bin after removing one event from that bin (assuming that
    //                at least one event is present in that bin) or, depending
    //                on "cvmode", after removing the whole bin. Intended
    //                for subsequent use in cross validation. Note that
    //                requesting this calculation will slow the code down
    //                considerably. Strictly speaking, the returned numbers
    //                are calculated for non-empty bins only, they are not
    //                really a density, and they are intended purely for cross
    //                validation (there should be no attempt to normalize them).
    //
    //  lenDensityMinusOne -- Length of the "densityMinusOne" buffer.
    //
    //  cvmode     -- If "densityMinusOne" array is provided, this parameter
    //                affects calculation of this density. For binned densities,
    //                this calculation can be performed in a number of ways
    //                which differ in their treatment of discretization effects.
    //                Possible modes are:
    //
    //                CV_MODE_FAST       -- Remove the bin and use the same
    //                                      global background approximation
    //                                      to construct the EDF weights for
    //                                      each such bin.
    //
    //                CV_MODE_MINUSBIN   -- Remove the bin and recalculate
    //                                      the background approximation
    //                                      without this bin by iterations.
    //
    //                CV_MODE_MINUSONE   -- Reduce the bin value by 1 and
    //                                      recalculate the background
    //                                      approximation by iterations.
    //
    //                CV_MODE_LINEARIZED -- Faster, linearized version of
    //                                      CV_MODE_MINUSONE calculation
    //                                      (but might be less reliable).
    //
    //  regularizationParameter -- If this parameter is non-negative, the code
    //                will attempt to figure out the minimum reasonable value 
    //                of "densityMinusOne" if that density is estimated to be
    //                zero at some point which has data present. This minimum
    //                density will be inversely proportional to
    //                pow(N, regularizationParameter). This feature can be
    //                useful in pseudo-likelihood cross validation scenarios.
    //
    //  lastDivergence -- If provided, *lastDivergence will be filled
    //                by the actual divergence between the two most recent
    //                iterations used to calculate the background density.
    //                The absolute value tells the difference. If the value
    //                is negative, this means that there were negative entries
    //                that were truncated to zero (in this case one should
    //                not expect very good convergence anyway).
    //
    // The function returns the iteration number for which convergence
    // was established. If it is equal to "maxIterations" then the convergence
    // target was not reached.
    */
    template<typename Numeric, typename NumIn, typename NumOut>
    unsigned lorpeBackground1D(
        const HistoND<Numeric>& histo, AbsSymbetaFilterProvider& fbuilder,
        const BoundaryHandling& bm,
        const AbsDistribution1D& signal, double signalFraction,
        unsigned nIntegrationPoints,
        const NumIn* initialApproximation, unsigned lenApproximation,
        int m, double bandwidth, double maxDegree,
        double convergenceEpsilon, unsigned maxIterations,
        NumOut* signalDensity, unsigned lenSignalDensity,
        NumOut* bgDensity, unsigned lenBgDensity,
        std::vector<double>& workspace,
        NumOut* densityMinusOne = 0, unsigned lenDensityMinusOne = 0,
        unsigned cvmode = CV_MODE_LINEARIZED,
        double regularizationParameter = -1.0, double* lastDivergence = 0);

    /**
    // Function that can calculate pseudo log-likelihood for cross validation
    // using the output generated by "lorpeBackground1D". This function is
    // using a subset of "lorpeBackground1D" arguments. It is assumed that
    // "lorpeBackground1D" calculations have converged and that
    // "densityMinusOne" was calculated. The "minlog" parameter limits
    // the contribution into the log-likelihood from any single bin.
    */
    template<typename Numeric, typename NumOut>
    double lorpeBgCVPseudoLogli1D(
        const HistoND<Numeric>& histo, double signalFraction,
        const NumOut* signalDensity, unsigned lenSignalDensity,
        const NumOut* bgDensity, unsigned lenBgDensity,
        const NumOut* densityMinusOne, unsigned lenDensityMinusOne,
        double minlog = log(DBL_MIN));

    /**
    // Function that can calculate the least squares cross validation quantity
    // using the output generated by "lorpeBackground1D". This function is
    // using a subset of "lorpeBackground1D" arguments. It is assumed that
    // "lorpeBackground1D" calculations have converged and that
    // "densityMinusOne" was calculated.
    */
    template<typename Numeric, typename NumOut>
    double lorpeBgCVLeastSquares1D(
        const HistoND<Numeric>& histo, double signalFraction,
        const NumOut* signalDensity, unsigned lenSignalDensity,
        const NumOut* bgDensity, unsigned lenBgDensity,
        const NumOut* densityMinusOne, unsigned lenDensityMinusOne);

    /**
    // Function that can "regularize" the background density estimate
    // generated by "lorpeBackground1D" for subsequent log-likelihood
    // estimation. This procedure may be necessary in case there are
    // data points at the locations in which the density was estimated
    // to be 0 (or just too low).
    //
    // Parameter "minBgDensity1" specifies the minimum background-only
    // density which is expected for any bin which has at least one
    // background entry.
    //
    // The return value of the function tells in how many bins the
    // background density had to be adjusted.
    */
    template<typename Numeric, typename NumOut>
    unsigned lorpeRegularizeBgDensity1D(
        const HistoND<Numeric>& histo, double signalFraction,
        const NumOut* signalDensity, unsigned lenSignalDensity,
        NumOut* bgDensity, unsigned lenBgDensity, double minBgDensity1);    

    /**
    // Function that can calculate log-likelihood (for maximizing likelihood)
    // using the output generated by "lorpeBackground1D" (possibly, after
    // processing by "lorpeRegularizeBgDensity"). This function is using
    // a subset of "lorpeBackground1D" arguments. It is assumed that
    // "lorpeBackground1D" calculations have converged. The "minlog" parameter
    // limits the contribution into the log-likelihood from any single bin.
    */
    template<typename Numeric, typename NumOut>
    double lorpeBgLogli1D(
        const HistoND<Numeric>& histo, double signalFraction,
        const NumOut* signalDensity, unsigned lenSignalDensity,
        const NumOut* bgDensity, unsigned lenBgDensity,
        double minlog = log(DBL_MIN));

    /**
    // Estimate the maximum number of background points contained
    // inside the sliding window with the width "windowWidth". All other
    // parameters have the same meaning as in the "lorpeBackground1D"
    // function.
    */
    template<typename Numeric, typename NumIn>
    double maxBgPointsInWindow1D(
        const HistoND<Numeric>& histo,
        const AbsDistribution1D& signal, double signalFraction,
        unsigned nIntegrationPoints, double windowWidth,
        const NumIn* initialApproximation, unsigned lenApproximation,
        std::vector<double>& workspace);
}

#include "npstat/stat/lorpeBackground1D.icc"

#endif // NPSTAT_LORPEBACKGROUND1D_HH_
