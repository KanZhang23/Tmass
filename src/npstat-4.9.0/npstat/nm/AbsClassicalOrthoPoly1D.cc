#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include "npstat/nm/StorablePolySeries1D.hh"
#include "npstat/nm/GaussLegendreQuadrature.hh"

namespace npstat {
    namespace Private {
        class ClassicalPolyPower : public Functor1<long double, long double>
        {
        public:
            inline ClassicalPolyPower(const AbsClassicalOrthoPoly1D& p,
                                      const unsigned deg, const unsigned power,
                                      const bool i_useWeight)
                : poly(p), deg1(deg), polypow(power), useWeight(i_useWeight) {}

            inline long double operator()(const long double& x) const
            {
                long double p = 1.0L, w = 1.0L;
                if (useWeight)
                    w = poly.weight(x);
                if (polypow)
                {
                    p = poly.poly(deg1, x);
                    switch (polypow)
                    {
                    case 1U:
                        break;
                    case 2U:
                        p *= p;
                        break;
                    default:
                        p = powl(p, polypow);
                        break;
                    }
                }
                return w*p;
            }

        private:
            const AbsClassicalOrthoPoly1D& poly;
            unsigned deg1;
            unsigned polypow;
            bool useWeight;
        };
    }

    long double AbsClassicalOrthoPoly1D::poly(
        const unsigned degree, const long double x) const
    {
        long double polyk = 1.0L;
        if (degree)
        {
            if (degree > maxDegree()) throw std::invalid_argument(
                "In npstat::AbsClassicalOrthoPoly1D::poly: degree argument is out of range");
            long double polykm1 = 0.0L;
            std::pair<long double,long double> rcurrent = recurrenceCoeffs(0);
            for (unsigned k=0; k<degree; ++k)
            {
                const std::pair<long double,long double>& rnext = recurrenceCoeffs(k+1);
                const long double p = ((x - rcurrent.first)*polyk -
                                       rcurrent.second*polykm1)/rnext.second;
                polykm1 = polyk;
                polyk = p;
                rcurrent = rnext;
            }
        }
        return polyk;
    }

    long double AbsClassicalOrthoPoly1D::normpolyprod(
        const unsigned* degrees, const unsigned nDegrees,
        const unsigned maxdeg, const long double x) const
    {
        long double prod = 1.0L;
        if (nDegrees)
        {
            assert(degrees);
            if (maxdeg)
            {
                if (maxdeg > maxDegree()) throw std::invalid_argument(
                    "In npstat::AbsClassicalOrthoPoly1D::normpolyprod: "
                    "maximum degree argument is out of range");
                long double polyk = 1.0L, polykm1 = 0.0L;
                std::pair<long double,long double> rcurrent = recurrenceCoeffs(0);
                for (unsigned k=0; k<maxdeg; ++k)
                {
                    for (unsigned j=0; j<nDegrees; ++j)
                        if (k == degrees[j])
                            prod *= polyk;
                    const std::pair<long double,long double>& rnext = recurrenceCoeffs(k+1);
                    const long double p = ((x - rcurrent.first)*polyk -
                                           rcurrent.second*polykm1)/rnext.second;
                    polykm1 = polyk;
                    polyk = p;
                    rcurrent = rnext;
                }
                for (unsigned j=0; j<nDegrees; ++j)
                    if (maxdeg == degrees[j])
                        prod *= polyk;
            }
        }
        return prod;
    }

    std::pair<long double,long double>
    AbsClassicalOrthoPoly1D::twopoly(
        const unsigned deg1, const unsigned deg2, const long double x) const
    {
        long double p1 = 1.0L, p2 = 1.0L;
        const unsigned degree = std::max(deg1, deg2);
        if (degree)
        {
            if (degree > maxDegree()) throw std::invalid_argument(
                "In npstat::AbsClassicalOrthoPoly1D::twopoly: "
                "degree argument is out of range");
            long double polyk = 1.0L, polykm1 = 0.0L;
            std::pair<long double,long double> rcurrent = recurrenceCoeffs(0);
            for (unsigned k=0; k<degree; ++k)
            {
                if (k == deg1)
                    p1 = polyk;
                if (k == deg2)
                    p2 = polyk;
                const std::pair<long double,long double>& rnext = recurrenceCoeffs(k+1);
                const long double p = ((x - rcurrent.first)*polyk -
                                       rcurrent.second*polykm1)/rnext.second;
                polykm1 = polyk;
                polyk = p;
                rcurrent = rnext;
            }
            if (deg1 == degree)
                p1 = polyk;
            if (deg2 == degree)
                p2 = polyk;
        }
        return std::pair<long double,long double>(p1, p2);
    }

