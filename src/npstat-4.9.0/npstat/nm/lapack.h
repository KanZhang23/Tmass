#ifndef NPSTAT_LAPACK_H_
#define NPSTAT_LAPACK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* SVD solver for a linear least squares problem which minimizes
 * L2 norm of (A*x - b)
 */
void dgelsd_(int *M, int *N, int *NRHS, double *A,
             int *LDA, double *B, int *LDB, double *S,
             double *RCOND, int *RANK, double *WORK,
             int *LWORK, int *IWORK, int *INFO);
void sgelsd_(int *M, int *N, int *NRHS, float *A,
             int *LDA, float *B, int *LDB, float *S,
             float *RCOND, int *RANK, float *WORK,
             int *LWORK, int *IWORK, int *INFO);

/* Solve a system of linear equations */
void dgetrs_(char *TRANS, int *N, int *NRHS, double *A, int *LDA, int *IPIV,
	     double *B, int *LDB, int *INFO, int lenTRANS);
void sgetrs_(char *TRANS, int *N, int *NRHS, float *A, int *LDA, int *IPIV,
	     float *B, int *LDB, int *INFO, int lenTRANS);

/* Calculation of eigenvalues and (optionally) eigenvectors
 * of a general matrix
 */
void dgeev_(char *JOBVL, char *JOBVR, int *N, double *A, int *LDA,
            double *WR, double *WI, double *VL, int *LDVL,
            double *VR, int *LDVR, double *WORK,
            int *LWORK, int *INFO, int lenJOBVL, int lenJOBVR);
void sgeev_(char *JOBVL, char *JOBVR, int *N, float *A, int *LDA,
            float *WR, float *WI, float *VL, int *LDVL,
            float *VR, int *LDVR, float *WORK,
            int *LWORK, int *INFO, int lenJOBVL, int lenJOBVR);

/* Calculation of eigenvalues and (optionally) eigenvectors
 * of a symmetric matrix
 */
void dsyev_(char *JOBZ, char *UPLO, int *N, double *A, int *LDA, double *W,
            double *WORK, int *LWORK, int *INFO, int lenJOBZ, int lenUPLO);
void ssyev_(char *JOBZ, char *UPLO, int *N, float *A, int *LDA, float *W,
            float *WORK, int *LWORK, int *INFO, int lenJOBZ, int lenUPLO);

/* Calculation of eigenvalues and (optionally) eigenvectors
 * of a symmetric matrix using divide and conquer algorithm
 */
void dsyevd_(char *JOBZ, char *UPLO, int *N, double *A, int *LDA, double *W,
             double *WORK, int *LWORK, int *IWORK, int *LIWORK,
             int *INFO, int lenJOBZ, int lenUPLO);
void ssyevd_(char *JOBZ, char *UPLO, int *N, float *A, int *LDA, float *W,
             float *WORK, int *LWORK, int *IWORK, int *LIWORK,
             int *INFO, int lenJOBZ, int lenUPLO);

/* Calculation of eigenvalues and (optionally) eigenvectors
 * of a symmetric matrix using "Relatively Robust Representations"
 */
void dsyevr_(char *JOBZ, char *RANGE, char *UPLO,
             int *N, double *A, int *LDA, double *VL, double *VU,
             int *IL, int *IU, double *ABSTOL, int *M,
             double *W, double *Z, int *LDZ, int *ISUPPZ,
             double *WORK, int *LWORK, int *IWORK, int *LIWORK,
             int *INFO, int lenJOBZ, int lenRANGE, int lenUPLO);
void ssyevr_(char *JOBZ, char *RANGE, char *UPLO,
             int *N, float *A, int *LDA, float *VL, float *VU,
             int *IL, int *IU, float *ABSTOL, int *M,
             float *W, float *Z, int *LDZ, int *ISUPPZ,
             float *WORK, int *LWORK, int *IWORK, int *LIWORK,
             int *INFO, int lenJOBZ, int lenRANGE, int lenUPLO);

/* Calculation of eigenvalues and (optionally) eigenvectors of a symmetric
 * tridiagonal matrix using "Relatively Robust Representations"
 */
