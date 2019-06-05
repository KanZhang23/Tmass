#ifndef BW_POLYNOMIAL_H_
#define BW_POLYNOMIAL_H_

#define MAX_BW_POLY_DEGREE 20

#ifdef __cplusplus
extern "C" {
#endif

/* The following function calculates polynomials
 * whose generating function is 1/((x - x0)^2 + 1)
 * at x->Infinity. These polynomials are useful in
 * multipole expansion of some density convoluted
 * with Breit-Wigner distribution.
 *
 * "degree" argument is the power of 1/x in the
 * expansion. Should be <= MAX_BW_POLY_DEGREE.
 */
double bw_polynomial(double x0, unsigned degree);

/* Multipole expansion of Breit-Wigner convoluted density.
 * Should have (maxdegree - 1) coefficients, starting with
 * the coefficient for (1/x)^2. Logically |x| must be larger than 1,
 * otherwise the expansion will not converge. The function will
 * simply return 0 for all x such that |x| < 1.0.
 */
double bw_expansion(double x, const double *coeffs, unsigned maxdegree);

/* Improper integral of the above. If the integral to be
 * calculated from x to +Infinity then the result returned
 * by this function should be multiplied by -1.
 */
double bw_expansion_integral(double x, const double *coeffs,
                             unsigned maxdegree);

#ifdef __cplusplus
}
#endif

#endif /* BW_POLYNOMIAL_H_ */
