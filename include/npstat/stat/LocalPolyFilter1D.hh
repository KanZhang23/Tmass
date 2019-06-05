#ifndef NPSTAT_LOCALPOLYFILTER1D_HH_
#define NPSTAT_LOCALPOLYFILTER1D_HH_

/*!
// \file LocalPolyFilter1D.hh
//
// \brief Local polynomial filtering (regression) on 1-d equidistant grids
//
// This technique generalizes the Savitzky-Golay smoothing filter to
// arbitrary weight functions (not just a flat weight which leads to
// Legendre polynomials).
//
// Author: I. Volobouev
//
// November 2009
*/

#include "geners/CPP11_auto_ptr.hh"
#include "geners/ClassId.hh"

#include "npstat/nm/Matrix.hh"

#include "npstat/stat/AbsPolyFilter1D.hh"
#include "npstat/stat/AbsFilter1DBuilder.hh"

namespace npstat {
    /** This class performs local polynomial filtering in one dimension */
    class LocalPolyFilter1D : public AbsPolyFilter1D
    {
    public:
        /**
        // Main constructor. The arguments are as follows:
        //
        //  taper     -- Damping factors for each polynomial degree
        //               (starting with the 0th order term). This can be
        //               NULL in which case it is assumed that all
        //               factors are 1.
        //
        //  maxDegree -- Maximum degree of the polynomials. The length
        //               of the "taper" array (if not NULL) must be equal
        //               to maxDegree + 1. Note that, far away from the
        //               boundaries (where the situation is symmetric)
        //               the same filter will be produced using the same
        //               taper with an even degree N and with an odd degree
        //               N+1. Near the boundaries the filter coefficients
        //               will, of course, differ in these two cases.
        //
        //  filterBuilder -- An instance of a class which actually builds
        //                   the filters when requested. This builder
        //                   is used only inside the constructor.
        //
        //  dataLen   -- The length of the data arrays which will be
        //               used with this filter (this info is needed in
        //               order to take into account the boundary effects).
        */
        LocalPolyFilter1D(const double* taper, unsigned maxDegree,
                          const AbsFilter1DBuilder& filterBuilder,
                          unsigned dataLen);

        LocalPolyFilter1D(const LocalPolyFilter1D&);
        LocalPolyFilter1D& operator=(const LocalPolyFilter1D&);

        virtual ~LocalPolyFilter1D();

        bool operator==(const LocalPolyFilter1D& r) const;
        inline bool operator!=(const LocalPolyFilter1D& r) const
            {return !(*this == r);}

        //@{
        /** Inspect object properties */
        double taper(unsigned degree) const;
        inline unsigned maxDegree() const {return maxDegree_;}
        inline unsigned dataLen() const {return nbins_;}
        inline const std::vector<double>& getBandwidthFactors() const
            {return bandwidthFactors_;}
        //@}

        /** Self contribution needed for cross-validation */
        double selfContribution(unsigned binNumber) const;

        /** Get the filter coefficients for the given bin */
        const PolyFilter1D& getFilter(unsigned binNumber) const;

        /**
        // This method performs the filtering. "dataLen",
        // which is the length of both "in" and "out" arrays,
        // must be the same as the one in the constructor.
        */
        template <typename Tin, typename Tout>
        void filter(const Tin* in, unsigned dataLen, Tout* out) const;

        /**
        // A diffent filtering method in which the shapes of the
        // kernels are determined by the positions of the "sources"
        // (i.e., sample points) instead of the positions at which
        // the density (or response) is estimated.
        */
        template <typename Tin, typename Tout>
        void convolve(const Tin* in, unsigned dataLen, Tout* out) const;

        /**
        // Generate the complete (non-sparse) representation of the filter.
        // It will be a generalized stochastic matrix (each row sums to 1).
        */
        Matrix<double> getFilterMatrix() const;

        /**
        // Generate a doubly stochastic filter out of this one.
        // Such filters are useful for sequential copula smoothing.
        // NULL pointer will be returned in case the requested
        // margin tolerance is positive and can not be reached
        // within the number of iterations allowed.
        */
        CPP11_auto_ptr<LocalPolyFilter1D> doublyStochasticFilter(
            double tolerance, unsigned maxIterations) const;

