#ifndef GENERATE_TTBAR_PT_H
#define GENERATE_TTBAR_PT_H

#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

v3_obj generate_ttbar_pt_fromrand(float rnd_r, float rnd_phi);
double ttbar_pt_weight(double mt, float rnd_r);
v3_obj generate_ttbar_pt(double mt);

#ifdef __cplusplus
}
#endif

#endif /* GENERATE_TTBAR_PT_H */
