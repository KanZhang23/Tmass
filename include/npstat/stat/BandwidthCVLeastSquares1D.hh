#ifndef NPSTAT_BANDWIDTHCVLEASTSQUARES1D_HH_
#define NPSTAT_BANDWIDTHCVLEASTSQUARES1D_HH_

/*!
// \file BandwidthCVLeastSquares1D.hh
//
// \brief Cross-validating one-dimensional density estimates by optimizing
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
    // for 1-dimensional density estimates. operator() returns -MISE estimate
    // (a quantity to be maximized). This class is intended for use inside
    // degree and/or bandwidth scans.
    */
    template<
        typename Numeric,
        typename Num2,
        typename Num3 = double,
        typename Num4 = Numeric
    >
    struct BandwidthCVLeastSquares1D :
        public AbsBandwidthCV1D<Numeric,Num2,Num3,Num4>
    {
        inline virtual ~BandwidthCVLeastSquares1D() {}

        // Unweighted samples
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const Num2* densityEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const;

        // Weighted samples
        virtual double operator()(
            const HistoND<Numeric>& histo,
            double effectiveSampleSize,
            const Num2* densityEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const;

        // Weighted samples in case the sample is available
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const std::pair<Num3, Num4>* sample,
            unsigned long lenSample,
            const Num2* densityEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const;
    };
}

#include "npstat/stat/BandwidthCVLeastSquares1D.icc"

#endif // NPSTAT_BANDWIDTHCVLEASTSQUARES1D_HH_
