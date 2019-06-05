#ifndef NPSTAT_GAUSSHERMITEQUADRATURE_HH_
#define NPSTAT_GAUSSHERMITEQUADRATURE_HH_

/*!
// \file GaussHermiteQuadrature.hh
//
// \brief Gauss-Hermite quadratures in long double precision
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
    // Gauss-Hermite quadrature. Internally, operations are performed
    // in long double precision.
    */
    class GaussHermiteQuadrature
    {
    public:
        /**
        // At the moment, the following numbers of points are supported:
        // 16, 32, 64, 100, 128, 256.
        //
        // If an unsupported number of points is given in the
        // constructor, std::invalid_argument exception will be thrown.
        */
        explicit GaussHermiteQuadrature(unsigned npoints);

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

        /** Perform the quadrature */
        template <typename FcnResult, typename FcnArg>
        long double integrate(const Functor1<FcnResult,FcnArg>& fcn) const;

        /** Perform the quadrature with Gaussian density weight */
        template <typename FcnResult, typename FcnArg>
        long double integrateProb(long double mean, long double sigma,
                                  const Functor1<FcnResult,FcnArg>& fcn) const;

        /** Check if the rule with the given number of points is supported */
        static bool isAllowed(unsigned npoints);

        /** The complete set of allowed rules, in the increasing order */
        static std::vector<unsigned> allowedNPonts();

        /**
        // Minimum number of points, among the supported rules, which
        // integrates a polynomial with the given degree exactly (with
        // the appropriate weight). Returns 0 if the degree is out of range.
        */
        static unsigned minimalExactRule(unsigned polyDegree);

    private:
        GaussHermiteQuadrature();

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

#include "npstat/nm/GaussHermiteQuadrature.icc"

#endif // NPSTAT_GAUSSHERMITEQUADRATURE_HH_
