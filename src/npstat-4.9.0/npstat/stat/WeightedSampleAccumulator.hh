#ifndef NPSTAT_WEIGHTEDSAMPLEACCUMULATOR_HH_
#define NPSTAT_WEIGHTEDSAMPLEACCUMULATOR_HH_

/*!
// \file WeightedSampleAccumulator.hh
//
// \brief Accumulates a sample of weighted points and calculates various
//        descriptive statistics
//
// Author: I. Volobouev
//
// October 2011
*/

#include <vector>
#include <cassert>
#include <utility>

#include "geners/ClassId.hh"
#include "npstat/nm/PreciseType.hh"

namespace npstat {
    /**
    // Accumulator of weighted points for the purpose of calculating
    // statistical summaries in a numerically stable manner
    */
    template
    <
        typename Numeric,
        typename Precise=typename PreciseType<Numeric>::type
    >
    class WeightedSampleAccumulator
    {
    public:
        typedef Numeric value_type;
        typedef Precise precise_type;

        inline WeightedSampleAccumulator() {reset();}

        /** Maximum weight among those accumulate so far */
        inline double maxWeight() const {return maxWeight_;}

        /**
        // This method returns the total number of times
        // the "accumulate" method was called on individual points
        // since the last reset (or since the object was created)
        */
        inline unsigned long ncalls() const {return callCount_;}

        /**
        // This method returns the number of times the "accumulate"
        // method was called with a positive weight
        */
        inline unsigned long nfills() const {return data_.size();}

        /**
        // This method returns the effective number of counts
        // which is (squared sum of weights)/(sum of squared weights)
        */
        double count() const;

        /** Minimum value in the accumulated sample */
        Numeric min() const;

        /** Maximum value in the accumulated sample */
        Numeric max() const;

        /** Accumulated sample average */
        Precise mean() const;

        /** Estimate of the population standard deviation */
        Precise stdev() const;

        /** Uncertainty of the population mean */
        Precise meanUncertainty() const;

        /** Accumulated sample median */
        Numeric median() const;

        /** Empirical cumulative distribuition function */
        double cdf(Numeric value) const;

        /** Sum of weights below the given coordinate */
        double weightBelow(Numeric value) const;

        /** Empirical quantile function */
        Numeric quantile(double x) const;

        //@{
        /** Standard plotting accessor */
        inline Numeric location() const
            {return median();}
        inline Numeric rangeDown() const
            {return median() - quantile(0.158655253931457051);}
        inline Numeric rangeUp() const
            {return quantile(0.841344746068542949) - median();}
        //@}

        /**
        // Accumulated sample average. Will not throw an exception if
        // no data is accumulated. Instead, the provided function
        // argument will be returned.
        */
        Precise noThrowMean(const Precise& valueIfNoData) const;

        /**
        // Standard deviation estimate. Will not throw an exception if
        // no data is accumulated. Instead, the provided function
        // argument will be returned.
        */
        Precise noThrowStdev(const Precise& valueIfNoData) const;

        /**
        // Uncertainty of the mean. Will not throw an exception if
        // no data is accumulated. Instead, the provided function
        // argument will be returned.
        */
        Precise noThrowMeanUncertainty(const Precise& valueIfNoData) const;

        /** Average weight (fills with 0 weight are ignored) */
        double averageWeight() const;

        /** Sum of weights accumulated so far */
        double sumOfWeights() const;

        /**
        // Note that the result returned by this method
        // can be invalidated by a call to any non-const method.
        // The data will include only the points with non-zero
        // weights.
        */
        inline const std::pair<Numeric,double>* data() const
            {return data_.empty() ? 0 : &data_[0];}

        /** Comparison for equality */
        bool operator==(const WeightedSampleAccumulator& r) const;

        /** Logical negation of  operator== */
        bool operator!=(const WeightedSampleAccumulator& r) const;

        /**
        // Accumulate the sample. Weights must be non-negative.
        // Run-time error will be generated for negative weights.
        */
        void accumulate(const Numeric& value, double weight);

