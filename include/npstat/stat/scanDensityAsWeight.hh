#ifndef NPSTAT_SCANDENSITYASWEIGHT_HH_
#define NPSTAT_SCANDENSITYASWEIGHT_HH_

/*!
// \file scanDensityAsWeight.hh
//
// \brief Scan multivariate kernel for subsequent use as a tabulated weight
//
// Author: I. Volobouev
//
// December 2011
*/

#include "geners/CPP11_auto_ptr.hh"
#include "npstat/nm/ArrayND.hh"
#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    /**
    // Utility function for scanning multivariate kernels and returning
    // ArrayND of minimal size which encloses the complete density support
    // region. It is assumed that the density is symmetric under all possible
    // mirror reflections (i.e., whenever the sign of any "x" component
    // changes, the density does not change).
    //
    // Arguments are as follows:
    //
    // kernel            -- the density to scan. Assumed to be even in
    //                      any coordinate.
    //
    // maxOctantDim      -- maximum number of steps to make in each dimension
    //                      while scanning the hyperoctant
    //
    // bandwidthSet      -- density function bandwidth values in each dimension
    //
    // stepSize          -- scan step size in each dimension
    //
    // arrayLength       -- number of elements in each of the arrays
    //                      maxOctantDim, bandwidthSet, and stepSize
    //
    // fillOneOctantOnly -- set "true" to scan one octant only (including
    //                      central grid points), "false" to scan the complete
    //                      support region of the density
    */
    CPP11_auto_ptr<ArrayND<double> > scanDensityAsWeight(
        const AbsDistributionND& kernel,
        const unsigned* maxOctantDim, const double* bandwidthSet,
        const double* stepSize, unsigned arrayLength,
        bool fillOneOctantOnly);
}

#endif // NPSTAT_SCANDENSITYASWEIGHT_HH_
