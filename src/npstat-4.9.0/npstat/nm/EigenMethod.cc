#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "npstat/nm/EigenMethod.hh"

static const char* names[] = {
    "EIGEN_SIMPLE",
    "EIGEN_D_AND_C",
    "EIGEN_RRR"
};

static const unsigned nNames = sizeof(names)/sizeof(names[0]);

namespace npstat {
    EigenMethod parseEigenMethod(const char* methodName)
    {
        for (unsigned i=0; i<nNames; ++i)
            if (strcmp(names[i], methodName) == 0)
                return static_cast<EigenMethod>(i);
        std::ostringstream os;
        os << "In npstat::parseEigenMethod: invalid argument \""
           << methodName << "\". Must be one of "
           << validEigenMethodNames() << '.';
        throw std::invalid_argument(os.str());
    }

    const char* eigenMethodName(EigenMethod m)
    {
        const unsigned i = static_cast<unsigned>(m);
        assert(i < nNames);
        return names[i];
    }

    std::string validEigenMethodNames()
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
