#ifndef NPSTAT_GAUSSLEGENDREQUADRATURE_HH_
#define NPSTAT_GAUSSLEGENDREQUADRATURE_HH_

/*!
// \file GaussLegendreQuadrature.hh
//
// \brief Gauss-Legendre quadratures in long double precision
//
// Author: I. Volobouev
//
// April 2010
*/

#include <vector>
#include <utility>

#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /** 
    // Gauss-Legendre quadrature. Internally, operations are performed
    // in long double precision.
    */
    class GaussLegendreQuadrature
    {
    public:
        /**
        // At the moment, the following numbers of points are supported:
        // 2, 4, 6, 8, 10, 12, 16, 32, 64, 100, 128, 256, 512, 1024.
        //
        // If an unsupported number of points is given in the
        // constructor, std::invalid_argument exception will be thrown.
        */
        explicit GaussLegendreQuadrature(unsigned npoints);

        /** Return the number of quadrature points */
        inline unsigned npoints() const {return npoints_;}

        /**
        // The abscissae are returned for positive points only,
        // so the buffer length should be at least npoints/2.
        */
        void getAbscissae(long double* abscissae, unsigned len) const;

        /**
        // The weights are returned for positive points only,
        // so the buffer length should be at least npoints/2.
        */
        void getWeights(long double* weights, unsigned len) const;

        /** Perform the quadrature on [a, b] interval */
        template <typename FcnResult, typename FcnArg>
        long double integrate(const Functor1<FcnResult,FcnArg>& fcn,
                              long double a, long double b) const;

        /**
        // This method splits the interval [a, b] into "nsplit"
        // subintervals of equal length, applies Gauss-Legendre
        // quadrature to each subinterval, and sums the results.
        */
        template <typename FcnResult, typename FcnArg>
        long double integrate(const Functor1<FcnResult,FcnArg>& fcn,
                              long double a, long double b,
                              unsigned nsplit) const;

        /**
        // Weighted integration points on the given interval, suitable
        // for constructing orthogonal polynomials w.r.t. the given
        // weight function (in particular, by ContOrthoPoly1D class).
        // Naturally, rule with "npoints" points must be able to calculate
        // polynomial normalization integrals exactly.
        */
        template <class Functor>
        std::vector<std::pair<double,double> > weightedIntegrationPoints(
            const Functor& weight, long double a, long double b) const;

        /** Check if the rule with the given number of points is supported */
        static bool isAllowed(unsigned npoints);

        /** The complete set of allowed rules, in the increasing order */
        static std::vector<unsigned> allowedNPonts();

        /**
        // Minimum number of points, among the supported rules, which
        // integrates a polynomial with the given degree exactly.
        // Returns 0 if the degree is out of range.
        */
        static unsigned minimalExactRule(unsigned polyDegree);

    private:
        GaussLegendreQuadrature();

        const long double* a_;
        const long double* w_;
        mutable std::vector<std::pair<long double, long double> > buf_;
        unsigned npoints_;

#ifdef SWIG
    public:
        inline std::vector<double> abscissae2() const
        {
            return std::vector<double>(a_, a_+npoints_/2U);
        }

        inline std::vector<double> weights2() const
        {
            return std::vector<double>(w_, w_+npoints_/2U);
        }
#endif
    };
}

#include "npstat/nm/GaussLegendreQuadrature.icc"

#endif // NPSTAT_GAUSSLEGENDREQUADRATURE_HH_
