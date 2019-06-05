#ifndef NPSTAT_LORPESMOOTH1D_HH_
#define NPSTAT_LORPESMOOTH1D_HH_

/*!
// \file lorpeSmooth1D.hh
//
// \brief Simple driver function for 1-d density estimation by LOrPE
//
// Author: I. Volobouev
//
// March 2013
*/

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/BoundaryHandling.hh"

namespace npstat {
    /**
    // Simple driver function for density estimation from histograms using
    // LOrPE. It is assumed that the histogram fill counts correspond to
    // the actual number of points in the sample. The function arguments are:
    //
    //  m          -- Choose the kernel from the symmetric beta family
    //                proportional to (1 - x^2)^m. If m is negative,
    //                Gaussian kernel will be used, truncated at +- 12 sigma.
    //
    //  maxDegree  -- Degree of the LOrPE polynomial. Interpretation of
    //                non-integer arguments is by the "continuousDegreeTaper"
    //                function.
    //
    //  result     -- Buffer for storing the results. This can coincide
    //                with the input histogram bin contents (so that they
    //                will be changed in-place -- this makes sense only if
    //                the histogram contents are real).
    //
    //  lenResult  -- Length of the "result" buffer.
    //
    //  bm         -- Method used to handle LOrPE weight at the boundary.
    //
    //  bandwidthFactor -- The plugin bandwidth estimate will be multiplied
    //                by this factor to make the actual kernel bandwidth.
    //
    // The function returns the actual bandwidth used.
    */
    template<typename Numeric, typename NumOut>
    double weightedLorpeSmooth1D(
        const HistoND<Numeric>& histo, double effectiveSampleSize,
        int m, double maxDegree, NumOut* result, unsigned lenResult,
        const BoundaryHandling& bm, double bandwidthFactor=1.0);

    template<typename Numeric, typename NumOut>
    inline double lorpeSmooth1D(
        const HistoND<Numeric>& histo,
        int m, double maxDegree, NumOut* result, unsigned lenResult,
        const BoundaryHandling& bm, double bandwidthFactor=1.0)
    {
        const double n = histo.nFillsInRange();
        return weightedLorpeSmooth1D(histo, n, m, maxDegree, result,
                                     lenResult, bm, bandwidthFactor);
    }
}

#include "npstat/stat/lorpeSmooth1D.icc"

#endif // NPSTAT_LORPESMOOTH1D_HH_
