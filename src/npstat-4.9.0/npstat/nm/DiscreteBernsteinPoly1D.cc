#include <cmath>
#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "npstat/nm/DiscreteBernsteinPoly1D.hh"
#include "npstat/nm/binomialCoefficient.hh"
#include "npstat/nm/SpecialFunctions.hh"

#define GAUSS_LEGENDRE_POINTS 256U

namespace {
    class LongBeta : public npstat::Functor1<long double,long double>
    {
    public:
        inline LongBeta(const long double m,
                        const long double n,
                        const long double norm)
            : n_(n), m_(m), norm_(norm) {}

        inline long double operator()(const long double& t) const
        {
            assert(t >= 0.0L);
            assert(t <= 1.0L);
            const long double onemt = 1.0L - t;
            return norm_*powl(t, m_)*powl(onemt, n_-m_);
        }

    private:
        long double n_, m_, norm_;
    };
}

namespace npstat {
    DiscreteBernsteinPoly1D::DiscreteBernsteinPoly1D(
        const unsigned i_degree, const unsigned i_gridLength)
        : lookupTable_(i_gridLength*(i_degree+1U)),
          deriTable_(i_gridLength*(i_degree+1U)),
          degree_(i_degree),
          gridLength_(i_gridLength)
    {
        if (!i_gridLength) throw std::invalid_argument(
            "In npstat::DiscreteBernsteinPoly1D constructor: "
            "number of grid cells must be positive");
        if (degree_)
        {
            // We will integrate continuous Bernstein polynomials between
            // cell boundaries and will set the discrete polynomial values
            // to the average continuous polynomial value inside the cell.
            // For that we will need the values of continuous Bernstein
            // polynomials of degree degree_ + 1 at the cell boundaries.
            //
            const unsigned n = degree_ + 1U;
            const unsigned np1 = n + 1U;
            const unsigned datalenp1 = i_gridLength + 1U;
            const long double step = 1.0L/i_gridLength;

            std::vector<long double> tbuffer(2*datalenp1*np1);
            long double* tpowers = &tbuffer[0];
            long double* onemtpowers = tpowers + datalenp1*np1;

            for (unsigned loc=0; loc<=i_gridLength; ++loc)
            {
                const long double t = loc*1.0L/i_gridLength;
                const long double omt = 1.0L - t;
                long double tproduct = 1.0L, omtproduct = 1.0L;
                unsigned idx = loc*np1;
                for (unsigned ipow=0; ipow<=n; ++ipow, ++idx)
                {
                    tpowers[idx] = tproduct;
                    tproduct *= t;
                    onemtpowers[idx] = omtproduct;
                    omtproduct *= omt;
                }
            }

            std::vector<long double> binomialCoeffs(np1);
            long double* binomCoeffs = &binomialCoeffs[0];
            for (unsigned m=0; m<=n; ++m)
                binomCoeffs[m] = ldBinomialCoefficient(n, m);

            std::vector<long double> sumBuf;
            sumBuf.reserve(np1);

            // Fill out the table of average polynomial values
            for (unsigned polyNumber=0; polyNumber<=degree_; ++polyNumber)
            {
                long double oldInteg = 0.0L;
                for (unsigned icell=0; icell<gridLength_; ++icell)
                {
                    const unsigned locationIndex = icell+1U;
                    sumBuf.clear();
                    for (unsigned j=polyNumber+1U; j<=n; ++j)
                    {
                        const long double v = binomCoeffs[j]*
                            tpowers[locationIndex*np1 + j]*
                            onemtpowers[locationIndex*np1 + n - j];
                        sumBuf.push_back(v);
                    }
                    long double sum = 0.0L;
                    const unsigned sz = sumBuf.size();
                    if (sz)
                    {
                        // Sorting improves numerical precision of the sum
                        std::sort(sumBuf.begin(), sumBuf.end());
                        const long double* buf = &sumBuf[0];
                        for (unsigned i=0; i<sz; ++i)
                            sum += buf[i];
                    }
                    const long double integ = sum/n;

                    lookupTable_[polyNumber*gridLength_+icell] =
                        (integ - oldInteg)/step;
                    oldInteg = integ;
                }
            }

            // Fill out the table of average derivative values
            for (unsigned polyNumber=0; polyNumber<=degree_; ++polyNumber)
            {
                const double coeff = ldBinomialCoefficient(degree_, polyNumber);
                long double oldValue = coeff*tpowers[polyNumber]*
                    onemtpowers[degree_-polyNumber];
                unsigned loc = 1U;
                for (unsigned icell=0; icell<gridLength_; ++icell, ++loc)
                {
                    const long double v = coeff*tpowers[loc*np1 + polyNumber]*
                        onemtpowers[loc*np1 + degree_ - polyNumber];
                    deriTable_[polyNumber*gridLength_+icell] =
                        (v - oldValue)/step;
                    oldValue = v;
                }
            }
        }
        else
        {
            for (unsigned i=0; i<gridLength_; ++i)
            {
                lookupTable_[i] = 1.0;
                deriTable_[i] = 0.0;
            }
        }
    }

