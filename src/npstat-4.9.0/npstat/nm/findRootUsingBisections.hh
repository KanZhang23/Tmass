#ifndef NPSTAT_FINDROOTUSINGBISECTIONS_HH_
#define NPSTAT_FINDROOTUSINGBISECTIONS_HH_

/*!
// \file findRootUsingBisections.hh
//
// \brief Root finding with the bisection method
//
// Author: I. Volobouev
//
// October 2017
*/

#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /**
    // Numerical equation solving for 1-d functions using interval division.
    //
    // Input arguments are as follows:
    //
    //   f       -- The functor making up the equation to solve: f(x) == rhs.
    //              The comparison operator "<" must be defined for the Result
    //              type.
    //
    //   rhs     -- The "right hand side" of the equation.
    //
    //   x0, x1  -- The starting interval for the search.
    //
    //   tol     -- Tolerance parameter. Typically, the found solution
    //              will be within a factor of 1 +- tol of the real one.
    //
    //   root    -- Location where the root will be written. This could
    //              also be a discontinuity point of f(x) or a singularity.
    //
    // The function returns "false" in case the initial interval does not
    // bracket the root. In this case *root is not modified.
    */
    template <typename Result, typename Arg1>
    bool findRootUsingBisections(const Functor1<Result, Arg1>& f,
                                 Result rhs, Arg1 x0, Arg1 x1,
                                 Arg1 tol, Arg1* root);
}

#include "npstat/nm/findRootUsingBisections.icc"

#endif // NPSTAT_FINDROOTUSINGBISECTIONS_HH_
