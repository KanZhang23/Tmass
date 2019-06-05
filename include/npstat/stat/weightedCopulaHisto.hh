#ifndef NPSTAT_WEIGHTEDCOPULAHISTO_HH_
#define NPSTAT_WEIGHTEDCOPULAHISTO_HH_

/*!
// \file weightedCopulaHisto.hh
//
// \brief Build copula histograms out of sets of weighted points
//
// Author: I. Volobouev
//
// June 2015
*/

#include <vector>
#include <utility>

#include "npstat/stat/HistoND.hh"

namespace npstat {
    /**
    // Function for building copula density out of sets of points by ordering
    // the points and remembering the order in each dimension. The histogram
    // is filled with point weights. All histogram axes should normally have
    // minimum at 0.0 and maximum at 1.0. This function assumes (but does not
    // check) that the argument histogram is defined on the unit multivariate
    // cube. The histogram will be reset before it is filled from the provided
    // data. The histogram contents will not be normalized after the
    // histogram is filled. If you want normalized result, call the
    // "convertHistoToDensity" function (declared in the HistoND header).
    //
    // Note that ties are not resolved by this function (i.e., their mutual
    // order is arbitrary).
    //
    // The function arguments are as follows:
    //
    //  data       -- Input data. The pointers (the first element of the pair)
    //                assume to point to objects which have a subscripting
    //                operator. The second element of the pair is the weight.
    //
    //  dimsToUse  -- Point dimensions to use for building the copula. This
    //                array should have at least "nDimsToUse" elements.
    //
    //  nDimsToUse -- Number of copula dimensions.
    //
    //  result     -- The histogram to fill.
    //
    //  useFillC   -- Specifies whether "fillC" histogram method should be
    //                used instead of the "fill" method.
    //
    // The function returns the Kish's effective sample size.
    */
    template <class Point, typename Real>
    double weightedCopulaHisto(
        const std::vector<std::pair<const Point*, double> >& data,
        const unsigned* dimsToUse, const unsigned nDimsToUse,
        HistoND<Real>* result, bool useFillC = false);
}

#include "npstat/stat/weightedCopulaHisto.icc"

#endif // NPSTAT_WEIGHTEDCOPULAHISTO_HH_
