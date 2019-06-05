#ifndef NPSTAT_ARRAYNDFROMNUMPY_HH_
#define NPSTAT_ARRAYNDFROMNUMPY_HH_

#include <cassert>
#include <stdexcept>

#include "npstat/wrap/NumpyTypecode.hh"

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/Matrix.hh"

namespace npstat {
    template <typename Numeric, unsigned conversion>
    struct NumpyToArrayNDConverter
    {
        static void convert(Numeric* to, const Numeric* fromNumpy,
                            const unsigned long len)
        {
            typedef typename NumpyTypecode<Numeric>::c_type Ctype;
            const Ctype* from = reinterpret_cast<const Ctype*>(fromNumpy);
            copyBuffer(to, from, len);
        }
    };


    template <typename Numeric>
    struct NumpyToArrayNDConverter<Numeric, NumpyConvert::COMPLEX>
    {
        static void convert(Numeric* to, const Numeric* fromNumpy,
                            const unsigned long len)
        {
            typedef typename NumpyTypecode<Numeric>::c_type Ctype;
            if (len)
            {
                assert(to);
                assert(fromNumpy);
                const Ctype* from = reinterpret_cast<const Ctype*>(fromNumpy);
                for (unsigned long i=0; i<len; ++i)
                    to[i] = Numeric(from[i].real, from[i].imag);
            }
        }
    };


    template <typename Numeric>
    ArrayND<Numeric>* arrayNDFromNumpy(
        Numeric* IN_NUMPYARRAYND, long* shape, int np)
    {
        static const int conversion = NumpyTypecode<Numeric>::conversion;

        assert(IN_NUMPYARRAYND);
        assert(np >= 0);
        const unsigned dim = np;
        unsigned sh[CHAR_BIT*sizeof(unsigned long)];
        assert(dim <= sizeof(sh)/sizeof(sh[0]));
        if (dim)
            assert(shape);
        for (unsigned i=0; i<dim; ++i)
        {
            if (static_cast<unsigned long>(shape[i]) > UINT_MAX)
                throw std::invalid_argument("In npstat::arrayNDFromNumpy: array "
                                            "size exceeds npstat maximum");
            sh[i] = shape[i];
        }
        ArrayND<Numeric>* result = new ArrayND<Numeric>(sh, dim);
        NumpyToArrayNDConverter<Numeric,conversion>::convert(
            const_cast<Numeric*>(result->data()), IN_NUMPYARRAYND,
            result->length());
        return result;
    }


    ArrayND<unsigned char>* arrayNDFromNumpyBool(
        bool* IN_NUMPYARRAYND, long* shape, int np)
    {
        assert(IN_NUMPYARRAYND);
        assert(np >= 0);
        const unsigned dim = np;
        unsigned sh[CHAR_BIT*sizeof(unsigned long)];
        assert(dim <= sizeof(sh)/sizeof(sh[0]));
        if (dim)
            assert(shape);
        for (unsigned i=0; i<dim; ++i)
        {
            if (static_cast<unsigned long>(shape[i]) > UINT_MAX)
                throw std::invalid_argument("In npstat::arrayNDFromNumpyBool: "
                                            "array size exceeds npstat maximum");
            sh[i] = shape[i];
        }
        ArrayND<unsigned char>* result = new ArrayND<unsigned char>(sh, dim);
        copyBuffer(const_cast<unsigned char*>(result->data()),
                   reinterpret_cast<unsigned char*>(IN_NUMPYARRAYND),
                   result->length());
        return result;
    }


    template <typename Numeric>
    Matrix<Numeric>* matrixFromNumpy(
        Numeric* IN_NUMPYARRAYND, long* shape, int np)
    {
        static const int conversion = NumpyTypecode<Numeric>::conversion;

        assert(IN_NUMPYARRAYND);
        if (np != 2)
            throw std::invalid_argument("npstat::matrixFromNumpy: can only "
                                        "convert 2-d arrays");
        assert(shape);
        if (static_cast<unsigned long>(shape[0])*
            static_cast<unsigned long>(shape[1]) > UINT_MAX)
            throw std::invalid_argument("In npstat::matrixFromNumpy: matrix "
                                        "size exceeds npstat maximum");
        Matrix<Numeric>* result = new Matrix<Numeric>(shape[0], shape[1]);
        NumpyToArrayNDConverter<Numeric,conversion>::convert(
            const_cast<Numeric*>(result->data()), IN_NUMPYARRAYND,
            result->length());
        return result;
    }
}

#endif // NPSTAT_ARRAYNDFROMNUMPY_HH_
