#ifndef GENERS_ABSREADERWRITER_HH_
#define GENERS_ABSREADERWRITER_HH_

//=========================================================================
// AbsReaderWriter.hh
//
// Template implementation of a factory pattern for reading/writing
// inheritance hierarchies which are not under your control (that is,
// you can't add virtual "classId" and "write" methods to the classes
// in the hierarchy).
//
// The manner in which the I/O is envisioned for such classes is creation
// of one I/O wrapper class for each class in the hierarchy. The I/O class
// should be derived from AbsReaderWriter template parameterized upon the
// top-level base in the hierarchy.
//
// In you want to use this facility, C++11 support is required.
//
// I. Volobouev
// April 2015
//=========================================================================

#include "geners/CPP11_config.hh"

// C++11 support is needed for <typeindex> header
#ifdef CPP11_STD_AVAILABLE

#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>

#include "geners/ClassId.hh"

namespace gs {
    template<class Base>
    struct AbsReaderWriter
    {
        typedef Base wrapped_base;

        inline virtual ~AbsReaderWriter() {}

        virtual bool
        write(std::ostream& of, const wrapped_base& b, bool dumpId) const = 0;

        virtual wrapped_base*
        read(const gs::ClassId& id, std::istream& in) const = 0;
    };

    template<class Base>
    class DefaultReaderWriter
    {
    public:
        typedef Base value_type;

        inline DefaultReaderWriter() {}

        inline virtual ~DefaultReaderWriter()
        {
            for (typename std::map<std::string, AbsReaderWriter<Base>*>::
                     iterator it = readMap_.begin(); it != readMap_.end(); ++it)
                delete it->second;
        }

        inline bool write(std::ostream& of, const Base& b, const bool dumpId) const
        {
            const std::type_info& cppid(typeid(b));
            typename std::map<std::type_index, AbsReaderWriter<Base>*>::
                const_iterator it = writeMap_.find(std::type_index(cppid));
            if (it == writeMap_.end())
            {
                std::ostringstream os;
                os << "In gs::DefaultReaderWriter::write: serialization wrapper "
                   << "for class \"" << cppid.name() << "\" is not registered";
                throw std::invalid_argument(os.str());
            }
            return it->second->write(of, b, dumpId);
        }

        inline Base* read(const gs::ClassId& id, std::istream& in) const
        {
            typename std::map<std::string, AbsReaderWriter<Base>*>::
                const_iterator it = readMap_.find(id.name());
            if (it == readMap_.end()) 
            {
                std::ostringstream os;
                os << "In gs::DefaultReaderWriter::read: serialization wrapper "
                   << "for class \"" << id.name() << "\" is not registered";
                throw std::invalid_argument(os.str());
            }
            return it->second->read(id, in);
        }

        template <class ReaderWriter>
        inline void registerWrapper()
        {
            typedef typename ReaderWriter::wrapped_type Derived;
            const gs::ClassId& id(gs::ClassId::makeId<Derived>());
            std::unique_ptr<ReaderWriter> upt(new ReaderWriter());
            writeMap_[std::type_index(typeid(Derived))] = &*upt;
            delete readMap_[id.name()];
            readMap_[id.name()] = upt.release();
        }

        template <class ReaderWriter>
        inline void unregisterWrapper()
        {
            typedef typename ReaderWriter::wrapped_type Derived;
            const gs::ClassId& id(gs::ClassId::makeId<Derived>());
            delete readMap_[id.name()];
            readMap_.erase(id.name());
            writeMap_.erase(std::type_index(typeid(Derived)));
        }

    private:
        DefaultReaderWriter(const DefaultReaderWriter&);
        DefaultReaderWriter& operator=(const DefaultReaderWriter&);

        std::map<std::string, AbsReaderWriter<Base>*> readMap_;
        std::map<std::type_index, AbsReaderWriter<Base>*> writeMap_;
    };

    // Meyers singleton for use with reader/writer factories.
    // Assume that "Factory" is derived from "DefaultReaderWriter".
    //
    template <class Factory>
    class StaticReaderWriter
    {
    public:
        static const Factory& instance()
        {
            static Factory obj;
            return obj;
        }

        template <class ReaderWriter>
        static void registerWrapper()
        {
            Factory& f = const_cast<Factory&>(instance());
            f.template registerWrapper<ReaderWriter>();
        }

        //
        // Use the following method if you want to dynamically unload
        // a shared library which registered some wrappers
        //
        template <class ReaderWriter>
        static void unregisterWrapper()
        {
            Factory& f = const_cast<Factory&>(instance());
            f.template unregisterWrapper<ReaderWriter>();
        }

    private:
        // Disable the constructor
        StaticReaderWriter();
    };
}

#endif // CPP11_STD_AVAILABLE

#endif // GENERS_ABSREADERWRITER_HH_
