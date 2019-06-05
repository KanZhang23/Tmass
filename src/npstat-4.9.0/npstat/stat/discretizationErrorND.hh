#ifndef NPSTAT_DISCRETIZATIONERRORND_HH_
#define NPSTAT_DISCRETIZATIONERRORND_HH_

/*!
// \file discretizationErrorND.hh
//
// \brief Calculate the ISE due to binning for multivariate densities
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/DensityAveScanND.hh"
#include "npstat/stat/HistoND.hh"

namespace npstat {
    template<class Axis>
    double discretizationErrorND(
        const AbsDistributionND& fcn, const std::vector<Axis>& axes,
        const unsigned pointsPerBinWhenDiscretizing,
        const unsigned nIntegrationPoints, const bool normalize=true)
    {
        // First, discretize the density
        ArrayND<double> densityScan(Private::makeHistoShape(axes));
        densityScan.functorFill(makeDensityAveScanND(
            fcn, axes, pointsPerBinWhenDiscretizing));

        // Now, go over all bins and accumulate the ISE.
        // We also need to accumulate the density integral
        // in order to have proper normalization.
        const unsigned dim = fcn.dim();
        const unsigned long len = densityScan.length();
        long double integ = 0.0L;
        long double ise = 0.0L;
        double center[CHAR_BIT*sizeof(unsigned long)];
        double size[CHAR_BIT*sizeof(unsigned long)];
        unsigned index[CHAR_BIT*sizeof(unsigned long)];
        for (unsigned long ipt=0; ipt<len; ++ipt)
        {
            densityScan.convertLinearIndex(ipt, index, dim);
            double volume = 1.0;
            for (unsigned i=0; i<dim; ++i)
            {
                center[i] = axes[i].binCenter(index[i]);
                const double width = axes[i].binWidth(index[i]);
                volume *= width;
                size[i] = width;
            }
            const double c = densityScan.linearValue(ipt);
            integ += c*volume;
            DensityDiscretizationErrorND err(fcn, 1.0, c);
            ise += rectangleIntegralCenterAndSize(
                err, center, size, dim, nIntegrationPoints);
        }
        if (normalize)
        {
            if (integ > 0.0L)
                return ise/integ/integ;
            else
                return 0.0;
        }
        else
            return ise;
    }
}

#endif // NPSTAT_DISCRETIZATIONERRORND_HH_
