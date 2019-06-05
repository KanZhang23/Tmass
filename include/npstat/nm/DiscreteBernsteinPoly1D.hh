#ifndef NPSTAT_DISCRETEBERNSTEINPOLY1D_HH_
#define NPSTAT_DISCRETEBERNSTEINPOLY1D_HH_

/*!
// \file DiscreteBernsteinPoly1D.hh
//
// \brief Discrete Bernstein polynomials in one dimension
//
// Author: I. Volobouev
//
// June 2013
*/

#include <vector>

#include "npstat/nm/GaussLegendreQuadrature.hh"

namespace npstat {
    class DiscreteBernsteinPoly1D
    {
    public:
        DiscreteBernsteinPoly1D(unsigned degree, unsigned gridLength);

        //@{
        /** A simple inspector of object properties */
        inline unsigned gridLength() const {return gridLength_;}
        inline unsigned degree() const {return degree_;}
        //@}

        /** Return the value of one of the polynomials for the given cell */
        double poly(unsigned polyNumber, unsigned gridIndex) const;

        /** Return the average derivative for the given cell */
        double derivative(unsigned polyNumber, unsigned gridIndex) const;

    private:
        DiscreteBernsteinPoly1D();

        std::vector<double> lookupTable_;
        std::vector<double> deriTable_;
        unsigned degree_;
        unsigned gridLength_;
    };

    /**
    // Extrapolation of Bernstein polynomials to non-integer degrees.
    // Works significantly slower than the integer version.
    */
    class DiscreteBeta1D
    {
    public:
        DiscreteBeta1D(double effectiveDegree, unsigned gridLength);

        //@{
        /** A simple inspector of object properties */
        inline unsigned gridLength() const {return gridLength_;}
        inline double degree() const {return degree_;}
        //@}

        /** Return the value of the function for the given cell */
        double poly(double effectivePolyNumber, unsigned gridIndex) const;

        /** Return the average derivative for the given cell */
        double derivative(double effectivePolyNumber, unsigned gridIndex) const;

    private:
        DiscreteBeta1D();

        long double getNorm(double polyNumber) const;

        GaussLegendreQuadrature g_;
        long double loggamma_;
        long double gamma_;
        long double step_;
        mutable long double lastNorm_;
        mutable double lastEffPolyNumber_;
        double degree_;
        unsigned gridLength_;
    };
}

#endif // NPSTAT_DISCRETEBERNSTEINPOLY1D_HH_
