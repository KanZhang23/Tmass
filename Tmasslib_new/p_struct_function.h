#ifndef P_STRUCT_FUNCTION_H_
#define P_STRUCT_FUNCTION_H_

#include "simple_kinematics.h"

/* This is the parton distribution function term for the
 * p pbar -> t tbar process. Works by interfacing LHAPDF.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* In the function below, p1 and p2 are the two particles produced
 * in an inelastic p pbar collision, in the p pbar center of mass
 * system. Ecms is the p pbar center-of-mass energy.
 */
void pdfterm_qqbar(const particle_obj p1, const particle_obj p2,
                   const double Ecms, double *weight_qq, double *weight_gg);

#ifdef __cplusplus
}
#endif

#endif /* P_STRUCT_FUNCTION_H_ */
