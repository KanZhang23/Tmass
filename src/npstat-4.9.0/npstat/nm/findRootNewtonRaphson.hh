#ifndef NPSTAT_FINDROOTNEWTONRAPHSON_HH_
#define NPSTAT_FINDROOTNEWTONRAPHSON_HH_

/*!
// \file findRootNewtonRaphson.hh
//
// \brief Simple root finding when the derivative is known
//
// Author: I. Volobouev
//
// July 2016
*/

#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /**
    // Numerical equation solving for 1-d functions using
    // the Newton–Raphson method.
    //
    // Input arguments are as follows:
    //
    //   f     -- The functor making up the equation to solve, returning
    //            a pair. The first element of the pair is the function
    //            value (for which "rhs" is the desired value) and the
    //            second element is the derivative.
    //
    //   rhs   -- The "right hand side" of the equation.
    //
    //   x0    -- The starting point for the search.
    //
    //   tol   -- Tolerance parameter. Typically, the found solution
    //            will be within a factor of 1 +- tol of the real one.
    //
    //   x     -- Location where the solution will be stored.
    //
    //   deriv -- Location to store the derivative at x (if desired).
    //
    // The function returns "true" if it finds the root, "false" otherwise.
    //
    // Typically, "Numeric" should be ether float or double.
    */
    template <typename Numeric>
    bool findRootNewtonRaphson(const PairFunctor<Numeric>& f,
                               Numeric rhs, Numeric x0,
                               Numeric tol, Numeric* x,
                               Numeric* deriv = 0);
}

#include "npstat/nm/findRootNewtonRaphson.icc"

#endif // NPSTAT_FINDROOTNEWTONRAPHSON_HH_
