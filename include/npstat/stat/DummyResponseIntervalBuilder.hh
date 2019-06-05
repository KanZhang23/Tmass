#ifndef NPSTAT_DUMMYRESPONSEINTERVALBUILDER_HH_
#define NPSTAT_DUMMYRESPONSEINTERVALBUILDER_HH_

/*!
// \file DummyResponseIntervalBuilder.hh
//
// \brief Dummy implementation of AbsResponseIntervalBuilder useful for testing
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsResponseIntervalBuilder.hh"

namespace npstat {
    template <class Point>
    class DummyResponseIntervalBuilder : public AbsResponseIntervalBuilder<Point>
    {
    public:
        typedef AbsResponseIntervalBuilder<Point> B;
        typedef typename B::WeightedPointPtr WeightedPointPtr;
        typedef typename B::WeightedPtrVec WeightedPtrVec;

        inline virtual ~DummyResponseIntervalBuilder() {}

        inline Interval<double> makeResponseInterval(
            unsigned long, const double*, unsigned, const BoxND<double>&,
            std::vector<typename Point::value_type>&) const
            {return Interval<double>(0.0, 0.0);}

        inline Interval<double> makeResponseIntervalW(
            unsigned long, const double*, unsigned,
            const BoxND<double>&, const WeightedPtrVec&, unsigned) const
            {return Interval<double>(0.0, 0.0);}
    };
}

#endif // NPSTAT_DUMMYRESPONSEINTERVALBUILDER_HH_
