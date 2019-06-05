#ifndef NPSTAT_JOHNSONKDESMOOTHER_HH_
#define NPSTAT_JOHNSONKDESMOOTHER_HH_

/*!
// \file JohnsonKDESmoother.hh
//
// \brief Adaptive bandwidth KDE for 1-d unimodal densities
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/AbsMarginalSmootherBase.hh"

namespace npstat {
    /**
    // 1-d KDE implementation with adaptive bandwidth. The adaptive bandwidth
    // for each data point is set in inverse proportion to the square root of
    // the pilot density at that point. The pilot density bandwidth is
    // calculated with Johnson system densities used as reference distributions
    // for AMISE optimization. The fit of Johnson densities to the data sample
    // is done by matching sample moments to the curve moments.
    */
    class JohnsonKDESmoother : public AbsMarginalSmootherBase
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        // nbins, xmin, xmax  -- Parameters for the histogram which will
        //                       be accumulated using the data sample to be
        //                       smoothed.
        //
        // symbetaPower       -- Power of the symmetric beta kernel to use.
        //                       Gaussian kernel will be used in case
        //                       this parameter is negative. This parameter
        //                       must not exceed 10.
        //
        // bwFactor           -- Fudge factor for the plugin bandwidth used
        //                       for the pilot and final density estimates.
        //
        // label              -- Label for the axis. Useful in case
        //                       smoothing results are stored for inspection.
        */
        JohnsonKDESmoother(unsigned nbins, double xmin, double xmax,
                           int symbetaPower, double bwFactor=1.0,
                           const char* label = 0);

        inline virtual ~JohnsonKDESmoother() {}

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
        double bwFactor_;
        int symbetaPower_;

        // Useful workspace (1-d arrays with number of elements equal to nbins)
        ArrayND<double> scan_;
        ArrayND<double> pilot_;
    };
}

#endif // NPSTAT_JOHNSONKDESMOOTHER_HH_
