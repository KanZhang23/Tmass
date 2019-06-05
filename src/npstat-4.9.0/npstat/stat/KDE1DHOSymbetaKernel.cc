#include <cassert>
#include <stdexcept>

#include "npstat/nm/ClassicalOrthoPolys1D.hh"

#include "npstat/stat/KDE1DHOSymbetaKernel.hh"
#include "npstat/stat/Distributions1D.hh"

namespace npstat {
    KDE1DHOSymbetaKernel::KDE1DHOSymbetaKernel(
        const int i_power, const double i_filterDegree)
        : power_(i_power), filterDegree_(i_filterDegree), lastShrinkage_(1.0)
    {
        if (filterDegree_ < 0.0) throw std::invalid_argument(
            "In npstat::KDE1DHOSymbetaKernel constructor: filterDegree"
            " argument can not be negative");
        if (i_power < 0)
            poly_ = new HermiteProbOrthoPoly1D();
        else if (i_power == 0)
            poly_ = new LegendreOrthoPoly1D();
        else
            poly_ = new JacobiOrthoPoly1D(i_power, i_power);

        maxdeg_ = floor(filterDegree_);
        if (static_cast<double>(maxdeg_) != filterDegree_)
        {
            lastShrinkage_ = sqrt(filterDegree_ - maxdeg_);
            ++maxdeg_;
        }
        polyValues_.resize(2U*(maxdeg_ + 1U));
        poly_->allpoly(0.0L, &polyValues_[0], maxdeg_);

        if (i_power < 0)
        {
            Gauss1D g(0.0, 1.0);
            xmin_ = g.quantile(0.0);
            xmax_ = g.quantile(1.0);
        }
        else
        {
            xmin_ = poly_->xmin();
            xmax_ = poly_->xmax();
        }
    }

    KDE1DHOSymbetaKernel::KDE1DHOSymbetaKernel(const KDE1DHOSymbetaKernel& r)
        : poly_(0)
    {
        copyInternals(r);
    }

    void KDE1DHOSymbetaKernel::copyInternals(const KDE1DHOSymbetaKernel& r)
    {
        assert(!poly_);
        poly_ = r.poly_->clone();
        power_ = r.power_;
        maxdeg_ = r.maxdeg_;
        filterDegree_ = r.filterDegree_;
        xmin_ = r.xmin_;
        xmax_ = r.xmax_;
        lastShrinkage_ = r.lastShrinkage_;
        polyValues_ = r.polyValues_;
    }

    KDE1DHOSymbetaKernel& KDE1DHOSymbetaKernel::operator=(
        const KDE1DHOSymbetaKernel& r)
    {
        if (this != &r)
        {
            delete poly_;
            poly_ = 0;
            copyInternals(r);
        }
        return *this;
    }

    double KDE1DHOSymbetaKernel::operator()(const double x) const
    {
        long double sum = 1.0L;
        if (filterDegree_)
        {
            const long double* poly0 = &polyValues_[0];
            long double* polyx = &polyValues_[0] + (maxdeg_ + 1U);
            poly_->allpoly(x, polyx, maxdeg_);
            for (unsigned i=1U; i<maxdeg_; ++i)
                sum += poly0[i]*polyx[i];
            sum += poly0[maxdeg_]*polyx[maxdeg_]*lastShrinkage_;
        }
        return sum*poly_->weight(x);
    }
}
