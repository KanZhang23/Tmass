#include <cfloat>

#include "HighEnergyTauTF.hh"
#include "misc_utils.hh"

#include "geners/binaryIO.hh"
#include "npstat/stat/distributionReadError.hh"

using namespace npstat;

bool HighEnergyTauTF::write(std::ostream& os) const
{
    gs::write_pod(os, polar_);
    return !os.fail();
}

HighEnergyTauTF* HighEnergyTauTF::read(const gs::ClassId& id, std::istream& in)
{
    static const gs::ClassId current(gs::ClassId::makeId<HighEnergyTauTF>());
    current.ensureSameId(id);

    double p;
    gs::read_pod(in, &p);
    if (!in.fail())
        return new HighEnergyTauTF(p);
    else
    {
        distributionReadError(in, classname());
        return 0;
    }
}

double HighEnergyTauTF::density(const double x) const
{
    if (x >= 0.0 && x < 1.0)
        return 5.0/3.0-3.0*x*x+4.0/3.0*x*x*x-
            polar_*(1.0/3.0-3.0*x*x+8.0/3.0*x*x*x);
    else
        return 0.0;
}

double HighEnergyTauTF::cdf(const double x) const
{
    if (x <= 0.0)
        return 0.0;
    else if (x >= 1.0)
        return 1.0;
    else
    {
        const double xm1 = x - 1.0;
        return x*(5.0 - 3.0*x*x + x*x*x - polar_*xm1*xm1*(2.0*x + 1.0))/3.0;
    }
}

double HighEnergyTauTF::exceedance(const double x) const
{
    return 1.0 - cdf(x);
}

double HighEnergyTauTF::quantile(const double rnd) const
{
    assert(rnd >= 0.0 && rnd <= 1.0);
    if (rnd == 0.0)
        return 0.0;
    else if (rnd == 1.0)
        return 1.0;
    else
    {
        double xmin = 0.0;
        double xmax = 1.0;
        for (unsigned i=0; i<1000; ++i)
        {
            if (relative_delta(xmin, xmax) <= 4.0*DBL_EPSILON)
                break;
            const double x = (xmin + xmax)/2.0;
            const double cdfval = cdf(x);
            if (cdfval == rnd)
                return x;
            else if (cdfval > rnd)
                xmax = x;
            else
                xmin = x;
        }
        return (xmin + xmax)/2.0;
    }
}
