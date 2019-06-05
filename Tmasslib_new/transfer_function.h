#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include "jet_info.h"
#include "simple_kinematics.h"
#include "tf_interpolator.h"

/* #define USE_ET_AS_TF_PREDICTOR */
/* #define USING_P_OVER_E_TF */

#ifdef __cplusplus
extern "C" {
#endif

/* The argument isB should be set to 0 for light quarks,
 * to 1 for b quarks on the hadronic side, and to 2 for
 * b quarks on the leptonic side.
 */
double transfer_function(const particle_obj solved_jet,
                         const jet_info* jseen,
                         double jes, int isB);

/* The "ptCut" argument below should include the JES factor already.
 * So, it should be set to
 *
 * tf_pt_cutoff*(1.0 + deltaJES*sigma_at_cutoff)
 *
 * The function assumes that jet eta and detector eta are the same.
 * This assumption differs with reality at the edges of the eta
 * bin definitions.
 */
double transfer_function_efficiency_2(const particle_obj solved_jet,
                                      double etaJet, int isB, double ptCut);

#ifdef __cplusplus
}
#endif

#endif /* TRANSFER_FUNCTION_H */
