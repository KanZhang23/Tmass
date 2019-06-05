#ifndef NPSTAT_PEARSONSCHISQUARED_HH_
#define NPSTAT_PEARSONSCHISQUARED_HH_

/*!
// \file PearsonsChiSquared.hh
//
// \brief Pearson's chi-squared test
//
// Author: I. Volobouev
//
// May 2014
*/

#include <string>

#include "npstat/stat/AbsBinnedComparison1D.hh"

namespace npstat {
    class PearsonsChiSquared : public AbsBinnedComparison1D
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        // minCountToUse                  -- Only bins with reference counts
        //                                   above this cutoff (or bins with
        //                                   sum of reference and data above
        //                                   this cutoff in case the parameter
        //                                   "forUncertaintyUseReferenceOnly"
        //                                   is false) will be used to calculate
        //                                   the chi-square.
        //
        // subtractForNDoFCalculation     -- The number to be subtracted
        //                                   from the number of bins passing
        //                                   the cut in order to calculate the
        //                                   number of degrees of freedom. This
        //                                   can be useful in case the reference
        //                                   distribution was fitted from data
        //                                   or normalized to the data.
        //
        // forUncertaintyUseReferenceOnly -- If true, only reference counts
        //                                   will be used to estimate variance
        //                                   of bin difference (otherwise the
        //                                   variance is set to the sum of data
        //                                   and reference).
        */
        PearsonsChiSquared(double minCountToUse,
                           double subtractForNDoFCalculation,
                           bool forUncertaintyUseReferenceOnly);

        inline virtual ~PearsonsChiSquared() {}

        virtual const char* name() const;

    private:
        virtual void compareD(const double* data, const double* reference,
                              unsigned len, double* distance,
                              double* pvalue) const;

        mutable std::string name_;
        double minCount_;
        double subtractForNDoF_;
        bool referenceOnly_;
    };
}

#endif // NPSTAT_PEARSONSCHISQUARED_HH_
