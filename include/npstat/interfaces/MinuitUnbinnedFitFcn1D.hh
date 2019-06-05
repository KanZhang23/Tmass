#ifndef NPSI_MINUITUNBINNEDFITFCN1D_HH_
#define NPSI_MINUITUNBINNEDFITFCN1D_HH_

/*!
// \file MinuitUnbinnedFitFcn1D.hh
//
// \brief Unbinned fit of a parametric density by maximum likelihood
//
// Author: I. Volobouev
//
// July 2016
*/

#include <cfloat>

#include "npstat/stat/FitUtils.hh"

#include "Minuit2/FCNBase.h"

namespace npsi {
    /**
    // Target minimization function adapter class for running maximum
    // likelihood density fits to unbinned data by Minuit2.
    //
    // DensityConstructor is a functor which creates the necessary
    // density function on the stack out of a vector of parameters.
    // Must have "operator()(const std::vector<double>&) const"
    // which returns an object (or a reference) of some class which
    // was derived from AbsDistribution1D.
    */
    template <typename Numeric, class DensityConstructor>
    class MinuitUnbinnedFitFcn1D : public ROOT::Minuit2::FCNBase
    {
    public:
        /**
        // This class will not assume ownership of any pointers or references.
        */
        inline MinuitUnbinnedFitFcn1D(const Numeric* sampleData,
                                      const unsigned nSampleData,
                                      const DensityConstructor& densityMaker,
                                      const double minlog=log(DBL_MIN),
                                      const double up=0.05)
            : sampleData_(sampleData),
              nSampleData_(nSampleData),
              densityMaker_(densityMaker),
              minlog_(minlog),
              up_(up)
        {
        }

        inline virtual ~MinuitUnbinnedFitFcn1D() {}

        /** This method returns negative log likelihood */
        inline virtual double operator()(const std::vector<double>& x) const
        {
            return -npstat::unbinnedLogLikelihood1D(
                sampleData_, nSampleData_, densityMaker_(x), minlog_);
        }

        inline double Up() const {return up_;}

    private:
        const Numeric* sampleData_;
        unsigned nSampleData_;
        const DensityConstructor& densityMaker_;
        double minlog_;
        double up_;
    };
}

#endif // NPSI_MINUITUNBINNEDFITFCN1D_HH_
