#ifndef NPSTAT_DISCRETIZEDDISTANCE_HH_
#define NPSTAT_DISCRETIZEDDISTANCE_HH_

/*!
// \file discretizedDistance.hh
//
// \brief Utilities for calculating distances between two discretized functions
//
// Author: I. Volobouev
//
// February 2014
*/

namespace npstat {
    template<typename Numeric>
    double discretizedL1(const Numeric* a1, unsigned long length1,
                         const Numeric* a2, unsigned long length2,
                         double cellSize, bool normalizeAsDensity=false);

    template<typename Numeric>
    double discretizedL2(const Numeric* a1, unsigned long length1,
                         const Numeric* a2, unsigned long length2,
                         double cellSize, bool normalizeAsDensity=false);
}

#include "npstat/nm/discretizedDistance.icc"

#endif // NPSTAT_DISCRETIZEDDISTANCE_HH_
