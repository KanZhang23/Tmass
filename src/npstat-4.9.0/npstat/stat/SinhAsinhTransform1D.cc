#include <cmath>

#include "geners/binaryIO.hh"
#include "npstat/stat/SinhAsinhTransform1D.hh"

namespace npstat {
    SinhAsinhTransform1D::SinhAsinhTransform1D(const double a, const double b)
        : AbsDistributionTransform1D(2U)
    {
        params_[0] = a;
        params_[1] = b;

        if (b <= 0.0) throw std::invalid_argument(
            "In npstat::SinhAsinhTransform1D constructor: invalid parameters");
    }

    double SinhAsinhTransform1D::transformForward(const double x, double* dydx) const
    {
        const double a = params_[0];
        const double b = params_[1];
        if (dydx)
            *dydx = b*cosh(a + b*asinh(x))/sqrt(1.0 + x*x);
        return sinh(a + b*asinh(x));
    }

    double SinhAsinhTransform1D::transformBack(const double y) const
    {
        return sinh((asinh(y) - params_[0])/params_[1]);
    }

    bool SinhAsinhTransform1D::isEqual(const AbsDistributionTransform1D& o) const
    {
        const SinhAsinhTransform1D& r = static_cast<const SinhAsinhTransform1D&>(o);
        for (unsigned i=0; i<2U; ++i)
            if (!(params_[i] == r.params_[i]))
                return false;
        return true;
    }

    bool SinhAsinhTransform1D::write(std::ostream& os) const
    {
        gs::write_pod_array(os, params_, 2UL);
        return !os.fail();
    }

    SinhAsinhTransform1D* SinhAsinhTransform1D::read(const gs::ClassId& id,
                                                     std::istream& is)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<SinhAsinhTransform1D>());
        current.ensureSameId(id);

        double tmp[2];
        gs::read_pod_array(is, tmp, 2UL);
        if (is.fail()) throw gs::IOReadFailure(
            "In npstat::SinhAsinhTransform1D::read: input stream failure");
        return new SinhAsinhTransform1D(tmp[0], tmp[1]);
    }
}
