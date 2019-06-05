#include <cmath>

#include "npstat/nm/allocators.hh"
#include "npstat/nm/OrthoPoly1D.hh"

namespace npstat {
    OrthoPoly1D::OrthoPoly1D()
        : weight_(0), poly_(0), step_(0.0L), nw_(0), maxdeg_(0) {}

    OrthoPoly1D::OrthoPoly1D(const OrthoPoly1D& r)
        : weight_(0), poly_(0), step_(r.step_), nw_(r.nw_), maxdeg_(r.maxdeg_)
    {
        if (nw_)
        {
            weight_ = new long double[nw_];
            poly_ = new long double[nw_*(maxdeg_+1)];
            copyBuffer(weight_, r.weight_, nw_);
            copyBuffer(poly_, r.poly_, nw_*(maxdeg_+1));
        }
    }

    long double OrthoPoly1D::scalarProduct(
        const long double *x, const long double *y) const
    {
        long double sum = 0.0L;
        if (nw_ > 1U)
        {
            // Symmetric sum from both ends.
            // This can sometimes lead to useful
            // cancellation of round-off errors.
            //
            const unsigned midpoint = nw_/2;
            for (unsigned i=0; i<midpoint; ++i)
                sum += x[i]*y[i]*weight_[i];
            long double rsum = 0.0L;
            for (unsigned i=nw_-1U; i>=midpoint; --i)
                rsum += x[i]*y[i]*weight_[i];
            sum += rsum;
        }
        else
            sum = x[0]*y[0]*weight_[0];
        return sum*step_;
    }

    long double OrthoPoly1D::unweightedProduct(
        const long double *x, const long double *y) const
    {
        long double sum = 0.0L;
        if (nw_ > 1U)
        {
            const unsigned midpoint = nw_/2;
            for (unsigned i=0; i<midpoint; ++i)
                sum += x[i]*y[i];
            long double rsum = 0.0L;
            for (unsigned i=nw_-1U; i>=midpoint; --i)
                rsum += x[i]*y[i];
            sum += rsum;
        }
        else
            sum = x[0]*y[0];
        return sum*step_;
    }

    long double OrthoPoly1D::scalarProduct(
        const double *x, const long double *y) const
    {
        long double sum = 0.0L;
        if (nw_ > 1U)
        {
            const unsigned midpoint = nw_/2;
            for (unsigned i=0; i<midpoint; ++i)
                sum += x[i]*y[i]*weight_[i];
            long double rsum = 0.0L;
            for (unsigned i=nw_-1U; i>=midpoint; --i)
                rsum += x[i]*y[i]*weight_[i];
            sum += rsum;
        }
        else
            sum = x[0]*y[0]*weight_[0];
        return sum*step_;
    }

    double OrthoPoly1D::empiricalKroneckerDelta(
        const unsigned n, const unsigned m) const
    {
        if (n > maxdeg_ || m > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::empiricalKroneckerDelta: "
            "polynomial degree out of range");
        return scalarProduct(poly_ + n*nw_, poly_ + m*nw_);
    }

    double OrthoPoly1D::unweightedPolyProduct(
        const unsigned n, const unsigned m) const
    {
        if (n > maxdeg_ || m > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::unweightedPolyProduct: "
            "polynomial degree out of range");
        return unweightedProduct(poly_ + n*nw_, poly_ + m*nw_);
    }

    double OrthoPoly1D::weightExpansionCovariance(
        const unsigned n, const unsigned m) const
    {
        if (n > maxdeg_ || m > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::weightExpansionCovariance: "
            "polynomial degree out of range");
        const long double* pn = poly_ + n*nw_;
        const long double* pm = poly_ + m*nw_;
        const long double an = scalarProduct(weight_, pn);
        const long double am = (n == m ? an : scalarProduct(weight_, pm));
        long double sum = 0.0L;
        if (nw_ > 1U)
        {
            const unsigned midpoint = nw_/2;
            for (unsigned i=0; i<midpoint; ++i)
            {
                const long double w = weight_[i];
                sum += w*w*w*pn[i]*pm[i];
            }
            long double rsum = 0.0L;
            for (unsigned i=nw_-1U; i>=midpoint; --i)
            {
                const long double w = weight_[i];
                rsum += w*w*w*pn[i]*pm[i];
            }
            sum += rsum;
        }
        else
        {
            const long double w = weight_[0];
            sum = w*w*w*pn[0]*pm[0];
        }
        return sum*step_ - an*am;
    }

