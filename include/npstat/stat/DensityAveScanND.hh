#ifndef NPSTAT_DENSITYAVESCANND_HH_
#define NPSTAT_DENSITYAVESCANND_HH_

/*!
// \file DensityAveScanND.hh
//
// \brief Fill multivariate arrays with multivariate density values by
//        averaging inside the corresponding bins
//
// This class can be used with "functorFill" method of ArrayND (e.g., with
// histogram bin contents).
//
// Author: I. Volobouev
//
// June 2015
*/

#include <vector>
#include <cassert>
#include <stdexcept>

#include "npstat/nm/rectangleQuadrature.hh"
#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    template<class Axis>
    class DensityAveScanND
    {
    public:
        /**
        // This functor will NOT make copies of either "fcn" or "axes"
        // parameters. These parameters will be used by reference only
        // (aliased). It is up to the user of this class to ensure proper
        // lifetime of these objects.
        //
        // The density will be averaged over each bin using tensor
        // product quadrature with "nIntegrationPoints" in each dimension.
        */
        inline DensityAveScanND(const AbsDistributionND& fcn,
                                const std::vector<Axis>& axes,
                                const unsigned nIntegrationPoints,
                                double normfactor=1.0)
            : fcn_(fcn), axes_(axes), buf_(2U*fcn.dim()), norm_(normfactor),
              nInteg_(nIntegrationPoints), dim_(fcn.dim())
        {
            if (!(dim_ && dim_ == axes_.size())) throw std::invalid_argument(
                "In npstat::DensityAveScanND constructor: "
                "incompatible arguments");
        }

        inline double operator()(const unsigned* index,
                                 const unsigned indexLen) const
        {
            if (dim_ != indexLen) throw std::invalid_argument(
                "In npstat::DensityAveScanND::operator(): "
                "incompatible input point dimensionality");
            assert(index);
            double* center = &buf_[0];
            if (nInteg_ > 1U)
            {
                double volume = 1.0;
                double* size = center + dim_;
                for (unsigned i=0; i<dim_; ++i)
                {
                    center[i] = axes_[i].binCenter(index[i]);
                    const double width = axes_[i].binWidth(index[i]);
                    volume *= width;
                    size[i] = width;
                }
                const double integ = rectangleIntegralCenterAndSize(
                    fcn_, center, size, dim_, nInteg_);
                return norm_*integ/volume;
            }
            else
            {
                for (unsigned i=0; i<dim_; ++i)
                    center[i] = axes_[i].binCenter(index[i]);
                return norm_*fcn_(center, dim_);
            }
        }

    private:
        DensityAveScanND();

        DensityFunctorND fcn_;
        const std::vector<Axis>& axes_;
        mutable std::vector<double> buf_;
        double norm_;
        unsigned nInteg_;
        unsigned dim_;
    };

    // Helper function for creating DensityAveScanND objects
    template<class Axis>
    inline DensityAveScanND<Axis> makeDensityAveScanND(
        const AbsDistributionND& fcn, const std::vector<Axis>& axes,
        const unsigned nIntegPoints, double normfactor=1.0)
    {
        return DensityAveScanND<Axis>(fcn, axes, nIntegPoints, normfactor);
    }

    /**
    // A helper functor to be used for the determination of multivariate
    // density discretization errors (when the density is represented by
    // a collection of values on a grid)
    */
    class DensityDiscretizationErrorND : public AbsMultivariateFunctor
    {
    public:
        inline DensityDiscretizationErrorND(const AbsDistributionND& fcn,
                                            const double normfactor,
                                            const double discreteValue)
            : fcn_(fcn), norm_(normfactor), h_(discreteValue) {}

        inline virtual ~DensityDiscretizationErrorND() {}

        inline virtual double operator()(const double* pt, unsigned dim) const
        {
            const double d = norm_*fcn_.density(pt, dim) - h_;
            return d*d;
        }

        inline virtual unsigned minDim() const {return fcn_.dim();}

    private:
        DensityDiscretizationErrorND();
        const AbsDistributionND& fcn_;
        const double norm_;
        const double h_;
    };
}

#endif // NPSTAT_DENSITYAVESCANND_HH_
