#include <cassert>
#include <stdexcept>

#include "npstat/nm/LinearMapper1d.hh"

#include "npstat/stat/scanDensityAsWeight.hh"
#include "npstat/stat/DensityScanND.hh"

namespace npstat {
    CPP11_auto_ptr<ArrayND<double> > scanDensityAsWeight(
        const AbsDistributionND& kernel,
        const unsigned* maxOctantDim, const double* bandwidthSet,
        const double* stepSize, const unsigned dim,
        const bool fillOneOctantOnly)
    {
        if (!dim) throw std::invalid_argument(
            "In npstat::scanDensityAsWeight: empty input arrays");
        if (kernel.dim() != dim) std::invalid_argument(
            "In npstat::scanDensityAsWeight: "
            "incompatible argument dimensionalities");
        assert(maxOctantDim);
        assert(bandwidthSet);
        assert(stepSize);

        // Make sure inputs make sense and calculate normalization
        long double ldnorm = 1.0L;
        for (unsigned idim=0; idim<dim; ++idim)
        {
            if (!maxOctantDim[idim]) throw std::invalid_argument(
                "In npstat::scanDensityAsWeight: maximum number "
                "of steps must be positive in each dimension");
            if (!stepSize[idim]) throw std::invalid_argument(
                "In npstat::scanDensityAsWeight: step size in each "
                "dimension must not be zero");
            if (bandwidthSet[idim] <= 0.0) throw std::invalid_argument(
                "In npstat::scanDensityAsWeight: bandwidth "
                "must be positive in each dimension");
            ldnorm *= bandwidthSet[idim];
        }

        ArrayShape weightShape(dim);
        std::vector<double> coordBuf(dim);
        double* coords = &coordBuf[0];
        std::vector<LinearMapper1d> maps;
        maps.reserve(dim);

        // Determine the shape of the array we need to build
        for (unsigned idim=0; idim<dim; ++idim)
        {
            for (unsigned j=0; j<dim; ++j)
                coords[j] = 0.0;
            const unsigned nbins = maxOctantDim[idim];
            const double a = stepSize[idim]/bandwidthSet[idim];
            unsigned imax = 1U;
            for (; imax<nbins; ++imax)
            {
                coords[idim] = a*imax;
                if (kernel.density(coords, dim) <= 0.0)
                    break;
            }
            if (fillOneOctantOnly)
            {
                weightShape[idim] = imax;
                maps.push_back(LinearMapper1d(a, 0.0));
            }
            else
            {
                weightShape[idim] = 2U*imax - 1U;
                const unsigned imm1 = imax - 1U;
                maps.push_back(LinearMapper1d(0.0, -a*imm1, imm1, 0.0));
            }
        }

        // Create and fill the array with the given shape
        CPP11_auto_ptr<ArrayND<double> > arr(new ArrayND<double>(weightShape));
        DensityScanND<LinearMapper1d> scanner(kernel, maps, 1.0L/ldnorm);
        arr->functorFill(scanner);

        return arr;
    }
}
