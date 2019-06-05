#ifndef TTBAR_PHASE_SPACE_H_
#define TTBAR_PHASE_SPACE_H_

#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This function calculates the phase space factors for l+jets
 * ttbar decays. Various constant terms and terms which depend
 * only on jet angles and not energies are omitted.
 *
 * The first factor returned should be used while integrating
 * over the leptonic mW squared, and the second factor is needed
 * for integration over neutrino pz. If desired, one (or both)
 * result pointers can be safely set to 0 to save a few CPU cycles.
 */
void ttbar_phase_space(
    particle_obj lep, particle_obj nu, particle_obj blep,
    particle_obj q, particle_obj qbar, particle_obj bhad,
    double *factor_mWsq, double *factor_nuPz);

/* Function for setting various internal limits needed to avoid
 * phase space singularities. Do not call it unless you really
 * know what you are doing.
 */
void set_phase_space_limits(double nu_momentum_cutoff,
                            double q_momentum_cutoff,
                            double had_phase_space_cutoff,
                            double detlepNu_cutoff,
                            double detlepW_cutoff);

#ifdef __cplusplus
}
#endif

#endif /* TTBAR_PHASE_SPACE_H_ */
