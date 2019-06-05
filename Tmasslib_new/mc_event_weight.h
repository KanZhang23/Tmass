#ifndef MC_EVENT_WEIGHT_H_
#define MC_EVENT_WEIGHT_H_

#include "topmass_integrator.h"
#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

double mc_event_weight(
    const integ_parameters *ipars,
    particle_obj q, particle_obj qbar, particle_obj bhad,
    particle_obj blep, particle_obj lep, particle_obj nu,
    int leptonCharge, double mtpole);

#ifdef __cplusplus
}
#endif

#endif /* MC_EVENT_WEIGHT_H_ */
