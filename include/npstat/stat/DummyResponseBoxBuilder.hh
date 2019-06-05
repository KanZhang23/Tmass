#ifndef NPSTAT_DUMMYRESPONSEBOXBUILDER_HH_
#define NPSTAT_DUMMYRESPONSEBOXBUILDER_HH_

/*!
// \file DummyResponseBoxBuilder.hh
//
// \brief Dummy implementation of AbsResponseBoxBuilder useful for testing
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/nm/PointDimensionality.hh"
#include "npstat/stat/AbsResponseBoxBuilder.hh"

namespace npstat {
    template <class Point>
    class DummyResponseBoxBuilder : public AbsResponseBoxBuilder<Point>
    {
    public:
        typedef AbsResponseBoxBuilder<Point> B;
        typedef typename B::WeightedPointPtr WeightedPointPtr;
        typedef typename B::WeightedPtrVec WeightedPtrVec;

        inline virtual ~DummyResponseBoxBuilder() {}

        inline BoxND<double> makeResponseBox(
            unsigned long /* uniqueId */,
            const double* /* predictorCoords */, unsigned /* nPredictors */,
            const BoxND<double>& /* predictorBox */,
            std::vector<OrderedPointND<Point> >& /* data */) const
        {
            return BoxND<double>(PointDimensionality<Point>::dim_size);
        }

        inline BoxND<double> makeResponseBoxW(
            unsigned long /* uniqueId */,
            const double* /* predictorCoords */, unsigned /* nPredictors */,
            const BoxND<double>& /* predBox */, const WeightedPtrVec& /*data*/,
            const unsigned* /* dimsToUse */, const unsigned nDimsToUse) const
        {
            return BoxND<double>(nDimsToUse);
        } 
    };
}

#endif // NPSTAT_DUMMYRESPONSEBOXBUILDER_HH_
