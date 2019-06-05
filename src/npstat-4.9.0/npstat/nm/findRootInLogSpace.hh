#ifndef NPSTAT_FINDROOTINLOGSPACE_HH_
#define NPSTAT_FINDROOTINLOGSPACE_HH_

/*!
// \file findRootInLogSpace.hh
//
// \brief Root finding in the log space by interval division
//
// Author: I. Volobouev
//
// November 2011
*/

#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /**
    // Numerical equation solving for 1-d functions using interval division.
    //
    // Input arguments are as follows:
    //
    //   f    -- The functor making up the equation to solve: f(x) == rhs.
    //           The comparison operator "<" must be defined for the Result
    //           type.
    //
    //   rhs  -- The "right hand side" of the equation.
    //
    //   x0   -- The starting point for the search. For Arg1, operation of
    //           multiplication by a double must be defined. The space 
    //           searched for solution will be x0*c, where c is a positive
    //           constant (for example, Arg1 can be a vector).
    //
    //   tol  -- Tolerance parameter. Typically, the found solution
    //           will be within a factor of 1 +- tol of the real one.
    //
    //   x    -- Location where the solution will be stored.
    //
    //   logstep -- Initial step in the log space. The code will first try
    //              the points x0*exp(logstep) and x0/exp(logstep) to bound
    //              the root, and will descend along the slope from there.
    //
    // The function returns "true" if it finds the root, "false" otherwise.
    */
    template <typename Result, typename Arg1>
    bool findRootInLogSpace(const Functor1<Result, Arg1>& f,
                            const Result& rhs, const Arg1& x0,
                            double tol, Arg1* x,
                            double logstep = 0.5);
}

#include "npstat/nm/findRootInLogSpace.icc"

#endif // NPSTAT_FINDROOTINLOGSPACE_HH_
