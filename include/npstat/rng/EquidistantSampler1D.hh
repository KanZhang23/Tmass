#ifndef NPSTAT_EQUIDISTANTSAMPLER1D_HH_
#define NPSTAT_EQUIDISTANTSAMPLER1D_HH_

/*!
// \file EquidistantSampler1D.hh
//
// \brief Make equidistant points in a manner that conforms
//        to the AbsRandomGenerator interface
//
// Author: I. Volobouev
//
// April 2011
*/

#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    /**
    // Simple equidistant sampler of the unit interval conforming to
    // the AbsRandomGenerator interface. Imagine that the interval [0,1]
    // is split into bins, like in a histogram. The samples will be taken
    // in the middle of those bins -- either from left to right of from
    // right to left, depending on the value of "increasingOrder" costructor
    // parameter.
    */
    class EquidistantSampler1D : public AbsRandomGenerator
    {
    public:
        explicit EquidistantSampler1D(unsigned long long nIntervals,
                                      bool increasingOrder=true);

        inline virtual ~EquidistantSampler1D() {}

        inline unsigned dim() const {return 1U;}
        inline unsigned long long nCalls() const {return nCalls_;}
        inline unsigned long long maxPoints() const {return nIntervals_;}

        // The following operator will result in a run-time error
        // if the number of calls has already reached "maxPoints()"
        double operator()();

        // Reset to the initial state
        void reset();

    private:
        EquidistantSampler1D();

        long double intervalWidth_;
        unsigned long long nIntervals_;
        unsigned long long nCalls_;
        bool increasing_;
    };
}

#endif // NPSTAT_EQUIDISTANTSAMPLER1D_HH_
