#ifndef NPSTAT_ABSRESPONSEINTERVALBUILDER_HH_
#define NPSTAT_ABSRESPONSEINTERVALBUILDER_HH_

/*!
// \file AbsResponseIntervalBuilder.hh
//
// \brief Base class for building response boxes for univariate density
//        estimation in the regression context
//
// Author: I. Volobouev
//
// July 2015
*/

#include <vector>
#include <utility>

#include "npstat/nm/Interval.hh"

namespace npstat {
    template <class Point>
    class AbsResponseIntervalBuilder
    {
    public:
        typedef std::pair<const Point*, double> WeightedPointPtr;
        typedef std::vector<WeightedPointPtr> WeightedPtrVec;

        inline virtual ~AbsResponseIntervalBuilder() {}

        virtual Interval<double> makeResponseInterval(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox,
            std::vector<typename Point::value_type>& data) const = 0;

        virtual Interval<double> makeResponseIntervalW(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox, const WeightedPtrVec& data,
            unsigned responseDimToUse) const = 0;
    };
}

#endif // NPSTAT_ABSRESPONSEINTERVALBUILDER_HH_
