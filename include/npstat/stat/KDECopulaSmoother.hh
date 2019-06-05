#ifndef NPSTAT_KDECOPULASMOOTHER_HH_
#define NPSTAT_KDECOPULASMOOTHER_HH_

/*!
// \file KDECopulaSmoother.hh
//
// \brief Constant bandwidth multivariate KDE smoother with cross-validation
//
// The cross-validation is implemented in the base class, this class only
// needs to build the filters for different bandwidth values.
//
// Author: I. Volobouev
//
// December 2011
*/

#include "npstat/stat/CVCopulaSmoother.hh"
#include "npstat/stat/KDEFilterND.hh"
#include "npstat/stat/AbsDistributionND.hh"

#include "npstat/nm/ConvolutionEngineND.hh"

namespace npstat {
    /** Class which builds KDE filters for different bandwidth values */
    template <unsigned MaxKDEDeg>
    class KDECopulaSmoother : public CVCopulaSmoother<KDEFilterND<MaxKDEDeg> >
    {
    public:
        typedef CVCopulaSmoother<KDEFilterND<MaxKDEDeg> > Base;

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
        // kernel           -- kernel to use for generating the filters.
        //                     This should be a standard kernel, with
        //                     location of 0 in each dimension. The scales
        //                     should normally be set to 1 (unless you
        //                     want different scale factors for different
        //                     dimensions).
        //
        // taper, maxDegree -- KDEFilterND parameters
        //
        // initialBw        -- "central" bandwidth for cross validation
        //                     calculations (or the actual bandwidth used
        //                     in case cross validation is not performed).
        //                     Set this parameter to 0.0 in order to
        //                     disable filtering altogether.
        //
        // cvCalc           -- calculator for the quantity being optimized
        //                     in the cross validation process. May be NULL
        //                     in which case cross validation will not
        //                     be used.
        //
        // becomeCvCalcOwner -- tells us whether we should destroy cvCalc
        //                      in the destructor of the base class.
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
        // dummy            -- unused. Included for consistency with
        //                     arguments of LOrPECopulaSmoother, etc.
        */
        KDECopulaSmoother(const unsigned* nBinsInEachDim,
                          unsigned dim, double marginTolerance,
                          unsigned maxNormCycles,
                          const AbsDistributionND& kernel,
                          const double* taper, unsigned maxDegree,
                          double initialBw,
                          const typename Base::CVCalc* cvCalc,
                          bool becomeCvCalcOwner,
                          double cvRange, unsigned nCV, bool dummy);

        virtual ~KDECopulaSmoother();

    private:
        ArrayND<double> buffer_;
        ConvolutionEngineND engine_;
    };
}

#include "npstat/stat/KDECopulaSmoother.icc"

#endif // NPSTAT_KDECOPULASMOOTHER_HH_
