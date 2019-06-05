#ifndef NPSTAT_PICKLE_ITEM_HH_
#define NPSTAT_PICKLE_ITEM_HH_

#include <cassert>
#include "Python.h"

namespace npstat {
    // The following function converts an arbitrary Python object
    // into a PyBytesObject. It returns a new reference.
    //
    inline PyObject* pickle_item(PyObject* object)
    {
        assert(object);
        PyObject* pmod = PyImport_ImportModule("pickle");
        assert(pmod);
        PyObject* item = PyEval_CallMethod(pmod, "dumps", "(O)", object);
        Py_DECREF(pmod);
        return item;
    }

    // The following function restores an arbitrary Python object
    // from a Python PyBytesObject previously filled by "pickle_item".
    // It returns a new reference.
    //
    inline PyObject* unpickle_item(PyObject* object)
    {
        assert(object);
        PyObject* pmod = PyImport_ImportModule("pickle");
        assert(pmod);
        PyObject* item = PyEval_CallMethod(pmod, "loads", "(O)", object);
        Py_DECREF(pmod);
        return item;
    }
}

#endif // NPSTAT_PICKLE_ITEM_HH_
