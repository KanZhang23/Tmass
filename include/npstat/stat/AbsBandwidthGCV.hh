#ifndef NPSTAT_ABSBANDWIDTHGCV_HH_
#define NPSTAT_ABSBANDWIDTHGCV_HH_

/*!
// \file AbsBandwidthGCV.hh 
//
// \brief Interface definitions for cross-validation with grouped data
//
// Implementations of these interfaces should calculate the quantity
// which is to be maximized (pseudo log likelihood, -AMISE, etc.)
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/HistoND.hh"

namespace npstat {
    // Forward declarations
    struct AbsPolyFilter1D;
    struct AbsPolyFilterND;

    /**
    // Cross-validation for univariate densities.
    // "Numeric" template parameter is the type of histogram bins.
    // "Num2" is the type produced by the density estimation code
    // (usually just double).
    */
    template<typename Numeric, typename Num2>
    struct AbsBandwidthGCV1D
    {
        inline virtual ~AbsBandwidthGCV1D() {}

        /**
        // It should be assumed that the "nFillsInRange" method of the
        // argument histogram returns the actual number of fills (that is,
        // the histogram represents an actual collection of points, has
        // possible bin values of 0, 1, 2, ..., and it is not scaled).
        */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const Num2* densityEstimate,
            const Num2* leaveOneOutEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const = 0;

        /** Cross-validation for samples of univariate weighted points */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            double effectiveSampleSize,
            const Num2* densityEstimate,
            const Num2* leaveOneOutEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const = 0;
    };

    /**
    // Cross-validation for multivariate densities.
    // "Numeric" template parameter is the type of histogram bins.
    // "Array" template parameter should be one of ArrayND types.
    */
    template<typename Numeric, class Array>
    struct AbsBandwidthGCVND
    {
        inline virtual ~AbsBandwidthGCVND() {}

        /**
        // It should be assumed that the "nFillsInRange" method of the
        // argument histogram returns the actual number of fills.
        */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const Array& densityEstimate,
            const Array& leaveOneOutEstimate,
            const AbsPolyFilterND& filterUsed) const = 0;

        /** Cross-validation for samples of multivariate weighted points */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            double effectiveSampleSize,
            const Array& densityEstimate,
            const Array& leaveOneOutEstimate,
            const AbsPolyFilterND& filterUsed) const = 0;
    };
}

#endif // NPSTAT_ABSBANDWIDTHGCV_HH_
