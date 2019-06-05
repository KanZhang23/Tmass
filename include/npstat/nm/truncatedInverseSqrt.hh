#ifndef NPSTAT_TRUNCATEDINVERSESQRT_HH_
#define NPSTAT_TRUNCATEDINVERSESQRT_HH_

/*!
// \file truncatedInverseSqrt.hh
//
// \brief Utility for truncating square roots of symmetric
//        positive-semidefinite matrices
//
// Author: I. Volobouev
//
// July 2018
*/

#include "npstat/nm/Matrix.hh"

namespace npstat {
    /**
    // The following function constructs an inverse square root of
    // a symmetric positive semidefinite matrix (given by the "covmat"
    // argument) by keeping only a certain number of eigenvectors
    // corresponding to the largest eigenvalues. The number of
    // eigenvectors to keep is given by the "nEigenvectorsToKeep"
    // argument. The "result" matrix will have each _row_ set to a kept
    // eigenvector multiplied by the inverse square root of the
    // corresponding eigenvalue.
    //
    // If the egenvalue to keep is 0 or negative, it will be
    // converted into the product of the largest eigenvalue times
    // "eigenvaluePrecision". "eigenvaluePrecision" argument must
    // not be negative.
    //
    // The function returns the ratio of the sum of the rejected
    // eigenvalues of the "covmat" matrix to the total sum of its
    // eigenvalues. In this calculation, all negative eigenvalues
    // are assumed to be due to round-off, so they are converted to 0.
    //
    // std::invalid_argument will be thrown in case something is
    // wrong with the arguments (e.g., the input matrix is not square)
    // and if the largest eigenvalue is not positive.
    //
    // Intended for use in linear least squares problems with
    // degenerate covariance matrices when the "proper" number
    // of degrees of freedom is known in advance.
    //
    // If desired, the function can return the number of eigenvalues
    // affected by the "eigenvaluePrecision" cutoff.
    */
    double truncatedInverseSqrt(const Matrix<double>& covmat,
                                unsigned nEigenvectorsToKeep,
                                double eigenvaluePrecision,
                                Matrix<double>* result,
                                unsigned* numEigenvaluesAdjusted = 0,
                                EigenMethod m = EIGEN_SIMPLE);
}

#endif // NPSTAT_TRUNCATEDINVERSESQRT_HH_
