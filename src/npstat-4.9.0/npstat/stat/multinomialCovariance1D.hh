#ifndef NPSTAT_MULTINOMIALCOVARIANCE1D_HH_
#define NPSTAT_MULTINOMIALCOVARIANCE1D_HH_

/*!
// \file multinomialCovariance1D.hh
//
// \brief Multinomial covariance matrix for a discretized 1-d density
//
// Author: I. Volobouev
//
// March 2014
*/

#include "npstat/nm/Matrix.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // Input arguments are:
    //
    //   fcn                -- the distribution to discretize
    //
    //   sampleSize         -- the number of points in the sample
    //
    //   nbins              -- number of discretization intervals
    //
    //   xmin, xmax         -- discretization region. The input density
    //                         will be normalized on this region.
    //
    //   nIntegrationPoints -- determines how many points per bin
    //                         will be used to calculate the bin average.
    //                         Can be 0 (use cdf difference at the bin edges),
    //                         1 (use density value at the center of the bin),
    //                         or one of the numbers of points supported by
    //                         the "GaussLegendreQuadrature" class.
    //
    // The returned matrix will have dimensions nbins x nbins.
    */
    Matrix<double> multinomialCovariance1D(
        const AbsDistribution1D& fcn, double sampleSize,
        unsigned nbins, double xmin, double xmax,
        unsigned nIntegrationPoints = 0);
}

#endif // NPSTAT_MULTINOMIALCOVARIANCE1D_HH_
