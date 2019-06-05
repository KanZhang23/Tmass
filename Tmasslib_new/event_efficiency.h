#ifndef EVENT_EFFICIENCY_H_
#define EVENT_EFFICIENCY_H_

#include "simple_kinematics.h"

#define W_RECO_EFF_BIT 1U

#ifdef __cplusplus
extern "C" {
#endif

double w_reco_efficiency(particle_obj q, particle_obj qbar);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_EFFICIENCY_H_ */
