#ifndef NPSTAT_VARIABLEBANDWIDTHSMOOTHER1D_HH_
#define NPSTAT_VARIABLEBANDWIDTHSMOOTHER1D_HH_

/*!
// \file VariableBandwidthSmoother1D.hh
//
// \brief Variable bandwidth smoother inheriting from AbsMarginalSmootherBase
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsMarginalSmootherBase.hh"

namespace npstat {
    class VariableBandwidthSmoother1D : public AbsMarginalSmootherBase
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
        // bwFactor           -- Fudge factor for the bandwidth used for
        //                       the density estimate.
        //
        // label              -- Label for the axis. Useful in case
        //                       smoothing results are stored for inspection.
        */
        VariableBandwidthSmoother1D(unsigned nbins, double xmin, double xmax,
                                    int symbetaPower, double bwFactor = 1.0,
                                    const char* label = 0);

        inline virtual ~VariableBandwidthSmoother1D() {}

        //@{
        /** Simple inspector of object properties */
        inline int symbetaPower() const {return symbetaPower_;}
        inline double bwFactor() const {return bwFactor_;}
        //@}

    private:
        virtual void smoothHisto(HistoND<double>& histo,
                                 double effectiveSampleSize,
                                 double* bandwidthUsed,
                                 bool isSampleWeighted);
        std::vector<double> databuf_;
        int symbetaPower_;
        double bwFactor_;
    };
}

#endif // NPSTAT_VARIABLEBANDWIDTHSMOOTHER1D_HH_
