#ifndef NPSTAT_DENSITYSCAN1D_HH_
#define NPSTAT_DENSITYSCAN1D_HH_

/*!
// \file DensityScan1D.hh
//
// \brief Utilities for discretizing 1-d densities
//
// Author: I. Volobouev
//
// March 2014
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /** A functor for filling one-dimensional ArrayND with density values */
    class DensityScan1D
    {
    public:
        /**
        // Parameter "nIntegrationPoints" determines how many points per bin
        // will be used to calculate the bin average. This number must be
        // either 0 (use cdf difference at the bin edges), 1 (use density
        // value at the center of the bin), or one of the numbers of points
        // supported by the "GaussLegendreQuadrature" class.
        */
        DensityScan1D(const AbsDistribution1D& fcn, double normfactor,
                      unsigned long nbins, double xmin, double xmax,
                      unsigned nIntegrationPoints=1);

        double operator()(const unsigned* index, unsigned len) const;

        inline double averageDensity(const unsigned binNumber) const
            {return (*this)(&binNumber, 1U);}

    private:
        DensityScan1D();

        const AbsDistribution1D& fcn_;
        double norm_;
        double xmin_;
        double bw_;
        std::vector<long double> a_;
        std::vector<long double> weights_;
        unsigned nPoints_;
    };

    /**
    // A functor for filling one-dimensional ArrayND with density values.
    //
    // Similar to DensityScan1D but can be used with an arbitrary transform
    // of the array index. The arbitrariness of the transform also prevents
    // us from integrating over the bin size.
    */
    template<class Transform>
    class DensityScan1DTrans
    {
    public:
        /**
        // The "transform" parameter will not be copied intrernally. The user
        // is responsible for ensuring the proper lifetime of this object.
        */
        inline DensityScan1DTrans(const AbsDistribution1D& fcn,
                                  const Transform& transform,
                                  const double normfactor) :
            fcn_(fcn), mapping_(transform), norm_(normfactor) {}

        inline double operator()(const unsigned* index, const unsigned len) const
        {
            if (len != 1U) throw std::invalid_argument(
                "In npstat::DensityScan1DTrans::operator(): "
                "incompatible input point dimensionality");
            assert(index);
            const double x = mapping_(static_cast<double>(index[0]));
            return norm_*fcn_.density(x);
        }

    private:
        DensityScan1DTrans();

        const AbsDistribution1D& fcn_;
        const Transform& mapping_;
        double norm_;
    };

    /**
    // A helper functor to be used for the determination of density
    // discretization errors (when the density is represented by
    // a collection of values on a grid)
    */
    class DensityDiscretizationError1D : public Functor1<double, double>
    {
    public:
        /** 
        // "discreteValue" is the density value on a particular
        // discretization interval. It is expected that the driver
        // code will integrate the L2 error returned by this functor
        // over all such intervals.
        */
        inline DensityDiscretizationError1D(const AbsDistribution1D& fcn,
                                            const double normfactor,
                                            const double discreteValue)
            : fcn_(fcn), norm_(normfactor), h_(discreteValue) {}

        inline virtual ~DensityDiscretizationError1D() {}

        inline virtual double operator()(const double& a) const
        {
            const double d = norm_*fcn_.density(a) - h_;
            return d*d;
        }

    private:
        DensityDiscretizationError1D();
        const AbsDistribution1D& fcn_;
        const double norm_;
        const double h_;
    };
}

#endif // NPSTAT_DENSITYSCAN1D_HH_
