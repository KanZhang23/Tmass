#ifndef NPSTAT_ABSCOMPOSITEDISTROBUILDER_HH_
#define NPSTAT_ABSCOMPOSITEDISTROBUILDER_HH_

/*!
// \file AbsCompositeDistroBuilder.hh
//
// \brief Interface definition for classes which build composite distrubutions
//
// Author: I. Volobouev
//
// September 2010
*/

#include <vector>
#include <utility>

#include "npstat/nm/BoxND.hh"

#include "npstat/stat/OrderedPointND.hh"
#include "npstat/stat/CompositeDistributionND.hh"

namespace npstat {
    /**
    // Interface definition for classes which build composite distrubutions
    // out of data samples (typically, by fitting or smoothing) -- see header
    // CompositeDistributionND.hh for more details on these distributions.
    */
    template <class Point>
    class AbsCompositeDistroBuilder
    {
    public:
        typedef CompositeDistributionND result_type;

        // Second element of the pairs defined below is the weight
        typedef std::pair<const Point*, double> WeightedPointPtr;
        typedef std::vector<WeightedPointPtr> WeightedPtrVec;
        typedef std::pair<typename Point::value_type, double> WeightedValue;
        typedef std::vector<WeightedValue> WeightedValueVec;

        inline virtual ~AbsCompositeDistroBuilder() {}

        /**
        // This method will build a CompositeDistributionND
        // object on the heap using a sample of points. It is
        // a responsibility of the user to delete such an object
        // when it is no longer needed.
        */
        virtual CompositeDistributionND* build(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox,
            std::vector<OrderedPointND<Point> >& data) const;

        /**
        // This method will build a CompositeDistributionND
        // object on the heap using a sample of weighted points.
        // It is a responsibility of the user to delete such
        // an object when it is no longer needed.
        */
        virtual CompositeDistributionND* buildWeighted(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox, const WeightedPtrVec& data,
            const unsigned* dimsToUse, unsigned nDimsToUse) const;

    private:
        // The following two methods should simply return an empty
        // box (constructed with the default constructor) in case
        // the box is not needed
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

        // The following methods should construct the relevant
        // objects on the heap
        virtual AbsDistribution1D* buildMarginal(
            unsigned long uniqueId, unsigned dimNumber,
            const Interval<double>& responseRange,
            const std::vector<typename Point::value_type>&) const = 0;

        virtual AbsDistributionND* buildCopula(
            unsigned long uniqueId,
            std::vector<OrderedPointND<Point> >&) const = 0;

        virtual AbsDistribution1D* buildMarginalW(
            unsigned long uniqueId, unsigned dimUsed,
            unsigned dimNumber, const Interval<double>& responseRange,
            const WeightedValueVec& data) const = 0;

        virtual AbsDistributionND* buildCopulaW(unsigned long uniqueId,
                                                const WeightedPtrVec& data,
                                                const unsigned* dimsToUse,
                                                unsigned nDimsToUse) const = 0;
    };
}

#include "npstat/stat/AbsCompositeDistroBuilder.icc"

#endif // NPSTAT_ABSCOMPOSITEDISTROBUILDER_HH_
