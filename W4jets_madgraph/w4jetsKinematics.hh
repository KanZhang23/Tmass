#ifndef W4JETSKINEMATICS_HH_
#define W4JETSKINEMATICS_HH_

#include "rk/rk.hh"

// Minimum possible W mass for known charged lepton momentum,
// known neutrino transverse momentum, and arbitrary neutrino Pz.
// In this calculation, charged lepton is assumed to be massless.
//
double minimumWMassSquared(const geom3::Vector3& neutrinoPt,
                           const geom3::Vector3& leptonMomentum);

// Determine the neutrino Pz in the decay W -> l nu in case we know
// lepton momentum, neutrino transverse momentum, and W mass.
// The function returns the number of kinematic solutions found.
// On output, the "nuPz" array will be filled with these solutions.
// In this calculation, charged lepton is assumed to be massless.
//
unsigned determineNuPz(const geom3::Vector3& neutrinoPt,
                       const geom3::Vector3& leptonMomentum,
                       double wMassSquared, double nuPz[2]);

// The following function returns the maximum possible lepton momentum
// in the decay of the W into a lepton and a neutrino in case we know
// the direction of the lepton as well as W mass and transverse momentum.
// The "leptonDirection" vector can have arbitrary magnitude, only
// its direction will be used internally.
//
double maxLeptonMomentum(double wPx, double wPy, double wMassSquared,
                         const geom3::Vector3& leptonDirection);

#endif // W4JETSKINEMATICS_HH_
