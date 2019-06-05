#ifndef NPSTAT_BANDWIDTHCVLEASTSQUARESND_HH_
#define NPSTAT_BANDWIDTHCVLEASTSQUARESND_HH_

/*!
// \file BandwidthCVLeastSquaresND.hh
//
// \brief Cross-validating multivariate density estimates by optimizing
//        the L2 distance (MISE)
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/AbsBandwidthCV.hh"

namespace npstat {
    /**
    // Class for calculating KDE or LOrPE cross-validation MISE approximations
    // for multivariate density estimates. operator() returns -MISE estimate
    // (a quantity to be maximized).
    */
    template<typename Num, class Array>
    struct BandwidthCVLeastSquaresND : public AbsBandwidthCVND<Num,Array>
    {
        inline virtual ~BandwidthCVLeastSquaresND() {}

        // Unweighted samples
        virtual double operator()(
            const HistoND<Num>& histo,
            const Array& densityEstimate,
            const AbsPolyFilterND& filterUsed) const;

        // Weighted samples
        virtual double operator()(
            const HistoND<Num>& histo,
            double effectiveSampleSize,
            const Array& densityEstimate,
            const AbsPolyFilterND& filterUsed) const;
    };
}

#include "npstat/stat/BandwidthCVLeastSquaresND.icc"

#endif // NPSTAT_BANDWIDTHCVLEASTSQUARESND_HH_
