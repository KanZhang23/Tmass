#include <cmath>

#include "npstat/stat/HistoNDCdf.hh"
#include "npstat/nm/findRootInLogSpace.hh"

namespace {
    class HistoCdfFcn : public npstat::Functor1<double,double>
    {
    public:
        inline HistoCdfFcn(const npstat::HistoNDCdf& cdf,
                           const npstat::BoxND<double>& bx)
            : cdf_(cdf), iniBox_(bx), scaledBox_(bx) {}

        inline double operator()(const double& scale) const
        {
            scaledBox_ = iniBox_;
            scaledBox_.expand(scale);
            return cdf_.boxCoverage(scaledBox_);
        }

    private:
        const npstat::HistoNDCdf& cdf_;
        npstat::BoxND<double> iniBox_;
        mutable npstat::BoxND<double> scaledBox_;
    };
}


namespace npstat {
    BoxND<double> HistoNDCdf::boundingBox() const
    {
        BoxND<double> box;
        box.reserve(dim_);
        const HistoAxis* ax = &axes_[0];
        for (unsigned i=0; i<dim_; ++i)
            box.push_back(Interval<double>(ax[i].min(), ax[i].max()));
        return box;
    }

    double HistoNDCdf::cdf(const double* x, const unsigned xlen) const
    {
        if (xlen != dim_) throw std::invalid_argument(
            "In npstat::HistoNDCdf::cdf: "
            "incompatible input point dimensionality");
        double* buf = &buf_[0];
        const HistoAxis* ax = &axes_[0];
        for (unsigned i=0; i<dim_; ++i)
            buf[i] = ax[i].fltBinNumber(x[i]);
        return cdf_.interpolate1(buf, dim_);
    }

    double HistoNDCdf::boxCoverage(const BoxND<double>& inbox) const
    {
        if (dim_ != inbox.size()) throw std::invalid_argument(
            "In npstat::HistoNDCdf::boxCoverage: "
            "incompatible input box dimensionality");
        long double sum = 0.0L;
        const unsigned long maxcycle = 1UL << dim_;
        double* buf = &buf_[0];
        const Interval<double>* box = &inbox[0];
        for (unsigned long icycle=0UL; icycle<maxcycle; ++icycle)
        {
            unsigned n1 = 0U;
            for (unsigned i=0; i<dim_; ++i)
            {
                if (icycle & (1UL << i))
                {
                    ++n1;
                    buf[i] = box[i].max();
                }
                else
                    buf[i] = box[i].min();
            }
            const double cdfval = cdf(buf, dim_);
            if ((dim_ - n1) % 2U)
                sum -= cdfval;
            else
                sum += cdfval;
        }
        return sum;
    }

    void HistoNDCdf::coveringBox(
        const double frac, const double* center,
        const unsigned lenCenter, BoxND<double>* outbox) const
    {
        if (lenCenter != dim_) throw std::invalid_argument(
            "In npstat::HistoNDCdf::coveringBox: "
            "incompatible center point dimensionality");
        if (!(frac >= 0.0 && frac <= 1.0)) throw std::domain_error(
            "In npstat::HistoNDCdf::coveringBox: "
            "coverage argument outside of [0, 1] interval");

        assert(outbox);
        *outbox = startBox_;
        outbox->shift(center, dim_);
        double scale = 0.0;

        if (frac > 0.0)
        {
            // Find a reasonable initial scale
            double maxscale = 0.0;
            for (unsigned i=0; i<dim_; ++i)
            {
                const double d1 = fabs(axes_[i].min() - center[i]);
                const double d2 = fabs(axes_[i].max() - center[i]);
                const double dist = d1 > d2 ? d1 : d2;
                const double r = 2.0*dist/startBox_[i].length();
                if (r > maxscale)
                    maxscale = r;
            }

            if (frac == 1.0)
                scale = maxscale;
            else
            {
                HistoCdfFcn fcn(*this, *outbox);
                const double factor = pow(2.0, 0.5/dim_);
                const double lfactor = log(factor);
                maxscale *= 1.0 + 0.5*tol_;
                bool converged = false;
                for (unsigned i=0; i<128U && !converged; ++i)
                {
                    maxscale /= factor;
                    converged = findRootInLogSpace(
                        fcn, frac, maxscale, tol_, &scale, lfactor);
                }
                if (!converged) throw std::runtime_error(
                    "In npstat::HistoNDCdf::coveringBox: could not find "
                    "the box with requested coverage and proportions");
            }
        }
        outbox->expand(scale);
    }
}
