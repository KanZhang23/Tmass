#ifndef NPSTAT_KDE1DCV_HH_
#define NPSTAT_KDE1DCV_HH_

/*!
// \file KDE1DCV.hh
//
// \brief Cross-validation utilities for brute-force KDE in 1-d
//
// Author: I. Volobouev
//
// November 2018
*/

#include "npstat/stat/AbsKDE1DKernel.hh"

namespace npstat {
    /**
    // A lightweight functor for getting negative LSCV values (that is,
    // quantity to be maximized). It will not copy the kernel and will
    // not own the data. It is the responsibility of the user of this
    // class to ensure that the lifetimes of the kernel and the data
    // exceed the lifetime of the functor.
    //
    // Intended usage is via the "KDE1DLSCVFunctor" utility function.
    */
    template <typename Numeric>
    class KDE1DLSCVFunctorHelper : public Functor1<double, double>
    {
    public:
        inline KDE1DLSCVFunctorHelper(const AbsKDE1DKernel& kernel,
                                      const double xmin, const double xmax,
                                      const unsigned nIntegIntervals,
                                      const unsigned nIntegPoints,
                                      const Numeric* coords,
                                      const unsigned long nCoords,
                                      const bool coordinatesSorted,
                                      const bool useReverseKde)
            : kernel_(kernel), xmin_(xmin), xmax_(xmax),
              nIntegIntervals_(nIntegIntervals), nIntegPoints_(nIntegPoints),
              coords_(coords), nCoords_(nCoords),
              coordinatesSorted_(coordinatesSorted),
              useReverseKde_(useReverseKde) {}

        inline double operator()(const double& bw) const
        {
            long double msum = 0.0L;
            for (unsigned long i=0; i<nCoords_; ++i)
            {
                const double x = coords_[i];
                double kde;
                if (useReverseKde_)
                    kde = kernel_.reverseKde(x, bw, coords_, nCoords_,
                                             coordinatesSorted_);
                else
                    kde = kernel_.kde(x, bw, coords_, nCoords_,
                                      coordinatesSorted_);
                const double loo = kernel_.looKde(bw, nCoords_, kde);
                msum += loo;
            }
            const double integ = kernel_.integratedKdeSquared(
                xmin_, xmax_, nIntegIntervals_, nIntegPoints_,
                bw, useReverseKde_, coords_, nCoords_, coordinatesSorted_);
            return -(integ - msum*2.0/nCoords_);
        }

    private:
        const AbsKDE1DKernel& kernel_;
        double xmin_;
        double xmax_;
        unsigned nIntegIntervals_;
        unsigned nIntegPoints_;
        const Numeric* coords_;
        unsigned long nCoords_;
        bool coordinatesSorted_;
        bool useReverseKde_;
    };

    /** A convenience function for creating KDE1DLSCVFunctorHelper objects */
    template <typename Numeric>
    inline KDE1DLSCVFunctorHelper<Numeric> KDE1DLSCVFunctor(
        const AbsKDE1DKernel& kernel,
        const double xmin, const double xmax,
        const unsigned nIntegIntervals,
        const unsigned nIntegPoints,
        const Numeric* coords,
        const unsigned long nCoords,
        const bool coordinatesSorted = false,
        const bool useReverseKde = false)
    {
        return KDE1DLSCVFunctorHelper<Numeric>(
            kernel, xmin, xmax, nIntegIntervals, nIntegPoints,
            coords, nCoords, coordinatesSorted, useReverseKde);
    }

    /**
    // A lightweight functor for getting log RLCV values. It will not
    // copy the kernel and will not own the data. It is the responsibility
    // of the user of this class to ensure that the lifetimes of the kernel
    // and the data exceed the lifetime of the functor.
    //
    // Intended usage is via the "KDE1DRLCVFunctor" utility function.
    */
    template <typename Numeric>
    class KDE1DRLCVFunctorHelper : public Functor1<double, double>
    {
    public:
        inline KDE1DRLCVFunctorHelper(const AbsKDE1DKernel& kernel,
                                      const double plcvAlpha,
                                      const Numeric* coords,
                                      const unsigned long nCoords,
                                      const bool coordinatesSorted,
                                      const bool useReverseKde)
            : kernel_(kernel), plcvAlpha_(plcvAlpha),
              coords_(coords), nCoords_(nCoords),
              coordinatesSorted_(coordinatesSorted),
              useReverseKde_(useReverseKde) {}

        inline double operator()(const double& bw) const
        {
            const double nPt = nCoords_;
            const double selfC = kernel_(0.0)/bw/nPt;
            const double minDens = selfC/pow(nPt, plcvAlpha_);
            const double logm = log(minDens);

            long double msum = 0.0L;
            for (unsigned long i=0; i<nCoords_; ++i)
            {
                const double x = coords_[i];
                double kde;
                if (useReverseKde_)
                    kde = kernel_.reverseKde(x, bw, coords_, nCoords_,
                                             coordinatesSorted_);
                else
                    kde = kernel_.kde(x, bw, coords_, nCoords_,
                                      coordinatesSorted_);
                const double loo = kernel_.looKde(bw, nCoords_, kde);
                if (loo > minDens)
                    msum += log(loo);
                else
                    msum += logm;
            }
            return msum;
        }

    private:
        const AbsKDE1DKernel& kernel_;
        double plcvAlpha_;
        const Numeric* coords_;
        unsigned long nCoords_;
        bool coordinatesSorted_;
        bool useReverseKde_;
    };

    /** A convenience function for creating KDE1DRLCVFunctorHelper objects */
    template <typename Numeric>
    inline KDE1DRLCVFunctorHelper<Numeric> KDE1DRLCVFunctor(
        const AbsKDE1DKernel& kernel,
        const double plcvAlpha,
        const Numeric* coords,
        const unsigned long nCoords,
        const bool coordinatesSorted = false,
        const bool useReverseKde = false)
    {
        return KDE1DRLCVFunctorHelper<Numeric>(
            kernel, plcvAlpha, coords, nCoords,
            coordinatesSorted, useReverseKde);
    }
}

#endif // NPSTAT_KDE1DCV_HH_
