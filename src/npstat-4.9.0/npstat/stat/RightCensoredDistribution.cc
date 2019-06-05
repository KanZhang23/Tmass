#include <stdexcept>

#include "npstat/stat/RightCensoredDistribution.hh"
#include "npstat/stat/distributionReadError.hh"

#include "geners/binaryIO.hh"
#include "geners/CPP11_auto_ptr.hh"

namespace npstat {
    RightCensoredDistribution::RightCensoredDistribution(
        const RightCensoredDistribution& r)
        : distro_(r.distro_->clone()),
          frac_(r.frac_),
          infty_(r.infty_)
    {
    }

    RightCensoredDistribution& RightCensoredDistribution::operator=(
        const RightCensoredDistribution& r)
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

    RightCensoredDistribution::~RightCensoredDistribution()
    {
        delete distro_;
    }

    bool RightCensoredDistribution::isEqual(const AbsDistribution1D& other) const
    {
        const RightCensoredDistribution& r = 
            static_cast<const RightCensoredDistribution&>(other);
        return *distro_ == *r.distro_ && frac_ == r.frac_ && infty_ == r.infty_;
    }

    double RightCensoredDistribution::density(const double x) const
    {
        return frac_*distro_->density(x);
    }

    double RightCensoredDistribution::cdf(const double x) const
    {
        if (x > infty_)
            return 1.0;
        else
            return frac_*distro_->cdf(x);
    }

    double RightCensoredDistribution::exceedance(const double x) const
    {
        return 1.0 - cdf(x);
    }

    double RightCensoredDistribution::quantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::RightCensoredDistribution::quantile: "
            "cdf argument outside of [0, 1] interval");
        if (frac_ == 0.0)
            return infty_;
        else if (r1 <= frac_)
            return distro_->quantile(r1/frac_);
        else
            return infty_;
    }

    RightCensoredDistribution::RightCensoredDistribution()
        : distro_(0), frac_(0.0), infty_(0.0)
    {
    }

    RightCensoredDistribution::RightCensoredDistribution(
        const AbsDistribution1D& distro,
        const double visible,
        const double infty)
        : distro_(distro.clone()),
          frac_(visible),
          infty_(infty)
    {
        if (frac_ < 0.0 || frac_ > 1.0)
            throw std::invalid_argument(
                "In npstat::RightCensoredDistribution constructor: "
                "visible fraction outside of [0, 1] interval");
        if (infty_ < distro_->quantile(1.0))
            throw std::invalid_argument(
                "In npstat::RightCensoredDistribution constructor: "
                "effective infinity is not large enough");
    }

    bool RightCensoredDistribution::write(std::ostream& os) const
    {
        const bool status = distro_->classId().write(os) && distro_->write(os);
        if (status)
        {
            gs::write_pod(os, frac_);
            gs::write_pod(os, infty_);
        }
        return status && !os.fail();
    }

    RightCensoredDistribution* RightCensoredDistribution::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<RightCensoredDistribution>());
        current.ensureSameId(id);

        gs::ClassId clid(in, 1);
        CPP11_auto_ptr<AbsDistribution1D> d(AbsDistribution1D::read(clid, in));
        RightCensoredDistribution* distro = new RightCensoredDistribution();

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
