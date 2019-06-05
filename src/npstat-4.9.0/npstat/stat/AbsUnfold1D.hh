#ifndef NPSTAT_ABSUNFOLD1D_HH_
#define NPSTAT_ABSUNFOLD1D_HH_

/*!
// \file AbsUnfold1D.hh
//
// \brief Interface definition for 1-d unfolding algorithms
//
// Author: I. Volobouev
//
// February 2014
*/

#include <vector>
#include <utility>

#include "npstat/nm/Matrix.hh"

namespace npstat {
    class LocalPolyFilter1D;

    class AbsUnfold1D
    {
    public:
        /**
        // Number of rows of the response matrix should be equal
        // to the number of discretization intervals in the 
        // space of observations. Number of columns should be equal
        // to the number of intervals in the original (unfolded) space.
        */
        explicit AbsUnfold1D(const Matrix<double>& responseMatrix);

        inline virtual ~AbsUnfold1D() {}

        inline const Matrix<double>& responseMatrix() const
            {return responseMatrix_;}

        inline const Matrix<double>& transposedResponseMatrix() const
            {return responseMatrixT_;}

        inline const std::vector<double>& efficiency() const
            {return efficiency_;}

        //@{
        /**
        // The first element of the pair is the entropic nDoF and
        // the second is the nDoF based on the trace
        */
        std::pair<double, double> smoothingNDoF() const;
        std::pair<double, double> responseNDoF() const;
        std::pair<double, double> smoothedResponseNDoF() const;
        //@}

        /** Set the initial approximation to the unfolded solution */
        virtual void setInitialApproximation(const double* approx,
                                             unsigned lenApprox);

        /** Clear the initial approximation to the unfolded solution */
        virtual void clearInitialApproximation();

        /** Return the initial approximation to the unfolded solution */
        virtual const std::vector<double>& getInitialApproximation() const;

        /**
        // Set the smoothing filter used. The filter will not be copied.
        // The user must ensure that the filter exists while this object
        // is in use.
        */
        virtual void setFilter(const LocalPolyFilter1D* f);

        /** Retrieve the smoothing filter used */
        virtual const LocalPolyFilter1D* getFilter(bool throwIfNull=false) const;

        /** Switch between using filtering or convolution */
        inline virtual void useConvolutions(const bool b) {useConvolutions_ = b;}

        /** Check if the filter should use "filter" or "convolve" method */
        inline bool usingConvolutions() const {return useConvolutions_;}

        /**
        // Method to be implemented by derived classes. "lenObserved"
        // and "lenUnfolded" should be consistent with the dimensionality
        // of the response matrix provided in the constructor. The
        // "observationCovarianceMatrix" pointer to the covariance matrix
        // of observations can be NULL in which case this covariance
        // matrix will be evaluated internally (typically, assuming
        // either Poisson or multinomial distributions for the observed
        // counts). "unfoldedCovarianceMatrix" pointer, if not NULL,
        // should be used to return the covariance matrix of unfolded
        // values with dimensions (lenUnfolded x lenUnfolded). This
        // function should return "true" on success and "false" on failure.
        */
        virtual bool unfold(const double* observed, unsigned lenObserved,
                            const Matrix<double>* observationCovarianceMatrix,
                            double* unfolded, unsigned lenUnfolded,
                            Matrix<double>* unfoldedCovarianceMatrix) = 0;

        /** L1 distance between two unnormalized distributions */
        static double probDelta(const double* prev, const double* next,
                                const unsigned len);

        /** 
        // Procedure for building covariance matrix of observations
        // according to either Poisson or multinomial distribution
        */
        static Matrix<double> observationCovariance(
            const double* yhat, unsigned lenObserved, bool isMultinomial);

    protected:
        // Build uniform initial approximation to the unfolded solution.
        // This is useful if the initial approximation was not set explicitly.
        void buildUniformInitialApproximation(const double* observed,
                                              unsigned lenObserved,
                                              std::vector<double>* result) const;

        // The following function will throw the std::invalid_argument
        // exception if the dimensions are incompatible with those of the
        // response matrix
        void validateDimensions(unsigned lenObserved, unsigned lenUnfold) const;

        // The following function will throw the std::invalid_argument
        // exception if the input array has negative entries or if all
        // values are 0.
        static void validateDensity(const double* observ,
                                    unsigned lenObserved);
    private:
        // Disable copy and assignment because using the same
        // filter by two different objects of this type is not
        // necessarily cool
        AbsUnfold1D();
        AbsUnfold1D(const AbsUnfold1D&);
        AbsUnfold1D& operator=(const AbsUnfold1D&);

        const Matrix<double> responseMatrix_;
        const Matrix<double> responseMatrixT_;
        std::vector<double> efficiency_;
        std::vector<double> initialApproximation_;
        const LocalPolyFilter1D* filt_;
        bool useConvolutions_;
    };
}

#endif // NPSTAT_ABSUNFOLD1D_HH_
