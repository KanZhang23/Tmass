#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "npstat/nm/OrthoPolyMethod.hh"

static const char* names[] = {
    "OPOLY_STIELTJES",
    "OPOLY_LANCZOS"
};

static const unsigned nNames = sizeof(names)/sizeof(names[0]);

namespace npstat {
    OrthoPolyMethod parseOrthoPolyMethod(const char* methodName)
    {
        for (unsigned i=0; i<nNames; ++i)
            if (strcmp(names[i], methodName) == 0)
                return static_cast<OrthoPolyMethod>(i);
        std::ostringstream os;
        os << "In npstat::parseOrthoPolyMethod: invalid argument \""
           << methodName << "\". Must be one of "
           << validOrthoPolyMethodNames() << '.';
        throw std::invalid_argument(os.str());
    }

    const char* orthoPolyMethodName(OrthoPolyMethod m)
    {
        const unsigned i = static_cast<unsigned>(m);
        assert(i < nNames);
        return names[i];
    }

    std::string validOrthoPolyMethodNames()
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
