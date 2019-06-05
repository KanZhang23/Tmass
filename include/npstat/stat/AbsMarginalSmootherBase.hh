#ifndef NPSTAT_ABSMARGINALSMOOTHERBASE_HH_
#define NPSTAT_ABSMARGINALSMOOTHERBASE_HH_

/*!
// \file AbsMarginalSmootherBase.hh
//
// \brief Interface definition for 1-d nonparametric density estimation
//
// Author: I. Volobouev
//
// June 2015
*/

#include <vector>
#include <utility>

#include "geners/AbsArchive.hh"
#include "npstat/stat/HistoND.hh"

namespace npstat {
    class AbsMarginalSmootherBase
    {
    public:
        /** Constructor takes binning parameters for the internal histogram */
        AbsMarginalSmootherBase(unsigned nbins, double xmin, double xmax,
                                const char* axisLabel = 0);

        inline virtual ~AbsMarginalSmootherBase() {delete histo_;}

        void setAxisLabel(const char* axisLabel);

        //@{
        /** Simple inspector of the object properties */
        inline unsigned nBins() const {return nbins_;}
        inline double xMin() const {return xmin_;}
        inline double xMax() const {return xmax_;}
        inline double binWidth() const {return (xmax_ - xmin_)/nbins_;}
        inline const std::string& getAxisLabel() const {return axlabel_;}
        //@}

        /** Set the archive for storing the histograms */
        void setArchive(gs::AbsArchive* ar, const char* category = 0);

        /**
        // Smoothing function for unweighted samples. The region
        // used for smoothing will be defined by the overlap of
        // the intervals [minValue, maxValue) and [xMin(), xMax()).
        */
        template <typename Numeric>
        const HistoND<double>& smooth(unsigned long uniqueId,
                                      unsigned dimNumber,
                                      const std::vector<Numeric>& inputPoints,
                                      double minValue, double maxValue,
                                      double* bandwidthUsed = 0);

        /** 
        // Smoothing function for weighted samples. The region
        // used for smoothing will be defined by the overlap of
        // the intervals [minValue, maxValue) and [xMin(), xMax()).
        */
        template <typename Numeric>
        const HistoND<double>& weightedSmooth(
            unsigned long uniqueId, unsigned multivariatePointDimNumber,
            const std::vector<std::pair<Numeric,double> >& inputPoints,
            double minValue, double maxValue, double* bandwidthUsed = 0);

    private:
        AbsMarginalSmootherBase();
        AbsMarginalSmootherBase(const AbsMarginalSmootherBase&);
        AbsMarginalSmootherBase& operator=(const AbsMarginalSmootherBase&);

        HistoND<double>& clearHisto(double xmin, double xmax);
        void storeHisto(unsigned long uniqueId, unsigned dim, double bw) const;

        // Method to implement in derived classes
        virtual void smoothHisto(HistoND<double>& histo,
                                 double effectiveSampleSize,
                                 double* bandwidthUsed,
                                 bool isSampleWeighted) = 0;
        HistoND<double>* histo_;
        double xmin_;
        double xmax_;
        unsigned nbins_;
        gs::AbsArchive* ar_;
        std::string category_;
        std::string axlabel_;
    };
}

#include "npstat/stat/AbsMarginalSmootherBase.icc"

#endif // NPSTAT_ABSMARGINALSMOOTHERBASE_HH_