    void OrthoPoly1D::gramSchmidt(const unsigned startingDegree)
    {
        for (unsigned ideg=startingDegree; ideg<=maxdeg_; ++ideg)
        {
            long double *data_deg = poly_ + ideg*nw_;
            for (unsigned i=0; i<ideg; ++i)
            {
                const long double *data_i = poly_ + i*nw_;
                const long double norm = scalarProduct(data_deg, data_i);
                for (unsigned j=0; j<nw_; ++j)
                    data_deg[j] -= norm*data_i[j];
            }
            long double norm = scalarProduct(data_deg, data_deg);
            assert(norm > 0.0L);
            norm = sqrtl(norm);
            for (unsigned i=0; i<nw_; ++i)
                data_deg[i] /= norm;
        }
    }

    OrthoPoly1D::OrthoPoly1D(const unsigned maxDeg, const double* weight,
                             const unsigned weightLen, const double istep)
        : weight_(0), poly_(0), step_(istep), nw_(weightLen), maxdeg_(maxDeg)
    {
        if (!nw_ || nw_ <= maxdeg_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D constructor: "
            "insufficient number of elements in the weight array");
        if (step_ <= 0.0) throw std::invalid_argument(
            "In npstat::OrthoPoly1D constructor: "
            "step size must be positive");
        assert(weight);
        bool havePositive = false;
        for (unsigned i=0; i<nw_; ++i)
        {
            if (weight[i] < 0.0)
                throw std::invalid_argument(
                    "In npstat::OrthoPoly1D constructor: "
                    "all weights must be non-negative");
            if (weight[i] > 0.0)
                havePositive = true;
        }
        if (!havePositive) throw std::invalid_argument(
            "In npstat::OrthoPoly1D constructor: no positive weights");

        weight_ = new long double[nw_];
        for (unsigned i=0; i<nw_; ++i)
            weight_[i] = weight[i];

        poly_ = new long double[nw_*(maxdeg_+1)];

        // Generate the 0th order polynomial
        for (unsigned i=0; i<nw_; ++i)
            poly_[i] = 1.0L;

        // Normalize the weight
        const long double norm = scalarProduct(poly_, poly_);
        for (unsigned i=0; i<nw_; ++i)
            weight_[i] /= norm;

        // Start with the Legendre basis. This is usually better
        // than starting with monomials.
        if (maxdeg_)
        {
            // 1st degree poly
            const long double ldstep = 2.0L / nw_;
            long double *xdata = poly_ + nw_;
            for (unsigned i=0; i<nw_; ++i)
                xdata[i] = -1.0L + (i + 0.5L)*ldstep;
            if (nw_ % 2U)
                xdata[nw_ / 2U] = 0.0L;

            // subsequent degrees
            for (unsigned ideg=2; ideg<=maxDeg; ++ideg)
            {
                long double *data = poly_ + ideg*nw_;
                const long double *pminus1 = poly_ + (ideg-1U)*nw_;
                const long double *pminus2 = poly_ + (ideg-2U)*nw_;

                for (unsigned i=0; i<nw_; ++i)
                    data[i] = ((2*ideg-1U)*xdata[i]*pminus1[i] - 
                               (ideg-1U)*pminus2[i])/ideg;
            }

            // Generate the monomials (old code version)
            // for (unsigned ideg=1; ideg<=maxDeg; ++ideg)
            // {
            //     long double *data = poly_ + ideg*nw_;
            //     for (unsigned i=0; i<nw_; ++i)
            //         data[i] = powl(i*istep, ideg);
            // }

            // Apply the Gram-Schmidt process.
            // Repeat it to improve orthogonality.
            gramSchmidt(1);
            gramSchmidt(1);
        }
    }

