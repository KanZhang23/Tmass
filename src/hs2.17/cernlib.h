#ifndef CERNLIB_H
#define CERNLIB_H

/* Header file for a few CERNLIB functions and subroutines */

#ifdef __cplusplus
extern "C" {
#endif

void lfit_(float *x, float *y, int *l, int *key,
	   float *a, float *b, float *var);
void lfitw_(float *x, float *y, float *w, int *l, int *key,
	    float *a, float *b, float *var);
void rnormx_(float *rvec, int *len, void (*rangen)(float *, int *));
void rnpssn_(float *mean, int *N, int *IERR);
void tlsc_(float *a, float *b, float *aux, int *ipiv, float *eps, float *x);
void tls_(float *a, float *b, float *aux, int *ipiv, float *eps, float *x);
void tlerr_(float *a, float *e, float *aux, int *ipiv);
void tlres_(float *a, float *b, float *aux);
void rlsqp1_(int *n, float *x, float *y,
	     float *a0, float *a1, float *s, int *ifail);
void rlsqp2_(int *n, float *x, float *y,
	     float *a0, float *a1, float *a2, float *s, int *ifail);
void dlsqp2_(int *n, double *x, double *y,
	     double *a0, double *a1, double *a2, double *s, int *ifail);
void lsq_(int *n, float *x, float *y, int *m, float *a);
void rfft_(float *y, int *m);
void dsinv_(int *n, double *a, int *idim, int *ifail);
void dseqn_(int *N, double *A, int *IDIM, int *IFAIL, int *K, double *B);
void deqn_(int *N, double *A, int *IDIM, double *WORK,
	   int *IFAIL, int *K, double *B);
void dinv_(int *n, double *a, int *idim, double *WORK, int *ifail);
void dfact_(int *n, double *a, int *idim, double *WORK, int *ifail,
            double *det, int *jfail);
double dgquad_(double (*f)(double *), double *a, double *b, int *n);
void dgset_(double *a, double *b, int *n, double *x, double *w);
double dgausn_(double *x);
float probkl_(float *x);
void drteq3_(double *R, double *S, double *T, double *X, double *D);
void drteq4_(double *A, double *B, double *C, double *D,
             double *Z, double *DC, int *MT);
void rnbnml_(int *M, float *P, int *N, int *IERR);
void rnmnml_(int *N, int *NSUM, float *PCUM, int *NVEC, int *IERR);
float prob_(float *x, int *n);

/* Note that s is a (double *), not (float *) in the declaration below.
 * This is correct, although contrary to the CERNLIB manual.
 */
void rlsqpm_(int *n, float *x, float *y, int *m,
	     float *a, double *s, int *ifail);

/* Interface to the RANLUX random number generator, CERNLIB entry V115 */
void ranlux_(float *rvec, int *len);    
void rluxgo_(int *lux, int *init, int *k1, int *k2);
void rluxat_(int *lux, int *init, int *k1, int *k2);
void rluxin_(int *ivec);
void rluxut_(int *ivec);

/* Interface to HBOOK/ZEBRA */
void hlimit_(int *limit);
void hropen_(int *unit, char *rzdir, char *fname, char *opt, int *recl,
	     int *ierr, int rzdir_len, int fname_len, int opt_len);
void rzink_(int *KEYU, int *ICYCLE, char *CHOPT, int CHOPT_len);
void hrin_(int *ID, int *ICYCLE, int *IOFSET);
void hdcofl_(void);
void hdelet_(int *id);
void zitoh_(int *INTV, int *HOLL, int *NP);
void uhtoc_(int *MS, int *NPW, char *MT, int *NCH, int MT_len);
void hcdir_(char *path, char *opt, int path_len, int opt_len);
void hnoent_(int *id, int *nentries);
void hgive_(int *id, char *chtitl, int *ncx, float *xmin, float *xmax,
	    int *ncy, float *ymin, float *ymax, int *nwt, int *idb, int len_80);
void hgiven_(int *id, char *chtitl, int *nvar, char *chtag,
	     float *rlow, float *rhi, int len_80, int len_chtag);
void hunpak_(int *id, float *data, char *choice, int *num, int len_choice);
float hij_(int *id, int *ix, int *iy);
float hi_(int *id, int *ix);
void hgnpar_(int *id, char *name, int name_len);
void hgnf_(int *id, int *i, float *x, int *ierr);
void hunpke_(int *id, float *data, char *choice, int *num, int len_choice);
void hldir_(char *dir, char *opt, int dir_len, int opt_len);

/* Interface to LAPACK */
void dsyev_(char *JOBZ, char *UPLO, int *N, double *A, int *LDA, double *W,
            double *WORK, int *LWORK, int *INFO, int lenJOBZ, int lenUPLO);
void dgetrf_(int *M, int *N, double *A, int *LDA, int *IPIV, int *INFO);
void dgetrs_(char *TRANS, int *N, int *NRHS, double *A, int *LDA, int *IPIV,
	     double *B, int *LDB, int *INFO, int lenTRANS);

#ifdef __cplusplus
}
#endif

#endif /* not CERNLIB_H */
