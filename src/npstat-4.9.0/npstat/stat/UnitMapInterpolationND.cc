#include <cmath>
#include <cfloat>
#include <stdexcept>

#include "npstat/stat/UnitMapInterpolationND.hh"

namespace npstat {
    UnitMapInterpolationND::UnitMapInterpolationND(
        const unsigned dim, const unsigned nInterpolated)
        :  AbsInterpolationAlgoND(dim),
           wsum_(0.0),
           nInterpolated_(nInterpolated),
           autoNormalize_(true)
    {
        distros_.reserve(nInterpolated);
        work_.resize(dim);
        point_.resize(dim);
    }

    void UnitMapInterpolationND::normalizeAutomatically(const bool allow)
    {
        autoNormalize_ = allow;
        if (allow && distros_.size() >= nInterpolated_)
            normalize();
    }

    void UnitMapInterpolationND::normalize()
    {
        if (autoNormalize_)
        {
            wsum_ = 0.0;
            const unsigned count = distros_.size();
            for (unsigned i=0; i<count; ++i)
                wsum_ += distros_[i].w;
            if (wsum_ <= 0.0) throw std::invalid_argument(
                "In npstat::UnitMapInterpolationND::normalize: "
                "sum of weights is not positive");
        }
    }

    void UnitMapInterpolationND::add(
        const AbsDistributionND& d, const double weight)
    {
        if (d.dim() != dim_) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::add: "
            "incompatible distribution dimensionality");
        if (!d.mappedByQuantiles()) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::add: "
            "can not use this distribution because it is not "
            "mapped to the unit cube with conditional quantiles");
        distros_.push_back(WeightedND(d, weight));
        if (distros_.size() >= nInterpolated_)
            normalize();
    }

    void UnitMapInterpolationND::replace(
        const unsigned i, const AbsDistributionND& d, const double weight)
    {
        if (d.dim() != dim_) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::replace: "
            "incompatible distribution dimensionality");
        if (!d.mappedByQuantiles()) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::replace: "
            "can not use this distribution because it is not "
            "mapped to the unit cube with conditional quantiles");
        distros_.at(i) = WeightedND(d, weight);
        if (distros_.size() >= nInterpolated_)
            normalize();
    }

    void UnitMapInterpolationND::setWeight(
        const unsigned i, const double weight)
    {
        distros_.at(i).w = weight;
        if (distros_.size() >= nInterpolated_)
            normalize();
    }
    
    void UnitMapInterpolationND::clear()
    {
        distros_.clear();
        wsum_ = 0.0;
    }

    double UnitMapInterpolationND::density(const double* xin,
                                           const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::density: "
            "incompatible input point dimensionality");
        if (!autoNormalize_) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::density: "
            "the overall weighted distribution is not normalized");
        const unsigned mysize = distros_.size();
        if (!(mysize && mysize >= nInterpolated_)) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::density: "
            "insufficient number of distribution added");
        double* w = &work_[0];
        double* p = &point_[0];
        const WeightedND* ptr = &distros_[0];
        double dens = 1.0;

        for (unsigned idim=0; idim<dim_; ++idim)
        {
            const unsigned buflen = idim + 1;
            const double x = xin[idim];

            // Figure out the range on which the distribution is supported
            double xmin = 0.0, xmax = 0.0;
            for (unsigned id=0; id<mysize; ++id)
            {
                p[idim] = 0.0;
                ptr[id].d->unitMap(p, buflen, w);
                xmin += w[idim]*ptr[id].w;
                p[idim] = 1.0;
                ptr[id].d->unitMap(p, buflen, w);
                xmax += w[idim]*ptr[id].w;
            }
            xmin /= wsum_;
            xmax /= wsum_;
            if (x <= xmin || x >= xmax)
                return 0.0;

            // Find the actual quantile
            const double eps = 2.0*DBL_EPSILON;
            double yhi = 1.0;
            double ylo = 0.0;
            while ((yhi - ylo)/(yhi + ylo + DBL_EPSILON) > eps)
            {
                p[idim] = (yhi + ylo)/2.0;
                double q = 0.0;
                for (unsigned id=0; id<mysize; ++id)
                {
                    ptr[id].d->unitMap(p, buflen, w);
                    q += w[idim]*ptr[id].w;
                }
                q /= wsum_;
                if (q >= x)
                    yhi = p[idim];
                else
                    ylo = p[idim];
            }
            const double y = (yhi + ylo)/2.0;

            // Figure out a reasonable step size
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

            p[idim] = yp;
            yhi = 0.0;
            for (unsigned id=0; id<mysize; ++id)
            {
                ptr[id].d->unitMap(p, buflen, w);
                yhi += w[idim]*ptr[id].w;
            }

            p[idim] = ym;
            ylo = 0.0;
            for (unsigned id=0; id<mysize; ++id)
            {
                ptr[id].d->unitMap(p, buflen, w);
                ylo += w[idim]*ptr[id].w;
            }

            const double invd = (yhi - ylo)/wsum_/(yp - ym);
            if (invd <= 0.0) throw std::runtime_error(
                "In npstat::UnitMapInterpolationND::density: "
                "inverse probability density is zero");
            dens /= invd;
            p[idim] = y;
        }

        return dens;
    }

    void UnitMapInterpolationND::unitMap(
        const double* rnd, const unsigned dim, double* x) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::unitMap: "
            "incompatible dimensionality of the random vector");
        if (!autoNormalize_) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::unitMap: "
            "the overall weighted distribution is not normalized");
        const unsigned mysize = distros_.size();
        if (!(mysize && mysize >= nInterpolated_)) throw std::invalid_argument(
            "In npstat::UnitMapInterpolationND::unitMap: "
            "insufficient number of distribution added");
        const WeightedND* ptr = &distros_[0];
        double* w = &work_[0];

        for (unsigned i=0; i<dim; ++i)
            x[i] = 0.0;
        for (unsigned id=0; id<mysize; ++id)
        {
            ptr[id].d->unitMap(rnd, dim, w);
            for (unsigned i=0; i<dim; ++i)
                x[i] += w[i]*ptr[id].w;
        }
        for (unsigned i=0; i<dim; ++i)
            x[i] /= wsum_;
    }

    bool UnitMapInterpolationND::isEqual(const AbsDistributionND& other) const
    {
        const UnitMapInterpolationND& r = 
            static_cast<const UnitMapInterpolationND&>(other);
        return dim_ == r.dim_ &&
               nInterpolated_ == r.nInterpolated_ &&
               distros_ == r.distros_;
    }
}
