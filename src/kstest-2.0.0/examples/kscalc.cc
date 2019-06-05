// This program calculates Kolmogorov-Smirnov cdf and exceedance
// using arguments provided on the command line

#include <iostream>

#include "CmdLine.hh"
#include "kstest/kstest.hh"

using namespace std;

static void print_usage(const char* progname)
{
    cout << "\nUsage: " << progname << " n x\n" << endl;
    cout << "The following arguments are required:" << endl;
    cout << "  n   is the sample size" << endl;
    cout << "  x   is the Kolmogorov distance" << endl;
    cout << endl;
}

int main(int argc, char** argv)
{
    CmdLine cmdline(argc, argv);

    if (argc == 1)
    {
        print_usage(cmdline.progname());
        return 0;
    }

    unsigned n;
    double x;

    // Parse input arguments
    try {
        cmdline.optend();
        if (cmdline.argc() != 2)
            throw CmdLineError("wrong number of command line arguments");
        cmdline >> n >> x;
    }
    catch (const CmdLineError& e) {
        cerr << "Error in " << cmdline.progname() << ": "
             << e.str() << endl;
        print_usage(cmdline.progname());
        return 1;
    }

    const double cdf = kstest::KScdf(n, x);
    const double exceedance = kstest::KSfbar(n, x);

    cout << "n = " << n << ", " << "x = " << x;
    cout.precision(17);
    cout << ", cdf = " << cdf << ", exc = " << exceedance << endl;

    return 0;
}
