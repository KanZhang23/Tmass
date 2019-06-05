#ifndef NPSTAT_BINOMIALCOEFFICIENT_HH_
#define NPSTAT_BINOMIALCOEFFICIENT_HH_

/*!
// \file binomialCoefficient.hh
//
// \brief Calculate binomial coefficients avoiding overflows
//
// Author: I. Volobouev
//
// February 2012
*/

namespace npstat {
    /**
    // This code calculates the binomial coefficient C(N, M) trying to avoid
    // overflows. Throws either std::overflow_error or std::invalid_argument
    // if things go wrong.
    */
    unsigned long binomialCoefficient(unsigned N, unsigned M);

    /**
    // Binomial coefficient as a long double. Has much larger dynamic range
    // than the version which returns unsigned long.
    */
    long double ldBinomialCoefficient(unsigned N, unsigned M);
}

#endif // NPSTAT_BINOMIALCOEFFICIENT_HH_
