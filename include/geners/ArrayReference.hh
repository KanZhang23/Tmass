#ifndef GENERS_ARRAYREFERENCE_HH_
#define GENERS_ARRAYREFERENCE_HH_

#include <cstddef>
#include "geners/AbsReference.hh"

namespace gs {
    template <typename T>
    class ArrayReference : public AbsReference
    {
    public:
        inline ArrayReference(AbsArchive& ar, const unsigned long long itemId)
            : AbsReference(ar, ClassId::makeId<T>(), "gs::Array", itemId) {}

        inline ArrayReference(AbsArchive& ar,
                              const char* name, const char* category)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Array",
                            name, category) {}

        inline ArrayReference(AbsArchive& ar,
                              const SearchSpecifier& namePattern,
                              const SearchSpecifier& categoryPattern)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Array",
                            namePattern, categoryPattern) {}

        // There is only one method to retrieve an array
        void restore(unsigned long idx, T* arr, std::size_t len) const;

    private:
        ArrayReference();
    };
}

#include "geners/ArrayReference.icc"

#endif // GENERS_ARRAYREFERENCE_HH_
