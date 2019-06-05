#ifndef NPSTAT_LOCALQUANTILEREGRESSION_HH_
#define NPSTAT_LOCALQUANTILEREGRESSION_HH_

/*!
// \file LocalQuantileRegression.hh
//
// \brief Facilities for performing local linear and quadratic quantile
//        regression.
//
// Gradient calculation is not supported.
//
// Classes declared in this header can be used with the
// top-level driver functions minuitQuantileRegression
// and minuitQuantileRegressionIncrBW declared in the
// minuitQuantileRegression.hh header (interfaces section).
//
// Author: I. Volobouev
//
// October 2011
*/

#include "npstat/nm/KDTree.hh"

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/QuadraticOrthoPolyND.hh"
#include "npstat/stat/WeightedSampleAccumulator.hh"

#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /** Base class for quantile regression */
    template <class Numeric>
    class QuantileRegressionBase
    {
    public:
        QuantileRegressionBase(const QuadraticOrthoPolyND& poly,
                               double quantile);
        virtual ~QuantileRegressionBase() {}

        //@{
        /** Inspect object properties */
        inline unsigned dim() const {return mydim_;}
        inline double quantile() const {return quantile_;}
        inline const QuadraticOrthoPolyND& getPoly() const {return poly_;}
        inline const std::vector<double>& lastCoeffs() const {return coeffs_;}
        //@}

        /**
        // Only the data inside the regression box will be used
        // in the fit (and some points may be subsequently weighted
        // down to zero by the weigh function).
        //
        // Note that the "setRegressionBox" method automatically
        // sets up the standard mapping: the edges of the
        // regression box are mapped into the boundaries which
        // were used to create the orthogonal polynomials. If the
        // standard mapping is not what you want (for example,
        // this might be the case near the boundaries of the
        // fitted region), you must call "setLinearMapping" after
        // calling "setRegressionBox" in order to override the
        // standard mapping.
        //
        // This method must be called at least once after the object
        // is constructed and before any call to "linearLoss".
        */
        void setRegressionBox(const BoxND<Numeric>& box);

        /**
        // This method sets up a mapping from the fitted region
        // into the region which was used to create the orthogonal
        // polynomials. The mapping is linear in every coordinate
        // separately:  ortho[i] = fitted[i]*scale[i] + location[i].
        // Note that the "setRegressionBox" method automatically sets
        // up the default mapping which will be adequate for most
        // applications.
        */
        void setLinearMapping(const double* location, const double* scale,
                              unsigned locationAndScaleArraySize);

        /**
        // This method will provide a guess for the 0th order
        // coefficient of the regression polynomial. It will also
        // calculate the effective number of points inside the current
        // regression box.
        */
        void empiricalC0(double* c0, double* c0Uncertainty, double* npoints);

        /**
        // This method calculates the function that has to be
        // minimized; the minimization itself is up to some external
        // optimization engine.
        */
        double linearLoss(const double* coeffs, unsigned nCoeffs);

    protected:
        void resetAccumulators();

        const QuadraticOrthoPolyND& poly_;

        // Various vectors with size equal to the space dimensionality
        BoxND<Numeric> regressionBox_;
        std::vector<double> location_;
        std::vector<double> scale_;
        std::vector<double> coords_;

        // Various vectors with size equal to the number of orthogonal
        // polynomials
        std::vector<double> coeffs_;

        // Main accumulator for the loss function
        long double loss_;

        // Stuff which does not change
        const double quantile_;
        const double onemq_;
        const unsigned mydim_;

    private:
        QuantileRegressionBase();
        QuantileRegressionBase(const QuantileRegressionBase&);
        QuantileRegressionBase& operator=(const QuantileRegressionBase&);

        virtual void makeStandardMapping();
        virtual double empiricalQuantile(double* err, double* npoints) = 0;
        virtual void actualLossCalculation() = 0;

        bool mappingIsDone_;
    };


    /** Quantile regression on data samples arranged into k-d trees */
    template<class Point, class Numeric>
    class QuantileRegressionOnKDTree : public QuantileRegressionBase<Numeric>,
                                       public AbsVisitor<Point, double>
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
        //   poly           -- the set of orthogonal polynomials used to
        //                     construct the local regression surface.
        //
        //   quantile       -- the target quantile (between 0.0 and 1.0).
        //
        // This object will not own "dataTree", "regressedValue",
        // or "poly" objects. These objects must still exist when the
        // QuantileRegressionOnKDTree object is in use.
        */
        QuantileRegressionOnKDTree(
            const KDTree<Point,Numeric>& dataTree,
            const Functor1<double,Point>& regressedValue,
            const QuadraticOrthoPolyND& poly,
            double quantile);
        virtual ~QuantileRegressionOnKDTree() {}

        /** Examine the data */
        inline const KDTree<Point,Numeric>& getDataTree() const
            {return dataTree_;}

        //@{
        /** Method from AbsVisitor that we have to implement */
        inline virtual void clear() {this->resetAccumulators();}
        inline virtual double result()
            {return static_cast<double>(this->loss_);}
        virtual void process(const Point& value);
        //@}

    protected:
        const KDTree<Point,Numeric>& dataTree_;
        const Functor1<double,Point>& regressedValue_;

    private:
        virtual double empiricalQuantile(double* err, double* npoints);

        inline virtual void actualLossCalculation()
            {dataTree_.visitInBox(*this, this->regressionBox_);}
    };


    /** Quantile regression on histograms */
    template <typename Numeric>
    class QuantileRegressionOnHisto : public QuantileRegressionBase<double>,
                                      public AbsArrayProjector<Numeric, double>
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //   histo    -- Histogrammed data. It is assumed that the histogram
        //               is filled with actual event counts and that
        //               the quantity which is being regressed is used for
        //               the last histogram axis. The dimensionality of
        //               the histogram therefore must be larger by 1 than
        //               the dimensionality of the QuadraticOrthoPolyND object.
        //               Histogram overflow bins will be ignored.
        //
        //   poly     -- the set of orthogonal polynomial used to construct
        //               the local regression surface.
        //
        //   quantile -- The quantile to determine (between 0.0 and 1.0).
        //
        // This object will not own "histo" or "poly" objects. These objects
        // must still exist when this QuantileRegressionOnHisto object is
        // in use.
        */
        QuantileRegressionOnHisto(const HistoND<Numeric>& histo,
                                  const QuadraticOrthoPolyND& poly,
                                  double quantile);
        virtual ~QuantileRegressionOnHisto() {}

        /** Examine the data */
        inline const HistoND<Numeric>& getHisto() const
            {return histo_;}

        //@{
        /** Method from AbsArrayProjector we have to implement */
        inline virtual void clear() {this->resetAccumulators();}
        inline virtual double result()
            {return static_cast<double>(this->loss_);}
        virtual void process(const unsigned *index, unsigned indexLen,
                             unsigned long linearIndex, const Numeric& value);
        //@}

    protected:
        const HistoAxis& lastAxis_;
        const double halfbin_;
        const Numeric zero_;
        const HistoND<Numeric>& histo_;
        BoxND<int> binBox_;

    private:
        virtual void makeStandardMapping();
        virtual double empiricalQuantile(double* err, double* npoints);

        inline virtual void actualLossCalculation()
            {histo_.binContents().processSubrange(*this, binBox_);}
    };
}

#include "npstat/stat/LocalQuantileRegression.icc"

#endif // NPSTAT_LOCALQUANTILEREGRESSION_HH_
