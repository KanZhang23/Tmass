//
// The "geners" package provides two templates for efficient I/O of
// tuple collections. These collections support the data processing
// mode in which precisely one tuple (or a part of one tuple) is
// needed in order to perform some meaningful calculation. The relevant
// templates are
//
// RowPacker    -- Persistent tuple collection optimized for data
//                 readback row-by-row. Should be used when calculations
//                 use the whole row or most part of it.
//
// ColumnPacker -- Persistent tuple collection in which entries for
//                 each variable (each tuple member, or spreadsheet
//                 column) are collected together and saved as soon
//                 as the amount of data in this column exceeds
//                 certain buffer size. This often results in better
//                 overall compression for large collections than
//                 buffering data row-by-row, and allows for efficient
//                 data access in each column separately.
//
// Both of these collections are not single monolithic objects. Instead,
// they consist of a number of data buffers stored in an archive, with
// only a small number of buffers present in the computer memory. For
// example, RowPacker keeps one or two buffers at a time in memory
// (current read buffer and current write buffer which are sometimes
// the same), while ColumnPacker keeps in memory one or two buffers
// per column (i.e., tuple variable). In this manner, the tuple
// collections can easily grow very large, typically limited only by
// the available amount of disk space.
//
// This example illustrates the usage of these collections. It can be
// compiled only if C++11 support is enabled.
//
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <numeric>

#include "geners/RowPacker.hh"
#include "geners/ColumnPacker.hh"
#include "geners/IOPtr.hh"
#include "geners/CP_column_iterator.hh"
#include "geners/StringArchive.hh"

using namespace gs;
using namespace std;

#ifdef CPP11_STD_AVAILABLE

// IOProxy is a useful object which is essentially a "named silent pointer":
// it works just like IOPtr for I/O purposes and it also has a string to
// store the name of the variable (or "column"). The macro below builds
// a pointer to a certain variable and gives it a name of that variable.
// Together with IOPtr, IOProxy is defined inside the "IOPtr.hh" header.
#define io_proxy(obj) gs::make_IOProxy( obj , #obj )


// Function which generates some data for the purposes of this example
static void generate_some_data(int* i, int* j, double* d,
                               double* e, float* f, float* g)
{
    *i = static_cast<int>(rand()*100.0/RAND_MAX);
    *j = static_cast<int>(rand()*(-200.0)/RAND_MAX);
    *d = rand()*2.0/RAND_MAX;
    *e = rand()*3.0/RAND_MAX;
    *f = rand()*4.0/RAND_MAX;
    *g = rand()*5.0/RAND_MAX;
}


// Function which illustrates the use of a RowPacker
//
static void row_packer_example(AbsArchive& ar)
{
    // Pretend that we need the folowing variables to perform a certain
    // calculation
    int i(0), j(0);
    double d(0), e(0);
    float f(0), g(0);

    // Make a tuple which organizes the I/O of these variables
    auto t1 = make_tuple(
        io_proxy(i),
        io_proxy(j),
        io_proxy(d),
        io_proxy(e),
        io_proxy(f),
        io_proxy(g)
    );

    // Make a RowPacker capable of storing multiple entries of this tuple.
    // Naturally, other ways to construct a RowPacker are available, see
    // "RowPacker.hh" header for details.
    typedef RowPacker<decltype(t1)> Packer1;
    Packer1 packer1("title", ar, "packer1", "top", t1);

    // To check that read/write operations of RowPacker work correctly,
    // let's also keep the packed data in a vector of data-holding tuples
    auto t1_data = std::tuple<int, int, double, double, float, float>();
    std::vector<decltype(t1_data)> vec;

    // Example cycle that generates and stores the packer data
    const unsigned nEntries = 1000;
    for (unsigned icycle=0; icycle<nEntries; ++icycle)
    {
        generate_some_data(&i, &j, &d, &e, &f, &g);

        // Store the variables calculated above. The "fill" function
        // returns "true" on success and "false" on failure.
        assert(packer1.fill(t1));

        // And make a copy as well for future data verification
        std::get<0>(t1_data) = i;
        std::get<1>(t1_data) = j;
        std::get<2>(t1_data) = d;
        std::get<3>(t1_data) = e;
        std::get<4>(t1_data) = f;
        std::get<5>(t1_data) = g;
        vec.push_back(t1_data);
    }

    // Call the following function to ensure that all packer buffers
    // were written into the archive. You don't have to call the "write"
    // function explicitly if the packer goes out of scope -- in this
    // case all of its data is automatically written out.
    packer1.write();

    // The "write()" method should be called only after all necessary
    // data has been added. The "write()" method tags the collection as
    // "read only", and it is no longer possible to fill it. Any new
    // invocation of the "fill" method will fail (returning "false")
    // and will not change the collection contents.
    assert(!packer1.fill(t1));

    // Retrieve the packed data from the archive. For large tuple
    // collections, RowPacker data buffers could be spread out across
    // the whole archive, sharing the archive space with many other
    // objects. Because of this, we need to use a special reference
    // to retrieve this data called "RPReference" (the standard
    // "Reference" template only works for stand-alone records).
    // RowPacker can only be retrieved onto the heap, using either
    // unique_ptr (with "get" method of RPReference) or shared_ptr
    // (with "getShared" method).
    //
    unique_ptr<Packer1> readback = RPReference<Packer1>(
        ar, "packer1", "top").get(0);

    // Verify integrity of the retrieved data
    assert(readback->nRows() == nEntries);
    for (unsigned icycle=0; icycle<nEntries; ++icycle)
    {
        // Retrieve the data produced in the given cycle
        readback->rowContents(icycle, &t1);

        // Get a reference to the alternative copy of the data
        // and make sure these two data sources agree
#ifndef NDEBUG
        const decltype(t1_data)& t2(vec[icycle]);
        assert(std::get<0>(t2) == i);
        assert(std::get<1>(t2) == j);
        assert(std::get<2>(t2) == d);
        assert(std::get<3>(t2) == e);
        assert(std::get<4>(t2) == f);
        assert(std::get<5>(t2) == g);
#endif
    }
}


