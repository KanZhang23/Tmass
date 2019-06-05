#ifndef NPSTAT_ABSFILTER1DBUILDER_HH_
#define NPSTAT_ABSFILTER1DBUILDER_HH_

/*!
// \file AbsFilter1DBuilder.hh
//
// \brief Abstract interface for building local polynomial filter weights in 1-d
//
// Author: I. Volobouev
//
// November 2009
*/

#include <vector>

#include "geners/ClassId.hh"
#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/SimpleFunctors.hh"

#include "npstat/stat/BoundaryHandling.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class OrthoPoly1D;

    /** 
    // Besides providing a table of weights, the filter should remember
    // for which point it was constructed (this info can potentially be
    // used in subsequent cross-validation calculations)
    */
    class PolyFilter1D : public std::vector<long double>
    {
    public:
        inline explicit PolyFilter1D(const unsigned peak) : peak_(peak) {}

        inline unsigned peakPosition() const {return peak_;}

        inline bool operator==(const PolyFilter1D& r) const
            {return peak_ == r.peak_ &&
                    static_cast<const std::vector<long double>&>(*this) ==
                    static_cast<const std::vector<long double>&>(r);}

        inline bool operator!=(const PolyFilter1D& r) const
            {return !(*this == r);}

        // Methods needed for I/O
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::PolyFilter1D";}
        static inline unsigned version() {return 1;}
        static PolyFilter1D* read(const gs::ClassId& id, std::istream& in);

    private:
        PolyFilter1D();
        unsigned peak_;
    };

    /** 
    // Abstract interface class for building local polynomial filter
    // weights in 1-d
    */
    struct AbsFilter1DBuilder
    {
        inline virtual ~AbsFilter1DBuilder() {}

        /**
        // Length of the filter constructed at a point deeply inside
        // the density support region
        */
        virtual unsigned centralWeightLength() const = 0;

        /**
        // Should we keep all filters or can we assume that
        // filters deeply inside the density support region
        // are identical?
        */
        virtual bool keepAllFilters() const = 0;

        /**
        // Build the filter from the given taper function, maximum
        // polynomial degree, bin number for which this filter is
        // constructed, and expected length of the data. The filter
        // is constructed on the heap and later must be deleted.
        */
        virtual PolyFilter1D* makeFilter(const double* taper,
                                         unsigned maxDegree,
                                         unsigned binnum,
                                         unsigned datalen) const = 0;

        /**
        // Some filter builders may adjust boundary kernels by
        // stretching them. In this case it may be interesting
        // to see the bandwidth factor used.
        */
        inline virtual double lastBandwidthFactor() const {return 1.0;}
    };

    /** 
    // Abstract interface class for building local polynomial filter
    // weights in 1-d via orthogonal polynomial systems
    */
    struct OrthoPolyFilter1DBuilder : public AbsFilter1DBuilder
    {
        inline virtual ~OrthoPolyFilter1DBuilder() {}

        /** Implemented from the base */
        virtual PolyFilter1D* makeFilter(const double* taper,
                                         unsigned maxDegree,
                                         unsigned binnum,
                                         unsigned datalen) const;
        /**
        // Build the orthogonal polynomial system that can later
        // be used to construct filters with different tapers.
        // This is constructed on the heap and later must be deleted.
        */
        virtual OrthoPoly1D* makeOrthoPoly(unsigned maxDegree,
                                           unsigned binnum,
                                           unsigned datalen,
                                           unsigned* filterCenter) const = 0;
    };

    /**
    // Abstract base clase for adjusting the weight function bandwidth near
    // the boundaries by keeping the value of a certain criterion constant.
    // Usually, this criterion is some kind of a functional (e.g., the
    // standard deviation of the weight function) which can be changed
    // by increasing the bandwidth.
    */
    class AbsBoundaryFilter1DBuilder : public OrthoPolyFilter1DBuilder
    {
    public:
        /**
        // This class will not own the AbsDistribution1D object.
        // It is the responsibility of the user of this class
        // to make sure that the AbsDistribution1D object stays
        // alive while this object is in use.
        //
        // It will be assumed that the distribution is symmetric
        // about x = 0.0.
        */
        AbsBoundaryFilter1DBuilder(const AbsDistribution1D* distro,
                                   double centralStepSize,
                                   const unsigned char* exclusionMask = 0,
                                   unsigned exclusionMaskLen = 0,
                                   bool excludeCentralPoint = false);

        inline virtual ~AbsBoundaryFilter1DBuilder() {}

        inline virtual unsigned centralWeightLength() const {return maxlen_;}

        inline virtual bool keepAllFilters() const
            {return !exclusionMask_.empty();}

        virtual OrthoPoly1D* makeOrthoPoly(unsigned maxDegree,
                                           unsigned binnum,
                                           unsigned datalen,
                                           unsigned* filterCenter) const;

        /** 
        // Return "true" for methods which fold the weight function
        // at the boundary
        */
        virtual bool isFolding() const = 0;

        /**
        // "lastBandwidthFactor" will be meaningfully defined after calling
        // the "makeOrthoPoly" method
        */
        inline virtual double lastBandwidthFactor() const
            {return lastBandwidthFactor_;}

    protected:
        void scanTheDensity(
            const AbsDistribution1D* distro, double h,
            int datalen, int weightCenterPos,
            double stepSize, double* workbuf,
            unsigned* firstWeightUsed=0, unsigned* sizeNeeded=0) const;

    private:
        AbsBoundaryFilter1DBuilder();

        /**
        // This function calculates the criterion which this class
        // will attempt to keep constant throughout the whole range
        // of x_fit by changing h
        */
        virtual double calculateCriterion(
            const AbsDistribution1D* distro, double h,
            int datalen, int weightCenterPos,
            double stepSize, double* workbuf) const = 0;

        class Fnc;
        friend class Fnc;
        class Fnc : public Functor1<double,double>
        {
        public:
            inline Fnc(const AbsBoundaryFilter1DBuilder& ref,
                       const unsigned datalen, const unsigned binnum)
                : ref_(ref), datalen_(datalen), binnum_(binnum) {}
            inline virtual ~Fnc() {}

            inline double operator()(const double& h) const
            {
                return ref_.calculateCriterion(ref_.distro_, h, datalen_,
                                        binnum_, ref_.step_, &ref_.w_[0]);
            }

        private:
            const AbsBoundaryFilter1DBuilder& ref_;
            unsigned datalen_;
            unsigned binnum_;
        };

        const AbsDistribution1D* distro_;
        double step_;
        mutable double lastBandwidthFactor_;
        mutable double centralIntegral_;
        unsigned maxlen_;
        mutable unsigned centralIntegralLen_;
        mutable std::vector<double> w_;
        std::vector<unsigned char> exclusionMask_;
        bool excludeCentralPoint_;
    };

    /**
    // A factory method for creating AbsBoundaryFilter1DBuilder objects.
    // Parameters are as follows:
    //
    //  boundaryHandling -- Definition of the boundary handling method.
    //
    //  distro           -- Function to use as the weight for generating
    //                      LOrPE polynomials. This weight is expected
    //                      to be even and to peak at 0. Note that the
    //                      filter builder will not own this pointer.
    //                      It is the responsibility of the user of this
    //                      code to make sure that the function exists
    //                      at all times when the builder is used.
    //
    //  stepSize         -- Step size (bin width) for the data grid on
    //                      which density estimation will be performed.
    //
    //  exclusionMask    -- Set values of "exclusionMask" != 0 if
    //                      corresponding data points have to be
    //                      excluded when weights are generated.
    //                      If no exclusions are necessary, just
    //                      leave this array as NULL.
    //
    //  exclusionMaskLen -- Length of the "exclusionMask" array. If
    //                      it is not 0 then it must coinside with
    //                      the "datalen" argument given to all future
    //                      invocations of the "makeFilter" method.
    //
    //  excludeCentralPoint -- If "true", the central point of the
    //                      weight will be set to zero. This can be
    //                      useful for certain types of cross validation
    //                      calculations.
    */
    CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> getBoundaryFilter1DBuilder(
        const BoundaryHandling& boundaryHandling,
        const AbsDistribution1D* distro, double stepSize,
        const unsigned char* exclusionMask = 0, unsigned exclusionMaskLen = 0,
        bool excludeCentralPoint = false);

#ifdef SWIG
    // Swig does not handle smart pointers properly
    inline AbsBoundaryFilter1DBuilder* getBoundaryFilter1DBuilder_2(
        const BoundaryHandling& boundaryMethod,
        const AbsDistribution1D* distro, double stepSize,
        const std::vector<int>* exclusionMask = 0,
        bool excludeCentralPoint = false)
    {
        typedef CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> BPtr;
        BPtr b;

        bool have_mask = false;
        if (exclusionMask)
            if (!exclusionMask->empty())
                have_mask = true;
        if (have_mask)
        {
            const unsigned n = exclusionMask->size();
            std::vector<unsigned char> mask(n, 0);
            for (unsigned i=0; i<n; ++i)
                if ((*exclusionMask)[i])
                    mask[i] = 1;
            b = getBoundaryFilter1DBuilder(
                boundaryMethod, distro, stepSize,
                &mask[0], n, excludeCentralPoint);
        }
        else
        {
            b = getBoundaryFilter1DBuilder(
                boundaryMethod, distro, stepSize,
                (unsigned char*)0, 0U, excludeCentralPoint);
        }
        return b.release();
    }
#endif
}

#endif // NPSTAT_ABSFILTER1DBUILDER_HH_
