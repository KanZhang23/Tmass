#ifndef NPSTAT_NONPARAMETRICDISTRO1DBUILDER_HH_
#define NPSTAT_NONPARAMETRICDISTRO1DBUILDER_HH_

/*!
// \file NonparametricDistro1DBuilder.hh
//
// \brief 1-d density estimation implementing AbsDistro1DBuilder interface
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsDistro1DBuilder.hh"
#include "npstat/stat/AbsMarginalSmootherBase.hh"
#include "npstat/stat/AbsResponseIntervalBuilder.hh"

namespace npstat {
    /**
    // A class for building conditional nonparametric estimates of univariate
    // densities
    */
    template <class Point>
    class NonparametricDistro1DBuilder : public AbsDistro1DBuilder<Point>
    {
    public:
        typedef AbsDistro1DBuilder<Point> B;
        typedef typename B::WeightedPointPtr WeightedPointPtr;
        typedef typename B::WeightedPtrVec WeightedPtrVec;
        typedef typename B::WeightedValue WeightedValue;
        typedef typename B::WeightedValueVec WeightedValueVec;

        /**
        // Constructor arguments are as follows:
        //
        // smoother1D            -- distribution smoother pointer
        //
        // intervalBuilder       -- pointer for the response interval builder
        //
        // takePointerOwnership  -- if true, delete smoother1D and
        //                          intervalBuilder in the destructor
        //
        // interpolationDegree   -- interpolation degree for the density
        //                          array. Currently must be 0 or 1, limited
        //                          by the abilities of BinnedDensity1D class.
        */
        NonparametricDistro1DBuilder(
            AbsMarginalSmootherBase* smoother1D,
            AbsResponseIntervalBuilder<Point>* intervalBuilder,
            bool takePointerOwnership,
            unsigned interpolationDegree = 1U);

        virtual ~NonparametricDistro1DBuilder();

        /** Bandwidth used in the most recent smoothing of the marginals */
        inline double lastBandwidth() const {return lastBandwidth_;}

        /** Set the archive for storing the histograms */
        void setArchive(gs::AbsArchive* ar, const char* category = 0);

    private:
        NonparametricDistro1DBuilder();
        NonparametricDistro1DBuilder(const NonparametricDistro1DBuilder&);
        NonparametricDistro1DBuilder& operator=(
            const NonparametricDistro1DBuilder&);

        virtual Interval<double> makeResponseInterval(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox,
            std::vector<typename Point::value_type>& data) const;

        virtual Interval<double> makeResponseIntervalW(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox, const WeightedPtrVec& data,
            unsigned responseDimToUse) const;

        virtual AbsDistribution1D* buildDistro(
            unsigned long uniqueId,
            const Interval<double>& responseRange,
            const std::vector<typename Point::value_type>&) const;

        virtual AbsDistribution1D* buildDistroW(
            unsigned long uniqueId, unsigned responseDimToUse,
            const Interval<double>& responseRange,
            const WeightedValueVec& data) const;

        AbsMarginalSmootherBase* smoother1D_;
        AbsResponseIntervalBuilder<Point>* intervalBuilder_;
        mutable double lastBandwidth_;
        unsigned iDegMarginals_;
        bool takePointerOwnership_;
    };
}

#include "npstat/stat/NonparametricDistro1DBuilder.icc"

#endif // NPSTAT_NONPARAMETRICDISTRO1DBUILDER_HH_
