#ifndef NPSTAT_STATACCUMULATOR_HH_
#define NPSTAT_STATACCUMULATOR_HH_

/*!
// \file StatAccumulator.hh
//
// \brief Single-pass accumulator of statistical summary for a set of numbers
//
// Author: I. Volobouev
//
// November 2011
*/

#include <stdexcept>

#include "geners/ClassId.hh"

namespace npstat {
    /**
    // This class calculates minimum value, maximum value, mean, and standard
    // deviation for a set of observations. "mean" and "stdev" functions will
    // cause a runtime exception to be thrown in case "accumulate" function
    // was never called after the object was created (or after it was reset).
    //
    // This code updates a running average, so it should work better than
    // a "naive" implementation. If you still need to improve the calculation
    // of the standard deviation, use SampleAccumulator class which remembers
    // all input points and performs a two-pass calculation.
    */
    class StatAccumulator
    {
    public:
        StatAccumulator();

        /** Number of observations processed so far */
        inline unsigned long count() const {return count_;}

        /** Minimum value in the processed sample */
        inline double min() const {return min_;}

        /** Maximum value in the processed sample */
        inline double max() const {return max_;}

        /** Sum of all observations */
        long double sum() const;

        /** Accumulated sum of squares */
        long double sumsq() const;

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

        //@{
        /** Main data accumulating function */
        void accumulate(double value);

        inline StatAccumulator& operator+=(const double r)
            {accumulate(r); return *this;}
        //@}

        //@{
        /** Add the sample from another accumulator */
        void accumulate(const StatAccumulator&);

        inline StatAccumulator& operator+=(const StatAccumulator& r)
            {accumulate(r); return *this;}
        //@}

        /** Accumulate a product of a StatAccumulator times double */
        void accumulate(const StatAccumulator&, double w);

        /**
        // The effect of "operator*=" is the same as if all values
        // were multiplied by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLongDouble>
        inline StatAccumulator& operator*=(const ConvertibleToLongDouble& r)
        {
            if (count_)
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
        template<typename ConvertibleToLongDouble>
        inline StatAccumulator& operator/=(const ConvertibleToLongDouble& r)
        {
            const long double denom = static_cast<long double>(r);
            if (denom == 0.0L) throw std::domain_error(
                "In npstat::StatAccumulator::operator/=: "
                "division by zero");
            return operator*=(1.0L/denom);
        }

        /** Binary multiplication by a double */
        template<typename ConvertibleToLDouble>
        inline StatAccumulator operator*(const ConvertibleToLDouble& r) const
        {
            StatAccumulator acc(*this);
            acc *= r;
            return acc;
        }

        /** Binary division by a double */
        template<typename ConvertibleToLDouble>
        inline StatAccumulator operator/(const ConvertibleToLDouble& r) const
        {
            StatAccumulator acc(*this);
            acc /= r;
            return acc;
        }

        /** Binary sum with another accumulator */
        inline StatAccumulator operator+(const StatAccumulator& r) const
        {
            StatAccumulator acc(*this);
            acc += r;
            return acc;
        }

        /** Clear all accumulators */
        void reset();

        /** Comparison for equality */
        bool operator==(const StatAccumulator& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const StatAccumulator& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname() {return "npstat::StatAccumulator";}
        static inline unsigned version() {return 2;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            StatAccumulator* acc);
    private:
        void recenter();
        void recenter(long double newCenter);

        long double sum_;
        long double sumsq_;
        long double runningMean_;
        double min_;
        double max_;
        unsigned long count_;
        unsigned long nextRecenter_;

#ifdef SWIG
    public:
        inline StatAccumulator mul2(const double r) const
            {return operator*(r);}

        inline StatAccumulator div2(const double r) const
            {return operator/(r);}

        inline StatAccumulator& imul2(const double r)
            {*this *= r; return *this;}

        inline StatAccumulator& idiv2(const double r)
            {*this /= r; return *this;}
#endif
    };
}

#endif // NPSTAT_STATACCUMULATOR_HH_
