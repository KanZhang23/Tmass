#ifndef NPSTAT_EMPIRICALCOPULA_HH_
#define NPSTAT_EMPIRICALCOPULA_HH_

/*!
// \file empiricalCopula.hh
//
// \brief Functions for building copulas out of sets of points
// 
// k-d trees are used internally for algorithm implementation.
// See also "empiricalCopulaHisto.hh" header.
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>

namespace npstat {
    /**
    // In the "calculateEmpiricalCopula" function the assumption is
    // that the result index 0 corresponds to copula argument 0.0
    // and that the maximum array index corresponds to copula argument 1.0.
    // Class Point should be subscriptable.
    */
    template <class Point, class Array>
    void calculateEmpiricalCopula(const std::vector<Point>& data,
                                  const unsigned* dimsToUse,
                                  unsigned nDimsToUse, Array* result);

    /**
    // In the "empiricalCopulaDensity" function the assumption is
    // that the result indices correspond to points equdistant
    // _inside_ the unit hypercube. That is, array index 0 corresponds
    // to copula argument 0.5/N, where N is the length of relevant
    // dimension. Array index N-1 corresponds to 1.0 - 0.5/N.
    // Class Point should be subscriptable.
    */
    template <class Point, class Array>
    void empiricalCopulaDensity(const std::vector<Point>& data,
                                const unsigned* dimsToUse,
                                unsigned nDimsToUse, Array* result);
}

#include "npstat/stat/empiricalCopula.icc"

#endif // NPSTAT_EMPIRICALCOPULA_HH_
