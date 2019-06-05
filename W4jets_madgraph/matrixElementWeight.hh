#ifndef MATRIXELEMENTWEIGHT_HH_
#define MATRIXELEMENTWEIGHT_HH_

#include "rk/rk.hh"
#include "Parameters_sm_no_b_mass.h"
#include "Pythia8/Pythia.h"

double matrixElementWeight(Pythia8::Pythia* pythiaIn,
                           Pythia8::Parameters_sm_no_b_mass* parsIn,
                           double eBeam, double nominalWmass,
                           const rk::P4 partons[4],
                           const rk::P4& l, int leptonCharge,
                           const rk::P4& nu);


double weightForKnownWMass(Pythia8::Pythia* pythiaIn,
                           Pythia8::Parameters_sm_no_b_mass* parsIn,
                           unsigned mode, double eBeam,
                           double nominalWmass, bool integrateOverNuPz,
                           const rk::P4 partons[4],
                           const geom3::Vector3& neutrinoPt,
                           const rk::P4& lepton, int leptonCharge,
                           double mwsq, double wSamplingDensity);

#endif // MATRIXELEMENTWEIGHT_HH_
