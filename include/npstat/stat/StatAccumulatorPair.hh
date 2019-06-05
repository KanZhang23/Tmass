#ifndef NPSTAT_STATACCUMULATORPAIR_HH_
#define NPSTAT_STATACCUMULATORPAIR_HH_

/*!
// \file StatAccumulatorPair.hh
//
// \brief Single-pass accumulator of statistical summary for a set of
//        two-dimensional observations
//
// Author: I. Volobouev
//
// July 2011
*/

#include <stdexcept>
#include <utility>

#include "npstat/stat/StatAccumulator.hh"

namespace npstat {
    /**
    // This class functions like a pair of StatAccumulator
    // objects but it also accumulates the cross term so that the
    // correlation coefficient between two variables can be evaluated.
    */
    class StatAccumulatorPair
    {
    public:
        inline StatAccumulatorPair() : crossSumsq_(0.0L) {}

        /** Statistical summary for the first variable */
        inline const StatAccumulator& first() const {return first_;}

        /** Statistical summary for the second variable */
        inline const StatAccumulator& second() const {return second_;}

        /** The sum of cross terms */
        inline long double crossSumsq() const {return crossSumsq_;}
 
        /** Number of observations processed so far */
        inline unsigned long count() const {return first_.count();}

        /** Covariance between two variables */
        double cov() const;

        /** Correlation coefficient between two variables */
        double corr() const;

        //@{
        /** Accumulate the data */
        void accumulate(double x, double y);
        void accumulate(const std::pair<double,double>& point);
        //@}

        /** Accumulate the data from another accumulator */
        void accumulate(const StatAccumulatorPair&);

        /** Clear all accumulators */
        void reset();

        /** Accumulate the data */
        inline StatAccumulatorPair& operator+=(
            const std::pair<double,double>& point)
            {accumulate(point); return *this;}

        /** Add the summary from another accumulator */
        inline StatAccumulatorPair& operator+=(const StatAccumulatorPair& r)
            {accumulate(r); return *this;}

        /**
        // The effect of "operator*=" is the same as if all values
        // were multiplied by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLDouble>
        inline StatAccumulatorPair& operator*=(const ConvertibleToLDouble& r)
        {
            const long double factor(static_cast<long double>(r));
            first_ *= factor;
            second_ *= factor;
            crossSumsq_ *= factor*factor;
            return *this;
        }

        /**
        // The effect of "operator/=" is the same as if all values
        // were divided by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLDouble>
        inline StatAccumulatorPair& operator/=(const ConvertibleToLDouble& r)
        {
            const long double denom = static_cast<long double>(r);
            if (denom == 0.0L) throw std::domain_error(
                "In npstat::StatAccumulatorPair::operator/=: "
                "division by zero");
            return operator*=(1.0L/denom);
        }

        /** Binary multiplication by a double */
        template<typename ConvToLDouble>
        inline StatAccumulatorPair operator*(const ConvToLDouble& r) const
        {
            StatAccumulatorPair acc(*this);
            acc *= r;
            return acc;
        }

        /** Binary division by a double */
        template<typename ConvToLDouble>
        inline StatAccumulatorPair operator/(const ConvToLDouble& r) const
        {
            StatAccumulatorPair acc(*this);
            acc /= r;
            return acc;
        }

        /** Comparison for equality */
        bool operator==(const StatAccumulatorPair& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const StatAccumulatorPair& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::StatAccumulatorPair";}
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            StatAccumulatorPair* acc);
    private:
        StatAccumulator first_;
        StatAccumulator second_;
        long double crossSumsq_;

#ifdef SWIG
        inline StatAccumulatorPair mul2(const double r) const
            {return operator*(r);}

        inline StatAccumulatorPair div2(const double r) const
            {return operator/(r);}

        inline StatAccumulatorPair& imul2(const double r)
            {*this *= r; return *this;}

        inline StatAccumulatorPair& idiv2(const double r)
            {*this /= r; return *this;}
#endif
    };
}

#endif // NPSTAT_STATACCUMULATORPAIR_HH_
