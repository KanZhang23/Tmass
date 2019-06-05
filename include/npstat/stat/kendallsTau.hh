#ifndef NPSTAT_KENDALLSTAU_HH_
#define NPSTAT_KENDALLSTAU_HH_

/*!
// \file kendallsTau.hh
//
// \brief Kendall's rank correlation coefficient (Kendall's tau)
//
// Calculation of the Kendall's tau measure of association of two
// variables. Here, tau can be estimated either from a sample of
// points or from an empirical copula.
//
// Author: I. Volobouev
//
// April 2012
*/

#include <vector>

namespace npstat {
    /**
    // Estimate Kendall's tau from a sample of multivariate points.
    // Class Point should be subscriptable.
    */
    template <class Point>
    double sampleKendallsTau(const std::vector<Point>& data,
                             unsigned firstDim, unsigned secondDim);

    /**
    // Estimate Kendall's tau from an empirical copula.
    //
    // In this function, the array should represent an empirical 2-d
    // copula (constructed, for example, by the calculateEmpiricalCopula
    // function -- see header file empiricalCopula.hh). Do not confuse it
    // with the copula density.
    */
    template <class Array>
    double kendallsTauFromCopula(const Array& copula);
}

#include "npstat/stat/kendallsTau.icc"

#endif // NPSTAT_KENDALLSTAU_HH_
