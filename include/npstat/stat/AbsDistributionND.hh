#ifndef NPSTAT_ABSDISTRIBUTIONND_HH_
#define NPSTAT_ABSDISTRIBUTIONND_HH_

/*!
// \file AbsDistributionND.hh
//
// \brief Interface definition for multivariate continuous statistical
//        distributions
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>
#include <typeinfo>

#include "npstat/nm/AbsMultivariateFunctor.hh"
#include "geners/ClassId.hh"

namespace npstat {
    struct AbsRandomGenerator;

    /** All classes derived from this base should have copy constructors */
    class AbsDistributionND
    {
    public:
        explicit AbsDistributionND(const unsigned dim);
        AbsDistributionND(const AbsDistributionND& r);
        AbsDistributionND& operator=(const AbsDistributionND& r);

        inline virtual ~AbsDistributionND() {}

        /** "Virtual copy constructor" */
        virtual AbsDistributionND* clone() const = 0;

        /**
        // The comparison operator is needed mostly in order to
        // support I/O testing. Because of this, do not expect
        // it to work well for classes which do not implement
        // the "isEqual" method. Moreover, derived classes should
        // not implement "operator==", provide only "isEqual" if needed.
        */
        bool operator==(const AbsDistributionND& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const AbsDistributionND& r) const
            {return !(*this == r);}

        /** Dimensionality of the density */
        inline unsigned dim() const {return dim_;}

        /** Probability density */
        virtual double density(const double* x, unsigned dim) const = 0;

        /**
        // Mapping from the unit hypercube into the density support
        // region. Note that "bufLen" does not have to be equal to
        // the dimensionality of the function. There may be an
        // efficient way to generate just the leading dimensions
        // in case "bufLen" is smaller than the dimensionality.
        */
        virtual void unitMap(const double* rnd,
                             unsigned bufLen, double* x) const = 0;

        /**
        // The following method should return "true" in case the
        // "unitMap" method is implemented by a sequence of conditional
        // quantile functions. Distributions with such maps permit
        // quantile-based interpolation procedures.
        */
        virtual bool mappedByQuantiles() const = 0;

        /**
        // Random number generator according to the given distribution.
        // Should return the number of random points used up from the
        // generator. Length of the provided buffer "x" should be equal
        // to the function dimensionality.
        */
        virtual unsigned random(AbsRandomGenerator& g,
                                double* x, unsigned lenX) const;

        //@{
        /** Prototype needed for I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream&) const {return false;}
        //@}

        static inline const char* classname() {return "npstat::AbsDistributionND";}
        static inline unsigned version() {return 1;}
        static AbsDistributionND* read(const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDistributionND&) const = 0;

        // Distribution dimensionality
        const unsigned dim_;

    private:
        AbsDistributionND();

        // Memory buffer for generating random numbers
        std::vector<double> temp_;
        double* ws_;

#ifdef SWIG
    public:
        inline std::vector<double> random2(AbsRandomGenerator& g) const
        {
            std::vector<double> buffer(dim_);
            random(g, &buffer[0], dim_);
            return buffer;
        }
#endif
    };

    /**
    // Distribution which is scalable and shiftable in each
    // direction separately (but can not be rotated)
    */
    class AbsScalableDistributionND : public AbsDistributionND
    {
    public:
        /**
        // Location and scale parameters must be provided.
        // The length of the location and scale arrays must be
        // equal to the dimensionality of the density support.
        */
        AbsScalableDistributionND(const double* location,
                                  const double* scale, unsigned dim);
        inline virtual ~AbsScalableDistributionND() {}

        /** Get the location parameter for the given coordinate */
        inline double location(unsigned i) const {return location_.at(i);}

        /** Get the scale parameter for the given coordinate */
        inline double scale(unsigned i) const {return scale_.at(i);}

        /** Set the location parameter for the given coordinate */
        inline void setLocation(unsigned i, double v) {location_.at(i) = v;}

        /** Set the scale parameter for the given coordinate */
        void setScale(unsigned i, double v);

        //@{
        /** Method overriden from the AbsDistributionND base class */
        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned dim, double* x) const;
        //@}

        /** "Virtual copy constructor" */
        virtual AbsScalableDistributionND* clone() const = 0;

        /** 
        // Is the mappling from the unit cube to the support region
        // performed by the conditional quantile functions?
        */
        virtual bool mappedByQuantiles() const = 0;

        //@{
        /** Method related to "geners" I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream& os) const;
        //@}

        /**
        // Pseudo-read function for I/O. Note that this class is abstract,
        // so its instance can not be created.
        */
        static bool read(std::istream& is, unsigned* dim,
                         std::vector<double>* locations,
                         std::vector<double>* scales);
    protected:
        /**
        // Derived classes should override the following method as long as
        // they have at least one additional data member. Don't forget to
        // call "isEqual" of the base class inside the derived class.
        */
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        virtual double unscaledDensity(const double* x) const = 0;
        virtual void unscaledUnitMap(const double* rnd, unsigned bufLen,
                                     double* x) const = 0;
        std::vector<double> location_;
        std::vector<double> scale_;
        mutable std::vector<double> work_;
        double scaleProd_;

#ifdef SWIG
    public:
        inline AbsScalableDistributionND(
            const double* location, unsigned lenLocation,
            const double* scale, unsigned lenScale);
#endif
    };

    /**
    // Product distribution in which every marginal is represented by
    // the same class. In the template below, Distro1D should be derived
    // from AbsDistribution1D.
    */
    template<class Distro1D>
    class HomogeneousProductDistroND : public AbsDistributionND
    {
    public:
        /**
        // The marginals_ vector should be filled by the constructors
        // of the derived classes
        */
        inline explicit HomogeneousProductDistroND(unsigned dim)
            : AbsDistributionND(dim) {marginals_.reserve(dim);}

        inline virtual ~HomogeneousProductDistroND() {}

        virtual HomogeneousProductDistroND* clone() const = 0;

        inline bool mappedByQuantiles() const {return true;}

        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned bufLen, double* x) const;

    protected:
        virtual bool isEqual(const AbsDistributionND& r) const;

    protected:
        std::vector<Distro1D> marginals_;
    };

    /**
    // A functor for the density function of the given multivariate
    // distribution which implements AbsMultivariateFunctor interface
    */
    class DensityFunctorND : public AbsMultivariateFunctor
    {
    public:
        inline explicit DensityFunctorND(const AbsDistributionND& fcn)
            : fcn_(fcn) {}

        inline virtual ~DensityFunctorND() {}

        inline virtual double operator()(const double* pt, unsigned dim) const
            {return fcn_.density(pt, dim);}

        inline virtual unsigned minDim() const {return fcn_.dim();}

    private:
        DensityFunctorND();
        const AbsDistributionND& fcn_;
    };
}

#include "npstat/stat/AbsDistributionND.icc"

#endif // NPSTAT_ABSDISTRIBUTIONND_HH_
