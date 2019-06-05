#ifndef NPSTAT_GAUSSIANRESPONSEMATRIX_HH_
#define NPSTAT_GAUSSIANRESPONSEMATRIX_HH_

/*!
// \file gaussianResponseMatrix.hh
//
// \brief Functions for building Gaussian response matrices for
//        1-d unfolding problems
//
// Author: I. Volobouev
//
// June 2014
*/

#include "npstat/nm/Matrix.hh"
#include "npstat/nm/SimpleFunctors.hh"

#include "npstat/stat/NUHistoAxis.hh"

namespace npstat {
    /**
    // Function arguments are as follows:
    //
    // unfoldedMin, unfoldedMax -- boundaries for the "unfolded" (that is,
    //                             physical) space.
    //
    // nUnfolded                -- number of subdivisions for the unfolded
    //                             space. Uniform binning is used.
    //
    // observedMin, observedMax -- boundaries for the "observed" space.
    //
    // nObserved                -- number of subdivisions for the observed
    //                             space. Uniform binning is used.
    //
    // gaussMeanFunction        -- this functor calculates the mean of the
    //                             observed gaussian for the given value
    //                             in the physical space. Use
    //                             "npstat::Same<double>()" as an argument
    //                             if you not not want to add an extra shift
    //                             to the point location.
    //
    // gaussWidthFunction       -- this functor calculates the width of the
    //                             observed gaussian for the given value
    //                             in the physical space. Use
    //                             "npstat::ConstValue1<double,double>(width)"
    //                             as an argument if you want a constant width.
    //
    // nQuadraturePoints        -- the number of points to use for integration
    //                             in each dimension. This should be one of
    //                             the numbers of points supported by the
    //                             GaussLegendreQuadrature class.
    //
    // This function will integrate the Gaussian density in the observed
    // space and average it in the unfolded space using Gauss-Legendre
    // quadratures with the given number of points in each space.
    */
    Matrix<double> gaussianResponseMatrix(
        double unfoldedMin, double unfoldedMax, unsigned nUnfolded,
        double observedMin, double observedMax, unsigned nObserved,
        const Functor1<double, double>& gaussMeanFunction,
        const Functor1<double, double>& gaussWidthFunction,
        unsigned nQuadraturePoints = 4U);

    /**
    // Function arguments are as follows:
    //
    // unfoldedAxis         -- Discretization of the "unfolded" (that is,
    //                         physical) space. Binning is expected to be
    //                         non-uniform.
    //
    // observedAxis         -- Discretization of the "observed" space.
    //                         Binning is expected to be non-uniform.
    //
    // gaussMeanFunction    -- this functor calculates the mean of the
    //                         observed gaussian for the given value
    //                         in the physical space. Use
    //                         "npstat::Same<double>()" as an argument
    //                         if you not not want to add an extra shift
    //                         to the point location.
    //
    // gaussWidthFunction   -- this functor calculates the width of the
    //                         observed gaussian for the given value
    //                         in the physical space. Use
    //                         "npstat::ConstValue1<double,double>(width)"
    //                         as an argument if you want a constant width.
    //
    // nQuadraturePoints    -- the number of points to use for integration
    //                         in each dimension. This should be one of
    //                         the numbers of points supported by the
    //                         GaussLegendreQuadrature class.
    //
    // This function will integrate the Gaussian density in the observed
    // space and average it in the unfolded space using Gauss-Legendre
    // quadratures with the given number of points in each space.
    */
    Matrix<double> gaussianResponseMatrix(
        const NUHistoAxis& unfoldedAxis, const NUHistoAxis& observedAxis,
        const Functor1<double, double>& gaussMeanFunction,
        const Functor1<double, double>& gaussWidthFunction,
        unsigned nQuadraturePoints = 8U);
}

#endif // NPSTAT_GAUSSIANRESPONSEMATRIX_HH_