    OrthoPoly1D::~OrthoPoly1D()
    {
        delete [] poly_;
        delete [] weight_;
    }

    OrthoPoly1D& OrthoPoly1D::operator=(const OrthoPoly1D& r)
    {
        if (this != &r)
        {
            delete [] poly_; poly_ = 0;
            delete [] weight_; weight_ = 0;
            step_ = r.step_;
            nw_ = r.nw_;
            maxdeg_ = r.maxdeg_;
            if (nw_)
            {
                weight_ = new long double[nw_];
                poly_ = new long double[nw_*(maxdeg_+1)];
                copyBuffer(weight_, r.weight_, nw_);
                copyBuffer(poly_, r.poly_, nw_*(maxdeg_+1));
            }
        }
        return *this;
    }

    double OrthoPoly1D::weight(const unsigned ix) const
    {
        if (!weight_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D::weight: uninitilalized polynomial set");
        if (ix >= nw_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::weight: point index out of range");
        return weight_[ix];
    }

    double OrthoPoly1D::poly(const unsigned deg, const unsigned ix) const
    {
        if (!poly_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D::poly: uninitilalized polynomial set");
        if (deg > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::poly: poly degree out of range");
        if (ix >= nw_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::poly: point index out of range");
        return poly_[deg*nw_ + ix];
    }

    double OrthoPoly1D::polyTimesWeight(const unsigned deg,
                                        const unsigned ix) const
    {
        if (!poly_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D::polyTimesWeight: "
            "uninitilalized polynomial set");
        if (deg > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::polyTimesWeight: poly degree out of range");
        if (ix >= nw_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::polyTimesWeight: point index out of range");
        return poly_[deg*nw_ + ix]*weight_[ix];
    }

    double OrthoPoly1D::series(const double *coeffs,
                               const unsigned maxdeg,
                               const unsigned ux) const
    {
        if (!poly_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D::series: uninitilalized polynomial set");
        if (maxdeg > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::series: filter degree out of range");
        if (ux >= nw_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::series: point index out of range");
        assert(coeffs);

        long double sum = 0.0L;
        const long double* localPoly = poly_ + ux;
        for (unsigned deg=0; deg <= maxdeg; ++deg)
            sum += coeffs[deg]*localPoly[deg*nw_];
        return sum;
    }

    void OrthoPoly1D::weightExpansionCoeffs(double *coeffs,
                                            const unsigned maxdeg) const
    {
        if (maxdeg > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::weightExpansionCoeffs: "
            "maximum degree out of range");
        assert(coeffs);

        for (unsigned deg=0; deg<=maxdeg; ++deg)
            coeffs[deg] = scalarProduct(weight_, poly_ + deg*nw_);
    }

    void OrthoPoly1D::calculateCoeffs(const double *data,
                                      const unsigned dataLen,
                                      double *coeffs,
                                      const unsigned maxdeg) const
    {
        if (!weight_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D::calculateCoeffs: "
            "uninitilalized polynomial set");
        if (dataLen != nw_) throw std::invalid_argument(
            "In npstat::OrthoPoly1D::calculateCoeffs: "
            "incompatible data length");
        if (maxdeg > maxdeg_) throw std::out_of_range(
            "In npstat::OrthoPoly1D::calculateCoeffs: "
            "maximum degree out of range");
        assert(data);
        assert(coeffs);

        for (unsigned deg=0; deg<=maxdeg; ++deg)
            coeffs[deg] = scalarProduct(data, poly_ + deg*nw_);
    }

    bool OrthoPoly1D::operator==(const OrthoPoly1D& r) const
    {
        if (step_ != r.step_ || nw_ != r.nw_ || maxdeg_ != r.maxdeg_)
            return false;
        for (unsigned i=0; i<nw_; ++i)
            if (weight_[i] != r.weight_[i])
                return false;
        const unsigned len = (maxdeg_+1)*nw_;
        for (unsigned i=0; i<len; ++i)
            if (poly_[i] != r.poly_[i])
                return false;
        return true;
    }

    bool OrthoPoly1D::operator!=(const OrthoPoly1D& r) const
    {
        return !(*this == r);
    }
}
