//
// The following program illustrates expectation-maximization
// unfolding with smoothing for 1-d distributions.
//
// It generates random numbers from a distribution, smears them
// according to a resolution function, and then reconstructs the
// original distribution. The smoothing bandwidth is selected
// automatically with the help of UnfoldingBandwidthScanner1D class.
//
// This particular problem has originally been considered in
// http://arxiv.org/abs/1401.8274 as an illustration to a different
// unfolding method. Both observed and unfolded distributions occupy
// the interval [-7, 7]. The "true" distribution is a mix of two
// gaussians and the uniform distribution, with parameters and weights
// as decsribed in that preprint. The resolution function is a gaussian
// with mean equal to x and width 1 (the width is independent from x).
//

#include <cmath>
#include <cassert>
#include <iostream>
#include <sys/time.h>

#include "npstat/rng/MersenneTwister.hh"

#include "npstat/stat/SmoothedEMUnfold1D.hh"
#include "npstat/stat/UnfoldingBandwidthScanner1D.hh"
#include "npstat/stat/DistributionMix1D.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/HistoND.hh"
#include "npstat/stat/gaussianResponseMatrix.hh"
#include "npstat/stat/DensityScan1D.hh"
#include "npstat/stat/BinnedADTest1D.hh"

using namespace std;
using namespace npstat;

// A useful procedure for time stamping
static long double epoch_seconds()
{
    const long double ufactor = 1.0e-6;
    struct timeval tv;
    assert(!gettimeofday(&tv, 0));
    return tv.tv_sec + ufactor*tv.tv_usec;
}

