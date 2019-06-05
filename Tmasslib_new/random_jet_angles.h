#ifndef RANDOM_JET_ANGLES_H_
#define RANDOM_JET_ANGLES_H_

#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

void set_eta_width_factor(double factor, int isB);
void set_phi_width_factor(double factor, int isB);

double deta_density(double pt, int isB, double deta);
double dphi_density(double pt, int isB, double dphi);

double random_b_deta(double pt);
double random_b_dphi(double pt);
double random_q_deta(double pt);
double random_q_dphi(double pt);

double b_eta_error(double pt);
double b_phi_error(double pt);
double q_eta_error(double pt);
double q_phi_error(double pt);

v3_obj new_eta_phi_fromrand(v3_obj orig, int isB,
                            float rnd_eta, float rnd_phi);

#ifdef __cplusplus
}
#endif

#endif /* RANDOM_JET_ANGLES_H_ */
