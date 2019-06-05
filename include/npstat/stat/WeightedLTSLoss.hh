#ifndef NPSTAT_WEIGHTEDLTSLOSS_HH_
#define NPSTAT_WEIGHTEDLTSLOSS_HH_

/*!
// \file WeightedLTSLoss.hh
//
// \brief Loss function for local least trimmed squares with one excluded point
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
    // with the central point excluded. The loss improvement is estimated only
    // for the point in the window center.
    //
    // Template parameters MaxDim and MaxReplace are just like those in
    // griddedRobustRegression.hh. For use with this particular loss
    // calculator, MaxReplace value of 1 is the most appropriate. Template
    // parameter MaxDeg is the maximum local polynomial degree.
    */
    template <unsigned MaxDim, unsigned MaxReplace, unsigned MaxDeg>
    class WeightedLTSLoss : public AbsLossCalculator<MaxDim,MaxReplace>
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
        */
        inline WeightedLTSLoss(const AbsDistributionND& weight, unsigned deg)
            : weightFcn_(weight.clone()), polys_(0), degree_(deg) {}

        virtual ~WeightedLTSLoss();

        virtual LocalLoss operator()(
            const ArrayND<double>& slidingWindow,
            const unsigned* indexInWindow,
            const unsigned* indexInDataset,
            BlockReplacement* block) const;

    private:
        typedef OrthoPolyND<MaxDeg> Poly;

        WeightedLTSLoss(const WeightedLTSLoss&);
        WeightedLTSLoss& operator=(const WeightedLTSLoss&);

        void initialize(const ArrayND<double>& in);

        AbsDistributionND* weightFcn_;
        ArrayND<Poly*>* polys_;
        mutable std::vector<double> coeffs_;
        unsigned degree_;
    };
}

#include "npstat/stat/WeightedLTSLoss.icc"

#endif // NPSTAT_WEIGHTEDLTSLOSS_HH_
