#include "npstat/stat/LogTransform1D.hh"

namespace npstat {
    LogTransform1D* LogTransform1D::read(const gs::ClassId& id, std::istream&)
    {
        static const gs::ClassId current(gs::ClassId::makeId<LogTransform1D>());
        current.ensureSameId(id);
        return new LogTransform1D();
    }
}
