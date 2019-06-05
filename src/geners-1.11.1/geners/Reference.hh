#ifndef GENERS_REFERENCE_HH_
#define GENERS_REFERENCE_HH_

#include "geners/CPP11_auto_ptr.hh"
#include "geners/CPP11_shared_ptr.hh"
#include "geners/AbsReference.hh"

namespace gs {
    template <typename T>
    class Reference : public AbsReference
    {
    public:
        inline Reference(AbsArchive& ar, const unsigned long long itemId)
            : AbsReference(ar, ClassId::makeId<T>(), "gs::Single", itemId) {}

        inline Reference(AbsArchive& ar, const char* name, const char* category)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Single",
                            name, category) {}

#ifndef SWIG
        inline Reference(AbsArchive& ar, const std::string& name,
                         const char* category)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Single",
                            name.c_str(), category) {}

        inline Reference(AbsArchive& ar, const char* name,
                         const std::string& category)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Single",
                            name, category.c_str()) {}

        inline Reference(AbsArchive& ar, const std::string& name,
                         const std::string& category)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Single",
                            name.c_str(), category.c_str()) {}
#endif

        inline Reference(AbsArchive& ar, const SearchSpecifier& namePattern,
                         const SearchSpecifier& categoryPattern)
            :  AbsReference(ar, ClassId::makeId<T>(), "gs::Single",
                            namePattern, categoryPattern) {}

        // Methods to retrieve the item
        void restore(unsigned long index, T* obj) const;
        CPP11_auto_ptr<T> get(unsigned long index) const;
        CPP11_shared_ptr<T> getShared(unsigned long index) const;

    private:
        Reference();
        T* getPtr(unsigned long index) const;
    };
}

#include "geners/Reference.icc"

#endif // GENERS_REFERENCE_HH_
