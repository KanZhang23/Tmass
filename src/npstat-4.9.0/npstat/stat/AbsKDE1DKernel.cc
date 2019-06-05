#include <stdexcept>

#include "npstat/stat/AbsKDE1DKernel.hh"

namespace npstat {
    double AbsKDE1DKernel::looKde(const double bandwidth,
                                  const unsigned long nCoords,
                                  const double original) const
    {
        if (nCoords < 2UL) throw std::invalid_argument(
            "In npstat::AbsKDE1DKernel::looKde: insufficient sample size");
        const double selfContrib = (*this)(0.0)/bandwidth/nCoords;
        return (original - selfContrib)*nCoords/(nCoords - 1UL);
    }

    double AbsKDE1DKernel::momentIntegral(const unsigned k,
                                          const unsigned nIntegPoints) const
    {
        GaussLegendreQuadrature glq(nIntegPoints);
        MomentFcn fcn(*this, k);
        return glq.integrate(fcn, this->xmin(), this->xmax());
    }

    double AbsKDE1DKernel::squaredIntegral(const unsigned nIntegPoints) const
    {
        GaussLegendreQuadrature glq(nIntegPoints);
        SquareFcn fcn(*this);
        return glq.integrate(fcn, this->xmin(), this->xmax());
    }
}
