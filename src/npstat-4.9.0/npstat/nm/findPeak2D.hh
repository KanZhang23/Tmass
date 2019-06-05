#ifndef NPSTAT_FINDPEAK2D_HH_
#define NPSTAT_FINDPEAK2D_HH_

/*!
// \file findPeak2D.hh
//
// \brief Peak finding for noisy two-dimensional surfaces
//
// Operations useful in 2-dimensional peak finding. Here, peak finding
// works by expanding local surface patches into 2-d orthogonal polynomials
// of degree 2 (this is equivalent to fitting a quadratic surface using the
// L2 norm) and then finding the peak locations of those polynomials.
//
// Author: I. Volobouev
//
// July 2011
*/

namespace npstat {
    /** Structure to store peak finding results */
    struct Peak2D
    {
        inline Peak2D()
            : x(0.0), y(0.0), magnitude(0.0), valid(false), inside(false)
            {hessian[0] = hessian[1] = hessian[2] = 0.0;}

        /** Shift and scale the peak coordinates */
        void rescaleCoords(double centerCellCoordinateInX,
                           double gridCellWidthInX,
                           double centerCellCoordinateInY,
                           double gridCellWidthInY);

        double x;          ///< Peak x coordinate.
        double y;          ///< Peak y coordinate.
        double magnitude;  ///< Value of the quadratic fit at the peak position.

        /**
        // hessian[0] = Hxx,
        // hessian[1] = Hxy,
        // hessian[2] = Hyy.
        */
        double hessian[3]; ///< Hessian of the fitted quadratic polynomial.
        bool valid;        ///< True if the Hessian is negative-definite.
        bool inside;       ///< True if the peak is inside the fitted patch.
    };

    /**
    // The "findPeak3by3" function assumes that the grid locations
    // are -1, 0, and 1 in both directions. Correct shifting
    // and scaling is up to the user of this function (use the
    // "rescaleCoords" method of the Peak2D class). The function
    // returns "true" if a valid peak is found inside the 3x3 mask
    // and "false" otherwise.
    */
    bool findPeak3by3(const double gridValues[3][3], Peak2D* peak);

    /**
    // The "findPeak5by5" function assumes that the grid locations
    // are -2, -1, 0, 1, and 2 in both directions. Correct shifting
    // and scaling is up to the user of this function (use the
    // "rescaleCoords" method of the Peak2D class). The function 
    // returns "true" if a valid peak is found inside the 5x5 mask
    // and "false" otherwise.
    */
    bool findPeak5by5(const double gridValues[5][5], Peak2D* peak);
}

#endif // NPSTAT_FINDPEAK2D_HH_
