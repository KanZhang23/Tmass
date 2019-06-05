#ifndef NPSTAT_TWOPOINTSLTSLOSS_HH_
#define NPSTAT_TWOPOINTSLTSLOSS_HH_

/*!
// \file TwoPointsLTSLoss.hh
//
// \brief Loss function for local least trimmed squares with two excluded points
//
// Author: I. Volobouev
//
// December 2011
*/

#include "npstat/nm/OrthoPolyND.hh"
#include "npstat/stat/AbsLossCalculator.hh"
#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    /**
    // Calculator of windowed least trimmed squares for use with gridded
    // robust regression. The chi-squared is calculated for the whole window
    // with excluded central point and arbitrary other point. The loss
    // improvement is estimated only for the point in the window center,
    // excluding that other point which results in the best local LTS.
    //
    // Template parameters MaxDim and MaxReplace are just like those in
    // griddedRobustRegression.hh. For use with this particular loss
    // calculator, MaxReplace value of 1 is the most appropriate. Template
    // parameter MaxDeg is the maximum local polynomial degree.
    */
    template <unsigned MaxDim, unsigned MaxReplace, unsigned MaxDeg>
    class TwoPointsLTSLoss : public AbsLossCalculator<MaxDim,MaxReplace>
    {
    public:
        typedef ReplacementBlock<MaxDim,MaxReplace> BlockReplacement;

        /**
        // "weight" is the function used to generate local polynomials.
        // It will be called on the local window using window coordinate
        // system. For example, the point in the window center will be
        // at the origin, and coordinates of all points neighboring the
        // origin in each dimension will be either 0 or 1. The weight
        // therefore must have an appropriate bandwidth, consistent
        // with the expected local window width. If the bandwidth is
        // too small (weight on the edges of the window is exactly 0)
        // then the code will still work but will be inefficient.
        //
        // "deg" is the actual degree of these polynomials (must not
        // exceed "maxdeg").
        //
        // If the "fixStripes" parameter is true, the local loss in the
        // window will be calculated not only by removing all possible
        // second points in addition to the central one but also by removing
        // all possible 1-d stripes of points passing through the center.
        */
        inline TwoPointsLTSLoss(const AbsDistributionND& weight, unsigned deg,
                                bool fixStripes = false)
            : wFcn_(weight.clone()), polyPairs_(0), polyLines_(0),
              degree_(deg), fixStripes_(fixStripes) {}

        virtual ~TwoPointsLTSLoss();

        virtual LocalLoss operator()(
            const ArrayND<double>& slidingWindow,
            const unsigned* indexInWindow,
            const unsigned* indexInDataset,
            BlockReplacement* block) const;

    private:
        typedef OrthoPolyND<MaxDeg> Poly;

        TwoPointsLTSLoss(const TwoPointsLTSLoss&);
        TwoPointsLTSLoss& operator=(const TwoPointsLTSLoss&);

        void initialize(const ArrayND<double>& in);

        AbsDistributionND* wFcn_;
        ArrayND<Poly*>* polyPairs_;
        ArrayND<Poly*>* polyLines_;
        ArrayShape expectedShape_;
        mutable std::vector<double> coeffs_;
        unsigned degree_;
        bool fixStripes_;
    };
}

#include "npstat/stat/TwoPointsLTSLoss.icc"

#endif // NPSTAT_TWOPOINTSLTSLOSS_HH_
