#ifndef NPSTAT_WEIGHTEDSTATACCUMULATORPAIR_HH_
#define NPSTAT_WEIGHTEDSTATACCUMULATORPAIR_HH_

/*!
// \file WeightedStatAccumulatorPair.hh
//
// \brief Single-pass accumulator of statistical summary for a set of
//        weighted points in two dimensions
//
// Author: I. Volobouev
//
// June 2012
*/

#include <stdexcept>
#include <utility>

#include "npstat/stat/WeightedStatAccumulator.hh"

namespace npstat {
    /**
    // This class functions like a pair of WeightedStatAccumulator
    // objects but it also accumulates the cross term so that the
    // correlation coefficient between two variables can be evaluated.
    */
    class WeightedStatAccumulatorPair
    {
    public:
        inline WeightedStatAccumulatorPair() : crossSumsq_(0.0L) {}

        /** Statistical summary for the first variable */
        inline const WeightedStatAccumulator& first() const {return first_;}

        /** Statistical summary for the second variable */
        inline const WeightedStatAccumulator& second() const {return second_;}

        /** The sum of cross terms */
        inline long double crossSumsq() const {return crossSumsq_;}
        
        /**
        // This method returns the effective number of counts
        // which is (squared sum of weights)/(sum of squared weights)
        */
        inline double count() const {return first_.count();}

        /**
        // This method returns the total number of times
        // the "accumulate" method was called since the last
        // reset (or since the object was created)
        */
        inline unsigned long ncalls() const {return first_.ncalls();}

        /**
        // This method returns the number of times
        // the "accumulate" method was called with a positive weight
        */
        inline unsigned long nfills() const {return first_.nfills();}

        /** Covariance between two variables */
        double cov() const;

        /** Correlation coefficient between two variables */
        double corr() const;

        //@{
        /** Accumulate the data */
        void accumulate(double x, double y, double w);
        void accumulate(const std::pair<double,double>& point, double w);
        //@}

        //@{
        /** Accumulate the data from another accumulator */
        void accumulate(const WeightedStatAccumulatorPair&);
        void accumulate(const WeightedStatAccumulatorPair&, double w);
        //@}

        /** Clear all accumulators */
        void reset();

        /** Accumulate the data */
        template<typename Triple>
        inline WeightedStatAccumulatorPair& operator+=(const Triple& t)
            {accumulate(t.first, t.second, t.third); return *this;}

        /** Add the summary from another accumulator */
        inline WeightedStatAccumulatorPair& operator+=(
            const WeightedStatAccumulatorPair& r)
            {accumulate(r); return *this;}

        /**
        // The effect of "operator*=" is the same as if all values
        // were multiplied by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLDouble>
        inline WeightedStatAccumulatorPair& operator*=(const ConvertibleToLDouble& r)
        {
            const long double factor(static_cast<long double>(r));
            first_ *= factor;
            second_ *= factor;
            crossSumsq_ *= factor*factor;
            return *this;
        }

        /**
        // Scaling of the weights. The effect is the same as if all
        // weights were multiplied by "r" for each "accumulate" call.
        // Note that r can not be negative for this method.
        */
        inline WeightedStatAccumulatorPair& scaleWeights(const double r)
        {
            first_.scaleWeights(r);
            second_.scaleWeights(r);
            crossSumsq_ *= r;
            return *this;
        }

        /**
        // The effect of "operator/=" is the same as if all values
        // were divided by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLDouble>
        inline WeightedStatAccumulatorPair& operator/=(const ConvertibleToLDouble& r)
        {
            const long double denom = static_cast<long double>(r);
            if (denom == 0.0L) throw std::domain_error(
                "In npstat::WeightedStatAccumulatorPair::operator/=: "
                "division by zero");
            return operator*=(1.0L/denom);
        }

        /** Binary multiplication by a double */
        template<typename ConvToLDouble>
        inline WeightedStatAccumulatorPair operator*(const ConvToLDouble& r) const
        {
            WeightedStatAccumulatorPair acc(*this);
            acc *= r;
            return acc;
        }

        /** Binary division by a double */
        template<typename ConvToLDouble>
        inline WeightedStatAccumulatorPair operator/(const ConvToLDouble& r) const
        {
            WeightedStatAccumulatorPair acc(*this);
            acc /= r;
            return acc;
        }

        /** Comparison for equality */
        bool operator==(const WeightedStatAccumulatorPair& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const WeightedStatAccumulatorPair& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::WeightedStatAccumulatorPair";}
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            WeightedStatAccumulatorPair* acc);
    private:
        WeightedStatAccumulator first_;
        WeightedStatAccumulator second_;
        long double crossSumsq_;

#ifdef SWIG
        inline WeightedStatAccumulatorPair mul2(const double r) const
            {return operator*(r);}

        inline WeightedStatAccumulatorPair div2(const double r) const
            {return operator/(r);}

        inline WeightedStatAccumulatorPair& imul2(const double r)
            {*this *= r; return *this;}

        inline WeightedStatAccumulatorPair& idiv2(const double r)
            {*this /= r; return *this;}
#endif
    };
}

#endif // NPSTAT_WEIGHTEDSTATACCUMULATORPAIR_HH_