    double AbsClassicalOrthoPoly1D::series(const double *coeffs,
                                           const unsigned degree,
                                           const double xIn) const
    {
        assert(coeffs);
        long double sum = coeffs[0];
        if (degree)
        {
            if (degree > maxDegree()) throw std::invalid_argument(
                "In npstat::AbsClassicalOrthoPoly1D::series: "
                "degree argument is out of range");
            const long double x = xIn;
            long double polyk = 1.0L, polykm1 = 0.0L;
            std::pair<long double,long double> rcurrent = recurrenceCoeffs(0);
            for (unsigned k=0; k<degree; ++k)
            {
                const std::pair<long double,long double>& rnext = recurrenceCoeffs(k+1);
                const long double p = ((x - rcurrent.first)*polyk -
                                       rcurrent.second*polykm1)/rnext.second;
                sum += p*coeffs[k+1];
                polykm1 = polyk;
                polyk = p;
                rcurrent = rnext;
            }
        }
        return sum;
    }

    void AbsClassicalOrthoPoly1D::allpoly(const long double x,
                                          long double* values,
                                          const unsigned degree) const
    {
        assert(values);
        values[0] = 1.0L;
        if (degree)
        {
            if (degree > maxDegree()) throw std::invalid_argument(
                "In npstat::AbsClassicalOrthoPoly1D::allpoly: "
                "degree argument is out of range");
            long double polyk = 1.0L, polykm1 = 0.0L;
            std::pair<long double,long double> rcurrent = recurrenceCoeffs(0);
            for (unsigned k=0; k<degree; ++k)
            {
                const std::pair<long double,long double>& rnext = recurrenceCoeffs(k+1);
                const long double p = ((x - rcurrent.first)*polyk -
                                       rcurrent.second*polykm1)/rnext.second;
                polykm1 = polyk;
                polyk = p;
                rcurrent = rnext;
                values[k+1] = p;
            }
        }
    }

    double AbsClassicalOrthoPoly1D::jointIntegral(
        const unsigned* degrees, const unsigned nDegrees) const
    {
        if (nDegrees)
        {
            assert(degrees);
            const unsigned degSum = std::accumulate(degrees, degrees+nDegrees, 0U);
            const unsigned nPt = GaussLegendreQuadrature::minimalExactRule(degSum);
            if (!nPt) throw std::invalid_argument(
                "In npstat::AbsClassicalOrthoPoly1D::jointIntegral: "
                "joint poly degree is too high");
            GaussLegendreQuadrature quad(nPt);
            MultiProdFcn fcn(*this, degrees, nDegrees, false);
            return quad.integrate(fcn, xmin(), xmax());
        }
        else
            return xmax() - xmin();
    }

    double AbsClassicalOrthoPoly1D::integratePoly(
        const unsigned degree, const unsigned power) const
    {
        const unsigned degSum = degree*power;
        if (degSum)
        {
            const unsigned nPt = GaussLegendreQuadrature::minimalExactRule(degSum);
            if (!nPt) throw std::invalid_argument(
                "In npstat::AbsClassicalOrthoPoly1D::integratePoly: "
                "poly degree or power is too high");
            GaussLegendreQuadrature quad(nPt);
            Private::ClassicalPolyPower fcn(*this, degree, power, false);
            return quad.integrate(fcn, xmin(), xmax());
        }
        else
            return xmax() - xmin();
    }

    CPP11_auto_ptr<StorablePolySeries1D> AbsClassicalOrthoPoly1D::makeStorablePolySeries(
        const unsigned maxPolyDeg, const double *coeffs, unsigned maxdeg) const
    {
        if (maxPolyDeg > maxDegree()) throw std::invalid_argument(
            "In npstat::AbsClassicalOrthoPoly1D::makeStorablePolySeries: "
            "poly degree argument is out of range");
        std::vector<std::pair<long double,long double> > rc(maxPolyDeg+1U);
        for (unsigned i=0; i<=maxPolyDeg; ++i)
            rc[i] = this->recurrenceCoeffs(i);
        return CPP11_auto_ptr<StorablePolySeries1D>(
            new StorablePolySeries1D(rc, this->xmin(), this->xmax(), 0.0, coeffs, maxdeg));        
    }
}
