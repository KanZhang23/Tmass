//
// The following program illustrates how to use references to search for
// items in "geners" archives together with the various methods that
// can be used to retrieve the items
//
#include <cassert>
#include <iostream>

#include "geners/StringArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

using namespace gs;
using namespace std;

// We will use the following function to print out the items found
// in the archive by an archive reference. All items considered in
// this example are simple strings.
//
static void print_items(const Reference<string>& ref)
{
    // Total number of items found by the reference
    const unsigned long nItems = ref.size();

    // Print the info about the reference to the standard output
    cout << "Found " << nItems << " item"
         << (nItems == 1 ? "" : "s")
         << " in the archive using name pattern \""
         << ref.namePattern().pattern() << '"'
         << (ref.namePattern().useRegex() ? " (regex)" : " (exact)")
         << "\nand category pattern \""
         << ref.categoryPattern().pattern() << '"'
         << (ref.categoryPattern().useRegex() ? " (regex)" : " (exact)")
         << (nItems ? ":\n" : ".\n");

    // Cycle over the items found, retrieve them from the archive,
    // and print them
    for (unsigned long i=0; i<nItems; ++i)
    {
        // Here, we will overwrite a string on the stack with a stored
        // string. See the code of "main" for other retrieval methods.
        string item;
        ref.restore(i, &item);
        cout << i+1 << ". " << item << '\n';
    }
}


int main(int, char**)
{
    // The archive to use
    StringArchive ar;

    // A number of items to store in the archive
    const string st1 = "Hello, world!";
    const string st2 = "Goodbye world";
    const string st3 = "Casablanka";
    const string st4 = "Gone with the Wind";
    const string st5 = "Tomorrow Never Dies";
    const string st6 = "Quantum of Solace";
    const string st7 = "Titanic";
    const string st8 = "Avatar";

    // Store the items
    ar << Record(st1, "hello", "Greetings")
       << Record(st2, "goodbye", "Greetings")
       << Record(st3, "classics", "Movies")
       << Record(st4, "classics", "Movies")
       << Record(st5, "tomorrow", "Movies/James Bond")
       << Record(st6, "quantum", "Movies/James Bond")
       << Record(st7, "movie 1", "Movies/Cameron")
       << Record(st8, "movie 2", "Movies/Cameron");

    // Find an exact item using unique name/category combination.
    // In this case the "size()" method of the reference will
    // return 1 and "unique()" will return "true".
    //
    cout << "\n** Searching for a unique name/category combination" << endl;
    Reference<string> ref1(ar, "hello", "Greetings");
    assert(ref1.unique());
    print_items(ref1);

    // What happens if name/category combination does not
    // exist? The reference "size()" method will return 0
    // and "empty()" will return "true".
    //
    cout << "\n** What if name/category combination does not exist?" << endl;
    Reference<string> ref2(ar, "no such name", "Greetings");
    assert(ref2.empty());
    print_items(ref2);

    // What happens if more than one item exists with the given
    // name/category combination? The reference will simply
    // collect all such items.
    //
    cout << "\n** What if there is more than one item "
         << "with given name/category?" << endl;
    Reference<string> ref3(ar, "classics", "Movies");
    print_items(ref3);

    // Archive references support regular expressions for names
    // and categories. If the "geners" package was compiled with
    // a C++11-enabled compiler, the standard C++11 "regex" engine
    // is used, otherwise POSIX extended regular expressions are used.
    // Note that the default mode of C++11 regex is "ECMAScript"
    // which is somewhat different from POSIX extended regular
    // expressions. If you really want to know the details of the
    // ECMAScript regex syntax, read the "RegExp" section of
    // http://www.ecma-international.org/publications/files/ecma-st/ECMA-262.pdf
    //
    // The number of operations needed to search for an item in
    // an archive using exact name and category typically scales as
    // O(log(C) + log(N)), where C is the number of categories and N
    // is the number of items in the category to which the item belongs.
    // If regular expressions are used, the scaling behavior changes.
    // In general, this behavior depends on the details of the archive
    // catalog implementation. The catalogs currently implemented
    // in the "geners" package typically need O(N*log(C))
    // operations if regular expression search is used for item name
    // only, O(C_m*log(N)) if regular expression search is used for
    // category only (where C_m is the number of categories matching
    // the expression), and O(C_m*N) if regular expressions are used
    // for both name and category. Therefore, regex searching can take
    // a considerable amount of time for large catalogs.
    //
    // In the examples below, a special constructor is used for the
    // "SearchSpecifier" objects with which references are made.
    // This is because we want to be able to print the regular
    // expressions used, and C++11 regular expressions do not
    // have a method to retrieve the original expression string
    // from them. When this special constructor is used,
    // "SearchSpecifier" stores this string for subsequent retrieval.
    //
    cout << "\n** Regex example -- arbitrary name" << endl;
    Reference<string> ref4(ar, SearchSpecifier(".*", true), "Greetings");
    print_items(ref4);

    cout << "\n** Regex example -- same search, but using regex argument "
         << "directly. Note\n   that the regular expression itself can no "
         << "longer be extracted from such\n   an argument. This example "
         << "prints an empty string instead." << endl;
    Reference<string> ref5(ar, Regex(".*"), "Greetings");
    print_items(ref5);

    cout << "\n** Regex example -- arbitrary category" << endl;
    Reference<string> ref6(ar, "quantum", SearchSpecifier(".*", true));
    print_items(ref6);

    cout << "\n** Regex example -- name matching a substring, "
         << "arbitrary category" << endl;
    Reference<string> ref7(ar, SearchSpecifier("(.*)movie(.*)", true),
                               SearchSpecifier(".*", true));
    print_items(ref7);

    // There are three main methods by which items can be retrieved from
    // the archive:
    //
    // 1. Overwrite another item of the same type present on the stack.
    //    This method is appropriate for built-in types, standard
    //    containers, and user-defined classes or templates that have
    //    a default constructor (in addition, these classes or templates
    //    should not be members of any inheritance hierarchy).
    //
    // 2. Get a unique pointer (std::unique_ptr for C++11, std::auto_ptr
    //    before that) to an item built on the heap. "geners" package
    //    uses "CPP11_auto_ptr" macro to name such a unique pointer.
    //    This method is most typical for types that do not have a default
    //    constructor or for members of some inheritance hierarchy. Items
    //    of derived types can be retrieved using a pointer to their base
    //    class if they are stored in the archive by reference to the base
    //    class.
    //
    // 3. Get a shared pointer (std::shared_ptr for C++11, std::tr1::shared_ptr
    //    before that) to an item built on the heap. "geners" package
    //    uses "CPP11_shared_ptr" macro to name such a shared pointer.
    //    Use this method whenever joint item ownership via a shared
    //    pointer is appropriate for your code.
    // 
    // Method 1 is how the "print_items" function defined earlier in this
    // file retrieves the strings from the archive. The following code
    // illustrates the other two methods.
    //
    cout << "\n** Placing an item on the heap using a unique pointer" << endl;
    CPP11_auto_ptr<string> p1 = ref1.get(0);
    cout << *p1 << endl;

    cout << "\n** Placing an item on the heap using a shared pointer" << endl;
    CPP11_shared_ptr<string> p2 = ref1.getShared(0);
    cout << *p2 << endl;

    // We are done
    return 0;
}
