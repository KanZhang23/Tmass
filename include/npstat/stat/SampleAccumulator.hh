#ifndef NPSTAT_SAMPLEACCUMULATOR_HH_
#define NPSTAT_SAMPLEACCUMULATOR_HH_

/*!
// \file SampleAccumulator.hh
//
// \brief Accumulates a sample and calculates various descriptive statistics
//
// Author: I. Volobouev
//
// July 2010
*/

#include <vector>

#include "geners/ClassId.hh"
#include "npstat/nm/PreciseType.hh"

namespace npstat {
    /**
    // Accumulator of values (or items) for the purpose of calculating
    // statistical summaries in a numerically stable manner.
    //
    // Note that calculating quantiles using "median", "cdf", and
    // "quantile" methods makes subsequent calls to the "cov" and "corr"
    // methods impossible (a dynamic fault will be generated). This is because
    // the data is sorted internally for quantile calculations, and subsequent
    // covariance calculation with sorted samples will return a result which
    // is most likely not intended.
    */
    template
    <
        typename Numeric, 
        typename Precise=typename PreciseType<Numeric>::type
    >
    class SampleAccumulator
    {
    public:
        typedef Numeric value_type;
        typedef Precise precise_type;

        inline SampleAccumulator() : sorted_(true), neverSorted_(true) {}

        /** Number of items in the accumulated sample */
        inline unsigned long count() const {return data_.size();}

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

        /** Exact number of points below the given value */
        unsigned long nBelow(Numeric value) const;

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

        /**
        // Note that the result returned by this method
        // may be invalidated by a call to any non-const method
        */
        inline const Numeric* data() const
            {return data_.empty() ? 0 : &data_[0];}

        //@{
        /**
        // Covariance and correlation with another accumulator.
        // That other accumulator must have the same number of
        // points stored. Note that "cov" and "corr" methods
        // typically make sense only for the original, unmodified
        // sequences of data values. If the data points have been
        // sorted (which happens internally whenever one of the
        // methods "median", "cdf", or "quantile" is called),
        // covariance and correlation can produce rather unexpected
        // results. To prevent any confusion, this class will throw
        // an exception of type std::runtime_error in case "cov"
        // or "corr" is called after the data in any of the two
        // accumulators has been sorted. Basically, if you plan
        // to calculate covariances between accumulators, do it
        // before calculating any quantiles or cdf values.
        */
        template <typename Numeric2, typename Precise2>
        Precise cov(const SampleAccumulator<Numeric2,Precise2>& other) const;

        template <typename Num2, typename Prec2>
        inline Precise corr(const SampleAccumulator<Num2,Prec2>& other) const
            {return cov(other)/stdev()/other.stdev();}
        //@}

        /** Comparison for equality */
        bool operator==(const SampleAccumulator& r) const;

        /** Logical negation of  operator== */
        bool operator!=(const SampleAccumulator& r) const;

        //@{
        /** Accumulate the sample */
        inline void accumulate(const Numeric& value)
            {data_.push_back(value); sorted_ = false;}
        inline SampleAccumulator& operator+=(const Numeric& r)
            {accumulate(r); return *this;}
        //@}

        /** Accumulate more than one value at a time */
        void accumulate(const Numeric* values, unsigned long n);

        //@{
        /** Accumulate the sample from another accumulator */
        void accumulate(const SampleAccumulator& acc);
        inline SampleAccumulator& operator+=(const SampleAccumulator& r)
            {accumulate(r); return *this;}
        //@}

        /** Multiply every entry by a constant */
        template<typename Num2>
        SampleAccumulator& operator*=(const Num2& r);

        /** Divide every entry by a constant */
        template<typename Num2>
        SampleAccumulator& operator/=(const Num2& r);

        //@{
        /*
        // The binary operators are rather inefficient
        // and should be avoided in tight loops
        */
        inline SampleAccumulator operator+(const SampleAccumulator& r) const
        {
            SampleAccumulator acc(*this);
            acc += r;
            return acc;
        }

        template<typename Num2>
        inline SampleAccumulator operator*(const Num2& r) const
        {
            SampleAccumulator acc(*this);
            acc *= r;
            return acc;
        }

        template<typename Num2>
        inline SampleAccumulator operator/(const Num2& r) const
        {
            SampleAccumulator acc(*this);
            acc /= r;
            return acc;
        }
        //@}

        /** Clear all accumulated data */
        inline void reset() {data_.clear(); sorted_=true; neverSorted_=true;}

        /** Reserve memory for data to be accumulated in the future */
        inline void reserve(const unsigned long n) {data_.reserve(n);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            SampleAccumulator* acc);
    private:
        void sort() const;

        std::vector<Numeric> data_;
        bool sorted_;
        bool neverSorted_;

#ifdef SWIG
    public:
        inline SampleAccumulator mul2(const double r) const
            {return operator*(r);}

        inline SampleAccumulator div2(const double r) const
            {return operator/(r);}

        inline SampleAccumulator& imul2(const double r)
            {*this *= r; return *this;}

        inline SampleAccumulator& idiv2(const double r)
            {*this /= r; return *this;}

        inline Precise cov2(const SampleAccumulator& other) const
            {return cov(other);}

        inline Precise corr2(const SampleAccumulator& other) const
            {return corr(other);}
#endif
    };

    typedef SampleAccumulator<float,long double>  FloatSampleAccumulator;
    typedef SampleAccumulator<double,long double> DoubleSampleAccumulator;
}

#include "npstat/stat/SampleAccumulator.icc"

#endif // NPSTAT_SAMPLEACCUMULATOR_HH_
