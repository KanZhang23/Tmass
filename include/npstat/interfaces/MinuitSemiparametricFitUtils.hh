#ifndef NPSI_MINUITSEMIPARAMETRICFITUTILS_HH_
#define NPSI_MINUITSEMIPARAMETRICFITUTILS_HH_

/*!
// \file MinuitSemiparametricFitUtils.hh
//
// \brief Utilities for semiparametric fitting of signal/backround mixtures
//
// Author: I. Volobouev
//
// October 2013
*/

#include <cmath>
#include <cassert>

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/amiseOptimalBandwidth.hh"

namespace npsi {
    //
    // histo     -- the fitted histogram
    //
    // filterDeg -- LOrPE filter degree
    //
    // m         -- power of the symmetric beta kernel (Gaussian if m < 0)
    //
    template <typename Numeric>
    inline double boundaryBandwidth1D(const npstat::HistoND<Numeric>& histo,
                                      const double filterDeg,
                                      const int m)
    {
        // The minimal bandwidth must be such that there are still
        // enough bins covered by the filter weight function in order
        // to fit the polynomial of the requested maximum degree near
        // the histogram edge.
        assert(histo.dim() == 1U);
        double minBandwidth = (filterDeg + 2.0)*histo.binVolume();
        if (m < 0)
            minBandwidth /= 12.0;
        return minBandwidth;
    }

    //
    // Approximate bandwidth needed to model a feature of certain size
    //
    inline double featureBandwidth1D(const double featureSize,
                                     const double filterDeg,
                                     const double effectiveNBg,
                                     const int m)
    {
        double bw = 0.0;
        if (featureSize > 0.0)
        {
            bw = npstat::approxAmisePluginBwGauss(filterDeg, effectiveNBg,
                                                  featureSize);
            if (m >= 0)
                bw *= npstat::approxSymbetaBandwidthRatio(m, filterDeg);
        }
        return bw;
    }

    //
    // Minimum bandwidth to use for background density estimation
    //
    template <typename Numeric>
    inline double minHistoBandwidth1D(const npstat::HistoND<Numeric>& histo,
                                      const double featureSize,
                                      const double filterDeg,
                                      const double nbg,
                                      const int m)
    {
        const double bw1 = boundaryBandwidth1D(histo, filterDeg, m);
        const double bw2 = featureBandwidth1D(featureSize, filterDeg, nbg, m);
        return std::sqrt(bw1*bw1 + bw2*bw2);
    }
}

#endif // NPSI_MINUITSEMIPARAMETRICFITUTILS_HH_
