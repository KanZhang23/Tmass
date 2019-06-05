#ifndef NPSTAT_ABSDISCRETEDISTRIBUTION1D_HH_
#define NPSTAT_ABSDISCRETEDISTRIBUTION1D_HH_

/*!
// \file AbsDiscreteDistribution1D.hh
//
// \brief Interface definition for 1-d discrete statistical distributions
//
// Author: I. Volobouev
//
// May 2013
*/

#include "geners/ClassId.hh"
#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    struct AbsDiscreteDistribution1D
    {
        inline virtual ~AbsDiscreteDistribution1D() {}

        /** Probability */
        virtual double probability(long x) const = 0;

        /** Cumulative distribution function */
        virtual double cdf(double x) const = 0;

        /** 1 - cdf, implementations should avoid subtractive cancellation */
        virtual double exceedance(double x) const = 0;

        /** The quantile function */
        virtual long quantile(double x) const = 0;

        /**
        // Derived classes should not implement "operator==", implement
        // "isEqual" instead
        */
        inline bool operator==(const AbsDiscreteDistribution1D& r) const
            {return (typeid(*this) == typeid(r)) && this->isEqual(r);}

        /** Logical negation of operator== */
        inline bool operator!=(const AbsDiscreteDistribution1D& r) const
            {return !(*this == r);}

        /** "Virtual copy constructor" */
        virtual AbsDiscreteDistribution1D* clone() const = 0;

        /**
        // Random number generator according to the given distribution.
        // Should return the number of random points used up from the
        // generator.
        */
        inline virtual unsigned random(AbsRandomGenerator& g,
                                       long* generatedRandom) const
        {
            assert(generatedRandom);
            *generatedRandom = quantile(g());
            return 1U;
        }

        //@{
        /** Prototype needed for I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream&) const {return false;}
        //@}

        static inline const char* classname()
            {return "npstat::AbsDiscreteDistribution1D";}
        static inline unsigned version() {return 1;}
        static AbsDiscreteDistribution1D* read(const gs::ClassId& id,
                                               std::istream& is);
    protected:
        /** Comparison for equality. To be implemented by derived classes. */
        virtual bool isEqual(const AbsDiscreteDistribution1D&) const = 0;

#ifdef SWIG
    public:
        inline long random2(AbsRandomGenerator& g) const
        {
            long l;
            this->random(g, &l);
            return l;
        }
#endif
    };

    /**
    // This base class is used to model discrete distributions which
    // have "trivial" behavior w.r.t. location parameter. That is,
    // if the distribution density is g(x), the density with location
    // parameter "mu" is g(x - mu).
    */
    class ShiftableDiscreteDistribution1D : public AbsDiscreteDistribution1D
    {
    public:
        inline explicit ShiftableDiscreteDistribution1D(const long location) :
            location_(location) {}

        inline virtual ~ShiftableDiscreteDistribution1D() {}

        /** Get the location parameter */
        inline long location() const {return location_;}

        /** Set the location parameter */
        inline void setLocation(const long v) {location_ = v;}

        //@{
        /** Method overriden from the AbsDiscreteDistribution1D base class */
        inline double probability(const long x) const
            {return unshiftedProbability(x - location_);}

        inline double cdf(const double x) const
            {return unshiftedCdf(x - location_);}

        inline double exceedance(const double x) const
            {return unshiftedExceedance(x - location_);}

        inline long quantile(const double x) const
            {return unshiftedQuantile(x) + location_;}
        //@}

        /** "Virtual copy constructor" */
        virtual ShiftableDiscreteDistribution1D* clone() const = 0;

        /** Method related to "geners" I/O */
        virtual gs::ClassId classId() const = 0;

    protected:
        /**
        // Derived classes should override the following method as long as
        // they have at least one additional data member. Don't forget to
        // call "isEqual" of the base class inside the derived classes.
        */
        virtual bool isEqual(const AbsDiscreteDistribution1D& other) const
        {
            const ShiftableDiscreteDistribution1D& r = 
                static_cast<const ShiftableDiscreteDistribution1D&>(other);
            return location_ == r.location_;
        }

    private:
        ShiftableDiscreteDistribution1D();

        virtual double unshiftedProbability(long x) const = 0;
        virtual double unshiftedCdf(double x) const = 0;
        virtual double unshiftedExceedance(double x) const = 0;
        virtual long unshiftedQuantile(double x) const = 0;

        long location_;
    };

    /**
    // Interface class for calculating some kind of a distance measure
    // between two distributions. If the distance is asymmetric, the
    // "reference" distribution (more precise in some sense) should
    // be the second argument of operator()(...). The pointer to the
    // pooled distribution can be NULL. If it is not NULL then it may
    // or may not be ignored by the distance functor.
    */
    struct AbsDiscreteDistribution1DDistance
    {
        inline virtual ~AbsDiscreteDistribution1DDistance() {}

        virtual double operator()(const AbsDiscreteDistribution1D& prob1,
                                  const AbsDiscreteDistribution1D& prob2,
                                  const AbsDiscreteDistribution1D* pooled,
                                  long first, long oneAboveLast) const = 0;
    };
}

#endif // NPSTAT_ABSDISCRETEDISTRIBUTION1D_HH_
