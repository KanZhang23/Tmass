// This example works only if C++11 is supported
#include "geners/CPP11_config.hh"
#ifdef CPP11_STD_AVAILABLE

// Header file for the factory
#include "ExtBaseIO.hh"

// Header file for all I/O wrapper classes
#include "ExtDerivedAIO.hh"
#include "ExtDerivedBIO.hh"

// Register the I/O wrappers with the factory
IOFactoryForExtBase::IOFactoryForExtBase()
{
    registerWrapper<ExtDerivedAIO>();
    registerWrapper<ExtDerivedBIO>();
}

// Note that it is also possible to register wrappers in any
// other place in the code (but before performing I/O for the
// affected wrapped classes) by calling
//
// StaticIOFactoryForExtBase::registerWrapper<SomeWrapperClass>();

#endif // CPP11_STD_AVAILABLE
