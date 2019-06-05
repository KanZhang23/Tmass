#ifndef PARTON_SYSERR_H_
#define PARTON_SYSERR_H_

#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef double (Parton_systematic_error)(const particle_obj* parton, int isB);

Parton_systematic_error parton_syserr;

void set_parton_syserr_calculator(Parton_systematic_error* calc);

#ifdef __cplusplus
}
#endif

#endif /* PARTON_SYSERR_H_ */
