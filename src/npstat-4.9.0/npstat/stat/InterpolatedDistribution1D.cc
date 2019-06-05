#include <cassert>
#include <stdexcept>
#include <cfloat>
#include <cmath>

#include "npstat/stat/InterpolatedDistribution1D.hh"

namespace npstat {
    InterpolatedDistribution1D::InterpolatedDistribution1D()
        : nExpected_(0), count_(0),
          wsum_(0.0), xmin_(0.0), xmax_(0.0),
          autoNormalize_(true)
    {
    }

    InterpolatedDistribution1D::InterpolatedDistribution1D(
        const unsigned expectedNDistros)
        : nExpected_(expectedNDistros), count_(0),
          wsum_(0.0), xmin_(0.0), xmax_(0.0),
          autoNormalize_(true)
    {
        wdmem_.reserve(nExpected_);
    }

    void InterpolatedDistribution1D::clear()
    {
        wdmem_.clear();
        wdmem_.reserve(nExpected_);
        count_ = 0;
        wsum_ = 0.0;
        xmin_ = 0.0;
        xmax_ = 0.0;
    }

    void InterpolatedDistribution1D::add(
        const AbsDistribution1D& d, const double weight)
    {
        wdmem_.push_back(Private::WeightedDistro1DPtr(d, weight));
        if (++count_ >= nExpected_)
            normalize();
    }

    void InterpolatedDistribution1D::replace(
        const unsigned i, const AbsDistribution1D& d, const double weight)
    {
        wdmem_.at(i) = Private::WeightedDistro1DPtr(d, weight);
        if (count_ >= nExpected_)
            normalize();
    }

    void InterpolatedDistribution1D::setWeight(
        const unsigned i, const double weight)
    {
        wdmem_.at(i).w = weight;
        if (count_ >= nExpected_)
            normalize();
    }

    void InterpolatedDistribution1D::normalizeAutomatically(const bool allow)
    {
        autoNormalize_ = allow;
        if (allow && count_ >= nExpected_)
            normalize();
    }

    void InterpolatedDistribution1D::normalize()
    {
        if (autoNormalize_)
        {
            const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
            long double sw = 0.0L, sxmin = 0.0L, sxmax = 0.0L;
            for (unsigned i=0; i<count_; ++i)
            {
                const double weight = wd[i].w;
                sw += weight;
                sxmin += wd[i].d->quantile(0.0)*weight;
                sxmax += wd[i].d->quantile(1.0)*weight;
            }
            wsum_ = sw;
            if (wsum_ <= 0.0) throw std::invalid_argument(
                "In npstat::InterpolatedDistribution1D::normalize: "
                "sum of weights is not positive");
            xmin_ = sxmin/sw;
            xmax_ = sxmax/sw;
        }
    }

    void InterpolatedDistribution1D::densityAndCdf(
        const double x, double* pdensity, double* pcdf) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::InterpolatedDistribution1D::densityAndCdf: "
            "normalization must be enabled");
        assert(pdensity);
        assert(pcdf);

        if (x <= xmin_)
        {
            *pdensity = 0.0;
            *pcdf = 0.0;
            return;
        }

        if (x >= xmax_)
        {
            *pdensity = 0.0;
            *pcdf = 1.0;
            return;
        }

        // Go to the cdf space
        const double y = cdf(x);
        *pcdf = y;

        // Figure out a reasonable step size.
        // It looks like the algorithmic error of
        // calculating the derivative of the quantile
        // function can get quite large. Because of
        // this, we use smaller h than the "normal"
        // for the central derivative formula.
        // double h = pow(DBL_EPSILON, 1.0/3.0);
        double h = 2.0*sqrt(DBL_EPSILON);
        if (y + h >= 1.0)
            h = (1.0 - y)/2.0;
        if (y - h <= 0.0)
            h = y / 2.0;
        if (h == 0.0)
        {
            *pdensity = 0.0;
            return;
        }

        // Calculate the derivative of the quantile function
        const volatile double yp = y + h;
        const volatile double ym = y - h;
        const double dq = (quantile(yp) - quantile(ym))/(yp - ym);
        if (dq > 0.0)
            *pdensity = 1.0/dq;
        else
            *pdensity = DBL_MAX;
    }

    double InterpolatedDistribution1D::density(const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::InterpolatedDistribution1D::density: "
            "normalization must be enabled");
        if (x <= xmin_ || x >= xmax_)
            return 0.0;

        // Go to the cdf space
        const double y = cdf(x);

        // Figure out a reasonable step size.
        // It looks like the algorithmic error of
        // calculating the derivative of the quantile
        // function can get quite large. Because of
        // this, we use smaller h than the "normal"
        // for the central derivative formula.
        // double h = pow(DBL_EPSILON, 1.0/3.0);
        double h = 2.0*sqrt(DBL_EPSILON);
        if (y + h >= 1.0)
            h = (1.0 - y)/2.0;
        if (y - h <= 0.0)
            h = y / 2.0;
        if (h == 0.0)
            return 0.0;

        // Calculate the derivative of the quantile function
        const volatile double yp = y + h;
        const volatile double ym = y - h;
        const double dq = (quantile(yp) - quantile(ym))/(yp - ym);
        if (dq > 0.0)
            return 1.0/dq;
        else
            return DBL_MAX;
    }

    double InterpolatedDistribution1D::cdf(const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::InterpolatedDistribution1D::cdf: "
            "normalization must be enabled");
        if (x <= xmin_)
            return 0.0;
        else if (x >= xmax_)
            return 1.0;
        else
        {
            // Need to return such y that quantile(y) = x
            const double eps = 2.0*DBL_EPSILON;
            double yhi = 1.0;
            double ylo = 0.0;
            while ((yhi - ylo)/(yhi + ylo + DBL_EPSILON) > eps)
            {
                const double half = (yhi + ylo)/2.0;
                if (quantile(half) >= x)
                    yhi = half;
                else
                    ylo = half;
            }
            return (yhi + ylo)/2.0;
        }
    }

    double InterpolatedDistribution1D::exceedance(const double x) const
    {
        return 1.0 - cdf(x);
    }

    double InterpolatedDistribution1D::quantile(const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::InterpolatedDistribution1D::quantile: "
            "normalization must be enabled");
        if (!(count_ && count_ >= nExpected_)) throw std::runtime_error(
            "In npstat::InterpolatedDistribution1D::quantile: "
            "not enough distributions added");
        long double qsum = 0.0L;
        const Private::WeightedDistro1DPtr* wd = &wdmem_[0];
        for (unsigned i=0; i<count_; ++i)
        {
            const double w = wd[i].w;
            if (w)
                qsum += w*wd[i].d->quantile(x);
        }
        return qsum/wsum_;
    }

    bool InterpolatedDistribution1D::isEqual(
        const AbsDistribution1D& otherBase) const
    {
        const InterpolatedDistribution1D& r = 
            static_cast<const InterpolatedDistribution1D&>(otherBase);
        return count_ == r.count_ && wdmem_ == r.wdmem_ && 
               nExpected_ == r.nExpected_;
    }
}
