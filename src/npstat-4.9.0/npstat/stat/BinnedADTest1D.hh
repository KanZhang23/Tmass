#ifndef NPSTAT_BINNEDADTEST1D_HH_
#define NPSTAT_BINNEDADTEST1D_HH_

/*!
// \file BinnedADTest1D.hh
//
// \brief Binned version of the Anderson-Darling test
//
// Author: I. Volobouev
//
// May 2014
*/

#include <string>

#include "npstat/stat/AbsBinnedComparison1D.hh"

namespace npstat {
    class BinnedADTest1D : public AbsBinnedComparison1D
    {
    public:
        /**
        // If either rng pointer is NULL or mcSamplesForPValue is 0
        // then theoretical formulae are used as if the samples are
        // unbinned.
        */
        BinnedADTest1D(bool useTwoSampleTest, AbsRandomGenerator* rng,
                       unsigned mcSamplesForPValue = 10000U);

        inline virtual ~BinnedADTest1D() {}

        virtual const char* name() const;

    private:
        virtual void compareD(const double* data, const double* reference,
                              unsigned len, double* distance,
                              double* pvalue) const;

        mutable std::string name_;
        AbsRandomGenerator* rng_;
        unsigned mcSamples_;
        bool twoSampleTest_;
    };
}

#endif // NPSTAT_BINNEDADTEST1D_HH_
