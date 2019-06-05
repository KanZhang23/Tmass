#ifndef NPSTAT_LORPEMISE1D_HH_
#define NPSTAT_LORPEMISE1D_HH_

/*!
// \file lorpeMise1D.hh
//
// \brief Deterministic MISE calculator for LOrPE smoothers that use kernels
//        from the symmetric beta family
//
// Author: I. Volobouev
//
// March 2014
*/

#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/stat/BoundaryHandling.hh"

namespace npstat {
    /**
    // Function arguments are as follows:
    //
    //  m           -- Choose the kernel from the symmetric beta family
    //                 proportional to (1 - x^2)^m. If m is negative,
    //                 Gaussian kernel will be used, truncated at +- 12 sigma.
    //
    //  lorpeDegree -- Degree of the LOrPE polynomial. Interpretation of
    //                 non-integer arguments is by the "continuousDegreeTaper"
    //                 function (see header file continuousDegreeTaper.hh).
    //
    //  bandwidth   -- Kernel bandwidth.
    //
    //  sampleSize  -- Number of data points in the sample.
    //
    //  nintervals  -- Number of discretization intervals. The CPU time of
    //                 the algorithm is O(nintervals^3).
    //
    //  xmin, xmax  -- The support of the distribution. Can be arbitrary
    //                 as long as the distribution is not 0 somewhere on it.
    //
    //  distro      -- The distribution for which the MISE will be determined.
    //
    //  bm          -- Method used to handle LOrPE weight at the boundary.
    //
    //  oversample  -- The number of points to use for calculating the
    //                 density integral on each discretization interval.
    //                 Can be 0 (use cdf method for the density), 1 (use
    //                 density value in the middle of the interval) or
    //                 any number of integration points supported by the
    //                 GaussLegendreQuadrature class.
    //
    //  ISB         -- If provided, the location to which this pointer refers
    //                 will be filled with the integrated squared bias.
    //
    //  variance    -- If provided, the location to which this pointer refers
    //                 with the variance component of the MISE.
    //
    // This function returns the estimated LOrPE MISE.
    */
    double lorpeMise1D(int m, double lorpeDegree, double bandwidth,
                       double sampleSize,
                       unsigned nintervals, double xmin, double xmax,
                       const AbsDistribution1D& distro,
                       const BoundaryHandling& bm,
                       unsigned oversample = 10U,
                       double *ISB = 0, double *variance = 0);
}

#endif // NPSTAT_LORPEMISE1D_HH_
