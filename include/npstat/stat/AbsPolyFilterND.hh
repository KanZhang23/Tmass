#ifndef NPSTAT_ABSPOLYFILTERND_HH_
#define NPSTAT_ABSPOLYFILTERND_HH_

/*!
// \file AbsPolyFilterND.hh
//
// \brief Interface definition for multivariate smoothers useable with
//        cross-validation
//
// Author: I. Volobouev
//
// September 2010
*/

#include <vector>

namespace npstat {
    /**
    // Base class for LOrPE, etc. smoothers which can be later used with
    // cross-validation algorithms. These algorithms usually work according
    // to the "leaving-one-out" method, in which the contribution of one
    // point into the density estimate must be subtracted. Therefore, this
    // contribution must be known to begin with, and this is precisely the
    // info this class is intended to convey.
    */
    struct AbsPolyFilterND
    {
        inline virtual ~AbsPolyFilterND() {}

        /** Dimensionality of the filter */
        virtual unsigned dim() const = 0;

        /** Required shape of the data array */
        virtual std::vector<unsigned> dataShape() const = 0;

        /**
        // Contribution of a single point into the density estimate
        // at that point (not normalized). This is needed for various
        // leaving-one-out cross-validation procedures.
        */
        virtual double selfContribution(
            const unsigned* index, unsigned lenIndex) const = 0;

        /**
        // Contribution of a single point into the density estimate
        // using the linear index of the point
        */
        virtual double linearSelfContribution(unsigned long index) const = 0;
    };
}

#endif // NPSTAT_ABSPOLYFILTERND_HH_
