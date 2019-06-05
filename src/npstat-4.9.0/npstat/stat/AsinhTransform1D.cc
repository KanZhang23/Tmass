#include <cmath>

#include "geners/binaryIO.hh"
#include "npstat/stat/AsinhTransform1D.hh"

namespace npstat {
    AsinhTransform1D::AsinhTransform1D(const double delta,
                                       const double lambda,
                                       const double gamma,
                                       const double xi)
        : AbsDistributionTransform1D(4U)
    {
        params_[0] = delta;
        params_[1] = lambda;
        params_[2] = gamma;
        params_[3] = xi;

        if (delta <= 0.0 || lambda <= 0.0) throw std::invalid_argument(
            "In npstat::AsinhTransform1D constructor: invalid parameters");
    }

    double AsinhTransform1D::transformForward(const double x, double* dydx) const
    {
        const double delta = (x - params_[3])/params_[1];
        if (dydx)
            *dydx = params_[0]/params_[1]/sqrt(1.0 + delta*delta);
        return params_[2] + params_[0]*asinh(delta);
    }

    double AsinhTransform1D::transformBack(const double y) const
    {
        return params_[3] + params_[1]*sinh((y - params_[2])/params_[0]);
    }

    bool AsinhTransform1D::isEqual(const AbsDistributionTransform1D& o) const
    {
        const AsinhTransform1D& r = static_cast<const AsinhTransform1D&>(o);
        for (unsigned i=0; i<4U; ++i)
            if (!(params_[i] == r.params_[i]))
                return false;
        return true;
    }

    bool AsinhTransform1D::write(std::ostream& os) const
    {
        gs::write_pod_array(os, params_, 4UL);
        return !os.fail();
    }

    AsinhTransform1D* AsinhTransform1D::read(const gs::ClassId& id,
                                             std::istream& is)
    {
        static const gs::ClassId current(gs::ClassId::makeId<AsinhTransform1D>());
        current.ensureSameId(id);

        double tmp[4];
        gs::read_pod_array(is, tmp, 4UL);
        if (is.fail()) throw gs::IOReadFailure(
            "In npstat::AsinhTransform1D::read: input stream failure");
        return new AsinhTransform1D(tmp[0], tmp[1], tmp[2], tmp[3]);
    }
}
