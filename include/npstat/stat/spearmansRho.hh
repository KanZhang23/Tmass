#ifndef NPSTAT_SPEARMANSRHO_HH_
#define NPSTAT_SPEARMANSRHO_HH_

/*!
// \file spearmansRho.hh
//
// \brief Spearman's rank correlation coefficient (Spearman's rho)
//
// Calculation of the Spearman's rho measure of association of two
// variables. Here, rho can be estimated either from a sample of points
// or from an empirical copula.
//
// Author: I. Volobouev
//
// April 2012
*/

#include <vector>

namespace npstat {
    /**
    // Estimate Spearman's rho from a sample of multivariate points.
    // Class Point should be subscriptable.
    */
    template <class Point>
    double sampleSpearmansRho(const std::vector<Point>& data,
                              unsigned firstDim, unsigned secondDim);

    /**
    // Estimate Spearman's rho from an empirical copula.
    //
    // In this function, the array should represent an empirical 2-d
    // copula (constructed, for example, by the calculateEmpiricalCopula
    // function -- see header file empiricalCopula.hh). Do not confuse it
    // with the copula density.
    */
    template <class Array>
    double spearmansRhoFromCopula(const Array& copula);

    /**
    // Estimate Spearman's rho from an empirical copula density.
    //
    // In this function, the array should represent an empirical
    // 2-d copula density (constructed, for example, by the
    // empiricalCopulaDensity function -- see header file
    // empiricalCopula.hh). Do not confuse it with the copula.
    */
    template <class Array>
    double spearmansRhoFromCopulaDensity(const Array& copulaDensity);
}

#include "npstat/stat/spearmansRho.icc"

#endif // NPSTAT_SPEARMANSRHO_HH_
