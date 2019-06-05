#include <math.h>

#include "random_extra_jet.h"
#include "simple_kinematics.h"
#include "random_jet_masses.h"

#ifdef __cplusplus
extern "C" {
#endif

double uniform_random(void);

void random_extra_jet(const int isB, jet_info *out_jet)
{
    const double pt = uniform_random()*100.0 + 20.0;
    const double eta = uniform_random()*4.0 - 2.0;
    const double phi = uniform_random()*2.0*M_PI;
    v3_obj pjet = pt_eta_phi(pt, eta, phi);
    fill_jet_info(out_jet, pjet.x, pjet.y, pjet.z, random_jet_mass(isB),
                  isB ? 1.0 : 0.0, 0.0, eta, 0.03, 0.0, 0.03, 0, -1, 0);
}

#ifdef __cplusplus
}
#endif
