#ifndef NPSTAT_CONVERTAXIS_HH_
#define NPSTAT_CONVERTAXIS_HH_

/**
// \file convertAxis.hh
//
// \brief Functions for converting between grid and histogram axes
//
// For histogram axes it is assumed that the "active" coordinate is
// at the bin center, while for grid axes the relevant coordinate
// is at the point itself.
//
// Author: I. Volobouev
//
// July 2012
*/

#include "npstat/nm/DualAxis.hh"
#include "npstat/stat/DualHistoAxis.hh"

namespace npstat {
    /** Convert uniform grid axis to uniform histogram axis */
    HistoAxis convertToHistoAxis(const UniformAxis& gridAxis);

    /** Convert uniform histogram to uniform grid axis axis */
    UniformAxis convertToGridAxis(const HistoAxis& histoAxis);

    /**
    // Note that conversion from non-uniform histogram axis into
    // the grid axis is always possible, but it is loosing information
    // (we are writing the positions of bin centers only, not edges)
    */
    GridAxis convertToGridAxis(const NUHistoAxis& histoAxis);

    /**
    // The conversion from non-uniform grid axis to non-uniform
    // histogram axis is only unambiguous when some additional
    // info is available. Here, in particular, we are asking for
    // the position of the axis minimum. This function will throw
    // std::invalid_argument in case the conversion is impossible.
    */
    NUHistoAxis convertToHistoAxis(const GridAxis& gridAxis, double xMin);

    /** Convert dual histogram axis into dual grid axis */
    DualAxis convertToGridAxis(const DualHistoAxis& histoAxis);
}

#endif // NPSTAT_CONVERTAXIS_HH_
