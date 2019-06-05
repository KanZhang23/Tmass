#include <stdexcept>

#include "npstat/stat/LOrPEMarginalSmoother.hh"
#include "npstat/stat/lorpeSmooth1D.hh"

namespace npstat {
    LOrPEMarginalSmoother::LOrPEMarginalSmoother(
        const unsigned nbins, const double xmin, const double xmax,
        const int isymbetaPower, const double ipolyDegree,
        const double ibwFactor, const BoundaryHandling& bm,
        const char* label)
        : AbsMarginalSmootherBase(nbins, xmin, xmax, label),
          bwFactor_(ibwFactor),
          symbetaPower_(isymbetaPower),
          polyDegree_(ipolyDegree),
          bm_(bm)
    {
        if (bwFactor_ <= 0.0) throw std::invalid_argument(
            "In npstat::LOrPEMarginalSmoother constructor: "
            "bandwidth factor must be positive");
    }

    void LOrPEMarginalSmoother::smoothHisto(
        HistoND<double>& histo, const double effectiveSampleSize,
        double* bandwidthUsed, bool /* isSampleWeighted */)
    {
        const double bandwidth = weightedLorpeSmooth1D(
            histo, effectiveSampleSize, symbetaPower_, polyDegree_,
            const_cast<double*>(histo.binContents().data()),
            histo.nBins(), bm_, bwFactor_);
        if (bandwidthUsed)
            *bandwidthUsed = bandwidth;
    }
}
