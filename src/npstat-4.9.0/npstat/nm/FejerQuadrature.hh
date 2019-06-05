#ifndef NPSTAT_FEJERQUADRATURE_HH_
#define NPSTAT_FEJERQUADRATURE_HH_

/*!
// \file FejerQuadrature.hh
//
// \brief Fejer quadratures in long double precision
//
// Author: I. Volobouev
//
// July 2018
*/

#include <vector>
#include <utility>

#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /** 
    // Fejer quadrature. Internally, operations are performed
    // in long double precision.
    */
    class FejerQuadrature
    {
    public:
        explicit FejerQuadrature(unsigned npoints);

        /** Return the number of quadrature points */
        inline unsigned npoints() const {return npoints_;}

        /**
        // The abscissae of the integration rule.
        // The buffer length should be at least npoints.
        */
        void getAbscissae(long double* abscissae, unsigned len) const;

        /**
        // The weights of the integration rule.
        // The buffer length should be at least npoints.
        */
        void getWeights(long double* weights, unsigned len) const;

        /** Perform the quadrature on [a, b] interval */
        template <typename FcnResult, typename FcnArg>
        long double integrate(const Functor1<FcnResult,FcnArg>& fcn,
                              long double a, long double b) const;

        /**
        // Weighted integration points (Chebyshev points in this case)
        // on the given interval, suitable for constructing orthogonal
        // polynomials w.r.t. the given weight function (in particular,
        // by ContOrthoPoly1D class). Naturally, rule with "npoints"
        // points must be able to calculate polynomial normalization
        // integrals exactly.
        */
        template <class Functor>
        std::vector<std::pair<double,double> > weightedIntegrationPoints(
            const Functor& weight, long double a, long double b) const;

        /**
        // Minimum number of points which integrates a polynomial
        // with the given degree exactly
        */
        static unsigned minimalExactRule(unsigned polyDegree);

    private:
        FejerQuadrature();

        std::vector<long double> a_;
        std::vector<long double> w_;
        mutable std::vector<std::pair<long double, long double> > buf_;
        unsigned npoints_;

#ifdef SWIG
    public:
        inline std::vector<double> abscissae2() const
        {
            return std::vector<double>(a_.begin(), a_.end());
        }

        inline std::vector<double> weights2() const
        {
            return std::vector<double>(w_.begin(), w_.end());
        }
#endif
    };
}

#include "npstat/nm/FejerQuadrature.icc"

#endif // NPSTAT_FEJERQUADRATURE_HH_
