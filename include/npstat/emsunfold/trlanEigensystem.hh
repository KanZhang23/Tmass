#ifndef EMSUNFOLD_TRLANEIGENSYSTEM_HH_
#define EMSUNFOLD_TRLANEIGENSYSTEM_HH_

/*!
// \file trlanEigensystem.hh
//
// \brief Determination of eigenvalues/vectors of covariance matrices with TRLAN
//
// Author: I. Volobouev
//
// July 2014
*/

#include <vector>

#include "npstat/emsunfold/EigenParameters.hh"

namespace emsunfold {
    /**
    // TRLAN diagnostic information, in a slightly more convenient form than
    // that provided by the Fortran 77 interface "trlan77".
    */
    class TrlanDiagnostics
    {
    public:
        /**
        // Default constructor creates a dummy object which is supposed
        // to be overwritten later.
        */
        TrlanDiagnostics();

        /**
        // The arguments of this constructor are as follows:
        //
        // tailFraction -- The fraction of eigenvalues not found. This is
        //                 (trace - (sum of eigenvalues found))/trace.
        //
        // ipar         -- input/output parameters of the trlan77 routine.
        */
        TrlanDiagnostics(double tailFraction, const int ipar[32]);

        /**
        // ((matrix trace) - (sum of eigenvalues found))/(matrix trace).
        // For the diagnostics object created by the default constructor,
        // this method returns -1.0.
        */
        inline double finalTailFraction() const {return tailFraction_;}

        /**
        // Error code returned by TRLAN. 0 means no error.
        // For the list of error codes, consult the TRLAN user guide.
        */
        inline int status() const {return ipar_[0];}

        /**
        // Number of eigenpairs actually determined. Note that it could be
        // less than the number requested.
        */
        inline int nConverged() const {return ipar_[3];}

        /** Number of Ritz pairs locked (with small residual norms) */
        inline int nLocked() const {return ipar_[23];}

        /** Actual number of matrix-vector multiplications performed */
        inline int nMatVec() const {return ipar_[24];}

        /** Number of restarting loops */
        inline int nRestart() const {return ipar_[25];}

        /**
        // Number of times the Gram-Schmidt orthogonalization has been applied
        */
        inline int nOrth() const {return ipar_[26];}

        /** Number of times initial random vectors were generated */
        inline int nRand() const {return ipar_[27];}

        /** Total TRLAN run time, in milliseconds */
        inline int tTotal() const {return ipar_[28];}

        /** Time spent multiplying matrix by vectors, in ms */
        inline int tMatVec() const {return ipar_[29];}

        /** Time spent re-orthogonalizing, in ms */
        inline int tOrth() const {return ipar_[30];}

        /** Time spent in restarting and Rayleigh-Ritz projections, in ms */
        inline int tRestart() const {return ipar_[31];}

    private:
        double tailFraction_;
        int ipar_[32];
    };

    /**
    // Determine eigenvalues and eigenvectors of the argument covariance
    // matrix using TRLAN, steered by the parameters given. This function
    // returns the TRLAN error code (status). 0 means everything is OK.
    // Consult the TRLAN user guide for the meaning of other error codes.
    // Note that, even when 0 is returned, the number of eigenpairs actually
    // found could be less than the number of eigenpairs requested and/or
    // tail fraction stopping condition might not be satisfied.
    //
    // The eigenvalues will be returned in the increasing order. Check
    // diagnostics->nConverged() to see how many of them are returned.
    // Note that the vector of eigenvalues can actually have more elements,
    // but only "diagnostics->nConverged()" of them are valid. Eigenvectors
    // will be placed into "eigenvectors" vector one after another. If the
    // matrix "covmat" has "nrows" rows then &eigenvectors[0] will point to
    // the first eigenvector, &eigenvectors[0] + nrows to the second, etc.
    // The order of eigenvectors corresponds to the order of eigenvalues.
    */
    template <class Matrix>
    int trlanEigensystem(const Matrix& covmat, const EigenParameters& params,
                         std::vector<double>* eigenvalues,
                         std::vector<double>* eigenvectors,
                         TrlanDiagnostics* diagnostics);
}

std::ostream& operator<<(std::ostream& os, const emsunfold::TrlanDiagnostics& d);

#include "npstat/emsunfold/trlanEigensystem.icc"

#endif // EMSUNFOLD_TRLANEIGENSYSTEM_HH_
