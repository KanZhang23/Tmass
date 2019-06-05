#ifndef CONVOLUTE_BREIT_WIGNER_H_
#define CONVOLUTE_BREIT_WIGNER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The function below assumes equidistant points. The "hwhm_ratio"
 * argument is the ratio between the width of the Breit-Wigner and
 * the distance between adjacent points in the density curve.
 */

void convolute_breit_wigner(double *data,
                            const size_t npoints,
                            const double hwhm_ratio);

#ifdef __cplusplus
}
#endif

#endif /* CONVOLUTE_BREIT_WIGNER_H_ */
