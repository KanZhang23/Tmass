#ifndef NPSTAT_CONTINUOUSDEGREETAPER_HH_
#define NPSTAT_CONTINUOUSDEGREETAPER_HH_

/*!
// \file continuousDegreeTaper.hh
//
// \brief A function that makes simple tapers for use with LocalPolyFilter1D
//
// Author: I. Volobouev
//
// September 2013
*/

#include <vector>

namespace npstat {
    /**
    // The input argument "degree" must be non-negative (an exception
    // will be thrown if a negative value is given).
    //
    // If "degree" is an exact integer, the resulting vector "vtaper"
    // will have size degree + 1, with all elements set to 1.0.
    //
    // If "degree" is not an exact integer, the resulting vector "vtaper"
    // will have size ceil(degree) + 1, with all elements except the last one
    // set to 1.0. The last element will be set to sqrt(degree - floor(degree)).
    // For a certain definition of the number of effective degrees of
    // freedom, this results in a direct correspondence between the poly
    // degree and the number of degrees of freedom at large bandwidth values.
    //
    // The resulting vector can be used as an argument of the LocalPolyFilter1D
    // constructor, with constructor "taper" argument set to &vtaper[0] and
    // "maxDegree" argument set to (vtaper.size() - 1).
    */
    void continuousDegreeTaper(double degree, std::vector<double>* vtaper);
}

#endif // NPSTAT_CONTINUOUSDEGREETAPER_HH_