    double DiscreteBernsteinPoly1D::poly(const unsigned polyNumber,
                                         const unsigned gridIndex) const
    {
        if (polyNumber > degree_) throw std::out_of_range(
            "In npstat::DiscreteBernsteinPoly1D::poly: "
            "polynomial number out of range");
        if (gridIndex >= gridLength_) throw std::out_of_range(
            "In npstat::DiscreteBernsteinPoly1D::poly: "
            "grid cell number out of range");
        return lookupTable_[polyNumber*gridLength_ + gridIndex];
    }

    double DiscreteBernsteinPoly1D::derivative(const unsigned polyNumber,
                                               const unsigned gridIndex) const
    {
        if (polyNumber > degree_) throw std::out_of_range(
            "In npstat::DiscreteBernsteinPoly1D::derivative: "
            "polynomial number out of range");
        if (gridIndex >= gridLength_) throw std::out_of_range(
            "In npstat::DiscreteBernsteinPoly1D::derivative: "
            "grid cell number out of range");
        return deriTable_[polyNumber*gridLength_ + gridIndex];
    }

    DiscreteBeta1D::DiscreteBeta1D(const double effectiveDegree,
                                   const unsigned i_gridLength)
        : g_(GAUSS_LEGENDRE_POINTS), loggamma_(-1.0L), gamma_(-1.0L),
          step_(1.0L/i_gridLength), lastNorm_(-1.0L), lastEffPolyNumber_(-1.0L),
          degree_(effectiveDegree), gridLength_(i_gridLength)
    {
        if (!i_gridLength) throw std::invalid_argument(
            "In npstat::DiscreteBeta1D constructor: "
            "number of grid cells must be positive");
        if (effectiveDegree < 0.0) throw std::invalid_argument(
            "In npstat::DiscreteBeta1D constructor: "
            "negative effective degrees are not supported");
        if (effectiveDegree > 170.0)
            loggamma_ = lgammal(effectiveDegree + 1.0);
        else
            gamma_ = Gamma(effectiveDegree + 1.0);
    }

    long double DiscreteBeta1D::getNorm(const double polyNumber) const
    {
        if (polyNumber != lastEffPolyNumber_)
        {
            if (gamma_ >= 0.0L)
                lastNorm_ = gamma_/Gamma(polyNumber+1.0)/
                    Gamma(degree_-polyNumber+1.0);
            else
                lastNorm_ = expl(loggamma_ - lgammal(polyNumber+1.0) - 
                                 lgammal(degree_-polyNumber+1.0));
            lastEffPolyNumber_ = polyNumber;
        }
        return lastNorm_;
    }

    double DiscreteBeta1D::poly(const double polyNumber,
                                const unsigned gridIndex) const
    {
        if (polyNumber < -1.0 || polyNumber > degree_+1.0)
            throw std::out_of_range(
                "In npstat::DiscreteBeta1D::poly: "
                "effective polynomial number is out of range");
        if (gridIndex >= gridLength_) throw std::out_of_range(
            "In npstat::DiscreteBeta1D::poly: "
            "grid cell number is out of range");
        if (polyNumber == -1.0)
        {
            if (gridIndex == 0U)
                return 1.0/(degree_+1.0)/step_;
            else
                return 0.0;
        }
        if (polyNumber == degree_+1.0)
        {
            if (gridIndex == gridLength_ - 1U)
                return 1.0/(degree_+1.0)/step_;
            else
                return 0.0;
        }
        const LongBeta lb(polyNumber, degree_, this->getNorm(polyNumber));
        const long double xmin = gridIndex*step_;
        long double xmax = xmin + step_;
        if (xmax > 1.0L)
            xmax = 1.0L;
        return g_.integrate(lb, xmin, xmax)/step_;
    }

    double DiscreteBeta1D::derivative(const double polyNumber,
                                      const unsigned gridIndex) const
    {
        if (gridIndex >= gridLength_) throw std::out_of_range(
            "In npstat::DiscreteBeta1D::derivative: "
            "grid cell number out of range");
        if (polyNumber < -1.0 || polyNumber > degree_+1.0 ||
            (polyNumber < 0.0 && gridIndex == 0U) ||
            (polyNumber > degree_ && gridIndex == gridLength_ - 1U))
            throw std::out_of_range(
                "In npstat::DiscreteBeta1D::derivative: "
                "effective polynomial number out of range");

        if (polyNumber == -1.0 || polyNumber == degree_+1.0)
            return 0.0;

        const LongBeta lb(polyNumber, degree_, this->getNorm(polyNumber));
        const long double xmin = gridIndex*step_;
        long double xmax = xmin + step_;
        if (xmax > 1.0L)
            xmax = 1.0L;
        return (lb(xmax) - lb(xmin))/step_;
    }
}
