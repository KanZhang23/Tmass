#include <cassert>
#include "SimpleSerializable1.hh"

// The following header defines functions used to read/write built-in
// types as well as arrays, vectors, and strings. Always use these functions
// to implement the I/O for your own classes instead of using std::ostream
// facilities directly. This will ensure interface consistency and will
// simplify future updates.
#include "geners/binaryIO.hh"

// The following header defines several useful exception classes
#include "geners/IOException.hh"

using namespace gs;

bool SimpleSerializable1::write(std::ostream& of) const
{
    write_pod(of, datum_);
    return !of.fail();
}

void SimpleSerializable1::restore(const ClassId& id, std::istream& in,
                                  SimpleSerializable1* ptr)
{
    // Here, we ensure that a correct class id argument was provided.
    // While a wrong class id argument should never be generated if
    // the item is simply read back from a "geners" archive, this check
    // might help to detect problems with other serialization scenarios.
    //
    static const ClassId myClassId(ClassId::makeId<SimpleSerializable1>());
    myClassId.ensureSameId(id);

    // Read back the object info
    int i;
    read_pod(in, &i);

    // Check the status of the stream
    if (in.fail()) throw IOReadFailure("In SimpleSerializable1::restore: "
                                       "input stream failure");

    // Overwrite the argument object. Ideally, we should be
    // doing this only when we know for sure that all necessary
    // information has been successfully read back.
    assert(ptr);
    ptr->datum_ = i;
}
