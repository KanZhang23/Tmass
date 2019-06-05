#include "SerializableBaseReader.hh"

// Include headers for all classes derived from SerializableBase
// which are known at this point in code development
#include "SerializableDerivedA.hh"
#include "SerializableDerivedB.hh"

// Simple macro for adding a reader for a class derived from SerializableBase
#define add_reader(Derived) do {                                              \
    const gs::ClassId& id(gs::ClassId::makeId<Derived >());                   \
    (*this)[id.name()] = new gs::ConcreteReader<SerializableBase,Derived >(); \
} while(0);

// Instead of adding each reader directly, you might be tempted
// to invoke the "registerClass" method of the associated static
// reader. Don't do that! Such a call will result in a recursive
// invocation of the singleton's "instance()" method -- this is
// something we really have to avoid.
//
SerializableBaseReader::SerializableBaseReader()
{
    add_reader(SerializableDerivedA);
    add_reader(SerializableDerivedB);
}
