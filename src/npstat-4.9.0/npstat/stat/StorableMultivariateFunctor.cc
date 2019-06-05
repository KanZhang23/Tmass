#include <stdexcept>

#include "npstat/stat/StorableMultivariateFunctorReader.hh"

namespace npstat {
    void StorableMultivariateFunctor::validateDescription(
        const std::string& description) const
    {
        if (description_ != description) 
        {
            std::string mesage = 
                "In npstat::StorableMultivariateFunctor::validateDescription: "
                "argument description string \"";
            mesage += description;
            mesage += "\" is different from the object description string \"";
            mesage += description_;
            mesage += "\"";
            throw std::runtime_error(mesage.c_str());
        }
    }

    StorableMultivariateFunctor* StorableMultivariateFunctor::read(
        const gs::ClassId& id, std::istream& in)
    {
        return StaticStorableMultivariateFunctorReader::instance().read(id, in);
    }
}
