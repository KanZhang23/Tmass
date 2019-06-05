//
// The "geners" package provides a wrapper template for std::tuple called
// "VarPack". VarPack implements "write" and "restore" methods, and it is
// optimized for overwriting the same stack-based object many times. The
// serialized representation of VarPack is identical to std::tuple, so
// VarPack uses class name "std::tuple" for I/O purposes. std::tuple objects
// stored in "geners" archives can also be retrieved as VarPacks, and
// stored VarPack objects can also be retrieved as std::tuples.
//
// Do not store collections of VarPacks, this is not what they are intended
// for. While it is possible to use std::vector<VarPack<...> >, its I/O will
// be less efficient than that of std::vector<std::tuple<...> >.
//
// By default, VarPack checks the class ids of its template parameters on
// the first use only. This means that default VarPack retrieval mechanism
// is inappropriate for use with multiple archives (or one combined archive)
// which were written using different I/O versions of a class used as
// a VarPack template parameter. If you have to work with such archives,
// call "checkTypeEveryTime(true)" method of the VarPack object immediately
// after the object is created. This will slow down the VarPack I/O to
// about the speed of std::tuple I/O, but the class versioning will work
// properly.
//
// Both std::tuple and VarPack are often convenient to use as containers of
// pointers. There is one problem, however. Pointers are "loud" objects
// for I/O purposes -- when the serialization system encounters a pointer
// (bare, shared_ptr, or unique_ptr), it normally emits the class id of
// the pointee into the data stream. This is because the most common use
// of pointers for I/O purposes consists in storing a container of pointers
// to a base class in some kind of an inheritance hierarchy. AFAIK, there
// is no way to check at the compile time whether the "classId" method of
// a class is virtual or not, so the class id is written anyway, even in
// case there is no inheritance hierarchy to take care of. To tell the
// system that it is not necessary to write out the class id of the pointee,
// a special "silent" pointer type, IOPtr, should be used.
//
// This example illustrates usage of VarPack and IOPtr. It can be compiled
// only if C++11 support is enabled.
//
#include <cassert>
#include <iostream>

#include "geners/VarPack.hh"
#include "geners/IOPtr.hh"

#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

using namespace gs;
using namespace std;

#ifdef CPP11_STD_AVAILABLE

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // Pretend that we need the folowing variables to perform a certain
    // calculation
    int i = 15;
    double d = 25.0;
    float f = 3.f;

    // Make a tuple (VarPack) which effectively acts as a struct for I/O
    // purposes. Use "silent" pointers.
    //
    auto t = make_VarPack(
        make_IOPtr(i),
        make_IOPtr(d),
        make_IOPtr(f));
    ar << Record(t, "pack1", "top");

    // Now, change the values of the variables and save them again via
    // the tuple. Naturally, it is expected that in a real calculation
    // this will be done in a cycle (over spreadsheet rows, etc).
    i = 123;
    d = 33.0;
    f = 7.f;
    ar << Record(t, "pack1", "top");

    // Make sure that we can read the data back from the archive
    int i2(0);
    double d2(0.0);
    float f2(0.f);
    auto t2 = make_VarPack(
        make_IOPtr(i2),
        make_IOPtr(d2),
        make_IOPtr(f2));
    Reference<decltype(t2)> ref(ar, "pack1", "top");
    assert(ref.size() == 2);

    ref.restore(0, &t2);
    assert(i2 == 15);
    assert(d2 == 25.0);
    assert(f2 == 3.f);

    ref.restore(1, &t2);
    assert(i2 == 123);
    assert(d2 == 33.0);
    assert(f2 == 7.f);

    // We are done
    return 0;
}

#else // CPP11_STD_AVAILABLE

int main(int, char**)
{
    cout << "To run this example, please recompile "
         << "the package with C++11 features enabled" << endl;
    return 0;
}

#endif // CPP11_STD_AVAILABLE
