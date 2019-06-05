#include <cmath>
#include <cassert>
#include <stdexcept>

#include "npstat/stat/CompositeDistributionND.hh"
#include "npstat/stat/CopulaInterpolationND.hh"
#include "npstat/stat/StatUtils.hh"

namespace npstat {
    CopulaInterpolationND::CopulaInterpolationND(
        const unsigned dim, const unsigned nInterpolated)
        : AbsInterpolationAlgoND(dim),
          wsum_(0.0),
          nInterpolated_(nInterpolated),
          autoNormalize_(true)
    {
        if (dim <= 1U) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND constructor: "
            "distribution dimensionality must be at least 2");
        marginInterpolators_.reserve(dim);
        InterpolatedDistribution1D i1d(nInterpolated_);
        for (unsigned i=0; i<dim; ++i)
            marginInterpolators_.push_back(i1d);
        copulas_.reserve(nInterpolated_);
        work_.resize(dim);
    }

    void CopulaInterpolationND::normalizeAutomatically(const bool allow)
    {
        for (unsigned i=0; i<dim_; ++i)
            marginInterpolators_[i].normalizeAutomatically(allow);
        autoNormalize_ = allow;
        if (allow && copulas_.size() >= nInterpolated_)
            normalize();
    }

    void CopulaInterpolationND::normalize()
    {
        if (autoNormalize_)
        {
            const unsigned count = copulas_.size();
            wCdf_.resize(count + 1);
            wsum_ = 0.0;
            for (unsigned i=0; i<count; ++i)
            {
                wCdf_[i] = wsum_;
                // Can't generate random numbers if the weights
                // are negative
                if (copulas_[i].w < 0.0) throw std::domain_error(
                    "In npstat::CopulaInterpolationND::normalize: "
                    "negative weights are not allowed");
                wsum_ += copulas_[i].w;
            }
            if (!wsum_) throw std::domain_error(
                "In npstat::CopulaInterpolationND::normalize: "
                "sum of all weights is zero");
            for (unsigned i=1; i<count; ++i)
                wCdf_[i] /= wsum_;
            wCdf_[count] = 1.0;
        }
    }

    void CopulaInterpolationND::add(
        const AbsDistributionND& dIn, const double weight)
    {
        const CompositeDistributionND& d(
            dynamic_cast<const CompositeDistributionND&>(dIn));
        if (d.dim() != dim_) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND::add: "
            "incompatible distribution dimensionality");
        copulas_.push_back(WeightedND(d.copula(), weight));
        for (unsigned i=0; i<dim_; ++i)
            marginInterpolators_[i].add(*d.marginal(i), weight);
        if (copulas_.size() >= nInterpolated_)
            normalize();
    }

    void CopulaInterpolationND::replace(
        const unsigned i, const AbsDistributionND& dIn, const double weight)
    {
        const CompositeDistributionND& d(
            dynamic_cast<const CompositeDistributionND&>(dIn));
        if (d.dim() != dim_) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND::replace: "
            "incompatible distribution dimensionality");
        copulas_.at(i) = WeightedND(d.copula(), weight);
        for (unsigned j=0; j<dim_; ++j)
            marginInterpolators_[j].replace(i, *d.marginal(j), weight);
        if (copulas_.size() >= nInterpolated_)
            normalize();
    }

    void CopulaInterpolationND::setWeight(
        const unsigned i, const double weight)
    {
        copulas_.at(i).w = weight;
        for (unsigned j=0; j<dim_; ++j)
            marginInterpolators_[j].setWeight(i, weight);
        if (copulas_.size() >= nInterpolated_)
            normalize();
    }

    void CopulaInterpolationND::clear()
    {
        copulas_.clear();
        copulas_.reserve(nInterpolated_);
        for (unsigned i=0; i<dim_; ++i)
            marginInterpolators_[i].clear();
        wsum_ = 0.0;
    }

    double CopulaInterpolationND::copulaDensity(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND::copulaDensity: "
            "incompatible dimensionality of the input point");
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::copulaDensity: "
            "normalization must be enabled");
        assert(x);

        double* w = &work_[0];
        for (unsigned i=0; i<dim_; ++i)
            w[i] = marginInterpolators_[i].cdf(x[i]);
        double copsum = 0.0;
        const unsigned count = copulas_.size();
        for (unsigned i=0; i<count; ++i)
            copsum += copulas_[i].w*copulas_[i].d->density(w, dim);
        copsum /= wsum_;
        return copsum;
    }

    double CopulaInterpolationND::productOfTheMarginals(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND::productOfTheMarginals: "
            "incompatible dimensionality of the input point");
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::productOfTheMarginals: "
            "normalization must be enabled");
        assert(x);

        double marginProd = 1.0;
        for (unsigned i=0; i<dim_; ++i)
            marginProd *= marginInterpolators_[i].density(x[i]);
        return marginProd;
    }

    double CopulaInterpolationND::marginalDensity(
        const unsigned idim, const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::marginalDensity: "
            "normalization must be enabled");
        return marginInterpolators_.at(idim).density(x);
    }

    double CopulaInterpolationND::marginalCdf(
        const unsigned idim, const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::marginalCdf: "
            "normalization must be enabled");
        return marginInterpolators_.at(idim).cdf(x);
    }

    double CopulaInterpolationND::marginalExceedance(
        const unsigned idim, const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::marginalExceedance: "
            "normalization must be enabled");
        return marginInterpolators_.at(idim).exceedance(x);
    }

    double CopulaInterpolationND::marginalQuantile(
        const unsigned idim, const double x) const
    {
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::marginalQuantile: "
            "normalization must be enabled");
        return marginInterpolators_.at(idim).quantile(x);
    }

    double CopulaInterpolationND::density(
        const double* x, const unsigned dim) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND::density: "
            "incompatible dimensionality of the input point");
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::density: "
            "normalization must be enabled");
        assert(x);

        double copsum = 0.0, marginProd = 1.0;
        double* w = &work_[0];
        for (unsigned i=0; i<dim_; ++i)
        {
            marginInterpolators_[i].densityAndCdf(x[i], &copsum, w+i);
            marginProd *= copsum;
        }
        copsum = 0.0;
        const unsigned count = copulas_.size();
        for (unsigned i=0; i<count; ++i)
            copsum += copulas_[i].w*copulas_[i].d->density(w, dim);
        copsum /= wsum_;
        return marginProd*copsum;
    }

    // The following function does not create a standard map.
    // However, composition of this map will be appropriate for
    // generating random numbers which are correct on average.
    void CopulaInterpolationND::unitMap(
        const double* rnd, const unsigned dim, double* x) const
    {
        if (dim != dim_) throw std::invalid_argument(
            "In npstat::CopulaInterpolationND::unitMap: "
            "incompatible dimensionality of the random vector");
        if (!autoNormalize_) throw std::runtime_error(
            "In npstat::CopulaInterpolationND::unitMap: "
            "normalization must be enabled");
        assert(rnd);
        assert(x);

        double* w = &work_[0];
        unsigned copulaNum = copulas_.size() - 1U;
        double newRandom = rnd[0];
        if (newRandom < 1.0)
            copulaNum = quantileBinFromCdf(&wCdf_[0], wCdf_.size(),
                                           newRandom, &newRandom);
        w[0] = newRandom;
        for (unsigned i=1; i<dim; ++i)
            w[i] = rnd[i];

        copulas_.at(copulaNum).d->unitMap(w, dim, x);
        for (unsigned i=0; i<dim; ++i)
            x[i] = marginInterpolators_[i].quantile(x[i]);
    }

    bool CopulaInterpolationND::isEqual(const AbsDistributionND& other) const
    {
        const CopulaInterpolationND& r = 
            static_cast<const CopulaInterpolationND&>(other);
        return dim_ == r.dim_ &&
               nInterpolated_ == r.nInterpolated_ &&
               marginInterpolators_ == r.marginInterpolators_ &&
               copulas_ == r.copulas_;
    }
}
