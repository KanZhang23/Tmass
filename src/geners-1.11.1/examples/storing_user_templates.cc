//
// The following program illustrates usage of the archive I/O in
// the "geners" package with user-developed template classes.
// Please read the comments in header files "SerializableTemplate1.hh"
// and "SerializableTemplate2.hh" for more details.
//
#include <cassert>

#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

#include "SimpleSerializable2.hh"
#include "SerializableTemplate1.hh"
#include "SerializableTemplate2.hh"

using namespace gs;

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // The objects which we want to store in the archive
    const SerializableTemplate1<int> t1(123);
    const SerializableTemplate2<SimpleSerializable2> t2(SimpleSerializable2(3.0));

    // Store the objects in the archive
    ar << Record(t1, "First object", "Top")
       << Record(t2, "Second object", "Top");

    // Read the objects back and make sure that the archived copies and
    // the originals are the same
    SerializableTemplate1<int> t1_2;
    Reference< SerializableTemplate1<int> >(
        ar, "First object", "Top").restore(0, &t1_2);
    assert(t1 == t1_2);

    CPP11_auto_ptr<SerializableTemplate2<SimpleSerializable2> > t2_2 = 
        Reference<SerializableTemplate2<SimpleSerializable2> >(
            ar, "Second object", "Top").get(0);
    assert(t2 == *t2_2);

    // We are done
    return 0;
}
