#ifndef NPSTAT_EXPMAPPER1D_HH_
#define NPSTAT_EXPMAPPER1D_HH_

/*!
// \file ExpMapper1d.hh
//
// \brief One-dimensional log-linear transformation functor (log in y)
//
// Author: I. Volobouev
//
// January 2014
*/

#include <cmath>
#include <stdexcept>

namespace npstat {
    /** Functor which performs log-linear mapping in 1-d (log in y) */
    class ExpMapper1d
    {
    public:
        /** The default constructor creates a transform y = exp(x) */
        inline ExpMapper1d() : a_(1.0), b_(0.0) {}

        /**
        // The point at x0 is mapped into y0, the point at x1
        // is mapped into y1. y0 and y1 must be positive.
        // Points between x0 and x1 are mapped into y in such a way
        // that the transformation is linear in the log(y) space.
        */
        inline ExpMapper1d(const double x0, const double y0,
                           const double x1, const double y1)
        {
            if (!(y0 > 0.0 && y1 > 0.0)) throw std::invalid_argument(
                "In npstat::ExpMapper1d constructor: "
                "both ordinates must be positive");
            const double lny1 = log(y1);
            const double lny0 = log(y0);
            const double dx = x1 - x0;
            if (!dx) throw std::invalid_argument(
                "In npstat::ExpMapper1d constructor: "
                "invalid arguments (x0 == x1)");
            a_ = (lny1 - lny0)/dx;
            b_ = ((lny0 + lny1) - a_*(x0 + x1))/2.0;
        }

        /** Explicit constructor for the transform y = exp(ca*x + cb) */
        inline ExpMapper1d(const double ca, const double cb) : a_(ca), b_(cb) {}

        /** Perform the transformation */
        inline double operator()(const double& x) const {return exp(a_*x+b_);}

        /** Get the linear coefficient of the transform */
        inline double a() const {return a_;}

        /** Get the transform constant */
        inline double b() const {return b_;}

    private:
        double a_;
        double b_;
    };
}

#endif // NPSTAT_EXPMAPPER1D_HH_
