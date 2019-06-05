#ifndef NPSTAT_DUMMYDISTRO1DBUILDER_HH_
#define NPSTAT_DUMMYDISTRO1DBUILDER_HH_

/*!
// \file DummyDistro1DBuilder.hh
//
// \brief An implementation of AbsDistro1DBuilder useful for testing
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsDistro1DBuilder.hh"

namespace npstat {
    template <class Point>
    class DummyDistro1DBuilder : public AbsDistro1DBuilder<Point>
    {
    public:
        typedef AbsDistro1DBuilder<Point> B;
        typedef typename B::WeightedPtrVec WeightedPtrVec;
        typedef typename B::WeightedValueVec WeightedValueVec;

        inline virtual ~DummyDistro1DBuilder() {}

        inline AbsDistribution1D* build(
            unsigned long, const double*, unsigned, const BoxND<double>&,
            std::vector<typename Point::value_type>&) const
            {return 0;}

        inline AbsDistribution1D* buildWeighted(
            unsigned long, const double*, unsigned,
            const BoxND<double>&, const WeightedPtrVec&, unsigned) const
            {return 0;}

    private:
        inline Interval<double> makeResponseInterval(
            unsigned long, const double*, unsigned, const BoxND<double>&,
            std::vector<typename Point::value_type>&) const
            {return Interval<double>(0.0, 0.0);}
        inline Interval<double> makeResponseIntervalW(
            unsigned long, const double*, unsigned, const BoxND<double>&,
            const WeightedPtrVec&, unsigned) const
            {return Interval<double>(0.0, 0.0);}
        inline AbsDistribution1D* buildDistro(
            unsigned long, const Interval<double>&,
            const std::vector<typename Point::value_type>&) const
            {return 0;}
        inline AbsDistribution1D* buildDistroW(
            unsigned long, unsigned,
            const Interval<double>&, const WeightedValueVec&) const
            {return 0;}
    };
}

#endif // NPSTAT_DUMMYDISTRO1DBUILDER_HH_
