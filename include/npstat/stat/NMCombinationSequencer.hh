#ifndef NPSTAT_NMCOMBINATIONSEQUENCER_HH_
#define NPSTAT_NMCOMBINATIONSEQUENCER_HH_

/*!
// \file NMCombinationSequencer.hh
//
// \brief Iteration over M distinct indices with N possible values
//
// Author: I. Volobouev
//
// June 2011
*/

#include <vector>

namespace npstat {
    /**
    // This class iterates over all possible choices of j1, .., jm from
    // N possible values for each jk in such a way that all j1, .., jm
    // are distinct and appear in the sequence in the increasing order,
    // last index changing most often.
    //
    // For the case m = 2 (two-dimensional), this is like going over the
    // indices of the part of a square N x N matrix which is above the
    // diagonal. In general, the total number of permutations equals
    // the binomial coefficient C(N, M).
    */
    class NMCombinationSequencer
    {
    public:
        /**
        // M is the number of indices and N is the number
        // of possible values for each index (from 0 to N-1)
        */
        NMCombinationSequencer(unsigned M, unsigned N);

        //@{
        /** Examine object properties */
        inline unsigned M() const {return m_;}
        inline unsigned N() const {return n_;}
        //@}

        /** Prefix increment */
        NMCombinationSequencer& operator++();

        /** Postfix increment (distinguished by the dummy "int" parameter) */
        void operator++(int);

        /** Reset the sequencer */
        void reset();

        /** Retrieve the current combination of indices */
        inline const unsigned* combination() const {return &coeffs_[0];}

        /** Linear iteration number */
        inline unsigned long count() const {return count_;}

        /**
        // This method returns "false" upon cycling over
        // the complete sequence of all possible choices
        */
        inline bool isValid() const {return valid_;}

    private:
        NMCombinationSequencer();

        void increment();

        std::vector<unsigned> coeffs_;
        unsigned long count_;
        unsigned m_;
        unsigned n_;
        bool valid_;
    };
}

#endif // NPSTAT_NMCOMBINATIONSEQUENCER_HH_
