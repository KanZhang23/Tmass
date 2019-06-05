#include <cmath>
#include <cfloat>
#include <stdexcept>

#include "geners/binaryIO.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/UGaussConvolution1D.hh"
#include "npstat/stat/distributionReadError.hh"

namespace npstat {
    UGaussConvolution1D::UGaussConvolution1D(
        const double i_location, const double i_scale,
        const double i_leftEdge, const double i_uniformWidth)
        : AbsScalableDistribution1D(i_location, i_scale),
          leftEdge_(i_leftEdge), width_(i_uniformWidth)
    {
        if (width_ < 0.0) throw std::invalid_argument(
            "In npstat::UGaussConvolution1D constructor: "
            "width of the uniform distribution can not be negative");
        rightEdge_ = leftEdge_ + width_;
    }

    UGaussConvolution1D::UGaussConvolution1D(
        const double i_location, const double i_scale,
        const std::vector<double>& params)
        : AbsScalableDistribution1D(i_location, i_scale),
          leftEdge_(params[0]), width_(params[1])
    {
        if (width_ < 0.0) throw std::invalid_argument(
            "In npstat::UGaussConvolution1D constructor: "
            "width of the uniform distribution can not be negative");
        rightEdge_ = leftEdge_ + width_;
    }

    bool UGaussConvolution1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod(os, leftEdge_);
        gs::write_pod(os, width_);
        return !os.fail();
    }

    UGaussConvolution1D* UGaussConvolution1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<UGaussConvolution1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            double a, b;
            gs::read_pod(in, &a);
            gs::read_pod(in, &b);
            if (!in.fail())
                return new UGaussConvolution1D(location, scale, a, b);
        }
        distributionReadError(in, classname());
        return 0;
    }

    bool UGaussConvolution1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const UGaussConvolution1D& r = 
            static_cast<const UGaussConvolution1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) &&
               leftEdge_ == r.leftEdge_ && width_ == r.width_;
    }

    double UGaussConvolution1D::unscaledDensity(const double x) const
    {
        if (width_ == 0.0)
        {
            Gauss1D g(leftEdge_, 1.0);
            return g.density(x);
        }
        else
        {
            Gauss1D g(0.0, 1.0);
            if (leftEdge_ + width_/2.0 > 0.0)
                return (g.exceedance(x - rightEdge_) - 
                        g.exceedance(x - leftEdge_))/width_;
            else
                return (g.cdf(x - leftEdge_) - g.cdf(x - rightEdge_))/width_;
        }
    }

    double UGaussConvolution1D::unscaledCdf(const double x) const
    {
        if (width_ == 0.0)
        {
            Gauss1D g(leftEdge_, 1.0);
            return g.cdf(x);
        }
        else
        {
            Gauss1D g(0.0, 1.0);
            const double a = leftEdge_;
            const double b = rightEdge_;
            double cd = (g.density(a - x) - g.density(b - x) -
                   (a - x)*g.exceedance(a - x) + (b - x)*g.exceedance(b - x))/width_;
            if (cd < 0.0)
                cd = 0.0;
            if (cd > 1.0)
                cd = 1.0;
            return cd;
        }
    }

    double UGaussConvolution1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::UGaussConvolution1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (width_ == 0.0)
        {
            Gauss1D g(leftEdge_, 1.0);
            return g.quantile(r1);
        }
        else
        {
            Gauss1D g1(leftEdge_, 1.0);
            double qmin = g1.quantile(r1);
            double qmax = qmin + width_;
            const double fmin = cdf(qmin);
            const double fmax = cdf(qmax);
            if (!(fmin < r1 && r1 < fmax))
                throw std::runtime_error(
                    "In npstat::UGaussConvolution1D::unscaledQuantile: "
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
    }

    double UGaussConvolution1D::unscaledExceedance(const double x) const
    {
        if (width_ == 0.0)
        {
            Gauss1D g(leftEdge_, 1.0);
            return g.exceedance(x);
        }
        else
        {
            Gauss1D g(0.0, 1.0);
            const double a = leftEdge_;
            const double b = rightEdge_;
            double exc = (g.density(b - x) - g.density(a - x) -
                   (a - x)*g.cdf(a - x) + (b - x)*g.cdf(b - x))/width_;
            if (exc < 0.0)
                exc = 0.0;
            if (exc > 1.0)
                exc = 1.0;
            return exc;
        }
    }
}
