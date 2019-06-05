//
// The following program illustrates "geners" archive I/O for arrays
//
#include <cassert>

#include "geners/StringArchive.hh"
#include "geners/ArrayRecord.hh"
#include "geners/ArrayReference.hh"

using namespace gs;

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // The array which we want to store in the archive
    int array[] = {1, 2, 3, 4, 5};

    // To store arrays, we need to use a special record: "ArrayRecord".
    // Note that the array size must be known to the user both when
    // the array is stored in the archive and when it is retrieved.
    // The array size itself is not stored in the archive since it is
    // normally just a compile-time constant. Store the array size
    // in a separate record or, better, use std::vector to store and
    // retrieve collections whose size is not known in advance.
    const unsigned arrLen = sizeof(array)/sizeof(array[0]);
    ar << ArrayRecord(array, arrLen, "My array", "Top");

    // Retrieve the array from the archive. Use "ArrayReference" to find
    // the array in the archive.
    int array2[arrLen] = {0};
    ArrayReference<int>(ar, "My array", "Top").restore(0, array2, arrLen);

    // Make sure that the retrieved and the original arrays are identical
    for (unsigned i=0; i<arrLen; ++i)
        assert(array2[i] == array[i]);

    // We are done
    return 0;
}
