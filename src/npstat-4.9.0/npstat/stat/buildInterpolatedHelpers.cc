#include <cmath>

#include "npstat/nm/findRootInLogSpace.hh"

#include "npstat/stat/buildInterpolatedHelpers.hh"
#include "npstat/stat/amiseOptimalBandwidth.hh"

namespace {
    class SymbetaNeffFunctor : public npstat::Functor1<double,double>
    {
    public:
        inline SymbetaNeffFunctor(const int symbetaPower,
                                  const double* coords, const unsigned dim)
            : symbeta_(0.0, 1.0, symbetaPower > 0 ? symbetaPower : 0),
              gauss_(0.0, 1.0), coords_(coords), symbetaPow_(symbetaPower),
              dim_(dim) {}

        double operator()(const double& h) const
        {
            assert(h > 0.0);
            const npstat::AbsDistribution1D& g = symbetaPow_ < 0 ?
                dynamic_cast<const npstat::AbsDistribution1D&>(gauss_) :
                dynamic_cast<const npstat::AbsDistribution1D&>(symbeta_);
            double prod = 1.0;
            for (unsigned idim=0; idim<dim_; ++idim)
            {
                const double a = (0.0-coords_[idim])/h;
                const double b = (1.0-coords_[idim])/h;
                const double integ = g.cdf(b) - g.cdf(a);
                const double r = npstat::integralOfSymmetricBetaSquared(
                    symbetaPow_, a, b);
                prod *= (h*integ*integ/r);
            }
            return prod;
        }

    private:
        npstat::SymmetricBeta1D symbeta_;
        npstat::Gauss1D gauss_;
        const double* coords_;
        int symbetaPow_;
        unsigned dim_;
    };
}

namespace npstat {
    namespace Private {
        double symbetaNumSigmas(const int symbetaPower, const unsigned dim)
        {
            if (symbetaPower < 0)
                return 5.0 + sqrt(dim);
            else
                return 1.0;
        }

        double symbetaBandwidthInsideUnitBox(
            const int symbetaPower, const double nEffToNRatio,
            const double* coords, const unsigned dim)
        {
            assert(coords);
            assert(dim);
            assert(nEffToNRatio > 0.0);
            assert(nEffToNRatio < 1.0);

            // First, try to see if we are very deep inside the box
            // and the number of effective points is small
            const double sigmaRange = symbetaNumSigmas(symbetaPower, dim);
            const double Rinf = integralOfSymmetricBetaSquared(symbetaPower);
            const double hinf = pow(nEffToNRatio, 1.0/dim)*Rinf;
            bool wellInside = true;
            for (unsigned idim=0; idim<dim && wellInside; ++idim)
            {
                const double coordmin = coords[idim] - sigmaRange*hinf;
                const double coordmax = coords[idim] + sigmaRange*hinf;
                if (coordmin < 0.0 || coordmax > 1.0)
                    wellInside = false;
            }
            if (wellInside)
                return hinf;

            // At this point we are close to at least one of the
            // boundaries and must take boundary effects into account
            SymbetaNeffFunctor fcn(symbetaPower, coords, dim);
            const double tol = 1.0e-10;
            double h = 0.0;
            const bool status = findRootInLogSpace(fcn, nEffToNRatio,
                                                   hinf, tol, &h, 0.02);
            assert(status);
            return h;
        }

        double symbetaEffRatioInsideUnitBox(
            const int symbetaPower, const double bandwidth,
            const double* coords, const unsigned dim)
        {
            SymbetaNeffFunctor fcn(symbetaPower, coords, dim);
            return fcn(bandwidth);
        }
    }
}
