#ifndef NPSTAT_KDE1D_HH_
#define NPSTAT_KDE1D_HH_

/*!
// \file KDE1D.hh
//
// \brief Convenience class which aggregates the kernel and the data
//        for brute-force 1-d KDE without boundary correction
//
// Author: I. Volobouev
//
// November 2018
*/

#include <cassert>
#include <stdexcept>
#include <vector>
#include <algorithm>

#include "npstat/stat/KDE1DCV.hh"

namespace npstat {
    template <class Numeric>
    class KDE1D
    {
    public:
        inline KDE1D(const AbsKDE1DKernel& i_kernel,
                     const Numeric* i_coords, const unsigned long i_nCoords)
            : kernel_(0), coords_(i_coords, i_coords + i_nCoords)
        {
            if (!i_nCoords) throw std::invalid_argument(
                "In npstat::KDE1D constructor: empty point sample");
            assert(i_coords);
            std::sort(coords_.begin(), coords_.end());
            kernel_ = i_kernel.clone();
        }

        inline KDE1D(const KDE1D& r)
            : kernel_(r.kernel_->clone()), coords_(r.coords_) {}

        inline KDE1D& operator=(const KDE1D& r)
        {
            if (&r != this)
            {
                delete kernel_; kernel_ = 0;
                coords_ = r.coords_;
                kernel_ = r.kernel_->clone();
            }
            return *this;
        }

        inline ~KDE1D() {delete kernel_;}

        inline const AbsKDE1DKernel& kernel() const {return *kernel_;}
        inline const std::vector<Numeric>& coords() const {return coords_;}
        inline unsigned long nCoords() const {return coords_.size();}
        inline Numeric minCoordinate() const {return coords_[0];}
        inline Numeric maxCoordinate() const {return coords_.back();}

        inline double density(const double x, const double bw) const
            {return kernel_->kde(x, bw, &coords_[0], coords_.size(), true);}

        /** 
        // The lifetime of the lightweight functor returned by this method
        // must not exceed the lifetime of this object
        */
        inline KDE1DFunctorHelper<Numeric> densityFunctor(
            const double bandwidth) const
        {
            return KDE1DFunctorHelper<Numeric>(
                *kernel_, bandwidth, &coords_[0], coords_.size(), true, false);
        }

        inline double integratedSquaredError(
            const AbsDistribution1D& distro,
            const unsigned nIntegIntervals, const unsigned nIntegPoints,
            const double bandwidth) const
        {
            return kernel_->integratedSquaredError(
                distro, nIntegIntervals, nIntegPoints, bandwidth,
                false, &coords_[0], coords_.size(), true);
        }

        inline double rlcv(const double bw, const double plcvAlpha) const
        {
            return KDE1DRLCVFunctor(*kernel_, plcvAlpha,
                                    &coords_[0], coords_.size(), true)(bw);
        }

        inline double lscv(const double bw,
                           const double xmin, const double xmax,
                           const unsigned nIntegIntervals,
                           const unsigned nIntegPoints) const
        {
            return KDE1DLSCVFunctor(*kernel_, xmin, xmax,
                                    nIntegIntervals, nIntegPoints,
                                    &coords_[0], coords_.size(), true)(bw);
        }

    private:
        AbsKDE1DKernel* kernel_;
        std::vector<Numeric> coords_;
    };
}

#endif // NPSTAT_KDE1D_HH_
