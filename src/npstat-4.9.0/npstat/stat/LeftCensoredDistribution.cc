#include <stdexcept>

#include "npstat/stat/LeftCensoredDistribution.hh"
#include "npstat/stat/distributionReadError.hh"

#include "geners/binaryIO.hh"
#include "geners/CPP11_auto_ptr.hh"

namespace npstat {
    LeftCensoredDistribution::LeftCensoredDistribution(
        const LeftCensoredDistribution& r)
        : distro_(r.distro_->clone()),
          frac_(r.frac_),
          infty_(r.infty_)
    {
    }

    LeftCensoredDistribution& LeftCensoredDistribution::operator=(
        const LeftCensoredDistribution& r)
    {
        if (&r != this)
        {
            delete distro_;
            distro_ = r.distro_->clone();
            frac_ = r.frac_;
            infty_ = r.infty_;
        }
        return *this;
    }

    LeftCensoredDistribution::~LeftCensoredDistribution()
    {
        delete distro_;
    }

    bool LeftCensoredDistribution::isEqual(const AbsDistribution1D& other) const
    {
        const LeftCensoredDistribution& r = 
            static_cast<const LeftCensoredDistribution&>(other);
        return *distro_ == *r.distro_ && frac_ == r.frac_ && infty_ == r.infty_;
    }

    double LeftCensoredDistribution::density(const double x) const
    {
        return frac_*distro_->density(x);
    }

    double LeftCensoredDistribution::cdf(const double x) const
    {
        if (x < infty_)
            return 0.0;
        else
            return 1.0 - frac_ + frac_*distro_->cdf(x);
    }

    double LeftCensoredDistribution::exceedance(const double x) const
    {
        if (x < infty_)
            return 1.0;
        else
            return frac_*distro_->exceedance(x);
    }

    double LeftCensoredDistribution::quantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::LeftCensoredDistribution::quantile: "
            "cdf argument outside of [0, 1] interval");
        if (frac_ == 0.0)
            return infty_;
        const volatile double oneMinusFrac = 1.0 - frac_;
        if (r1 < oneMinusFrac)
            return infty_;
        else
            return distro_->quantile((r1 - oneMinusFrac)/frac_);
    }

    LeftCensoredDistribution::LeftCensoredDistribution()
        : distro_(0), frac_(0.0), infty_(0.0)
    {
    }

    LeftCensoredDistribution::LeftCensoredDistribution(
        const AbsDistribution1D& distro,
        const double visible,
        const double infty)
        : distro_(distro.clone()),
          frac_(visible),
          infty_(infty)
    {
        if (frac_ < 0.0 || frac_ > 1.0)
            throw std::invalid_argument(
                "In npstat::LeftCensoredDistribution constructor: "
                "visible fraction outside of [0, 1] interval");
        if (infty_ > distro_->quantile(0.0))
            throw std::invalid_argument(
                "In npstat::LeftCensoredDistribution constructor: "
                "effective infinity is not negative enough");
    }

    bool LeftCensoredDistribution::write(std::ostream& os) const
    {
        const bool status = distro_->classId().write(os) && distro_->write(os);
        if (status)
        {
            gs::write_pod(os, frac_);
            gs::write_pod(os, infty_);
        }
        return status && !os.fail();
    }

    LeftCensoredDistribution* LeftCensoredDistribution::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<LeftCensoredDistribution>());
        current.ensureSameId(id);

        gs::ClassId clid(in, 1);
        CPP11_auto_ptr<AbsDistribution1D> d(AbsDistribution1D::read(clid, in));
        LeftCensoredDistribution* distro = new LeftCensoredDistribution();

        gs::read_pod(in, &distro->frac_);
        gs::read_pod(in, &distro->infty_);

        if (in.fail())
        {
            delete distro;
            distro = 0;
            distributionReadError(in, classname());
        }
        else
            distro->distro_ = d.release();
        return distro;
    }
}
