#ifndef SBSHAPE_H_
#define SBSHAPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* These functions calculate the integrals
 * 1/sqrt(Pi)/(1+exp(-(x-|b|)/|a|))^n * exp(-x^2)
 * from -Infinity to Infinity for n = 1 ... 6.
 *
 * They also find mean, variance, skewness, and kurtosis
 * of Johnson's S_b function from the translational
 * parameters gamma and delta. a and b differ from
 * delta and gamma by the sqrt(2) scale factor.
 *
 * After calling "johnint" function, values of mean, dskewdb,
 * and dkurtdb should be multiplied by exp(*logfactor).
 * The integrals should be multiplied by exp(*logfactor*n).
 * *var should be multiplied by exp(*logfactor*2.0). All these
 * multiplications are not necessary for "johnintf_".
 */
void johnint(double a, double b, double *ivalue, double *mean, double *var,
	     double *skew, double *kurt, double *logfactor, double *dskewda,
             double *dskewdb, double *dkurtda, double *dkurtdb);
void johnintf_(const double *gamma, const double *delta,
	       double *mean, double *var, double *skew, double *kurt,
               double *dskewddelta, double *dskewdgamma,
               double *dkurtddelta, double *dkurtdgamma);

/* Function for calculating Johnson's parameters from moments */
void sbfitmod_(const int *dumm, const double *mean,
               const double *sigma, const double *skew,
               const double *kurt, double *gamma, double *delta,
               double *xlam, double *xi, int *fault);

#ifdef __cplusplus
}
#endif

#endif /* SBSHAPE_H_ */
