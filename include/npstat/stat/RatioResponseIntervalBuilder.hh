#ifndef NPSTAT_RATIORESPONSEINTERVALBUILDER_HH_
#define NPSTAT_RATIORESPONSEINTERVALBUILDER_HH_

/*!
// \file RatioResponseIntervalBuilder.hh
//
// \brief Interval for a ratio response variable with a cut on the numerator
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsResponseIntervalBuilder.hh"

namespace npstat {
    template <class Point>
    class RatioResponseIntervalBuilder : public AbsResponseIntervalBuilder<Point>
    {
    public:
        typedef AbsResponseIntervalBuilder<Point> B;
        typedef typename B::WeightedPointPtr WeightedPointPtr;
        typedef typename B::WeightedPtrVec WeightedPtrVec;

        /**
        // The constructor arguments are as follows:
        //
        //  denomDim      -- the dimension number of the predictor variable
        //                   used as the ratio denominator inside the Point
        //                   class
        //
        //  numeratorCut  -- the cut imposed on the numerator of the ratio
        //                   (assuming "numerator > cut" requirement). It is
        //                   assumed that both numerator and denominator are
        //                   positive.
        //
        //  badFraction   -- the maximum fraction of weights for which the
        //                   distribution of the ratio will be distorted.
        //                   These are all the weights to the left of the
        //                   predictor value given by the ratio
        //                   numeratorCut/(chosen ratioCut).
        */
        RatioResponseIntervalBuilder(unsigned denomDim,
                                     double numeratorCut,
                                     double badFraction);

        inline virtual ~RatioResponseIntervalBuilder() {}

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

    private:
        unsigned predictorDim_;
        double numeratorCut_;
        double frac_;
    };
}

#include "npstat/stat/RatioResponseIntervalBuilder.icc"

#endif // NPSTAT_RATIORESPONSEINTERVALBUILDER_HH_
