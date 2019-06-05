//
// The following program illustrates "geners" archive I/O for C++11
// tuples. These containers rely heavily on the new language feature
// called "variadic templates". Tuple is the first truly heterogeneous
// container in C++. There were no such containers in the previous C++
// standard -- struct was the closest thing. Compared to similar containers
// in dynamically typed languages (python, Tcl, etc), C++ tuple is less
// capable and has a somewhat clumsier interface. However, it does offer
// a type-safe alternative to creating a struct when a number of
// heterogeneous items must be grouped together for various purposes.
//
// The "geners" package provides a number of facilities for reading and
// writing both single tuples and tuple collections. Collections support
// the data processing mode in which precisely one tuple is needed in
// order to perform some meaningful calculation. Think about spreadsheet
// processing one row at a time, or accessing a database table one entry
// at a time, or looking at any other independent entities such as particle
// physics events produced by accelerators. See example files
// "optimized_tuple_io.cc" and "tuple_collections.cc" for more details
// about these facilities.
//
// This particular example can be compiled only if C++11 support is enabled.
//
#include <cassert>
#include <iostream>
#include <cstdlib>

// The following header is needed to read/write tuples
#include "geners/tupleIO.hh"

#include "geners/vectorIO.hh"
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
    std::shared_ptr<unsigned> pu(new unsigned(10U));

    // Instead of combining all these variables into a struct, we can
    // make a tuple which effectively acts like such a struct for I/O
    // purposes. This saves quite a bit of typing because there is no
    // need to implement all those I/O-related methods (classId, write,
    // restore, etc).
    //
    // There is, however, a better way to do the same thing: see file
    // "optimized_tuple_io.cc".
    //
    auto t = make_tuple(&i, &d, pu);
    ar << Record(t, "tuple1", "top");

    // Now, change the values of the variables and save them again via
    // the tuple. Naturally, it is expected that in a real calculation
    // this will be done in a cycle (over spreadsheet rows, etc).
    i = 123;
    d = 33.0;
    *pu = 7U;
    ar << Record(t, "tuple1", "top");

    // Make sure that we can read the data back from the archive.
    // Note the difference between bare pointer and shared pointer.
    // When the tuple is read back, the pointee object is modified
    // for the bare pointer, while the shared pointer is made to
    // point to a new object reconstructed on the heap.
    int i2(0);
    double d2(0.0);
    std::shared_ptr<unsigned> pu2(new unsigned(789U));
    auto t2 = make_tuple(&i2, &d2, pu2);
    Reference<decltype(t2)> ref(ar, "tuple1", "top");
    assert(ref.size() == 2);

    ref.restore(0, &t2);
    assert(i2 == 15);
    assert(d2 == 25.0);
    assert(*std::get<2>(t2) == 10U);

    // Note that at this point *pu2 is still 789U -- it was not modified
    assert(*pu2 == 789U);

    ref.restore(1, &t2);
    assert(i2 == 123);
    assert(d2 == 33.0);
    assert(*std::get<2>(t2) == 7U);

    // While tuples that contain data are probably less convenient to use
    // than tuples that contain pointer-like objects, it is of course
    // possible to serialize them as well. Here, for example, how one can
    // store a vector of tuples.
    auto t3 = std::tuple<bool, int, double>();
    std::vector<decltype(t3)> vec;
    for (unsigned count=0; count<10; ++count)
    {
        std::get<0>(t3) = rand()*1.0/RAND_MAX < 0.5;
        std::get<1>(t3) = static_cast<int>(rand()*100.0/RAND_MAX);
        std::get<2>(t3) = rand()*1.0/RAND_MAX;
        vec.push_back(t3);
    }
    ar << Record(vec, "tuplevec", "top");

    // Retrieve the vector of tuples and compare it with the original
    std::vector<decltype(t3)> vec2;
    Reference<decltype(vec2)>(ar, "tuplevec", "top").restore(0, &vec2);
    assert(vec2 == vec);

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
