#ifndef NPSTAT_NTUPLECOLUMNTONUMPY_HH_
#define NPSTAT_NTUPLECOLUMNTONUMPY_HH_

#include <stdexcept>
#include <sstream>

#include "npstat/stat/AbsNtuple.hh"
#include "npstat/wrap/NumpyTypecode.hh"

namespace npstat {
    template <typename T>
    PyObject* ntupleColumnToNumpy(const AbsNtuple<T>& nt, const Column& c)
    {
        const int typenum = NumpyTypecode<T>::code;

        if (typenum == NPY_NOTYPE)
        {
            std::ostringstream os;
            os << "In npstat::ntupleColumnToNumpy: don't know how to "
               << "convert column data from " << nt.classId().name()
               << " to numpy array";
            throw std::invalid_argument(os.str());
        }

        if (!c.isValid(nt)) throw std::invalid_argument(
            "In npstat::ntupleColumnToNumpy: invalid column specification");

        npy_intp sh[1];
        sh[0] = nt.nRows();
        PyObject* array = PyArray_SimpleNew(1, sh, typenum);
        if (array)
        {
            PyArrayObject* a = reinterpret_cast<PyArrayObject*>(array);
            try {
                nt.columnContents(c, (T*)(PyArray_DATA(a)), sh[0]);
            }
            catch (...) {
                Py_DECREF(array);
                throw;
            }
        }
        return array;
    }

    template <typename T>
    PyObject* ntupleRowToNumpy(const AbsNtuple<T>& nt, const unsigned long row)
    {
        const int typenum = NumpyTypecode<T>::code;

        if (typenum == NPY_NOTYPE)
        {
            std::ostringstream os;
            os << "In npstat::ntupleRowToNumpy: don't know how to "
               << "convert row data from " << nt.classId().name()
               << " to numpy array";
            throw std::invalid_argument(os.str());
        }

        if (row >= nt.nRows())
            throw std::out_of_range("In npstat::ntupleRowToNumpy: "
                                    "row number is out of range");

        npy_intp sh[1];
        sh[0] = nt.nColumns();
        PyObject* array = PyArray_SimpleNew(1, sh, typenum);
        if (array)
        {
            PyArrayObject* a = reinterpret_cast<PyArrayObject*>(array);
            try {
                nt.rowContents(row, (T*)(PyArray_DATA(a)), sh[0]);
            } 
            catch (...) {
                Py_DECREF(array);
                throw;
            }            
        }
        return array;
    }    
}

#endif // NPSTAT_NTUPLECOLUMNTONUMPY_HH_
