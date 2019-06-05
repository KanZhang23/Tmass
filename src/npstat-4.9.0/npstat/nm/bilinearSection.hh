#ifndef NPSTAT_BILINEARSECTION_HH_
#define NPSTAT_BILINEARSECTION_HH_

/*!
// \file bilinearSection.hh
//
// \brief Determine constant level contours in a bilinear interpolation cell
//
// Author: I. Volobouev
//
// July 2011
*/

#include <vector>
#include <utility>

namespace npstat {
    /**
    // This function finds the contours of the intersection of a bilinear
    // interpolation cell (specified by values at the corners of the unit
    // square) with a given constant level. To be used as a low-level building
    // block of contouring procedures which work with interpolated surfaces.
    //
    // The function assumes that the parameters z00, z10, z11, and z01
    // are the values at the corners (x,y) = (0,0), (1,0), (1,1), and (0,1),
    // respectively (i.e., in the counterclockwise order starting from
    // the origin).
    //
    // "level" is the crossing level for which the contours are found.
    //
    // "nPointsToSample" is the number of points to have in each contour.
    // If this is 1, a fast and crude check will be made and only one point
    // produced (at the center of the cell) if the level crosses the cell
    // anywhere. More reasonable curve representations typically need
    // at least 5 points.
    //
    // The return value of the function is the number of contours found
    // (could be 0, 1, or 2). "section1" is filled with "nPointsToSample"
    // points if this value is at least 1. "section2" is filled if this
    // value is 2. All coordinates will be between 0 and 1. Appropriate
    // shifting and scaling is left up to the user of this function.
    */
    unsigned bilinearSection(double z00, double z10, double z11, double z01,
                             double level, unsigned nPointsToSample,
                             std::vector<std::pair<double,double> >* section1,
                             std::vector<std::pair<double,double> >* section2);
}

#endif // NPSTAT_BILINEARSECTION_HH_
