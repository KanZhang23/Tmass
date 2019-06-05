#ifndef NPSTAT_CONSTANTBANDWIDTHSMOOTHERND_HH_
#define NPSTAT_CONSTANTBANDWIDTHSMOOTHERND_HH_

/*!
// \file ConstantBandwidthSmootherND.hh
//
// \brief Constant bandwidth multivariate KDE via FFT
//
// Author: I. Volobouev
//
// October 2011
*/

#include "npstat/nm/ConvolutionEngineND.hh"

#include "npstat/stat/AbsDistributionND.hh"
#include "npstat/stat/HistoND.hh"

namespace npstat {
    /**
    // N-d KDE implementation with constant bandwidth. Uses FFT, so it
    // should be reasonably fast even if the number of bins is large.
    // By default, the data is mirrored at the boundaries to reduce the
    // edge effects.
    //
    // The output density is truncated non-negative and normalized.
    //
    // If you would like to run cross-validation on bandwidth scans, consider
    // using a collection of KDEFilterND objects instead.
    */
    template <unsigned MaxDeg>
    class ConstantBandwidthSmootherND
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        // protoHisto -- Prototype histogram which defines expected data
        //               shape and grid coordinates.
        //
        // kernel     -- The "base" kernel function. This must have correct
        //               bandwidth settings already. Normally, the location
        //               vector has to be set to all zeros.
        //
        // taper      -- Damping factors for each polynomial degree
        //               (starting with the 0th order term). This can
        //               be NULL in which case it is assumed that all
        //               factors are 1.
        //
        // maxDeg     -- Polynomial degree to use with the given kernel.
        //               Must not exceed the MaxDeg template parameter.
        //
        // mirror     -- If true, the data will be mirrored at the
        //               boundaries to reduce the edge effects.
        */
        template <class Numeric>
        ConstantBandwidthSmootherND(const HistoND<Numeric>& protoHisto,
                                    const AbsDistributionND& kernel,
                                    const double* taper, unsigned maxDeg,
                                    bool mirror = true);

        inline ~ConstantBandwidthSmootherND() {delete engine_; delete buf_;}

        //@{
        /** Simple inspector of object properties */
        inline unsigned dim() const {return shape_.size();}
        inline const ArrayShape& shape() const {return shape_;}
        inline const unsigned* shapeData() const {return &shape_[0];}
        inline unsigned maxDegree() const {return taper_.size() - 1U;}
        inline bool mirrorsData() const {return mirror_;}
        double taper(unsigned degree) const;
        //@}

        /** 
        // Perform actual convolution. histoIn and histoOut can refer
        // to the same histogram.
        */
        template <class Numeric, class Real>
        void smoothHistogram(const HistoND<Numeric>& histoIn,
                             HistoND<Real>* histoOut);
    private:
        ConvolutionEngineND* engine_;
        ArrayShape shape_;
        ArrayShape indexBuf_;
        std::vector<double> taper_;
        ArrayND<double>* buf_;
        bool mirror_;
    };
}

#include "npstat/stat/ConstantBandwidthSmootherND.icc"

#endif // NPSTAT_CONSTANTBANDWIDTHSMOOTHERND_HH_
