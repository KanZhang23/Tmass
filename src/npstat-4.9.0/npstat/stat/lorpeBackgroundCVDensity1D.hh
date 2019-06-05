#ifndef NPSTAT_LORPEBACKGROUNDCVDENSITY1D_HH_
#define NPSTAT_LORPEBACKGROUNDCVDENSITY1D_HH_

/*!
// \file lorpeBackgroundCVDensity1D.hh
//
// \brief Linearization of cross-validation calculations. This replaces
//        iterations performed inside lorpeBackground1D function.
//
// Author: I. Volobouev
//
// October 2013
*/

#include "npstat/stat/LocalPolyFilter1D.hh"

namespace npstat {
    template<typename Numeric, typename NumOut>
    void lorpeBackgroundCVDensity1D(
        const LocalPolyFilter1D& filter, const HistoND<Numeric>& histo,
        double signalFraction,
        const NumOut* signalDensity, unsigned lenSignalDensity,
        const NumOut* bgDensity, unsigned lenBgDensity,
        std::vector<double>& workspace,
        NumOut* densityMinusOne, unsigned lenDensityMinusOne);
}

#include "npstat/stat/lorpeBackgroundCVDensity1D.icc"

#endif // NPSTAT_LORPEBACKGROUNDCVDENSITY1D_HH_
