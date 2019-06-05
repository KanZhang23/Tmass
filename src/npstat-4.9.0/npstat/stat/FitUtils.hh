#ifndef NPSTAT_FITUTILS_HH_
#define NPSTAT_FITUTILS_HH_

/*!
// \file FitUtils.hh
//
// \brief Utility functions which facilitate fitting of parametric distributions
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // This function calculates log-likelihood of a 1-d histogram
    // fit by a 1-d density (the value can then be maximized -- see,
    // for example, MinuitDensityFitFcn1D.hh header in the "interfaces"
    // section).
    //
    // The length of the "binMask" array should be the same as the number
    // of bins in the histogram. Only those bins will be used for which
    // corresponding mask values are not 0.
    //
    // The "workBuffer" array should be at least as long as the number
    // of bins in the histogram.
    //
    // "minlog" is the minimal value of log-likelihood which can be
    // contributed by one data point (that is, by a part of the bin height
    // which corresponds to 1 point). This should normally be a negative
    // number with reasonably large magnitude.
    //
    // "nQuad" is the number of quadrature points to use for calculating
    // the density integral inside each bin (should be supported by
    // GaussLegendreQuadrature class). If this parameter is set to 0,
    // cumulative density function will be used.
    //
    // "densityArea" and "enabledBinCount" can be used to obtain the
    // area of the density and the bin count for masked bins.
    //
    // The code essentially assumes Poisson distribution of counts
    // for each bin, constrained by the requirement that the sum of
    // expected events should be equal to the sum of events observed
    // (all of this is relevant for unmasked bins only). Note that,
    // although histogram scaling will not affect the central value
    // of the fit, it will affect the fit error determination.
    // Therefore, it is important to use histograms with actual
    // event counts rather than any kind of a scaled version.
    */
    template<typename Numeric,class Axis>
    double densityFitLogLikelihood1D(const HistoND<Numeric,Axis>& histo,
                                     const unsigned char* binMask,
                                     unsigned maskLength,
                                     const AbsDistribution1D& density,
                                     double minlog,
                                     double* workBuffer,
                                     unsigned workBufferLength,
                                     unsigned nQuad = 6U,
                                     double* densityArea = 0,
                                     double* enabledBinCount = 0);


    /** Simple unbinned log-likelihood for a 1-d sample of points */
    template<typename Numeric>
    double unbinnedLogLikelihood1D(const Numeric* samplePonts,
                                   unsigned nSamplePonts,
                                   const AbsDistribution1D& density,
                                   double minlog);
}

#include "npstat/stat/FitUtils.icc"

#endif // NPSTAT_FITUTILS_HH_
