#ifndef NPSTAT_MULTIVARIATEWEIGHTEDSUMSQACCUMULATOR_HH_
#define NPSTAT_MULTIVARIATEWEIGHTEDSUMSQACCUMULATOR_HH_

/*!
// \file MultivariateWeightedSumsqAccumulator.hh
//
// \brief Accumulator of weighted sums of squares for covariance calculations
//
// Author: I. Volobouev
//
// April 2011
*/

#include <vector>

#include "npstat/nm/Matrix.hh"
#include "npstat/stat/MultivariateWeightedSumAccumulator.hh"

namespace npstat {
   /**
    * Class for accumulating multivariate weighted sums of squares
    * and calculating corresponding covariances. Intended for use
    * together with MultivariateWeightedSumAccumulator. For example,
    * to determine covariance matrix of all ntuple columns, do
    * something along the following lines:
    *
    * @code
    *    const AbsNtuple<T>& nt = ...        // reference to your ntuple
    *    const WeightCalculator& wcalc = ... // reference to the weight functor
    *    MultivariateWeightedSumAccumulator sums;
    *    nt.weightedCycleOverRows(sums, wcalc);
    *    MultivariateWeightedSumsqAccumulator sumsqs(sums);
    *    nt.weightedCycleOverRows(sumsqs, wcalc);
    *    const Matrix<long double>& m = sumsqs.covMat(); // covariance matrix
    * @endcode
    */
    template<typename Precise = long double>
    class MultivariateWeightedSumsqAccumulator
    {
    public:
        typedef Precise precise_type;

        /** 
        // To calculate covariances for select columns only, use this
        // constructor with a non-trivial column map. Statistics will be
        // accumulated only for the columns included in the "columns"
        // argument. This can be useful for speeding up the code, as the
        // computational complexity of covariance matrix calculations
        // increases as the number of columns squared.
        */
        template<typename Precise2>
        MultivariateWeightedSumsqAccumulator(
            const unsigned long* columns, unsigned nColumns,
            const MultivariateWeightedSumAccumulator<Precise2>& sums);

        /** Constructor with a trivial column map */
        template<typename Precise2>
        explicit MultivariateWeightedSumsqAccumulator(
            const MultivariateWeightedSumAccumulator<Precise2>& sums);

        //@{
        /** Inspect object properties */
        inline unsigned dim() const {return mapLen_;}
        inline unsigned long ncalls() const {return ncalls_;}
        inline unsigned long expectedNumCols() const {return nCols_;}
        inline Precise weightSum() const {return weightSum_;}
        inline const std::vector<unsigned long>& indexMap() const
            {return indexMap_;}
        //@}

        //@{
        /** Mean values from the sum accumulator provided in the constructor */
        inline const std::vector<Precise>& meanVector() const {return mean_;}
        inline Precise mean(const unsigned i) const {return mean_.at(i);}
        //@}

        /**
        // This method returns the effective number of counts
        // which is (squared sum of weights)/(sum of squared weights)
        */
        Precise count() const;

        /** Accumulate statistics for an array (ntuple row) */
        template<typename T>
        void accumulate(const T* data, unsigned long len, double w);

        /** Reset all accumulators and counters */
        void reset();

        /** Accumulated sums of squares or cross terms */
        const Precise& sumsq(unsigned i, unsigned j) const;

        /**
        // Covariance between the variables corresponding to the
        // given indices (as mapped to the original indices)
        */
        Precise cov(unsigned i, unsigned j) const;

        /** Correlation coefficient between the given variables */
        Precise corr(unsigned i, unsigned j) const;

        /** Standard deviation for the given variable */
        Precise stdev(unsigned i) const;

        /** Retrieve the covariance matrix */
        Matrix<Precise> covMat() const;

        /** Retrieve the correlation matrix */
        Matrix<Precise> corrMat() const;

    private:
        std::vector<Precise> sumsq_;
        std::vector<Precise> mean_;
        std::vector<unsigned long> indexMap_;
        Precise weightSum_;
        Precise weightSumSq_;
        unsigned long nCols_;
        unsigned long ncalls_;
        unsigned mapLen_;
    };
}

#include "npstat/stat/MultivariateWeightedSumsqAccumulator.icc"

#endif // NPSTAT_MULTIVARIATEWEIGHTEDSUMSQACCUMULATOR_HH_
