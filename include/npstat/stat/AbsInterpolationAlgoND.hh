#ifndef NPSTAT_ABSINTERPOLATIONALGOND_HH_
#define NPSTAT_ABSINTERPOLATIONALGOND_HH_

/*!
// \file AbsInterpolationAlgoND.hh
//
// \brief Interface for multidimensional interpolation between densities
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    class AbsInterpolationAlgoND : public AbsDistributionND
    {
    public:
        inline explicit AbsInterpolationAlgoND(const unsigned dim)
            : AbsDistributionND(dim) {}

        inline virtual ~AbsInterpolationAlgoND() {}

        virtual AbsInterpolationAlgoND* clone() const = 0;

        virtual unsigned size() const = 0;

        virtual void add(const AbsDistributionND& d, double w) = 0;
        virtual void replace(unsigned i, const AbsDistributionND& d, double w)=0;
        virtual void setWeight(unsigned i, double w) = 0;
        virtual void clear() = 0;
        virtual void normalizeAutomatically(bool allow) = 0;

    protected:
        struct WeightedND
        {
            inline WeightedND(const AbsDistributionND* id, const double iw)
                : d(id), w(iw) {}

            inline WeightedND(const AbsDistributionND& id, const double iw)
                : d(&id), w(iw) {}

            inline bool operator==(const WeightedND& r) const
                {return *d == *r.d && w == r.w;}
            inline bool operator!=(const WeightedND& r) const
                {return !(*this == r);}

            const AbsDistributionND* d;
            double w;

        private:
            WeightedND();
        };
    };
}

#endif // NPSTAT_ABSINTERPOLATIONALGOND_HH_
