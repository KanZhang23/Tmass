#ifndef NPSTAT_RECTANGLEQUADRATURE_HH_
#define NPSTAT_RECTANGLEQUADRATURE_HH_

/*!
// \file rectangleQuadrature.hh
//
// \brief Gaussian quadratures on rectangles and hyperrectangles using tensor
//        product integration
//
// Author: I. Volobouev
//
// December 2010
*/

#include "npstat/nm/AbsMultivariateFunctor.hh"

namespace npstat {
    /**
    // The "integrationPoints" parameter below must be one of the number
    // of points supported by the GaussLegendreQuadrature class. See the
    // corresponding header file for the list of allowed values.
    //
    // Naturally, length of the "rectangleCenter" and "rectangleSize"
    // arrays (with obvious meaning) should be at least "dim".
    */
    double rectangleIntegralCenterAndSize(const AbsMultivariateFunctor& f,
                                          const double* rectangleCenter,
                                          const double* rectangleSize,
                                          unsigned dim,
                                          unsigned integrationPoints);
}

#endif // NPSTAT_RECTANGLEQUADRATURE_HH_
