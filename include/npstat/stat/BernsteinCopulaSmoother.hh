#ifndef NPSTAT_BERNSTEINCOPULASMOOTHER_HH_
#define NPSTAT_BERNSTEINCOPULASMOOTHER_HH_

/*!
// \file BernsteinCopulaSmoother.hh
//
// \brief Copula smoothing with Bernstein polynomials using cross-validation
//
// The cross-validation is implemented in the base class, this class only
// needs to build the filters for different bandwidth values.
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/nm/Matrix.hh"

#include "npstat/stat/CVCopulaSmoother.hh"
#include "npstat/stat/SequentialPolyFilterND.hh"

namespace npstat {
    /**
    // This class builds multivariate copula filters which are
    // tensor products of univariate Bernstein polynomial filters
    */
    class BernsteinCopulaSmoother : 
        public CVCopulaSmoother<SequentialPolyFilterND>
    {
    public:
        typedef CVCopulaSmoother<SequentialPolyFilterND> Base;

        /**
        // Constructor arguments are as follows:
        //
        // nBinsInEachDim    -- number of copula bins in each dimension
        //
        // dim               -- copula dimensionality
        //
        // marginTolerance   -- tolerance for the margin to be uniform
        //
        // maxNormCycles     -- max number of copula normalization cycles
        //
        // cvCalc            -- calculator for the quantity being optimized
        //                      in the cross validation process. May be NULL
        //                      in which case cross validation will not
        //                      be used.
        //
        // becomeCvCalcOwner -- tells us whether we should destroy cvCalc
        //                      in our own destructor
        //
        // polyDegreesToUse  -- Bernstein polynomial degrees to try. The
        //                      row number of the matrix corresponds to
        //                      the "trial" number and the column number
        //                      gives the Bernstein polynomial degree for
        //                      the corresponding dimension.
        */
        BernsteinCopulaSmoother(const unsigned* nBinsInEachDim,
                                unsigned dim, double marginTolerance,
                                unsigned maxNormCycles,
                                const CVCalc* cvCalc, bool becomeCvCalcOwner,
                                const Matrix<unsigned>& polyDegreesToUse);

        inline virtual ~BernsteinCopulaSmoother() {}
    };
}

#endif // NPSTAT_BERNSTEINCOPULASMOOTHER_HH_
