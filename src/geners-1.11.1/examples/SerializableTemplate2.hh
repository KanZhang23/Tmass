//
// Example simple template without a default constructor.
//
// The type upon which this template is parameterized
// must itself have the copy constructor but not necessarily
// the default constructor.
//
// In comparison with "SerializableTemplate1", the "restore"
// method has been replaced with the "read" method.
//

#ifndef GSEXAMPLES_SERIALIZABLETEMPLATE2_HH_
#define GSEXAMPLES_SERIALIZABLETEMPLATE2_HH_

#include <cassert>
#include "geners/ClassId.hh"
#include "geners/CPP11_auto_ptr.hh"

// In order to read/write arbitrary types, you will need to include
// the following header file
#include "geners/GenericIO.hh"

template<typename T>
class SerializableTemplate2
{
public:
    inline SerializableTemplate2(const T& value) : datum_(value) {}

    inline const T& datum() const {return datum_;}

    // Operators which compare objects for equality are very useful
    // for I/O testing
    inline bool operator==(const SerializableTemplate2& r) const
        {return datum_ == r.datum_;}
    inline bool operator!=(const SerializableTemplate2& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    inline bool write(std::ostream& of) const
    {
        // The following function should be used to write out single
        // objects in a generic manner
        return gs::write_item(of, datum_);
    }

    // I/O methods needed for reading
    static inline const char* classname()
    {
        static const std::string myClassName(
            gs::template_class_name<T>("SerializableTemplate2"));
        return myClassName.c_str();
    }
    static inline unsigned version() {return 1;}
    static inline SerializableTemplate2* read(const gs::ClassId& /* id */,
                                              std::istream& in)
    {
        // How do we read back in a generic way an object which might
        // not have a default constructor? We can't construct it on the
        // stack here -- in this scope we do not have enough information
        // to figure out how to build it. Instead, we need to call the
        // "gs::read_item" function which reads back single objects and
        // places them on the heap (gs::read_item will invoke the "read"
        // method of the class if the class has one -- or will work in
        // other ways appropriate for type T). gs::read_item either
        // succeeds or throws an exception.
        //
        CPP11_auto_ptr<T> ptr = gs::read_item<T>(in);

        // Now, we can make an object of this class by copying
        // the T object we've just read back
        //
        return new SerializableTemplate2<T>(*ptr);
    }

private:
    T datum_;
};

#endif // GSEXAMPLES_SERIALIZABLETEMPLATE2_HH_
