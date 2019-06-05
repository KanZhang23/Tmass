#ifndef NPSTAT_INTERPOLATEDDISTRIBUTION1D_HH_
#define NPSTAT_INTERPOLATEDDISTRIBUTION1D_HH_

/*!
// \file InterpolatedDistribution1D.hh
//
// \brief Interpolation of 1-d distributions via linear interpolation
//        of quantile functions
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>

#include "npstat/stat/AbsInterpolatedDistribution1D.hh"
#include "npstat/stat/WeightedDistro1DPtr.hh"

namespace npstat {
    /**
    // 1-d continuous statistical distribution which interpolates between
    // other distributions by performing linear interpolation of the
    // quantile function.
    //
    // Note that the interpolated distributions must still exist while this
    // object is in use (this class neither owns nor copies them).
    */
    class InterpolatedDistribution1D : public AbsInterpolatedDistribution1D
    {
    public:
        /**
        // It is expected that the object will be constructed
        // incrementally: first, the constructor will be called
        // using the expected number of distributions to use
        // and then function "add" will be used to put the
        // distributions and weights in. Use the default constructor
        // if the number of distributions is not known in advance
        // (the class will still operate as expected but the
        //  performance will take a hit).
        */
        explicit InterpolatedDistribution1D(unsigned expectedNDistros);

        InterpolatedDistribution1D();

        inline virtual ~InterpolatedDistribution1D() {}

        inline virtual InterpolatedDistribution1D* clone() const
            {return new InterpolatedDistribution1D(*this);}

        /** Add a new distribution */
        void add(const AbsDistribution1D& d, double weight);

        /** Replace an existing distribution */
        void replace(unsigned i, const AbsDistribution1D& d, double weight);

        /** Modify the weight for an existing dostribution */
        void setWeight(unsigned i, double weight);

        /** Clear all distributions */
        void clear();

        /**
        // The following function should be called to disable
        // (and later enable) automatic weight normalization
        // if you want to use the "setWeight" or "replace" methods
        // many times and, especially, if at some point in this process
        // the sum of the weights becomes zero. The "density" method
        // can not be called if normalization is not enabled.
        */
        void normalizeAutomatically(bool allow);

        /** The number of distributions participating in the interpolation */
        inline unsigned size() const {return count_;}

        inline unsigned expectedSize() const {return nExpected_;}

        // Methods inherited from AbsDistribution1D
        double density(double x) const;
        double cdf(double x) const;
        double exceedance(double x) const;
        double quantile(double x) const;

        /**
        // Return both density and cumulative distribution function.
        // More efficient than obtaining these two results separately.
        */
        void densityAndCdf(double x, double* pdensity, double* pcdf) const;

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname()
            {return "npstat::InterpolatedDistribution1D";}
        static inline unsigned version() {return 1;}

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        void normalize();

        std::vector<Private::WeightedDistro1DPtr> wdmem_;
        unsigned nExpected_;
        unsigned count_;
        double wsum_;
        double xmin_;
        double xmax_;
        bool autoNormalize_;
    };
}

#endif // NPSTAT_INTERPOLATEDDISTRIBUTION1D_HH_
