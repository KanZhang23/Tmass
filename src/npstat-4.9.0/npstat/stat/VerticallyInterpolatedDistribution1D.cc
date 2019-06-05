#include <cassert>
#include <stdexcept>
#include <cfloat>
#include <cmath>

#include "npstat/stat/VerticallyInterpolatedDistribution1D.hh"

namespace npstat {
    VerticallyInterpolatedDistribution1D::VerticallyInterpolatedDistribution1D()
        : nExpected_(0), count_(0),
          wsum_(0.0), xmin_(DBL_MAX), xmax_(-DBL_MAX),
          autoNormalize_(true)
    {
    }

    VerticallyInterpolatedDistribution1D::VerticallyInterpolatedDistribution1D(
        const unsigned expectedNDistros)
        : nExpected_(expectedNDistros), count_(0),
          wsum_(0.0), xmin_(0.0), xmax_(0.0),
          autoNormalize_(true)
    {
        wdmem_.reserve(nExpected_);
    }

    void VerticallyInterpolatedDistribution1D::clear()
    {
        wdmem_.clear();
        wdmem_.reserve(nExpected_);
        count_ = 0;
        wsum_ = 0.0;
        xmin_ = DBL_MAX;
        xmax_ = -DBL_MAX;
    }

    void VerticallyInterpolatedDistribution1D::add(
        const AbsDistribution1D& d, const double weight)
    {
        wdmem_.push_back(Private::WeightedDistro1DPtr(d, weight));
        if (++count_ >= nExpected_)
            normalize();
    }

    void VerticallyInterpolatedDistribution1D::replace(
        const unsigned i, const AbsDistribution1D& d, const double weight)
    {
        wdmem_.at(i) = Private::WeightedDistro1DPtr(d, weight);
        if (count_ >= nExpected_)
            normalize();
    }

    void VerticallyInterpolatedDistribution1D::setWeight(
        const unsigned i, const double weight)
    {
        wdmem_.at(i).w = weight;
        if (count_ >= nExpected_)
            normalize();
    }

    void VerticallyInterpolatedDistribution1D::normalizeAutomatically(const bool allow)
    {
        autoNormalize_ = allow;
        if (allow && count_ >= nExpected_)
            normalize();
    }

    void VerticallyInterpolatedDistribution1D::normalize()
    {
        if (autoNormalize_)
        {
            const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
            wsum_ = 0.0;
            xmin_ = DBL_MAX;
            xmax_ = -DBL_MAX;
            for (unsigned i=0; i<count_; ++i)
            {
                const double w = wd[i].w;
                if (w)
                {
                    wsum_ += w;
                    const double q0 = wd[i].d->quantile(0.0);
                    if (q0 < xmin_)
                        xmin_ = q0;
                    const double q1 = wd[i].d->quantile(1.0);
                    if (q1 > xmax_)
                        xmax_ = q1;
                }
            }
            if (wsum_ <= 0.0) throw std::invalid_argument(
                "In npstat::VerticallyInterpolatedDistribution1D::normalize: "
                "sum of weights is not positive");
        }
    }

    double VerticallyInterpolatedDistribution1D::density(const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::density: "
            "normalization must be enabled");
        if (!(count_ && count_ >= nExpected_)) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::density: "
            "not enough distributions added");
        if (x < xmin_ || x > xmax_)
            return 0.0;
        double dsum = 0.0;
        const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
        for (unsigned i=0; i<count_; ++i)
        {
            const double w = wd[i].w;
            if (w)
                dsum += w*wd[i].d->density(x);
        }
        return dsum/wsum_;
    }

    double VerticallyInterpolatedDistribution1D::cdf(const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::cdf: "
            "normalization must be enabled");
        if (!(count_ && count_ >= nExpected_)) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::cdf: "
            "not enough distributions added");
        if (x <= xmin_)
            return 0.0;
        else if (x >= xmax_)
            return 1.0;
        else
        {
            double dsum = 0.0;
            const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
            for (unsigned i=0; i<count_; ++i)
            {
                const double w = wd[i].w;
                if (w)
                    dsum += w*wd[i].d->cdf(x);
            }
            return dsum/wsum_;
        }
    }

    double VerticallyInterpolatedDistribution1D::exceedance(const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::exceedance: "
            "normalization must be enabled");
        if (!(count_ && count_ >= nExpected_)) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::exceedance: "
            "not enough distributions added");
        if (x <= xmin_)
            return 1.0;
        else if (x >= xmax_)
            return 0.0;
        else
        {
            double dsum = 0.0;
            const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
            for (unsigned i=0; i<count_; ++i)
            {
                const double w = wd[i].w;
                if (w)
                    dsum += w*wd[i].d->exceedance(x);
            }
            return dsum/wsum_;
        }
    }

    double VerticallyInterpolatedDistribution1D::quantile(const double r1) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::quantile: "
            "normalization must be enabled");
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::VerticallyInterpolatedDistribution1D::quantile: "
            "cdf argument outside of [0, 1] interval");
        double qmax = -DBL_MAX;
        double qmin = DBL_MAX;
        const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
        for (unsigned i=0; i<count_; ++i)
        {
            const double w = wd[i].w;
            if (w)
            {
                const double q = wd[i].d->quantile(r1);
                if (q > qmax)
                    qmax = q;
                if (q < qmin)
                    qmin = q;
            }
        }
        if (qmax == qmin)
            return qmin;
        if (r1 == 1.0)
            return qmax;
        if (r1 == 0.0)
            return qmin;
        const double fmin = cdf(qmin);
        const double fmax = cdf(qmax);
        if (!(fmin < r1 && r1 < fmax)) throw std::runtime_error(
            "In npstat::VerticallyInterpolatedDistribution1D::quantile: "
            "algorithm precondition error");
        for (unsigned i=0; i<1000; ++i)
        {
            const double x = (qmin + qmax)/2.0;
            if (fabs(qmax - qmin)/std::max(fabs(qmin), fabs(qmax)) <
                2.0*DBL_EPSILON)
                return x;
            const double fval = cdf(x);
            if (fval == r1)
                return x;
            else if (fval > r1)
                qmax = x;
            else
                qmin = x;
            if (qmax == qmin)
                return qmin;
        }
        return (qmin + qmax)/2.0;
    }

    bool VerticallyInterpolatedDistribution1D::isEqual(
        const AbsDistribution1D& otherBase) const
    {
        const VerticallyInterpolatedDistribution1D& r = 
            static_cast<const VerticallyInterpolatedDistribution1D&>(otherBase);
        return count_ == r.count_ && wdmem_ == r.wdmem_ && 
               nExpected_ == r.nExpected_;
    }
}
