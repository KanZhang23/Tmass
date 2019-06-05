#ifndef NPSTAT_WEIGHTEDSTATACCUMULATOR_HH_
#define NPSTAT_WEIGHTEDSTATACCUMULATOR_HH_

/*!
// \file WeightedStatAccumulator.hh
//
// \brief Single-pass accumulator of statistical summary for a set of weighted
//        observations.
//
// Author: I. Volobouev
//
// May 2010
*/

#include <utility>
#include <stdexcept>

#include "geners/ClassId.hh"

namespace npstat {
    /**
    // This class calculates minimum value, maximum value, mean, and standard
    // deviation of weighted observations. "mean" and "stdev" functions will
    // cause a runtime exception to be thrown in case "accumulate" function
    // was never called after the object was created (or after it was reset).
    //
    // It is assumed that the weights are known precisely and that the weights
    // do not depend on the observations (this is important!). Weights can not
    // be negative.
    //
    // This code updates a running average, so it should work better than
    // a "naive" implementation.
    */
    class WeightedStatAccumulator
    {
    public:
        WeightedStatAccumulator();

        /** Minimum value in the processed sample */
        inline double min() const {return min_;}

        /** Maximum value in the processed sample */
        inline double max() const {return max_;}

        /** Maximum weight among observations processed so far */
        inline double maxWeight() const {return maxWeight_;}

        /** Accumulated sample average */
        double mean() const;

        /** Estimate of the population standard deviation */
        double stdev() const;

        /** Uncertainty of the population mean */
        double meanUncertainty() const;

        //@{
        /** Standard plotting accessor */
        inline double location() const {return mean();}
        inline double rangeDown() const {return stdev();}
        inline double rangeUp() const {return stdev();}
        //@}

        /**
        // Accumulated sample average. Will not throw an exception if
        // no data is accumulated. Instead, the provided function
        // argument will be returned.
        */
        double noThrowMean(double valueIfNoData=0.0) const;

        /**
        // Standard deviation estimate. Will not throw an exception if
        // no data is accumulated. Instead, the provided function
        // argument will be returned.
        */
        double noThrowStdev(double valueIfNoData=0.0) const;

        /**
        // Uncertainty of the mean. Will not throw an exception if
        // no data is accumulated. Instead, the provided function
        // argument will be returned.
        */
        double noThrowMeanUncertainty(double valueIfNoData=0.0) const;

        /**
        // This method returns the effective number of counts
        // which is (squared sum of weights)/(sum of squared weights)
        */
        double count() const;

        /**
        // This method returns the total number of times
        // the "accumulate(double, double)" method was called since
        // the last reset (or since the object was created)
        */
        inline unsigned long ncalls() const {return callCount_;}

        /**
        // This method returns the number of times
        // the "accumulate(double, double)" method
        // was called with a positive weight
        */
        inline unsigned long nfills() const {return cnt_;}

        /** Average weight (fills with 0 weight are ignored) */
        double averageWeight() const;

        /** Sum of weights accumulated so far */
        double sumOfWeights() const;

        /** Main data accumulating function */
        void accumulate(double value, double weight);

        /** Add the sample from another accumulator */
        void accumulate(const WeightedStatAccumulator&);

        /** Accumulate a product of a WeightedStatAccumulator times double */
        void accumulate(const WeightedStatAccumulator&, double weight);

        /**
        // In this method, it is assumed that the first element
        // of the pair is the value and the second is the weight
        */
        template<typename Pair>
        inline WeightedStatAccumulator& operator+=(const Pair& pair)
            {accumulate(pair.first, pair.second); return *this;}

        /** Add the summary from another accumulator */
        inline WeightedStatAccumulator& operator+=(
            const WeightedStatAccumulator& r)
            {accumulate(r); return *this;}

        /**
        // The effect of "operator*=" is the same as if all values
        // were multiplied by "r" for each "accumulate" call
        */
        template<typename ConvertibleToDouble>
        inline WeightedStatAccumulator& operator*=(const ConvertibleToDouble& r)
        {
            if (cnt_)
            {
                const long double factor(static_cast<long double>(r));
                const double scale(factor);
                sum_ *= factor;
                sumsq_ *= factor*factor;
                runningMean_ *= factor;
                min_ *= scale;
                max_ *= scale;
                if (max_ < min_)
                {
                    const double tmp(min_);
                    min_ = max_;
                    max_ = tmp;
                }
            }
            return *this;
        }

        /**
        // The effect of "operator/=" is the same as if all values
        // were divided by "r" for each "accumulate" call
        */
        template<typename ConvertibleToDouble>
        inline WeightedStatAccumulator& operator/=(const ConvertibleToDouble& r)
        {
            const long double denom = static_cast<long double>(r);
            if (denom == 0.0L) throw std::domain_error(
                "In npstat::WeightedStatAccumulator::operator/=: "
                "division by zero");
            return operator*=(1.0L/denom);
        }

        /** Binary multiplication by a double */
        template<typename ConvToDouble>
        inline WeightedStatAccumulator operator*(const ConvToDouble& r) const
        {
            WeightedStatAccumulator acc(*this);
            acc *= r;
            return acc;
        }

        /** Binary division by a double */
        template<typename ConvToDouble>
        inline WeightedStatAccumulator operator/(const ConvToDouble& r) const
        {
            WeightedStatAccumulator acc(*this);
            acc /= r;
            return acc;
        }

        /** Binary sum with another accumulator */
        inline WeightedStatAccumulator operator+(
            const WeightedStatAccumulator& r) const
        {
            WeightedStatAccumulator acc(*this);
            acc += r;
            return acc;
        }

        /**
        // Scaling of the weights. The effect is the same as if all
        // weights were multiplied by "r" for each "accumulate" call.
        // Note that r can not be negative for this method.
        */
        WeightedStatAccumulator& scaleWeights(double r);

        /** Clear all accumulators */
        void reset();

        /** Comparison for equality */
        bool operator==(const WeightedStatAccumulator& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const WeightedStatAccumulator& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::WeightedStatAccumulator";}
        static inline unsigned version() {return 2;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            WeightedStatAccumulator* acc);
    private:
        void recenter();
        void recenter(long double newCenter);

        long double sum_;
        long double sumsq_;
        long double wsum_;
        long double wsumsq_;
        long double runningMean_;
        double min_;
        double max_;
        double maxWeight_;
        unsigned long cnt_;
        unsigned long callCount_;
        unsigned long nextRecenter_;

#ifdef SWIG
    public:
        inline WeightedStatAccumulator mul2(const double r)
            {return operator*(r);}

        inline WeightedStatAccumulator div2(const double r)
            {return operator/(r);}

        inline WeightedStatAccumulator& imul2(const double r)
            {*this *= r; return *this;}

        inline WeightedStatAccumulator& idiv2(const double r)
            {*this /= r; return *this;}

        inline WeightedStatAccumulator& operator+=(
            const std::pair<double,double>& pair)
            {accumulate(pair.first, pair.second); return *this;}
#endif
    };
}

#endif // NPSTAT_WEIGHTEDSTATACCUMULATOR_HH_
