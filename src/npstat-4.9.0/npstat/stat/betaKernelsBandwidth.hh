#ifndef NPSTAT_BETAKERNELSBANDWIDTH_HH_
#define NPSTAT_BETAKERNELSBANDWIDTH_HH_

/*!
// \file betaKernelsBandwidth.hh
//
// \brief Optimal bandwidth for density estimation with beta kernels
//
// The formulae implemented in this code come from the paper by
// S.X. Chen, "Beta kernel estimators for density functions",
// Computational Statistics & Data Analysis 31, pp. 131-145 (1999).
// Note that printed versions of formulae for AMISE contain algebraic
// mistakes. These formulae have instead been rederived starting from
// equations 4.2 and 4.3 in the paper.
//
// Bandwidth values returned by this function may not be optimal at all
// for finite samples as the precision of curve estimation by sums of
// beta functions drops very sharply as a function of derivative number
// (only the terms proportional to the first and the second derivatives
// are considered in the paper which is sufficient for asymptotic reasoning).
// Thus the returned bandwidth values should be used as an approximate guide
// only, perhaps as starting points for a cross validation bandwidth scan.
// Both b1* and b2* should be calculated -- the difference between them
// gives an idea about potential spread of the optimal bandwidth.
//
// Author: I. Volobouev
//
// June 2013
*/

namespace npstat {
    /**
    // AMISE optimal bandwidth for density estimation by beta kernels.
    // The arguments are as follows:
    //
    // npoints       -- Number of points in the data sample.
    //
    // fvalues       -- Array of scanned values of the reference density.
    //                  It is assumed that the density is scanned at the
    //                  bin centers on the [0, 1] interval.
    //
    // nValues       -- Number of elements in the array "fvalues".
    //
    // returnB2Star  -- If "true", the function will return b_2* from Chen's
    //                  paper (and corresponding AMISE), otherwise it will
    //                  return b_1* (using corrected algebra).
    //
    // expectedAmise -- If this argument is provided, it will be filled
    //                  with the expected AMISE value.
    //
    // The generalized Bernstein polynomial degree is simply the inverse
    // of the bandwidth.
    */
    template<typename Real>
    double betaKernelsBandwidth(double npoints, const Real* fvalues,
                                unsigned long nValues, bool returnB2Star,
                                double* expectedAmise = 0);
}

#include "npstat/stat/betaKernelsBandwidth.icc"

#endif // NPSTAT_BETAKERNELSBANDWIDTH_HH_
