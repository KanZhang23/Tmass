#ifndef CERNLIB_REPLACEMENT_H_
#define CERNLIB_REPLACEMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* CERNLIB replacements */

double cr_dfreq(const double *x);

double cr_dgausn(const double *x);

void cr_dgset(const double *a, const double *b, const int *n,
              double *x, double *w);

#ifdef __cplusplus
}
#endif

#endif /* CERNLIB_REPLACEMENT_H_ */
