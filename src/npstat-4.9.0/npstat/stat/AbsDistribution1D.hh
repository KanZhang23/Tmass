#ifndef NPSTAT_ABSDISTRIBUTION1D_HH_
#define NPSTAT_ABSDISTRIBUTION1D_HH_

/*!
// \file AbsDistribution1D.hh
//
// \brief Interface definition for 1-d continuous statistical distributions
//
// Author: I. Volobouev
//
// November 2009
*/

#include <cassert>
#include <typeinfo>
#include <stdexcept>

#include "geners/ClassId.hh"
#include "npstat/nm/SimpleFunctors.hh"
#include "npstat/rng/AbsRandomGenerator.hh"

namespace npstat {
    /**
    // All classes derived from this base should have copy constructors
    // (possibly automatic).
    //
    // quantile(0.0) and quantile(1.0) should return the boundaries
    // of the support interval.
    */
    struct AbsDistribution1D
    {
        inline virtual ~AbsDistribution1D() {}

        /** Probability density */
        virtual double density(double x) const = 0;

        /** Cumulative distribution function */
        virtual double cdf(double x) const = 0;

        /**
        // 1 - cdf, known as "survival function" or "exceedance".
        // Implementations should avoid subtractive cancellation.
        */
        virtual double exceedance(double x) const = 0;

        /** The quantile function (inverse cdf) */
        virtual double quantile(double x) const = 0;

        /**
        // Derived classes should not implement "operator==", implement
        // "isEqual" instead
        */
        inline bool operator==(const AbsDistribution1D& r) const
            {return (typeid(*this) == typeid(r)) && this->isEqual(r);}

        /** Logical negation of operator== */
        inline bool operator!=(const AbsDistribution1D& r) const
            {return !(*this == r);}

        /** "Virtual copy constructor" */
        virtual AbsDistribution1D* clone() const = 0;

        /**
        // Random number generator according to the given distribution.
        // Should return the number of random points used up from the
        // generator.
        */
        inline virtual unsigned random(AbsRandomGenerator& g,
                                       double* generatedRandom) const
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
            {return "npstat::AbsDistribution1D";}
        static inline unsigned version() {return 1;}
        static AbsDistribution1D* read(const gs::ClassId& id, std::istream&);

    protected:
        /** Comparison for equality. To be implemented by derived classes. */
        virtual bool isEqual(const AbsDistribution1D&) const = 0;

#ifdef SWIG
    public:
        inline double random2(AbsRandomGenerator& g) const
        {
            double d;
            this->random(g, &d);
            return d;
        }
#endif
    };

    /**
    // This base class is used to model distributions which have
    // "trivial" behavior w.r.t. location and scale parameters. That is,
    // if the distribution density is g(x), the density with location
    // parameter "mu" and scale parameter "sigma" is g((x - mu)/sigma)/sigma.
    //
    // All distributions with infinite support on both sides fall in this
    // category, as well as a large number of distributions with finite
    // support.
    */
    class AbsScalableDistribution1D : public AbsDistribution1D
    {
    public:
        /** Location and scale parameters must be provided */
        inline AbsScalableDistribution1D(const double location,
                                         const double scale) :
            location_(location), scale_(scale)
        {
            if (scale_ <= 0.0) throw std::invalid_argument(
                "In npstat::AbsScalableDistribution1D constructor: "
                "scale parameter must be positive");
        }

        inline virtual ~AbsScalableDistribution1D() {}

        /** Get the location parameter */
        inline double location() const {return location_;}

        /** Get the scale parameter */
        inline double scale() const {return scale_;}

        /** Set the location parameter */
        inline void setLocation(const double v) {location_ = v;}

        /** Set the scale parameter */
        inline void setScale(const double v)
        {
            if (v <= 0.0) throw std::invalid_argument(
                "In npstat::AbsScalableDistribution1D::setScale: "
                "scale parameter must be positive");
            scale_ = v;
        }

        //@{
        /** Method overriden from the AbsDistribution1D base class */
        inline double density(const double x) const
            {return unscaledDensity((x - location_)/scale_)/scale_;}

        inline double cdf(const double x) const
            {return unscaledCdf((x - location_)/scale_);}

        inline double exceedance(const double x) const
            {return unscaledExceedance((x - location_)/scale_);}

        inline double quantile(const double x) const
            {return scale_*unscaledQuantile(x) + location_;}
        //@}

