#include "geners/binaryIO.hh"
#include "npstat/stat/Distribution1DReader.hh"

namespace npstat {
    bool AbsScalableDistribution1D::write(std::ostream& os) const
    {
        gs::write_pod(os, location_);
        gs::write_pod(os, scale_);
        return !os.fail();
    }

    bool AbsScalableDistribution1D::read(std::istream& is,
                                         double* location, double* scale)
    {
        gs::read_pod(is, location);
        gs::read_pod(is, scale);
        return !is.fail();
    }

    AbsDistribution1D* AbsDistribution1D::read(const gs::ClassId& id,
                                               std::istream& in)
    {
        return StaticDistribution1DReader::instance().read(id, in);
    }
}
