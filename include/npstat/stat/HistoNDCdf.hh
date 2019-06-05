#ifndef NPSTAT_HISTONDCDF_HH_
#define NPSTAT_HISTONDCDF_HH_

/*!
// \file HistoNDCdf.hh
//
// \brief Construct multivariate cumulative density out of a histogram
//
// Author: I. Volobouev
//
// November 2011
*/

#include <cassert>
#include <stdexcept>

#include "npstat/stat/HistoND.hh"

namespace npstat {
    /**
    // Multivariate cumulative density for a histogram of some scalar type.
    // This class is mainly useful for its ability to come up with boxes
    // that cover a certain predefined fraction of the distribution.
    */
    class HistoNDCdf
    {
    public:
        /**
        // The argument histogram can not have negative bins
        // and also can not be empty
        //
        // The "iniScales" parameter in the following constructor
        // specifies the proportions for the sides of the boxes
        // which will be returned by the "coveringBox" method.
        // For example, if for a 3-d histogram these proportions
        // are 1, 2, and 3 then the sides of the returned boxes
        // will be two times longer in the second dimension in
        // comparison with the first dimension, and three times
        // longer in the third dimension. All scales must be positive.
        //
        // The "eps" parameter specifies the tolerance within which
        // the sizes of the boxes returned by the "coveringBox" method
        // will be calculated. Their linear sizes will be within
        // the factor (1 +- eps) of the correct ones. Naturally,
        // this number must be larger than DBL_EPSILON.
        */
        template <typename Numeric>
        HistoNDCdf(const HistoND<Numeric>& histo,
                   const double* iniScales, const unsigned lenScales,
                   const double eps=1.0e-12)
            : cdf_(ArrayND<double>(histo.binContents()).template 
                   cdfArray<long double>()),
              axes_(histo.axes()),
              startBox_(BoxND<double>::sizeTwoBox(lenScales)),
              buf_(lenScales),
              tol_(eps),
              dim_(lenScales)
        {
            if (histo.dim() == 0) throw std::invalid_argument(
                "In npstat::HistoNDCdf constructor: "
                "can not use zero-dimensional histograms");
            if (!histo.binContents().isDensity()) throw std::invalid_argument(
                "In npstat::HistoNDCdf constructor: "
                "histogram is not a density");
            if (lenScales != histo.dim()) throw std::invalid_argument(
                "In npstat::HistoNDCdf constructor: "
                "incompatible number of scales");
            assert(iniScales);
            for (unsigned i=0; i<dim_; ++i)
            {
                if (iniScales[i] <= 0.0) throw std::invalid_argument(
                    "In npstat::HistoNDCdf constructor: "
                    "all scales must be positive");
                startBox_[i].expand(iniScales[i]);
            }
            const double mx = cdf_.linearValue(cdf_.length() - 1U);
            assert(mx > 0.0);
            cdf_ /= mx;
        }

        /** Histogram dimensionality */
        inline unsigned dim() const {return dim_;}

        /** Bounding box of the histogram from which this object was created */
        BoxND<double> boundingBox() const;

        /** Multivariate cumulative density function */
        double cdf(const double* x, unsigned dim) const;

        /** Fraction of the distribution inside the given box */
        double boxCoverage(const BoxND<double>& box) const;

        /**
        // The box that is centered at the given position and covers
        // the given fraction of the distribution
        */
        void coveringBox(double coveredFraction, const double* boxCenter,
                         unsigned dimCenter, BoxND<double>* coverBox) const;
    private:
        HistoNDCdf();

        ArrayND<double> cdf_;
        std::vector<HistoAxis> axes_;
        BoxND<double> startBox_;
        mutable std::vector<double> buf_;
        double tol_;
        unsigned dim_;
    };
}

#endif // NPSTAT_HISTONDCDF_HH_
