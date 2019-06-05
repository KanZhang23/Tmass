#ifndef NPSTAT_LOCALMULTIFILTER1D_HH_
#define NPSTAT_LOCALMULTIFILTER1D_HH_

/*!
// \file LocalMultiFilter1D.hh
//
// \brief Local polynomial filtering (regression) on 1-d equidistant grids
//
// Generates a collection of filters. Each filter in this collection
// corresponds to a polynomial of a certain degree from the orthogonal
// polynomial set, with degrees varying from 0 to the specified maximum.
// Intended mainly for numerical exploration of such filters.
//
// Author: I. Volobouev
//
// April 2015
*/

#include "geners/CPP11_auto_ptr.hh"
#include "geners/ClassId.hh"

#include "npstat/nm/Matrix.hh"

#include "npstat/stat/AbsFilter1DBuilder.hh"
#include "npstat/stat/BoundaryHandling.hh"

namespace npstat {
    class LocalMultiFilter1D
    {
    public:
        /**
        // Main constructor. The arguments are as follows:
        //
        //  maxDegree -- Maximum degree of the polynomials.
        //
        //  filterBuilder -- An instance of a class which actually builds
        //                   the filters when requested. This builder
        //                   is used only inside the constructor.
        //
        //  dataLen   -- The length of the data arrays which will be
        //               used with this filter (this info is needed in
        //               order to take into account the boundary effects).
        */
        LocalMultiFilter1D(unsigned maxDegree,
                           const OrthoPolyFilter1DBuilder& filterBuilder,
                           unsigned dataLen);

        LocalMultiFilter1D(const LocalMultiFilter1D&);
        LocalMultiFilter1D& operator=(const LocalMultiFilter1D&);

        ~LocalMultiFilter1D();

        bool operator==(const LocalMultiFilter1D& r) const;
        inline bool operator!=(const LocalMultiFilter1D& r) const
            {return !(*this == r);}

        //@{
        /** Inspect object properties */
        inline unsigned maxDegree() const {return maxDegree_;}
        inline unsigned dataLen() const {return nbins_;}
        //@}

        /** Get the filter coefficients for the given bin */
        const PolyFilter1D& getFilter(unsigned degree, unsigned binNumber) const;

        /**
        // This method performs the filtering. "dataLen",
        // which is the length of both "in" and "out" arrays,
        // must be the same as the one in the constructor.
        */
        template <typename Tin, typename Tout>
        void filter(unsigned degree, const Tin* in,
                    unsigned dataLen, Tout* out) const;

        /**
        // A diffent filtering method in which the shapes of the
        // kernels are determined by the positions of the "sources"
        // (i.e., sample points) instead of the positions at which
        // the density (or response) is estimated.
        */
        template <typename Tin, typename Tout>
        void convolve(unsigned degree, const Tin* in,
                      unsigned dataLen, Tout* out) const;

        /**
        // Generate the complete (non-sparse) representation of the filter.
        // It will be a generalized stochastic matrix (each row sums to 1).
        */
        Matrix<double> getFilterMatrix(unsigned degree) const;

        // Methods needed for I/O
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::LocalMultiFilter1D";}
        static inline unsigned version() {return 1;}
        static LocalMultiFilter1D* read(const gs::ClassId& id, std::istream& in);

    private:
        LocalMultiFilter1D();

        mutable std::vector<long double> sumBuffer_;
        std::vector<PolyFilter1D*> unique_;
        std::vector<PolyFilter1D**> bins_;
        unsigned nbins_;
        unsigned maxDegree_;

        template <typename T>
        double convolute(const T* data, const PolyFilter1D* filter) const;

        void addWeightedFilter(long double w, unsigned degree,
                               unsigned binNum) const;

        void clearSumBuffer() const;

        void releaseMem();
        void copyOtherData(const LocalMultiFilter1D&);

        // The following function will assume that is was
        // given a matrix filled with zeros as an argument
        void fillFilterMatrix(unsigned degree, Matrix<double>* fm) const;
    };

    /**
    // The utility function that generates the most common filters using
    // kernels from the symmetric beta family. The arguments are as follows:
    //
    //  m          -- Choose the kernel from the symmetric beta family
    //                proportional to (1 - x^2)^m. If m is negative,
    //                Gaussian kernel will be used, truncated at +- 12 sigma.
    //
    //  bandwidth  -- Kernel bandwidth.
    //
    //  maxDegree  -- Maximum degree of the LOrPE polynomial.
    //
    //  numberOfGridPoints -- Length of the data array to be used with this
    //                        filter (typically, number of histogram bins
    //                        and such)
    //
    //  xmin, xmax -- Data grid limits (as in a histogram).
    //
    //  boundaryMethod -- Method for handling the weight function at
    //                    the boundary of the density support region.
    //
    //  exclusionMask -- If provided, array with numberOfGridPoints elements.
    //                   If an element of this array is not 0, corresponding
    //                   data point will be excluded from the filtering process.
    //
    //  excludeCentralPoint -- If "true", the weight will be set to 0 for
    //                the central bin of the filter. This can be useful in
    //                some cross validation scenarios.
    */
    CPP11_auto_ptr<LocalMultiFilter1D> symbetaMultiFilter1D(
        int m, double bandwidth, unsigned maxDegree,
        unsigned numberOfGridPoints, double xmin, double xmax,
        const BoundaryHandling& boundaryMethod,
        const unsigned char* exclusionMask = 0,
        bool excludeCentralPoint = false);
}

#include "npstat/stat/LocalMultiFilter1D.icc"

#endif // NPSTAT_LOCALMULTIFILTER1D_HH_
