#ifndef NPSTAT_VOLUMEDENSITYFROMBINNEDRADIAL_HH_
#define NPSTAT_VOLUMEDENSITYFROMBINNEDRADIAL_HH_

/*!
// \file volumeDensityFromBinnedRadial.hh
//
// \brief Convert spherically symmetric radial densities modeled by
//        BinnedDensity1D into densities per unit area, volume, etc
//
// Author: I. Volobouev
//
// February 2016
*/

namespace npstat {
    /**
    // The function arguments are as follows:
    //
    //  dim           -- Dimensionality of the space.
    //
    //  binWidth      -- The width of the bins used originally to construct
    //                   the density estimate. Typically, the value of r
    //                   would be histogrammed and then smoothed. This
    //                   argument must be non-negative.
    //
    //  r             -- The distance from the origin. This argument must
    //                   be non-negative. In addition, "binWidth" and "r"
    //                   can not simultaneously be 0s.
    //
    //  radialDensity -- The value of density in r, i.e., (d Prob)/(d r).
    //                   If the default value of 1.0 is used, the function
    //                   returns the density conversion factor.
    */
    double volumeDensityFromBinnedRadial(unsigned dim, double binWidth,
                                         double r, double radialDensity=1.0);
}

#endif // NPSTAT_VOLUMEDENSITYFROMBINNEDRADIAL_HH_
