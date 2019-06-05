#ifndef NPSTAT_ABSCVCOPULASMOOTHER_HH_
#define NPSTAT_ABSCVCOPULASMOOTHER_HH_

/*!
// \file AbsCVCopulaSmoother.hh
//
// \brief Interface definition for smoothing copulas with cross-validation
//
// Abbreviation "CV" used in various places in this code stands for
// "cross-validation"
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsCopulaSmootherBase.hh"

namespace npstat {
    class AbsCVCopulaSmoother : public AbsCopulaSmootherBase
    {
    public:
        inline virtual ~AbsCVCopulaSmoother() {}

        /** Check how the kernel is used */
        inline bool isConvolving() const {return useConvolute_;}

        /**
        // Use either "filter" (kernel placement at the points in which
        // the density is estimated) or "convolve" mode (kernel placement
        // at the sample points)
        */
        inline void setConvolving(const bool b) {useConvolute_ = b;}

        /** Bandwidth values to cross-validate */
        inline const std::vector<double>& bandwidthValues() const
            {return bandwidthValues_;}

        /** Calculated values of the cross-validation criterion */
        inline const std::vector<double>& lastCVValues() const
            {return cvValues_;}

        /** Fraction of bins that was affected by regularization */
        inline const std::vector<double>& lastRegularizedFractions() const
            {return regFractions_;}

        /** Number of bandwidth values to cross-validate */
        inline unsigned getNFilters() const {return bandwidthValues_.size();}

        /** Index of the bandwidth best according to cross-validation */
        inline unsigned lastFilterChosen() const {return bestFilt_;}

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
        // cvRange           -- we will scan bandwidth values between
        //                      initialBw/cvRange and initialBw*cvRange
        //                      uniformly in the log space.
        //
        // nCV               -- number of bandwidth values to try in the
        //                      bandwidth scan. If this number is even, it
        //                      will be increased by 1 internally so that
        //                      the "central" bandwidth is included in
        //                      the scan. If this parameter is 0 or 1, the
        //                      value given by "initialBw" will be used
        //                      and cross-validation will not be performed.
        //
        // useConvolve       -- if "true", use "convolve" method of the
        //                      filter rather than "filter" method.
        */
        AbsCVCopulaSmoother(const unsigned* nBinsInEachDim, unsigned dim,
                            double marginTolerance, unsigned maxNormCycles,
                            double initialBw, double cvRange, unsigned nCV,
                            bool useConvolve);
        /**
        // Constructor which explicitly specifies the complete set of
        // bandwidth values to use in cross-validation
        */
        AbsCVCopulaSmoother(const unsigned* nBinsInEachDim, unsigned dim,
                            double marginTolerance, unsigned maxNormCycles,
                            const std::vector<double>& bandwidthValues,
                            bool useConvolve);
    private:
        // Override from the base class
        void smoothHisto(HistoND<double>& histo,
                         double effectiveSampleSize,
                         double* bandwidthUsed,
                         bool isSampleWeighted);

        // This method should return the value of the cross-validation
        // criterion to be maximized in case "runCrossValidation"
        // argument is "true" (and, of course, fill out the "result",
        // "regularizedFraction", and "isNonNegativeAndNormalized").
        // In case "runCrossValidation" argument is "false", the
        // smoothing should be performed but the result of the
        // cross-validation calculation will be ignored (so, to save
        // time, it is unnecessary to run this calculation).
        virtual double smoothAndCV(const HistoND<double>& histo,
                                   double effectiveSampleSize,
                                   bool isSampleWeighted,
                                   unsigned bandwidthIndex,
                                   bool runCrossValidation,
                                   ArrayND<double>* result,
                                   double* regularizedFraction,
                                   bool* isNonNegativeAndNormalized) = 0;

        std::vector<double> bandwidthValues_;
        std::vector<double> cvValues_;
        std::vector<double> regFractions_;
        ArrayND<double> density_;
        unsigned bestFilt_;
        bool useConvolute_;
    };
}

#endif // NPSTAT_ABSCVCOPULASMOOTHER_HH_
