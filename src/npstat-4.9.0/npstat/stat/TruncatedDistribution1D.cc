#include <stdexcept>

#include "npstat/stat/TruncatedDistribution1D.hh"
#include "npstat/stat/distributionReadError.hh"

#include "geners/binaryIO.hh"
#include "geners/CPP11_auto_ptr.hh"

namespace npstat {
    TruncatedDistribution1D::TruncatedDistribution1D(
        const TruncatedDistribution1D& r)
        : distro_(r.distro_->clone()),
          xmin_(r.xmin_),
          xmax_(r.xmax_),
          cdfmin_(r.cdfmin_),
          cdfmax_(r.cdfmax_),
          exmin_(r.exmin_)
    {
    }

    TruncatedDistribution1D& TruncatedDistribution1D::operator=(
        const TruncatedDistribution1D& r)
    {
        if (&r != this)
        {
            delete distro_;
            distro_ = r.distro_->clone();
            xmin_ = r.xmin_;
            xmax_ = r.xmax_;
            cdfmin_ = r.cdfmin_;
            cdfmax_ = r.cdfmax_;
            exmin_ = r.exmin_;
        }
        return *this;
    }

    TruncatedDistribution1D::~TruncatedDistribution1D()
    {
        delete distro_;
    }

    bool TruncatedDistribution1D::isEqual(const AbsDistribution1D& other) const
    {
        const TruncatedDistribution1D& r = 
            static_cast<const TruncatedDistribution1D&>(other);
        return *distro_ == *r.distro_ && xmin_ == r.xmin_ && xmax_ == r.xmax_ &&
            cdfmin_ == r.cdfmin_ && cdfmax_ == r.cdfmax_ && exmin_ == r.exmin_;
    }

    double TruncatedDistribution1D::density(const double x) const
    {
        if (x < xmin_ || x > xmax_)
            return 0.0;
        else
            return distro_->density(x)/(cdfmax_ - cdfmin_);
    }

    double TruncatedDistribution1D::cdf(const double x) const
    {
        if (x <= xmin_)
            return 0.0;
        else if (x >= xmax_)
            return 1.0;
        else
            return (distro_->cdf(x) - cdfmin_)/(cdfmax_ - cdfmin_);
    }

    double TruncatedDistribution1D::exceedance(const double x) const
    {
        if (x <= xmin_)
            return 1.0;
        else if (x >= xmax_)
            return 0.0;
        else
            return (distro_->exceedance(x) - exmin_)/(cdfmax_ - cdfmin_);
    }

    double TruncatedDistribution1D::quantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::TruncatedDistribution1D::quantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return xmin_;
        else if (r1 == 1.0)
            return xmax_;
        else
            return distro_->quantile(r1*(cdfmax_ - cdfmin_) + cdfmin_);
    }

    TruncatedDistribution1D::TruncatedDistribution1D()
        : distro_(0), xmin_(0.0), xmax_(0.0),
          cdfmin_(0.0), cdfmax_(0.0), exmin_(0.0)
    {
    }

    TruncatedDistribution1D::TruncatedDistribution1D(
        const AbsDistribution1D& distro, double xmin, double xmax,
        const bool minAndMaxAreForCDF)
        : distro_(distro.clone())
    {
        if (xmin > xmax)
            std::swap(xmin, xmax);
        if (xmin == xmax)
            throw std::invalid_argument(
                "In npstat::TruncatedDistribution1D constructor: "
                "can not use the same value for the support limits");
        if (minAndMaxAreForCDF)
        {
            cdfmin_ = xmin;
            cdfmax_ = xmax;
            xmin_ = distro_->quantile(xmin);
            xmax_ = distro_->quantile(xmax);
        }
        else
        {
            xmin_ = xmin;
            xmax_ = xmax;
            cdfmin_ = distro_->cdf(xmin_);
            cdfmax_ = distro_->cdf(xmax_);
        }
        if (cdfmax_ == cdfmin_)
            throw std::invalid_argument(
                "In npstat::TruncatedDistribution1D constructor: "
                "can not use the same value for the minimum and maximum cdf");
        exmin_ = distro_->exceedance(xmax_);
    }

    bool TruncatedDistribution1D::write(std::ostream& os) const
    {
        const bool status = distro_->classId().write(os) && distro_->write(os);
        if (status)
        {
            gs::write_pod(os, xmin_);
            gs::write_pod(os, xmax_);
            gs::write_pod(os, cdfmin_);
            gs::write_pod(os, cdfmax_);
            gs::write_pod(os, exmin_);
        }
        return status && !os.fail();
    }

    TruncatedDistribution1D* TruncatedDistribution1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<TruncatedDistribution1D>());
        current.ensureSameId(id);

        gs::ClassId clid(in, 1);
        CPP11_auto_ptr<AbsDistribution1D> d(AbsDistribution1D::read(clid, in));
        TruncatedDistribution1D* distro = new TruncatedDistribution1D();

        gs::read_pod(in, &distro->xmin_);
        gs::read_pod(in, &distro->xmax_);
        gs::read_pod(in, &distro->cdfmin_);
        gs::read_pod(in, &distro->cdfmax_);
        gs::read_pod(in, &distro->exmin_);

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
