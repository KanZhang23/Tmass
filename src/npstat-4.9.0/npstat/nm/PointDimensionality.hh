#ifndef NPSTAT_POINTDIMENSIONALITY_HH_
#define NPSTAT_POINTDIMENSIONALITY_HH_

/*!
// \file PointDimensionality.hh
//
// \brief Compile-time dimensionality detector for classes like std::array
//
// Author: I. Volobouev
//
// October 2010
*/

#include "geners/CPP11_array.hh"

namespace npstat {
    /** Compile-time dimensionality detector for classes like std::array */
    template <class T>
    struct PointDimensionality
    {
        enum {dim_size = T::dim_size};
    };

    template <class T, std::size_t N>
    struct PointDimensionality<CPP11_array<T,N> >
    {
        enum {dim_size = N};
    };
}

#endif // NPSTAT_POINTDIMENSIONALITY_HH_
