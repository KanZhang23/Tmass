#ifndef NPSI_MINUITQUANTILEREGRESSION1DFCN_HH_
#define NPSI_MINUITQUANTILEREGRESSION1DFCN_HH_

/*!
// \file MinuitQuantileRegression1DFcn.hh
//
// \brief Minuit function to minimize for local quantile regression
//        with single predictor
//
// Author: I. Volobouev
//
// April 2011
*/

#include "npstat/stat/QuantileRegression1D.hh"

#include "Minuit2/FCNBase.h"

namespace npsi {
    /**
    // Target minimization function adapter class for running local quantile
    // regression fits with one predictor by Minuit2
    */
    template <typename MyPoint, typename WeightType>
    class MinuitQuantileRegression1DFcn : public ROOT::Minuit2::FCNBase
    {
    public:
        /** This class will not own any pointers or references */
        inline MinuitQuantileRegression1DFcn(
            const npstat::QuantileRegression1D<MyPoint, WeightType>& qr,
            const double up = 0.05)
            : qr_(qr), up_(up) {}

        inline virtual ~MinuitQuantileRegression1DFcn() {}

        inline virtual double operator()(const std::vector<double>& x) const
        {
            assert(!x.empty());
            return qr_(&x[0], x.size());
        }

        inline double Up() const {return up_;}

    private:
        const npstat::QuantileRegression1D<MyPoint, WeightType>& qr_;
        double up_;
    };
}

#endif // NPSI_MINUITQUANTILEREGRESSION1DFCN_HH_
