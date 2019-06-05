#include <cassert>
#include <stdexcept>

#include "npstat/stat/VariableBandwidthSmoother1D.hh"
#include "npstat/stat/variableBandwidthSmooth1D.hh"

namespace npstat {
    VariableBandwidthSmoother1D::VariableBandwidthSmoother1D(
        const unsigned nbins, const double xmin, const double xmax,
        const int symbetaPower, const double bwFactor, const char* label)
        : AbsMarginalSmootherBase(nbins, xmin, xmax, label),
          databuf_(nbins),
          symbetaPower_(symbetaPower),
          bwFactor_(bwFactor)
    {
        if (bwFactor_ <= 0.0) throw std::invalid_argument(
            "In npstat::VariableBandwidthSmoother1D constructor: "
            "bandwidth factor must be positive");
    }

    void VariableBandwidthSmoother1D::smoothHisto(
        HistoND<double>& histo, const double effectiveSampleSize,
        double* bandwidthUsed, bool /* isSampleWeighted */)
    {
        if (histo.dim() != 1U) throw std::invalid_argument(
            "In npstat::VariableBandwidthSmoother1D::smoothHistogram: "
            "histogram to smooth must be one-dimensional");
        const unsigned sz = histo.nBins();
        if (databuf_.size() != sz) throw std::invalid_argument(
            "In npstat::VariableBandwidthSmoother1D::smoothHistogram: "
            "incompatible number of histogram bins");
        double* recoData = &databuf_[0];
        const double bw = weightedVariableBandwidthSmooth1D(
            histo, effectiveSampleSize, symbetaPower_, recoData, sz, bwFactor_);
        histo.setBinContents(recoData, sz);
        if (bandwidthUsed)
            *bandwidthUsed = bw;
    }
}
