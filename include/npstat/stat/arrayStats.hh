#ifndef NPSTAT_ARRAYSTATS_HH_
#define NPSTAT_ARRAYSTATS_HH_

/*!
// \file arrayStats.hh
//
// \brief Various descriptive statistics for 1-d and multidimensional arrays
//
// Author: I. Volobouev
//
// July 2010
*/

#include "npstat/nm/BoxND.hh"
#include "npstat/nm/Matrix.hh"

namespace npstat {
    /**
    // This function estimates population mean, standard
    // deviation, skewness, and kurtosis. The array must have at least
    // one element. A numerically sound two-pass algorithm is used.
    //
    // Any of the output pointers can be specified as NULL if the
    // corresponding quantity is not needed (this might speed the code up).
    */
    template<typename Numeric>
    void arrayStats(const Numeric* arr, unsigned long arrLen,
                    double* mean, double* stdev,
                    double* skewness=0, double* kurtosis=0);

    /**
    // Calculate the mean along each coordinate using array values as
    // weights. The coordinate ranges are defined by the given box,
    // and the array points are assumed to be in the centers of the bins,
    // like in a histogram. The results are stored in the "mean" array
    // whose length (given by lengthMean) must not be smaller than
    // the rank of the array.
    //
    // For this function, the array dimensionality can not exceed
    // CHAR_BIT*sizeof(unsigned long) which is normally 64 on 64-bit machines.
    */
    template<class Array>
    void arrayCoordMean(const Array& a, const BoxND<double>& limits,
                        double* mean, unsigned lengthMean);

    /** 
    // Calculate the array covariance matrix. The "limits" argument has
    // the same meaning as in the arrayCoordMean function.
    */
    template<class Array, unsigned Len>
    void arrayCoordCovariance(const Array& a, const BoxND<double>& limits,
                              Matrix<double,Len>* covarianceMatrix);

    /** 
    // One-dimensional mean, variance, skewness, and kurtosis
    // using array values as weights. Any of the argument pointers
    // can be NULL in which case the corresponding quantity is not
    // calculated. The code estimates population shape quantities
    // rather than sample quantities (so that it can be best used
    // with equidistantly binned histograms and such).
    */
    template<class Array>
    void arrayShape1D(const Array& a, double xmin, double xmax,
                      double* mean, double* stdev,
                      double* skewness, double* kurtosis);

    /** 
    // Quantiles for 1-d arrays in which array values are used as weights.
    // Essentially, the values are treated as histogram bin contents.
    // All input "qvalues" should be between 0 and 1. The results are
    // returned in the corresponding elements of the "quantiles" array.
    // The function will work faster if "qvalues" are sorted in the
    // increasing order.
    //
    // If you expect to call this function more than once using the same
    // data, it is likely that you can obtain the same information more
    // efficiently by creating the "BinnedDensity1D" object with its
    // interpolationDegree argument set to 0 and then using its "quantile"
    // method.
    */
    template<class Numeric>
    void arrayQuantiles1D(const Numeric* data, unsigned long len,
                          double xmin, double xmax, const double* qvalues,
                          double* quantiles, unsigned nqvalues);

    /** 
    // This function returns negative sum of p_i ln(p_i) over array
    // elements p_i, divided by the total number of array elements.
    // All elements must be non-negative, and there must be at least
    // one positive element present. Useful for calculating entropies
    // of distributions represented on a grid. For example, mutual
    // information of two variables is just the entropy of their copula.
    */
    template<class Numeric>
    double arrayEntropy(const Numeric* p, unsigned long len,
                        bool normalize = false);

    /** 
    // This function assumes that Poisson distribution parameters are
    // given in the "means" array while the array "counts" contains
    // corresponding observations. "len" is the length of both "means"
    // and "counts" arrays.
    */
    template<typename Real, typename Numeric>
    double poissonLogLikelihood(const Real* means, const Numeric* counts,
                                unsigned long len);
}

#include "npstat/stat/arrayStats.icc"

#endif // NPSTAT_ARRAYSTATS_HH_
