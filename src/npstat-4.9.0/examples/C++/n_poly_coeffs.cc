// The following program calculates the number of coefficients
// for a multivariate polynomial

#include <climits>
#include <cassert>
#include <iostream>

#include "CmdLine.hh"
#include "npstat/nm/binomialCoefficient.hh"

using namespace std;

static void print_usage(const char* progname)
{
    cout << "\nUsage: " << progname << " dim deg\n" << endl;
    cout << "The following arguments are required:" << endl;
    cout << "  dim   is the dimensionality of the space" << endl;
    cout << "  deg   is the maximum polynomial degree" << endl;
    cout << endl;
}

int main(int argc, char** argv)
{
    CmdLine cmdline(argc, argv);

    // If the program is executed without any arguments,
    // print its usage instructions and exit
    if (argc == 1)
    {
        print_usage(cmdline.progname());
        return 0;
    }

    // Parse input arguments
    unsigned mydim, upDeg;
    try {
        cmdline.optend();
        if (cmdline.argc() != 2)
            throw CmdLineError("wrong number of command line arguments");
        cmdline >> mydim >> upDeg;
    }
    catch (const CmdLineError& e) {
        cerr << "Error in " << cmdline.progname() << ": "
             << e.str() << endl;
        print_usage(cmdline.progname());
        return 1;
    }
    if (!mydim)
    {
        cerr << "Error in " << cmdline.progname()
             << ": dimensionality of the space must be positive" << endl;
        return 1;
    }

    unsigned long ntotal = 1;
    for (unsigned deg=1; deg<=upDeg; ++deg)
    {
        // For each degree we have a multichoose problem.
        // Therefore, we need to add (dim + deg - 1)!/(deg! (dim - 1)!)
        ntotal += npstat::binomialCoefficient(mydim+deg-1U, deg);
    }
    std::cout << ntotal << std::endl;

    return 0;
}
