#ifndef NPSTAT_ABSDISTRO1DBUILDER_HH_
#define NPSTAT_ABSDISTRO1DBUILDER_HH_

/*!
// \file AbsDistro1DBuilder.hh
//
// \brief Interface definition for classes which reconstruct univariate
//        distrubutions from data samples
//
// Author: I. Volobouev
//
// July 2015
*/

#include <vector>
#include <utility>

#include "npstat/nm/BoxND.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // Interface definition for classes which reconstruct univariate
    // distrubutions out of data samples (typically, by fitting or smoothing)
    */
    template <class Point>
    class AbsDistro1DBuilder
    {
    public:
        typedef AbsDistribution1D result_type;

        // Second element of the pairs defined below is the weight
        typedef std::pair<const Point*, double> WeightedPointPtr;
        typedef std::vector<WeightedPointPtr> WeightedPtrVec;
        typedef std::pair<typename Point::value_type, double> WeightedValue;
        typedef std::vector<WeightedValue> WeightedValueVec;

        inline virtual ~AbsDistro1DBuilder() {}

        /**
        // This method will build an AbsDistribution1D
        // object on the heap using a sample of points. It is
        // a responsibility of the user to delete such an object
        // when it is no longer needed.
        */
        virtual AbsDistribution1D* build(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox,
            std::vector<typename Point::value_type>& data) const;

        /**
        // This method will build an AbsDistribution1D
        // object on the heap using a sample of weighted points.
        // It is a responsibility of the user to delete such
        // an object when it is no longer needed.
        */
        virtual AbsDistribution1D* buildWeighted(
            unsigned long uniqueId,
            const double* predictorCoords, unsigned nPredictors,
            const BoxND<double>& predictorBox, const WeightedPtrVec& data,
            unsigned responseDimToUse) const;

    private:
        // The following two methods should return an empty
        // interval in case such an interval is not needed
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

        // The following methods should construct the relevant
        // objects on the heap
        virtual AbsDistribution1D* buildDistro(
            unsigned long uniqueId,
            const Interval<double>& responseRange,
            const std::vector<typename Point::value_type>& data) const = 0;

        virtual AbsDistribution1D* buildDistroW(
            unsigned long uniqueId, unsigned responseDimToUse,
            const Interval<double>& responseRange,
            const WeightedValueVec& data) const = 0;
    };
}

#include "npstat/stat/AbsDistro1DBuilder.icc"

#endif // NPSTAT_ABSDISTRO1DBUILDER_HH_
