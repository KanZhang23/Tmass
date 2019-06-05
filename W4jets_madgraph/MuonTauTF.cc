#include <cmath>
#include <cfloat>
#include <cassert>

#include "MuonTauTF.hh"
#include "misc_utils.hh"

#include "geners/binaryIO.hh"
#include "npstat/stat/distributionReadError.hh"

#define SQRPI 1.77245385090551603

using namespace npstat;

// Calculate the integral over x from 0 to a of the following expression:
//
// exp(-x*x/s/s)*(5.0/3.0-3.0*x*x+4.0/3.0*x*x*x-
//                p*(1.0/3.0-3.0*x*x+8.0/3.0*x*x*x));
//
static double muonInteg(const double p, const double s, const double a)
{
    const double c0 = (5.0 - p)/3.0;
    const double c2 = 3.0*(p - 1.0);
    const double c3 = 4.0/3.0*(1.0 - 2.0*p);

    const double r = a/s;
    const double ssq = s*s;
    const double erfr = erf(r);
    const double exprsq = exp(-r*r);
    const double i0 = SQRPI*s/2.0*erfr;
    const double i2 = ssq*(i0 - a*exprsq)/2.0;
    const double i3 = ssq*(ssq - (ssq + a*a)*exprsq)/2.0;

    return c0*i0 + c2*i2 + c3*i3;
}

MuonTauTF::MuonTauTF(const double tauPolarization, const double s)
    : polar_(tauPolarization),
      s_(s)
{
    assert(s_ > 0.0);
    normfactor_ = 1.0/(1.0 - muonInteg(polar_, s_, 1.0));
}

bool MuonTauTF::write(std::ostream& os) const
{
    gs::write_pod(os, polar_);
    gs::write_pod(os, s_);
    return !os.fail();
}

MuonTauTF* MuonTauTF::read(const gs::ClassId& id, std::istream& in)
{
    static const gs::ClassId current(gs::ClassId::makeId<MuonTauTF>());
    current.ensureSameId(id);

    double p, s;
    gs::read_pod(in, &p);
    gs::read_pod(in, &s);
    if (!in.fail())
        return new MuonTauTF(p, s);
    else
    {
        distributionReadError(in, classname());
        return 0;
    }
}

double MuonTauTF::density(const double x) const
{
    if (x > 0.0 && x < 1.0)
    {
        const double r = x/s_;
        return normfactor_*(1.0-exp(-r*r))*(5.0/3.0-3.0*x*x+4.0/3.0*x*x*x-
                                polar_*(1.0/3.0-3.0*x*x+8.0/3.0*x*x*x));
    }
    else
        return 0.0;
}

double MuonTauTF::cdf(const double x) const
{
    if (x <= 0.0)
        return 0.0;
    else if (x >= 1.0)
        return 1.0;
    else
    {
        const double xm1 = x - 1.0;
        const double cdf1 = x*(5.0 - 3.0*x*x + x*x*x - 
                               polar_*xm1*xm1*(2.0*x + 1.0))/3.0;
        return normfactor_*(cdf1 - muonInteg(polar_, s_, x));
    }
}

double MuonTauTF::exceedance(const double x) const
{
    return 1.0 - cdf(x);
}

double MuonTauTF::quantile(const double rnd) const
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
