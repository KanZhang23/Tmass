// The following program merges contents of several string archives

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "geners/stringArchiveIO.hh"
#include "geners/CPP11_auto_ptr.hh"

#include "CmdLine.hh"

using namespace gs;
using namespace std;


static void print_usage(const char* progname)
{
    cout << "\nUsage: " << progname << " -o output_file input_file0 input_file1 ...\n\n"
         << "This program merges the contents of multiple string archives into one.\n"
         << endl;
}


int main(int argc, char const* argv[])
{
    CmdLine cmdline(argc, argv);

    if (argc == 1)
    {
        print_usage(cmdline.progname());
        return 0;
    }

    string outputfile;
    vector<string> inputfiles;

    try {
        cmdline.require("-o") >> outputfile;
        cmdline.optend();
        
        while (cmdline)
        {
            std::string s;
            cmdline >> s;
            inputfiles.push_back(s);
        }

        if (inputfiles.empty())
            throw CmdLineError("must specify at least one input file");
    }
    catch (CmdLineError& e) {
        cerr << "Error in " << cmdline.progname() << ": "
             << e.str() << endl;
        print_usage(cmdline.progname());
        return 1;
    }

    // Before we perform any significant data processing,
    // make sure that we can open all input files
    const unsigned nfiles = inputfiles.size();
    for (unsigned ifile=0; ifile<nfiles; ++ifile)
    {
        const std::string& inputfile(inputfiles[ifile]);
        ifstream in(inputfile.c_str(), ios_base::binary);
        if (!in.is_open())
        {
            cerr << "Error: failed to open file \""
                 << inputfile << "\"" << endl;
            return 1;
        }
    }

    // Come up with the collection of allowed I/O prototypes
    vector<string> goodProtos;
    goodProtos.push_back("gs::Single");
    goodProtos.push_back("gs::Array");

    CPP11_auto_ptr<StringArchive> out;

    // Now, do the real cycle over the input files
    for (unsigned ifile=0; ifile<nfiles; ++ifile)
    {
        CPP11_auto_ptr<StringArchive> ar(readCompressedStringArchiveExt(
                                             inputfiles[ifile].c_str()));
        if (!ar.get())
        {
            cerr << "Error: failed to read a string archive from file \""
                 << inputfiles[ifile] << "\"" << endl;
            return 1;
        }
        if (!ifile)
            out = CPP11_auto_ptr<StringArchive>(
                new StringArchive(ar->name().c_str()));

        const unsigned nGoodProtos = goodProtos.size();
        const unsigned long long last = ar->largestId();
        for (unsigned long long id=ar->smallestId(); id<=last; ++id)
        {
            if (!ar->itemExists(id)) continue;
            CPP11_shared_ptr<const CatalogEntry> e = ar->catalogEntry(id);
            bool good = false;
            for (unsigned iproto=0; iproto<nGoodProtos && !good; ++iproto)
                good = e->ioPrototype() == goodProtos[iproto];
            if (!good)
            {
                cerr << "Error: can not merge item with type \""
                     << e->type().name()
                     << "\", name \"" << e->name()
                     << "\", category \"" << e->category()
                     << "\", and I/O prototype \"" << e->ioPrototype()
                     << "\" residing in a string archive file \""
                     << inputfiles[ifile] << "\"" << endl;
                return 1;
            }
            const unsigned long long newid = ar->copyItem(id, out.get());
            if (!newid)
            {
                cerr << "Error: failed to merge item with type \""
                     << e->type().name()
                     << "\", name \"" << e->name()
                     << "\", and category \"" << e->category()
                     << "\" residing in a string archive file \""
                     << inputfiles[ifile] << "\"" << endl;
                return 1;
            }
        }
    }

    if (!writeCompressedStringArchiveExt(*out, outputfile.c_str()))
    {
        cerr << "Error: failed to write merged archive in file \""
             << outputfile << "\"" << endl;
        return 1;
    }
 
    return 0;
}
