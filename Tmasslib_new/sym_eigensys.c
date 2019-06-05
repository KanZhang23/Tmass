#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sym_eigensys.h"

#ifdef __cplusplus
extern "C" {
#endif

void dsyev_(char *JOBZ, char *UPLO, int *N, double *A, int *LDA, double *W,
            double *WORK, int *LWORK, int *INFO, int lenJOBZ, int lenUPLO);

void sym_eigensys(const double *data, const unsigned nrows,
		  double *eigenvectors, double *eigenvalues)
{
    static double *mem = 0;
    static int memsize = 0;
    const int NB = 32;
    char *UPLO = "L", *JOBZ = "V";
    int N, LDA, LWORK, INFO;
    unsigned row, col;

    assert(data);
    assert(nrows);
    assert(eigenvectors);
    assert(eigenvalues);

    /* Get the work memory */
    LWORK = (NB+2)*nrows;
    if (LWORK > memsize)
    {
	if (mem)
	    free(mem);
	mem = (double *)malloc(LWORK*sizeof(double));
	if (mem == NULL)
	{
	    fprintf(stderr, "Fatal error in sym_eigensys: out of memory. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	memsize = LWORK;
    }

    /* Symmetrize the matrix */
    for (row=0; row<nrows; ++row)
	for (col=0; col<=row; ++col)
  	    eigenvectors[row+nrows*col] = 0.5*(data[row+nrows*col] + data[col+nrows*row]);

    /* Get the eigenvectors and eigenvalues */
    N = nrows;
    LDA = nrows;
    dsyev_(JOBZ, UPLO, &N, eigenvectors, &LDA, eigenvalues, mem, &LWORK, &INFO, 1, 1);
    if (INFO < 0) assert(0); /* Bad arguments */
    if (INFO > 0) assert(0); /* Algorithm failed to converge */
}

#ifdef __cplusplus
}
#endif
