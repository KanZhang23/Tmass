#include "SimpleSerializable2.hh"

// The following header defines functions used to read/write built-in
// types as well as arrays, vectors, and strings. Always use these functions
// to implement the I/O for your own classes instead of using std::ostream
// facilities directly. This will ensure interface consistency and will
// simplify future updates.
#include "geners/binaryIO.hh"

// The following header defines several useful exception classes
#include "geners/IOException.hh"

using namespace gs;

bool SimpleSerializable2::write(std::ostream& of) const
{
    write_pod(of, datum_);
    return !of.fail();
}

SimpleSerializable2* SimpleSerializable2::read(const ClassId& id,
                                               std::istream& in)
{
    // Here, we ensure that a correct class id argument was provided.
    // While a wrong class id argument should never be generated if
    // the item is simply read back from a "geners" archive, this check
    // might help to detect problems with other serialization scenarios.
    //
    static const ClassId myClassId(ClassId::makeId<SimpleSerializable2>());
    myClassId.ensureSameId(id);

    // Read back the object info
    double d(0.0);
    read_pod(in, &d);

    // Check the status of the stream
    if (in.fail()) throw IOReadFailure("In SimpleSerializable2::read: "
                                       "input stream failure");
    return new SimpleSerializable2(d);
}
