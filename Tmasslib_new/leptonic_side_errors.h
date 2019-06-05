#ifndef LEPTONIC_SIDE_ERRORS_H_
#define LEPTONIC_SIDE_ERRORS_H_

#include "simple_kinematics.h"
#include "solve_top.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    LEPERR_MWSQ = 0,
    LEPERR_MTSQ,
    N_LEPERR_ROWS
};

enum {
    LEPERR_B_ETA = 0,
    LEPERR_B_PHI,
    LEPERR_B_MSQ,
    LEPERR_L_PT,
    N_LEPERR_COLUMNS
};

void fill_lepton_side_solution(particle_obj l, particle_obj nu,
                               particle_obj b, lepton_side_solution *lsol);

/* The following function calculates MW^2 gradient assuming
 * that the effective top mass is a fixed parameter and the neutrino Pz
 * can vary. Lepton momentum components and mb are those for which
 * the lsol was obtained (mb is normally 0).
 */
void leptonic_mwsq_gradient(double lPx, double lPy, double lPz,
                            double mb, const lepton_side_solution *lsol,
                            double mwsq_gradient[N_LEPERR_COLUMNS]);

/* The following function calculates leptonic Mt^2 gradient assuming
 * that the leptonic W mass is a fixed parameter and the neutrino Pz
 * can vary. Lepton momentum components and mb are those for which
 * the lsol was obtained (mb is normally 0).
 */
void leptonic_mtsq_gradient(double lPx, double lPy, double lPz,
                            double mb, const lepton_side_solution *lsol,
                            double mtsq_gradient[N_LEPERR_COLUMNS]);

/* The following function assumes that neutrino Pz is a fixed
 * parameter and that effective top mass depends on it.
 */
void leperr_jacobian(double lPx, double lPy, double lPz,
		     double mb, const lepton_side_solution *lsol,
		     double jaco[N_LEPERR_ROWS][N_LEPERR_COLUMNS]);

#ifdef __cplusplus
}
#endif

#endif /* LEPTONIC_SIDE_ERRORS_H_ */
