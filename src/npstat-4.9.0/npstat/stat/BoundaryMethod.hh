#ifndef NPSTAT_BOUNDARYMETHOD_HH_
#define NPSTAT_BOUNDARYMETHOD_HH_

/*!
// \file BoundaryMethod.hh
//
// \brief Enumeration of possible boundary handling methods for 1-d LOrPE
//
// This header should not be included directly in any application code.
// It is created, essentially, for convenience and for documentation
// purposes only. If necessary, use "npstat/stat/BoundaryHandling.hh"
// header file in the application code.
//
// Author: I. Volobouev
//
// June 2015
*/

namespace npstat {
  namespace Private {
    /**
    // The enum values have the following meanings:
    //
    // BM_TRUNCATE   Use simple truncation of the weight function
    //               (with subsequent renormalization).
    //
    // BM_STRETCH    Stretch the weight function near the boundary.
    //
    // BM_FOLD       Fold the weight function back into the support interval.
    //
    // BM_CONSTSQ    Stretch the weight function so that, when it is
    //               normalized, the integral of its square remains constant.
    //
    // BM_FOLDSQ     Combine folding and stretching so that the integral of
    //               weight squared remains constant.
    //
    // BM_CONSTVAR   Stretch the weight function to preserve its variance.
    //
    // BM_FOLDVAR    Stretch and fold the weight function to preserve its
    //               variance.
    //
    // BM_CONSTBW    Stretch the weight function locally so that the optimal
    //               bandwidth remains approximately constant throughout the
    //               support of the estimated density. This methods takes
    //               two parameters: the degree "M" of the filter and the
    //               typical distribution width.
    //
    // BM_FOLDBW     Stretch and fold the weight function locally so that
    //               the optimal bandwidth remains approximately constant
    //               throughout the support of the estimated density. This
    //               methods takes two parameters: the degree "M" of the
    //               filter and the typical distribution width.
    */
    enum BoundaryMethod
    {
        BM_TRUNCATE = 0,
        BM_STRETCH,
        BM_FOLD,
        BM_CONSTSQ,
        BM_FOLDSQ,
        BM_CONSTVAR,
        BM_FOLDVAR,
        BM_CONSTBW,
        BM_FOLDBW
    };
  }
}

#endif // NPSTAT_BOUNDARYMETHOD_HH_
