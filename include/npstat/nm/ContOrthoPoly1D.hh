#ifndef NPSTAT_CONTORTHOPOLY1D_HH_
#define NPSTAT_CONTORTHOPOLY1D_HH_

/*!
// \file ContOrthoPoly1D.hh
//
// \brief Continuous orthogonal polynomial series in one dimension
//        generated for a discrete measure
//
// Author: I. Volobouev
//
// May 2017
*/

#include <vector>
#include <utility>
#include <cassert>
#include <cmath>
#include <map>

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/OrthoPolyMethod.hh"
#include "npstat/nm/Matrix.hh"
#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    class StorablePolySeries1D;

    class ContOrthoPoly1D
    {
    public:
        // The first element of the pair is the coordinate and the
        // second measure is the weight (all weights must be non-negative)
        typedef std::pair<double,double> MeasurePoint;

        /**
        // Main constructor, with obvious arguments. Internally, the
        // measure will be sorted in the order of increasing weight
        // and the measure coordinates will be shifted so that their
        // weighted mean is at 0.
        */
        ContOrthoPoly1D(unsigned maxDegree,
                        const std::vector<MeasurePoint>& measure,
                        OrthoPolyMethod m = OPOLY_STIELTJES);

        /** Constructor which assumes that all weights are 1.0 */
        template<typename Numeric>
        ContOrthoPoly1D(unsigned maxDegree,
                        const std::vector<Numeric>& coords,
                        OrthoPolyMethod m = OPOLY_STIELTJES);

        //@{
        /** A simple inspector of object properties */
        inline unsigned maxDegree() const {return maxdeg_;}
        inline unsigned long measureLength() const {return measure_.size();}
        inline MeasurePoint measure(const unsigned index) const
            {return measure_.at(index);}
        inline double coordinate(const unsigned index) const
            {return measure_.at(index).first;}
        inline double weight(const unsigned index) const
            {return measure_.at(index).second;}
        inline double sumOfWeights() const {return wsum_;}
        inline double sumOfWeightSquares() const {return wsumsq_;}
        inline bool areAllWeightsEqual() const {return allWeightsEqual_;}
        inline double minCoordinate() const {return minX_;}
        inline double maxCoordinate() const {return maxX_;}
        inline double meanCoordinate() const {return meanX_;}
        //@}

        /** Kish's effective sample size for the measure */
        double effectiveSampleSize() const;

        /** Return the value of one of the normalized polynomials */
        double poly(unsigned deg, double x) const;

        /** Return the values of two of the normalized polynomials */
        std::pair<double,double> polyPair(
            unsigned deg1, unsigned deg2, double x) const;

        //@{
        /**
        // Return the values of all orthonormal polynomials up to some
        // maximum degree. Size of the "polyValues" array should be
        // at least maxdeg + 1.
        */
        void allPolys(double x, unsigned maxdeg, double *polyValues) const;
        void allPolys(double x, unsigned maxdeg, long double *polyValues) const;
        //@}

        /** Return the value of the orthonormal polynomial series */
        double series(const double *coeffs, unsigned maxdeg, double x) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the given function. The length of the array "coeffs"
        // should be at least "maxdeg" + 1. Note that the coefficients
        // are returned in the order of increasing degree (same order
        // is used by the "series" function).
        */
        template <class Functor>
        void calculateCoeffs(const Functor& fcn,
                             double *coeffs, unsigned maxdeg) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the given function in such a way that the integral of
        // the truncated series on the [xmin, xmax] interval is constrained
        // to "integralValue". The length of the array "coeffs" should be
        // at least "maxdeg" + 1. Note that the coefficients are returned
        // in the order of increasing degree (same order is used by the
        // "series" function). The construction minimizes the ISE weighted
        // by the empirical density.
        //
        // The function returns the chi-square from the corresponding
        // constrained least squares problem. This is the sum of
        // (coeffs[i] - c[i])^2, 0 <= i <= maxdeg, where c[i] are
        // determined by the "calculateCoeffs" method.
        */
        template <class Functor>
        double constrainedCoeffs(const Functor& fcn,
                                 double *coeffs, unsigned maxdeg,
                                 double xmin, double xmax,
                                 double integralValue = 1.0) const;

        /**
        // Build the coefficients of the orthogonal polynomial series
        // for the discrete weight values (that is, fcn(x_i) = w_i,
        // using the terminology of the "calculateCoeffs" method). This
        // can sometimes be useful for weighted measures. Of course,
        // for unweighted measures this results in just delta_{deg,0}.
        */
        void weightCoeffs(double *coeffs, unsigned maxdeg) const;

        /**
        // Integrated squared error between the given function and the
        // polynomial series. Parameter "integrationRulePoints" specifies
        // the parameter "npoints" for the GaussLegendreQuadrature class.
        // If "integrationRulePoints" is 0, the rule will be picked
        // automatically in such a way that it integrates a polynomial
        // of degree maxdeg*2 exactly.
        */
        template <class Functor>
        double integratedSquaredError(const Functor& fcn,
                                      const double *coeffs, unsigned maxdeg,
                                      double xmin, double xmax,
                                      unsigned integrationRulePoints=0U) const;

        /**
        // Squared error between the given function and the polynomial
        // series, weighted according to the measure
        */
        template <class Functor>
        double weightedSquaredError(const Functor& fcn, const double *coeffs,
                                    unsigned maxdeg) const;

        /**
        // This method is useful for testing the numerical precision
        // of the orthonormalization procedure. It returns the scalar
        // products between various polynomials.
        */
        double empiricalKroneckerDelta(unsigned deg1, unsigned deg2) const;

        /**
        // If the Kronecker deltas were calculated from a sample, they
        // would be random and would have a covariance matrix. This
        // is an estimate of the terms in that covariance matrix. The
        // covariance is between delta_{deg1,deg2} and delta_{deg3,deg4}.
        */
        double empiricalKroneckerCovariance(unsigned deg1, unsigned deg2,
                                            unsigned deg3, unsigned deg4) const;

        /** 
        // Examine the recurrence coefficients. The first element of
        // the returned pair is alpha, and the second element is beta.
        */
        std::pair<double,double> recurrenceCoeffs(unsigned deg) const;

        /**
        // Generate principal minor of order n of the Jacobi matrix.
        // n must not exceed "maxDegree"
        */
        Matrix<double> jacobiMatrix(unsigned n) const;

        /** 
        // Roots of the polynomial with the given degree. Naturally,
        // the degree argument must not exceed "maxDegree".
        */
        void calculateRoots(double *roots, unsigned degree) const;

        /**
        // Integral of an orthonormal polynomials to some power without
        // the weight (so this is not an inner product with the poly of
        // degree 0).
        */
        double integratePoly(unsigned degree, unsigned power,
                             double xmin, double xmax) const;

        /**
        // Integral of the product of three orthonormal polynomials
        // without the weight (so this is not an inner product)
        */
        double integrateTripleProduct(unsigned deg1, unsigned deg2,
                                      unsigned deg3, double xmin,
                                      double xmax) const;

        /**
        // Integral of the product of two orthonormal polynomials
        // with some external weight function (e.g., oracle density).
        // "EW" in the method name stands for "externally weighted".
        //
        // "integrationRulePoints" argument will be passed to the
        // GaussLegendreQuadrature constructor and must be supported
        // by that class.
        */
        template <class Functor>
        double integrateEWPolyProduct(const Functor& weight,
                                      unsigned deg1, unsigned deg2,
                                      double xmin, double xmax,
                                      unsigned integrationRulePoints) const;

        /**
        // A measure-weighted average of orthonormal poly values
        // to some power
        */
        double powerAverage(unsigned deg, unsigned power) const;

        /**
        // A measure-weighted average of the product of two orthonormal
        // poly values to some powers
        */
        double jointPowerAverage(unsigned deg1, unsigned power1,
                                 unsigned deg2, unsigned power2) const;

        /** 
        // A measure-weighted average of the product of several
        // orthonormal poly values
        */
        double jointAverage(const unsigned* degrees, unsigned nDegrees,
                            bool degreesSortedIncreasingOrder = false) const;

        //@{
        /**
        // A measure-weighted average that will be remembered internally
        // so that another call to this function with the same arguments
        // will return the answer quickly
        */
        double cachedJointAverage(unsigned deg1, unsigned deg2,
                                  unsigned deg3, unsigned deg4) const;
        double cachedJointAverage(unsigned deg1, unsigned deg2,
                                  unsigned deg3, unsigned deg4,
                                  unsigned deg5, unsigned deg6) const;
        double cachedJointAverage(unsigned deg1, unsigned deg2,
                                  unsigned deg3, unsigned deg4,
                                  unsigned deg5, unsigned deg6,
                                  unsigned deg7, unsigned deg8) const;
        //@}

        /** Covariance between v_{ab} and v_{cd} */
        double cov4(unsigned a, unsigned b, unsigned c, unsigned d) const;

        /** Covariance between v_{ab}*v_{cd} and v_{ef} */
        double cov6(unsigned a, unsigned b, unsigned c, unsigned d,
                    unsigned e, unsigned f) const;

        /** Covariance between v_{ab}*v_{cd} and v_{ef}*v_{gh} */
        double cov8(unsigned a, unsigned b, unsigned c, unsigned d,
                    unsigned e, unsigned f, unsigned g, unsigned h) const;

        /** Covariance between two cov4 estimates (i.e., error on the error) */
        double covCov4(unsigned a, unsigned b, unsigned c, unsigned d,
                       unsigned e, unsigned f, unsigned g, unsigned h) const;

        /**
        // Alternative implementation of "cov8". It is slower than "cov8"
        // but easier to program and verify. Useful mainly for testing "cov8".
        */
        double slowCov8(unsigned a, unsigned b, unsigned c, unsigned d,
                        unsigned e, unsigned f, unsigned g, unsigned h) const;

        /** Estimate of eps_{mn} expectation */
        double epsExpectation(unsigned m, unsigned n, bool highOrder) const;

        /**
         // Estimate of covariance between eps_{ij} and eps_{mn}.
         //
         // The result should be identical with "empiricalKroneckerCovariance"
         // in case "highOrder" argument is "false". However, this method
         // caches results of various calculations of averages and should
         // perform better inside cycles over indices.
         */
        double epsCovariance(unsigned i, unsigned j,
                             unsigned m, unsigned n, bool highOrder) const;

#ifndef SWIG
        /** Construct a StorablePolySeries1D object out of this one */
        CPP11_auto_ptr<StorablePolySeries1D> makeStorablePolySeries(
            double xmin, double xmax,
            const double *coeffs=0, unsigned maxdeg=0) const;
#endif

    private:
        struct Recurrence
        {
            inline Recurrence(const long double a,
                              const long double b)
                : alpha(a), beta(b)
            {
                assert(beta > 0.0L);
                sqrbeta = sqrtl(beta);
            }

            long double alpha;
            long double beta;
            long double sqrbeta;
        };

        class PolyFcn;
        friend class PolyFcn;

        class PolyFcn : public Functor1<long double, long double>
        {
        public:
            inline PolyFcn(const ContOrthoPoly1D& p,
                           const unsigned d1, const unsigned power)
                : poly(p), deg1(d1), polypow(power) {}

            inline long double operator()(const long double& x) const
            {
                long double p = 1.0L;
                if (polypow)
                {
                    p = poly.normpoly(deg1, x);
                    switch (polypow)
                    {
                    case 1U:
                        break;
                    case 2U:
                        p *= p;
                        break;
                    default:
                        p = powl(p, polypow);
                        break;
                    }
                }
                return p;
            }

        private:
            const ContOrthoPoly1D& poly;
            unsigned deg1;
            unsigned polypow;
        };

        class TripleProdFcn;
        friend class TripleProdFcn;

        class TripleProdFcn : public Functor1<long double, long double>
        {
        public:
            inline TripleProdFcn(const ContOrthoPoly1D& p, const unsigned d1,
                                 const unsigned d2, const unsigned d3)
                : poly(p)
            {
                degs[0] = d1;
                degs[1] = d2;
                degs[2] = d3;
                degmax = std::max(d1, std::max(d2, d3));
            }

            inline long double operator()(const long double& x) const
                {return poly.normpolyprod(degs, 3, degmax, x);}

        private:
            const ContOrthoPoly1D& poly;
            unsigned degs[3];
            unsigned degmax;
        };

        template<class Funct> class EWPolyProductFcn;
        template<class Funct> friend class EWPolyProductFcn;

        template<class Funct>
        class EWPolyProductFcn : public Functor1<long double, long double>
        {
        public:
            inline EWPolyProductFcn(const ContOrthoPoly1D& p, const Funct& w,
                                    const unsigned d1, const unsigned d2)
                : poly(p), wf(w)
            {
                degs[0] = d1;
                degs[1] = d2;
                degmax = std::max(d1, d2);
            }

            inline long double operator()(const long double& x) const
                {return poly.normpolyprod(degs,2,degmax,x-poly.meanX_)*wf(x);}

        private:
            const ContOrthoPoly1D& poly;
            const Funct& wf;
            unsigned degs[2];
            unsigned degmax;
        };

        class MemoKey
        {
        public:
            inline MemoKey(const unsigned d0, const unsigned d1,
                           const unsigned d2, const unsigned d3)
                : nDegs_(4U)
            {
                degs_[0] = d0;
                degs_[1] = d1;
                degs_[2] = d2;
                degs_[3] = d3;                
                std::sort(degs_, degs_+nDegs_);
            }

            inline MemoKey(const unsigned d0, const unsigned d1,
                           const unsigned d2, const unsigned d3,
                           const unsigned d4, const unsigned d5)
                : nDegs_(6U)
            {
                degs_[0] = d0;
                degs_[1] = d1;
                degs_[2] = d2;
                degs_[3] = d3;                
                degs_[4] = d4;                
                degs_[5] = d5;                
                std::sort(degs_, degs_+nDegs_);
            }

            inline MemoKey(const unsigned d0, const unsigned d1,
                           const unsigned d2, const unsigned d3,
                           const unsigned d4, const unsigned d5,
                           const unsigned d6, const unsigned d7)
                : nDegs_(8U)
            {
                degs_[0] = d0;
                degs_[1] = d1;
                degs_[2] = d2;
                degs_[3] = d3;                
                degs_[4] = d4;                
                degs_[5] = d5;                
                degs_[6] = d6;                
                degs_[7] = d7;                
                std::sort(degs_, degs_+nDegs_);
            }

            inline const unsigned* degrees() const {return degs_;}
            inline unsigned nDegrees() const {return nDegs_;}

            inline bool operator<(const MemoKey& r) const
            {
                if (nDegs_ < r.nDegs_)
                    return true;
                if (r.nDegs_ < nDegs_)
                    return false;
                for (unsigned i=0; i<nDegs_; ++i)
                {
                    if (degs_[i] < r.degs_[i])
                        return true;
                    if (r.degs_[i] < degs_[i])
                        return false;
                }
                return false;
            }

            inline bool operator==(const MemoKey& r) const
            {
                if (nDegs_ != r.nDegs_)
                    return false;
                for (unsigned i=0; i<nDegs_; ++i)
                    if (degs_[i] != r.degs_[i])
                        return false;
                return true;
            }

            inline bool operator!=(const MemoKey& r) const
                {return !(*this == r);}

        private:
            unsigned degs_[8];
            unsigned nDegs_;
        };

        void calcRecurrenceCoeffs(OrthoPolyMethod m);

        void calcRecurrenceStieltjes();
        void calcRecurrenceLanczos();

        long double monicpoly(unsigned degree, long double x) const;

        std::pair<long double,long double> monicInnerProducts(
            unsigned degree) const;

        long double normpoly(unsigned degree, long double x) const;

        std::pair<long double,long double> twonormpoly(
            unsigned deg1, unsigned deg2, long double x) const;

        long double normpolyprod(const unsigned* degrees, unsigned nDeg,
                                 unsigned degmax, long double x) const;

        long double normseries(const double *coeffs, unsigned maxdeg,
                               long double x) const;

        template <typename Numeric>
        void getAllPolys(double x, unsigned maxdeg, Numeric *polyValues) const;

        double getCachedAverage(const MemoKey& key) const;

        std::vector<MeasurePoint> measure_;
        std::vector<Recurrence> rCoeffs_;
        mutable std::map<MemoKey,double> cachedAverages_;
        long double wsum_;
        long double wsumsq_;
        double meanX_;
        double minX_;
        double maxX_;
        unsigned maxdeg_;
        bool allWeightsEqual_;
    };
}

#include "npstat/nm/ContOrthoPoly1D.icc"

#endif // NPSTAT_CONTORTHOPOLY1D_HH_
