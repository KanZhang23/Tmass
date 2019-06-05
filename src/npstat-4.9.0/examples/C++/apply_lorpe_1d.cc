//
// The following program illustrates LOrPE usage in 1d.
// It reads a set of numbers from a user-provided file and reconstructs
// the distribution according to the options provided on the command line.
//
// Run this program without any arguments to see its usage instructions.
//
// See also the "lorpeSmooth1D" function in the header file
// "npstat/stat/lorpeSmooth1D.hh" which provides a slightly
// different functionality.

#include <string>
#include <fstream>
#include <stdexcept>

#include "CmdLine.hh"

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/fillHistoFromText.hh"
#include "npstat/stat/amiseOptimalBandwidth.hh"
#include "npstat/stat/arrayStats.hh"

using namespace std;
using namespace npstat;

static void print_usage(const char* progname)
{
    cout << endl << "Usage: " << progname
         << " -- power degree bandwidth b_method n_intervals \\\n"
         << "            x_min x_max input_file input_column output_file\n"
         << endl;
    cout << "  power         power of the symmetric Beta kernel. Use a negative number\n"
         << "                here in order to employ the Gaussian kernel.\n"
         << endl;
    cout << "  degree        LOrPE polynomial degree to use. Use a negative number here in\n"
         << "                order to use the polynomial degree from the AMISE plugin rule.\n"
         << endl;
    cout << "  bandwidth     LOrPE bandwidth to use. Use zero or a negative number here\n"
         << "                in order to use the bandwidth from the AMISE plugin rule.\n"
         << endl;
    cout << "  b_method      The type of boundary correction to perform for the kernel.\n"
         << "                Should be one of "
         << BoundaryHandling::parameterFreeNames() << ".\n"
         << endl;
    cout << "  n_intervals   Number of intervals to use for splitting the density support.\n"
         << endl;
    cout << "  x_min, x_max  Density support interval. Data points outside of this interval\n"
         << "                will be ignored.\n"
         << endl;
    cout << "  input_file    The input file which contains data columns separated by white\n"
         << "                space. Lines which start with the \"#\" character are ignored.\n"
         << endl;
    cout << "  input_column  The column number in the input file for which LOrPE density\n"
         << "                estimation should be performed.\n"
         << endl;
    cout << "  output_file   The name of the output file. This file will contain two\n"
         << "                columns: the interval center will be in the first column\n"
         << "                and the estimated density will be in the second column.\n"
         << endl;
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

    // Parse the input arguments, making sure that
    // their values are reasonable
    int symbetaPower;
    int lorpeDegree;
    double bandwidth;
    unsigned nIntervals;
    double xmin, xmax;
    unsigned inputColumn;
    std::string infile, outfile;
    BoundaryHandling bm;

    try {
        cmdline.optend();
        if (cmdline.argc() != 10)
            throw CmdLineError("wrong number of command line arguments");

        std::string bh;
        cmdline >> symbetaPower >> lorpeDegree >> bandwidth
                >> bh >> nIntervals >> xmin >> xmax
                >> infile >> inputColumn >> outfile;

        if (symbetaPower > 10)
            throw CmdLineError("kernel power outside of supported range");

        if (!BoundaryHandling::isParameterFree(bh.c_str()))
            throw CmdLineError("invalid boundary method name \"") <<
                bh << "\". Must be one of " << BoundaryHandling::parameterFreeNames()
                   << "\".";
        bm = BoundaryHandling(bh.c_str());

        if (nIntervals < 1)
            throw CmdLineError("invalid number of intervals");

        if (xmin >= xmax)
            throw CmdLineError("invalid boundaries, must have x_min < x_max");
    }
    catch (const CmdLineError& e) {
        cerr << "Error in " << cmdline.progname() << ": "
             << e.str() << endl;
        print_usage(cmdline.progname());
        return 1;
    }

    // Histogram for the input data
    HistoND<double> histo(HistoAxis(nIntervals, xmin, xmax));

    // Fill this histogram
    {
        std::ifstream is(infile.c_str());
        if (!is.is_open())
        {
            cerr << "Failed to open input file \"" << infile << '"' << endl;
            return 1;
        }
        if (!fillHistoFromText(is, &histo, &inputColumn))
        {
            cerr << "Failed to parse contents of the input file \""
                 << infile << '"' << endl;
            return 1;
        }
    }

    // Figure out which LOrPE degree to use
    const unsigned long npoints = histo.nFillsInRange();
    if (lorpeDegree < 0)
    {
        // Pick up the degree by the AMISE plugin rule
        if (symbetaPower < 0)
            lorpeDegree = amisePluginDegreeGauss(npoints);
        else
            lorpeDegree = amisePluginDegreeSymbeta(symbetaPower, npoints);
        cout << "Using AMISE plugin degree " << lorpeDegree << endl;
    }

    // Figure out which bandwidth to use
    if (bandwidth <= 0.0)
    {
        // Calculate the scale parameter of the distribution
        // using a robust quantile-based method
        Gauss1D g(0.0, 1.0);
        double qvalues[2], quantiles[2];
        qvalues[0] = g.cdf(-1.0);
        qvalues[1] = g.cdf(1.0);
        arrayQuantiles1D(histo.binContents().data(), nIntervals,
                         xmin, xmax, qvalues, quantiles, 2);
        const double sigma = (quantiles[1] - quantiles[0])/2.0;

        // Pick up the bandwidth by the AMISE plugin rule
        if (symbetaPower < 0)
            bandwidth = amisePluginBwGauss(lorpeDegree, npoints, sigma);
        else
            bandwidth = amisePluginBwSymbeta(
                symbetaPower, lorpeDegree, npoints, sigma);
        cout << "Using AMISE plugin bandwidth " << bandwidth << endl;
    }

    // Create the LOrPE filter
    CPP11_auto_ptr<LocalPolyFilter1D> filter = symbetaLOrPEFilter1D(
        symbetaPower, bandwidth, lorpeDegree,
        nIntervals, xmin, xmax, bm);

    // Buffer to store the LOrPE-estimated density
    ArrayND<double> reconstructed(ArrayShape(1, nIntervals));
    double* reconstructedData = const_cast<double*>(reconstructed.data());

    // Run LOrPE on the histogram contents
    filter->filter(histo.binContents().data(), nIntervals, reconstructedData);

    // Chop off the negative values of the reconstructed density
    reconstructed.makeNonNegative();

    // Renormalize reconstructed density so that its integral is 1
    const double binwidth = (xmax - xmin)/nIntervals;
    const double lorpeIntegral = reconstructed.sum<long double>()*binwidth;
    assert(lorpeIntegral);
    reconstructed /= lorpeIntegral;

    // Write out the estimated density
    {
        std::ofstream of(outfile.c_str());
        if (!of.is_open())
        {
            cerr << "Failed to open output file \"" << outfile << '"' << endl;
            return 1;
        }

        of << "# Generated by: " << cmdline.progname();
        for (int i=1; i<argc; ++i)
            of << ' ' << argv[i];
        of << '\n';

        of.precision(12);
        const HistoAxis& axis = histo.axis(0);
        for (unsigned binNumber=0; binNumber<nIntervals; ++binNumber)
            of << axis.binCenter(binNumber) << ' '
               << reconstructed(binNumber) << '\n';
        cout << "Wrote file " << outfile << endl;
    }

    // We are done
    return 0;
}
