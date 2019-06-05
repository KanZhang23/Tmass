// The following program illustrates multivariate LOrPE usage.
// It generates a set of multivariate Gaussian random numbers
// and then reconstructs the distribution using LOrPE.

#include <tr1/array>

#include "npstat/nm/BoxNDScanner.hh"
#include "npstat/nm/rectangleQuadrature.hh"

#include "npstat/rng/MersenneTwister.hh"

#include "npstat/stat/DistributionsND.hh"
#include "npstat/stat/ScalableGaussND.hh"
#include "npstat/stat/LocalPolyFilterND.hh"
#include "npstat/stat/HistoND.hh"
#include "npstat/stat/scanDensityAsWeight.hh"

using namespace npstat;

int main(int, char**)
{
    // Some simple program configuration parameters.
    // They are hardwired here -- of course, one can also
    // write some parsing code so that these parameters
    // are picked up from the command line.

    // Dimensionality of the sample
    const unsigned DIM = 2;

    // Parameters of the multivariate Gaussian distribution
    const double gaussian_mean[DIM]   = {0.0, 0.5};
    const double gaussian_sigma[DIM]  = {1.0, 1.5};
    const unsigned sample_points      = 400;

    // Parameters of the data discretization grid. Note that the grid
    // chosen here effectively chops off a fraction of the distribution
    // in order to illustrate the boundary effects.
    const double interval_min[DIM]    = {0.0, 0.0};
    const double interval_max[DIM]    = {10.0, 10.0};
    const unsigned grid_points[DIM]   = {100, 100};

    // Grid cell size in each dimension
    double grid_step[DIM];
    for (unsigned i=0; i<DIM; ++i)
        grid_step[i] = (interval_max[i] - interval_min[i])/grid_points[i];

    // Choose LOrPE kernel. For subsequent use with "scanDensityAsWeight"
    // function, kernel should be centered at the origin and have unit
    // width in each dimension.
    const double kernel_location[DIM] = {0., 0.}, kernel_width[DIM] = {1., 1.};
    const AbsDistributionND* kernel = new ScalableSymmetricBetaND(
        kernel_location, kernel_width, DIM, 4);

    // Degree of the LOrPE filter
    const unsigned lorpe_degree = 2;

    // Bandwidth of the LOrPE filter
    const double lorpe_bandwidth[DIM] = {3.0, 4.0};

    // In this example we will use the simplest LOrPE filter
    // which does not have a taper function. There is instead
    // the maximum degree of the polynomial.
    //
    // First, create the table of kernel values. We will need
    // to scan one quadrant only.
    //
    CPP11_auto_ptr<ArrayND<double> > kernelScan = scanDensityAsWeight(
        *kernel, grid_points, lorpe_bandwidth, grid_step, DIM, true);

    // Now, create the LOrPE filter
    ArrayShape dataShape(grid_points, grid_points+DIM);
    LocalPolyFilterND<lorpe_degree> lorpeFilter(
        0, lorpe_degree, *kernelScan, dataShape);

    // Create the random number generator. The default constructor of
    // MersenneTwister will pick the generator seed from /dev/urandom.
    MersenneTwister rng;

    // Come up with the sequence of generated points 
    // (this is our pseudo-data)
    std::vector<std::tr1::array<double,DIM> > points;
    points.reserve(sample_points);
    ScalableGaussND gauss(gaussian_mean, gaussian_sigma, DIM);
    for (unsigned i=0; i<sample_points; ++i)
    {
        std::tr1::array<double,DIM> r;
        gauss.random(rng, &r[0], DIM);
        points.push_back(r);
    }

    // Discretize the sequence on a grid. We will use a histogram
    // to represent the discretized empirical density.
    std::vector<HistoAxis> axes;
    for (unsigned i=0; i<DIM; ++i)
        axes.push_back(HistoAxis(grid_points[i], interval_min[i], interval_max[i]));
    HistoND<double> sampleHistogram(axes);
    for (unsigned i=0; i<sample_points; ++i)
        sampleHistogram.fill(&points[i][0], DIM, 1.0);

    // Filter the histogrammed sample. The result is written into
    // an array object. This is our reconstructed density.
    ArrayND<double> reconstructed(dataShape);
    lorpeFilter.filter(sampleHistogram.binContents(), &reconstructed);

    // Chop off negative values of the reconstructed density
    reconstructed.makeNonNegative();

    // Normalize reconstructed density so that its integral is 1
    const double cell_size = sampleHistogram.binVolume();
    const double lorpeIntegral = reconstructed.sum<long double>()*cell_size;
    assert(lorpeIntegral > 0.0);
    reconstructed /= lorpeIntegral;

    // Calculate the ISE for this sample. For this, we need to
    // discretize the actual distribution according to which
    // the random points were generated. The distribution density
    // will be integrated across each discretization grid cell
    // using Gauss-Legendre quadrature.
    BoxND<double> scanbox;
    for (unsigned i=0; i<DIM; ++i)
        scanbox.push_back(Interval<double>(interval_min[i], interval_max[i]));
    ArrayND<double> original(dataShape);
    DensityFunctorND gaussFcn(gauss);
    double coords[DIM];
    for (BoxNDScanner<double> scanner(scanbox,dataShape);
         scanner.isValid(); ++scanner)
    {
        scanner.getCoords(coords, DIM);
        original.linearValue(scanner.state()) = rectangleIntegralCenterAndSize(
            gaussFcn, coords, grid_step, DIM, 4);
    }

    // Normalize the original density inside the grid interval to 1
    const double normalizationIntegral = original.sum<long double>()*cell_size;
    assert(normalizationIntegral > 0.0);
    original /= normalizationIntegral;

    // The actual ISE calculation
    const double ise = (reconstructed - original).sumsq<long double>()*cell_size;

    // Print the ISE value to the standard output
    std::cout << "LOrPE ISE for the current mutivariate random sample is "
              << ise << std::endl;

    // Some cleanup (release memory allocated inside this program)
    delete kernel;

    // We are done
    return 0;
}