        /** "Virtual copy constructor" */
        virtual AbsScalableDistribution1D* clone() const = 0;

        //@{
        /** Method related to "geners" I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream& os) const;
        //@}

        /**
        // Pseudo-read function for I/O. Note that this class is abstract,
        // so its instance can not be created.
        */
        static bool read(std::istream& is, double* location, double* scale);

    protected:
        /**
        // Derived classes should override the following method as long as
        // they have at least one additional data member. Don't forget to
        // call "isEqual" of the base class inside the derived classes.
        */
        virtual bool isEqual(const AbsDistribution1D& other) const
        {
            const AbsScalableDistribution1D& r = 
                static_cast<const AbsScalableDistribution1D&>(other);
            return location_ == r.location_ && scale_ == r.scale_;
        }

    private:
        AbsScalableDistribution1D();

        virtual double unscaledDensity(double x) const = 0;
        virtual double unscaledCdf(double x) const = 0;
        virtual double unscaledExceedance(double x) const = 0;
        virtual double unscaledQuantile(double x) const = 0;

        double location_;
        double scale_;
    };

    /**
    // A functor for the density function of the given 1-d distribution.
    // For this and subsequent functors, the distribution is not copied,
    // only a reference is used. It is a responsibility of the user to
    // make sure that the lifetime of the distribution object exceeds
    // the lifetime of the functor.
    */
    class DensityFunctor1D : public Functor1<double, double>
    {
    public:
        inline DensityFunctor1D(const AbsDistribution1D& fcn,
                                const double normfactor=1.0)
            : fcn_(fcn), norm_(normfactor) {}

        inline virtual ~DensityFunctor1D() {}

        inline virtual double operator()(const double& a) const
            {return norm_*fcn_.density(a);}

    private:
        DensityFunctor1D();
        const AbsDistribution1D& fcn_;
        const double norm_;
    };

    /** A functor for the density squared of the given 1-d distribution */
    class DensitySquaredFunctor1D : public Functor1<double, double>
    {
    public:
        inline DensitySquaredFunctor1D(const AbsDistribution1D& fcn,
                                       const double normfactor=1.0)
            : fcn_(fcn), norm_(normfactor) {}

        inline virtual ~DensitySquaredFunctor1D() {}

        inline virtual double operator()(const double& a) const
        {
            const double d = fcn_.density(a);
            return norm_*norm_*d*d;
        }

    private:
        DensitySquaredFunctor1D();
        const AbsDistribution1D& fcn_;
        const double norm_;
    };

    /** 
    // A functor for the cumulative distribution function of
    // the given 1-d distribution
    */
    class CdfFunctor1D : public Functor1<double, double>
    {
    public:
        inline CdfFunctor1D(const AbsDistribution1D& fcn,
                            const double normfactor=1.0)
            : fcn_(fcn), norm_(normfactor) {}

        inline virtual ~CdfFunctor1D() {}

        inline virtual double operator()(const double& a) const
            {return norm_*fcn_.cdf(a);}

    private:
        CdfFunctor1D();
        const AbsDistribution1D& fcn_;
        const double norm_;
    };

    /** A functor for the exceedance function of the given 1-d distribution */
    class ExceedanceFunctor1D : public Functor1<double, double>
    {
    public:
        inline ExceedanceFunctor1D(const AbsDistribution1D& fcn,
                                   const double normfactor=1.0)
            : fcn_(fcn), norm_(normfactor) {}

        inline virtual ~ExceedanceFunctor1D() {}

        inline virtual double operator()(const double& a) const
            {return norm_*fcn_.exceedance(a);}

    private:
        ExceedanceFunctor1D();
        const AbsDistribution1D& fcn_;
        const double norm_;
    };

    /** A functor for the quantile function of the given 1-d distribution */
    class QuantileFunctor1D : public Functor1<double, double>
    {
    public:
        inline QuantileFunctor1D(const AbsDistribution1D& fcn,
                                 const double normfactor=1.0)
            : fcn_(fcn), norm_(normfactor) {}

        inline virtual ~QuantileFunctor1D() {}

        inline virtual double operator()(const double& a) const
            {return fcn_.quantile(a/norm_);}

    private:
        QuantileFunctor1D();
        const AbsDistribution1D& fcn_;
        const double norm_;
    };
}

#endif // NPSTAT_ABSDISTRIBUTION1D_HH_
