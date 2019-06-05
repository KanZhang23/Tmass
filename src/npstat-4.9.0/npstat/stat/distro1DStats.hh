#ifndef NPSTAT_DISTRO1DSTATS_HH_
#define NPSTAT_DISTRO1DSTATS_HH_

/*!
// \file distro1DStats.hh
//
// \brief Empirical moments of 1-d distributions using simple rectangle
//        integration (so that densities do not have to be very smooth)
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    void distro1DStats(const AbsDistribution1D& distro,
                       double xmin, double xmax, unsigned nPointsToUse,
                       double* mean, double* stdev,
                       double* skewness=0, double* kurtosis=0);
}

#endif // NPSTAT_DISTRO1DSTATS_HH_
