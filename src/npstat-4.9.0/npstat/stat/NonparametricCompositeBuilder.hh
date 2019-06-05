#ifndef NPSTAT_NONPARAMETRICCOMPOSITEBUILDER_HH_
#define NPSTAT_NONPARAMETRICCOMPOSITEBUILDER_HH_

/*!
// \file NonparametricCompositeBuilder.hh
//
// \brief Density estimation decomposed: do it separately for the marginals
//        and the copula
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/AbsCompositeDistroBuilder.hh"
#include "npstat/stat/AbsMarginalSmootherBase.hh"
#include "npstat/stat/AbsCopulaSmootherBase.hh"
#include "npstat/stat/AbsResponseBoxBuilder.hh"

namespace npstat {
    /**
    // A class for building nonparametric estimates of multivariate
    // densities by performing separate density estimation for each
    // marginal and the copula. This allows us to introduce a larger
    // number of bandwidth parameters into the system which can
    // often lead to a better estimate.
    */
    template <class Point>
    class NonparametricCompositeBuilder : 
        public AbsCompositeDistroBuilder<Point>
    {
    public:
        typedef AbsCompositeDistroBuilder<Point> B;
        typedef typename B::WeightedPointPtr WeightedPointPtr;
        typedef typename B::WeightedPtrVec WeightedPtrVec;
        typedef typename B::WeightedValue WeightedValue;
        typedef typename B::WeightedValueVec WeightedValueVec;

        /**
        // Constructor arguments are as follows:
        //
        // smoothND              -- copula smoother pointer
        //
        // smooth1D              -- array of pointers for margin smoothers
        //
        // boxBuilder            -- pointer for the response box builder
        //
        // takePointerOwnership  -- if true, delete smoothND, boxBuilder,
        //                          and pointers inside smooth1D in the
        //                          destructor
        //
        // interpolationDegreeCopula    -- interpolation degree for the
        //                                 copula density array. Currently
        //                                 must be 0 or 1, limited by the
        //                                 abilities of BinnedDensityND
        //                                 class.
        //
        // interpolationDegreeMarginals -- interpolation degree for the
        //                                 marginal density array. Currently
        //                                 must be 0 or 1, limited by the
        //                                 abilities of BinnedDensity1D
        //                                 class.
        */
        NonparametricCompositeBuilder(
            AbsCopulaSmootherBase* smoothND,
            const std::vector<AbsMarginalSmootherBase*>& smooth1D,
            AbsResponseBoxBuilder<Point>* boxBuilder,
            bool takePointerOwnership,
            unsigned interpolationDegreeCopula = 1U,
            unsigned interpolationDegreeMarginals = 1U);

        virtual ~NonparametricCompositeBuilder();

        /** Bandwidth factor used in the most recent copula smoothing */
        inline double lastCopulaBandwidth() const
            {return copulaBandwidth_;}

        /** Bandwidth used in the most recent smoothing of the marginals */
        inline double lastMarginBandwidth(const unsigned i) const
            {return marginBandwidth_.at(i);}

        /** Dimensionality of the reconstructed density */
        inline unsigned dim() const {return smooth1D_.size();}

        /** Set the archive for storing the histograms */
        void setArchive(gs::AbsArchive* ar, const char* category = 0);

    private:
        NonparametricCompositeBuilder();
        NonparametricCompositeBuilder(const NonparametricCompositeBuilder&);
        NonparametricCompositeBuilder& operator=(
            const NonparametricCompositeBuilder&);

        virtual BoxND<double> makeResponseBox(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox,
            std::vector<OrderedPointND<Point> >& data) const;

        virtual BoxND<double> makeResponseBoxW(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox, const WeightedPtrVec& data,
            const unsigned* dimsToUse, unsigned nDimsToUse) const;

        virtual AbsDistribution1D* buildMarginal(
            unsigned long uniqueId, unsigned dimNumber,
            const Interval<double>& responseRange,
            const std::vector<typename Point::value_type>&) const;

        virtual AbsDistributionND* buildCopula(
            unsigned long uniqueId, std::vector<OrderedPointND<Point> >&) const;

        virtual AbsDistribution1D* buildMarginalW(
            unsigned long uniqueId, unsigned dimUsed, unsigned dimNumber,
            const Interval<double>& responseRange,
            const WeightedValueVec& data) const;

        virtual AbsDistributionND* buildCopulaW(unsigned long uniqueId,
                                                const WeightedPtrVec& data,
                                                const unsigned* dimsToUse,
                                                unsigned nDimsToUse) const;
        AbsCopulaSmootherBase* smoothND_;
        std::vector<AbsMarginalSmootherBase*> smooth1D_;
        AbsResponseBoxBuilder<Point>* boxBuilder_;
        mutable std::vector<double> marginBandwidth_;
        mutable double copulaBandwidth_;
        unsigned iDegCopula_;
        unsigned iDegMarginals_;
        bool takePointerOwnership_;
    };
}

#include "npstat/stat/NonparametricCompositeBuilder.icc"

#endif // NPSTAT_NONPARAMETRICCOMPOSITEBUILDER_HH_
