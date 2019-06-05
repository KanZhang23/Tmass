//
// The reader factory for descendants of SerializableBase
//
#ifndef GSEXAMPLES_SERIALIZABLEBASEREADER_HH_
#define GSEXAMPLES_SERIALIZABLEBASEREADER_HH_

#include "geners/AbsReader.hh"
#include "SerializableBase.hh"

// Note that the folowing class does not have any public constructors.
// All application usage is through the gs::StaticReader wrapper.
//
class SerializableBaseReader : public gs::DefaultReader<SerializableBase>
{
    typedef gs::DefaultReader<SerializableBase> Base;
    friend class gs::StaticReader<SerializableBaseReader>;
    SerializableBaseReader();
};

typedef gs::StaticReader<SerializableBaseReader> StaticSerializableBaseReader;

#endif // GSEXAMPLES_SERIALIZABLEBASEREADER_HH_
