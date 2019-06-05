// The following program illustrates LOrPE usage in 1d.
// It generates a set of Gaussian random numbers and then
// reconstructs the distribution using LOrPE.

#include "npstat/rng/MersenneTwister.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/DensityScan1D.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/HistoND.hh"

using namespace npstat;

int main(int, char**)
{
    // Some simple program configuration parameters.
    // They are hardwired here -- of course, one can also
    // write some parsing code so that these parameters
    // are picked up from the command line.

    // Parameters of the Gaussian distribution
    const double gaussian_mean   = 0.0;
    const double gaussian_sigma  = 1.5;
    const unsigned sample_points = 100;

    // Parameters of the grid. Note that the grid chosen here
    // effectively chops off one half of the distribution in
    // order to illustrate the boundary effects.
    const double interval_min    = 0.0;
    const double interval_max    = 10.0;
    const unsigned grid_points   = 1000;
    const double grid_step       = (interval_max - interval_min)/grid_points;

    // Degree of the LOrPE filter
    const unsigned lorpe_degree = 3;

    // Bandwidth of the LOrPE filter
    const double lorpe_bandwidth = 3.0;

    // LOrPE boundary handling method
    BoundaryHandling boundaryMethod("BM_TRUNCATE");

    // Choose LOrPE kernel. Note that the use of SymmetricBeta1D
    // kernels is very common, and there is a helper finction
    // called "symbetaLOrPEFilter1D" which automatically builds
    // the corresponding LocalPolyFilter1D object. This example,
    // however, demostrates a more general method of constructing
    // such a filter.
    CPP11_auto_ptr<AbsDistribution1D> kernel(
        new SymmetricBeta1D(0.0, lorpe_bandwidth, 4));

    // Before constructing the filter, we need to make a "filter builder"
    // which knows how to handle the boundary effects during smoothing
    CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> filterBuilder =
        getBoundaryFilter1DBuilder(boundaryMethod, kernel.get(), grid_step);

    // In this example we will use a simple LOrPE filter
    // which does not have a taper function. There is instead
    // the maximum degree of the polynomial.
    LocalPolyFilter1D lorpeFilter(0, lorpe_degree, *filterBuilder, grid_points);

    // Create the random number generator
    MersenneTwister rng;

    // Come up with the sequence of generated points 
    // (this is our pseudo-data)
    std::vector<double> points;
    points.reserve(sample_points);
    Gauss1D gauss(gaussian_mean, gaussian_sigma);
    for (unsigned i=0; i<sample_points; ++i)
    {
        double r;
        gauss.random(rng, &r);
        points.push_back(r);
    }

    // Discretize the sequence on a grid. We will use a histogram
    // to represent the discretized empirical density.
    HistoND<double> sampleHistogram(HistoAxis(grid_points,
                                              interval_min, interval_max));
    for (unsigned i=0; i<sample_points; ++i)
        sampleHistogram.fill(points[i], 1.0);

    // Filter the histogrammed sample. The result is written into
    // an array object. This is our reconstructed density.
    ArrayND<double> reconstructed(ArrayShape(1, grid_points));
    double* reconstructedData = const_cast<double*>(reconstructed.data());
    lorpeFilter.filter(sampleHistogram.binContents().data(), grid_points,
                       reconstructedData);

    // Chop off negative values of the reconstructed density
    reconstructed.makeNonNegative();

    // Normalize reconstructed density so that its integral is 1
    const double lorpeIntegral = reconstructed.sum<double>()*grid_step;
    assert(lorpeIntegral > 0.0);
    reconstructed /= lorpeIntegral;

    // Calculate the ISE for this sample. For this, we need to
    // discretize the actual distribution according to which
    // the random points were generated.
    ArrayND<double> original(grid_points);
    original.functorFill(DensityScan1D(gauss, 1.0, grid_points,
                                       interval_min, interval_max, 0));

    // Normalize the original density inside the grid interval to 1
    const double normalizationIntegral = original.sum<double>()*grid_step;
    assert(normalizationIntegral > 0.0);
    original /= normalizationIntegral;

    // The actual ISE calculation
    const double ise = (reconstructed - original).sumsq<double>()*grid_step;

    // Print the ISE value to the standard output
    std::cout << "LOrPE ISE for the current 1d random sample is "
              << ise << std::endl;

    // Uncomment the following snippet in order to print the original
    // and the reconstructed distributions to the standard output.
    // The printout order is: x original random reconstructed
    //
    //     for (unsigned i=0; i<grid_points; ++i)
    //         std::cout << interval_min + (i+0.5)*grid_step
    //                   << ' ' << original(i)
    //                   << ' ' << sampleHistogram.binContents()(i)
    //                   << ' ' << reconstructed(i)
    //                   << std::endl;
 
    // We are done
    return 0;
}
