#ifndef NPSTAT_MATRIXINDEXPAIRS_HH_
#define NPSTAT_MATRIXINDEXPAIRS_HH_

/*!
// \file matrixIndexPairs.hh
//
// \brief Utility for enumerating pairs on diagonals of symmetric matrices 
//
// Author: I. Volobouev
//
// July 2018
*/

#include <vector>
#include <utility>

namespace npstat {
    typedef std::pair<unsigned,unsigned> UUPair;

    /*
    // This function will collect all index pairs (i, j) on the main
    // diagonal of a square matrix and above (therefore, i <= j), with
    // "nDiagonalsToUse" diagonals included in the result. Naturally,
    // "nDiagonalsToUse" argument must not exceed "nRows". The (0, 0) pair
    // will be excluded from the result in case "remove00" argument is "true".
    // If "addAllZeroPairs" argument is "true", all pairs of type (0, j)
    // will be added as well (the inclusion of the (0, 0) pair is still
    // governed by the "remove00" argument).
    //
    // This function is expected to be useful in some scenarios involving
    // symmetric matrices.
    */
    std::vector<UUPair> matrixIndexPairs(unsigned nDiagonalsToUse,
                                         unsigned nRows, bool remove00=false,
                                         bool addAllZeroPairs=false);
}

#endif // NPSTAT_MATRIXINDEXPAIRS_HH_
