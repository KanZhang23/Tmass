#include <cmath>
#include <stdexcept>

#include "npstat/nm/GaussLegendreQuadrature.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/gaussianResponseMatrix.hh"

namespace npstat {
    Matrix<double> gaussianResponseMatrix(
        const double unfoldedMin, const double unfoldedMax,
        const unsigned nUnfolded,
        const double observedMin, const double observedMax,
        const unsigned nObserved,
        const Functor1<double, double>& gaussMeanFunction,
        const Functor1<double, double>& gaussWidthFunction,
        const unsigned nIntegrationPoints)
    {
        const unsigned maxIntegrationPoints = 256U;

        if (nUnfolded == 0 || nObserved == 0) throw std::invalid_argument(
            "In npstat::gaussianResponseMatrix: "
            "number of intervals must be positive");
        if (nIntegrationPoints > maxIntegrationPoints)
            throw std::invalid_argument(
                "In npstat::gaussianResponseMatrix: "
                "number of integration points is too large");

        GaussLegendreQuadrature quad(nIntegrationPoints);
        assert(quad.npoints() == nIntegrationPoints);

        long double abscissae[maxIntegrationPoints];
        long double weights[maxIntegrationPoints];
        const unsigned halfpoints = nIntegrationPoints/2;
        quad.getAbscissae(abscissae, halfpoints);
        quad.getWeights(weights, halfpoints);
        for (unsigned i=0; i<halfpoints; ++i)
        {
            abscissae[i + halfpoints] = -abscissae[i];
            weights[i + halfpoints] = weights[i];
        }

        const double unfoldedstep = (unfoldedMax - unfoldedMin)/nUnfolded;
        const double observedstep = (observedMax - observedMin)/nObserved;
        const double unfoldedUnit = fabs(unfoldedstep/2.0);
        const double observedUnit = fabs(observedstep/2.0);

        double means[maxIntegrationPoints];
        double sigmas[maxIntegrationPoints];

        Matrix<double> result(nObserved, nUnfolded);

        for (unsigned iunf = 0; iunf < nUnfolded; ++iunf)
        {
            const double umid = unfoldedMin + (iunf + 0.5)*unfoldedstep;
            for (unsigned i=0; i<nIntegrationPoints; ++i)
            {
                const double x = umid + unfoldedUnit*abscissae[i];
                means[i] = gaussMeanFunction(x);
                sigmas[i] = gaussWidthFunction(x);
            }

            for (unsigned iobs = 0; iobs < nObserved; ++iobs)
            {
                const double omid = observedMin + (iobs + 0.5)*observedstep;
                long double bigsum = 0.0L;
                for (unsigned i=0; i<nIntegrationPoints; ++i)
                {
                    Gauss1D g(means[i], sigmas[i]);
                    long double sum = 0.0L;
                    for (unsigned j=0; j<nIntegrationPoints; ++j)
                    {
                        const double x = omid + observedUnit*abscissae[j];
                        sum += weights[j]*g.density(x);
                    }
                    bigsum += weights[i]*sum;
                }
                result[iobs][iunf] = 0.5*bigsum*observedUnit;
            }
        }

        return result;
    }


    Matrix<double> gaussianResponseMatrix(
        const NUHistoAxis& unfoldedAxis,
        const NUHistoAxis& observedAxis,
        const Functor1<double, double>& gaussMeanFunction,
        const Functor1<double, double>& gaussWidthFunction,
        const unsigned nIntegrationPoints)
    {
        const unsigned maxIntegrationPoints = 256U;

        if (nIntegrationPoints > maxIntegrationPoints)
            throw std::invalid_argument(
                "In npstat::gaussianResponseMatrix: "
                "number of integration points is too large");

        GaussLegendreQuadrature quad(nIntegrationPoints);
        assert(quad.npoints() == nIntegrationPoints);

        long double abscissae[maxIntegrationPoints];
        long double weights[maxIntegrationPoints];
        const unsigned halfpoints = nIntegrationPoints/2;
        quad.getAbscissae(abscissae, halfpoints);
        quad.getWeights(weights, halfpoints);
        for (unsigned i=0; i<halfpoints; ++i)
        {
            abscissae[i + halfpoints] = -abscissae[i];
            weights[i + halfpoints] = weights[i];
        }

        const unsigned nUnfolded = unfoldedAxis.nBins();
        const unsigned nObserved = observedAxis.nBins();

        double means[maxIntegrationPoints];
        double sigmas[maxIntegrationPoints];

        Matrix<double> result(nObserved, nUnfolded);

        for (unsigned iunf = 0; iunf < nUnfolded; ++iunf)
        {
            const double unfoldedUnit = unfoldedAxis.binWidth(iunf)/2.0;
            const double umid = unfoldedAxis.binCenter(iunf);

            for (unsigned i=0; i<nIntegrationPoints; ++i)
            {
                const double x = umid + unfoldedUnit*abscissae[i];
                means[i] = gaussMeanFunction(x);
                sigmas[i] = gaussWidthFunction(x);
            }

            for (unsigned iobs = 0; iobs < nObserved; ++iobs)
            {
                const double observedUnit = observedAxis.binWidth(iobs)/2.0;
                const double omid = observedAxis.binCenter(iobs);

                long double bigsum = 0.0L;
                for (unsigned i=0; i<nIntegrationPoints; ++i)
                {
                    Gauss1D g(means[i], sigmas[i]);
                    long double sum = 0.0L;
                    for (unsigned j=0; j<nIntegrationPoints; ++j)
                    {
                        const double x = omid + observedUnit*abscissae[j];
                        sum += weights[j]*g.density(x);
                    }
                    bigsum += weights[i]*sum;
                }
                result[iobs][iunf] = 0.5*bigsum*observedUnit;
            }
        }

        return result;
    }
}
