#ifndef NPSTAT_CLASSICALORTHOPOLYS1D_HH_
#define NPSTAT_CLASSICALORTHOPOLYS1D_HH_

/*!
// \file ClassicalOrthoPolys1D.hh
//
// \brief Orthonormal versions of some classical orthogonal polynomials
//
// Author: I. Volobouev
//
// July 2018
*/

#include <cmath>

#include "npstat/nm/AbsClassicalOrthoPoly1D.hh"

namespace npstat {
    /** Orthonormal Legendre polynomials on [-1, 1] */
    class LegendreOrthoPoly1D : public AbsClassicalOrthoPoly1D
    {
    public:
        inline virtual LegendreOrthoPoly1D* clone() const
            {return new LegendreOrthoPoly1D(*this);}

        inline virtual ~LegendreOrthoPoly1D() {}

        inline long double weight(const long double x) const
            {return (x >= -1.0L && x <= 1.0L) ? 0.5L : 0.0L;}
        inline double xmin() const {return -1.0L;}
        inline double xmax() const {return 1.0L;}

    protected:
        inline std::pair<long double,long double>
        recurrenceCoeffs(const unsigned k) const
        {
            long double sqb = 1.0L;
            if (k)
                sqb = 1.0L/sqrtl(4.0L - 1.0L/k/k);
            return std::pair<long double,long double>(0.0L, sqb);
        }
    };

    /** Orthonormal Legendre polynomials on [0, 1] (that is, shifted) */
    class ShiftedLegendreOrthoPoly1D : public AbsClassicalOrthoPoly1D
    {
    public:
        inline virtual ShiftedLegendreOrthoPoly1D* clone() const
            {return new ShiftedLegendreOrthoPoly1D(*this);}

        inline virtual ~ShiftedLegendreOrthoPoly1D() {}

        inline long double weight(const long double x) const
            {return (x >= 0.0L && x <= 1.0L) ? 1.0L : 0.0L;}
        inline double xmin() const {return 0.0L;}
        inline double xmax() const {return 1.0L;}

    protected:
        inline std::pair<long double,long double>
        recurrenceCoeffs(const unsigned k) const
        {
            long double sqb = 1.0L;
            if (k)
                sqb = 0.5L/sqrtl(4.0L - 1.0L/k/k);
            return std::pair<long double,long double>(0.5L, sqb);
        }
    };

    /**
    // The weight function for Jacobi polynomials is proportional
    // to (1 - x)^alpha (1 + x)^beta
    */
    class JacobiOrthoPoly1D : public AbsClassicalOrthoPoly1D
    {
    public:
        JacobiOrthoPoly1D(double alpha, double beta);

        inline virtual JacobiOrthoPoly1D* clone() const
            {return new JacobiOrthoPoly1D(*this);}

        inline virtual ~JacobiOrthoPoly1D() {}

        inline double alpha() const {return alpha_;}
        inline double beta() const {return beta_;}

        long double weight(long double x) const;
        inline double xmin() const {return -1.0L;}
        inline double xmax() const {return 1.0L;}

    protected:
        std::pair<long double,long double>
        recurrenceCoeffs(const unsigned k) const;

    private:
        double alpha_;
        double beta_;
        long double norm_;
    };

    /** Orthonormal Chebyshev polynomials of the 1st kind */
    class ChebyshevOrthoPoly1st : public AbsClassicalOrthoPoly1D
    {
    public:
        inline virtual ChebyshevOrthoPoly1st* clone() const
            {return new ChebyshevOrthoPoly1st(*this);}

        inline virtual ~ChebyshevOrthoPoly1st() {}

        inline long double weight(const long double x) const
        {
            const long double Pi = 3.14159265358979323846264338L;
            return (x > -1.0L && x < 1.0L) ? 1.0/sqrtl(1.0L - x*x)/Pi : 0.0L;
        }
        inline double xmin() const {return -1.0L;}
        inline double xmax() const {return 1.0L;}

    protected:
        inline std::pair<long double,long double>
        recurrenceCoeffs(const unsigned k) const
        {
            long double sqb = 0.5L;
            if (k == 0U)
                sqb = 1.772453850905516027298167483L; // sqrt(Pi)
            else if (k == 1U)
                sqb = 0.7071067811865475244008443621L; // 1/sqrt(2)
            return std::pair<long double,long double>(0.0L, sqb);
        }
    };

    /** Orthonormal Chebyshev polynomials of the 2nd kind */
    class ChebyshevOrthoPoly2nd : public AbsClassicalOrthoPoly1D
    {
    public:
        inline virtual ChebyshevOrthoPoly2nd* clone() const
            {return new ChebyshevOrthoPoly2nd(*this);}

        inline virtual ~ChebyshevOrthoPoly2nd() {}

        inline long double weight(const long double x) const
        {
            const long double PiOver2 = 1.57079632679489661923132169L;
            return (x > -1.0L && x < 1.0L) ? sqrtl(1.0L - x*x)/PiOver2 : 0.0L;
        }
        inline double xmin() const {return -1.0L;}
        inline double xmax() const {return 1.0L;}

    protected:
        inline std::pair<long double,long double>
        recurrenceCoeffs(const unsigned k) const
        {
            long double sqb = 0.5L;
            if (!k)
                sqb = 1.25331413731550025120788264L; // sqrt(Pi/2)
            return std::pair<long double,long double>(0.0L, sqb);
        }
    };

    /** Orthonormal probabilists' Hermite polynomials */
    class HermiteProbOrthoPoly1D : public AbsClassicalOrthoPoly1D
    {
    public:
        inline virtual HermiteProbOrthoPoly1D* clone() const
            {return new HermiteProbOrthoPoly1D(*this);}

        inline virtual ~HermiteProbOrthoPoly1D() {}

        long double weight(long double x) const;
        inline double xmin() const {return -xmax();}
        inline double xmax() const {return 150.0;}

    protected:
        std::pair<long double,long double>
        recurrenceCoeffs(const unsigned k) const;
    };
}

#endif // NPSTAT_CLASSICALORTHOPOLYS1D_HH_
