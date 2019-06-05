#ifndef NPSTAT_SVDMETHOD_HH_
#define NPSTAT_SVDMETHOD_HH_

/*!
// \file SvdMethod.hh
//
// \brief Enumeration of LAPACK methods used to calculate
//        singular value decompositions
//
// Author: I. Volobouev
//
// March 2014
*/

#include <string>

namespace npstat {
    /**
    // SVD_SIMPLE: use simple LAPACK driver for calculating SVD (such as DGESVD)
    //
    // SVD_D_AND_C: use divide and conquer LAPACK driver (such as DGESDD)
    */
    enum SvdMethod
    {
        SVD_SIMPLE = 0,
        SVD_D_AND_C
    };

    /** Enums corresponding to method names */
    SvdMethod parseSvdMethod(const char* methodName);

    /** Method names corresponding to enums */
    const char* svdMethodName(SvdMethod m);

    /** Valid method names for use in error messages, etc */
    std::string validSvdMethodNames();
}

#endif // NPSTAT_SVDMETHOD_HH_
