#ifndef NORM_ACCUMULATOR_H_
#define NORM_ACCUMULATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    long double wsum;
    long double wsumsq;
    double wmax;
    unsigned ntries;
} norm_accumulator;

void norm_accumulate(norm_accumulator *acc, double weight);
void norm_reset(norm_accumulator *acc);
void norm_scale(norm_accumulator *acc, double factor);
double norm_value(const norm_accumulator *acc);
double norm_error(const norm_accumulator *acc);

/* The following function adds two measurements
 * assuming maximum correlation (that is, errors 
 * are added linearly). The result is placed into
 * the first accumulator.
 */
void norm_add_maxcorr(norm_accumulator *acc,
                      const norm_accumulator *added);

/* The following function adds two measurements
 * assuming no correlation (that is, errors
 * are added in quadrature). The result is placed
 * into the first accumulator.
 */
void norm_add_nocorr(norm_accumulator *acc,
                     const norm_accumulator *added);

#ifdef __cplusplus
}
#endif

#endif /* NORM_ACCUMULATOR_H_ */
