#include <sstream>

#include "geners/IOException.hh"
#include "npstat/stat/distributionReadError.hh"

namespace npstat {
    void distributionReadError(std::istream& in, const char* classname)
    {
        std::ostringstream os;
        os << "In " << classname << "::read: ";
        if (in.fail())
        {
            os << "input stream failure";
            throw gs::IOReadFailure(os.str());
        }
        else
        {
            os << "invalid distribution data";
            throw gs::IOInvalidData(os.str());
        }
    }
}
