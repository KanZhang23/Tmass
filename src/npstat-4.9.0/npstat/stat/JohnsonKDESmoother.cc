#include <stdexcept>

#include "npstat/stat/JohnsonKDESmoother.hh"
#include "npstat/stat/arrayStats.hh"
#include "npstat/stat/JohnsonCurves.hh"
#include "npstat/stat/amiseOptimalBandwidth.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/WeightTableFilter1DBuilder.hh"
#include "npstat/stat/variableBandwidthSmooth1D.hh"
#include "npstat/stat/DensityScan1D.hh"

namespace npstat {
    JohnsonKDESmoother::JohnsonKDESmoother(
        const unsigned nbins, const double xmin, const double xmax,
        const int isymbetaPower, const double ibwFactor, const char* label)
        : AbsMarginalSmootherBase(nbins, xmin, xmax, label),
          bwFactor_(ibwFactor),
          symbetaPower_(isymbetaPower),
          scan_(makeShape(nbins)),
          pilot_(makeShape(nbins))
    {
        if (bwFactor_ <= 0.0) throw std::invalid_argument(
            "In npstat::JohnsonKDESmoother constructor: "
            "bandwidth factor must be positive");
    }

    void JohnsonKDESmoother::smoothHisto(
        HistoND<double>& histo, const double effectiveSampleSize,
        double* bandwidthUsed, bool /* isSampleWeighted */)
    {
        if (histo.dim() != 1U) throw std::invalid_argument(
            "In npstat::JohnsonKDESmoother::smoothHistogram: "
            "histogram must be one-dimensional");

        // We will use LOrPE of 0th degree (equivalent to KDE)
        const unsigned filterDeg = 0;

        const double xmin = histo.axis(0).min();
        const double xmax = histo.axis(0).max();
        const double range = xmax - xmin;

        // Figure out the shape parameters of the histogram
        double mean, sigma, skew, kurt;
        arrayShape1D(histo.binContents(), xmin, xmax,
                     &mean, &sigma, &skew, &kurt);

        // Fit the Johnson system to the distribution
        JohnsonSystem johns(mean, sigma, skew, kurt);
        if (!johns.isValid()) throw std::runtime_error(
            "In npstat::JohnsonKDESmoother::smoothHistogram: "
            "failed to fit Johnson system to the input data");

        // Scan the Johnson system
        const unsigned nbins = scan_.length();
        scan_.functorFill(DensityScan1D(johns, 1.0, nbins, xmin, xmax));

        // Figure out AMISE-optimal bandwidth for the reference
        // Johnson system and Gaussian or Symmetric Beta kernel
        const double binwidth = range/nbins;
        const double amiseBw = symbetaPower_ < 0 ?
            amiseOptimalBwGauss(
                filterDeg, effectiveSampleSize,
                const_cast<double*>(scan_.data()), nbins, binwidth) :
            amiseOptimalBwSymbeta(
                symbetaPower_, filterDeg, effectiveSampleSize, 
                const_cast<double*>(scan_.data()), nbins, binwidth);

        // Bandwidth for the pilot estimate
        const double pilotBw = amiseBw*bwFactor_;

        // Scan the kernel using the pilot bandwidth
        Gauss1D gauss(binwidth/2.0, pilotBw);
        SymmetricBeta1D symbeta(binwidth/2.0, pilotBw,
                                symbetaPower_ < 0 ? 0 : symbetaPower_);
        AbsScalableDistribution1D* kernel = symbetaPower_ < 0 ?
            dynamic_cast<AbsScalableDistribution1D*>(&gauss) :
            dynamic_cast<AbsScalableDistribution1D*>(&symbeta);
        scan_.functorFill(DensityScan1D(*kernel, 1.0, nbins, 0.0, range));

        // Figure out when the scanned density is becoming 0
        unsigned bincount = 0;
        for (; bincount<nbins && scan_.linearValue(bincount)>0.0; ++bincount);

        // Build the KDE / LOrPE filter for the pilot density estimate
        WeightTableFilter1DBuilder fbuilder(scan_.data(), bincount);
        LocalPolyFilter1D f(0, filterDeg, fbuilder, nbins);

        // Build the pilot estimate by applying the filter
        f.filter(histo.binContents().data(), nbins,
                 const_cast<double*>(pilot_.data()));

        // Build the final estimate using KDE with adaptive bandwidth
        kernel->setLocation(0.0);
        kernel->setScale(1.0);
        variableBandwidthSmooth1D(
            histo, pilot_.data(), pilot_.length(),
            *kernel, pilotBw, 0.5, false,
            const_cast<double*>(scan_.data()), nbins);

        // Fill the histogram from the estimate
        histo.setBinContents(scan_.data(), nbins);

        // Fill the output bandwidth
        if (bandwidthUsed)
            *bandwidthUsed = pilotBw;
    }
}
