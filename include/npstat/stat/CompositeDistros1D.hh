#ifndef NPSTAT_COMPOSITEDISTROS1D_HH_
#define NPSTAT_COMPOSITEDISTROS1D_HH_

/*!
// \file CompositeDistros1D.hh
//
// \brief A few concrete 1-d composite distributions.
//
// See the description of CompositeDistribution1D class for more details
// about this family of distributions.
//
// These are pure helper classes, to simplify the construction a little
// bit. They do not have their own read/write methods, so for the purpose
// of persistence they will be converted into CompositeDistribution1D
// objects (all essential functions such as density, cdf, etc. will be
// preserved but the actual class name will be lost). Refer to objects
// of these clases by CompositeDistribution1D pointer or reference if you
// plan to use persistency.
//
// Author: I. Volobouev
//
// September 2010
*/

#include <cmath>
#include <cassert>
#include <stdexcept>

#include "npstat/stat/CompositeDistribution1D.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/JohnsonCurves.hh"

namespace npstat {
    /** Composite using LogQuadratic1D as G(x) and Gauss1D as H(x) */
    struct CompositeGauss1D : public CompositeDistribution1D
    {
        /**
        // "location" and "scale" arguments are for the Gauss1D
        // distribution, "a" and "b" are for LogQuadratic1D.
        */
        inline CompositeGauss1D(const double location, const double scale,
                                const double a, const double b)
            : CompositeDistribution1D(LogQuadratic1D(0.0, 1.0, a, b),
                                      Gauss1D(location, scale)) {}

        inline virtual ~CompositeGauss1D() {}

        inline virtual CompositeGauss1D* clone() const
            {return new CompositeGauss1D(*this);}
    };


    /** Composite using Beta1D as G(x) and Gauss1D as H(x) */
    struct BetaGauss1D : public CompositeDistribution1D
    {
        /**
        // "location" and "scale" arguments are for the Gauss1D
        // distribution, "a" and "b" are for Beta1D. The standard
        // alpha and beta arguments of Beta1D will be set as follows:
        // alpha = exp(a), beta = exp(b).
        */
        inline BetaGauss1D(const double location, const double scale,
                           const double a, const double b)
            : CompositeDistribution1D(Beta1D(0.0, 1.0, exp(a), exp(b)),
                                      Gauss1D(location, scale)) {}

        inline virtual ~BetaGauss1D() {}

        inline virtual BetaGauss1D* clone() const
            {return new BetaGauss1D(*this);}
    };


    /** This composite combines a number of LogQuadratic1D distributions */
    struct LogQuadraticLadder : public CompositeDistribution1D
    {
        /**
        // "location" and "scale" arguments are for the LogQuadratic1D
        // distribution placed at the bottom of the ladder
        */
        template <typename Real>
        inline LogQuadraticLadder(const double location, const double scale,
                                  const Real* a, const Real* b,
                                  const unsigned nLogQuadraticDistros)
            : CompositeDistribution1D()
        {
            if (nLogQuadraticDistros < 2U) throw std::invalid_argument(
                "In npstat::LogQuadraticLadder constructor: "
                "at least two distributions must be provided");
            assert(a);
            assert(b);

            pG_ = new LogQuadratic1D(0.0, 1.0, a[0], b[0]);
            if (nLogQuadraticDistros > 2U)
                pH_ = new LogQuadraticLadder(location, scale,
                                             a+1, b+1, nLogQuadraticDistros-1);
            else
                pH_ = new LogQuadratic1D(location, scale, a[1], b[1]);
        }

        inline virtual ~LogQuadraticLadder() {}

        inline virtual LogQuadraticLadder* clone() const
            {return new LogQuadraticLadder(*this);}
    };


    /** 
    // This composite combines an arbitrary tabulated distribution as G(x)
    // and JohnsonSystem as H(x)
    */
    struct BinnedCompositeJohnson : public CompositeDistribution1D
    {
        /**
        // "location", "scale", "skewness", and "kurtosis" arguments are for
        // the JohnsonSystem. "data" and "dataLen" are for BinnedDensity1D.
        */
        template <typename Real>
        inline BinnedCompositeJohnson(double location, double scale,
                                      double skewness, double kurtosis,
                                      const Real* data, unsigned dataLen)
            : CompositeDistribution1D(
                BinnedDensity1D(0.0, 1.0, data, dataLen, 1U),
                JohnsonSystem(location, scale, skewness, kurtosis)) {}

        inline virtual ~BinnedCompositeJohnson() {}

        inline virtual BinnedCompositeJohnson* clone() const
            {return new BinnedCompositeJohnson(*this);}
    };


    /**
    // This composite distribution combines a number of
    // LogQuadratic1D distributions with the Johnson curve
    // placed at the bottom of the ladder
    */
    struct JohnsonLadder : public CompositeDistribution1D
    {
        /**
        // "location", "scale", "skewness", and "kurtosis" arguments are for
        // the JohnsonSystem distribution placed at the bottom of the ladder
        */
        template <typename Real>
        inline JohnsonLadder(const double location, const double scale,
                             const double skewness, const double kurtosis,
                             const Real* a, const Real* b,
                             const unsigned nLogQuadraticDistros)
            : CompositeDistribution1D()
        {
            if (!nLogQuadraticDistros) throw std::invalid_argument(
                "In npstat::JohnsonLadder constructor: "
                "at least one distribution must be provided");
            assert(a);
            assert(b);

            pG_ = new LogQuadratic1D(0.0, 1.0, a[0], b[0]);
            if (nLogQuadraticDistros > 1U)
                pH_ = new JohnsonLadder(location, scale, skewness, kurtosis,
                                        a + 1, b + 1, nLogQuadraticDistros-1U);
            else
                pH_ = new JohnsonSystem(location, scale, skewness, kurtosis);
        }

        inline virtual ~JohnsonLadder() {}

        inline virtual JohnsonLadder* clone() const
            {return new JohnsonLadder(*this);}
    };
}

#endif // NPSTAT_COMPOSITEDISTROS1D_HH_
