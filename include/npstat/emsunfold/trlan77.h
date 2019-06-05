#ifndef EMSUNFOLD_TRLAN77_H_
#define EMSUNFOLD_TRLAN77_H_

/*!
// \file trlan77.h
//
// \brief Declaration of TRLAN interface usable from C or C++
//
// Author: I. Volobouev
//
// July 2014
*/

#ifdef __cplusplus
extern "C" {
#endif
    /*
    // "trlan77" is a Fortran 77 interface to "trlan" (written in F90).
    // For the description of "trlan" and, in particular, for more details
    // about various parameters that control the algorithm behavior, see
    //   http://crd-legacy.lbl.gov/~kewu/ps/trlan_.html#SEC13
    // The arguments of the "trlan77_" routine are as follows:
    //
    //   op     -- function which applies the operator (i.e., matrix) whose
    //             eigenvalues and eigenvectors are being calculated to the
    //             input vector. Note that "trlan77_" does not need the
    //             matrix itself as an argument because the algorithm does
    //             not access individual matrix elements. The arguments of
    //             "op" are as follows:
    //                nrows -- The number of rows on this processor if
    //                         using MPI, otherwise the total number of
    //                         rows in a Lanczos vector.
    //                ncols -- The number of vectors to be multiplied.
    //                xin   -- Input vectors, in columnwise order.
    //                ldx   -- "Leading dimension" of array xin. Should,
    //                         be at least as large as nrows.
    //                yout  -- The array to store multiplication results.
    //                ldy   -- "Leading dimension" of array yout.
    //
    //   ipar   -- input/output parameters for the algorithm. Mapping of
    //             this array to TRL_INFO_T is as follows ("I" stands for
    //             input parameter and "O" for output):
    //
    //              O  ipar[0]  = status, the error flag. 0 means no error,
    //                                    but it is possible that "trlan" has
    //                                    not computed all wanted eigenpairs.
    //                                    Check the value of "nec" (ipar[3]).
    //                                    If it is smaller than "ned", increase
    //                                    maxlan and/or increase maxmv and/or
    //                                    use a different restarting strategy.
    //                                    For the meaning of other status
    //                                    values, see the online manual.
    //             I   ipar[1]  = lohi,   which end of the eigenspectrum to
    //                                    compute. >0 means compute the largest
    //                                    eigenvalues and corresponding vectors.
    //             I   ipar[2]  = ned,    number of eigenvalues/vectors desired.
    //             IO  ipar[3]  = nec,    number of eigenvalues/vectors computed
    //                                    (or available on input).
    //             I   ipar[4]  = maxlan, maximum Lanczos basis size. This
    //                                    parameter determines the maximum
    //                                    memory requirement of "trlan".
    //                                    maxlan + 1 vectors will be stored
    //                                    internally, plus maxlan*(maxlan+10)
    //                                    additional memory will be needed
    //                                    for internal computations.
    //             I   ipar[5]  = restart, which thick-restart scheme to use
    //                                     (1, 2, 3, 4, 5). See online manual
    //                                     for interpretation of these values.
    //             I   ipar[6]  = maxmv,  the maximum number of matrix-vector
    //                                    multiplications allowed.
    //             I   ipar[7]  = mpicom, the MPI communicator to be used.
    //                                    Ignored if MPI is not used.
    //             I   ipar[8]  = verbose, if < 0, do not generate debugging
    //                                    outputs. Numbers between 0 and 10
    //                                    increase the amount of debugging
    //                                    information printed.
    //             I   iapr(9]  = log_io, the Fortran I/O unit number to be
    //                                    used for writing debug information.
    //             I   ipar[10] = iguess, tells "trlan" how to generate the
    //                                    initial guess vector. The meaning
    //                                    is as follows:
    //                                    < 0: use [1, 1, ..., 1] plus random
    //                                         perturbations,
    //                                      0: use [1, 1, ..., 1],
    //                                      1: starting vector is user-supplied,
    //                                    > 1: read a checkpoint file and use
    //                                         its content to start the Lanczos
    //                                         process.
    //             I   ipar[11] = cpflag, if > 0, use checkpoints.
    //             I   ipar[12] = cpio,   Fortran I/O unit number to be used
    //                                    for writing checkpoint files.
    //             I   ipar[13] = mvflop, the number of floating-point
    //                                    operations performed while completing
    //                                    one matrix-vector multiplication.
    //                                    Used to compute MFLOPS rate of the
    //                                    program.
    //              O  ipar[23] = locked, number of Ritz pairs that have
    //                                    extremely small residual norms.
    //                                    "trlan" locks not only the wanted
    //                                    eigenpairs with small residual norm,
    //                                    it also locks unwanted ones depending
    //                                    on the restarting strategy.
    //              O  ipar[24] = matvec, actual number of matrix-vector
    //                                    multiplications, which is also
    //                                    the number of iterations.
    //              O  ipar[25] = nloop,  number of restarting loops, i.e.,
    //                                    the number of times "trlan" has
    //                                    reached maximum size and restarted.
    //              O  ipar[26] = north,  number of times the Gram-Schmidt
    //                                    orthogonalization has been applied.
    //              O  ipar[27] = nrand,  number of times "trlan" has generated
    //                                    random vectors in an attempt to
    //                                    produce a vector that is orthogonal
    //                                    to the current basis vectors. 
    //              O  ipar[28] = total time in milliseconds.
    //              O  ipar[29] = matrix times vector time in milliseconds.
    //              O  ipar[30] = re-orthogonalization time in milliseconds.
    //              O  ipar[31] = time spent in restarting (including
    //                            performing Rayleigh-Ritz projections).
    //
    //             Note that in this description array indices start with 0
    //             and differ by 1 from those in the "trlan" manual. All
    //             values of ipar not set explicitly on input should be
    //             initialized to 0.
    //
    //   nrow   -- The number of rows on this processor if the problem is
    //             distributed using MPI, else the number of total rows in
    //             a Lanczos vector. 
    //
    //   mev    -- The number of elements in the array "eval" and the number
    //             of columns in the array "evec".
    //
    //   eval   -- Array of eigenvalues. If "ipar[3] = nec" is not 0, the
    //             first "nec" elements should contain, on input, eigenvalues
    //             already computed.
    //
    //   evec   -- Array of eigenvectors. Note that, even if the number of
    //             desired eigenvectors is smaller than "mev", the array
    //             must still contain enough space for "nrow" x "mev" elements
    //             (it is used by the algorithm to store Lanczos vectors).
    //             If "ipar[3] = nec" is not 0, it should contain, on input,
    //             the first "nec" eigenvectors already computed.
    //
    //   lde    -- The leading dimension of array evec (must be >= nrow).
    //
    //   wrk    -- Workspace. On input, wrk[0] MUST contain the tolerance
    //             parameter. On output, wrk[0:nec-1] will contain the
    //             residual norms, and wrk[nec] will contain "crat" (the
    //             convergence factor -- see online manual for details).
    //
    //   lwrk   -- Workspace length. To accommodate everything, use
    //             (maxlan + 1 - mev) * nrow + maxlan * (maxlan + 10),
    //             where maxlan = ipar[4]. In case mev >= maxlan + 1
    //             or in case you want to allow "trlan" to allocate
    //             its own memory, just use maxlan * (maxlan + 10).
    */
    void trlan77_(void(*op)(const int* nrows,  const int* ncols,
                            const double* xin, const int* ldx,
                            double* yout, const int* ldy),
                  int ipar[32], const int* nrow, const int* mev, double* eval,
                  double* evec, const int* lde, double* wrk, const int* lwrk);

#ifdef __cplusplus
}
#endif

#endif /* EMSUNFOLD_TRLAN77_H_ */
