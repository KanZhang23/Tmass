#ifndef MATREL_KLEISS_STIRLING_H_
#define MATREL_KLEISS_STIRLING_H_
 
#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif
 
/* This is the production and decay matrix element squared for
 *
 * q qbar -> t tbar -> wplus b wminus bbar -> ebar nu b mu nubar bbar
 * g g    -> t tbar -> wplus b wminus bbar -> ebar nu b mu nubar bbar
 *
 * according to R. Kleiss and W.J.Stirling, Z.Phys,C40 (1988) 419-423
 * which include spin correlations between the production and the decay.
 *
 * Input arguments are the four-vectors of the decay products, the
 * central value (mWnominal) and width (GW) of the W Breit-Wigner mass, 
 * and the top quark mass (mtop) for which the likelihood is calculated.
 * Note: b quarks can be massive.
 *       W and top masses need NOT to be on mass shell.
 *
 * The return arguments are the value of the squared matrix element
 * for qqbar->ttbar (matrel_qq) and gg->ttbar (matrel_gg).
 *
 * Oct-20-2004   Pedro Movilla Fernandez
 */
 
void matrel_kleiss_stirling(
     particle_obj ebar_lab, particle_obj nu_lab, particle_obj b_lab,
     particle_obj mu_lab, particle_obj nubar_lab, particle_obj bbar_lab,
     double mWnominal, double GW, double mtop, 
     double *matrel_qq, double *matrel_gg);

/* Similar function which automatically calls the above function
 * with proper placement of q and qbar. Note that it does not
 * symmetrize over the daus of hadronically decaying W (so that
 * this symmetrization has to be performed by the outside code).
 */
void kleiss_stirling_weights(
    particle_obj q, particle_obj qbar, particle_obj bhad,
    particle_obj blep, particle_obj lep, particle_obj nu,
    int leptonCharge, double mW, double widthW, double mt,
    double *weight_qq, double *weight_gg);
 
#ifdef __cplusplus
}
#endif

#endif /* MATREL_KLEISS_STIRLING_H_ */
