#ifndef NPSTAT_ARRAYNDTONUMPY_HH_
#define NPSTAT_ARRAYNDTONUMPY_HH_

#include <stdexcept>
#include <sstream>

#include "npstat/wrap/NumpyTypecode.hh"

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/Matrix.hh"

namespace npstat {
    template <typename Numeric, unsigned conversion=NumpyConvert::NONE>
    struct ArrayNDToNumpyConverter
    {
        static PyObject* convert(const Numeric* data, const unsigned long len,
                                 npy_intp *sh, const unsigned dim,
                                 const bool makeCopy)
        {
            typedef typename NumpyTypecode<Numeric>::c_type Ctype;
            const int typenum = NumpyTypecode<Numeric>::code;
            PyObject* array = NULL;
            if (makeCopy)
            {
                array = PyArray_SimpleNew(dim, sh, typenum);
                if (array)
                    copyBuffer((Ctype*)array_data(array), data, len);
            }
            else
                array = PyArray_SimpleNewFromData(
                    dim, sh, typenum, const_cast<Ctype*>(data));
            return array;
        }
    };


    template <typename Numeric>
    struct ArrayNDToNumpyConverter<Numeric, NumpyConvert::BOOL>
    {
        static PyObject* convert(const Numeric* data, const unsigned long len,
                                 npy_intp *sh, const unsigned dim,
                                 const bool /* makeCopy */)
        {
            // Convert by copy only, can't just have a wrapper
            // to existing data
            typedef typename NumpyTypecode<Numeric>::c_type Ctype;
            const int typenum = NumpyTypecode<Numeric>::code;
            PyObject* array = PyArray_SimpleNew(dim, sh, typenum);
            if (array)
                copyBuffer((Ctype*)array_data(array), data, len);
            return array;
        }
    };


    template <typename Numeric>
    struct ArrayNDToNumpyConverter<Numeric, NumpyConvert::COMPLEX>
    {
        static PyObject* convert(const Numeric* data, const unsigned long len,
                                 npy_intp *sh, const unsigned dim,
                                 const bool /* makeCopy */)
        {
            typedef typename NumpyTypecode<Numeric>::c_type Complex;
            const int typenum = NumpyTypecode<Numeric>::code;
            PyObject* array = PyArray_SimpleNew(dim, sh, typenum);
            if (array)
            {
                Complex* to = (Complex *)array_data(array);
                for (unsigned long i=0; i<len; ++i)
                {
                    to[i].real = data[i].real();
                    to[i].imag = data[i].imag();
                }
            }
            return array;
        }
    };


    template <typename Numeric>
    PyObject* arrayNDToNumpy(const ArrayND<Numeric>& arr, bool copy=true)
    {
        static const int typenum = NumpyTypecode<Numeric>::code;
        static const int conversion = NumpyTypecode<Numeric>::conversion;

        if (typenum == NPY_NOTYPE)
        {
            std::ostringstream os;
            os << "In npstat::arrayNDToNumpy: don't know how to convert "
               << ArrayND<Numeric>::classname();
            throw std::invalid_argument(os.str());
        }

        const unsigned dim = arr.rank();
        npy_intp sh[CHAR_BIT*sizeof(unsigned long)];
        assert(dim <= sizeof(sh)/sizeof(sh[0]));
        for (unsigned i=0; i<dim; ++i)
            sh[i] = arr.shapeData()[i];

        return ArrayNDToNumpyConverter<Numeric,conversion>::convert(
            arr.data(), arr.length(), sh, dim, copy);
    }


    template <typename Numeric>
    PyObject* matrixToNumpy(const Matrix<Numeric>& m, bool copy=true)
    {
        static const int typenum = NumpyTypecode<Numeric>::code;
        static const int conversion = NumpyTypecode<Numeric>::conversion;

        if (typenum == NPY_NOTYPE)
        {
            std::ostringstream os;
            os << "In npstat::matrixToNumpy: don't know how to convert "
               << Matrix<Numeric>::classname();
            throw std::invalid_argument(os.str());
        }

        npy_intp sh[2];
        sh[0] = m.nRows();
        sh[1] = m.nColumns();

        return ArrayNDToNumpyConverter<Numeric,conversion>::convert(
            m.data(), m.length(), sh, 2U, copy);
    }
}

#endif // NPSTAT_ARRAYNDTONUMPY_HH_
