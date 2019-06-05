#ifndef NPSTAT_ABSKDE1DKERNEL_HH_
#define NPSTAT_ABSKDE1DKERNEL_HH_

/*!
// \file AbsKDE1DKernel.hh
//
// \brief Brute-force non-discretized KDE in 1-d. No boundary correction.
//
// Author: I. Volobouev
//
// November 2018
*/

#include <cmath>

#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /** Base class for 1-d kernels and KDE */
    class AbsKDE1DKernel
    {
    public:
        inline virtual ~AbsKDE1DKernel() {}

        virtual double xmin() const = 0;
        virtual double xmax() const = 0;
        virtual double operator()(double x) const = 0;

        /** "Virtual copy constructor" */
        virtual AbsKDE1DKernel* clone() const = 0;

        //@{
        /**
        // "nIntegPoints" argument in the following method is the
        // number of points to use in the quadrature rule. If this
        // method is not overriden, Gauss-Legendre quadrature is used.
        */
        virtual double momentIntegral(unsigned k, unsigned nIntegPoints) const;
        virtual double squaredIntegral(unsigned nIntegPoints) const;
        //@}

        /**
        // Summing K((x - x[i])/h), as in the standard KDE definition.
        // Setting "coordinatesSorted" flag to "true" if coordinates
        // are sorted in the increasing order can potentially increase
        // the speed of KDE evaluation.
        */
        template <typename Numeric>
        double kde(double x, double bandwidth,
                   const Numeric* coords, unsigned long nCoords,
                   bool coordinatesSorted = false) const;

        /** Summing K((x[i] - x)/h) */
        template <typename Numeric>
        double reverseKde(double x, double bandwidth,
                          const Numeric* coords, unsigned long nCoords,
                          bool coordinatesSorted = false) const;

        /**
        // Leave-one-out density estimate. The original density estimate
        // should be obtained earlier with either "kde" or "reverseKde"
        // method used with the same values of "bandwidth" and "nCoords"
        // arguments.
        */
        double looKde(double bandwidth, unsigned long nCoords,
                      double originalDensityEstimate) const;

        /**
        // Integrated squared error w.r.t. a known density. Parameter
        // "nIntegPoints" is the number of integration points per interval,
        // and it must be supported by the GaussLegendreQuadrature class.
        // The integration interval is defined by the support of the density.
        */
        template <typename Numeric>
        double integratedSquaredError(const AbsDistribution1D& distro,
                   unsigned nIntegIntervals, unsigned nIntegPoints,
                   double bandwidth, bool useReverseKde,
                   const Numeric* coords, unsigned long nCoords,
                   bool coordinatesSorted = false) const;

        /**
        // Integral of KDE squared. Parameter "nIntegPoints" is the number
        // of integration points per interval, and it must be supported by
        // the GaussLegendreQuadrature class.
        */
        template <typename Numeric>
        double integratedKdeSquared(
                   double xmin, double xmax,
                   unsigned nIntegIntervals, unsigned nIntegPoints,
                   double bandwidth, bool useReverseKde,
                   const Numeric* coords, unsigned long nCoords,
                   bool coordinatesSorted = false) const;
    protected:
        class MomentFcn : public Functor1<double, double>
        {
        public:
            inline MomentFcn(const AbsKDE1DKernel& ref, const unsigned power)
                : kernel(ref), xpow(power) {}
            inline double operator()(const double& x) const
                {return kernel(x)*(xpow ? pow(x, xpow) : 1.0);}
        private:
            const AbsKDE1DKernel& kernel;
            unsigned xpow;
        };

        class SquareFcn : public Functor1<double, double>
        {
        public:
            inline explicit SquareFcn(const AbsKDE1DKernel& ref)
                : kernel(ref) {}
            inline double operator()(const double& x) const
                {const double k = kernel(x); return k*k;}
        private:
            const AbsKDE1DKernel& kernel;
        };
    };

    /** Arbitrary 1-d densities used as kernel smoothers */
    class KDE1DDensityKernel : public AbsKDE1DKernel
    {
    public:
        inline explicit KDE1DDensityKernel(const AbsDistribution1D& d)
            : distro_(0) {distro_ = d.clone();}

        inline KDE1DDensityKernel(const KDE1DDensityKernel& r)
            : distro_(0) {distro_ = r.distro_->clone();}

        inline KDE1DDensityKernel& operator=(const KDE1DDensityKernel& r)
        {
            if (this != &r)
            {
                delete distro_;
                distro_ = 0;
                distro_ = r.distro_->clone();
            }
            return *this;
        }

        inline virtual KDE1DDensityKernel* clone() const
            {return new KDE1DDensityKernel(*this);}

        inline virtual ~KDE1DDensityKernel() {delete distro_;}

        inline double xmin() const {return distro_->quantile(0.0);}
        inline double xmax() const {return distro_->quantile(1.0);}
        inline double operator()(const double x) const
            {return distro_->density(x);}

    private:
        AbsDistribution1D* distro_;
    };

    /**
    // A lightweight functor for getting KDE values. It will not copy
    // the kernel and will not own the data. It is the responsibility
    // of the user of this class to ensure that the lifetimes of the
    // kernel and the data exceed the lifetime of the functor.
    //
    // Intended usage is via the "KDE1DFunctor" utility function.
    */
    template <typename Numeric>
    class KDE1DFunctorHelper : public Functor1<double, double>
    {
    public:
        inline KDE1DFunctorHelper(const AbsKDE1DKernel& kernel,
                                  const double bandwidth,
                                  const Numeric* coords,
                                  const unsigned long nCoords,
                                  const bool coordinatesSorted,
                                  const bool useReverseKde)
            : kernel_(kernel), bandwidth_(bandwidth),
              coords_(coords), nCoords_(nCoords),
              coordinatesSorted_(coordinatesSorted),
              useReverseKde_(useReverseKde) {}
        
        inline double operator()(const double& x) const
        {
            double kde;
            if (useReverseKde_)
                kde = kernel_.reverseKde(x, bandwidth_, coords_, nCoords_,
                                         coordinatesSorted_);
            else
                kde = kernel_.kde(x, bandwidth_, coords_, nCoords_,
                                  coordinatesSorted_);
            return kde;
        }

    private:
        const AbsKDE1DKernel& kernel_;
        double bandwidth_;
        const Numeric* coords_;
        unsigned long nCoords_;
        bool coordinatesSorted_;
        bool useReverseKde_;
    };

    /** A convenience function for making lightweight KDE functors */
    template <typename Numeric>
    inline KDE1DFunctorHelper<Numeric> KDE1DFunctor(
        const AbsKDE1DKernel& kernel, const double bandwidth,
        const Numeric* coords, const unsigned long nCoords,
        const bool coordinatesSorted = false, const bool useReverseKde = false)
    {
        return KDE1DFunctorHelper<Numeric>(kernel, bandwidth, coords, nCoords,
                                           coordinatesSorted, useReverseKde);
    }
}

#include "npstat/stat/AbsKDE1DKernel.icc"

#endif // NPSTAT_ABSKDE1DKERNEL_HH_
