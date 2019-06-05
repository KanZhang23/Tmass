#ifndef NPSTAT_ABSUNFOLDND_HH_
#define NPSTAT_ABSUNFOLDND_HH_

/*!
// \file AbsUnfoldND.hh
//
// \brief Interface definition for multivariate unfolding algorithms
//
// Author: I. Volobouev
//
// June 2014
*/

#include "npstat/stat/ResponseMatrix.hh"
#include "npstat/stat/AbsUnfoldingFilterND.hh"

namespace npstat {
    class AbsUnfoldND
    {
    public:
        typedef ResponseMatrix response_matrix_type;
        typedef Matrix<double> input_covariance_type;
        typedef Matrix<double> output_covariance_type;
        typedef AbsUnfoldingFilterND filter_type;

        explicit AbsUnfoldND(const ResponseMatrix& responseMatrix);

        inline virtual ~AbsUnfoldND() {}

        inline const ResponseMatrix& responseMatrix() const
            {return responseMatrix_;}

        inline const ArrayND<double>& efficiency() const
            {return efficiency_;}

        /** Set the initial approximation to the unfolded solution */
        virtual void setInitialApproximation(const ArrayND<double>& approx);

        /** Clear the initial approximation to the unfolded solution */
        virtual void clearInitialApproximation();

        /** Return the initial approximation to the unfolded solution */
        virtual const ArrayND<double>& getInitialApproximation() const;

        /**
        // Set the smoothing filter used. The filter will not be copied.
        // The user must ensure that the filter exists while this object
        // is in use.
        */
        virtual void setFilter(const AbsUnfoldingFilterND* f);

        /** Retrieve the smoothing filter used */
        virtual const AbsUnfoldingFilterND* getFilter(
            bool throwIfNull = false) const;

        /** Switch between using filtering or convolution */
        inline virtual void useConvolutions(const bool b) {useConvolutions_=b;}

        /** Check if the filter should use "filter" or "convolve" method */
        inline bool usingConvolutions() const {return useConvolutions_;}

        /** Shape of the expected observed input */
        inline ArrayShape getObservedShape() const
            {return responseMatrix_.observedShape();}

        /** Shape of the expected unfolded output */
        inline ArrayShape getUnfoldedShape() const
            {return responseMatrix_.shape();}

        /** 
        // Method to be implemented by derived classes. The covariance
        // matrix of observations should assume linear ordering of the
        // observed data, per ordering by the "ArrayND" class. If the
        // "observationCovarianceMatrix" pointer is NULL, the matrix
        // should be constructed internally, assuming Poisson or multinomial
        // statistics. The "unfoldedCovarianceMatrix" pointer can be NULL
        // as well in which case the corresponding matrix should not be
        // calculated. This function should return "true" on success,
        // "false" on failure.
        */
        virtual bool unfold(const ArrayND<double>& observed,
                            const Matrix<double>* observationCovarianceMatrix,
                            ArrayND<double>* unfolded,
                            Matrix<double>* unfoldedCovarianceMatrix) = 0;
        //@{
        /**
        // This function will throw the "std::invalid_argument" exception if
        // the dimensions are incompatible with those of the response matrix
        */
        void validateUnfoldedShape(const ArrayND<double>& unfolded) const;
        void validateUnfoldedShape(const ArrayShape& unfoldedShape) const;
        void validateObservedShape(const ArrayND<double>& observed) const;
        void validateObservedShape(const ArrayShape& observedShape) const;
        //@}

        /** L1 distance between two unnormalized distributions */
        static double probDelta(const ArrayND<double>& prev,
                                const ArrayND<double>& next);
    protected:
        // Build uniform initial approximation to the unfolded solution.
        // This is useful if the initial approximation was not set explicitly.
        void buildUniformInitialApproximation(const ArrayND<double>& observed,
                                              ArrayND<double>* result) const;
    private:
        // Disable copy and assignment because using the same
        // filter by two different objects of this type is not
        // necessarily cool
        AbsUnfoldND();
        AbsUnfoldND(const AbsUnfoldND&);
        AbsUnfoldND& operator=(const AbsUnfoldND&);

        const ResponseMatrix responseMatrix_;
        ArrayND<double> efficiency_;
        ArrayND<double> initialApproximation_;
        const AbsUnfoldingFilterND* filt_;
        bool useConvolutions_;
    };
}

#endif // NPSTAT_ABSUNFOLDND_HH_
