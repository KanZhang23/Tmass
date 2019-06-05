#ifndef NPSTAT_BANDWIDTHGCVLEASTSQUARESND_HH_
#define NPSTAT_BANDWIDTHGCVLEASTSQUARESND_HH_

/*!
// \file BandwidthGCVLeastSquaresND.hh
//
// \brief Cross-validating multivariate density estimates of grouped data
//        by optimizing the L2 distance (MISE)
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsBandwidthGCV.hh"

namespace npstat {
    /**
    // Class for calculating KDE or LOrPE cross-validation MISE approximations
    // for multivariate density estimates. operator() returns -MISE estimate
    // (a quantity to be maximized).
    */
    template<typename Num, class Array>
    struct BandwidthGCVLeastSquaresND : public AbsBandwidthGCVND<Num,Array>
    {
        inline virtual ~BandwidthGCVLeastSquaresND() {}

        // Unweighted samples
        virtual double operator()(
            const HistoND<Num>& histo,
            const Array& densityEstimate,
            const Array& leaveOneOutEstimate,
            const AbsPolyFilterND& filterUsed) const;

        // Weighted samples
        virtual double operator()(
            const HistoND<Num>& histo,
            double effectiveSampleSize,
            const Array& densityEstimate,
            const Array& leaveOneOutEstimate,
            const AbsPolyFilterND& filterUsed) const;
    };
}

#include "npstat/stat/BandwidthGCVLeastSquaresND.icc"

#endif // NPSTAT_BANDWIDTHGCVLEASTSQUARESND_HH_
