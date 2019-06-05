#ifndef NPSI_SCALABLEDENSITYCONSTRUCTOR1D_HH_
#define NPSI_SCALABLEDENSITYCONSTRUCTOR1D_HH_

/*!
// \file ScalableDensityConstructor1D.hh
//
// \brief Constructor of location-scale models for use with Minuit fitting
//        functions
//
// Author: I. Volobouev
//
// October 2013
*/

#include <vector>
#include <stdexcept>

#include "npstat/stat/AbsDistribution1D.hh"

namespace npsi {
    class ScalableDensityConstructor1D
    {
    public:
        inline explicit ScalableDensityConstructor1D(
            const npstat::AbsDistribution1D& fcn)
            : density_(0)
        {
            const npstat::AbsScalableDistribution1D* d = 
                dynamic_cast<const npstat::AbsScalableDistribution1D*>(&fcn);
            if (!d) throw std::invalid_argument(
                "In npsi::ScalableDensityConstructor1D::constructor : "
                "input distribution is not scalable");
            density_ = d->clone();
        }

        inline ScalableDensityConstructor1D(
            const ScalableDensityConstructor1D& r)
            : density_(r.density_->clone()) {}

        ScalableDensityConstructor1D& operator=(
            const ScalableDensityConstructor1D& r)
        {
            if (&r != this)
            {
                delete density_;
                density_ = 0;
                density_ = r.density_->clone();
            }
            return *this;
        }

        inline ~ScalableDensityConstructor1D() {delete density_;}

        inline const npstat::AbsDistribution1D& operator()(
            const std::vector<double>& x) const
        {
            if (x.size() != 2U) throw std::invalid_argument(
                "In npsi::ScalableDensityConstructor1D::operator() : "
                "unexpected number of parameters");

            density_->setScale(x[1]);
            density_->setLocation(x[0]);

            return *density_;
        }

    private:
        // Disable default constructor
        ScalableDensityConstructor1D();

        mutable npstat::AbsScalableDistribution1D* density_;
    };
}

#endif // NPSI_SCALABLEDENSITYCONSTRUCTOR1D_HH_
