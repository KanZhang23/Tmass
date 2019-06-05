#ifndef NPSTAT_WEIGHTEDDISTRO1DPTR_HH_
#define NPSTAT_WEIGHTEDDISTRO1DPTR_HH_

//======================================================================
// WeightedDistro1DPtr.hh
//
// This is an internal header which is subject to change without notice.
// Application code should never use the classes declared/defined in
// this header directly.
//
// Author: I. Volobouev
//
// June 2015
//======================================================================

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    namespace Private {
        struct WeightedDistro1DPtr
        {
            inline WeightedDistro1DPtr(const AbsDistribution1D& id,
                                       const double weight)
                : d(&id), w(weight) {}

            const AbsDistribution1D* d;
            double w;

            inline bool operator==(const WeightedDistro1DPtr& r) const
                {return *d == *r.d && w == r.w;}
            inline bool operator!=(const WeightedDistro1DPtr& r) const
                {return !(*this == r);}

        private:
            WeightedDistro1DPtr();
        };
    }
}

#endif // NPSTAT_WEIGHTEDDISTRO1DPTR_HH_
