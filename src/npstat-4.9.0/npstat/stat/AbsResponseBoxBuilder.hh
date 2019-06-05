#ifndef NPSTAT_ABSRESPONSEBOXBUILDER_HH_
#define NPSTAT_ABSRESPONSEBOXBUILDER_HH_

/*!
// \file AbsResponseBoxBuilder.hh
//
// \brief Base class for building response boxes for multivariate density
//        estimation in regression context
//
// Author: I. Volobouev
//
// July 2015
*/

#include <utility>

#include "npstat/nm/BoxND.hh"
#include "npstat/nm/PointDimensionality.hh"

#include "npstat/stat/OrderedPointND.hh"

namespace npstat {
    template <class Point>
    class AbsResponseBoxBuilder
    {
    public:
        typedef std::pair<const Point*, double> WeightedPointPtr;
        typedef std::vector<WeightedPointPtr> WeightedPtrVec;

        inline virtual ~AbsResponseBoxBuilder() {}

        virtual BoxND<double> makeResponseBox(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox,
            std::vector<OrderedPointND<Point> >& data) const = 0;

        virtual BoxND<double> makeResponseBoxW(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox, const WeightedPtrVec& data,
            const unsigned* dimsToUse, unsigned nDimsToUse) const = 0;
    };
}

#endif // NPSTAT_ABSRESPONSEBOXBUILDER_HH_
