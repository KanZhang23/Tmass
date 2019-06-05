#ifndef GENERS_ARRAYRECORD_HH_
#define GENERS_ARRAYRECORD_HH_

#include "geners/ArchiveRecord.hh"
#include "geners/ArrayAdaptor.hh"

namespace gs {
    template <typename T>
    inline ArchiveRecord<ArrayAdaptor<T> > ArrayRecord(
        const T* arr, const std::size_t sz,
        const char* name, const char* category)
    {
        return ArchiveRecord<ArrayAdaptor<T> >(
            ArrayAdaptor<T>(arr, sz), name, category);
    }
}

#endif // GENERS_ARRAYRECORD_HH_
