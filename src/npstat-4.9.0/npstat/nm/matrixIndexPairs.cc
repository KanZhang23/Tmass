#include <cassert>
#include <stdexcept>

#include "npstat/nm/matrixIndexPairs.hh"

namespace npstat {
    std::vector<UUPair> matrixIndexPairs(const unsigned nDiagonalsToUse,
                                         const unsigned nRows,
                                         const bool remove00,
                                         const bool addAllZeroPairs)
    {
        if (nDiagonalsToUse > nRows) throw std::invalid_argument(
            "In npstat::matrixIndexPairs: too many diagonals requested");

        unsigned long expectedElements = 0;
        unsigned diagLength = nRows;
        for (unsigned irow=0; irow<nDiagonalsToUse; ++irow, --diagLength)
            expectedElements += diagLength;
        if (addAllZeroPairs)
            expectedElements += diagLength;
        if (remove00)
            // This will be meaningless if either nDiagonalsToUse or nRows is 0
            --expectedElements;

        std::vector<UUPair> pairs;
        if ((nDiagonalsToUse || addAllZeroPairs) && nRows)
        {
            pairs.reserve(expectedElements);
            if (nDiagonalsToUse)
                for (unsigned irow=0; irow<nRows; ++irow)
                    for (unsigned icol = remove00 && irow == 0U ? 1U : irow;
                         icol - irow < nDiagonalsToUse && icol < nRows; ++icol)
                        pairs.push_back(UUPair(irow, icol));
            if (addAllZeroPairs)
                for (unsigned icol = nDiagonalsToUse; icol < nRows; ++icol)
                    if (icol || !remove00)
                        pairs.push_back(UUPair(0U, icol));
            assert(expectedElements == pairs.size());
        }
        return pairs;
    }
}
