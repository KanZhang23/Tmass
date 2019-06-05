#include "npstat/stat/LocalPolyFilter1DReader.hh"

#define add_reader(Derived) do {\
     const gs::ClassId& id(gs::ClassId::makeId<Derived >());\
     (*this)[id.name()] = new gs::ConcreteReader<LocalPolyFilter1D, Derived >();\
} while(0);

namespace npstat {
    LocalPolyFilter1DReader::LocalPolyFilter1DReader()
    {
        add_reader(DummyLocalPolyFilter1D);
    }
}
