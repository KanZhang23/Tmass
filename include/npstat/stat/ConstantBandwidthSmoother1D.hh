#ifndef NPSTAT_CONSTANTBANDWIDTHSMOOTHER1D_HH_
#define NPSTAT_CONSTANTBANDWIDTHSMOOTHER1D_HH_

/*!
// \file ConstantBandwidthSmoother1D.hh
//
// \brief Fast constant bandwidth KDE in one dimension via FFT
//
// Author: I. Volobouev
//
// October 2011
*/

#include "npstat/stat/AbsMarginalSmootherBase.hh"
#include "npstat/stat/AbsPolyFilter1D.hh"

namespace npstat {
    class ConvolutionEngine1D;

    /**
    // 1-d KDE implementation with constant bandwidth, using kernels
    // from the beta family (or the Gaussian). Based on FFT, so it
    // should be reasonably fast even if the number of bins is large.
    // By default, the data is mirrored at the boundary to reduce the
    // edge effects.
    //
    // The output density is truncated non-negative and normalized.
    */
    class ConstantBandwidthSmoother1D : public AbsMarginalSmootherBase,
                                        public AbsPolyFilter1D
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        // nbins, xmin, xmax  -- Parameters for the histogram which will
        //                       be accumulated using the data sample to be
        //                       smoothed. nbins should preferrably be
        //                       a power of 2 (for subsequent FFT).
        //
        // symbetaPower       -- Power of the symmetric beta kernel to use.
        //                       Gaussian kernel chopped of at 12 sigma will
        //                       be used in case this parameter is negative.
        //                       This parameter must not exceed 10.
        //
        // kernelOrder        -- Order of the kernel. Meaningful arguments
        //                       are even numbers 2 or larger. Numbers below
        //                       2 will be converted to 2, 1 will be subtracted
        //                       from odd numbers (poly degree is the kernel
        //                       order minus 2).
        //
        // bandwidth          -- Fixed bandwidth to use. Value of 0.0
        //                       (default) means to use the Gaussian plug-in
        //                       estimate which will change from one dataset
        //                       to another. Note that the bin width of the
        //                       histogram with which "smoothHisto" method
        //                       will be called is not necessarily the same
        //                       from one call to another, so the usage
        //                       of fixed bandwidth has to take this into
        //                       account (basically, by also using the class
        //                       DummyResponseBoxBuilder in the appropriate
        //                       place which will lead to constant bin width).
        //
        // bwFactor           -- Fudge factor for the bandwidth used for
        //                       the density estimate (either for the plug-in
        //                       bandwidth or for the fixed one provided by
        //                       the previous argument).
        //
        // mirrorData         -- If true, the data will be mirrored at
        //                       the boundary.
        //
        // label              -- Label for the axis. Useful in case
        //                       smoothing results are stored for inspection.
        */
        ConstantBandwidthSmoother1D(unsigned nbins, double xmin, double xmax,
                                    int symbetaPower, unsigned kernelOrder,
                                    double bandwidth=0.0, double bwFactor=1.0,
                                    bool mirrorData=true, const char* label=0);

        virtual ~ConstantBandwidthSmoother1D();

        //@{
        /** Simple inspector of object properties */
        inline int symbetaPower() const {return symbetaPower_;}
        inline unsigned kernelOrder() const {return kernelOrder_;}
        inline double fixedBandwidth() const {return fixedBandwidth_;}
        inline double bwFactor() const {return bwFactor_;}
        inline bool mirrorsData() const {return mirror_;}
        //@}

        /** Method that has to be overriden from AbsPolyFilter1D */
        inline unsigned dataLen() const {return nBins();}

        /** 
        // Method that has to be overriden from AbsPolyFilter1D.
        // If the bandwidth is not fixed, the value returned by
        // "selfContribution" will change after each call to "smooth"
        // or "weightedSmooth".
        */
        inline double selfContribution(unsigned) const {return filter0_;}

    private:
        /** Method that has to be overriden from AbsMarginalSmootherBase */
        virtual void smoothHisto(HistoND<double>& histo,
                                 double effectiveSampleSize,
                                 double* bandwidthUsed,
                                 bool isSampleWeighted);

        double pluginBandwidth(const HistoND<double>& histo,
                               double effectiveSampleSize);
        void makeKernel(double actualBandwidth);

        ConvolutionEngine1D* engine_;
        std::vector<double> databuf_;
        std::vector<double> taper_;
        int symbetaPower_;
        unsigned kernelOrder_;
        double fixedBandwidth_;
        double bwFactor_;
        double filter0_;
        bool mirror_;
    };
}

#endif // NPSTAT_CONSTANTBANDWIDTHSMOOTHER1D_HH_
