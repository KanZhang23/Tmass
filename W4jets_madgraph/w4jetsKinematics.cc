#include <cmath>
#include <cassert>

#include "npstat/nm/MathUtils.hh"

#include "w4jetsKinematics.hh"

using namespace rk;
using namespace geom3;

double minimumWMassSquared(const Vector3& neutrinoPt,
                           const Vector3& leptonMomentum)
{
    // The W will have the minimum possible mass when the theta
    // of the neutrino is the same as the theta of the lepton
    const double nux = neutrinoPt.x();
    const double nuy = neutrinoPt.y();
    const double nupt = sqrt(nux*nux + nuy*nuy);

    const double lx = leptonMomentum.x();
    const double ly = leptonMomentum.y();
    const double lpt = sqrt(lx*lx + ly*ly);
    assert(lpt > 0.0);

    Vector3 nuW(nux, nuy, leptonMomentum.z()/lpt*nupt);
    return 2.0*(nuW.length()*leptonMomentum.length() - nuW.dot(leptonMomentum));
}

unsigned determineNuPz(const geom3::Vector3& neutrinoPt,
                       const geom3::Vector3& leptonMomentum,
                       const double wMassSquared, double nuPz[2])
{
    const double nux = neutrinoPt.x();
    const double nuy = neutrinoPt.y();
    const double nuptSquared = nux*nux + nuy*nuy;

    const double lx = leptonMomentum.x();
    const double ly = leptonMomentum.y();
    const double lz = leptonMomentum.z();
    const double lptSquared = lx*lx + ly*ly;
    assert(lptSquared > 0.0);

    const double tmp = wMassSquared/2.0 + lx*nux + ly*nuy;
    const double b = -2.0*lz*tmp;
    const double c = (lptSquared + lz*lz)*nuptSquared - tmp*tmp;

    return npstat::solveQuadratic(b/lptSquared, c/lptSquared, nuPz, nuPz+1);
}

double maxLeptonMomentum(const double wx, const double wy,
                         const double wMassSquared,
                         const geom3::Vector3& leptonMomentum)
{
    const double lx = leptonMomentum.x();
    const double ly = leptonMomentum.y();
    const double lz = leptonMomentum.z();
    const double lptSquared = lx*lx + ly*ly;
    assert(lptSquared > 0.0);

    const double ptsq = wx*wx + wy*wy;
    const double etsq = ptsq + wMassSquared;
    const double wzsq = etsq/lptSquared*lz*lz;
    double wz = sqrt(wzsq);
    if (lz < 0.0)
        wz *= -1.0;
    const double ew = sqrt(ptsq + wzsq + wMassSquared);
    const double len = sqrt(lptSquared + lz*lz);

    return wMassSquared/2.0/(ew - (lx*wx + ly*wy + lz*wz)/len);
}
