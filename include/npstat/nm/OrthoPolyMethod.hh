#ifndef NPSTAT_ORTHOPOLYMETHOD_HH_
#define NPSTAT_ORTHOPOLYMETHOD_HH_

/*!
// \file OrthoPolyMethod.hh
//
// \brief Enumeration of methods used to create orthogonal polynomials
//        with discrete weights
//
// Author: I. Volobouev
//
// May 2017
*/

#include <string>

namespace npstat {
    /** Method to generate the recurrence coefficients */
    enum OrthoPolyMethod
    {
        OPOLY_STIELTJES = 0, // Discrete Stieltjes algorithm

        OPOLY_LANCZOS        // Numerically stable version of Lanczos
                             // algorithm (RKPW algorithm from W.B. Gragg
                             // and W.J. Harrod, "The numerically stable
                             // reconstruction of Jacobi matrices from
                             // spectral data", Numer. Math. 44, 317 (1984))
    };

    /** Enums corresponding to method names */
    OrthoPolyMethod parseOrthoPolyMethod(const char* methodName);

    /** Method names corresponding to enums */
    const char* orthoPolyMethodName(OrthoPolyMethod m);

    /** All valid method names for use in error messages, etc */
    std::string validOrthoPolyMethodNames();
}

#endif // NPSTAT_ORTHOPOLYMETHOD_HH_
