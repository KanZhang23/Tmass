#include "SerializableBaseReader.hh"

unsigned long SerializableBase::nextObjectNumber()
{
    static unsigned long objectCounter = 0UL;
    return objectCounter++;
}

SerializableBase* SerializableBase::read(const gs::ClassId& id,
                                         std::istream& in)
{
    return StaticSerializableBaseReader::instance().read(id, in);
}
