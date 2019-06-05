#ifndef EMSUNFOLD_PRUNECOVARIANCE_HH_
#define EMSUNFOLD_PRUNECOVARIANCE_HH_

/*!
// \file pruneCovariance.hh
//
// \brief Prune small correlation coefficients in a covariance matrix
//
// Author: I. Volobouev
//
// July 2014
*/

#include <vector>

#include "geners/CPP11_auto_ptr.hh"
#include "Eigen/SparseCore"

namespace emsunfold {
    /**
    // This function will prune off-diagonal elements of a sparse
    // symmetric positive-semidefinite matrix which correspond to
    // correlation coefficients smaller in magnitude than "tol".
    // This function will also make sure that the correlation
    // coefficients do not exceed 1 in magnitude.
    */
    template<class Matrix>
    CPP11_auto_ptr<std::vector<Eigen::Triplet<double,typename Matrix::Index> > >
    pruneCovariance(const Matrix& m, double tol);
}

#include "npstat/emsunfold/pruneCovariance.icc"

#endif // EMSUNFOLD_PRUNECOVARIANCE_HH_
