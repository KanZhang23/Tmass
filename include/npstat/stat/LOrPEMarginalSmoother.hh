#ifndef NPSTAT_LORPEMARGINALSMOOTHER_HH_
#define NPSTAT_LORPEMARGINALSMOOTHER_HH_

/*!
// \file LOrPEMarginalSmoother.hh
//
// \brief LOrPE adapter for automatic smoothing of 1-d marginals
//
// Author: I. Volobouev
//
// March 2013
*/

#include "npstat/stat/AbsMarginalSmootherBase.hh"
#include "npstat/stat/BoundaryHandling.hh"

namespace npstat {
    class LOrPEMarginalSmoother : public AbsMarginalSmootherBase
    {
    public:
        LOrPEMarginalSmoother(unsigned nbins, double xmin, double xmax,
                              int symbetaPower, double polyDegree,
                              double bandwidthfactor,
                              const BoundaryHandling& bm,
                              const char* label = 0);

        inline virtual ~LOrPEMarginalSmoother() {}

        //@{
        /** Inspect object properties */
        inline int symbetaPower() const {return symbetaPower_;}
        inline double bwFactor() const {return bwFactor_;}
        inline double polyDegree() const {return polyDegree_;}
        inline const BoundaryHandling& boundaryMethod() const {return bm_;}
        //@}

    private:
        virtual void smoothHisto(HistoND<double>& histo,
                                 double effectiveSampleSize,
                                 double* bandwidthUsed,
                                 bool isSampleWeighted);
        double bwFactor_;
        int symbetaPower_; 
        double polyDegree_;
        BoundaryHandling bm_;
    };
}

#endif // NPSTAT_LORPEMARGINALSMOOTHER_HH_
