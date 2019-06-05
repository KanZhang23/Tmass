#ifndef NPSTAT_GOLDENSECTIONSEARCH_HH_
#define NPSTAT_GOLDENSECTIONSEARCH_HH_

/*!
// \file goldenSectionSearch.hh
//
// \brief Search for 1-d function minimum in log space using
//        the golden section method
//
// Author: I. Volobouev
//
// October 2011
*/

#include "npstat/nm/SimpleFunctors.hh"
#include "npstat/nm/MinSearchStatus1D.hh"

namespace npstat {
    class DualAxis;

    /**
    // Search for 1-d function minimum using the golden section method.
    //
    // Input arguments are as follows:
    //
    //   f    -- The functor whose value we are minimizing. The comparison
    //           operator "<" must be defined for the Result type.
    //
    //   x0   -- The starting point for the search. For Arg1, operation of
    //           multiplication by a double must be defined. The space 
    //           searched for minimum will be x0*c, where c is a positive
    //           constant (for example, Arg1 can be a vector).
    //
    //   tol  -- Tolerance parameter. It will often be reasonable to set
    //           tol = sqrt(DBL_EPSILON). Typically, the found minimum
    //           will be within a factor of 1 +- tol of the real one.
    //
    //   minimum -- Location where the found minimum will be stored.
    //
    //   fmin -- Provide this if you want the function value at the minimum.
    //
    //   logstep -- Initial step in the log space. The code will first try
    //              the points x0*exp(logstep) and x0/exp(logstep) to bound
    //              the minimum, and will descend along the slope from there.
    //              "false" will be returned if the initial interval bounds
    //              a maximum instead.
    //
    // The function returns "true" if it finds the minimum, "false" otherwise.
    */
    template <typename Result, typename Arg1>
    bool goldenSectionSearchInLogSpace(const Functor1<Result, Arg1>& f,
                                       const Arg1& x0, double tol,
                                       Arg1* minimum, Result* fmin = 0,
                                       double logstep = 0.5);

    /**
    // Search for 1-d function minimum using the golden section method
    // on a discrete grid. The purpose is to find a 3-cell interval
    // whose middle cell corresponds to the lowest function value.
    // This search can be useful for functions that are expensive to
    // evaluate: it memoizes the results internally and will never call
    // the function twice with the same argument. The function returns
    // the search status.
    //
    // Input arguments are as follows:
    //
    //   f           -- The functor whose value we are minimizing.
    //
    //   axis        -- The axis which defines locations of the grid points.
    //
    //   i0          -- The starting grid cell for the search.
    //
    //   initialStep -- The initial step, in units of cells.
    //
    //   imin        -- Location where the found minimum will be stored
    //                  in case returned status in not MIN_SEARCH_FAILED.
    //
    //   fMinusOne, fmin, fPlusOne -- In case the status is MIN_SEARCH_OK,
    //                  these will contain function values the at the grid
    //                  cell which precedes the minimum, at the minimum
    //                  cell, and at the next cell, respectively. In case
    //                  the status is MIN_ON_LEFT_EDGE, *fMinusOne will be
    //                  set to *fmin. In case the status is MIN_ON_RIGHT_EDGE,
    //                  *fPlusOne will be set to *fmin. In case the status
    //                  is MIN_SEARCH_FAILED, the results are undefined.
    */
    MinSearchStatus1D goldenSectionSearchOnAGrid(
        const Functor1<double,double>& f, const DualAxis& axis,
        unsigned i0, unsigned initialStep,
        unsigned* imin, double* fMinusOne, double* fmin, double* fPlusOne);
}

#include "npstat/nm/goldenSectionSearch.icc"

#endif // NPSTAT_GOLDENSECTIONSEARCH_HH_
