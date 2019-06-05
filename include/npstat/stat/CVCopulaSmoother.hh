#ifndef NPSTAT_CVCOPULASMOOTHER_HH_
#define NPSTAT_CVCOPULASMOOTHER_HH_

/*!
// \file CVCopulaSmoother.hh
//
// \brief Smoothing copulas with AbsBandwidthCVND cross-validation
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsCVCopulaSmoother.hh"
#include "npstat/stat/AbsBandwidthCV.hh"

namespace npstat {
    template<class F>
    class CVCopulaSmoother : public AbsCVCopulaSmoother
    {
    public:
        typedef AbsBandwidthCVND<double,ArrayND<double> > CVCalc;
        typedef F Filter;

        virtual ~CVCopulaSmoother();

    protected:
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
        // initialBw         -- "central" bandwidth for cross validation
        //                      calculations (or the actual bandwidth used
        //                      in case cross validation is not performed).
        //                      Set this parameter to 0.0 in order to
        //                      disable filtering altogether.
        //
        // cvCalc            -- calculator for the quantity being optimized
        //                      in the cross validation process. May be NULL
        //                      in which case cross validation will not
        //                      be used.
        //
        // becomeCvCalcOwner -- tells us whether we should destroy cvCalc
        //                      in our own destructor
        //
        // cvRange           -- we will scan bandwidth values between
        //                      initialBw/cvRange and initialBw*cvRange
        //                      uniformly in the log space.
        //
        // nCV               -- number of bandwidth values to try in the
        //                      bandwidth scan. If this number is even, it
        //                      will be increased by 1 internally so that
        //                      the "central" bandwidth is included in
        //                      the scan. If this parameter is 0 or 1, the
        //                      value given by "initialBw" will be used.
        //
        // useConvolve       -- if "true", use "convolve" method of the
        //                      filter rather than "filter" method.
        */
        CVCopulaSmoother(const unsigned* nBinsInEachDim, unsigned dim,
                         double marginTolerance,
                         unsigned maxNormCycles, double initialBw,
                         const CVCalc* cvCalc, bool becomeCvCalcOwner,
                         double cvRange, unsigned nCV, bool useConvolve);
        /**
        // Constructor which explicitly specifies the complete set of
        // bandwidth values to use in cross-validation
        */
        CVCopulaSmoother(const unsigned* nBinsInEachDim, unsigned dim,
                         double marginTolerance, unsigned maxNormCycles,
                         const std::vector<double>& bandwidthValues,
                         const CVCalc* cvCalc, bool becomeCvCalcOwner,
                         bool useConvolve);
        /**
        // Constructors of the derived classes should call
        // this method for each bandwidth value. "i" is
        // the bandwidth value number, while the bandwidth
        // corresponding to this number should be obtained from
        // the "bandwidthValues()" vector.
        //
        // This base class will assume the ownership of the filter object.
        */
        void setFilter(unsigned i, Filter* filter);

    private:
        double smoothAndCV(const HistoND<double>& histo,
                           double effectiveSampleSize,
                           bool isSampleWeighted,
                           unsigned bandwidthIndex,
                           bool runCrossValidation,
                           ArrayND<double>* result,
                           double* regularizedFraction,
                           bool* isNonNegativeAndNormalized);

        std::vector<Filter*> filters_;
        const CVCalc* cvCalc_;
        bool assumeCvCalcOwnership_;
    };
}

#include "npstat/stat/CVCopulaSmoother.icc"

#endif // NPSTAT_CVCOPULASMOOTHER_HH_
