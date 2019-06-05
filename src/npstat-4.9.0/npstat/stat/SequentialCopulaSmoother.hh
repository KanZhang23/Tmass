#ifndef NPSTAT_SEQUENTIALCOPULASMOOTHER_HH_
#define NPSTAT_SEQUENTIALCOPULASMOOTHER_HH_

/*!
// \file SequentialCopulaSmoother.hh
//
// \brief LOrPE copula smoother with cross-validation using tensor product
//        filters
//
// The cross-validation is implemented in the base class, this class only
// needs to build the filters for different bandwidth values.
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/CVCopulaSmoother.hh"
#include "npstat/stat/SequentialPolyFilterND.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // This class builds multivariate copula filters 
    // which are tensor products of univariate filters
    */
    class SequentialCopulaSmoother : 
        public CVCopulaSmoother<SequentialPolyFilterND>
    {
    public:
        typedef CVCopulaSmoother<SequentialPolyFilterND> Base;

        /**
        // Constructor arguments are as follows:
        //
        // nBinsInEachDim   -- number of copula bins in each dimension
        //
        // dim              -- copula dimensionality
        //
        // marginTolerance  -- tolerance for the margin to be uniform
        //
        // maxNormCycles    -- max number of copula normalization cycles
        //
        // symbetaPower     -- these parameters will be passed to the
        // maxFilterDegree     "symbetaLOrPEFilter1D" function when the
        // boundaryMethod      filters will be constructed
        //
        // initialBw        -- "central" bandwidth for cross validation
        //                     calculations (or the actual bandwidth used
        //                     in case cross validation is not performed).
        //                     Set this parameter to 0.0 in order to
        //                     disable filtering altogether.
        //
        // bwCoeffs         -- bandwidth factors for each dimension. This
        //                     array must have "dim" positive numbers.
        //                     The argument can also be NULL in which case
        //                     it is assumed that all factors are 1.0.
        //
        // cvCalc           -- calculator for the quantity being optimized
        //                     in the cross validation process. May be NULL
        //                     in which case cross validation will not
        //                     be used.
        //
        // becomeCvCalcOwner -- tells us whether we should destroy cvCalc
        //                      in our own destructor
        //
        // cvRange          -- we will scan bandwidth values between
        //                     initialBw/cvRange and initialBw*cvRange
        //                     uniformly in the log space.
        //
        // nCV              -- number of bandwidth values to try in the
        //                     bandwidth scan. If this number is even, it
        //                     will be increased by 1 internally so that
        //                     the "central" bandwidth is included in
        //                     the scan. If this parameter is 0 or 1, the
        //                     value given by "initialBw" will be used.
        //
        // useConvolve      -- if "true", use "convolve" method of the
        //                     filter rather than "filter" method.
        //
        // doublyStochastic -- if "true", a doubly stochastic approximation
        //                     will be derived for every original filter
        */
        SequentialCopulaSmoother(const unsigned* nBinsInEachDim,
                                 unsigned dim, double marginTolerance,
                                 unsigned maxNormCycles,
                                 int symbetaPower, double maxFilterDegree,
                                 const BoundaryHandling& boundaryMethod,
                                 double initialBw, const double* bwCoeffs,
                                 const CVCalc* cvCalc, bool becomeCvCalcOwner,
                                 double cvRange, unsigned nCV, bool useConvolve,
                                 bool doublyStochastic=true);

        inline virtual ~SequentialCopulaSmoother() {}
    };
}

#endif // NPSTAT_SEQUENTIALCOPULASMOOTHER_HH_
