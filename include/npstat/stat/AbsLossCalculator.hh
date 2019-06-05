#ifndef NPSTAT_ABSLOSSCALCULATOR_HH_
#define NPSTAT_ABSLOSSCALCULATOR_HH_

/*!
// \file AbsLossCalculator.hh
//
// \brief Interfaces and utility classes for gridded robust regression
//
// Author: I. Volobouev
//
// December 2011
*/

#include "geners/CPP11_array.hh"
#include "npstat/nm/ArrayND.hh"

namespace npstat {
    /** 
    // Class for keeping information about local regression
    // loss (e.g., in a sliding window)
    */
    struct LocalLoss
    {
        inline LocalLoss() : value(0.0), improvement(0.0) {}
        inline LocalLoss(double v, double i) : value(v), improvement(i) {}

        double value;       ///< current "loss" (e.g., chi-squared)
        double improvement; ///< possible improvement due to the point replacement

        inline bool operator<(const LocalLoss& r) const
            {return improvement < r.improvement;}
    };

    /**
    // Class for representing the information about
    // a potential point replacement in nonlinear
    // regression algorithms on rectangular grids
    */
    template <unsigned MaxDim>
    struct PointReplacement
    {
        typedef CPP11_array<unsigned,MaxDim> Index;

        inline PointReplacement() : improvedValue(0.0) {}

        double improvedValue; ///< improved loss value upon replacement
        Index coord;          ///< point index
    };

    /**
    // Class for representing the information about multiple
    // potential point replacements in nonlinear regression
    // algorithms on rectangular grids
    */
    template <unsigned MaxDim, unsigned MaxReplace>
    struct ReplacementBlock
    {
        typedef CPP11_array<unsigned,MaxDim> Index;

        inline ReplacementBlock() : nReplacements(0) {}

        /**
        // Reset this object to a meanigful state with zero
        // suggested replacements
        */
        inline void reset(const unsigned* index, const unsigned sz)
        {
            for (unsigned i=0; i<sz; ++i)
                idx[i] = index[i];
            for (unsigned i=sz; i<MaxDim; ++i)
                idx[i] = 0U;
            dim = sz;
            nReplacements = 0;
        }

        /** Index of the point for which the replacements are suggested */
        Index idx;

        /** Actual dimensionality of the point index */
        unsigned dim;

        /**
        // Collection of suggested replacements which result
        // in the corresponding loss improvement
        */
        PointReplacement<MaxDim> replacement[MaxReplace];

        /** Actual number of suggested replacements */
        unsigned nReplacements;
    };

    /** Abstract base class for calculations of the local loss */
    template <unsigned MaxDim, unsigned MaxReplace>
    struct AbsLossCalculator
    {
        inline virtual ~AbsLossCalculator() {}

        /**
        // In addition to returning the loss, the following
        // function must fill out the suggested replacement block
        */
        virtual LocalLoss operator()(
            const npstat::ArrayND<double>& slidingWindow,
            const unsigned* indexInWindow,
            const unsigned* indexInDataset,
            ReplacementBlock<MaxDim,MaxReplace>* block) const = 0;
    };
}

#endif // NPSTAT_ABSLOSSCALCULATOR_HH_
