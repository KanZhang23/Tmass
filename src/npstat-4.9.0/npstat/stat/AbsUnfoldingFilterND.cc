#include "npstat/stat/UnfoldingFilterNDReader.hh"

namespace npstat {
    AbsUnfoldingFilterND* AbsUnfoldingFilterND::read(const gs::ClassId& id,
                                                     std::istream& in)
    {
        return StaticUnfoldingFilterNDReader::instance().read(id, in);
    }
}
