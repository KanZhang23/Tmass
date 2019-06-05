#ifndef NPSTAT_HISTOSTATS_HH_
#define NPSTAT_HISTOSTATS_HH_

/*!
// \file histoStats.hh
//
// \brief Calculate various descriptive statistics for histograms
//
// Helper functions for calculating simple stats of multivariate
// histograms. Histogram bin centers are used as coordinates for
// which the statistical info is calculated and bin contents are
// used as weights. Naturally, a numerically sound two-pass algorithm
// is used to calculate the covariance matrix.
//
// The code is not going to verify that every bin entry is non-negative
// (very often, this is already guaranteed by the manner in which the
// histogram is accumulated). Note, however, that in the presence of
// negative weights the results can not be easily interpreted.
//
// Author: I. Volobouev
//
// December 2011
*/

#include <vector>

#include "geners/CPP11_auto_ptr.hh"
#include "npstat/nm/Matrix.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/DistributionsND.hh"

namespace npstat {
    /**
    // Calculate the mean of histogrammed data. The length of the "mean"
    // array should be equal to or exceed the dimensionality of the histogram.
    */
    template<class Histo>
    void histoMean(const Histo& histo, double* mean, unsigned lengthMean);

    /**
    // Calculate covariance matrix for the histogrammed data.
    // Upon exit, the size of the matrix will be N x N, where N
    // is the histogram dimensionality.
    */
    template<class Histo, unsigned Len>
    void histoCovariance(const Histo& histo, Matrix<double,Len>* covariance);

    /**
    // Construct a "BinnedDensity1D" object out of a uniformly binned
    // 1-d histogram. All histogram bins must be non-negative.
    */
    template<class Histo>
    CPP11_auto_ptr<BinnedDensity1D> histoDensity1D(
        const Histo& histo, unsigned interpolationDegree=0);

    /**
    // Construct a "BinnedDensityND" object out of a uniformly binned
    // multivariate histogram. All histogram bins must be non-negative.
    */
    template<class Histo>
    CPP11_auto_ptr<BinnedDensityND> histoDensityND(
        const Histo& histo, unsigned interpolationDegree=0);

#ifdef SWIG
    template<class Histo>
    inline BinnedDensity1D* histoDensity1D2(
        const Histo& histo, unsigned iDegree=0)
    {
        CPP11_auto_ptr<BinnedDensity1D> p(histoDensity1D(histo, iDegree));
        return p.release();
    }

    template<class Histo>
    inline BinnedDensityND* histoDensityND2(
        const Histo& histo, unsigned iDegree=0)
    {
        CPP11_auto_ptr<BinnedDensityND> p(histoDensityND(histo, iDegree));
        return p.release();
    }

    template<class Histo>
    inline std::vector<double> histoMean2(const Histo& histo)
    {
        const unsigned dim = histo.dim();
        std::vector<double> vec(dim, 0.0);
        if (dim)
            histoMean(histo, &vec[0], dim);
        return vec;
    }
#endif
}

#include "npstat/stat/histoStats.icc"

#endif // NPSTAT_HISTOSTATS_HH_