// Function which illustrates the use of a ColumnPacker
//
static void column_packer_example(AbsArchive& ar)
{
    // Pretend that we need the folowing variables to perform a certain
    // calculation
    int i(0), j(0);
    double d(0), e(0);
    float f(0), g(0);

    // Make a tuple which organizes the I/O of these variables
    auto t1 = make_tuple(
        io_proxy(i),
        io_proxy(j),
        io_proxy(d),
        io_proxy(e),
        io_proxy(f),
        io_proxy(g)
    );

    // Make a ColumnPacker capable of storing multiple entries of this tuple
    // Naturally, other ways to construct a ColumnPacker are available, see
    // "ColumnPacker.hh" header for details.
    typedef ColumnPacker<decltype(t1)> Packer1;
    Packer1 packer1("title", ar, "packer1", "top", t1);

    // Example cycle that generates and stores the packer data
    const unsigned nEntries = 1000;
    for (unsigned icycle=0; icycle<nEntries; ++icycle)
    {
        generate_some_data(&i, &j, &d, &e, &f, &g);
        packer1.fill(t1);
    }

    // Call the "write" function to ensure that all packer buffers
    // were written into the archive. Naturally, the packer writes out
    // all of its data automatically when it goes out of scope. After
    // calling this method, it is no longer possible to modify the
    // collection.
    packer1.write();

    // Retrieve the packed data from the archive. For large tuple
    // collections, ColumnPacker data buffers could be spread out across
    // the whole archive, sharing the archive space with many other
    // objects. Because of this, we need to use a special reference
    // to retrieve this data called "CPReference". Just as RowPacker,
    // ColumnPacker can only be retrieved on the heap, either into
    // unique_ptr or into shared_ptr.
    //
    unique_ptr<Packer1> readback = CPReference<Packer1>(
        ar, "packer1", "top").get(0);

    // So far, this example proceeded more or less along the lines
    // of RowPacker example. However, ColumnPacker has a number
    // of unique interface features. These features are illustrated
    // below.

    // ColumnPacker has an appropriate iterator template class,
    // CP_column_iterator, capable of iterating over column contents.
    // For example, the code snippet below will calculate the sum of
    // all entries in column 2 using variable "temp" to store fetched
    // column values when the iterator is dereferenced. Naturally,
    // with a column iterator it becomes easy to use a number of other
    // STL algorithms which do not modify the container as well as to
    // fetch column contents into an std::vector.
    //
    // Column iterator works only forward (no operator--) and it is
    // effectively const. When dereferenced, it refers to a copy of
    // the data and not to actual ColumnPacker contents.
    //
    double temp(0);
    const double sum2 = std::accumulate(
        CP_column_begin<2>(*readback, make_IOPtr(temp)),
        CP_column_end<2>(*readback, make_IOPtr(temp)), 0.0);

    // Print the sum and its expected value (note that column 2 values
    // are random numbers uniformly distributed between 0 and 2)
    cout << "Sum of column 2 contents is " << sum2 << ", expected "
         << nEntries << " +- " << 2.0/sqrt(12.0)*sqrt(nEntries) << endl;

    // Another interesting feature of ColumnPacker is that it is possible
    // to cycle over ColumnPacker rows in such a manner that only those
    // columns which are needed in subsequent calculations are read back
    // from the archive (which, of course, speeds up the data retrieval).
    // In order to do this, one needs to call a special CPReference
    // constructor which refers only to a few selected columns. Suppose,
    // in some calculation we only need to use the info which was
    // available in the original variables "i" and "d". Then one can
    // proceed as follows:
    int i2;
    double d2;
    auto tup = make_tuple(
        make_IOProxy(i2, "i"),
        make_IOProxy(d2, "d")
    );
    typedef ColumnPacker<decltype(tup)> Packer2;
    std::unique_ptr<Packer2> readback2 = CPReference<Packer2>(
        tup, ar, "packer1", "top").get(0);

    // Now, cycling over "readback2" rows will fill out variables
    // "i2" and "d2" with the data that originally was in "i" and "d".
    // For example, let's check that we can reproduce the sum of the
    // column 2 contents calculated above.
    double sum = 0.0;
    for (unsigned icycle=0; icycle<nEntries; ++icycle)
    {
        readback2->rowContents(icycle, &tup);
        sum += d2;
    }
    assert(sum == sum2);
}


int main(int, char**)
{
    StringArchive ar;

    row_packer_example(ar);
    column_packer_example(ar);

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
