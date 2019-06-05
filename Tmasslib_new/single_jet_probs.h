#ifndef SINGLE_JET_PROBS_H_
#define SINGLE_JET_PROBS_H_

#include "jet_info.h"
#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef double (Prob_to_acquire_jet)(const jet_info *jets, unsigned njets,
                                     unsigned which, double jes);

typedef double (Prob_to_loose_parton)(const particle_obj *,
                                      double deltaJES, int isB);

Prob_to_acquire_jet unit_prob_to_acquire_jet;
Prob_to_acquire_jet invalid_prob_to_acquire_jet;

Prob_to_loose_parton unit_prob_to_loose_parton;
Prob_to_loose_parton mc_eff_prob_to_loose_parton;
Prob_to_loose_parton drop_prob_to_loose_parton;
Prob_to_loose_parton invalid_prob_to_loose_parton;
Prob_to_loose_parton single_parton_eff_prob_to_loose_parton;
Prob_to_loose_parton total_prob_to_loose_parton;

Prob_to_acquire_jet* choose_prob_to_acquire_jet(const char *name);
Prob_to_loose_parton* choose_prob_to_loose_parton(const char *name);

void set_params_to_loose_parton(double pt_cut, double eta_cut,
                                double max_efficiency,
                                double jes_sigma_factor);

#ifdef __cplusplus
}
#endif

#endif /* SINGLE_JET_PROBS_H_ */
