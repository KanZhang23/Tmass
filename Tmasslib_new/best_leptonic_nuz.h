#ifndef BEST_LEPTONIC_NUZ_
#define BEST_LEPTONIC_NUZ_

#ifdef __cplusplus
extern "C" {
#endif

int best_leptonic_nuz(const double topPx, const double topPy, 
                      const double lPx, const double lPy, const double lPz,
                      const double bPx, const double bPy, const double bPz,
                      const double mb, const double mW0, const double mT0,
                      const double hwhmWsq, const double hwhmTsq,
                      const double nuPz_ref, const double Ecms, double *nuPz,
		      double *distance, double *mw, double *mt);

#ifdef __cplusplus
}
#endif

#endif /* BEST_LEPTONIC_NUZ_ */
