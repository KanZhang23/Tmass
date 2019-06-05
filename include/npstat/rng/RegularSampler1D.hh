#ifndef NPSTAT_REGULARSAMPLER1D_HH_
#define NPSTAT_REGULARSAMPLER1D_HH_

/*!
// \file RegularSampler1D.hh
//
// \brief Regular interval sampling in a manner that conforms
//        to the AbsRandomGenerator interface
//
// Author: I. Volobouev
//
// March 2011
*/

#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    /**
    // Simple regular sampler of the unit interval conforming to the
    // AbsRandomGenerator interface. It first splits [0,1] into two equal
    // intervals and returns the coordinate of the boundary (i.e., 0.5).
    // Then it splits the two obtained intervals (the returned points are
    // at 0.25 and 0.75). Then it splits the four intervals, then it splits
    // the eight intervals, and so on. The coordinates of the boundaries
    // are returned from left to right for each level of splitting.
    */
    class RegularSampler1D : public AbsRandomGenerator
    {
    public:
        inline RegularSampler1D() {reset();}

        inline virtual ~RegularSampler1D() {}

        inline unsigned dim() const {return 1U;}
        inline unsigned long long nCalls() const {return nCalls_;}

        double operator()();

        /** Reset to the initial state */
        void reset();

        /**
        // This method should be called to determine the number
        // of generated points after which the interval will be
        // uniformly sampled. The "level" can be 0, 1, 2, ..., 62.
        */
        static unsigned long long uniformCount(unsigned level);

    private:
        long double intervalWidth_;
        unsigned long long nIntervals_;
        unsigned long long interval_;
        unsigned long long nCalls_;
    };
}

#endif // NPSTAT_REGULARSAMPLER1D_HH_
