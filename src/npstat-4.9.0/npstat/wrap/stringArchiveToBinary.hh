#ifndef NPSTAT_STRINGARCHIVETOBINARY_HH_
#define NPSTAT_STRINGARCHIVETOBINARY_HH_

#include <sstream>
#include <istream>
#include <stdexcept>
#include <cassert>

#include "Python.h"

#include "geners/GenericIO.hh"
#include "geners/StringArchive.hh"

#include "npstat/wrap/OneShotReadBuf.hh"

namespace npstat {
    PyObject* stringArchiveToBinary(const gs::StringArchive& ar)
    {
        std::ostringstream out(std::ios_base::out | std::ios_base::binary);
        const_cast<gs::StringArchive&>(ar).flush();
        if (!gs::write_item(out, ar))
            throw std::runtime_error("In npstat::stringArchiveToBinary: "
                                     "failed to write input archive into "
                                     "std::ostringstream. Not enough memory?");
        const std::string& rep = out.str();
        const std::size_t len = rep.size();
        assert(len);
        return PyByteArray_FromStringAndSize(&rep[0], len);
    }

    gs::StringArchive* stringArchiveFromBinary(PyObject* object)
    {
        if (!PyByteArray_Check(object))
            throw std::invalid_argument("In npstat::stringArchiveFromBinary: "
                                        "input object is not PyByteArray");
        char* rep = PyByteArray_AS_STRING(object);
        const std::size_t len = PyByteArray_GET_SIZE(object);
        if (rep == 0 || len == 0)
            throw std::invalid_argument("In npstat::stringArchiveFromBinary: "
                                        "PyByteArray is empty or invalid");
        Private::OneShotReadBuf buf(rep, len);
        std::istream in(&buf);
        CPP11_auto_ptr<gs::StringArchive> ar = 
            gs::read_item<gs::StringArchive>(in);
        return ar.release();
    }
}

#endif // NPSTAT_STRINGARCHIVETOBINARY_HH_
