#ifndef NPSTAT_WEIGHTTABLEFILTER1DBUILDER_HH_
#define NPSTAT_WEIGHTTABLEFILTER1DBUILDER_HH_

/*!
// \file WeightTableFilter1DBuilder.hh
//
// \brief Build local polynomial filters using scanned tables of values
//
// The filters are intended for use with the "LocalPolyFilter1D" class.
// Boundary is handled by simple truncation of the weight.
//
// Author: I. Volobouev
//
// November 2009
*/

#include "npstat/stat/AbsFilter1DBuilder.hh"

namespace npstat {
    /**
    // This class will construct a local polynomial filter out of
    // an arbitrary sampled even weight function.
    */
    class WeightTableFilter1DBuilder : public OrthoPolyFilter1DBuilder
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        //  weight    -- Tabulated weight function, in equidistant
        //               steps, in the order of increasing x.
        //               The first value corresponds to x = 0,
        //               the second to x + step, etc. The weight
        //               is assumed to be symmetric around 0.
        //
        //  weightLen -- The length of the "weight" array.
        //
        //  exclusionMask    -- Set values of "exclusionMask" != 0 if
        //                      corresponding data points have to be
        //                      excluded when weights are generated.
        //                      If no exclusions are necessary, just
        //                      leave this array as NULL.
        //
        //  exclusionMaskLen -- Length of the "exclusionMask" array. If
        //                      it is not 0 then it must coinside with
        //                      the "datalen" argument given to all future
        //                      invocations of the "makeFilter" method.
        //
        //  excludeCentralPoint -- If "true", the central point of the
        //                      weight will be set to zero. This can be
        //                      useful for certain types of cross validation
        //                      calculations.
        */
        WeightTableFilter1DBuilder(const double* weight, unsigned weightLen,
                                   const unsigned char* exclusionMask = 0,
                                   unsigned exclusionMaskLen = 0,
                                   bool excludeCentralPoint = false);

        inline virtual ~WeightTableFilter1DBuilder() {}

        inline virtual unsigned centralWeightLength() const {return w_.size();}

        inline virtual bool keepAllFilters() const
            {return !exclusionMask_.empty();}

        virtual OrthoPoly1D* makeOrthoPoly(unsigned maxDegree,
                                           unsigned binnum,
                                           unsigned datalen,
                                           unsigned* filterCenter) const;
    private:
        WeightTableFilter1DBuilder();

        std::vector<double> w_;
        mutable std::vector<double> wexcl_;
        std::vector<unsigned char> exclusionMask_;
    };

    /**
    // This class will construct a filter which does not smooth the input.
    // Such a filter is sometimes useful for testing purposes.
    */
    class NonmodifyingFilter1DBuilder : public WeightTableFilter1DBuilder
    {
    public:
        NonmodifyingFilter1DBuilder();

        inline virtual ~NonmodifyingFilter1DBuilder() {}

        virtual OrthoPoly1D* makeOrthoPoly(unsigned maxDegree,
                                           unsigned binnum,
                                           unsigned datalen,
                                           unsigned* filterCenter) const;
    };
}

#endif // NPSTAT_WEIGHTTABLEFILTER1DBUILDER_HH_
