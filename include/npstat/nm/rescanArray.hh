#ifndef NPSTAT_RESCANARRAY_HH_
#define NPSTAT_RESCANARRAY_HH_

/*!
// \file rescanArray.hh
//
// \brief Fill a multidimensional array using values from another array
//        with a different shape
//
// Author: I. Volobouev
//
// October 2010
*/

#include "npstat/nm/ArrayND.hh"

namespace npstat {
    /**
    // A utility for filling one array using values of another. The
    // array shapes do not have to be the same but the ranks have to be.
    // Roughly, the arrays are treated as values of histogram bins inside
    // the unit box. The array "to" is filled either with the closest bin
    // value of the array "from" or with an interpolated value (if 
    // "interpolationDegree" parameter is not 0).
    //
    // interpolationDegree parameter must be one of 0, 1, or 3.
    */
    template<typename Num1, unsigned Len1, unsigned Dim1,
             typename Num2, unsigned Len2, unsigned Dim2>
    void rescanArray(const ArrayND<Num1,Len1,Dim1>& from,
                     ArrayND<Num2,Len2,Dim2>* to,
                     unsigned interpolationDegree=0);
}

#include "npstat/nm/rescanArray.icc"

#endif // NPSTAT_RESCANARRAY_HH_
