#ifndef TOPMASS_UTILS_H_
#define TOPMASS_UTILS_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Utility for converting covariance matrix into correlation matrix.
 * Square roots of the diagonal elements are returned in the "stdevs" array.
 */
void covar_to_corr(double *covmat, const unsigned n_rows, double *stdevs);

/* Utility to allocate work memory */
void get_static_memory(void **ptr, size_t structsize,
		       unsigned *nmemb, unsigned newmemb);

/* A function to read from file to a buffer. Retries after EINTR. */
void read_n_bytes(int fileno, void *buf, size_t toread);

/* Breit-Wigner densities. Note that "relativistic_bw_density"
 * returns a density in m^2 space even though the argument is m.
 */
double cauchy_density(double x, double peak, double hwhm);
double relativistic_bw_density(double m, double polemass, double width);

/* Some random number generators */
double uniform_random(void);
double normal_random(void);
double gauss_random(double mean, double sigma);
double cauchy_random(double peak, double hwhm);
double ouni_(void);

/* Set the random generator seed. If argument is 0, the seed will be read
 * from /dev/urandom. Returns the seed used.
 */
unsigned uniform_random_setseed(unsigned seed);

/* A function to pick the closest array member for a given distance function */
double pick_closest_element(const double *arr, unsigned len, double value,
                            double (*distance)(double, double));

#ifdef __cplusplus
}
#endif

#endif /* TOPMASS_UTILS_H_ */
