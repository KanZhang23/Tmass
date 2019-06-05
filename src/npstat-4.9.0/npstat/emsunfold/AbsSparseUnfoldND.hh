#ifndef EMSUNFOLD_ABSSPARSEUNFOLDND_HH_
#define EMSUNFOLD_ABSSPARSEUNFOLDND_HH_

/*!
// \file AbsSparseUnfoldND.hh
//
// \brief Interface definition for multivariate unfolding algorithms that
//        use sparse matrices
//
// Author: I. Volobouev
//
// July 2014
*/

#include "npstat/emsunfold/AbsSparseUnfoldingFilterND.hh"

#include "Eigen/Core"

namespace emsunfold {
    template<class Matrix>
    class AbsSparseUnfoldND
    {
    public:
        typedef Matrix response_matrix_type;
        typedef Eigen::DiagonalMatrix<double,Eigen::Dynamic> input_covariance_type;
        typedef Matrix output_covariance_type;
        typedef AbsSparseUnfoldingFilterND filter_type;

        AbsSparseUnfoldND(const response_matrix_type& responseMatrix,
                          const npstat::ArrayShape& unfoldedShape,
                          const npstat::ArrayShape& observedShape);

        AbsSparseUnfoldND(const response_matrix_type& responseMatrix,
                          const unsigned* unfoldedShape, unsigned unfoldedDim,
                          const unsigned* observedShape, unsigned observedDim);

        inline virtual ~AbsSparseUnfoldND() {}

        inline const response_matrix_type& responseMatrix() const
            {return responseMatrix_;}

        inline const npstat::ArrayND<double>& efficiency() const
            {return efficiency_;}

        /** Set the initial approximation to the unfolded solution */
        virtual void setInitialApproximation(const npstat::ArrayND<double>& a);

        /** Clear the initial approximation to the unfolded solution */
        virtual void clearInitialApproximation();

        /** Return the initial approximation to the unfolded solution */
        virtual const npstat::ArrayND<double>& getInitialApproximation() const;

        /**
        // Set the smoothing filter used. The filter will not be copied.
        // The user must ensure that the filter exists while this object
        // is in use.
        */
        virtual void setFilter(const filter_type* f);

        /** Retrieve the smoothing filter used */
        virtual const filter_type* getFilter(
            bool throwIfNull = false) const;

        /** Switch between using filtering or convolution */
        inline virtual void useConvolutions(const bool b) {useConvolutions_=b;}

        /** Check if the filter should use "filter" or "convolve" method */
        inline bool usingConvolutions() const {return useConvolutions_;}

        /** Shape of the expected observed input */
        inline npstat::ArrayShape getObservedShape() const
            {return observedShape_;}

        /** Shape of the expected unfolded output */
        inline npstat::ArrayShape getUnfoldedShape() const
            {return unfoldedShape_;}

        /** 
        // Method to be implemented by derived classes. The covariance
        // matrix of observations should assume linear ordering of the
        // observed data, per ordering by the "ArrayND" class. If the
        // "observationCovarianceMatrix" pointer is NULL, the matrix
        // should be constructed internally, assuming Poisson statistics.
        // The "unfoldedCovarianceMatrix" pointer can be NULL as well in
        // which case the corresponding matrix should not be calculated.
        // If it is not NULL, the resulting matrix should be pruned.
        //
        // This function should return "true" on success, "false" on failure.
        */
        virtual bool unfold(
            const npstat::ArrayND<double>& observed,
            const input_covariance_type* observationCovarianceMatrix,
            npstat::ArrayND<double>* unfolded,
            output_covariance_type* unfoldedCovarianceMatrix) = 0;

        //@{
        /**
        // This function will throw the "std::invalid_argument" exception if
        // the dimensions are incompatible with those of the response matrix
        */
        void validateUnfoldedShape(const npstat::ArrayND<double>& u) const;
        void validateUnfoldedShape(const npstat::ArrayShape& uShape) const;
        void validateObservedShape(const npstat::ArrayND<double>& o) const;
        void validateObservedShape(const npstat::ArrayShape& oShape) const;
        //@}

    protected:
        // Build uniform initial approximation to the unfolded solution.
        // This is useful if the initial approximation was not set explicitly.
        void buildUniformInitialApproximation(const npstat::ArrayND<double>& o,
                                              npstat::ArrayND<double>* r) const;
    private:
        // Disable copy and assignment because using the same
        // filter by two different objects of this type is not
        // necessarily cool
        AbsSparseUnfoldND();
        AbsSparseUnfoldND(const AbsSparseUnfoldND&);
        AbsSparseUnfoldND& operator=(const AbsSparseUnfoldND&);

        void initialize();

        response_matrix_type responseMatrix_;
        const npstat::ArrayShape unfoldedShape_;
        const npstat::ArrayShape observedShape_;
        npstat::ArrayND<double> efficiency_;
        npstat::ArrayND<double> initialApproximation_;
        const filter_type* filt_;
        bool useConvolutions_;
    };
}

#include "npstat/emsunfold/AbsSparseUnfoldND.icc"

#endif // EMSUNFOLD_ABSSPARSEUNFOLDND_HH_
