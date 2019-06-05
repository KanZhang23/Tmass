//
// The following program illustrates the usage of "geners" package
// serialization mechanisms with an inheritance hierarchy over which
// you have no control -- in particular, the classes in this hierarchy
// do not have virtual "classId" and "write" methods. Naturally, it is
// assumed that these classes allow for complete examination of their
// state.
//
// In this case, the implemented serialization mechanism relies on
// the parallel hierarchy of I/O classes which wrap the "functional"
// classes and provide both read and write methods. The I/O classes
// are placed into an I/O factory which automatically chooses proper
// wrapper class for each functional class. The factory implementation
// assumes that your compiler supports C++11.
//
// The following classes are used in this example: ExtBase (pure virtual
// base class), ExtDerivedA (derived from ExtBase), and ExtDerivedB
// (derived from ExtDerivedA).
//
// The I/O factory for this hierarchy is implemented inside files
// ExtBaseIO.hh and ExtBaseIO.cc. Note that I/O wrappers for all
// concrete classes in the inheritance hierarchy are registered with
// this factory inside its constructor (see ExtBaseIO.cc).
//
// The wrappers for concrete classes are implemented in files
// ExtDerivedAIO.{hh,cc} and ExtDerivedBIO.{hh,cc}.
//
#include <memory>
#include <iostream>

#include "geners/CPP11_config.hh"
#ifdef CPP11_STD_AVAILABLE

#include "ExtDerivedAIO.hh"
#include "ExtDerivedBIO.hh"

#include "geners/vectorIO.hh"
#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

using namespace gs;
using namespace std;

int main(int, char**)
{
    // The archive
    gs::StringArchive ar;

    // Objects we want to serialize
    const ExtDerivedA a1(5);
    const ExtDerivedA a2(8);
    const ExtDerivedB b1(6, 7.0);
    const ExtDerivedB b2(9, 10.0);
    const ExtDerivedB b3(11, 12.0);

    std::vector<std::shared_ptr<ExtBase> > v;
    v.push_back(std::shared_ptr<ExtBase>(new ExtDerivedA(15)));
    v.push_back(std::shared_ptr<ExtBase>(new ExtDerivedB(16, 17.0)));

    // Write out the items
    ar << gs::Record(a1, "saved A", "")
       << gs::Record(dynamic_cast<const ExtBase&>(a2), "A saved by base ref", "")
       << gs::Record(b1, "saved B", "")
       << gs::Record(dynamic_cast<const ExtBase&>(b2), "B saved by base ref", "")
       << gs::Record(dynamic_cast<const ExtDerivedA&>(b3),
                     "B saved by intermediate base ref", "")
       << gs::Record(v, "vector of heterogeneous pointers", "");

    // Retrieve the items and compare with originals
    gs::Reference<ExtDerivedA> ra1(ar, "saved A", "");
    assert(*ra1.get(0) == a1);

    // Objects of concrete classes saved by their own references
    // can also be retrieved on the stack (of course, under the
    // condition that the class has the assignment operator). Objects
    // saved using base references can only be retrieved on the heap.
    ExtDerivedA tmpa(2323);
    ra1.restore(0, &tmpa);
    assert(tmpa == a1);

    gs::Reference<ExtBase> ra2(ar, "A saved by base ref", "");
    assert(dynamic_cast<ExtDerivedA&>(*ra2.get(0)) == a2);

    gs::Reference<ExtDerivedB> rb1(ar, "saved B", "");
    assert(*rb1.get(0) == b1);

    ExtDerivedB tmpb(678, 5.0);
    rb1.restore(0, &tmpb);
    assert(tmpb == b1);

    gs::Reference<ExtBase> rb2(ar, "B saved by base ref", "");
    assert(dynamic_cast<ExtDerivedB&>(*rb2.get(0)) == b2);

    gs::Reference<ExtDerivedA> rb3(ar, "B saved by intermediate base ref", "");
    assert(dynamic_cast<ExtDerivedB&>(*rb3.get(0)) == b3);

    std::vector<std::shared_ptr<ExtBase> > v2;
    gs::Reference<decltype(v2)> r(ar, "vector of heterogeneous pointers", "");
    r.restore(0, &v2);
    assert(dynamic_cast<ExtDerivedA&>(*v[0])==dynamic_cast<ExtDerivedA&>(*v2[0]));
    assert(dynamic_cast<ExtDerivedB&>(*v[1])==dynamic_cast<ExtDerivedB&>(*v2[1]));

    return 0;
}

#else // CPP11_STD_AVAILABLE

int main(int, char**)
{
    std::cout << "To run this example, please recompile "
              << "the package with C++11 features enabled" << std::endl;
    return 0;
}

#endif // CPP11_STD_AVAILABLE
