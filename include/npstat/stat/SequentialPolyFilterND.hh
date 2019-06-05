#ifndef NPSTAT_SEQUENTIALPOLYFILTERND_HH_
#define NPSTAT_SEQUENTIALPOLYFILTERND_HH_

/*!
// \file SequentialPolyFilterND.hh
//
// \brief Sequential local polynomial filtering (regression) on uniform
//        hyperrectangular grids
//
// Author: I. Volobouev
//
// November 2009
*/

#include "geners/ClassId.hh"
#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/Matrix.hh"

#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/AbsPolyFilterND.hh"

namespace npstat {
    /**
    // This class performs local polynomial filtering in multiple dimensions
    // by sequential application of one-dimensional filters
    */
    class SequentialPolyFilterND : public AbsPolyFilterND
    {
    public:
        /**
        // Main constructor. The arguments are as follows:
        //
        //  filters       -- Array of pointers to LocalPolyFilter1D
        //                   objects which will be used to perform
        //                   filtering in each dimension. The filters
        //                   should be compatible with the expected
        //                   span of the data arrays.
        //
        //  nFilters      -- Number of elements in the "filters" array.
        //                   Should be equal to the dimensionality of
        //                   the data arrays which will be processed
        //                   by the "filter" function.
        //
        //  takeOwnership -- If this argument is set to "true", we will
        //                   call the "delete" operator on each element
        //                   of the "filters" array the destructor.
        */
        SequentialPolyFilterND(const LocalPolyFilter1D** filters,
                               unsigned nFilters, bool takeOwnership);
        virtual ~SequentialPolyFilterND();

        bool operator==(const SequentialPolyFilterND& r) const;
        inline bool operator!=(const SequentialPolyFilterND& r) const
            {return !(*this == r);}

        //@{
        /** Inspect object properties */
        inline unsigned dim() const {return nFilters_;}
        inline bool takesOwnership() const {return takeOwnership_;}
        ArrayShape dataShape() const;
        //@}

        /** Get the filter for the given dimension */
        const LocalPolyFilter1D* filter(unsigned dimNumber) const;

        /**
        // Get the effective multivariate filter coefficients for the
        // given grid point. Note that this operation is quite slow
        // (it involves building a multivariate array using outer products
        // of 1-d arrays) and should not be used when fast performance
        // is essential.
        */
        ArrayND<double> getFilter(const unsigned* index,
                                  unsigned lenIndex) const;

        /** Get the complete effective filter matrix */
        Matrix<double> getFilterMatrix() const;

        /** Get the info needed to construct the sparse filter matrix */
        template <class Triplet>
        CPP11_auto_ptr<std::vector<Triplet> > sparseFilterTriplets() const;

        /** Check compatibility of an array with the filter */
        template <typename T, unsigned StackLen, unsigned StackDim>
        bool isCompatible(const ArrayND<T,StackLen,StackDim>& in) const;

        //@{
        /**
        // Contribution of a single point into the density estimate
        // at that point (not normalized). This is needed for various
        // leaving-one-out cross validation procedures.
        */
        double selfContribution(const unsigned* index,
                                unsigned lenIndex) const;
        double linearSelfContribution(unsigned long index) const;
        //@}

        /** This method performs the filtering */
        template <typename T1, unsigned StackLen, unsigned StackDim,
                  typename T2, unsigned StackLen2, unsigned StackDim2>
        void filter(const ArrayND<T1,StackLen,StackDim>& in,
                    ArrayND<T2,StackLen2,StackDim2>* out) const;

        /**
        // A diffent filtering method in which the shapes of the
        // kernels are determined by the positions of the "sources"
        // (i.e., sample points) instead of the positions at which
        // the density (or response) is estimated. Note that elements
        // of "out" array themselves are used as result accumulators.
        */
        template <typename T1, unsigned StackLen, unsigned StackDim,
                  typename T2, unsigned StackLen2, unsigned StackDim2>
        void convolve(const ArrayND<T1,StackLen,StackDim>& in,
                      ArrayND<T2,StackLen2,StackDim2>* out) const;

        // Methods needed for I/O
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::SequentialPolyFilterND";}
        static inline unsigned version() {return 1;}
        static SequentialPolyFilterND* read(const gs::ClassId& id,
                                            std::istream& in);
    private:
        SequentialPolyFilterND();
        SequentialPolyFilterND(const SequentialPolyFilterND&);
        SequentialPolyFilterND& operator=(const SequentialPolyFilterND&);

        const LocalPolyFilter1D** filters_;
        unsigned nFilters_;
        std::vector<unsigned long> strides_;
        mutable std::vector<unsigned> indexBuf_;
        bool takeOwnership_;

        template <typename T2, unsigned StackLen2, unsigned StackDim2>
        void filterLoop(bool useConvolve, unsigned level, unsigned long idx,
                        unsigned dimNumber, T2* inbuf, T2* outbuf,
                        ArrayND<T2,StackLen2,StackDim2>* out) const;
    };
}

#include "npstat/stat/SequentialPolyFilterND.icc"

#endif // NPSTAT_SEQUENTIALPOLYFILTERND_HH_
