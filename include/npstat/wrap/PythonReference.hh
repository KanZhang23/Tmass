#ifndef NPSTAT_PYTHONREFERENCE_HH_
#define NPSTAT_PYTHONREFERENCE_HH_

#include "geners/AbsReference.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/wrap/PyClassId.hh"
#include "npstat/wrap/pickle_item.hh"

namespace gs {
    class Ref_PyObject : public AbsReference
    {
    public:
        inline Ref_PyObject(AbsArchive& ar, const unsigned long long itemId)
            : AbsReference(ar, ClassId::makeId<PyObject>(),
                           "gs::Single", itemId) {}

        inline Ref_PyObject(AbsArchive& ar, const char* name,
                            const char* category)
            :  AbsReference(ar, ClassId::makeId<PyObject>(),
                            "gs::Single", name, category) {}

        inline Ref_PyObject(AbsArchive& ar, const SearchSpecifier& namePattern,
                            const SearchSpecifier& categoryPattern)
            :  AbsReference(ar, ClassId::makeId<PyObject>(), "gs::Single",
                            namePattern, categoryPattern) {}

        inline virtual ~Ref_PyObject() {}

        // Method to retrieve the item
        PyObject* retrieve(unsigned long index) const
        {
            const unsigned long long itemId = this->id(index);
            assert(itemId);
            std::istream& is = this->positionInputStream(itemId);
            unsigned long sz = 0;
            read_pod(is, &sz);
            PyObject* str = PyBytes_FromStringAndSize((const char *)0, sz);
            //
            // Now, we own "str" and must DECREF it before returning
            // from this scope
            //
            if (PyErr_Occurred())
            {
                Py_XDECREF(str);
                PyErr_Print();
                throw IOInvalidData("In gs::Ref_PyObject::retrieve: "
                                    "failed to create a read buffer");
            }
            char* data = PyBytes_AsString(str);
            if (sz)
                read_pod_array(is, data, sz);
            if (is.fail())
            {
                Py_DECREF(str);
                throw IOInvalidData("In gs::Ref_PyObject::retrieve: "
                                    "failed to read object data");
            }
            PyObject* p = npstat::unpickle_item(str);
            Py_DECREF(str);
            if (PyErr_Occurred())
            {
                Py_XDECREF(p);
                PyErr_Print();
                throw IOInvalidData("In gs::Ref_PyObject::retrieve: "
                                    "failed to unpickle object data");
            }
            return p;
        }

        inline PyObject* getValue(unsigned long index) const
        {
            return retrieve(index);
        }

    private:
        Ref_PyObject();
    };
}

#endif // NPSTAT_PYTHONREFERENCE_HH_
