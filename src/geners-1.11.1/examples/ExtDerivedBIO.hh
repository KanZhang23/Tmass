#ifndef GSEXAMPLE_EXTDERIVEDBIO_HH_
#define GSEXAMPLE_EXTDERIVEDBIO_HH_

// The header for the class which we need to wrap
#include "ExtDerivedB.hh"

// The header for the I/O factory
#include "ExtBaseIO.hh"

// Read/write wrapper for class ExtDerivedB.
// Note publication of ExtDerivedB as "wrapped_type".
struct ExtDerivedBIO : public gs::AbsReaderWriter<ExtBase>
{
    typedef ExtDerivedB wrapped_type;

    // Methods that have to be overriden from the base
    bool write(std::ostream&, const wrapped_base&, bool dumpId) const override;
    wrapped_type* read(const gs::ClassId& id, std::istream& in) const override;

    // The class id for ExtDerivedB will be needed both in the "read" and
    // in the "write" methods. Because of this, we will just return it from
    // one static function.
    static const gs::ClassId& wrappedClassId();
};

// Operators checking for equality (useful for I/O testing)
inline bool operator==(const ExtDerivedB& l, const ExtDerivedB& r)
{
    return l.getValue() == r.getValue() && l.getAnother() == r.getAnother();
}

inline bool operator!=(const ExtDerivedB& l, const ExtDerivedB& r)
{
    return !(l == r);
}

// Call the necessary macros to register the ExtDerivedB class with Geners
gs_specialize_class_id(ExtDerivedB, 1)
gs_declare_type_external(ExtDerivedB)
gs_associate_serialization_factory(ExtDerivedB, StaticIOFactoryForExtBase)

#endif // GSEXAMPLE_EXTDERIVEDBIO_HH_
