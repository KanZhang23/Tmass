#include <assert.h>
#include "parton_syserr.h"

#ifdef __cplusplus
extern "C" {
#endif

static Parton_systematic_error* calculator = 0;

void set_parton_syserr_calculator(Parton_systematic_error* calc)
{
    calculator = calc;
}

double parton_syserr(const particle_obj* parton, int isB)
{
    assert(calculator);
    return calculator(parton, isB);
}
    
#ifdef __cplusplus
}
#endif
