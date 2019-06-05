#ifndef N_D_RANDOM_H_
#define N_D_RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    N_D_RANDOM_DRAND48 = 0,
    N_D_RANDOM_HALTON,
    N_D_RANDOM_SOBOL,
    N_D_RANDOM_FORT_SOBOL,
    N_D_RANDOM_SCRAM_SOBOL,
    N_D_RANDOM_NIEDER,
    N_D_RANDOM_INVALID
} N_d_random_method;

/* For N_D_RANDOM_DRAND48 method, "param" argument is used as a seed
 * if it is not 0. For N_D_RANDOM_SOBOL and N_D_RANDOM_SCRAM_SOBOL methods,
 * positive "param" value can be used to skip "param" initial points.
 */
void init_n_d_random(N_d_random_method rand_method, unsigned ndim, int param);
void next_n_d_random(float *r);
void cleanup_n_d_random(void);

N_d_random_method parse_n_d_random_method(char *cmethod);
unsigned current_n_d_random_ndim(void);
N_d_random_method current_n_d_random_method(void);

#ifdef __cplusplus
}
#endif

#endif /* N_D_RANDOM_H_ */
