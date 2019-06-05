#ifndef NPSTAT_STATUTILS_HH_
#define NPSTAT_STATUTILS_HH_

/*!
// \file StatUtils.hh
//
// \brief Statistical utilities which did not end up in dedicated headers
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>

namespace npstat {
    /**
    // This function calculates an empirical quantile
    // from a _sorted_ vector of data points. If the "increaseRange"
    // parameter is "true" then the value returned for x = 0.0
    // will be smaller than data[0] and the value returned for
    // x = 1.0 will be larger than the largest data value.
    */
    template<typename Data>
    Data empiricalQuantile(const std::vector<Data>& data, double x,
                           bool increaseRange=false);

    /**
    // The inverse to the npstat::empiricalQuantile (if there are no
    // duplicate entries in the data). The data vector must be sorted.
    */
    template<typename Data>
    double empiricalCdf(const std::vector<Data>& data, const Data& x);

    /**
    // Find the bin number corresponding to the given cdf value in
    // an array which represents a cumulative distribution function
    // (the numbers in the array must increase). It is expected that
    // the "cdfValue" input is between cdf[0] and cdf[arrLen-1].
    */
    template<typename Data>
    unsigned long quantileBinFromCdf(const Data* cdf, unsigned long arrLen,
                                     Data cdfValue, Data* remainder = 0);

    /**
    // This function returns the mathematical functional R(d^n f(x)/d x^n),
    // where function f(x) is given by its tabulated values on a grid
    // with constant distance h between points (it is assumed that each
    // value is given in the middle of a cell, like in a histogram). The
    // functional R(y(x)) is, by definition, the integral of y(x) squared.
    // d^n f(x)/d x^n is the derivative of order n.
    //
    // Note that the table of function values is NOT preserved.
    */
    template<typename Real>
    Real squaredDerivativeIntegral(Real* fvalues, unsigned long arrLen,
                                   unsigned n, Real h);

    /**
    // This function sets all negative elements of the input array to zero
    // and normalizes it so that the sum of the elements times the "binwidth"
    // argument becomes 1. If the input array is nowhere positive,
    // std::runtime_error is thrown. "true" is returned in case any negative
    // array elements are found, otherwise the function returns "false".
    // Upon exit (and if the "normfactor" pointer is not NULL), value of
    // *normfactor is set to the factor by which array elements are multiplied
    // so that they become normalized.
    */
    template<typename Real>
    bool normalizeArrayAsDensity(Real* arr, unsigned long arrLen,
                                 double binwidth, double* normfactor=0);

    /** Akaike information criterion corrected for the sample size */
    double aicc(const double ndof, const double logli, const double n);
}

#include "npstat/stat/StatUtils.icc"

#endif // NPSTAT_STATUTILS_HH_
