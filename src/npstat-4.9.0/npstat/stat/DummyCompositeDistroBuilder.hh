#ifndef NPSTAT_DUMMYCOMPOSITEDISTROBUILDER_HH_
#define NPSTAT_DUMMYCOMPOSITEDISTROBUILDER_HH_

/*!
// \file DummyCompositeDistroBuilder.hh
//
// \brief An implementation of AbsCompositeDistroBuilder useful for testing
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/nm/PointDimensionality.hh"
#include "npstat/stat/AbsCompositeDistroBuilder.hh"

namespace npstat {
    template <class Point>
    class DummyCompositeDistroBuilder : public AbsCompositeDistroBuilder<Point>
    {
    public:
        typedef AbsCompositeDistroBuilder<Point> B;
        typedef typename B::WeightedPtrVec WeightedPtrVec;
        typedef typename B::WeightedValueVec WeightedValueVec;

        inline virtual ~DummyCompositeDistroBuilder() {}

        inline CompositeDistributionND* build(
            unsigned long, const double*, unsigned,
            const BoxND<double>&, std::vector<OrderedPointND<Point> >&) const
            {return 0;}

        inline CompositeDistributionND* buildWeighted(
            unsigned long, const double*, unsigned,
            const BoxND<double>&, const WeightedPtrVec&,
            const unsigned*, unsigned) const
            {return 0;}

    private:
        inline BoxND<double> makeResponseBox(
            unsigned long, const double*, unsigned,
            const BoxND<double>&, std::vector<OrderedPointND<Point> >&) const
            {return BoxND<double>(PointDimensionality<Point>::dim_size);}
        inline BoxND<double> makeResponseBoxW(
            unsigned long, const double*, unsigned,
            const BoxND<double>&, const WeightedPtrVec&,
            const unsigned*, const unsigned nDimsToUse) const
            {return BoxND<double>(nDimsToUse);}
        inline AbsDistribution1D* buildMarginal(
            unsigned long, unsigned, const Interval<double>&,
            const std::vector<typename Point::value_type>&) const
            {return 0;}
        inline AbsDistributionND* buildCopula(
            unsigned long, std::vector<OrderedPointND<Point> >&) const
            {return 0;}
        inline AbsDistribution1D* buildMarginalW(
            unsigned long, unsigned, unsigned,
            const Interval<double>&, const WeightedValueVec&) const
            {return 0;}
        inline AbsDistributionND* buildCopulaW(
            unsigned long, const WeightedPtrVec&,
            const unsigned*, unsigned) const
            {return 0;}
    };
}

#endif // NPSTAT_DUMMYCOMPOSITEDISTROBUILDER_HH_
