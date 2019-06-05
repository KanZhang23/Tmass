#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "npstat/nm/SvdMethod.hh"

static const char* names[] = {
    "SVD_SIMPLE",
    "SVD_D_AND_C"
};

static const unsigned nNames = sizeof(names)/sizeof(names[0]);

namespace npstat {
    SvdMethod parseSvdMethod(const char* methodName)
    {
        for (unsigned i=0; i<nNames; ++i)
            if (strcmp(names[i], methodName) == 0)
                return static_cast<SvdMethod>(i);
        std::ostringstream os;
        os << "In npstat::parseSvdMethod: invalid argument \""
           << methodName << "\". Must be one of "
           << validSvdMethodNames() << '.';
        throw std::invalid_argument(os.str());
    }

    const char* svdMethodName(SvdMethod m)
    {
        const unsigned i = static_cast<unsigned>(m);
        assert(i < nNames);
        return names[i];
    }

    std::string validSvdMethodNames()
    {
        std::ostringstream os;
        for (unsigned i=0; i<nNames; ++i)
        {
            if (i == 0)
                os << names[i];
            else if (i == nNames - 1)
                os << " or " << names[i];
            else
                os << ", " << names[i];
        }
        return os.str();
    }
}
