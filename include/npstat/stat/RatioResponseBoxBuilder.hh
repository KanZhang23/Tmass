#ifndef NPSTAT_RATIORESPONSEBOXBUILDER_HH_
#define NPSTAT_RATIORESPONSEBOXBUILDER_HH_

/*!
// \file RatioResponseBoxBuilder.hh
//
// \brief Box for a ratio response variable with a cut on the numerator
//
// Author: I. Volobouev
//
// July 2015
*/

#include "npstat/stat/AbsResponseBoxBuilder.hh"

namespace npstat {
    template <class Point>
    class RatioResponseBoxBuilder : public AbsResponseBoxBuilder<Point>
    {
    public:
        typedef AbsResponseBoxBuilder<Point> B;
        typedef typename B::WeightedPointPtr WeightedPointPtr;
        typedef typename B::WeightedPtrVec WeightedPtrVec;

        /**
        // The constructor arguments are as follows:
        //
        //  denomDim      -- the dimension number of the predictor variable
        //                   used as the ratio denominator inside the Point
        //                   class
        //
        //  ratioDim      -- the dimension number (inside the Point class)
        //                   of the ratio variable itself
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
        RatioResponseBoxBuilder(unsigned denomDim,
                                unsigned ratioDim,
                                double numeratorCut,
                                double badFraction);

        inline virtual ~RatioResponseBoxBuilder() {}

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

    private:
        unsigned predictorDim_;
        unsigned ratioDim_;
        double numeratorCut_;
        double frac_;
    };
}

#include "npstat/stat/RatioResponseBoxBuilder.icc"

#endif // NPSTAT_RATIORESPONSEBOXBUILDER_HH_
