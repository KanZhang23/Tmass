#include "npstat/stat/DistributionTransform1DReader.hh"

namespace npstat {
    AbsDistributionTransform1D* AbsDistributionTransform1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        return StaticDistributionTransform1DReader::instance().read(id, in);
    }
}
