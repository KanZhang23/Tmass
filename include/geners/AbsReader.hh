#ifndef GENERS_ABSREADER_HH_
#define GENERS_ABSREADER_HH_

//=========================================================================
// AbsReader.hh
//
// Template implementation of a factory pattern for reading objects from
// C++ streams. Derive a reader for your concrete inheritance hierarchy
// from the "DefaultReader" template (as illustrated by the examples
// provided with the package). Wrap your reader using the "StaticReader"
// template when the actual reading is performed to ensure that the reader
// factory is unique.
//
// The classes which can be read must implement the static "read" function
// which creates objects on the heap. The signature of this function is
//
// static ClassName* read(const ClassId& id, std::istream& is);
//
// In this function, the class id argument can be used to implement object
// versioning. The "read" method must always succeed. If it is unable to
// build the object, it must throw an exception which inherits from
// std::exception.
//
// I. Volobouev
// September 2010
//=========================================================================

#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

#include "geners/ClassId.hh"
#include "geners/CPP11_auto_ptr.hh"

namespace gs {
    template<class Base>
    struct AbsReader
    {
        inline virtual ~AbsReader() {}

        virtual Base* read(const ClassId& id, std::istream& in) const = 0;
    };

    template<class Base, class Derived>
    struct ConcreteReader : public AbsReader<Base>
    {
        inline virtual ~ConcreteReader() {}

        inline Derived* read(const ClassId& id, std::istream& in) const
        {
            // Assume that Derived::read(id, in) returns a new object
            // of type "Derived" allocated on the heap
            return Derived::read(id, in);
        }
    };

    template<class Base>
    class DefaultReader : public std::map<std::string, AbsReader<Base>*>
    {
    public:
        typedef Base value_type;

        inline DefaultReader() : std::map<std::string, AbsReader<Base>*>() {}

        inline virtual ~DefaultReader()
        {
            for (typename std::map<std::string, AbsReader<Base>*>::
                     iterator it = this->begin(); it != this->end(); ++it)
                delete it->second;
        }

        inline Base* read(const ClassId& id, std::istream& in) const
        {
            typename std::map<std::string, AbsReader<Base>*>::
                const_iterator it = this->find(id.name());
            if (it == this->end()) 
            {
                std::ostringstream os;
                os << "In gs::DefaultReader::read: class \""
                   << id.name() << "\" is not mapped to a concrete reader";
                throw std::invalid_argument(os.str());
            }
            return it->second->read(id, in);
        }

    private:
        DefaultReader(const DefaultReader&);
        DefaultReader& operator=(const DefaultReader&);
    };

    // A trivial implementation of the Meyers singleton for use with reader
    // factories. Naturally, this assumes that all factories are independent
    // from each other (otherwise we are getting into trouble with undefined
    // singleton destruction order). Also, this particular code is not
    // thread-safe (but should become thread-safe in C++11 if I understand
    // static local initialization guarantees correctly).
    //
    // Assume that "Reader" is derived from "DefaultReader" and that it
    // publishes its base class as "Base".
    //
    template <class Reader>
    class StaticReader
    {
    public:
        typedef typename Reader::Base::value_type InheritanceBase;

        static const Reader& instance()
        {
            static Reader obj;
            return obj;
        }

        template <class Derived>
        static void registerClass()
        {
            Reader& rd = const_cast<Reader&>(instance());
            const ClassId& id(ClassId::makeId<Derived>());
            CPP11_auto_ptr<ConcreteReader<InheritanceBase,Derived> >
                pt(new ConcreteReader<InheritanceBase,Derived>());
            delete rd[id.name()];
            rd[id.name()] = pt.release();
        }

        //
        // The "unregisterClass" method is needed in case you want
        // to dynamically unload the shared library from which some
        // classes were registered earlier. Naturally, classes should
        // be unregistered before unloading the library.
        //
        template <class Derived>
        static void unregisterClass()
        {
            Reader& rd = const_cast<Reader&>(instance());
            const ClassId& id(ClassId::makeId<Derived>());
            delete rd[id.name()];
            rd.erase(id.name());
        }

    private:
        // Disable the constructor
        StaticReader();
    };
}

#endif // GENERS_ABSREADER_HH_
