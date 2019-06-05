//
// The following program illustrates "geners" archive I/O for STL
// containers. To serialize an STL container, include the corresponding
// "geners" header which ends in IO.hh. For example, to store lists,
// include "geners/listIO.hh", to store maps or multimaps include
// "geners/mapIO.hh", etc. Here is the full list of relevant standard
// headers together with the corresponding headers from the "geners"
// package
//
// Standard header   Geners header        Comment
// ---------------   ---------------      ----------------------------------
// <array>           arrayIO.hh           compiler must support C++11 or TR1
// <deque>           dequeIO.hh
// <forward_list>    forward_listIO.hh    C++11 support required
// <list>            listIO.hh
// <map>             mapIO.hh
// <set>             setIO.hh
// <tuple>           tupleIO.hh           C++11 support required
// <unordered_map>   unordered_mapIO.hh   C++11 support required
// <unordered_set>   unordered_setIO.hh   C++11 support required
// <valarray>        valarrayIO.hh
// <vector>          vectorIO.hh
//
// If a header requires C++11 support and your compiler does not provide
// it, you will not be able to use that header. This does not prevent you
// from using headers and containers that do not need C++11.
//
// Strings are an exception from this scheme -- their serialization
// facilities are defined in "geners/binaryIO.hh" and they are essentially
// treated as PODs.
//
#include <cassert>

// In this example, we will use vectors and maps
#include "geners/vectorIO.hh"
#include "geners/mapIO.hh"

#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

#include "SimpleSerializable2.hh"

using namespace gs;
using namespace std;

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // Make some objects that we want to store in the archive
    vector<SimpleSerializable2> objects;
    objects.push_back(SimpleSerializable2(10.));
    objects.push_back(SimpleSerializable2(-12345.));
    objects.push_back(SimpleSerializable2(42.));

    map<string,int> roomNumbers;
    roomNumbers["Andrew"] = 21;
    roomNumbers["John"] = 33;

    // And now store these objects
    ar << Record(objects, "Some", "Objects")
       << Record(roomNumbers, "Room Numbers", "My Department");

    // Retrieve the objects and make sure that they are identical
    // to the original ones. As usual, the restored objects can be
    // placed either on the stack or on the heap.
    vector<SimpleSerializable2> objects_2;
    Reference<vector<SimpleSerializable2> >(ar, "Some", "Objects").restore(
        0, &objects_2);
    assert(objects == objects_2);

    CPP11_auto_ptr<map<string,int> > roomNumbers_2 = 
        Reference<map<string,int> >(ar, "Room Numbers", "My Department").get(0);
    assert(roomNumbers == *roomNumbers_2);

    // We are done
    return 0;
}
