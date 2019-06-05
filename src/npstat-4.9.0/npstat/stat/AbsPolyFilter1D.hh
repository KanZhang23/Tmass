#ifndef NPSTAT_ABSPOLYFILTER1D_HH_
#define NPSTAT_ABSPOLYFILTER1D_HH_

/*!
// \file AbsPolyFilter1D.hh
//
// \brief Interface definition for 1-d smoothers useable with cross-validation
//
// Author: I. Volobouev
//
// September 2010
*/

namespace npstat {
    /**
    // Base class for LOrPE, etc. smoothers which can be later used with
    // cross-validation algorithms. These algorithms usually work according
    // to the "leaving-one-out" method, in which the contribution of one
    // point into the density estimate must be subtracted. Therefore, this
    // contribution must be known to begin with, and this is precisely the
    // info this class is intended to convey.
    */
    struct AbsPolyFilter1D
    {
        inline virtual ~AbsPolyFilter1D() {}

        /** Length of the data array */
        virtual unsigned dataLen() const = 0;

        /**
        // Contribution of a single point into the density estimate
        // at that point (not normalized). This is needed for various
        // leaving-one-out cross-validation procedures.
        */
        virtual double selfContribution(unsigned index) const = 0;
    };
}

#endif // NPSTAT_ABSPOLYFILTER1D_HH_
