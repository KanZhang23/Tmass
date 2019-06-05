#ifndef NPSTAT_EIGENMETHOD_HH_
#define NPSTAT_EIGENMETHOD_HH_

/*!
// \file EigenMethod.hh
//
// \brief Enumeration of LAPACK methods used to calculate
//        eigenvalues and eigenvectors
//
// Author: I. Volobouev
//
// May 2013
*/

#include <string>

namespace npstat {
    /**
    // EIGEN_SIMPLE: use simple LAPACK driver for calculating eigenvalues
    //               and eigenvectors (such as DSYEV)
    //
    // EIGEN_D_AND_C: use divide and conquer LAPACK driver (such as DSYEVD)
    //
    // EIGEN_RRR: use "Relatively Robust Representations" driver (e.g., DSYEVR)
    */
    enum EigenMethod
    {
        EIGEN_SIMPLE = 0,
        EIGEN_D_AND_C,
        EIGEN_RRR
    };

    /** Enums corresponding to method names */
    EigenMethod parseEigenMethod(const char* methodName);

    /** Method names corresponding to enums */
    const char* eigenMethodName(EigenMethod m);

    /** All valid method names for use in error messages, etc */
    std::string validEigenMethodNames();
}

#endif // NPSTAT_EIGENMETHOD_HH_
