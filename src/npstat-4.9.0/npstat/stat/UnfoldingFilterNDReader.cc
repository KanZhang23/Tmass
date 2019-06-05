#include "npstat/stat/UnfoldingFilterNDReader.hh"

#include "npstat/stat/SequentialPolyFilterND.hh"
#include "npstat/stat/LocalPolyFilterND.hh"

#define add_reader(Derived) do {\
    const gs::ClassId& id(gs::ClassId::makeId<Derived >());\
    (*this)[id.name()] = new gs::ConcreteReader<AbsUnfoldingFilterND,Derived >();\
} while(0);

namespace npstat {
    UnfoldingFilterNDReader::UnfoldingFilterNDReader()
    {
        add_reader(UnfoldingFilterND<SequentialPolyFilterND>);
        add_reader(UnfoldingFilterND<LocalPolyFilterND<4U> >);
    }
}
