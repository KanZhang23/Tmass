#ifndef JOHNSON_RANDOM_H_
#define JOHNSON_RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The function returns 0 if the numbers are successfully generated
 * and 1 if the input parameters formed an incorrect combination
 */
int johnson_random(double mean, double sigma, double skew,
                   double kurt, double locat, double width,
		   double *rand, int nrand);

/* The following function returns the value of Johnson's
 * cumulative density without the "tanh_rise" component.
 * The function returns -1.0 in case the parameters make up
 * an incorrect combination.
 */
double johnson_cdf(double x, double mean, double sigma, 
                   double skew, double kurt);

/* The following function inverts the Johnson's cdf.
 * *status is set to 0 in case everything is OK.
 * The function returns 0.0 and *status is set to
 * a small non-zero integer in case of a problem.
 */
double johnson_invcdf(double cdf, double mean, double sigma, 
                      double skew, double kurt, int *status);

double tanh_rise(double x, double locat, double w);

#ifdef __cplusplus
}
#endif

#endif /* JOHNSON_RANDOM_H_ */
