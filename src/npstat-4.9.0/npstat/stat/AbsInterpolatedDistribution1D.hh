#ifndef NPSTAT_ABSINTERPOLATEDDISTRIBUTION1D_HH_
#define NPSTAT_ABSINTERPOLATEDDISTRIBUTION1D_HH_

/*!
// \file AbsInterpolatedDistribution1D.hh
//
// \brief Interface for interpolating 1-d distributions
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class AbsInterpolatedDistribution1D : public AbsDistribution1D
    {
    public:
        inline virtual ~AbsInterpolatedDistribution1D() {}

        /** "Virtual copy constructor" */
        virtual AbsInterpolatedDistribution1D* clone() const = 0;

        /** Add a new distribution */
        virtual void add(const AbsDistribution1D& d, double weight) = 0;

        /** Replace an existing distribution */
        virtual void replace(unsigned i, const AbsDistribution1D& d,
                             double weight) = 0;

        /** Modify the weight for an existing dostribution */
        virtual void setWeight(unsigned i, double weight) = 0;

        /** Clear all distributions */
        virtual void clear() = 0;

        /** The number of distributions participating in the interpolation */
        virtual unsigned size() const = 0;

        /**
        // The number of distributions expected to participate
        // in the interpolation
        */
        virtual unsigned expectedSize() const = 0;

        /**
        // The following function should be called to disable
        // (and later enable) automatic weight normalization
        // if you want to use the "setWeight" or "replace" methods
        // many times and, especially, if at some point in this process
        // the sum of the weights becomes zero. The "density" method
        // can not be called if normalization is not enabled.
        */
        virtual void normalizeAutomatically(bool allow) = 0;
    };
}

#endif // NPSTAT_ABSINTERPOLATEDDISTRIBUTION1D_HH_
