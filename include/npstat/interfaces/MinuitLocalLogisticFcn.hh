#ifndef NPSI_MINUITLOCALLOGISTICFCN_HH_
#define NPSI_MINUITLOCALLOGISTICFCN_HH_

/*!
// \file MinuitLocalLogisticFcn.hh
//
// \brief Minuit function to minimize for local logistic regression
//
// Author: I. Volobouev
//
// March 2010
*/

#include <cassert>

#include "Minuit2/FCNGradientBase.h"
#include "npstat/stat/LocalLogisticRegression.hh"

namespace npsi {
    /**
    // Target minimization function adapter class for running local logistic
    // regression fits by Minuit2
    */
    template <class Numeric>
    class MinuitLocalLogisticFcn : public ROOT::Minuit2::FCNGradientBase
    {
    public:
        /**
        // Will not assume ownership the LogisticRegressionBase object
        */
        inline MinuitLocalLogisticFcn(
            npstat::LogisticRegressionBase<Numeric>* reg,
            const unsigned maxdeg, const double up=0.05)
            : reg_(reg), up_(up), force_(true)
        {
            assert(reg_);
            assert(up_ > 0.0);
            setMaxDeg(maxdeg);
        }

        inline virtual ~MinuitLocalLogisticFcn() {}

        inline void setMaxDeg(const unsigned maxdeg)
        {
            switch (maxdeg)
            {
            case 0U:
                npara_ = 1U;
                break;
            case 1U:
                npara_ = reg_->dim() + 1U;
                break;
            case 2U:
                npara_ = ((reg_->dim() + 1U)*(reg_->dim() + 2U))/2U;
                break;
            default:
                assert(!"Invalid maximum polynomial degree");
            }
        }

        inline unsigned nParameters() const {return npara_;}

        // Make Minuit look for a better minimum by reducing
        // the recommended "Up" value by an order of magnitude
        inline double Up() const {return up_;}

        // Do not waste time checking gradient calculations. They are correct.
        inline bool CheckGradient() const {return false;}

        inline virtual double operator()(const std::vector<double>& x) const
        {
            calculateMemoized(x);
            return lastResult_;
        }

        inline std::vector<double> Gradient(const std::vector<double>& x) const
        {
            // Returning std::vector<double> on the stack is
            // a pretty inefficient thing to do. However,
            // this is how Minuit2 interface is designed.
            calculateMemoized(x);
            std::vector<double> result(x.size(), 0.0);
            reg_->getGradient(&result[0], npara_);
            return result;
        }

    private:
        MinuitLocalLogisticFcn();

        // Minuit2 has separate interfaces for calculating the function
        // value and the gradient. I have yet to see a non-trivial realistic
        // problem in which the function value can not be easily obtained
        // along with the gradient. It would be much better to have them
        // calculated together. We will alleviate this design problem
        // by memoizing the argument / result pairs so that we do not
        // have to repeat the calculation when the gradient is wanted
        // at the same point where the function was just evaluated.
        void calculateMemoized(const std::vector<double>& x) const
        {
            assert(x.size() >= npara_);
            bool recalculate = 
                reg_->lastCoeffs().size() != npara_ || force_;
            if (!recalculate)
            {
                const double* lastX = &reg_->lastCoeffs()[0];
                const double* px = &x[0];
                for (unsigned i=0; i<npara_; ++i)
                    if (px[i] != lastX[i])
                    {
                        recalculate = true;
                        break;
                    }
            }
            if (recalculate)
            {
                force_ = false;
                lastResult_ = reg_->calculateLogLikelihood(&x[0], npara_);
            }
        }

        npstat::LogisticRegressionBase<Numeric>* reg_;
        double up_;
        mutable double lastResult_;
        unsigned npara_;
        mutable bool force_;
    };
}

#endif // NPSI_MINUITLOCALLOGISTICFCN_HH_
