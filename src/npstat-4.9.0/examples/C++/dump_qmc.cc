#include <cmath>
#include <vector>
#include <iostream>

#include "CmdLine.hh"

#include "npstat/rng/HOSobolGenerator.hh"

using namespace std;
using namespace npstat;

static void print_usage(const char* progname)
{
    cout << "\nUsage: " << progname << " [-s nSkip] [-i interlacing] dim nPrint\n"
         << endl;
}

int main(int argc, char *argv[])
{
    CmdLine cmdline(argc, argv);
    if (argc == 1)
    {
        print_usage(cmdline.progname());
        return 0;
    }

    unsigned nSkip = 0;
    unsigned interlacing = 2;
    unsigned nPrint;
    unsigned dim;
    
    try {
        cmdline.option("-s", "--skip") >> nSkip;
        cmdline.option("-i", "--interlacing") >> interlacing;

        if (interlacing < 1 || interlacing > 3)
            throw CmdLineError("interlacing factor out of range");

        cmdline.optend();

        if (cmdline.argc() != 2)
            throw CmdLineError("wrong number of command line arguments");
        cmdline >> dim >> nPrint;

        if (nPrint > pow(2, 62/interlacing))
            throw CmdLineError("too many points to print");
    }
    catch (const CmdLineError& e) {
        std::cerr << "Error in " << cmdline.progname() << ": "
                  << e.str() << std::endl;
        return 1;
    }

    cout.precision(12);
    HOSobolGenerator hosobol(dim, interlacing, 62/interlacing);
    std::vector<double> bufVec(dim);
    double* buf = &bufVec[0];
    for (unsigned i=0; i<nPrint; ++i)
    {
        hosobol.run(buf, dim, 1);
        if (i >= nSkip)
        {
            for (unsigned j=0; j<dim; ++j)
            {
                if (j) cout << ' ';
                cout << buf[j];
            }
            cout << endl;
        }
    }

    return 0;
}
