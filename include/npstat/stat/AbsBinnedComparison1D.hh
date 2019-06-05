#ifndef NPSTAT_ABSBINNEDCOMPARISON1D_HH_
#define NPSTAT_ABSBINNEDCOMPARISON1D_HH_

/*!
// \file AbsBinnedComparison1D.hh
//
// \brief Interface definition for comparing binned samples and/or densities
//
// Author: I. Volobouev
//
// May 2014
*/

#include <vector>

namespace npstat {
    struct AbsDiscreteDistribution1DDistance;
    struct AbsRandomGenerator;
    class DiscreteTabulated1D;

    class AbsBinnedComparison1D
    {
    public:
        inline virtual ~AbsBinnedComparison1D() {}

        /** Printable name identifying the comparison method */
        virtual const char* name() const = 0;

        /**
        // The comparison method interface. Pointers "distance" or "pvalue"
        // can be NULL in which case the corresponding quantity does not
        // have to be calculated. If it makes sense for a test to use
        // a "reference" sample (e.g., exact density with no sampling
        // uncertainty), this should be the second argument of the function.
        //
        // If types T1 or T2 are not doubles, the corresponding arrays
        // will be converted to doubles internally.
        */
        template<typename T1, typename T2>
        void compare(const T1* data, const T2* reference, unsigned len,
                     double* distance, double* pvalue) const;

    protected:
        // Helper function for converting real numbers into unsigned.
        // Will throw std::invalid_argument if the input is negative
        // or larger than the largest unsigned long.
        static unsigned long realToULong(const long double d);

        // Helper function for calculating p-values by running
        // pseudo-experiments. Sample size of 0.5 or less indicates
        // infinitelty precise density which will not be sampled.
        static double pvalueByPseudo(const AbsDiscreteDistribution1DDistance& c,
                                     AbsRandomGenerator& rng,
                                     const DiscreteTabulated1D& data,
                                     double dataSampleSize,
                                     const DiscreteTabulated1D& reference,
                                     double refSampleSize,
                                     double observedDistance, unsigned nPseudo);
    private:
        // Main comparison function to be overriden by derived classes
        virtual void compareD(const double* data, const double* reference,
                              unsigned len, double* distance,
                              double* pvalue) const = 0;

        mutable std::vector<double> buf1_;
        mutable std::vector<double> buf2_;
    };
}

#include "npstat/stat/AbsBinnedComparison1D.icc"

#endif // NPSTAT_ABSBINNEDCOMPARISON1D_HH_
