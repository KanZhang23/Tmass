#ifndef NPSTAT_BINSUMMARY_HH_
#define NPSTAT_BINSUMMARY_HH_

/*!
// \file BinSummary.hh
//
// \brief A type for displaying histogram bins with associated uncertainties
//
// Author: I. Volobouev
//
// October 2012
*/

#include <cfloat>
#include <stdexcept>

#include "geners/ClassId.hh"

namespace npstat {
    class StatAccumulator;
    class WeightedStatAccumulator;

    /**
    // A type which can be used as a histogram bin in order to display bin
    // values as mean with variance or as a five-number summary in box plots
    */
    class BinSummary
    {
    public:
        /**
        // Both rangeDown and rangeUp arguments in this
        // constructor must be non-negative
        */
        BinSummary(double location, double rangeDown, double rangeUp);

        /**
        // Both rangeDown and rangeUp arguments in this constructor
        // must be non-negative. "minValue" and "maxValue" complete
        // the five-number summary.
        */
        BinSummary(double location, double rangeDown, double rangeUp,
                   double minValue, double maxValue);

        /**
        // This constructor sets both rangeDown and rangeUp
        // to the same value (which must be non-negative)
        */
        BinSummary(double location, double stdev);

        /** The default constructor is equivalent to BinSummary(0.0, 0.0) */
        BinSummary();

        BinSummary(const BinSummary&);
        BinSummary& operator=(const BinSummary&);

        /**
        // Converting constructor from accumulator classes. If accumulators
        // on the right side contain no data, BinSummary will look as if
        // it was made by the default constructor.
        */
        template<class Accumulator>
        BinSummary(const Accumulator& acc);

        /** Converting assignment operator from accumulator classes */
        template<class Accumulator>
        inline BinSummary& operator=(const Accumulator& acc)
            {*this = BinSummary(acc); return *this;}

        //@{
        /** Constructors from a single number (the location) */
        BinSummary(bool location);
        BinSummary(unsigned char location);
        BinSummary(int location);
        BinSummary(long long location);
        BinSummary(float location);
        BinSummary(double location);
        //@}

        //@{
        /** Assignment operators from a single number (the location) */
        BinSummary& operator=(const double& location);
        inline BinSummary& operator=(const bool& loc)
            {return operator=(static_cast<double>(loc));}
        inline BinSummary& operator=(const unsigned char& loc)
            {return operator=(static_cast<double>(loc));}
        inline BinSummary& operator=(const int& loc)
            {return operator=(static_cast<double>(loc));}
        inline BinSummary& operator=(const long long& loc)
            {return operator=(static_cast<double>(loc));}
        inline BinSummary& operator=(const float& loc)
            {return operator=(static_cast<double>(loc));}
        //@}

        //@{
        /** Standard plotting accessor */
        inline double location() const {return location_;}
        inline double rangeDown() const {return rangeDown_;}
        inline double rangeUp() const {return rangeUp_;}
        //@}

        /**
        // stdev() throws std::runtime_error in case rangeDown()
        // and rangeUp() would return different values
        */
        double stdev() const;

        //@{
        /**
        // min() and max() will throw std::runtime_error
        // in case the corresponding quantities are not set
        */
        double min() const;
        double max() const;
        //@}

        //@{
        /**
        // This accessor will not throw an exception in case the corresponding
        // quantities are undefined. Instead, it will return its argument.
        */
        double noThrowStdev(double valueIfNoData=0.0) const;
        double noThrowMin(double valueIfNoData=-DBL_MAX) const;
        double noThrowMax(double valueIfNoData=DBL_MAX) const;
        //@}

        //@{
        /** Check if the corresponding quantity is set */
        bool hasStdev() const;
        bool hasMin() const;
        bool hasMax() const;
        bool hasLimits() const;
        //@}

        //@{
        /**
        // Modifier method (what it does is obvious from the method name).
        // Will throw std::invalid_argument in case changing the value would
        // result in an inconsistent object.
        */
        void setLocation(double newValue);
        void setStdev(double newValue);
        void setRangeDown(double newValue);
        void setRangeUp(double newValue);
        void setRanges(double newRangeDown, double newRangeUp);
        void setMin(double newValue);
        void setMax(double newValue);
        void setLimits(double newMin, double newMax);
        void setLocationAndLimits(double newLocation,
                                  double newMin, double newMax);
        //@}

        /** Shift the location of the represented distribution */
        void shift(double delta);

        /**
        // Change the width of the represented distribution by
        // a certain factor
        */
        void scaleWidth(double scale);

        /**
        // Symmetrize the ranges. Equivalent to calling setStdev() with the
        // value set to the arithmetic average of the upper and lower range.
        */
        void symmetrizeRanges();

        /** In-place multiplication of all elements by a constant */
        BinSummary& operator*=(double scaleFactor);

        /** In-place division of all elements by a constant */
        inline BinSummary& operator/=(const double denom)
        {
            if (denom == 0.0) throw std::domain_error(
                "In npstat::BinSummary::operator/=: division by zero");
            return operator*=(1.0/denom);
        }

        /** Multiplication of all elements by a constant */
        inline BinSummary operator*(const double r) const
        {
            BinSummary tmp(*this);
            tmp *= r;
            return tmp;
        }

        /** Division of all elements by a constant */
        inline BinSummary operator/(const double r) const
        {
            BinSummary tmp(*this);
            tmp /= r;
            return tmp;
        }

        /**
        // Add another summary to this one assuming that both represent
        // independent Gaussian distributions.
        //
        // This operator will work only in the case lower and upper
        // ranges are symmetric in both summaries. If either min or max
        // are undefined in any of the summaries, both of them will be
        // undefined in this object after this operator runs. Uncertainties
        // are added in quadrature.
        */
        BinSummary& operator+=(const BinSummary& r);

        /**
        // Subtract another summary from this one assuming that both represent
        // independent Gaussian distributions.
        //
        // This operator will work only in the case lower and upper
        // ranges are symmetric in both summaries. If either min or max
        // are undefined in any of the summaries, both of them will be
        // undefined in this object after this operator runs. Note that
        // "-=" does not undo "+=": it is assumed that the correlation
        // is 0 in both cases and uncertainties are added in quadrature.
        */
        BinSummary& operator-=(const BinSummary& r);

        /** Binary addition of two summaries */
        inline BinSummary operator+(const BinSummary& r) const
        {
            BinSummary tmp(*this);
            tmp += r;
            return tmp;
        }

        /** Binary subtraction of two summaries */
        inline BinSummary operator-(const BinSummary& r) const
        {
            BinSummary tmp(*this);
            tmp -= r;
            return tmp;
        }

        /** Comparison for equality */
        bool operator==(const BinSummary& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const BinSummary& r) const
            {return !(*this == r);}

        //@{
        // Method related to "geners" I/O
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname() {return "npstat::BinSummary";}
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            BinSummary* acc);
    private:
        static void validateBinSummary(const char* where, double location,
                                       double down, double up,
                                       double min, double max);
        double location_;
        double rangeDown_;
        double rangeUp_;
        double min_;
        double max_;
    };
}

#include "npstat/stat/BinSummary.icc"

#endif // NPSTAT_BINSUMMARY_HH_
