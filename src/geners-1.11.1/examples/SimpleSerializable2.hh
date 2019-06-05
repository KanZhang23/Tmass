//
// Example simple stand-alone class without a default constructor
// which plays nicely with the "geners" serialization facilities.
//
// In comparison with "SimpleSerializable1", the "restore" method
// has been replaced by the "read" method which creates a new object
// on the heap. Just as the "restore" method, the "read" method must
// always succeed. If it is unable to build the object, it must throw
// an exception which inherits from std::exception.
//

#ifndef GSEXAMPLES_SIMPLESERIALIZABLE2_HH_
#define GSEXAMPLES_SIMPLESERIALIZABLE2_HH_

#include "geners/ClassId.hh"

class SimpleSerializable2
{
public:
    inline SimpleSerializable2(const double value) : datum_(value) {}

    inline double datum() const {return datum_;}

    // Operators which compare objects for equality are very useful
    // for I/O testing
    inline bool operator==(const SimpleSerializable2& r) const
        {return datum_ == r.datum_;}
    inline bool operator!=(const SimpleSerializable2& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    bool write(std::ostream& of) const;

    // I/O methods needed for reading
    static inline const char* classname() {return "SimpleSerializable2";}
    static inline unsigned version() {return 1;}
    static SimpleSerializable2* read(const gs::ClassId& id, std::istream& in);

private:
    SimpleSerializable2();

    double datum_;
};

#endif // GSEXAMPLES_SIMPLESERIALIZABLE2_HH_
