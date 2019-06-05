#include "npstat/stat/DiscreteDistribution1DReader.hh"

namespace npstat {
    AbsDiscreteDistribution1D* AbsDiscreteDistribution1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        return StaticDiscreteDistribution1DReader::instance().read(id, in);
    }
}
