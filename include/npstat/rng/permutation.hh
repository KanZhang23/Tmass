#ifndef NPSTAT_PERMUTATION_HH_
#define NPSTAT_PERMUTATION_HH_

/*!
// \file permutation.hh
//
// \brief Utilities related to permuting a set of consecutive integers
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    /**
    // On output, array "permutation" will be filled by the permutation
    // of numbers from 0 to permLen-1 which correspond to the given
    // permutation number. The input permutation number must be
    // less than factorial(permLen).
    */
    void orderedPermutation(unsigned long permutationNumber,
                            unsigned *permutation, unsigned permLen);

    /**
    // On output, array "permutation" will be filled by a random
    // permutation of numbers from 0 to permLen-1. "permLen" can
    // be as large as the largest unsigned integer (but you will
    // probably run into memory limitations of your computer first).
    */
    void randomPermutation(AbsRandomGenerator& gen,
                           unsigned *permutation, unsigned permLen);

    /**
    // A mapping from a permuted set into a linear sequence.
    // factorial(permLen) should be less than the largest unsigned long.
    */
    unsigned long permutationNumber(const unsigned *permutation,
                                    unsigned permLen);

    /**
    // Simple factorial. Will generate a run-time error if n!
    // is larger than the largest unsigned long.
    */
    unsigned long factorial(unsigned n);

    /**
    // Precise integer function for the power. Will generate a run-time
    // error if a^n exceeds the largest unsigned long.
    */
    unsigned long intPower(unsigned a, unsigned n);

    /**
    // Factorial as a long double. Although imprecise, this has much
    // larger dynamic range than "factorial".
    */
    long double ldfactorial(unsigned n);

    /** Natural log of a factorial (using Stirling's series for large n) */
    long double logfactorial(unsigned long n);
}

#endif // NPSTAT_PERMUTATION_HH_
