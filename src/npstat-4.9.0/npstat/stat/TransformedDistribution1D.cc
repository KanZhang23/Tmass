#include <cmath>

#include "npstat/stat/TransformedDistribution1D.hh"
#include "npstat/stat/distributionReadError.hh"

#include "geners/binaryIO.hh"

namespace npstat {
    TransformedDistribution1D::TransformedDistribution1D() : t_(0), d_(0) {}

    TransformedDistribution1D::TransformedDistribution1D(
        const AbsDistributionTransform1D& transform,
        const AbsDistribution1D& distro)
        : t_(transform.clone()), d_(distro.clone())
    {
    }

    TransformedDistribution1D::TransformedDistribution1D(
        const TransformedDistribution1D& r)
        : t_(r.t_->clone()), d_(r.d_->clone())
    {
    }

    void TransformedDistribution1D::cleanup()
    {
        delete d_; d_ = 0;
        delete t_; t_ = 0;
    }

    TransformedDistribution1D& TransformedDistribution1D::operator=(
        const TransformedDistribution1D& r)
    {
        if (this != &r)
        {
            AbsDistribution1D::operator=(r);
            cleanup();
            t_ = r.t_->clone();
            d_ = r.d_->clone();
        }
        return *this;
    }

    bool TransformedDistribution1D::isEqual(const AbsDistribution1D& o) const
    {
        const TransformedDistribution1D& r = 
            static_cast<const TransformedDistribution1D&>(o);
        return *t_ == *r.t_ && *d_ == *r.d_;
    }

    bool TransformedDistribution1D::write(std::ostream& os) const
    {
        return gs::write_obj(os, *t_) && gs::write_obj(os, *d_);
    }

    TransformedDistribution1D* TransformedDistribution1D::read(
        const gs::ClassId& id, std::istream& is)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<TransformedDistribution1D>());
        myClassId.ensureSameId(id);

        CPP11_auto_ptr<AbsDistributionTransform1D> t =
            gs::read_obj<AbsDistributionTransform1D>(is);
        CPP11_auto_ptr<AbsDistribution1D> d =
            gs::read_obj<AbsDistribution1D>(is);
        if (!is.fail())
        {
            TransformedDistribution1D* tmp = new TransformedDistribution1D();
            tmp->t_ = t.release();
            tmp->d_ = d.release();
            return tmp;
        }
        distributionReadError(is, classname());
        return 0;
    }

    double TransformedDistribution1D::density(const double x) const
    {
        double deriv = 0.0;
        const double y = t_->transformForward(x, &deriv);
        return d_->density(y)*fabs(deriv);
    }

    double TransformedDistribution1D::cdf(const double x) const
    {
        const double y = t_->transformForward(x, 0);
        if (t_->isIncreasing())
            return d_->cdf(y);
        else
            return d_->exceedance(y);
    }

    double TransformedDistribution1D::exceedance(const double x) const
    {
        const double y = t_->transformForward(x, 0);
        if (t_->isIncreasing())
            return d_->exceedance(y);
        else
            return d_->cdf(y);
    }

    double TransformedDistribution1D::quantile(const double r1) const
    {
        const double r = t_->isIncreasing() ? r1 : 1.0 - r1;
        const double y = d_->quantile(r);
        return t_->transformBack(y);
    }
}
