//
// The following program illustrates usage of the archive I/O
// in the "geners" package with classes adapted to be compatible
// with this I/O facility. Please read the comments in the
// "SimpleSerializable1.hh" header for more details.
//
#include <cassert>

#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

#include "SimpleSerializable1.hh"
#include "SimpleSerializable2.hh"

using namespace gs;

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // The objects which we want to store in the archive
    const SimpleSerializable1 s1(42);
    const SimpleSerializable2 s2(1.2345);

    // Store the objects in the archive
    ar << Record(s1, "First object", "Top")
       << Record(s2, "Second object", "Top");

    // Read the objects back using optimal I/O mechanisms: restore
    // the object on the stack for a class which implements the
    // "restore" method, and make an object on the heap for a class
    // which implements the "read" method.
    SimpleSerializable1 s1_2;
    Reference<SimpleSerializable1>(ar, "First object", "Top").restore(0,&s1_2);
    assert(s1 == s1_2);

    CPP11_auto_ptr<SimpleSerializable2> s2_2 = Reference<SimpleSerializable2>(
        ar, "Second object", "Top").get(0);
    assert(s2 == *s2_2);

    // It is possible to use the "get" function of the archive reference
    // with a class that has the "restore" method and the default constructor.
    // This looses some efficiency because operator "new" is usually slower
    // than placing an object on the stack.
    CPP11_auto_ptr<SimpleSerializable1> s1_3 = Reference<SimpleSerializable1>(
        ar, "First object", "Top").get(0);
    assert(s1 == *s1_3);

    // With some loss of efficiency, it is also possible to use the "restore"
    // method of the reference with a class that has the "read" method and
    // the assignment operator. An extra copy of the object will be made 
    // internally.
    SimpleSerializable2 s2_3(5.678);
    Reference<SimpleSerializable2>(ar, "Second object", "Top").restore(0,&s2_3);
    assert(s2 == s2_3);

    // We are done
    return 0;
}
