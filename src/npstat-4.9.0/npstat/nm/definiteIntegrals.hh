#ifndef NPSTAT_DEFINITEINTEGRALS_HH_
#define NPSTAT_DEFINITEINTEGRALS_HH_

/*!
// \file definiteIntegrals.hh
//
// \brief Formulae for various definite integrals based on exact expressions
//
// Author: I. Volobouev
//
// June 2013
*/

namespace npstat {
    /**
    // Integrate[(a x + b)/Sqrt[x (1-x)], {x, xmin, xmax}]
    // for the case both xmin and xmax are inside the [0, 1] interval
    */
    double definiteIntegral_1(double a, double b, double xmin, double xmax);
}

#endif // NPSTAT_DEFINITEINTEGRALS_HH_
