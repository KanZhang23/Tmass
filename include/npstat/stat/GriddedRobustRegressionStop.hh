#ifndef NPSTAT_GRIDDEDROBUSTREGRESSIONSTOP_HH_
#define NPSTAT_GRIDDEDROBUSTREGRESSIONSTOP_HH_

/*!
// \file GriddedRobustRegressionStop.hh
//
// \brief A simple stopping functor for iterative local gridded robust regression
//
// Author: I. Volobouev
//
// December 2011
*/

#include "npstat/stat/AbsLossCalculator.hh"
#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /**
    // A simple stopping functor for iterative local gridded robust regression
    */
    class GriddedRobustRegressionStop : 
        public Functor2<bool,LocalLoss,unsigned long>
    {
    public:
        /**
        // The robust regression will be stopped when at least one
        // of the following conditions is satisfied:
        //
        // -- Number of iterations reaches "maxcalls"
        //
        // -- The loss function improvement due to point replacement
        //    operation becomes smaller than "lossImprovementTarget"
        //
        // -- The loss value itself becomes smaller than "lossValueTarget".
        */
        inline explicit GriddedRobustRegressionStop(unsigned long maxcalls,
                                      double lossImprovementTarget = -1.0e300,
                                      double lossValueTarget = -1.0e300)
            : lossImprovementTarget_(lossImprovementTarget),
              lossValueTarget_(lossValueTarget),
              maxcalls_(maxcalls) {}

        inline virtual ~GriddedRobustRegressionStop() {}

        inline virtual bool operator()(const LocalLoss& lastLoss,
                                       const unsigned long& nReplacements) const
        {
            return nReplacements >= maxcalls_ ||
                   lastLoss.improvement < lossImprovementTarget_ ||
                   lastLoss.value < lossValueTarget_;
        }

    private:
        double lossImprovementTarget_;
        double lossValueTarget_;
        unsigned long maxcalls_;
    };
}

#endif // NPSTAT_GRIDDEDROBUSTREGRESSIONSTOP_HH_
