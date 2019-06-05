#include <cmath>
#include <stdexcept>

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/discretizedDistance.hh"

#include "npstat/stat/lorpeMise1D.hh"

#include "npstat/stat/multinomialCovariance1D.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/DensityScan1D.hh"

namespace npstat {
    double lorpeMise1D(const int power, const double lorpeDegree,
                       const double bandwidth,
                       const double N,
                       const unsigned nintervals,
                       const double xmin, const double xmax,
                       const AbsDistribution1D& distro,
                       const BoundaryHandling& bm,
                       const unsigned oversample,
                       double *ISB, double *variance)
    {
        if (N <= 0.0) throw std::invalid_argument(
            "In npstat::lorpeMise1D: invalid sample size");
        if (xmin == xmax) throw std::invalid_argument(
            "In npstat::lorpeMise1D: "
            "zero length discretization interval");

        // Scan the original density
        ArrayND<double> densityScan(nintervals);
        densityScan.functorFill(DensityScan1D(
            distro, 1.0, nintervals, xmin, xmax, oversample));
        if (!densityScan.isDensity()) throw std::invalid_argument(
            "In npstat::lorpeMise1D: "
            "density is not supported on the given interval");
        const double binsize = std::abs((xmax - xmin)/nintervals);
        double integ = binsize*densityScan.sum<long double>();
        densityScan /= integ;

        // Construct the multinomial covariance matrix
        const double* d = densityScan.data();
        Matrix<double> mcov(nintervals, nintervals);
        for (unsigned i=0; i<nintervals; ++i)
        {
            const double pi = d[i]*binsize;
            double* mat = mcov[i];
            mat[i] = N*pi*(1.0 - pi);
            for (unsigned j=0; j<i; ++j)
            {
                const double c = -N*pi*d[j]*binsize;
                mat[j] = c;
                mcov[j][i] = c;
            }
        }

        // Normalize the covariance for the empirical density
        mcov /= (N*N*binsize*binsize);

        // Build the LOrPE evaluator (LocalPolyFilter1D object)
        CPP11_auto_ptr<LocalPolyFilter1D> filter(symbetaLOrPEFilter1D(
            power, bandwidth, lorpeDegree, nintervals, xmin, xmax, bm));
        assert(filter.get());

        // Get the filter matrix
        const Matrix<double>& fmat = filter->getFilterMatrix();

        // Get the variance part of the MISE
        const double var = binsize*mcov.productTr(fmat.TtimesThis());

        // Get the integrated squared bias part of the MISE
        ArrayND<double> scan2(nintervals);
        filter->filter(d, nintervals, const_cast<double*>(scan2.data()));
        scan2.makeNonNegative();
        integ = binsize*scan2.sum<long double>();
        scan2 /= integ;
        const double squaredBias = discretizedL2(d, nintervals, scan2.data(),
                                                 nintervals, binsize);

        if (ISB)
            *ISB = squaredBias;
        if (variance)
            *variance = var;

        return squaredBias + var;
    }
}
