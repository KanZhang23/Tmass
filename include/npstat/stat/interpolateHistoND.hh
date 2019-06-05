#ifndef NPSTAT_INTERPOLATEHISTOND_HH_
#define NPSTAT_INTERPOLATEHISTOND_HH_

/*!
// \file interpolateHistoND.hh
//
// \brief Interpolate histogram contents
//
// Functions which interpolate histogram contents are not included
// into the HistoND template itself because we do not always want to
// create histograms using bin types which can be multiplied by doubles
// (also, results of such a multiplication have to be automatically
// converted back to the same type).
//
// The implementations work by invoking "interpolate1" or "interpolate3"
// ArrayND methods on the histogram bin contents after an appropriate
// coordinate transformation.
//
// Author: I. Volobouev
//
// November 2011
*/

#include "npstat/stat/HistoND.hh"

namespace npstat {
    /**
    // The interpolation degree in this method can be set to 0, 1, or 3
    // which results, respectively, in closest bin lookup, multilinear
    // interpolation, or multicubic interpolation. Value of the closest
    // bin inside the histogram range is used if some coordinate is outside
    // of the corresponding axis limits.
    */
    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             const double *coords, unsigned coordsDim,
                             unsigned interpolationDegree);
    //@{
    /**
    // Convenience function for interpolating histograms, with
    // an explicit coordinate argument for each histogram dimension
    */
    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1,
                             unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2,
                             unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             double x4, unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             double x4, double x5,
                             unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             double x4, double x5, double x6,
                             unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             double x4, double x5, double x6, double x7,
                             unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             double x4, double x5, double x6, double x7,
                             double x8, unsigned interpolationDegree);

    template <typename Float, class Axis>
    Float interpolateHistoND(const HistoND<Float,Axis>& histo, 
                             double x0, double x1, double x2, double x3,
                             double x4, double x5, double x6, double x7,
                             double x8, double x9,
                             unsigned interpolationDegree);
    //@}
}

#include "npstat/stat/interpolateHistoND.icc"

#endif // NPSTAT_INTERPOLATEHISTOND_HH_
