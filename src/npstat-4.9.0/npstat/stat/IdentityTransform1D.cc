#include "npstat/stat/IdentityTransform1D.hh"

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

namespace npstat {
    bool IdentityTransform1D::write(std::ostream& os) const
    {
        gs::write_pod(os, p_);
        return !os.fail();
    }

    IdentityTransform1D* IdentityTransform1D::read(const gs::ClassId& id,
                                                   std::istream& is)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<IdentityTransform1D>());
        current.ensureSameId(id);
        double tmp;
        gs::read_pod(is, &tmp);
        if (is.fail()) throw gs::IOReadFailure(
            "In npstat::IdentityTransform1D::read: input stream failure");
        return new IdentityTransform1D(tmp);
    }
}
