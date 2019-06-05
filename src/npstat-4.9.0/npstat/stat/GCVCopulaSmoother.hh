#ifndef NPSTAT_GCVCOPULASMOOTHER_HH_
#define NPSTAT_GCVCOPULASMOOTHER_HH_

/*!
// \file GCVCopulaSmoother.hh
//
// \brief Smoothing copulas with AbsBandwidthGCVND cross-validation
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsCVCopulaSmoother.hh"
#include "npstat/stat/AbsBandwidthGCV.hh"

namespace npstat {
    template<class F>
    class GCVCopulaSmoother : public AbsCVCopulaSmoother
    {
    public:
        typedef AbsBandwidthGCVND<double,ArrayND<double> > GCVCalc;
        typedef F Filter;

        virtual ~GCVCopulaSmoother();

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
        // nGCV               -- number of bandwidth values to try in the
        //                      bandwidth scan. If this number is even, it
        //                      will be increased by 1 internally so that
        //                      the "central" bandwidth is included in
        //                      the scan. If this parameter is 0 or 1, the
        //                      value given by "initialBw" will be used.
        //
        // useConvolve       -- if "true", use "convolve" method of the
        //                      filter rather than "filter" method.
        */
        GCVCopulaSmoother(const unsigned* nBinsInEachDim, unsigned dim,
                          double marginTolerance,
                          unsigned maxNormCycles, double initialBw,
                          const GCVCalc* cvCalc, bool becomeCvCalcOwner,
                          double cvRange, unsigned nGCV, bool useConvolve);
        /**
        // Constructor which explicitly specifies the complete set of
        // bandwidth values to use in cross-validation
        */
        GCVCopulaSmoother(const unsigned* nBinsInEachDim, unsigned dim,
                          double marginTolerance, unsigned maxNormCycles,
                          const std::vector<double>& bandwidthValues,
                          const GCVCalc* cvCalc, bool becomeCvCalcOwner,
                          bool useConvolve);
        /**
        // Constructors of the derived classes should call
        // this method for each bandwidth value. "i" is
        // the bandwidth value number, while the bandwidth
        // corresponding to this number should be obtained from
        // the "bandwidthValues()" vector.
        //
        // This base class will assume the ownership of the filter objects.
        */
        void setFilter(unsigned i, Filter* filter, Filter* looFilter);

    private:
        double smoothAndCV(const HistoND<double>& histo,
                           double effectiveSampleSize,
                           bool isSampleWeighted,
                           unsigned bandwidthIndex,
                           bool runCrossValidation,
                           ArrayND<double>* result,
                           double* regularizedFraction,
                           bool* isNonNegativeAndNormalized);

        // The following vectors must have the same size
        std::vector<Filter*> filters_;
        std::vector<Filter*> looFilters_;

        ArrayND<double> looDensity_;
        const GCVCalc* cvCalc_;
        bool assumeCvCalcOwnership_;
    };
}

#include "npstat/stat/GCVCopulaSmoother.icc"

#endif // NPSTAT_GCVCOPULASMOOTHER_HH_
