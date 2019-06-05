#include <cstring>
#include <cassert>
#include <stdexcept>

#include "npstat/stat/ConstantBandwidthSmoother1D.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/arrayStats.hh"
#include "npstat/stat/amiseOptimalBandwidth.hh"

#include "npstat/nm/ConvolutionEngine1D.hh"
#include "npstat/nm/OrthoPoly1D.hh"

using namespace std;

namespace npstat {
    ConstantBandwidthSmoother1D::ConstantBandwidthSmoother1D(
        const unsigned nbins, const double xmin, const double xmax,
        const int symbetaPower, const unsigned kernelOrder,
        const double bandwidth, const double bwFactor,
        const bool useMirror, const char* label)
        : AbsMarginalSmootherBase(nbins, xmin, xmax, label),
          engine_(0),
          databuf_(2U*nbins),
          symbetaPower_(symbetaPower),
          kernelOrder_(kernelOrder),
          fixedBandwidth_(bandwidth),
          bwFactor_(bwFactor),
          filter0_(0.0),
          mirror_(useMirror)
    {
        if (nbins < 2U) throw std::invalid_argument(
            "In npstat::ConstantBandwidthSmoother1D constructor: "
            "must have at least two bins");
        if (fixedBandwidth_ < 0.0) throw std::invalid_argument(
            "In npstat::ConstantBandwidthSmoother1D constructor: "
            "bandwidth must be non-negative");
        if (bwFactor_ <= 0.0) throw std::invalid_argument(
            "In npstat::ConstantBandwidthSmoother1D constructor: "
            "bandwidth factor must be positive");

        if (kernelOrder_ < 2)
            kernelOrder_ = 2;
        if (kernelOrder_ % 2)
            --kernelOrder_;

        const unsigned degree = kernelOrder_ - 2U;
        taper_.reserve(degree+1);
        for (unsigned i=0; i<=degree; ++i)
            taper_.push_back(1.0);

        engine_ = new ConvolutionEngine1D(2U*nbins);

        if (fixedBandwidth_ > 0.0)
            makeKernel(fixedBandwidth_*bwFactor_);
    }

    ConstantBandwidthSmoother1D::~ConstantBandwidthSmoother1D()
    {
        delete engine_;
    }

    void ConstantBandwidthSmoother1D::smoothHisto(
        HistoND<double>& histo, const double effectiveSampleSize,
        double* bandwidthUsed, bool /* isSampleWeighted */)
    {
        // Make sure histogram is compatible with our assumptions
        if (histo.dim() != 1U) throw std::invalid_argument(
            "In npstat::ConstantBandwidthSmoother1D::smoothHistogram: "
            "histogram to smooth must be one-dimensional");
        const unsigned nintervals = histo.nBins();
        if (databuf_.size() != 2U*nintervals) throw std::invalid_argument(
            "In npstat::ConstantBandwidthSmoother1D::smoothHistogram: "
            "incompatible number of histogram bins");

        // Calculate the bandwidth to use. Build the kernel if necessary.
        double bw = fixedBandwidth_*bwFactor_;
        if (bw == 0.0)
        {
            bw = pluginBandwidth(histo, effectiveSampleSize)*bwFactor_;
            makeKernel(bw);
        }
        if (bandwidthUsed)
            *bandwidthUsed = bw;

        // Pad the histogram data for FFT
        double* recoData = &databuf_[0];
        double* recoData2 = recoData + nintervals;
        const double* histoData = histo.binContents().data();
        memcpy(recoData, histoData, nintervals*sizeof(double));
        if (mirror_)
            for (unsigned i=0; i<nintervals; ++i)
                recoData2[i] = recoData[nintervals - i - 1];
        else
            for (unsigned i=0; i<nintervals; ++i)
                recoData2[i] = 0.0;

        // Perform KDE
        engine_->convolveWithFilter(recoData, recoData, 2U*nintervals);

        // Chop off negative values of the reconstructed density
        // and renormalize the whole estimate
        long double integ = 0.0L;
        for (unsigned i=0; i<nintervals; ++i)
        {
            if (recoData[i] < 0.0) recoData[i] = 0.0;
            integ += recoData[i];
        }
        const double denom = static_cast<double>(integ)*histo.binVolume();
        if (denom == 0.0) throw std::domain_error(
            "In npstat::ConstantBandwidthSmoother1D::smoothHistogram: "
            "density integral is zero so it can not be normalized");
        for (unsigned i=0; i<nintervals; ++i)
            recoData[i] /= denom;

        // Fill the histogram from the estimate
        histo.setBinContents(recoData, nintervals);
    }

    void ConstantBandwidthSmoother1D::makeKernel(const double bandwidth)
    {
        const unsigned nkde = databuf_.size();
        const unsigned grid_points = nkde/2U;
        double* kernelData = &databuf_[0];

        TruncatedGauss1D gauss(0.0, bandwidth, 12.0);
        SymmetricBeta1D symbeta(0.0, bandwidth,
                                symbetaPower_ < 0 ? 0 : symbetaPower_);
        AbsScalableDistribution1D* kernel = symbetaPower_ < 0 ?
            dynamic_cast<AbsScalableDistribution1D*>(&gauss) :
            dynamic_cast<AbsScalableDistribution1D*>(&symbeta);
        assert(kernel);

        // Scan the kernel (assuming it is symmetric)
        const unsigned center = grid_points - 1U;
        unsigned iscan = 0;
        const double grid_step = this->binWidth();
        for (; iscan<grid_points; ++iscan)
        {
            const double val = kernel->density(iscan*grid_step);
            if (val == 0.0)
                break;
            kernelData[center + iscan] = val;
            kernelData[center - iscan] = val;
        }
        if (iscan == grid_points)
            --iscan;

        // Prepare the filter
        const unsigned degree = kernelOrder_ - 2U;
        const unsigned nscan = 2*iscan + 1;
        OrthoPoly1D poly(degree, &kernelData[center-iscan], nscan, grid_step);
        poly.linearFilter(&taper_[0], degree, iscan, kernelData, nscan);
        for (unsigned i=nscan; i<nkde; ++i)
            kernelData[i] = 0.0;

        filter0_ = kernelData[iscan];

        // Transform the filter
        engine_->setFilter(kernelData, nkde, iscan);
    }

    double ConstantBandwidthSmoother1D::pluginBandwidth(
        const HistoND<double>& histo, const double effectiveSampleSize)
    {
        // Estimate the scale parameter
        const double inp[2] = {0.15865525393145705, 0.84134474606854295};
        double quantiles[2];
        arrayQuantiles1D(histo.binContents().data(), histo.nBins(),
                         histo.axis(0U).min(), histo.axis(0U).max(),
                         inp, quantiles, 2U);
        const double sigma = (quantiles[1] - quantiles[0])/2.0;

        // Guess the bandwidth using Gaussian plug-in
        const unsigned filterDeg = kernelOrder_ - 2U;
        const double amiseBw = symbetaPower_ < 0 ?
            amisePluginBwGauss(filterDeg, effectiveSampleSize, sigma) : 
            amisePluginBwSymbeta(symbetaPower_, filterDeg,
                                 effectiveSampleSize, sigma);
        return amiseBw;
    }
}
