#ifndef NPSTAT_EMPIRICALCOPULAHISTO_HH_
#define NPSTAT_EMPIRICALCOPULAHISTO_HH_

/*!
// \file empiricalCopulaHisto.hh
//
// \brief Build copula histograms out of sets of points
//
// Author: I. Volobouev
//
// September 2010
*/

#include <vector>

#include "npstat/stat/OrderedPointND.hh"
#include "npstat/stat/HistoND.hh"

namespace npstat {
    /**
    // Function for building copula density out of sets of points by ordering
    // the points and remembering the order in each dimension. The histogram
    // is filled with point counts. All histogram axes should normally have
    // minimum at 0.0 and maximum at 1.0.
    //
    // The input "data" vector will be reordered by this function. It is
    // assumed that the "data" items have correct coordinates but that
    // their order has not been established yet. A typical way to fill
    // such a vector is to use the "fillOrderedPoints" function declared
    // in the "OrderedPointND.hh" header.
    //
    // See also functions declared in the empiricalCopula.hh header
    // which can perform similar tasks.
    //
    // This function assumes (but does not check) that the argument
    // histogram is defined on the unit multivariate cube. The histogram
    // will be reset before it is filled from the provided data.
    // The histogram contents will not be normalized after the
    // histogram is filled. If you want normalized result, call the
    // "convertHistoToDensity" function (declared in the HistoND header).
    //
    // The dimensionality of the points can be larger than the dimensionality
    // of the histogram. In this case only the leading dimensions are used.
    */
    template <class Point, typename Real>
    void empiricalCopulaHisto(std::vector<OrderedPointND<Point> >& data,
                              HistoND<Real>* result, bool useFillC = false);
}

#include "npstat/stat/empiricalCopulaHisto.icc"

#endif // NPSTAT_EMPIRICALCOPULAHISTO_HH_
