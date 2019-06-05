#include <cmath>
#include <stdexcept>

#include "npstat/stat/multinomialCovariance1D.hh"
#include "npstat/stat/DensityScan1D.hh"

#include "npstat/nm/ArrayND.hh"

namespace npstat {
    Matrix<double> multinomialCovariance1D(
        const AbsDistribution1D& fcn, const double N,
        const unsigned nbins, double xmin, double xmax,
        const unsigned nIntegrationPoints)
    {
        if (N <= 0.0) throw std::invalid_argument(
            "In npstat::multinomialCovariance1D: invalid sample size");
        if (xmin == xmax) throw std::invalid_argument(
            "In npstat::multinomialCovariance1D: "
            "zero length discretization interval");

        ArrayND<double> densityScan(nbins);
        densityScan.functorFill(DensityScan1D(fcn, 1.0, nbins,
                                              xmin, xmax, nIntegrationPoints));
        if (!densityScan.isDensity()) throw std::invalid_argument(
            "In npstat::multinomialCovariance1D: "
            "density is not supported on the given interval");
        const double binwidth = std::abs((xmax - xmin)/nbins);
        const double integ = binwidth*densityScan.sum<long double>();
        densityScan /= integ;

        const double* d = densityScan.data();
        Matrix<double> m(nbins, nbins);
        for (unsigned i=0; i<nbins; ++i)
        {
            const double pi = d[i]*binwidth;
            double* mat = m[i];
            mat[i] = N*pi*(1.0 - pi);
            for (unsigned j=0; j<i; ++j)
            {
                const double c = -N*pi*d[j]*binwidth;
                mat[j] = c;
                m[j][i] = c;
            }
        }
        return m;
    }
}
