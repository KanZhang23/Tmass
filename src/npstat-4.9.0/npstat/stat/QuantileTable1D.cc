#include <cfloat>

#include "npstat/nm/interpolate.hh"

#include "geners/binaryIO.hh"

#include "npstat/stat/QuantileTable1D.hh"
#include "npstat/stat/distributionReadError.hh"
#include "npstat/stat/StatUtils.hh"

namespace npstat {
    bool QuantileTable1D::isEqual(const AbsDistribution1D& otherBase) const
    {
        const QuantileTable1D& r = 
            static_cast<const QuantileTable1D&>(otherBase);
        return AbsScalableDistribution1D::isEqual(r) && qtable_ == r.qtable_;
    }

    QuantileTable1D::QuantileTable1D(const double location, const double scale,
                                     const std::vector<double>& params)
        : AbsScalableDistribution1D(location, scale),
          qtable_(params)
    {
        if (params.empty())
            throw std::invalid_argument(
                "In npstat::QuantileTable1D constructor: no data");
        const unsigned long dataLen = qtable_.size();
        if (dataLen > 1U && !isNonDecreasing(qtable_.begin(), qtable_.end()))
            throw std::invalid_argument(
                "In npstat::QuantileTable1D constructor: invalid table");
        for (unsigned long i=0; i<dataLen; ++i)
            if (qtable_[i] < 0.0 || qtable_[i] > 1.0)
                throw std::invalid_argument(
                    "In npstat::QuantileTable1D constructor: out of range table value");
    }

    bool QuantileTable1D::write(std::ostream& os) const
    {
        AbsScalableDistribution1D::write(os);
        gs::write_pod_vector(os, qtable_);
        return !os.fail();
    }

    QuantileTable1D* QuantileTable1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<QuantileTable1D>());
        current.ensureSameId(id);

        double location, scale;
        if (AbsScalableDistribution1D::read(in, &location, &scale))
        {
            std::vector<double> table;
            gs::read_pod_vector(in, &table);
            if (!in.fail() && table.size())
                return new QuantileTable1D(location, scale, table);
        }
        distributionReadError(in, classname());
        return 0;
    }

    double QuantileTable1D::unscaledDensity(const double x) const
    {
        if (x <= 0.0 || x >= 1.0)
            return 0.0;
        else
        {
            const unsigned long sz = qtable_.size();
            const double step = 1.0/sz;
            const double qmin = 0.5*step;
            if (x <= qtable_[0])
                return qmin/qtable_[0];
            else if (x >= qtable_[sz - 1])
                return qmin/(1.0 - qtable_[sz - 1]);
            else
            {
                const unsigned bin = quantileBinFromCdf(&qtable_[0], sz, x);
                assert(bin <= sz - 2);
                const double denom = qtable_[bin+1] - qtable_[bin];
                if (denom > 0.0)
                    return step/denom;
                else
                    return DBL_MAX;
            }
        }
    }

    double QuantileTable1D::unscaledCdf(const double x) const
    {
        if (x <= 0.0)
            return 0.0;
        else if (x >= 1.0)
            return 1.0;
        else
        {
            double res = 0.0;
            const unsigned long sz = qtable_.size();
            const double step = 1.0/sz;
            const double qmin = 0.5*step;
            if (x <= qtable_[0])
                res = x/qtable_[0]*qmin;
            else if (x >= qtable_[sz - 1])
                res = 1.0 + qmin*((x - qtable_[sz - 1])/(1.0 - qtable_[sz - 1]) - 1.0);
            else
            {
                double delta = 0.0;
                const unsigned bin = quantileBinFromCdf(&qtable_[0], sz, x, &delta);
                assert(bin <= sz - 2);
                res = qmin + (bin + delta)*step;
            }
            if (res < 0.0)
                res = 0.0;
            if (res > 1.0)
                res = 1.0;
            return res;
        }
    }

    double QuantileTable1D::unscaledQuantile(const double r1) const
    {
        if (!(r1 >= 0.0 && r1 <= 1.0)) throw std::domain_error(
            "In npstat::QuantileTable1D::unscaledQuantile: "
            "cdf argument outside of [0, 1] interval");
        if (r1 == 0.0)
            return 0.0;
        else if (r1 == 1.0)
            return 1.0;
        else
        {
            double res = 0.0;
            const unsigned long sz = qtable_.size();
            const double step = 1.0/sz;
            const double qmin = 0.5*step;
            const double qmax = 1.0 - qmin;
            if (r1 <= qmin)
                res = qtable_[0]*r1/qmin;
            else if (r1 >= qmax || sz == 1UL)
                res = 1.0 - (1.0 - qtable_[sz - 1])*(1.0 - r1)/qmin;
            else
            {
                unsigned long bin = static_cast<unsigned>((r1 - qmin)/step);
                if (bin >= sz - 1)
                    bin = sz - 2;
                const double delta = r1 - qmin - bin*step;
                res = qtable_[bin] + delta/step*(qtable_[bin+1] - qtable_[bin]);
            }
            if (res > 1.0)
                res = 1.0;
            return res;
        }
    }
}
