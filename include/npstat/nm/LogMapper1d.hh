#ifndef NPSTAT_LOGMAPPER1D_HH_
#define NPSTAT_LOGMAPPER1D_HH_

/*!
// \file LogMapper1d.hh
//
// \brief One-dimensional log-linear transformation functor (log in x)
//
// Author: I. Volobouev
//
// March 2010
*/

#include <cmath>
#include <stdexcept>

namespace npstat {
    /** Functor which performs log-linear mapping in 1-d (log in x) */
    class LogMapper1d
    {
    public:
        /** The default constructor creates a transform y = log(x) */
        inline LogMapper1d() : a_(1.0), b_(0.0) {}

        /**
        // The point at x0 is mapped into y0, the point at x1
        // is mapped into y1. x0 and x1 must be positive and distinct.
        // Points between x0 and x1 are mapped into y in such a way
        // that the transformation is linear in the log(x) space.
        */
        inline LogMapper1d(const double x0, const double y0,
                           const double x1, const double y1)
        {
            if (!(x0 > 0.0 && x1 > 0.0)) throw std::invalid_argument(
                "In npstat::LogMapper1d constructor: "
                "both abscissae must be positive");
            const double lnx1 = log(x1);
            const double lnx0 = log(x0);
            const double dx = lnx1 - lnx0;
            if (!dx) throw std::invalid_argument(
                "In npstat::LogMapper1d constructor: "
                "invalid arguments (log(x0) == log(x1))");
            a_ = (y1 - y0)/dx;
            b_ = ((y0 + y1) - a_*(lnx0 + lnx1))/2.0;
        }

        /** Explicit constructor for the transform y = ca*log(x) + cb */
        inline LogMapper1d(const double ca, const double cb) : a_(ca), b_(cb) {}

        /** Perform the transformation */
        inline double operator()(const double& x) const
        {
            if (!(x > 0.0)) throw std::invalid_argument(
                "In npstat::LogMapper1d::operator(): argument must be positive");
            return a_*log(x)+b_;
        }

        /** Get the linear coefficient of the transform */
        inline double a() const {return a_;}

        /** Get the transform constant */
        inline double b() const {return b_;}

    private:
        double a_;
        double b_;
    };
}

#endif // NPSTAT_LOGMAPPER1D_HH_
