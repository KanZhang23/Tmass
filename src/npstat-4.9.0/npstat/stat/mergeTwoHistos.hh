#ifndef NPSTAT_MERGETWOHISTOS_HH_
#define NPSTAT_MERGETWOHISTOS_HH_

/*!
// \file mergeTwoHistos.hh
//
// \brief Smooth interpolation between two histograms
//
// Author: I. Volobouev
//
// July 2012
*/

#include "npstat/nm/AbsMultivariateFunctor.hh"

namespace npstat {
    /**
    // This function merges two histograms using a variable
    // weight which usually changes smoothly from 0 to 1
    // (or from 1 to 0) along a certain direction in the
    // space spanned by histogram axes. Here, the histograms
    // are basically treated as uniform data grids.
    //
    // The function arguments are as follows:
    //
    //  h1, h2   -- The histograms to merge. Their axes do not have to
    //              be the same.
    //
    //  w1       -- Functor which calculates the weight for histogram 1
    //              (histogram 2 will be assigned weight equal to 1.0 - w1).
    //
    //  result   -- Points to the result histogram (its bin contents will
    //              be modified). Dimensionalities of h1, h2, and the result
    //              histogram must be the same.
    //
    //  truncateWeight      -- If true, weights calculated by w1 will be
    //                         truncated so that they are between 0 and 1.
    //
    //  interpolationDegree -- This argument will be passed to the
    //                         "interpolateHistoND" functions which will
    //                         be employed to determine bin contents of
    //                         h1 and h2 at the bin locations of the
    //                         result histo.
    */
    template <class H1, class H2, class H3>
    void mergeTwoHistos(const H1& h1, const H2& h2,
                        const AbsMultivariateFunctor& w1,
                        H3* result, bool truncateWeight=true,
                        unsigned interpolationDegree=1);
}

#include "npstat/stat/mergeTwoHistos.icc"

#endif // NPSTAT_MERGETWOHISTOS_HH_
