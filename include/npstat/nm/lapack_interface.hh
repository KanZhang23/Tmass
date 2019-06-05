#ifndef NPSTAT_LAPACK_INTERFACE_HH_
#define NPSTAT_LAPACK_INTERFACE_HH_

/*!
// \file lapack_interface.hh
//
// \brief Template interface to LAPACK (in its original F77 implementation)
//
// Typically, generic templates will throw an appropriate exception,
// while templates specialized for float, double, etc. will perform
// the requested calculations via corresponding LAPACK routines.
//
// This code is rather low-level. It may be more convenient to use
// the API provided by the Matrix class which will eventually call
// appropriate LAPACK interface functions from here.
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/nm/GeneralizedComplex.hh"

namespace npstat {
    /**
    // Invert a positive definite symmetric matrix
    // (LAPACK DPOTRF/DPOTRI routines are used for doubles)
    */
    template<typename Numeric>
    void invert_posdef_sym_matrix(const Numeric* in, unsigned dim,
                                  Numeric* out);

    /**
    // Invert a symmetric matrix
    // (LAPACK DSYTRF/DSYTRI routines are used for doubles)
    */
    template<typename Numeric>
    void invert_sym_matrix(const Numeric* in, unsigned dim, Numeric* out);

    /**
    // Invert a general matrix
    // (LAPACK DGETRF/DGETRI routines are used for doubles)
    */
    template<typename Numeric>
    void invert_general_matrix(const Numeric* in, unsigned dim, Numeric* out);

    /**
    // Determine eigenvalues of a general matrix
    // (LAPACK DGEEV routine is used for doubles)
    */
    template<typename Numeric>
    void gen_matrix_eigenvalues(const Numeric* in, unsigned dim,
                                typename GeneralizedComplex<Numeric>::type 
                                *eigenvalues);

    /**
    // Determine eigenvalues of a symmetric matrix
    // (LAPACK DSYEV routine is used for doubles)
    */
    template<typename Numeric>
    void sym_matrix_eigenvalues(const Numeric* in, unsigned dim,
                                Numeric* eigenvalues);

    template<typename Numeric>
    void td_sym_matrix_eigenvalues(const Numeric* in, unsigned dim,
                                   Numeric* eigenvalues);

    /**
    // Determine eigenvalues of a symmetric matrix using the divide
    // and conquer LAPACK driver (DSYEVD routine is used for doubles)
    */
    template<typename Numeric>
    void sym_matrix_eigenvalues_dc(const Numeric* in, unsigned dim,
                                   Numeric* eigenvalues);

    template<typename Numeric>
    void td_sym_matrix_eigenvalues_dc(const Numeric* in, unsigned dim,
                                      Numeric* eigenvalues);

    /**
    // Determine eigenvalues of a symmetric matrix using the
    // "Relatively Robust Representations" (RRR) LAPACK driver
    // (DSYEVR routine is used for doubles)
    */
    template<typename Numeric>
    void sym_matrix_eigenvalues_rrr(const Numeric* in, unsigned dim,
                                    Numeric* eigenvalues);

    /**
    // Determine eigenvalues of a symmetric tridiagonal matrix using
    //  the "Relatively Robust Representations" (RRR) LAPACK driver
    // (DSTEVR routine is used for doubles)
    */
    template<typename Numeric>
    void td_sym_matrix_eigenvalues_rrr(const Numeric* in, unsigned dim,
                                       Numeric* eigenvalues);

    /**
    // Determine eigenvalues and eigenvectors of a general matrix
    // (LAPACK DGEEV routine is used for doubles)
    */
    template<typename Numeric>
    void gen_matrix_eigensystem(const Numeric* in, unsigned dim,
                                typename GeneralizedComplex<Numeric>::type *ev,
                                Numeric* rightEigenvectors,
                                Numeric* leftEigenvectors);

    /**
    // Determine eigenvalues and eigenvectors of a symmetric matrix
    // (LAPACK DSYEV routine is used for doubles)
    */
    template<typename Numeric>
    void sym_matrix_eigensystem(const Numeric* in, unsigned dim,
                                Numeric* eigenvalues,
                                Numeric* eigenvectors);

    template<typename Numeric>
    void td_sym_matrix_eigensystem(const Numeric* in, unsigned dim,
                                   Numeric* eigenvalues,
                                   Numeric* eigenvectors);

    /**
    // Determine eigenvalues and eigenvectors of a symmetric matrix
    // using the divide and conquer LAPACK driver (DSYEVD routine
    // is used for doubles)
    */
    template<typename Numeric>
    void sym_matrix_eigensystem_dc(const Numeric* in, unsigned dim,
                                   Numeric* eigenvalues,
                                   Numeric* eigenvectors);

    template<typename Numeric>
    void td_sym_matrix_eigensystem_dc(const Numeric* in, unsigned dim,
                                      Numeric* eigenvalues,
                                      Numeric* eigenvectors);

    /**
    // Determine eigenvalues and eigenvectors of a symmetric matrix
    // using the "Relatively Robust Representations" (RRR) LAPACK
    // driver (DSYEVR routine is used for doubles)
    */
    template<typename Numeric>
    void sym_matrix_eigensystem_rrr(const Numeric* in, unsigned dim,
                                    Numeric* eigenvalues,
                                    Numeric* eigenvectors);

    template<typename Numeric>
    void td_sym_matrix_eigensystem_rrr(const Numeric* in, unsigned dim,
                                       Numeric* eigenvalues,
                                       Numeric* eigenvectors);

    /**
    // Solve a linear system (LAPACK DGETRF/DGETRS routines are used
    // for doubles). This function returns "true" on success or "false"
    // in case the matrix is degenerate.
    */
    template<typename Numeric>
    bool solve_linear_system(const Numeric* in, unsigned dim,
                             const Numeric* rhs, Numeric* solution);

    /**
    // Solve multiple linear systems (LAPACK DGETRF/DGETRS routines are
    // used for doubles). This function returns "true" on success or "false"
    // in case the matrix is degenerate.
    */
    template<typename Numeric>
    bool solve_linear_systems(const Numeric* in, unsigned nrows,
                              unsigned ncols, const Numeric* rhs,
                              Numeric* solution);

    /**
    // Solve an overdetermined linear system in the least squares sense
    // (DGELSD is used for doubles). This function returns "true" on
    // success or "false" on failure.
    */
    template<typename Numeric>
    bool linear_least_squares(const Numeric* mat, unsigned nrows,
                              unsigned ncols, const Numeric* rhs,
                              Numeric* solution);

    /**
    // Constrained least squares problem (DGGLSE is used for doubles).
    // "true" is returned on success and "false" on failure.
    */
    template<typename Numeric>
    bool constrained_least_squares(const Numeric* mat, unsigned nrows,
                                   unsigned ncols, const Numeric* rhs,
                                   const Numeric* constraintMatrix,
                                   unsigned constrRows, unsigned constrCols,
                                   const Numeric* constrRhs, Numeric* solution);

    /**
    // Singular value decomposition (by DGESVD, etc) of M x N matrix.
    // This function returns "true" on success or "false" on failure.
    */
    template<typename Numeric>
    bool gen_matrix_svd(const Numeric* in, unsigned M, unsigned N,
                        Numeric* U, Numeric* singularValues, Numeric* VT);

    /**
    // Divide and conquer SVD (by DGESDD, etc) of M x N matrix.
    // This function returns "true" on success or "false" on failure.
    */
    template<typename Numeric>
    bool gen_matrix_svd_dc(const Numeric* in, unsigned M, unsigned N,
                           Numeric* U, Numeric* singularValues, Numeric* VT);

    namespace Private {
        int lapack_nlvl_dgelsd(int dim);
        int lapack_nlvl_sgelsd(int dim);
    }
}

#include "npstat/nm/lapack_interface.icc"

#endif // NPSTAT_LAPACK_INTERFACE_HH_
