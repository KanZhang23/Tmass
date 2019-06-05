//
// The following program illustrates very basic usage of the archive I/O
// in the "geners" package
//
#include <cassert>
#include <iostream>

#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

// All "geners" facilities reside in the namespace "gs".
using namespace gs;
using namespace std;

int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // The object which we want to store in the archive
    const string greeting = "Hello, world!";

    // Store the object in the archive. Besides a reference to the object,
    // "Record" function takes two additional arguments: item name and
    // item category. Both of these are arbitrary C-strings (C++ strings
    // can be used as well), intended to be convenient for labelling the
    // object for subsequent retrieval.
    //
    ar << Record(greeting, "My greeting", "Greetings");

    // It is worth emphasizing that, for performance reasons, "Record" takes
    // a reference to the object instead of making a copy. This means that
    // the "greeting" object in the above example must not go out of scope
    // before the archive writing is complete. For example, it would not be
    // a good idea to do something like this:
    //
    // ar << Record(std::string("Don't do this"), "Trouble", "Bugs");
    //
    // even though this code will compile and might even work as intended
    // on some systems. The problem is that the object
    // std::string("Don't do this") can be destroyed after "Record" runs
    // to completion but before operator << starts executing.
    //
    // If you are ready to sacrifice some efficiency for API convenience,
    // use the "ValueRecord" function instead which stores a copy of the
    // argument object internally:
    //
    // ar << ValueRecord(std::string("This is OK"), "but less efficient", "");

    // Now, we will find the object in the archive by creating an archive
    // reference. Here, we are simply looking up an item of type "string"
    // with the same name and category as in the record used above.
    //
    Reference<string> ref(ar, "My greeting", "Greetings");

    // Make sure that exactly one item with the given name, category,
    // and type was found in the archive
    //
    assert(ref.unique());

    // Retrieve the object from the archive. Here, we will overwrite
    // another object of the same type already present on the stack
    // (it is also possible to retrive archive items in other ways,
    // see "use_of_references.cc" example for illustrations). The first
    // argument of the "restore" method of the reference is the item
    // number in the set of items found by that particular reference.
    // In general, an archive reference may find more than one item
    // satisfying its search criteria.
    //
    string greeting2;
    ref.restore(0, &greeting2);

    // Print the retrieved and the original objects
    //
    cout << "Original greeting is \"" << greeting << '"' << endl;
    cout << "Archived greeting is \"" << greeting2 << '"' << endl;

    // We are done
    return 0;
}
