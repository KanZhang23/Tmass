#ifndef NPSTAT_GENERALIZEDCOMPLEX_HH_
#define NPSTAT_GENERALIZEDCOMPLEX_HH_

/*!
// \file GeneralizedComplex.hh
//
// \brief Typedef complex type if it makes sense (parameter is real);
//        otherwise typedef the original type
//
// Author: I. Volobouev
//
// October 2013
*/

#include <complex>

namespace npstat {
    template <typename T>
    struct GeneralizedComplex
    {
        typedef T type;
    };

    template <>
    struct GeneralizedComplex<float>
    {
        typedef std::complex<float> type;
    };

    template <>
    struct GeneralizedComplex<double>
    {
        typedef std::complex<double> type;
    };

    template <>
    struct GeneralizedComplex<long double>
    {
        typedef std::complex<long double> type;
    };
}

#endif // NPSTAT_GENERALIZEDCOMPLEX_HH_
