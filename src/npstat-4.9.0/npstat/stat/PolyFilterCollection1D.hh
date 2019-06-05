#ifndef NPSTAT_POLYFILTERCOLLECTION1D_HH_
#define NPSTAT_POLYFILTERCOLLECTION1D_HH_

/*!
// \file PolyFilterCollection1D.hh
//
// \brief Collection of 1-d local polynomial filters intended for use in
//        bandwidth scans
//
// Author: I. Volobouev
//
// September 2010
*/

#include <map>
#include <vector>

#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // Collection of LocalPolyFilter1D objects with different bandwidth
    // values. Filters are stored internally after their corresponding
    // bandwidth values are used for the first time.
    //
    // This class is intended for use with bandwidth scans (for example,
    // in cross-validation scenarios).
    */
    class PolyFilterCollection1D
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        // distro    -- Weight function to use in building the filter.
        //
        // taper     -- The taper function used to build the filter
        //              (can be NULL).
        //
        // maxDegree -- Maximum polynomial degree of the filter.
        //              If the taper array is provided, its length
        //              should be maxDegree + 1.
        //
        // binwidth  -- Bin width used to bin the data.
        //
        // dataLen   -- Expected length of the data array to filter.
        //
        // bm        -- Method used to handle LOrPE weight at the boundary.
        */
        PolyFilterCollection1D(const AbsScalableDistribution1D& distro,
                               const double* taper, unsigned maxDegree,
                               double binwidth, unsigned dataLen,
                               const BoundaryHandling& bm);

        ~PolyFilterCollection1D();

        //@{
        /** Inspect object properties */
        inline unsigned dataLength() const {return dataLen_;}
        inline double binWidth() const {return binwidth_;}
        inline const BoundaryHandling& boundaryMethod() const {return bm_;}
        inline unsigned maxDegree() const {return maxDegree_;}
        double taper(unsigned i) const;
        //@}

        /**
        // Retrive the filter which corresponds to the given bandwidth.
        // If such a filter does not exist yet, construct it first.
        */
        const LocalPolyFilter1D& getPolyFilter(double bandwidth);

        /**
        // The method which performs the filtering. "dataLen",
        // which is the length of both "in" and "out" arrays,
        // must be the same as the one provided in the constructor.
        */
        template <typename Tin, typename Tout>
        inline void filter(const double bandwidth, const Tin* in,
                           const unsigned dataLen, Tout* out)
            {getPolyFilter(bandwidth).filter(in, dataLen, out);}

        /**
        // This filtering method calls the "convolve" method of
        // LocalPolyFilter1D objects instead of their "filter" method
        */
        template <typename Tin, typename Tout>
        inline void convolve(const double bandwidth, const Tin* in,
                             const unsigned dataLen, Tout* out)
            {getPolyFilter(bandwidth).convolve(in, dataLen, out);}

    private:
        PolyFilterCollection1D();
        PolyFilterCollection1D(const PolyFilterCollection1D&);
        PolyFilterCollection1D& operator=(const PolyFilterCollection1D&);

        LocalPolyFilter1D* processBandwidth(double bandwidth);

        std::map<double,const LocalPolyFilter1D*> filterMap_;
        std::vector<double> scanBuf_;
        AbsScalableDistribution1D* kernel_;
        double binwidth_;
        double* taper_;
        unsigned maxDegree_;
        unsigned dataLen_;
        BoundaryHandling bm_;
    };
}

#endif // NPSTAT_POLYFILTERCOLLECTION1D_HH_
