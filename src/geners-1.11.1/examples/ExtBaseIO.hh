#ifndef GSEXAMPLE_EXTBASEIO_HH_
#define GSEXAMPLE_EXTBASEIO_HH_

// The header for the top-level base class in the hierarchy
#include "ExtBase.hh"

// Header for the I/O factory template
#include "geners/AbsReaderWriter.hh"

// Header for the macro associating a class with the I/O factory
#include "geners/associate_serialization_factory.hh"

// I/O factory for classes derived from ExtBase.
// Note the absence of public constructors.
class IOFactoryForExtBase : public gs::DefaultReaderWriter<ExtBase>
{
    friend class gs::StaticReaderWriter<IOFactoryForExtBase>;
    IOFactoryForExtBase();
};

// I/O factory wrapped into a singleton
typedef gs::StaticReaderWriter<IOFactoryForExtBase> StaticIOFactoryForExtBase;

// If the top-level base is not pure virtual, you also need to develop
// the I/O wrapper for it. The corresponding declarations can be placed
// here, for example.

// Call the necessary macros to register the ExtBase class with Geners
gs_specialize_class_id(ExtBase, 1)
gs_declare_type_external(ExtBase)
gs_associate_serialization_factory(ExtBase, StaticIOFactoryForExtBase)

#endif // GSEXAMPLE_EXTBASEIO_HH_
