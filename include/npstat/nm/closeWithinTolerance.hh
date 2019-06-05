#ifndef NPSTAT_CLOSEWITHINTOLERANCE_HH_
#define NPSTAT_CLOSEWITHINTOLERANCE_HH_

/*!
// \file closeWithinTolerance.hh
//
// \brief Determine if two doubles are within requested relative tolerance
//        of each other
//
// Author: I. Volobouev
//
// July 2012
*/

#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace npstat {
    /**
    // Check if two doubles are within certain relative tolerance from
    // each other. The "tol" argument which specifies the tolerance
    // must be non-negative.
    */
    inline bool closeWithinTolerance(const double& a, const double& b,
                                     const double& tol)
    {
        if (tol < 0.0)
            throw std::invalid_argument("In npstat::closeWithinTolerance: "
                                        "negative tolerance is not allowed");
        if (a == b)
            return true;
        else
            return fabs(a - b)/std::max(fabs(a), fabs(b)) <= tol;
    }
}

#endif // NPSTAT_CLOSEWITHINTOLERANCE_HH_
