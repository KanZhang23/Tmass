#ifndef NPSTAT_NEYMANPEARSONWINDOW1D_HH_
#define NPSTAT_NEYMANPEARSONWINDOW1D_HH_

/*!
// \file neymanPearsonWindow1D.hh
//
// \brief Search for a rectangular window in which the likelihood ratio
//        is above the given threshold
//
// Author: I. Volobouev
//
// March 2013
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    struct NeymanPearson {
        enum Status {
            OK = 0,
            SUPPORT_BOUNDARY,
            INDETERMINATE,
            INVALID
        };
    };

    /**
    // neymanPearsonWindow1D function assumes that the signal density
    // is unimodal and that the background density does not vary too 
    // quickly so that the S/B density ratio crosses the threshold
    // only twice (once on each side of the signal peak).
    //
    // Function input arguments are as follows:
    //
    //   signal          -- the signal distribution
    //
    //   background      -- the background distribution
    //
    //   searchStartCoordinate -- starting point for the search. At this
    //                      point the signal/background density ratio
    //                      should be above the threshold.
    //
    //   initialStepSize -- initial step size for the search (in either
    //                      direction)
    //                      
    //   threshold       -- ratio of signal/background densities
    //                      to search for. Must be positive.
    //
    // On exit, this function fills out the contents of leftBound,
    // leftBoundStatus, rightBound, and rightBoundStatus. Checking the
    // status of the window boundary calculation should be considered
    // essential. The meaning of status values is as follows:
    //
    //   OK               -- Found a good solution. The boundary corresponds
    //                       to the given signal/background density ratio.
    //
    //   SUPPORT_BOUNDARY -- Support boundary of either signal or background
    //                       density was reached before solution could be found.
    //
    //   INDETERMINATE    -- The signal/background density ratio became
    //                       indeterminate (for example, 0/0) before solution
    //                       could be found.
    //
    //   INVALID          -- The function was called with invalid input
    //                       values. For example, the density ratio was
    //                       below the threshold at the starting point.
    //
    // The function returns 0 in case the input arguments pass all sanity
    // checks and an error code otherwise. For precise meaning of different
    // error codes see comments to the return statements inside
    // neymanPearsonWindow1D.cc file.
    //
    // Before calling this function, it may be useful to call
    // "signalToBgMaximum1D". Then the "searchStartCoordinate" could be set
    // to the position of S/B maximum determined by "signalToBgMaximum1D"
    // while the threshold must be set to something smaller than the S/B
    // density ratio at the maximum.
    */
    int neymanPearsonWindow1D(const AbsDistribution1D& signal,
                              const AbsDistribution1D& background,
                              double searchStartCoordinate,
                              double initialStepSize,
                              double threshold,
                              double* leftBound,
                              NeymanPearson::Status* leftBoundStatus,
                              double* rightBound,
                              NeymanPearson::Status* rightBoundStatus);

    /** 
    // This function attempts to locate the position of the maximum of
    // S/B density ratio. It returns 0 in case the input arguments pass
    // all sanity checks and an error code otherwise.
    //
    // Note that, if a point is found for which background density is 0
    // and signal density is not, this point will be returned as a result.
    // In this case *maximumSignalToBgRatio will be set to DBL_MAX.
    */
    int signalToBgMaximum1D(const AbsDistribution1D& signal,
                            const AbsDistribution1D& background,
                            double searchStartCoordinate,
                            double initialStepSize,
                            double* maximumPosition,
                            double* maximumSignalToBgRatio,
                            NeymanPearson::Status* searchStatus);
}

#endif // NPSTAT_NEYMANPEARSONWINDOW1D_HH_
