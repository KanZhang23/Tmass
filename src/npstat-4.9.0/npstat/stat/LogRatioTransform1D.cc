#include <cmath>

#include "geners/binaryIO.hh"
#include "npstat/stat/LogRatioTransform1D.hh"

namespace npstat {
    LogRatioTransform1D::LogRatioTransform1D(const double delta,
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
            "In npstat::LogRatioTransform1D constructor: invalid parameters");
    }    

    double LogRatioTransform1D::transformForward(const double x, double* dydx) const
    {
        const double diff = (x - params_[3])/params_[1];
        const double onemdiff = 1.0 - diff;
        if (diff <= 0.0 || onemdiff <= 0.0) throw std::invalid_argument(
            "In npstat::LogRatioTransform1D::transformForward: argument is out of range");
        if (dydx)
            *dydx = params_[0]/params_[1]/diff/onemdiff;
        return params_[2] + params_[0]*log(diff/onemdiff);
    }

    double LogRatioTransform1D::transformBack(const double z) const
    {
        const double tmp = exp((z - params_[2])/params_[0]);
        return params_[3] + params_[1]*tmp/(1.0 + tmp);
    }

    bool LogRatioTransform1D::isEqual(const AbsDistributionTransform1D& o) const
    {
        const LogRatioTransform1D& r = static_cast<const LogRatioTransform1D&>(o);
        for (unsigned i=0; i<4U; ++i)
            if (!(params_[i] == r.params_[i]))
                return false;
        return true;
    }

    bool LogRatioTransform1D::write(std::ostream& os) const
    {
        gs::write_pod_array(os, params_, 4UL);
        return !os.fail();
    }

    LogRatioTransform1D* LogRatioTransform1D::read(const gs::ClassId& id,
                                             std::istream& is)
    {
        static const gs::ClassId current(gs::ClassId::makeId<LogRatioTransform1D>());
        current.ensureSameId(id);

        double tmp[4];
        gs::read_pod_array(is, tmp, 4UL);
        if (is.fail()) throw gs::IOReadFailure(
            "In npstat::LogRatioTransform1D::read: input stream failure");
        return new LogRatioTransform1D(tmp[0], tmp[1], tmp[2], tmp[3]);
    }
}