void dstevr_(char *JOBZ, char *RANGE,
             int *N, double *D, double *E, double *VL, double *VU,
             int *IL, int *IU, double *ABSTOL, int *M,
             double *W, double *Z, int *LDZ, int *ISUPPZ,
             double *WORK, int *LWORK, int *IWORK, int *LIWORK,
             int *INFO, int lenJOBZ, int lenRANGE);
void sstevr_(char *JOBZ, char *RANGE,
             int *N, float *D, float *E, float *VL, float *VU,
             int *IL, int *IU, float *ABSTOL, int *M,
             float *W, float *Z, int *LDZ, int *ISUPPZ,
             float *WORK, int *LWORK, int *IWORK, int *LIWORK,
             int *INFO, int lenJOBZ, int lenRANGE);

/* LU factorization */
void dgetrf_(int *M, int *N, double *A, int *LDA, int *IPIV, int *INFO);
void sgetrf_(int *M, int *N, float *A, int *LDA, int *IPIV, int *INFO);

/* Cholesky factorization */
void dpotrf_(char *UPLO, int *N, double *A, int *LDA, int *INFO, int lenUPLO);
void spotrf_(char *UPLO, int *N, float *A, int *LDA, int *INFO, int lenUPLO);

/* Factorization of a real symmetric matrix using Bunch-Kaufman
 * diagonal pivoting method
 */
void dsytrf_(char *UPLO, int *N, double *A, int *LDA, int *IPIV,
             double *WORK, int *LWORK, int *INFO, int lenUPLO);
void ssytrf_(char *UPLO, int *N, float *A, int *LDA, int *IPIV,
             float *WORK, int *LWORK, int *INFO, int lenUPLO);

/* Inverse of a general matrix */
void dgetri_(int *N, double *A, int *LDA, int *IPIV,
             double *WORK, int *LWORK, int *INFO);
void sgetri_(int *N, float *A, int *LDA, int *IPIV,
             float *WORK, int *LWORK, int *INFO);

/* Inverse of a symmetric matrix */
void dsytri_(char *UPLO, int *N, double *A, int *LDA, int *IPIV,
             double *WORK, int *INFO, int lenUPLO);
void ssytri_(char *UPLO, int *N, float *A, int *LDA, int *IPIV,
             float *WORK, int *INFO, int lenUPLO);

/* Inverse of a symmetric positive definite matrix */
void dpotri_(char *UPLO, int *N, double *A, int *LDA, int *INFO, int lenUPLO);
void spotri_(char *UPLO, int *N, float *A, int *LDA, int *INFO, int lenUPLO);

/* Singular value decomposition (SVD) */
void dgesvd_(char *JOBU, char *JOBVT, int *M, int *N,
             double *A, int *LDA, double *S,
             double *U, int *LDU, double *VT, int *LDVT,
             double *WORK, int *LWORK, int *INFO,
             int lenJOBU, int lenJOBVT);
void sgesvd_(char *JOBU, char *JOBVT, int *M, int *N,
             float *A, int *LDA, float *S,
             float *U, int *LDU, float *VT, int *LDVT,
             float *WORK, int *LWORK, int *INFO,
             int lenJOBU, int lenJOBVT);

/* Singular value decomposition using divide and conquer algorithm */
void dgesdd_(char *JOBZ, int *M, int *N, double *A, int *LDA,
             double *S, double *U, int *LDU,
             double *VT, int *LDVT, double *WORK, int *LWORK,
             int *IWORK, int *INFO, int lenJOBZ);
void sgesdd_(char *JOBZ, int *M, int *N, float *A, int *LDA,
             float *S, float *U, int *LDU,
             float *VT, int *LDVT, float *WORK, int *LWORK,
             int *IWORK, int *INFO, int lenJOBZ);

/* Constrained least squares */
void dgglse_(int *M, int *N, int *P, double *A, int *LDA,
             double *B, int *LDB, double *C, double *D,
             double *X, double *WORK, int *LWORK, int *INFO);
void sgglse_(int *M, int *N, int *P, float *A, int *LDA,
             float *B, int *LDB, float *C, float *D,
             float *X, float *WORK, int *LWORK, int *INFO);

/* Lapack auxiliary routine */
int ilaenv_(int *ISPEC, char *NAME, char *OPTS, int *N1, int *N2,
            int *N3, int *N4, int lenNAME, int lenOPTS);

#ifdef __cplusplus
}
#endif

#endif /* NPSTAT_LAPACK_H_ */
