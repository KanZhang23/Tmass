#ifndef NPSTAT_VARIABLEBANDWIDTHSMOOTH1D_HH_
#define NPSTAT_VARIABLEBANDWIDTHSMOOTH1D_HH_

/*!
// \file variableBandwidthSmooth1D.hh
//
// \brief Kernel density estimation with adaptive bandwidth
//
// Author: I. Volobouev
//
// August 2010
*/

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // This function performs kernel density estimation with variable
    // bandwidth which changes for each bin of the data histogram in the
    // inverse proportion to the pilot density estimate at that bin to some
    // power given by the parameter "alpha" (it seems 0.5 works well as
    // "alpha" argument in many contexts). The pilot estimate must be positive
    // for any bin that has any data in it. This pilot can be created,
    // for example, using fixed bandwidth polynomial filter of degree 0
    // (which is equivalent to KDE with boundary kernels).
    //
    // It is assumed that the kernel has its "center" at 0. It should normally
    // be symmetric around 0. Boundary kernel adjustment is performed
    // automatically.
    //
    // The variable bandwidth values will be adjusted in such a way that their
    // geometric mean will be equal to the argument "bandwidth".
    //
    // The "increaseBandwidthAtBoundary" argument allows the user to use wider
    // kernels at the boundaries of density support region (to compensate for
    // kernel leakage outside the support region). This adjustment comes after
    // the geometric mean normalization, so that geometric mean normalization
    // will not hold strictly in this case.
    //
    // Number of bins in the input histogram, length of the pilot density
    // estimate array, and length of the result array must all be the same.
    */
    template<typename Numeric, typename Num2, typename NumOut>
    void variableBandwidthSmooth1D(
        const HistoND<Numeric>& histo,
        const Num2* pilotDensityEstimate, unsigned lenEstimate,
        const AbsDistribution1D& kernel, double bandwidth,
        double alpha, bool increaseBandwidthAtBoundary,
        NumOut* result, unsigned lenResult);

    /**
    // High-level driver routine for "variableBandwidthSmooth1D". It is
    // assumed that one of the symmetric beta family kernels (including
    // the Gaussian for which "symbetaPower" parameter can be set to any
    // negative number) is used to build both the pilot and the final
    // density estimates. The "alpha" parameter is set to 0.5 and the
    // bandwidth is not increased at the boundary. The pilot estimate
    // is generated using the AMISE plugin bandwidth multiplied by the
    // "bandwidthFactor". It is assumed that the correct sample size
    // can be obtained by summing the histogram bin contents, so the
    // histogram should not be scaled.
    //
    // The function returns the pilot bandwidth used.
    */
    template<typename Numeric, typename NumOut>
    double simpleVariableBandwidthSmooth1D(
        const HistoND<Numeric>& histo, int symbetaPower,
        NumOut* result, unsigned lenResult, double bandwidthFactor = 1.0);

    /**
    // High-level driver routine for "variableBandwidthSmooth1D". It is
    // assumed that one of the symmetric beta family kernels (including
    // the Gaussian for which "symbetaPower" parameter can be set to any
    // negative number) is used to build both the pilot and the final
    // density estimates. The "alpha" parameter is set to 0.5 and the
    // bandwidth is not increased at the boundary. The pilot estimate
    // is generated using the AMISE plugin bandwidth multiplied by the
    // "bandwidthFactor". This code can be used with histograms that
    // are scaled or filled with weighted points as long as the correct
    // effective sample size is provided.
    //
    // The function returns the pilot bandwidth used.
    */
    template<typename Numeric, typename NumOut>
    double weightedVariableBandwidthSmooth1D(
        const HistoND<Numeric>& histo, double effectiveSampleSize,
        int symbetaPower, NumOut* result, unsigned lenResult,
        double bandwidthFactor = 1.0);
}

#include "npstat/stat/variableBandwidthSmooth1D.icc"

#endif // NPSTAT_VARIABLEBANDWIDTHSMOOTH1D_HH_
