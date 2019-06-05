#ifndef NPSTAT_ABSBANDWIDTHCV_HH_
#define NPSTAT_ABSBANDWIDTHCV_HH_

/*!
// \file AbsBandwidthCV.hh 
//
// \brief Interface definitions for KDE or LOrPE cross-validation calculations
//
// Implementations of these interfaces should calculate the quantity
// which is to be maximized (pseudo log likelihood, -AMISE, etc.)
//
// Author: I. Volobouev
//
// September 2010
*/

#include <utility>

#include "npstat/stat/HistoND.hh"

namespace npstat {
    // Forward declarations
    struct AbsPolyFilter1D;
    struct AbsPolyFilterND;

    /**
    // Cross-validation for univariate densities.
    // "Numeric" template parameter is the type of histogram bins.
    // "Num2" is the type produced by the density estimation code
    // (usually just double). "Num3" is the type of point coordinates
    // used to fill the histogram (normally, float or double), and
    // "Num4" is the type of point weights.
    */
    template<
        typename Numeric,
        typename Num2,
        typename Num3 = double,
        typename Num4 = Numeric
    >
    struct AbsBandwidthCV1D
    {
        typedef Numeric bin_type;
        typedef Num2 density_type;
        typedef Num3 coord_type;
        typedef Num3 weight_type;

        inline virtual ~AbsBandwidthCV1D() {}

        /**
        // It should be assumed that the "nFillsInRange" method of the
        // argument histogram returns the actual number of fills (that is,
        // the histogram represents an actual collection of points, has
        // possible bin values of 0, 1, 2, ..., and it is not scaled).
        //
        // "densityEstimate" is allowed to be an estimate without truncation
        // (even if it includes negative values). Dependence of the
        // optimized quantity on the bandwidth should be smoother without
        // truncation. Naturally, the order of values in "densityEstimate"
        // is the same as the order of bins in "histo".
        //
        // The "filterUsed" parameter is needed in order to be able to
        // do "leave-one-out" type of cross-validation. In such
        // calculations one has to know how much of the density estimate 
        // at coordinate x is contributed by the source point at x. When
        // density estimates are done on the grid, the filters differ for
        // the center of the grid and on the border. Because of this, we
        // will need to have an access to the complete filter collection.
        */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const Num2* densityEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const = 0;

        /** Cross-validation for samples of univariate weighted points */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            double effectiveSampleSize,
            const Num2* densityEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const = 0;

        /**
        // Cross-validation for samples of univariate weighted points
        // which can be used when the original sample is available.
        // For the sample points, the first element of the pair is
        // the point coordinate and the second element is the weight.
        // The histogram must be created in advance using that particular
        // sample.
        */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const std::pair<Num3, Num4>* sample,
            unsigned long lenSample,
            const Num2* densityEstimate,
            unsigned lenEstimate,
            const AbsPolyFilter1D& filterUsed) const = 0;
    };

    /**
    // Cross-validation for multivariate densities.
    // "Numeric" template parameter is the type of histogram bins.
    // "Array" template parameter should be one of ArrayND types.
    */
    template<typename Numeric, class Array>
    struct AbsBandwidthCVND
    {
        typedef Numeric bin_type;
        typedef Array density_type;

        inline virtual ~AbsBandwidthCVND() {}

        /**
        // It should be assumed that the "nFillsInRange" method of the
        // argument histogram returns the actual number of fills.
        //
        // "densityEstimate" is allowed to be an estimate without truncation
        // (even if it includes negative values).
        */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            const Array& densityEstimate,
            const AbsPolyFilterND& filterUsed) const = 0;

        /** Cross-validation for samples of multivariate weighted points */
        virtual double operator()(
            const HistoND<Numeric>& histo,
            double effectiveSampleSize,
            const Array& densityEstimate,
            const AbsPolyFilterND& filterUsed) const = 0;
    };
}

#endif // NPSTAT_ABSBANDWIDTHCV_HH_
