#ifndef SCRAMBLED_SOBOL_H_
#define SCRAMBLED_SOBOL_H_

#ifdef __cplusplus
extern "C" {
#endif

void i4_scrambled_sobol ( int dim_num, int *seed, float quasi[ ] );
void set_sobol_scrambling_multiplier(unsigned mult);

#ifdef __cplusplus
}
#endif

#endif /* SCRAMBLED_SOBOL_H_ */
