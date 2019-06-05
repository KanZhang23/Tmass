#ifndef SYM_EIGENSYS_H_
#define SYM_EIGENSYS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* This is just a wrapper for the LAPACK subroutine DSYEV.
 * The matrix will be symmetrized internally if it is not
 * symmetric already. Do not use this in a multi-threaded code.
 *
 * "eigenvectors" must be nrows x nrows 2d array. On output
 * each row of this array will contain an eigenvector. 
 * The ascending eigenvalues are returned in "eigenvalues".
 * The order of eigenvectors corresponds to the order of
 * eigenvalues.
 */
void sym_eigensys(const double *symmetric_matrix, unsigned nrows,
		  double *eigenvectors, double *eigenvalues);

#ifdef __cplusplus
}
#endif

#endif /* SYM_EIGENSYS_H_ */