        /** An experimental filter with an adjusted eigenspectrum */
        CPP11_auto_ptr<LocalPolyFilter1D> eigenGroomedFilter() const;

        // Methods needed for I/O
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::LocalPolyFilter1D";}
        static inline unsigned version() {return 2;}
        static LocalPolyFilter1D* read(const gs::ClassId& id, std::istream& in);

    private:
        LocalPolyFilter1D();

        mutable std::vector<long double> sumBuffer_;
        std::vector<PolyFilter1D*> unique_;
        double* taper_;
        PolyFilter1D** bins_;
        unsigned nbins_;
        unsigned maxDegree_;
        std::vector<double> bandwidthFactors_;

        template <typename T>
        double convolute(const T* data, const PolyFilter1D* filter) const;

        void addWeightedFilter(long double w, unsigned binNum) const;

        void clearSumBuffer() const;

        void releaseMem();
        void copyOtherData(const LocalPolyFilter1D&);

        // The following function will assume that is was
        // given a matrix filled with zeros as an argument
        void fillFilterMatrix(Matrix<double>* fm) const;

#ifdef SWIG
    public:
        LocalPolyFilter1D* doublyStochasticFilter_2(
            double tolerance, unsigned maxIterations) const;

        LocalPolyFilter1D* eigenGroomedFilter_2() const;

        void filter_2(const double* in, unsigned dataLen,
                      double* out, unsigned outLen) const;

        void convolve_2(const double* in, unsigned dataLen,
                        double* out, unsigned outLen) const;
#endif
    };

    /**
    // This class can be used in places where LocalPolyFilter1D is expected
    // but filtering is not needed due to some reason. Calling "filter" or
    // "convolve" methods of this class will transfer inputs to outputs
    // unmodified. The data length still must be correct.
    */
    class DummyLocalPolyFilter1D : public LocalPolyFilter1D
    {
    public:
        explicit DummyLocalPolyFilter1D(unsigned dataLen);

        inline virtual ~DummyLocalPolyFilter1D() {}

        // Methods related to I/O
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::DummyLocalPolyFilter1D";}
        static inline unsigned version() {return 1;}
        static DummyLocalPolyFilter1D* read(const gs::ClassId& id,
                                            std::istream& in);
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
    //  maxDegree  -- Degree of the LOrPE polynomial. Interpretation of
    //                non-integer arguments is by the "continuousDegreeTaper"
    //                function.
    //
    //  numberOfGridPoints -- Length of the data array to be used with this
    //                        filter (typically, number of histogram bins
    //                        and such).
    //
    //  xmin, xmax -- Data grid limits (as in a histogram).
    //
    //  boundaryMethod -- Method for handling the weight function at the
    //                    boundary of the density support region.
    //
    //  exclusionMask -- If provided, array with numberOfGridPoints elements.
    //                   If an element of this array is not 0, corresponding
    //                   data point will be excluded from the filtering process.
    //
    //  excludeCentralPoint -- If "true", the weight will be set to 0 for
    //                         the central bin of the filter. This can be
    //                         useful in some cross validation scenarios.
    */
    CPP11_auto_ptr<LocalPolyFilter1D> symbetaLOrPEFilter1D(
        int m, double bandwidth, double maxDegree,
        unsigned numberOfGridPoints, double xmin, double xmax,
        const BoundaryHandling& boundaryMethod,
        const unsigned char* exclusionMask = 0,
        bool excludeCentralPoint = false);

    /*
    // Weight generated by the symmetric beta function at 0 (or by the
    // Gaussian in case m < 0). Useful in certain cross validation scenarios.
    */
    double symbetaWeightAt0(int m, double bandwidth);

#ifdef SWIG
    LocalPolyFilter1D* symbetaLOrPEFilter1D_2(
        int m, double bandwidth, double maxDegree,
        unsigned numberOfGridPoints, double xmin, double xmax,
        const BoundaryHandling& boundaryMethod,
        const std::vector<int>* exclusionMask = 0,
        bool excludeCentralPoint = false);
#endif
}

#include "npstat/stat/LocalPolyFilter1D.icc"

#endif // NPSTAT_LOCALPOLYFILTER1D_HH_
