#ifndef NPSTAT_LOCALLOGISTICREGRESSION_HH_
#define NPSTAT_LOCALLOGISTICREGRESSION_HH_

/*!
// \file LocalLogisticRegression.hh
//
// \brief Facilities for performing local linear and quadratic
//        logistic regression
//
// Classes declared in this header can be used with the
// top-level driver functions minuitUnbinnedLogisticRegression
// and minuitLogisticRegressionOnGrid declared in the
// minuitLocalRegression.hh header (interfaces section).
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/KDTree.hh"

#include "npstat/stat/QuadraticOrthoPolyND.hh"

namespace npstat {
    /** Base class for logistic regression */
    template <class Numeric>
    class LogisticRegressionBase
    {
    public:
        LogisticRegressionBase(const QuadraticOrthoPolyND& poly,
                               bool calculateLikelihoodGradient);
        inline virtual ~LogisticRegressionBase() {}

        //@{
        /** Inspect object properties */
        inline unsigned dim() const {return mydim_;}
        inline const QuadraticOrthoPolyND& getPoly() const {return poly_;}
        inline bool calculatingGradient() const {return calcGradient_;}
        //@}

        //@{
        /** Inspect results of the most recent fit */
        inline const std::vector<double>& lastCoeffs() const {return coeffs_;}
        void getGradient(double* buffer, unsigned bufLen) const;
        inline double getPassCount() const
            {return static_cast<double>(passCount_);}
        inline double getFailCount() const
            {return static_cast<double>(failCount_);}
        //@}

        /**
        // Only the data inside the regression box will be used
        // in the fit (and some points may be subsequently weighted
        // down to zero by the weight function).
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
        // is constructed and before any call to "calculateLogLikelihood".
        */
        void setRegressionBox(const BoxND<Numeric>& box);

        /**
        // The next method sets up a mapping from the fitted region
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
        // The next method calculates the negative log-likelihood of
        // the local logistic regression fit for the current mapping.
        // In case the corresponding flag was set "true" in the constructor,
        // the gradient of this quantity will be calculated as well.
        // The gradient can be subsequently retrieved with the "getGradient"
        // method.
        */
        double calculateLogLikelihood(const double* coeffs, unsigned nCoeffs);

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
        std::vector<double> gradBuffer_;
        std::vector<long double> gradient_;

        // Main accumulator for the likelihood
        long double logli_;

        // Counters for passing and failing entries
        long double passCount_;
        long double failCount_;

        // Stuff which does not change
        const double minlog_;
        const double maxlog_;
        const unsigned mydim_;
        const bool calcGradient_;

    private:
        LogisticRegressionBase();
        LogisticRegressionBase(const LogisticRegressionBase&);
        LogisticRegressionBase& operator=(const LogisticRegressionBase&);

        virtual void makeStandardMapping();
        virtual void actualLogliCalculation() = 0;

        bool mappingIsDone_;
    };


    /** Logistic regression on data samples arranged into k-d trees */
    template<class Point, class Numeric, class BooleanFunctor>
    class LogisticRegressionOnKDTree : public LogisticRegressionBase<Numeric>,
                                       public AbsVisitor<Point, double>
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //   dataTree           -- the tree of data points.
        //
        //   pointPassesOrFails -- a functor that tells us whether the
        //                         point "passes" (observed value of 1)
        //                         or "fails" (observed value of 0).
        //
        //   poly               -- the set of orthogonal polynomials used to
        //                         construct the local regression surface.
        //
        //   calculateLikelihoodGradient -- flag which tells whether the
        //                         code should calculate the gradient
        //                         of log-likelihood with respect to
        //                         coefficients of the local polynomial.
        //
        // This object will not own "dataTree" or "poly" objects. These
        // objects must still exist when the LogisticRegressionOnKDTree
        // object is in use. The point selection functor will be copied.
        */
        LogisticRegressionOnKDTree(
            const KDTree<Point,Numeric>& dataTree,
            const BooleanFunctor& pointPassesOrFails,
            const QuadraticOrthoPolyND& poly,
            bool calculateLikelihoodGradient);

        inline virtual ~LogisticRegressionOnKDTree() {}

        /** Inspect the data */
        inline const KDTree<Point,Numeric>& getDataTree() const
            {return dataTree_;}

        //@{
        /** Method from AbsVisitor we have to implement */
        inline virtual void clear() {this->resetAccumulators();}
        inline virtual double result()
            {return static_cast<double>(-this->logli_);}
        virtual void process(const Point& value);
        //@}

    private:
        const KDTree<Point,Numeric>& dataTree_;
        BooleanFunctor pointPassesOrFails_;

        inline void actualLogliCalculation()
            {dataTree_.visitInBox(*this, this->regressionBox_);}
    };


    /** Logistic regression on regularly sampled data */
    template <typename Numeric, unsigned StackLen=1U, unsigned StackDim=10U>
    class LogisticRegressionOnGrid : public LogisticRegressionBase<int>,
                                     public AbsArrayProjector<Numeric, double>
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        //   numerator   -- count of points which "pass" for
        //                  this grid cell
        //
        //   denominator -- count of all points associated with
        //                  this grid cell
        //
        //   poly        -- the set of orthogonal polynomial used to
        //                  construct the local regression surface.
        //
        //   calculateLikelihoodGradient -- flag which tells whether
        //                  the code should calculate the gradient
        //                  of log-likelihood with respect to
        //                  coefficients of the local polynomial.
        //
        // This object will not own "numerator", "denominator",
        // or "poly" objects. These objects must still exist when
        // the LogisticRegressionOnGrid object is in use.
        //
        // The "standard" situation here is that the regression
        // box is simply taken to be an integer-sized box whose
        // length is the same as the number of steps used to
        // build the polynomials.
        */
        LogisticRegressionOnGrid(
            const ArrayND<Numeric,StackLen,StackDim>& numerator,
            const ArrayND<Numeric,StackLen,StackDim>& denominator,
            const QuadraticOrthoPolyND& poly,
            bool calculateLikelihoodGradient);

        inline virtual ~LogisticRegressionOnGrid() {}

        //@{
        /** Inspect object properties */
        inline const ArrayND<Numeric,StackLen,StackDim>& getNumerator() const
            {return numerator_;}
        inline const ArrayND<Numeric,StackLen,StackDim>& getDenominator() const
            {return denominator_;}        
        //@}

        //@{
        /** Method from AbsArrayProjector we have to implement */
        inline virtual void clear() {this->resetAccumulators();}
        inline virtual double result()
            {return static_cast<double>(-this->logli_);}
        virtual void process(const unsigned *index, unsigned indexLen,
                             unsigned long linearIndex, const Numeric& value);
        //@}

    private:
        const ArrayND<Numeric,StackLen,StackDim>& numerator_;
        const ArrayND<Numeric,StackLen,StackDim>& denominator_;
        const Numeric zero_;

        void makeStandardMapping();
        inline void actualLogliCalculation()
            {denominator_.processSubrange(*this, this->regressionBox_);}
    };
}

#include "npstat/stat/LocalLogisticRegression.icc"

#endif // NPSTAT_LOCALLOGISTICREGRESSION_HH_