        /** Accumulate the sample from another accumulator */
        void accumulate(const WeightedSampleAccumulator& acc);

        //@{
        /**
        // In this method, it is assumed that the first element
        // of the pair is the value of type Numeric and the second
        // is the weight which can be automatically converted to double
        */
        template<typename Pair>
        void accumulate(const Pair* values, unsigned long n);

        template<typename Pair>
        inline void accumulate(const Pair& pair)
            {accumulate(pair.first, pair.second);}

        template<typename Pair>
        inline WeightedSampleAccumulator& operator+=(const Pair& pair)
            {accumulate(pair.first, pair.second); return *this;}
        //@}

        /** Add the sample from another accumulator */
        inline WeightedSampleAccumulator& operator+=(
            const WeightedSampleAccumulator& r)
            {accumulate(r); return *this;}

        /**
        // The effect of "operator*=" is the same as if all values
        // were multiplied by "r" for each "accumulate" call
        */
        template<typename Num2>
        WeightedSampleAccumulator& operator*=(const Num2& r);

        /**
        // The effect of "operator/=" is the same as if all values
        // were divided by "r" for each "accumulate" call
        */
        template<typename Num2>
        WeightedSampleAccumulator& operator/=(const Num2& r);

        //@{
        /*
        // The binary operators are rather inefficient
        // and should be avoided in tight loops
        */
        inline WeightedSampleAccumulator operator+(
            const WeightedSampleAccumulator& r) const
        {
            WeightedSampleAccumulator acc(*this);
            acc += r;
            return acc;
        }

        template<typename Num2>
        inline WeightedSampleAccumulator operator*(const Num2& r) const
        {
            WeightedSampleAccumulator acc(*this);
            acc *= r;
            return acc;
        }

        template<typename Num2>
        inline WeightedSampleAccumulator operator/(const Num2& r) const
        {
            WeightedSampleAccumulator acc(*this);
            acc /= r;
            return acc;
        }
        //@}

        /**
        // Scale the weights. The effect is the same as if all
        // weights were multiplied by "r" for each "accumulate" call.
        // Note that r can not be negative for this method.
        */
        WeightedSampleAccumulator& scaleWeights(double r);

        /** Clear all accumulated data */
        void reset();

        /** Reserve memory for points to be accumulated in the future */
        inline void reserve(const unsigned long n) {data_.reserve(n);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            WeightedSampleAccumulator* acc);
    private:
        void sort() const;
        Precise stdev2(bool isMeanUncertainty) const;

        std::vector<std::pair<Numeric,double> > data_;
        long double wsum_;
        long double wsumsq_;
        double maxWeight_;
        unsigned long callCount_;
        bool sorted_;

#ifdef SWIG
    public:
        inline WeightedSampleAccumulator mul2(const double r) const
            {return operator*(r);}

        inline WeightedSampleAccumulator div2(const double r) const
            {return operator/(r);}

        inline WeightedSampleAccumulator& imul2(const double r)
            {*this *= r; return *this;}

        inline WeightedSampleAccumulator& idiv2(const double r)
            {*this /= r; return *this;}

        inline WeightedSampleAccumulator& operator+=(
            const std::pair<Numeric,double>& pair)
            {accumulate(pair.first, pair.second); return *this;}

        inline void accumulate(const std::pair<Numeric,double>* values,
                               const unsigned long n)
        {
            if (n)
            {
                assert(values);
                data_.reserve(data_.size() + n);
                for (unsigned long i=0; i<n; ++i)
                    accumulate(values[i].first, values[i].second);
            }
        }
#endif
    };

    typedef WeightedSampleAccumulator<float,long double>  FloatWeightedSampleAccumulator;
    typedef WeightedSampleAccumulator<double,long double> DoubleWeightedSampleAccumulator;
}

#include "npstat/stat/WeightedSampleAccumulator.icc"

#endif // NPSTAT_WEIGHTEDSAMPLEACCUMULATOR_HH_
