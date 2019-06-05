#ifndef EMSUNFOLD_EIGENPARAMETERS_HH_
#define EMSUNFOLD_EIGENPARAMETERS_HH_

/*!
// \file EigenParameters.hh
//
// \brief Parameters specifying how to search for eigenvalues/eigenvectors
//        of covariance matrices using TRLAN
//
// Author: I. Volobouev
//
// July 2014
*/

#include <iostream>

namespace emsunfold {
    /** Main parameters steering TRLAN behavior */
    class EigenParameters
    {
    public:
        /**
        // This constructor has the following aruments:
        //
        //  tolerance        -- This parameter determines how closely the
        //                      eigenvector equation must be satisfied.
        //                      If the matrix is A, eigenvalue lambda,
        //                      and eigenvector x, the condition is
        //                      || A x - lambda x || < tolerance * || A ||,
        //                      where || A || stands for the Frobenius
        //                      norm of A. If tolerance is 0 or negative,
        //                      a reasonable internal default will be used.
        //
        //  tailFraction     -- The code will attempt to determine the largest
        //                      eigenvalues of the given covariance matrix.
        //                      "tailFraction" is the stopping target: if
        //                      the sum of the calculated eigenvalues is
        //                      sufficiently close to the matrix trace, the
        //                      calculation will stop. "tailFraction" is the
        //                      ratio of the sum of eigenvalues not calculated
        //                      to the matrix trace.
        //
        //                      This cutoff affects entropy-based calculation
        //                      of the number of degrees of freedom (NDoF) for
        //                      covariance matrices. The relative precision
        //                      of NDoF calculation for an N x N matrix will
        //                      be roughly -tailFraction*log(tailFraction/N).
        //
        //  increaseRate     -- The code will increase the number of
        //                      calculated eigenpairs gradually, starting from
        //                      "minEigenvalues". This is the factor by which
        //                      the number of calculated eigenpairs is
        //                      increased during each subsequent attempt
        //                      to satisfy the "tailFraction" criterion.
        //                      This argument should probably be somewhere
        //                      between 1.1 and 1.5.
        //
        //  minEigenvalues   -- The minimum number of eigenpairs to calculate.
        //
        //  maxEigenvalues   -- The code will not attempt to calculate more
        //                      eigenpairs than "maxEigenvalues" even if the
        //                      "tailFraction" target is not satisfied.
        //
        //  lanczosBasisSize -- The size of the Lanczos basis. In principle,
        //                      the larger the basis size, the better the
        //                      algorithm will perform. It appears that making
        //                      the basis size twice larger than the number
        //                      of requested eigenpairs works really well.
        //                      The main limitation is the amount of computer
        //                      memory needed to store these basis vectors.
        //                      Also, the cost of re-orthogonalization starts
        //                      to contribute when the number of vectors
        //                      becomes really large. See comments in the TRLAN
        //                      user guide for more details about choosing the
        //                      value of this parameter.
        //
        //                      Note that "trlanEigensystem" function will
        //                      ignore this parameter if it's value is
        //                      unacceptably small. In particular, the
        //                      basis size will be set to the smaller of
        //                      2*(# of desired eigenvectors) and
        //                      (# of desired eigenvectors) + 6.
        //
        //  restartScheme    -- TRLAN restarting scheme: 1, 2, 3, 4, or 5.
        //                      See the TRLAN user guide for the meaning
        //                      of these arguments.
        //
        //  maxOperatorCalls -- Hard limit on the number of matrix-vector
        //                      multiplications in a single TRLAN run. The
        //                      TRLAN user guide suggests 1000 per eigenvalue
        //                      found (so something like 1000*maxEigenvalues).
        */
        EigenParameters(double tolerance, double tailFraction,
                        double increaseRate, int minEigenvalues,
                        int maxEigenvalues, int lanczosBasisSize,
                        int restartScheme, int maxOperatorCalls);

        //@{
        /** A simple inspector of object properties */
        inline double tolerance() const {return tol_;}
        inline double tailFraction() const {return tailFraction_;}
        inline double increaseRate() const {return increaseRate_;}
        inline int minEigenvalues() const {return minEigenvalues_;}
        inline int maxEigenvalues() const {return maxEigenvalues_;}
        inline int lanczosBasisSize() const {return maxlan_;}
        inline int restartScheme() const {return restartScheme_;}
        inline int maxOperatorCalls() const {return maxmv_;}
        //@}

    private:
        double tol_;
        double tailFraction_;
        double increaseRate_;
        int minEigenvalues_;
        int maxEigenvalues_;
        int maxlan_;
        int restartScheme_;
        int maxmv_;
    };
}

std::ostream& operator<<(std::ostream& os, const emsunfold::EigenParameters& p);

#include "npstat/emsunfold/EigenParameters.icc"

#endif // EMSUNFOLD_EIGENPARAMETERS_HH_
