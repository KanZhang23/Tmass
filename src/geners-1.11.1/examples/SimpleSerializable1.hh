//
// Example simple stand-alone class with a default constructor
// which plays nicely with "geners" serialization facilities.
//
// Static methods "classname" and "version" are used by "geners"
// to construct the class identifier for serialization purposes.
// Static method "restore" is used to read the object from an
// istream. In addition, the class must implement member functions
// "classId" and "write". "classId" is just a non-static wrapper
// around "classname" and "version" which can always be implemented
// as shown in this example. The purpose of the "write" method is,
// naturally, to dump the object data into an ostream. In certain
// situations, "classId" and "write" methods must be made virtual
// in order to support polymorphic serialization.
//
// The "write" method must return "true" on success and "false" on
// failure. The "restore" method must always succeed. If, for any
// reason, "restore" is unable to reconstruct the object from
// the istream, it must throw an exception which inherits from
// std::exception. Several useful exception classes are declared
// in the "geners/IOException.hh" header.
//
// The "restore" function overwrites an existing object which usually
// lives on the stack. One can instead implement the "read" function
// which makes a new object on the heap (see SimpleSerializable2.hh
// header for such an example). As a rule of thumb, simple light-weight
// classes without virtual methods and with default constructors
// should implement "restore" for deserialization. In all other cases
// implementing "read" is preferable. In particular, all classes which
// are members of inheritance hierarchies and which can be accessed via
// base class pointers must use virtual "classId" and "write" methods
// and must implement "read" function for deserialization. An example
// program called "inheritance_example.cc" gives more details about
// serializing inheritance hierarchies.
//

#ifndef GSEXAMPLES_SIMPLESERIALIZABLE1_HH_
#define GSEXAMPLES_SIMPLESERIALIZABLE1_HH_

// The "geners/ClassId.hh" header contains the gs::ClassId declaration
// (the class identifier). This header should be included whenever you
// develop classes which use "geners" for serialization.
//
#include "geners/ClassId.hh"

class SimpleSerializable1
{
public:
    inline SimpleSerializable1() : datum_(0) {}
    inline SimpleSerializable1(const int value) : datum_(value) {}

    inline int datum() const {return datum_;}

    // Operators which compare objects for equality are very useful
    // for I/O testing
    inline bool operator==(const SimpleSerializable1& r) const
        {return datum_ == r.datum_;}
    inline bool operator!=(const SimpleSerializable1& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    bool write(std::ostream& of) const;

    // I/O methods needed for reading
    static inline const char* classname() {return "SimpleSerializable1";}
    static inline unsigned version() {return 1;}
    static void restore(const gs::ClassId& id, std::istream& in,
                        SimpleSerializable1* ptr);
private:
    // If necessary, the copy constructor and the assignment operator
    // can be disabled. Together with the default constructor, "write"
    // and "restore" methods give us everything we need for I/O.
    SimpleSerializable1(const SimpleSerializable1&);
    SimpleSerializable1& operator=(const SimpleSerializable1&);

    int datum_;
};

#endif // GSEXAMPLES_SIMPLESERIALIZABLE1_HH_
