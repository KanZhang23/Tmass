#ifndef MATREL_MAHLON_PARKE_H_
#define MATREL_MAHLON_PARKE_H_

#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is the production and decay matrix element squared for
 * q qbar -> t tbar -> wplus b wminus bbar -> ebar nu b mu nubar bbar
 *
 * The calculations are implemented according to formula (4) from
 * Mahlon and Parke, Physics Letters B, 411, pp 173-179 (1997).
 * b quarks are assumed to be massless. Constant factors are omitted.
 *
 * The central value and width of the W mass Breit-Wigner should
 * be provided as mWnominal and GW. The top mass value for which
 * likelihood is calculated should be given by mtop.
 *
 * The function returns the value of the squared matrix element.
 * If desired, pointers ptterm, ptbarterm, and pbigbrace can
 * be used to obtain the three major formula factors separately.
 */

double matrel_mahlon_parke(
    particle_obj ebar_lab, particle_obj nu_lab, particle_obj b_lab,
    particle_obj mu_lab, particle_obj nubar_lab, particle_obj bbar_lab,
    double mWnominal, double GW, double mtop,
    double *ptterm, double *ptbarterm, double *pbigbrace);

/* Similar function which automatically swaps W daughters
 * depending on the charge of the lepton. WORKS ONLY
 * FOR HERWIG MC!
 */
double mahlon_parke_weight(
    particle_obj q, particle_obj qbar, particle_obj bhad,
    particle_obj blep, particle_obj lep, particle_obj nu,
    int leptonCharge, double mW, double widthW, double mt);

#ifdef __cplusplus
}
#endif

#endif /* MATREL_MAHLON_PARKE_H_ */
