#ifndef NPSTAT_BERNSTEINFILTER1DBUILDER_HH_
#define NPSTAT_BERNSTEINFILTER1DBUILDER_HH_

/*!
// \file BernsteinFilter1DBuilder.hh
//
// \brief Builder of Bernstein polynomial filters in one dimension
//
// The filters are intended for use with the "LocalPolyFilter1D" class.
//
// Author: I. Volobouev
//
// June 2013
*/

#include "npstat/stat/AbsFilter1DBuilder.hh"
#include "npstat/nm/DiscreteBernsteinPoly1D.hh"

namespace npstat {
    /**
    // This class will construct Bernstein polynomial filters
    // for the given length of data discretization grid (number
    // of bins). These filters are positive doubly stochastic and,
    // therefore, useful for sequential copula filtering. They
    // are designed to be used with the "convolve" method of
    // LocalPolyFilter1D class.
    */
    class BernsteinFilter1DBuilder : public AbsFilter1DBuilder
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        // polyDegree        -- Degree of Bernstein polynomials. Its inverse
        //                      plays the role of bandwidth.
        //
        // dataLen           -- Length of data arrays to be filtered by
        //                      the filters built with this object. Must be
        //                      larger than polyDegree.
        //
        // useClosestPoly    -- Determines how data bins are mapped into
        //                      integer polynomial degrees. If this argument
        //                      is "true", normally the "closest" polynomial
        //                      is used. If "false", each bin will use
        //                      a combination of two or three closest
        //                      polynomials.
        //
        // In order to use the smallest possible number of bins with the
        // given poly degree, set dataLen to polyDegree + 1. Also, set
        // "useClosestPoly" parameters to "true".
        */
        BernsteinFilter1DBuilder(unsigned polyDegree, unsigned dataLen,
                                 bool useClosestPoly);

        inline virtual ~BernsteinFilter1DBuilder() {}

        /** This method is pure virtual in the base, so it must be implemented */
        inline unsigned centralWeightLength() const {return dataLen_;}

        /** Internal filters are all different */
        inline bool keepAllFilters() const {return true;}

        //@{
        /** Simple inspector of object properties */
        inline unsigned polyDegree() const {return polyDegree_;}
        inline bool usingClosestPoly() const {return useClosestPoly_;}
        //@}

        /** The "taper" and "lenTaper" arguments will be ignored */
        virtual PolyFilter1D* makeFilter(const double* taper, unsigned lenTaper,
                                       unsigned binnum, unsigned datalen) const;
    private:
        BernsteinFilter1DBuilder();

        void fillClosestPoly(unsigned binnum, long double* f) const;
        void fillWeighted(unsigned binnum, long double* f) const;

        DiscreteBernsteinPoly1D polySet_;
        unsigned polyDegree_;
        unsigned dataLen_;
        bool useClosestPoly_;
    };
}

#endif // NPSTAT_BERNSTEINFILTER1DBUILDER_HH_