// The main program
int main(int, char**)
{
    // Some configuration parameters for this program.
    // They are hardwired here -- of course, one can also
    // write some parsing code so that these parameters
    // are picked up from the command line.

    // Number of pseudo-experiments to run
    const unsigned n_pseudo_experiments = 5;

    // How many random points to generate per pseudo-experiment?
    const unsigned points_per_pseudo_exp = 1000;

    // Limits for the unfolded and observed intervals
    const double xmin = -7.0;
    const double xmax = 7.0;

    // Number of cells to use for the observations. Should be
    // much larger than the number of effective degrees of freedom
    // of the model (this can be checked later).
    const unsigned n_bins_observed = 100;

    // Number of cells to use for the unfolded space.
    // Setting n_unfolded to 700 results in the the cell width of 0.02
    // which is much smaller than the resolution function width of 1.
    const unsigned n_unfolded = 700;

    // The original distribution to reconstruct
    DistributionMix1D orig;
    orig.add(Gauss1D(-2, 1), 0.2);
    orig.add(Gauss1D(2, 1), 0.5);
    orig.add(Uniform1D(xmin, xmax - xmin), 0.3);

    // It will be useful to have a scan of the density of the original
    // distribution in order to compare it with the unfolded result
    ArrayND<double> expectedResult(n_unfolded);
    const double normfactor = points_per_pseudo_exp*(xmax - xmin)/n_unfolded;
    DensityScan1D dscan(orig, normfactor, n_unfolded, xmin, xmax, 0);
    expectedResult.functorFill(dscan);

    // Distribution which will be used to emulate uncertainty
    // introduced by the "detector response"
    Gauss1D smear(0.0, 1.0);

    // Build the corresponding response matrix
    const Matrix<double>& responseMatrix = gaussianResponseMatrix(
        xmin, xmax, n_unfolded,
        xmin, xmax, n_bins_observed,
        Shift<double>(smear.location()),
        ConstValue1<double,double>(smear.scale()));

    // Histogram to use for storing the "observed" data
    HistoND<double> h(HistoAxis(n_bins_observed, xmin, xmax), "Observed");

    // Create the object which runs the unfolding algorithm. To begin with,
    // we will pass a dummy filter to it because its constructor requires
    // the filter argument. However, this filter will be replaced when
    // the bandwidth scan is performed.
    DummyLocalPolyFilter1D dummyFilter(n_unfolded);
    const bool useConvolutions = true;
    SmoothedEMUnfold1D unfolder(responseMatrix, dummyFilter, useConvolutions);

    // We will run the Anderson-Darling test on the fit to the observed data
    BinnedADTest1D test_AD(false, 0);
    std::vector<const AbsBinnedComparison1D*> observedSpaceComparators;
    observedSpaceComparators.push_back(&test_AD);

    // Create the object which will run the bandwidth scans.
    // Together with the "useConvolutions" argument of the unfolder,
    // the arguments provided here define the smothing filter which
    // behaves like the Green's function of the heat equation.
    //
    // If you don't have an expectation for the result, just put 0
    // both for its data and for its length.
    //
    const int symbetaPower = -1;
    const double maxLOrPEDegree = 0;
    BoundaryHandling bm("BM_FOLD");
    std::vector<const AbsBinnedComparison1D*> empty;
    UnfoldingBandwidthScanner1D bwscan(
        unfolder, h.binContents().data(), h.nBins(), 0,
        expectedResult.data(), expectedResult.length(),
        symbetaPower, maxLOrPEDegree, xmin, xmax, bm,
        1.0, observedSpaceComparators, empty);

    // Bandwidth scanner will produce a number of variables for
    // unfolding diagnostics. Get the names of those variables.
    const std::vector<std::string>& scanVariables = bwscan.variableNames();

    // Buffer for holding the values of these variables
    const unsigned nScanVariables = scanVariables.size();
    std::vector<double> scanValues(nScanVariables);

    // Create a random number generator
    MersenneTwister rng;

    // Cycle over the pseudo-experiments
    long double starttime = epoch_seconds();
    for (unsigned ipseudo=0; ipseudo<n_pseudo_experiments; ++ipseudo)
    {
        // Fill the histogram of "observed" values
        h.clear();
        for (unsigned ipt=0; ipt<points_per_pseudo_exp; ++ipt)
        {
            double value, shift;
            orig.random(rng, &value);
            smear.random(rng, &shift);
            h.fill(value + shift, 1.0);
        }

        // Give the new observed dataset to the bandwidth scanner
        bwscan.setObservedData(h.binContents().data(), h.nBins(), 0);

        // Determine the bandwidth by minimizing EAIC_c and unfold the
        // distribution for that bandwidth value. The code will perform
        // a search on a grid of bandwidth values (but not necessarily
        // trying each grid location), and then the three results closest
        // to the minimum are interpolated quadratically to pinpoint the
        // minimum. In the "production" code, you might want to increase
        // the number of cells for the bandwidth space.
        const double minBandwidthToSearch = (xmax - xmin)/n_unfolded;
        const double maxBandwidthToSearch = smear.scale();
        const unsigned nCellsInBandwidthSpace = 40;
        const double startingValueForBandwidthSearch = 0.2;
        const double startingFactorForBandwidthSearch = 1.5;
        const MinSearchStatus1D searchStatus = bwscan.processAICcBandwidth(
            minBandwidthToSearch, maxBandwidthToSearch, nCellsInBandwidthSpace,
            startingValueForBandwidthSearch, startingFactorForBandwidthSearch);

        // Update the processing time
        const long double t = epoch_seconds();
        const double dt = t - starttime;
        starttime = t;

        // Bail out if the minimum EAIC_c search failed
        if (searchStatus == MIN_SEARCH_FAILED)
        {
            cout << "\n**** Bandwidth scan failed in pseudo-experiment "
                 << ipseudo << endl;
            continue;
        }

        // Extract the unfolded result. We are not doing anything
        // useful with it here, so the lines are commented out
        // in order to prevent compiler warnings.
        //
        // const std::vector<double>& unfolded = bwscan.unfoldedResult();
        // const Matrix<double>& covmat = bwscan.unfoldedCovariance();

        // Extract diagnostic information about the unfolded result
        bwscan.ntuplize(&scanValues[0], nScanVariables);

        // Print diagnostic information about the unfolded result to the
        // standard output. Of course, one could also fill out an ntuple
        // instead and look at these values with some data analysis program.
        // The meaning of various variables is explained in the comments
        // to the header file "npstat/stat/UnfoldingBandwidthScanner1D.hh".
        //
        // Note that the code takes quite a bit of time to process the
        // first pseudo-experiment and then it runs faster. The choice of
        // "n_bins_observed" should be validated by comparing it with
        // "modelNDoFEntropic" or "modelNDoFTrace" (n_bins_observed
        // should be much larger than either one of these variables).
        //
        cout << "\n**** Diagnostics for pseudo-experiment " << ipseudo
             << " (processing took " << dt << " sec)" << endl;
        for (unsigned ivar=0; ivar<nScanVariables; ++ivar)
            cout << scanVariables[ivar] << " = " << scanValues[ivar] << endl;
    }

    // We are done
    return 0;
}
