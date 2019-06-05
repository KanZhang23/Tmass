#ifndef NPSTAT_RANDOMSEQUENCEREPEATER_HH_
#define NPSTAT_RANDOMSEQUENCEREPEATER_HH_

/*!
// \file RandomSequenceRepeater.hh
//
// \brief Repeat a random sequence produced by another generator
//
// Author: I. Volobouev
//
// May 2010
*/

#include <vector>

#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    /**
    // This class remembers and repeats a random sequence.
    //
    // Use method "repeat()" to rewind to the beginning of the sequence.
    //
    // Use method "skip(...)" to skip an arbitrary number of sequence points.
    //
    // Use method "run(..)", as usual, to get the points. The class will
    // either call an underlying generator (when the number of "run" calls
    // exceeds the length of stored sequence) or, after at least one rewind,
    // will return a member of the stored sequence, as appropriate.
    */
    class RandomSequenceRepeater : public AbsRandomGenerator
    {
    public:
        inline RandomSequenceRepeater(AbsRandomGenerator& original)
            : orig_(original), dim_(original.dim()), pointer_(0UL) {}

        inline virtual ~RandomSequenceRepeater() {}

        inline unsigned dim() const {return dim_;}
        double operator()();
        void run(double* buf, const unsigned bufLen, const unsigned nPt);

        inline void repeat() {pointer_ = 0UL;}
        inline void clear() {pointer_ = 0UL; sequence_.clear();}
        void skip(unsigned long nSkip);

    private:
        AbsRandomGenerator& orig_;
        const unsigned dim_;

        std::vector<double> sequence_;
        unsigned long pointer_;
    };
}

#endif // NPSTAT_RANDOMSEQUENCEREPEATER_HH_
