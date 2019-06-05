#ifndef GENERS_ASSOCIATE_SERIALIZATION_FACTORY_HH_
#define GENERS_ASSOCIATE_SERIALIZATION_FACTORY_HH_

#include "geners/CPP11_config.hh"

#ifdef CPP11_STD_AVAILABLE

#include <memory>
#include <sstream>
#include <typeinfo>
#include <stdexcept>

#include "geners/GenericIO.hh"
#include "geners/assignIfPossible.hh"

#define gs_associate_serialization_factory(classname, factoryname) /**/       \
namespace gs {                                                                \
    template <class Stream, class State>                                      \
    struct GenericWriter<Stream, State, classname,                            \
                         Int2Type<IOTraits<int>::ISEXTERNAL> >                \
    {                                                                         \
        inline static bool process(const classname & a, Stream& os,           \
                                   State*, const bool processClassId)         \
            {return factoryname ::instance().write(os, a, processClassId);}   \
    };                                                                        \
    template <class Stream, class State>                                      \
    struct GenericReader<Stream, State, classname,                            \
                         Int2Type<IOTraits<int>::ISEXTERNAL> >                \
    {                                                                         \
        inline static bool readIntoPtr(classname *& ptr, Stream& is,          \
                                       State* st, const bool processClassId)  \
        {                                                                     \
            static const ClassId current(ClassId::makeId< classname >());     \
            const ClassId& id = processClassId?ClassId(is,1):st->back();      \
            auto tmp = factoryname ::instance().read(id, is);                 \
            classname * rptr = dynamic_cast< classname *>(tmp);               \
            if (!rptr)                                                        \
            {                                                                 \
                delete tmp;                                                   \
                std::ostringstream os;                                        \
                os << "In gs::GenericReader::readIntoPtr: failed to "         \
                   << "obtain pointer to \"" << current.name()                \
                   << "\" from pointer to \"" << id.name() << '"';            \
                throw std::runtime_error(os.str());                           \
            }                                                                 \
            if (ptr)                                                          \
            {                                                                 \
                std::unique_ptr< classname > utmp(rptr);                      \
                const std::type_info& sT(typeid(*rptr));                      \
                const bool typeMatches = typeid(classname) == sT &&           \
                                         typeid(*ptr) == sT;                  \
                if (typeMatches)                                              \
                {                                                             \
                  if (!assignIfPossible(*ptr, *rptr))                         \
                  {                                                           \
                    std::ostringstream os;                                    \
                    os << "In gs::GenericReader::readIntoPtr: can not restore"\
                       << " object of type \"" << current.name() << "\" on"   \
                       << " the stack, the type is not assignable. "          \
                       << " Use heap retrieval methods instead.";             \
                    throw std::runtime_error(os.str());                       \
                  }                                                           \
                }                                                             \
                else                                                          \
                {                                                             \
                    std::ostringstream os;                                    \
                    os << "In gs::GenericReader::readIntoPtr: can not restore"\
                       << " object of type \"" << current.name() << "\" on"   \
                       << " the stack, slicing would occur.";                 \
                    throw std::runtime_error(os.str());                       \
                }                                                             \
            }                                                                 \
            else                                                              \
                ptr = rptr;                                                   \
            return true;                                                      \
        }                                                                     \
        inline static bool process(classname & s, Stream& is,                 \
                                   State* st, const bool processClassId)      \
        {                                                                     \
            classname * ps = &s;                                              \
            return readIntoPtr(ps, is, st, processClassId);                   \
        }                                                                     \
    };                                                                        \
}

#endif // CPP11_STD_AVAILABLE

#endif // GENERS_ASSOCIATE_SERIALIZATION_FACTORY_HH_
