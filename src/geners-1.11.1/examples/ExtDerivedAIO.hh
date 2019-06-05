#ifndef GSEXAMPLE_EXTDERIVEDAIO_HH_
#define GSEXAMPLE_EXTDERIVEDAIO_HH_

// The header for the class which we need to wrap
#include "ExtDerivedA.hh"

// The header for the I/O factory
#include "ExtBaseIO.hh"

// Read/write wrapper for class ExtDerivedA.
// Note publication of ExtDerivedA as "wrapped_type".
struct ExtDerivedAIO : public gs::AbsReaderWriter<ExtBase>
{
    typedef ExtDerivedA wrapped_type;

    // Methods that have to be overriden from the base
    bool write(std::ostream&, const wrapped_base&, bool dumpId) const override;
    wrapped_type* read(const gs::ClassId& id, std::istream& in) const override;

    // The class id for ExtDerivedA will be needed both in the "read" and
    // in the "write" methods. Because of this, we will just return it from
    // one static function.
    static const gs::ClassId& wrappedClassId();
};

// Operators checking for equality (useful for I/O testing)
inline bool operator==(const ExtDerivedA& l, const ExtDerivedA& r)
{
    return l.getValue() == r.getValue();
}

inline bool operator!=(const ExtDerivedA& l, const ExtDerivedA& r)
{
    return !(l == r);
}

// Call the necessary macros to register the ExtDerivedA class with Geners
gs_specialize_class_id(ExtDerivedA, 1)
gs_declare_type_external(ExtDerivedA)
gs_associate_serialization_factory(ExtDerivedA, StaticIOFactoryForExtBase)

#endif // GSEXAMPLE_EXTDERIVEDAIO_HH_
