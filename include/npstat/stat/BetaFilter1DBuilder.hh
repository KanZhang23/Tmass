#ifndef NPSTAT_BETAFILTER1DBUILDER_HH_
#define NPSTAT_BETAFILTER1DBUILDER_HH_

/*!
// \file BetaFilter1DBuilder.hh
//
// \brief Builder of beta distribution filters in one dimension
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
    // This class will construct beta distribution filters for the
    // given length of data discretization grid (number of bins).
    // These filters will be positive. Positive doubly stochastic
    // filters can be easily constructed out of them using the
    // "doublyStochasticFilter" method of the LocalPolyFilter1D class.
    */
    class BetaFilter1DBuilder : public AbsFilter1DBuilder
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        // polyDegree            -- Effective degree for beta filters (using
        //                          the same convention as for Bernstein
        //                          polynomials). Its inverse plays the role
        //                          of bandwidth. This argument can not be
        //                          negative.
        //
        // dataLen               -- Length of data arrays to be filtered by
        //                          the filters built with this object. Must
        //                          be at least 2.
        //
        // effectiveDegreeOffset -- Effective polynomial degree to map into
        //                          the left edge of the density support
        //                          region. This mapping will be linear,
        //                          and the right edge will be mapped into
        //                          polyDegree - effectiveDegreeOffset.
        //                          Useful values of this parameter are
        //                          between -1.0 and 0.0. If a value outside
        //                          this interval is specified, the closest
        //                          value from this interval will be used
        //                          instead.
        //
        // effectiveDegreeCutoff -- The effective degree produced by linear
        //                          mapping will be truncated if it gets
        //                          below this value (or if it gets above
        //                          polyDegree - effectiveDegreeCutoff).
        //                          It can can be useful to tune this
        //                          parameter in such a way the standard
        //                          deviation of the beta kernel does not
        //                          become significantly smaller than the
        //                          width of the discretization grid bins.
        //                          Useful values of this parameter are
        //                          between -1.0 and 0.0. If a value outside
        //                          this interval is specified, the closest
        //                          value from this interval will be used
        //                          instead.
        */
        BetaFilter1DBuilder(double polyDegree, unsigned dataLen,
                            double effectiveDegreeOffset = -0.5,
                            double effectiveDegreeCutoff = -0.5);

        inline virtual ~BetaFilter1DBuilder() {}

        /** This method is pure virtual in the base, so it must be implemented */
        inline unsigned centralWeightLength() const {return dataLen_;}

        /** Internal filters are all different */
        inline bool keepAllFilters() const {return true;}

        //@{
        /** Simple inspector of object properties */
        inline double polyDegree() const {return polyDegree_;}
        inline double effectiveDegreeOffset() const {return effectiveOffset_;}
        inline double effectiveDegreeCutoff() const {return effectiveCutoff_;}
        //@}

        /** The "taper" and "lenTaper" arguments will be ignored */
        virtual PolyFilter1D* makeFilter(const double* taper, unsigned lenTaper,
                                       unsigned binnum, unsigned datalen) const;
    private:
        BetaFilter1DBuilder();

        void fillContinuousBeta(unsigned binnum, long double* f) const;

        DiscreteBeta1D betaSet_;
        double polyDegree_;
        double effectiveOffset_;
        double effectiveCutoff_;
        unsigned dataLen_;
    };
}

#endif // NPSTAT_BETAFILTER1DBUILDER_HH_
