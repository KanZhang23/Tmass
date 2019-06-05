#ifndef NPSTAT_PYTHONRECORD_HH_
#define NPSTAT_PYTHONRECORD_HH_

#include <stdexcept>

#include "geners/AbsRecord.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/wrap/PyClassId.hh"
#include "npstat/wrap/pickle_item.hh"

namespace gs {
    class PythonRecord : public AbsRecord
    {
    public:
        // The PyObject pointee in the constructor must be a Python 
        // PyBytesObject. We do not know (and do not want to know) how
        // to write anything else -- all other Python objects must be
        // converted to a Python PyBytesObject by "pickle_item" or other
        // similar tools before getting here.
        //
        // PythonRecord assumes that it is managing the Python object, so it
        // will DECREF the object in the destructor but it WILL NOT INCREF
        // it in the main constructor.
        //
        inline PythonRecord(PyObject* object, const char* name,
                            const char* category)
            : AbsRecord(ClassId::makeId<PyObject>(), "gs::Single",
                        name, category), obj_(object)
        {
            assert(obj_);
            if (!PyBytes_Check(obj_)) throw std::invalid_argument(
                "In gs::PythonRecord: input object is not a PyBytes object");
        }

        inline PythonRecord(const PythonRecord& other)
            : AbsRecord(other), obj_(other.obj_)
        {
            Py_INCREF(obj_);
        }

        inline PythonRecord& operator=(const PythonRecord& other)
        {
            if (&other != this)
            {
                AbsRecord::operator=(other);
                Py_INCREF(other.obj_);
                Py_DECREF(obj_);
                obj_ = other.obj_;
            }
            return *this;
        }

        inline virtual ~PythonRecord()
        {
            Py_DECREF(obj_);
        }

    private:
        PythonRecord();

        inline virtual bool writeData(std::ostream& of) const
        {
            const unsigned long sz = PyBytes_GET_SIZE(obj_);
            write_pod(of, sz);
            if (sz)
            {
                const char* data = PyBytes_AsString(obj_);
                assert(data);
                write_pod_array(of, data, sz);
            }
            return !of.fail();
        }

        PyObject* obj_;
    };


    inline PythonRecord NPRecord(
        PyObject* object, const char* name, const char* category)
    {
        PyObject* p = npstat::pickle_item(object);
        if (PyErr_Occurred())
        {
            Py_XDECREF(p);
            PyErr_Print();
            throw std::invalid_argument(
                "In gs::Record: failed to pickle object data");
        }
        return PythonRecord(p, name, category);
    }
}

#endif // NPSTAT_PYTHONRECORD_HH_
