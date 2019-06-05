//
// Example simple template with a default constructor.
//
// The type upon which this template is parameterized
// must itself have the default constructor, the copy
// constructor, and the operator which compares objects
// for equality. This type can not be a pointer type
// (if T is a pointer, the code will not compile).
//

#ifndef GSEXAMPLES_SERIALIZABLETEMPLATE1_HH_
#define GSEXAMPLES_SERIALIZABLETEMPLATE1_HH_

#include <cassert>
#include "geners/ClassId.hh"

// In order to read/write arbitrary types, you will need to include
// the following header file
#include "geners/GenericIO.hh"

template<typename T>
class SerializableTemplate1
{
public:
    inline SerializableTemplate1() : datum_() {}
    inline SerializableTemplate1(const T& value) : datum_(value) {}

    inline const T& datum() const {return datum_;}

    // Operators which compare objects for equality are very useful
    // for I/O testing
    inline bool operator==(const SerializableTemplate1& r) const
        {return datum_ == r.datum_;}
    inline bool operator!=(const SerializableTemplate1& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    inline bool write(std::ostream& of) const
    {
        // The following function should be used to write out single
        // objects in a generic manner
        return gs::write_item(of, datum_);
    }

    // Generating class names for templates is a bit more involved
    // than for simple classes. Here we use the "template_class_name"
    // template function declared in the "geners/ClassId.hh" header.
    // The function "template_class_name" can take up to six template
    // parameters.
    static inline const char* classname()
    {
        static const std::string myClassName(
            gs::template_class_name<T>("SerializableTemplate1"));
        return myClassName.c_str();
    }
    static inline unsigned version() {return 1;}
    static inline void restore(const gs::ClassId& id, std::istream& in,
                               SerializableTemplate1* ptr)
    {
        // Here, we ensure that a correct class id argument was provided.
        // While a wrong class id argument should never be generated if
        // the item is simply read back from a "geners" archive, this check
        // might help to detect problems with other serialization scenarios.
        //
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<SerializableTemplate1<T> >());
        myClassId.ensureSameId(id);

        // Make sure that we have an object to work with
        assert(ptr);

        // The following function should be used to read back single
        // objects in a generic manner. It takes the input stream as
        // the first argument and the pointer to an object that will
        // be overwritten as the second argument.
        //
        gs::restore_item(in, &ptr->datum_);
    }

private:
    T datum_;
};

#endif // GSEXAMPLES_SERIALIZABLETEMPLATE1_HH_
