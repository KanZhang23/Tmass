#ifndef NPSI_MINUITQUANTILEREGRESSIONNDFCN_HH_
#define NPSI_MINUITQUANTILEREGRESSIONNDFCN_HH_

/*!
// \file MinuitQuantileRegressionNDFcn.hh
//
// \brief Minuit function to minimize for local quantile regression
//        with multiple predictors
//
// Author: I. Volobouev
//
// October 2011
*/

#include "npstat/stat/LocalQuantileRegression.hh"

#include "Minuit2/FCNBase.h"

namespace npsi {
    /**
    // Target minimization function adapter class for running local quantile
    // regression fits with multiple predictors by Minuit2
    */
    template <typename Numeric>
    class MinuitQuantileRegressionNDFcn : public ROOT::Minuit2::FCNBase
    {
    public:
        /** This class will not own any pointers or references */
        inline MinuitQuantileRegressionNDFcn(
            npstat::QuantileRegressionBase<Numeric>& qr,
            const unsigned maxdeg, const double up = 0.05)
            : qr_(qr), up_(up)
        {
            assert(up_ > 0.0);
            setMaxDeg(maxdeg);
        }

        inline virtual ~MinuitQuantileRegressionNDFcn() {}

        /** Set maximum degree of the fitted polynomial (0, 1, or 2) */
        inline void setMaxDeg(const unsigned maxdeg)
        {
            switch (maxdeg)
            {
            case 0U:
                npara_ = 1U;
                break;
            case 1U:
                npara_ = qr_.dim() + 1U;
                break;
            case 2U:
                npara_ = ((qr_.dim() + 1U)*(qr_.dim() + 2U))/2U;
                break;
            default:
                assert(!"Invalid maximum polynomial degree");
            }
        }

        inline unsigned nParameters() const {return npara_;}

        inline virtual double operator()(const std::vector<double>& x) const
        {
            const unsigned len = x.size();
            assert(len >= npara_);
            return (const_cast<npstat::QuantileRegressionBase<
                    Numeric>&>(qr_)).linearLoss(&x[0], len);
        }

        inline double Up() const {return up_;}

    private:
        npstat::QuantileRegressionBase<Numeric>& qr_;
        double up_;
        unsigned npara_;
    };
}

#endif // NPSI_MINUITQUANTILEREGRESSIONNDFCN_HH_
