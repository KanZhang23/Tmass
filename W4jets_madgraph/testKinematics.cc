#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>

#include "w4jetsKinematics.hh"
#include "misc_utils.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/TruncatedDistribution1D.hh"

#include "npstat/nm/MathUtils.hh"

using namespace rk;
using namespace geom3;
using namespace npstat;

namespace {
    class RandomWGen
    {
    public:
        inline RandomWGen()
            : wMassScanner_(Cauchy1D(80.4*80.4, 80.4*2.1), 1.0, 4.0*80.4*80.4)
        {
        }

        inline P4 next() const
        {
            const double mwsq = wMassScanner_.quantile(drand48());
            const double mw = sqrt(mwsq);
            UnitVector3 u(UnitVector3::random(drand48(), drand48()));
            const double p(1.0 + drand48()*200.0);
            return P4(p*u, mw);
        }

    private:
        TruncatedDistribution1D wMassScanner_;
    };
}

static double max_lepton_pt(const double wx, const double wy, const double mwsq,
                            const double lPx, const double lPy)
{
    const double lPt = hypot(lPx, lPy);
    const double cex = lPx/lPt;
    const double cey = lPy/lPt;
    const double a = cey*cey*(mwsq + wx*wx) + cex*cex*(mwsq + wy*wy) -
                     2.0*cex*cey*wx*wy;
    const double b = -mwsq*(cex*wx + cey*wy);
    const double c = -mwsq*mwsq/4.0;

    double pt[2], pt_viable[2];
    int i, nsols, n_viable = 0;

    assert(lPt > 0.0);

    if (fabs(a) < 1.0e-20)
    {
        if (b == 0.0)
            return 0.0;
        else
        {
            nsols = 1;
            pt[0] = -c/b;
        }
    }
    else
        nsols = solveQuadratic(b/a, c/a, pt, pt + 1);

    for (i=0; i<nsols; ++i)
    {
        const double thispt = pt[i];
        if (thispt > 0.0)
        {
            const double nux = wx - thispt*cex;
            const double nuy = wy - thispt*cey;
            if (mwsq/2.0 + thispt*cex*nux + thispt*cey*nuy > 0.0)
                pt_viable[n_viable++] = thispt;
        }
    }

    switch (n_viable)
    {
    case 0:
        return 0.0;
    case 1:
        return pt_viable[0];
    case 2:
        return pt_viable[0] > pt_viable[1] ? pt_viable[0] : pt_viable[1];
    default:
        assert(0);
    }
}

static void test_nu_pz_solutions(const unsigned ntries)
{
    const double tol = 1.0e-9;
    RandomWGen wgen;

    for (unsigned itry=0; itry<ntries; ++itry)
    {
        const P4 w(wgen.next());
        const double mw = w.m();

        P4 l, nu;
        phaseSpaceDecay(w, 0.0, 0.0, drand48(), drand48(), &l, &nu);
        double nuPz[2];
        const unsigned nsol = determineNuPz(nu.transverse(), l.momentum(),
                                            mw*mw, nuPz);
        assert(nsol == 2);
        bool found = false;
        for (unsigned isol=0; isol<nsol; ++isol)
            if (relative_delta(nu.pz(), nuPz[isol]) < tol)
                found = true;
        assert(found);
    }
}

static void compare_lep_mom(const unsigned ntries)
{
    const double tol = 1.0e-9;
    RandomWGen wgen;

    for (unsigned itry=0; itry<ntries; ++itry)
    {
        const P4 w(wgen.next());
        const double mw = w.m();

        P4 l, nu;
        phaseSpaceDecay(w, 0.0, 0.0, drand48(), drand48(), &l, &nu);
        const double maxP = maxLeptonMomentum(w.px(), w.py(),
                                              mw*mw, l.momentum());
        const double maxPt = max_lepton_pt(w.px(), w.py(), mw*mw,
                                           l.px(), l.py());
        Vector3 maxl(maxP*l.momentum().direction());
        const double newPt = sqrt(maxl.x()*maxl.x() + maxl.y()*maxl.y());

        assert(relative_delta(maxPt, newPt) < tol);
    }
}

int main()
{
    const unsigned standardNCycles = 100000;

    test_nu_pz_solutions(standardNCycles);
    compare_lep_mom(standardNCycles);

    return 0;
}
