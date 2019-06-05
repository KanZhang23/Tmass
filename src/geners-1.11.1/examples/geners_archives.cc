//
// The following program illustrates the use of the three archives
// currently available in the "geners" package: StringArchive,
// BinaryFileArchive, and MultiFileArchive. All these archives are
// derived from the "AbsArchive" base class. For the code which writes
// C++ objects into the archives or reads them back, all archives are
// completely interchangeable.
//
// StringArchive     -- Simple memory-based archive always open for both
//                      reading and writing. Fast, very useful for testing
//                      serialization of user-developed classes. This
//                      archive itself can be stored in binary files (just
//                      one object per file) using the API defined in the
//                      "geners/stringArchiveIO.hh" header. It can also be
//                      serialized and stored in other archives.
//
// BinaryFileArchive -- File-based archive. Creates two files: binary data
//                      file (possibly compressed) and binary catalog file
//                      (not compressed). The data is stored in binary form.
//                      Very large archives of this type can be created if
//                      the operating system supports large files.
//
// MultiFileArchive  -- File-based archive using multiple binary data
//                      files. Catalog is still stored in one file only.
//                      Very large archives of this type can be created
//                      even if the operating system does not support
//                      large files. In particular, older linux systems
//                      (as well as older versions of NFS) have 2 GB file
//                      size limitation.
//
// Only memory-resident metadata catalogs are supported at this time.
// This can become a limitation for data samples with large quantities
// of small objects. In this situation one would have to create several
// archives and work with them one at a time.
//
// Due to the complete archive interchangeability, all other "geners"
// examples use the StringArchive class for archive-based I/O.
//
#include <iostream>

#include "geners/StringArchive.hh"
#include "geners/stringArchiveIO.hh"
#include "geners/BinaryFileArchive.hh"
#include "geners/MultiFileArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

using namespace gs;
using namespace std;

int main(int, char**)
{
    // The archives to use. String archives can be unnamed (or give
    // it a name using a C-string as a constructor argument).
    StringArchive sar;

    // The first argument of any binary file archive constructor
    // is the name of the archive. The names of files included in
    // the archive will be derived from this argument. For example,
    // if the name of the archive is "arch1", the following files
    // will be made: arch1.0.gsbd and arch1.gsbmf. Extension "gsbd"
    // stands for "generic serialization binary data", and extension
    // "gsbmf" stands for "generic serialization binary metafile".
    // The metafile contains the all-important archive catalog.
    //
    // The second constructor argument, "mode", specifies the opening
    // mode of the archive data files as well as certain archive
    // options. The "mode" string can have one or more sections,
    // separated by ":". The first section has the same meaning as
    // in the "fopen" call (see "man 3 fopen"). Additional sections
    // can specify other aspects of the archive behavior using the
    // format "option=value" if the default settings are not suitable.
    // The available options are:
    //
    // "z"   -- compression type. Possible option values are
    //           "n" -- no compression (default)
    //           "z" -- compress with zlib
    //           "b" -- compress with bzlib2
    //
    // "cl"  -- compression level (an integer between -1 and 9,
    //           -1 is default. Meanigful for zlib compression
    //           only. See zlib documentation for details.
    //
    // "cb"  -- compression buffer size in bytes (unsigned integer).
    //           Default is 1 MB.
    //
    // "cm"  -- minimum object size in bytes to compress (unsigned
    //           integer). Objects whose serialized size is below
    //           this limit are not compressed. Default is 1 KB.
    //
    // "cat" -- if the value is set to "i" (which means "internal"
    //           or "injected"), the catalog data will be injected
    //           into the data stream in addition to having it in
    //           a separate catalog file. This allows for catalog
    //           recovery from the data stream in cases of program
    //           failure but also increases the data file size.
    //           The default value of this option is "s" which means
    //           that the catalog data will be stored separately.
    //           This option is meaningful for new archives only,
    //           for existing archives the value of this option is
    //           taken from the archive header record.
    //
    // Example: "w+:z=z:cl=9:cm=2048:cat=s". This will compress
    // objects with 2 KB or larger size using level 9 zlib compression
    // (the best compression ratio possible in zlib which is also
    // the slowest). The archive will be open for reading and writing.
    // If an archive with the same name already exists, it will be
    // overwritten. The catalog will be stored in a separate file
    // created when the archive is closed, catalog recovery from the
    // data file(s) in case of a catastrophic program failure will
    // not be possible.
    //
    // Empty mode string is equivalent to "r".
    //
    // The third archive constructor argument is an arbitrary archive
    // annotation string which will be stored together with the archive
    // catalog. This argument is optional (empty string is the default).
    //
    const char* mode = "w"; // Here, open write-only archive, with
                            // default values for all other setting 
    BinaryFileArchive bfar("arch1", mode, "single file archive");

    // If a multi file archive is named "arch2", its binary data
    // files will be named arch2.N.gsbd, with N = 0, 1, 2, ...
    // The second and the third contructor arguments have the same
    // meaning as for the BinaryFileArchive. The fourth argument
    // is the approximate maximum file size in MB (default value
    // of that argument corresponds to the maximum file size of
    // about 1 GB). The value used here, 0, will force every new
    // object to be written into a new data file. THIS SETTING IS
    // USEFUL FOR ILLUSTRATIVE PURPOSES ONLY, LARGER VALUES SHOULD
    // BE USED IN PRACTICE (or just omit this argument altogether
    // in order to use the default value).
    //
    MultiFileArchive mfar("arch2", mode, "multi file archive", 0);

    // The objects which we want to store in the archives
    const string st1 = "Hello, world!";
    const string st2 = "Goodbye world";

    // Store the objects in the archives. Note that records can not
    // be reused, so we have to create a new record every time.
    sar << Record(st1, "hello", "Greetings")
        << Record(st2, "goodbye", "Greetings");

    bfar << Record(st1, "hello", "Greetings")
         << Record(st2, "goodbye", "Greetings");

    mfar << Record(st1, "hello", "Greetings")
         << Record(st2, "goodbye", "Greetings");

    // Write out the string archive. See comments in the
    // "stringArchiveIO.hh" header file on how to configure
    // compression for storing and retrieving string archives.
    //
    if (!writeStringArchive(sar, "arch3.gssa"))
        std::cerr << "Failed to write the string archive" << std::endl;

    // The archives will be automatically flushed to disk and closed
    // when they go out of scope. Therefore, do not use the "exit"
    // function to terminate the program! If "exit" is called here,
    // destructors of objects created inside "main" will not be executed.
    //
    // Once the program completes, you can examine the catalog files
    // using the "cdump" tool.
    //
    return 0;
}
