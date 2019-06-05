#ifndef NPSTAT_BANDWIDTHGCVLEASTSQUARES1D_HH_
#define NPSTAT_BANDWIDTHGCVLEASTSQUARES1D_HH_

/*!
// \file BandwidthGCVLeastSquares1D.hh
//
// \brief Cross-validating one-dimensional density estimates of grouped data
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
    // for 1-dimensional density estimates. operator() returns -MISE estimate
    // (a quantity to be maximized). This class is intended for use inside
    // degree and/or bandwidth scans.
    */
    template<typename Numeric, typename Num2>
    struct BandwidthGCVLeastSquares1D : public AbsBandwidthGCV1D<Numeric,Num2>
    {
        inline virtual ~BandwidthGCVLeastSquares1D() {}

        // Unweighted samples
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const Num2* densityEstimate,
            const Num2* leaveOneOutEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const;

        // Weighted samples
        virtual double operator()(
            const HistoND<Numeric>& histo,
            double effectiveSampleSize,
            const Num2* densityEstimate,
            const Num2* leaveOneOutEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const;
    };
}

#include "npstat/stat/BandwidthGCVLeastSquares1D.icc"

#endif // NPSTAT_BANDWIDTHGCVLEASTSQUARES1D_HH_
