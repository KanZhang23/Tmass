#ifndef RANDOM_LEPTON_MOMENTUM_H_
#define RANDOM_LEPTON_MOMENTUM_H_

#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

double mu_pt_bias(double pt);
double e_pt_bias(double pt);

double mu_pt_error(double pt);
double e_pt_error(double pt);

double random_mu_dpt(double pt);
double random_e_dpt(double pt);

v3_obj randomize_lepton_momentum(v3_obj orig, int isMu);

#ifdef __cplusplus
}
#endif

#endif /* RANDOM_LEPTON_MOMENTUM_H_ */
