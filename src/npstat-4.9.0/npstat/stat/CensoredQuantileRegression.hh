#ifndef NPSTAT_CENSOREDQUANTILEREGRESSION_HH_
#define NPSTAT_CENSOREDQUANTILEREGRESSION_HH_

/*!
// \file CensoredQuantileRegression.hh
//
// \brief Local linear and quadratic quantile regression with censoring
//
// Facilities for performing local linear and quadratic quantile
// regression on samples which experience point losses due to detection
// inefficiencies. It is assumed that the inefficiency is due to a cutoff
// on the response variable itself or on a variable which, for any given 
// predictor, can be obtained from response by a monotonous transformation
// (so that an equivalent cut can be made for response). Typical example
// of such a quantity from particle physics is particle (or jet) transverse
// momentum.
//
// Classes declared in this header can be used with the
// top-level driver functions minuitQuantileRegression
// and minuitQuantileRegressionIncrBW declared in the
// minuitQuantileRegression.hh header (interfaces section).
//
// Author: I. Volobouev
//
// November 2011
*/

#include "npstat/stat/LocalQuantileRegression.hh"

namespace npstat {
    /** A simple class to represent the necessary information about censoring */
    struct CensoringInfo
    {
        inline CensoringInfo(const double cut, const double eff)
            : cutoff(cut), efficiency(eff) {}

        double cutoff;     ///< censoring cutoff value at this predictor
        double efficiency; ///< censoring efficiency at this predictor
    };


    /**
    // Class that facilitates censored quantile regression on data samples
    // arranged into a k-d tree structure
    */
    template<class Point, class Numeric>
    class CensoredQuantileRegressionOnKDTree : 
        public QuantileRegressionOnKDTree<Point,Numeric>
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //   dataTree       -- the tree of data points.
        //
        //   regressedValue -- a functor that provides the observed
        //                     value for the given input point. Typically,
        //                     this value will be just one of the Point
        //                     coordinates not used in k-d tree construction.
        //
        //   censoringInfo  -- a functor that provides information about
        //                     censoring for the goven point.
        //
        //   poly           -- the set of orthogonal polynomials used to
        //                     construct the local regression surface.
        //
        //   quantile       -- the target quantile (between 0.0 and 1.0).
        //
        // The "valueLimit" parameter plays the role of a far-away point
        // which for sure lies below all uncut response values if the cut
        // was response > cutoff or above all uncut values if the cut was
        // response < cutoff. This limit should not be chosen extremely far
        // away from the realistically possible response values because
        // such a limit will contribute too much to the loss function and
        // the loss value calculation will suffer excessively from round-off
        // errors.
        //
        // This object will not own "dataTree", "regressedValue",
        // "censoringInfo", or "poly" objects. These objects must still
        // exist when the CensoredQuantileRegressionOnKDTree object is in use.
        */
        CensoredQuantileRegressionOnKDTree(
            const KDTree<Point,Numeric>& dataTree,
            const Functor1<double,Point>& regressedValue,
            const Functor1<CensoringInfo,Point>& censoringInfo,
            const QuadraticOrthoPolyND& poly,
            double quantile, double valueLimit);

        virtual ~CensoredQuantileRegressionOnKDTree() {}

        /** Method from AbsVisitor that we have to implement */
        virtual void process(const Point& value);

    private:
        const Functor1<CensoringInfo,Point>& censoringInfo_;
        const double valueLimit_;
        bool positiveTailPassing_;

        virtual double empiricalQuantile(double* err, double* npoints);
    };


    /**
    // This class assumes that the histogram is filled with
    // actual event counts and that the quantity which is being regressed
    // is used for the last histogram axis. The dimensionality of the
    // histogram therefore must be larger by 1 than the dimensionality
    // of the QuadraticOrthoPolyND object. Histogram overflow bins will be
    // ignored.
    */
    template <typename Numeric, typename Num2>
    class CensoredQuantileRegressionOnHisto :
        public QuantileRegressionOnHisto<Numeric>
    {
    public:
        /**
        // It is assumed that the arrays "efficiency" and "cutoff"
        // contain the efficiency and cutoff values for the predictor
        // values placed in the center of the argument histogram bins
        // (all histogram dimensions except last are predictor dimensions).
        // Therefore, they should be bin-compatible with the histogram
        // binning constructed without last axis.
        //
        // "valueLimit" has the same meaning as in the class
        // CensoredQuantileRegressionOnKDTree. Naturally, this value
        // should be outside (or exactly on the boundary) of the range
        // covered by the last histogram axis.
        */
        CensoredQuantileRegressionOnHisto(const HistoND<Numeric>& histo,
                                          const ArrayND<Num2>& efficiency,
                                          const ArrayND<Num2>& cutoff,
                                          const QuadraticOrthoPolyND& poly,
                                          double quantile, double valueLimit);

        virtual ~CensoredQuantileRegressionOnHisto() {}

        /** Method from AbsArrayProjector that we have to implement */
        virtual void process(const unsigned *index, unsigned indexLen,
                             unsigned long linearIndex, const Numeric& value);
    private:
        const ArrayND<Num2>& efficiency_;
        const ArrayND<Num2>& cutoff_;
        const double valueLimit_;
        bool positiveTailPassing_;

        virtual double empiricalQuantile(double* err, double* npoints);
    };
}

#include "npstat/stat/CensoredQuantileRegression.icc"

#endif // NPSTAT_CENSOREDQUANTILEREGRESSION_HH_
