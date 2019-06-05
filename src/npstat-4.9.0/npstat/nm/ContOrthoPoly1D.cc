#include <algorithm>

#include "npstat/nm/ContOrthoPoly1D.hh"
#include "npstat/nm/PairCompare.hh"
#include "npstat/nm/allocators.hh"
#include "npstat/nm/StorablePolySeries1D.hh"

inline static int kdelta(const unsigned i, const unsigned j)
{
    return i == j ? 1 : 0;
}

namespace npstat {
    ContOrthoPoly1D::ContOrthoPoly1D(const unsigned maxDegree,
                                     const std::vector<MeasurePoint>& inMeasure,
                                     const OrthoPolyMethod m)
        : measure_(inMeasure),
          wsum_(0.0L),
          wsumsq_(0.0L),
          minX_(DBL_MAX),
          maxX_(-DBL_MAX),
          maxdeg_(maxDegree),
          allWeightsEqual_(true)
    {
        // Check argument validity
        if (measure_.empty()) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D constructor: empty measure");

        // We expect all weights to be equal quite often.
        // Check if this is indeed the case. If not, sort the
        // weights in the increasing order, hopefully reducing
        // the round-off error.
        const unsigned long mSize = measure_.size();
        const double w0 = measure_[0].second;
        for (unsigned long i = 1; i < mSize && allWeightsEqual_; ++i)
            allWeightsEqual_ = (w0 == measure_[i].second);
        if (!allWeightsEqual_)
            std::sort(measure_.begin(), measure_.end(), LessBySecond<MeasurePoint>());
        if (measure_[0].second < 0.0) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D constructor: negative measure entry found");

        unsigned long nZeroWeights = 0;
        if (!allWeightsEqual_)
            for (unsigned long i = 0; i < mSize; ++i)
            {
                if (measure_[i].second == 0.0)
                    ++nZeroWeights;
                else
                    break;
            }
        if (mSize <= maxDegree + nZeroWeights) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D constructor: insufficient number of measure points");

        // Sum up the weights
        long double xsum = 0.0L;
        if (allWeightsEqual_)
        {
            const long double lw0 = w0;
            wsum_ = mSize*lw0;
            wsumsq_ = mSize*lw0*lw0;
            for (unsigned long i = 0; i < mSize; ++i)
            {
                const double x = measure_[i].first;
                xsum += x;
                if (x < minX_)
                    minX_ = x;
                if (x > maxX_)
                    maxX_ = x;
            }
            xsum *= lw0;
        }
        else
        {
            for (unsigned long i = 0; i < mSize; ++i)
            {
                const double x = measure_[i].first;
                const double w = measure_[i].second;
                wsum_ += w;
                wsumsq_ += w*w;
                xsum += w*x;
                if (x < minX_)
                    minX_ = x;
                if (x > maxX_)
                    maxX_ = x;
            }
        }

        if (wsum_ == 0.0L) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D constructor: measure is not positive");
        meanX_ = xsum/wsum_;

        // Shift the measure
        for (unsigned long i = 0; i < mSize; ++i)
            measure_[i].first -= meanX_;

        // Try to improve the mean
        xsum = 0.0L;
        for (unsigned long i = 0; i < mSize; ++i)
            xsum += measure_[i].first*measure_[i].second;
        const double dx = xsum/wsum_;
        for (unsigned long i = 0; i < mSize; ++i)
            measure_[i].first -= dx;

        meanX_ += dx;
        calcRecurrenceCoeffs(m);
    }

    void ContOrthoPoly1D::calcRecurrenceCoeffs(const OrthoPolyMethod m)
    {
        rCoeffs_.clear();
        rCoeffs_.reserve(maxdeg_ + 1);
        switch (m)
        {
        case OPOLY_STIELTJES:
            calcRecurrenceStieltjes();
            break;

        case OPOLY_LANCZOS:
            calcRecurrenceLanczos();
            break;

        default:
            throw std::runtime_error(
                "In npstat::ContOrthoPoly1D::calcRecurrenceCoeffs: "
                "incomplete switch statement. This is a bug. Please report.");
        }
        assert(rCoeffs_.size() == maxdeg_ + 1);
    }

    double ContOrthoPoly1D::effectiveSampleSize() const
    {
        if (allWeightsEqual_)
            return measure_.size();
        else
            return wsum_*wsum_/wsumsq_;
    }

    long double ContOrthoPoly1D::monicpoly(const unsigned degree,
                                           const long double x) const
    {
        long double polyk = 1.0L, polykm1 = 0.0L;
        for (unsigned k=0; k<degree; ++k)
        {
            const long double p = (x - rCoeffs_[k].alpha)*polyk -
                                   rCoeffs_[k].beta*polykm1;
            polykm1 = polyk;
            polyk = p;
        }
        return polyk;
    }

    long double ContOrthoPoly1D::normpoly(const unsigned degree,
                                          const long double x) const
    {
        long double polyk = 1.0L, polykm1 = 0.0L;
        for (unsigned k=0; k<degree; ++k)
        {
            const long double p = ((x - rCoeffs_[k].alpha)*polyk -
                                   rCoeffs_[k].sqrbeta*polykm1)/rCoeffs_[k+1].sqrbeta;
            polykm1 = polyk;
            polyk = p;
        }
        return polyk;
    }

    double ContOrthoPoly1D::powerAverage(const unsigned deg,
                                         const unsigned power) const
    {
        switch (power)
        {
        case 0:
            return 1.0;
        case 1:
            return kdelta(deg, 0U);
        case 2:
            return 1.0;
        default:
            {
                if (deg > maxdeg_) throw std::invalid_argument(
                    "In npstat::ContOrthoPoly1D::powerAverage: "
                    "degree argument is out of range");
                const unsigned long mSize = measure_.size();
                long double sum = 0.0L;
                for (unsigned long i = 0; i < mSize; ++i)
                    sum += measure_[i].second*powl(normpoly(deg, measure_[i].first), power);
                return sum/wsum_;
            }
        }
    }

    std::pair<long double,long double> ContOrthoPoly1D::twonormpoly(
        const unsigned deg1, const unsigned deg2, const long double x) const
    {
        long double p1 = 0.0L, p2 = 0.0L, polyk = 1.0L, polykm1 = 0.0L;
        const unsigned degmax = std::max(deg1, deg2);
        for (unsigned k=0; k<degmax; ++k)
        {
            if (k == deg1)
                p1 = polyk;
            if (k == deg2)
                p2 = polyk;
            const long double p = ((x - rCoeffs_[k].alpha)*polyk -
                                   rCoeffs_[k].sqrbeta*polykm1)/rCoeffs_[k+1].sqrbeta;
            polykm1 = polyk;
            polyk = p;
        }
        if (deg1 == degmax)
            p1 = polyk;
        if (deg2 == degmax)
            p2 = polyk;
        return std::pair<long double,long double>(p1, p2);
    }

    long double ContOrthoPoly1D::normpolyprod(const unsigned* degrees,
                                              const unsigned nDeg,
                                              const unsigned degmax,
                                              const long double x) const
    {
        long double prod = 1.0L, polyk = 1.0L, polykm1 = 0.0L;
        for (unsigned k=0; k<degmax; ++k)
        {
            for (unsigned i=0; i<nDeg; ++i)
                if (k == degrees[i])
                    prod *= polyk;
            const long double p = ((x - rCoeffs_[k].alpha)*polyk -
                                   rCoeffs_[k].sqrbeta*polykm1)/rCoeffs_[k+1].sqrbeta;
            polykm1 = polyk;
            polyk = p;
        }
        for (unsigned i=0; i<nDeg; ++i)
            if (degmax == degrees[i])
                prod *= polyk;
        return prod;
    }

    long double ContOrthoPoly1D::normseries(const double *coeffs, const unsigned maxdeg,
                                            const long double x) const
    {
        long double sum = coeffs[0];
        long double polyk = 1.0L, polykm1 = 0.0L;
        for (unsigned k=0; k<maxdeg; ++k)
        {
            const long double p = ((x - rCoeffs_[k].alpha)*polyk -
                                   rCoeffs_[k].sqrbeta*polykm1)/rCoeffs_[k+1].sqrbeta;
            sum += p*coeffs[k+1];
            polykm1 = polyk;
            polyk = p;
        }
        return sum;
    }

    std::pair<long double,long double>
    ContOrthoPoly1D::monicInnerProducts(const unsigned degree) const
    {
        if (degree)
        {
            long double sum = 0.0L, xsum = 0.0L;
            const unsigned long mSize = measure_.size();
            for (unsigned long i = 0; i < mSize; ++i)
            {
                const long double x = measure_[i].first;
                const long double p = monicpoly(degree, x);
                const long double pprod = p*p*measure_[i].second;
                sum += pprod;
                xsum += x*pprod;
            }
            return std::pair<long double,long double>(xsum/wsum_, sum/wsum_);
        }
        else
            // Average x should be 0 for degree == 0
            return std::pair<long double,long double>(0.0L, 1.0L);
    }

    void ContOrthoPoly1D::weightCoeffs(double *coeffs,
                                       const unsigned maxdeg) const
    {
        if (maxdeg > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::weightsSquaredCoeffs: "
            "maximum degree out of range");
        assert(coeffs);

        std::vector<long double> scalarProducts(maxdeg+1U, 0.0L);

        const unsigned long mSize = measure_.size();
        for (unsigned long i = 0; i < mSize; ++i)
        {
            const double x = measure_[i].first;
            const double w = measure_[i].second;
            const long double f = w;

            long double polyk = 1.0L, polykm1 = 0.0L;
            for (unsigned k=0; k<maxdeg; ++k)
            {
                scalarProducts[k] += polyk*f*w;
                const long double poly = ((x - rCoeffs_[k].alpha)*polyk -
                                          rCoeffs_[k].sqrbeta*polykm1)/rCoeffs_[k+1].sqrbeta;
                polykm1 = polyk;
                polyk = poly;
            }
            scalarProducts[maxdeg] += polyk*f*w;
        }

        for (unsigned deg=0; deg<=maxdeg; ++deg)
            coeffs[deg] = scalarProducts[deg]/wsum_;
    }

    void ContOrthoPoly1D::calcRecurrenceStieltjes()
    {
        long double prevSprod = 1.0L;
        for (unsigned deg=0; deg<=maxdeg_; ++deg)
        {
            const std::pair<long double,long double> ip = monicInnerProducts(deg);
            rCoeffs_.push_back(Recurrence(ip.first/ip.second, ip.second/prevSprod));
            prevSprod = ip.second;
        }
    }

    void ContOrthoPoly1D::calcRecurrenceLanczos()
    {
        typedef long double Real;

        const unsigned long mSize = measure_.size();
        std::vector<Real> dmem(mSize*2UL);

        Real* p0 = &dmem[0];
        for (unsigned long i=0; i<mSize; ++i)
            p0[i] = measure_[i].first;

        Real* p1 = &dmem[mSize];
        clearBuffer(p1, mSize);
        p1[0] = measure_[0].second/wsum_;

        const unsigned long mm1 = mSize - 1;
        for (unsigned long n=0; n<mm1; ++n)
        {
            const unsigned long np1 = n + 1UL;
            Real xlam = measure_[np1].first;
            Real pn = measure_[np1].second/wsum_;
            Real gam = 1.0, sig = 0.0, t = 0.0;

            for (unsigned long k=0; k<=np1; ++k)
            {
                const Real rho = p1[k] + pn;
                const Real tmp = gam*rho;
                Real tsig = sig;
                if (rho <= 0.0)
                {
                    gam = 1.0;
                    sig = 0.0;
                }
                else
                {
                    gam = p1[k]/rho;
                    sig = pn/rho;
                }
                const Real tk = sig*(p0[k] - xlam) - gam*t;
                p0[k] -= tk - t;
                t = tk;
                if (sig < 0.0)
                    pn = tsig*p1[k];
                else
                    pn = t*t/sig;
                tsig = sig;
                p1[k] = tmp;
            }
        }

        rCoeffs_.push_back(Recurrence(0.0, 1.0));
        for (unsigned deg=1; deg<=maxdeg_; ++deg)
            rCoeffs_.push_back(Recurrence(p0[deg], p1[deg]));
    }

    double ContOrthoPoly1D::poly(const unsigned deg, const double x) const
    {
        if (deg > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::poly: degree argument is out of range");
        return normpoly(deg, x - meanX_);
    }

    std::pair<double,double> ContOrthoPoly1D::polyPair(
        const unsigned deg1, const unsigned deg2, const double x) const
    {
        if (deg1 > maxdeg_ || deg2 > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::polyPair: degree argument is out of range");
        const std::pair<long double,long double>& ld =
            twonormpoly(deg1, deg2, x - meanX_);
        return std::pair<double,double>(ld.first, ld.second);
    }

    double ContOrthoPoly1D::series(const double *coeffs, const unsigned deg,
                                   const double x) const
    {
        assert(coeffs);
        if (deg > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::series: degree argument is out of range");
        return normseries(coeffs, deg, x - meanX_);
    }

    void ContOrthoPoly1D::allPolys(const double x, const unsigned deg,
                                   double *polyValues) const
    {
        getAllPolys(x - meanX_, deg, polyValues);
    }

    void ContOrthoPoly1D::allPolys(const double x, const unsigned deg,
                                   long double *polyValues) const
    {
        getAllPolys(x - meanX_, deg, polyValues);
    }

    double ContOrthoPoly1D::empiricalKroneckerDelta(
        const unsigned ideg1, const unsigned ideg2) const
    {
        if (ideg1 > maxdeg_ || ideg2 > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::empiricalKroneckerDelta: "
            "degree argument is out of range");
        long double sum = 0.0L;
        const unsigned long mSize = measure_.size();
        for (unsigned long i = 0; i < mSize; ++i)
        {
            const std::pair<long double, long double>& p =
                twonormpoly(ideg1, ideg2, measure_[i].first);
            sum += measure_[i].second*p.first*p.second;
        }
        return sum/wsum_;
    }

    double ContOrthoPoly1D::jointPowerAverage(
        const unsigned ideg1, const unsigned power1,
        const unsigned ideg2, const unsigned power2) const
    {
        // Process various simple special cases first
        if (!power1)
            return powerAverage(ideg2, power2);
        if (!power2)
            return powerAverage(ideg1, power1);
        if (power1 == 1U && power2 == 1U)
            return kdelta(ideg1, ideg2);

        // General calculation
        if (ideg1 > maxdeg_ || ideg2 > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::jointPowerAverage: "
            "degree argument is out of range");
        long double sum = 0.0L;
        const unsigned long mSize = measure_.size();
        for (unsigned long i = 0; i < mSize; ++i)
        {
            const std::pair<long double, long double>& p =
                twonormpoly(ideg1, ideg2, measure_[i].first);
            sum += measure_[i].second*powl(p.first,power1)*powl(p.second,power2);
        }
        return sum/wsum_;
    }

    double ContOrthoPoly1D::jointAverage(
        const unsigned* degrees, const unsigned nDegrees,
        const bool sorted) const
    {
        if (nDegrees)
        {
            assert(degrees);

            // See if we can avoid a direct calculation
            if (nDegrees == 1U)
                return kdelta(degrees[0], 0U);
            if (nDegrees == 2U)
                return kdelta(degrees[0], degrees[1]);

            // Check if we can remove leading zeros
            {
                unsigned nonZeroDegs = nDegrees;
                while (nonZeroDegs && !degrees[0])
                {
                    ++degrees;
                    --nonZeroDegs;
                }
                if (nonZeroDegs < nDegrees)
                    return jointAverage(degrees, nonZeroDegs, sorted);
            }

            // Check if we can remove any other zeros
            if (!sorted)
            {
                bool allNonZero = true;
                for (unsigned ideg=0; ideg<nDegrees && allNonZero; ++ideg)
                    allNonZero = degrees[ideg];
                if (!allNonZero)
                {
                    unsigned degBuf[256];
                    if (nDegrees - 1U <= sizeof(degBuf)/sizeof(degBuf[0]))
                    {
                        unsigned nonZeroDegs = 0;
                        for (unsigned ideg=0; ideg<nDegrees; ++ideg)
                            if (degrees[ideg])
                                degBuf[nonZeroDegs++] = degrees[ideg];
                        return jointAverage(degBuf, nonZeroDegs, sorted);
                    }
                }
            }

            unsigned degmax;
            if (sorted)
                degmax = degrees[nDegrees-1U];
            else
                degmax = *std::max_element(degrees, degrees+nDegrees);
            if (degmax > maxdeg_) throw std::invalid_argument(
                "In npstat::ContOrthoPoly1D::jointAverage: "
                "degree argument is out of range");

            long double sum = 0.0L;
            const unsigned long mSize = measure_.size();
            for (unsigned long i = 0; i < mSize; ++i)
                sum += measure_[i].second*normpolyprod(
                    degrees, nDegrees, degmax, measure_[i].first);
            return sum/wsum_;
        }
        else
            return 1.0;
    }

    double ContOrthoPoly1D::empiricalKroneckerCovariance(
        const unsigned deg1, const unsigned deg2,
        const unsigned deg3, const unsigned deg4) const
    {
        double cov = 0.0;
        if (!((deg1 == 0 && deg2 == 0) || (deg3 == 0 && deg4 == 0)))
        {
            unsigned degs[4];
            degs[0] = deg1;
            degs[1] = deg2;
            degs[2] = deg3;
            degs[3] = deg4;
            cov = (jointAverage(degs, 4) - kdelta(deg1, deg2)*kdelta(deg3, deg4))/
                effectiveSampleSize();
            if (std::min(deg1, deg2) == std::min(deg3, deg4) &&
                std::max(deg1, deg2) == std::max(deg3, deg4))
                if (cov < 0.0)
                    cov = 0.0;
        }
        return cov;
    }

    std::pair<double,double>
    ContOrthoPoly1D::recurrenceCoeffs(const unsigned deg) const
    {
        if (deg > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::recurrenceCoeffs: "
            "degree argument is out of range");
        const Recurrence& rc(rCoeffs_[deg]);
        return std::pair<double,double>(rc.alpha, rc.beta);
    }

    Matrix<double> ContOrthoPoly1D::jacobiMatrix(const unsigned n) const
    {
        if (!n) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::jacobiMatrix: "
            "can not build matrix of zero size");
        if (n > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::jacobiMatrix: "
            "matrix size is out of range");
        Matrix<double> mat(n, n, 0);
        unsigned ip1 = 1;
        for (unsigned i=0; i<n; ++i, ++ip1)
        {
            mat[i][i] = rCoeffs_[i].alpha;
            if (i)
                mat[i][i-1] = rCoeffs_[i].sqrbeta;
            if (ip1 < n)
                mat[i][ip1] = rCoeffs_[ip1].sqrbeta;
        }
        return mat;
    }

    void ContOrthoPoly1D::calculateRoots(
        double *roots, const unsigned deg) const
    {
        if (deg)
        {
            assert(roots);
            const Matrix<double>& mat = jacobiMatrix(deg);
            if (deg == 1U)
                roots[0] = mat[0][0];
            else
                mat.tdSymEigen(roots, deg);
            for (unsigned i=0; i<deg; ++i)
                roots[i] += meanX_;
        }
    }

    double ContOrthoPoly1D::integratePoly(
        const unsigned deg1, const unsigned power,
        const double xmin, const double xmax) const
    {
        if (!deg1)
            return xmax - xmin;
        if (deg1 > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::integratePoly: "
            "degree argument is out of range");
        const unsigned nGood = GaussLegendreQuadrature::minimalExactRule(deg1*power);
        if (!nGood) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::integratePoly: "
            "product of poly degree and power is too large");
        GaussLegendreQuadrature glq(nGood);
        PolyFcn fcn(*this, deg1, power);
        return glq.integrate(fcn, xmin - meanX_, xmax - meanX_);
    }

    double ContOrthoPoly1D::integrateTripleProduct(
        const unsigned deg1, const unsigned deg2,
        const unsigned deg3, const double xmin,
        const double xmax) const
    {
        const unsigned maxdex = std::max(std::max(deg1, deg2), deg3);
        if (maxdex > maxdeg_) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::integrateTripleProduct: "
            "degree argument is out of range");
        const unsigned sumdeg = deg1 + deg2 + deg3;
        const unsigned nGood = GaussLegendreQuadrature::minimalExactRule(sumdeg);
        if (!nGood) throw std::invalid_argument(
            "In npstat::ContOrthoPoly1D::integrateTripleProduct: "
            "sum of the polynomial degrees is too large");
        GaussLegendreQuadrature glq(nGood);
        TripleProdFcn fcn(*this, deg1, deg2, deg3);
        return glq.integrate(fcn, xmin - meanX_, xmax - meanX_);
    }

    double ContOrthoPoly1D::cachedJointAverage(const unsigned deg1,
                                               const unsigned deg2,
                                               const unsigned deg3,
                                               const unsigned deg4) const
    {
        MemoKey key(deg1, deg2, deg3, deg4);
        return getCachedAverage(key);
    }

    double ContOrthoPoly1D::cachedJointAverage(const unsigned deg1,
                                               const unsigned deg2,
                                               const unsigned deg3,
                                               const unsigned deg4,
                                               const unsigned deg5,
                                               const unsigned deg6) const
    {
        MemoKey key(deg1, deg2, deg3, deg4, deg5, deg6);
        return getCachedAverage(key);
    }

    double ContOrthoPoly1D::cachedJointAverage(const unsigned deg1,
                                               const unsigned deg2,
                                               const unsigned deg3,
                                               const unsigned deg4,
                                               const unsigned deg5,
                                               const unsigned deg6,
                                               const unsigned deg7,
                                               const unsigned deg8) const
    {
        MemoKey key(deg1, deg2, deg3, deg4, deg5, deg6, deg7, deg8);
        return getCachedAverage(key);
    }

    double ContOrthoPoly1D::getCachedAverage(const MemoKey& key) const
    {
        double value;
        std::map<MemoKey,double>::const_iterator it = cachedAverages_.find(key);
        if (it == cachedAverages_.end())
        {
            value = jointAverage(key.degrees(), key.nDegrees(), true);
            cachedAverages_.insert(std::pair<MemoKey,double>(key, value));
        }
        else
            value = it->second;
        return value;
    }

    double ContOrthoPoly1D::cov4(const unsigned deg1, const unsigned deg2,
                                 const unsigned deg3, const unsigned deg4) const
    {
        const bool has0 = (deg1 == 0 && deg2 == 0) || (deg3 == 0 && deg4 == 0);
        double cov = 0.0;
        if (!has0)
        {
            const double N = effectiveSampleSize();
            cov = (cachedJointAverage(deg1, deg2, deg3, deg4) -
                   kdelta(deg1, deg2)*kdelta(deg3, deg4))/N;
            if (std::min(deg1, deg2) == std::min(deg3, deg4) &&
                std::max(deg1, deg2) == std::max(deg3, deg4))
                if (cov < 0.0)
                    cov = 0.0;
        }
        return cov;
    }

    double ContOrthoPoly1D::cov6(const unsigned a, const unsigned b,
                                 const unsigned c, const unsigned d,
                                 const unsigned e, const unsigned f) const
    {
        const bool has0 = (a == 0 && b == 0) || (c == 0 && d == 0) || (e == 0 && f == 0);
        double cov = 0.0;
        if (!has0)
        {
            double sum = cachedJointAverage(a, b, c, d, e, f);
            double add = 2.0;

            if (kdelta(a, b))
                sum -= cachedJointAverage(c, d, e, f);
            else
                add = 0.0;
            if (kdelta(c, d))
                sum -= cachedJointAverage(a, b, e, f);
            else
                add = 0.0;
            if (kdelta(e, f))
                sum -= cachedJointAverage(a, b, c, d);
            else
                add = 0.0;

            const double N = effectiveSampleSize();
            cov = (sum + add)/N/N;
        }
        return cov;
    }

    double ContOrthoPoly1D::slowCov8(const unsigned a, const unsigned b,
                                     const unsigned c, const unsigned d,
                                     const unsigned e, const unsigned f,
                                     const unsigned g, const unsigned h) const
    {
        const bool has0 = (a == 0 && b == 0) || (c == 0 && d == 0) ||
                          (e == 0 && f == 0) || (g == 0 && h == 0);
        double cov = 0.0;
        if (!has0)
        {
            const double pabcd = cachedJointAverage(a, b, c, d);
            const double pefgh = cachedJointAverage(e, f, g, h);
            const double pabef = cachedJointAverage(a, b, e, f);
            const double pcdgh = cachedJointAverage(c, d, g, h);
            const double pabgh = cachedJointAverage(a, b, g, h);
            const double pcdef = cachedJointAverage(c, d, e, f);
            const double pabcdef = cachedJointAverage(a, b, c, d, e, f);
            const double pabcdgh = cachedJointAverage(a, b, c, d, g, h);
            const double pabefgh = cachedJointAverage(a, b, e, f, g, h);
            const double pcdefgh = cachedJointAverage(c, d, e, f, g, h);
            const double pabcdefgh = cachedJointAverage(a, b, c, d, e, f, g, h);

            const double deltaprod = kdelta(a, b)*kdelta(c, d)*kdelta(e, f)*kdelta(g, h);

            const double tmp = kdelta(e,f)*kdelta(g,h)*pabcd +
                               kdelta(c,d)*kdelta(g,h)*pabef +
                               kdelta(c,d)*kdelta(e,f)*pabgh +
                               kdelta(a,b)*kdelta(c,d)*pefgh +
                               kdelta(a,b)*kdelta(e,f)*pcdgh +
                               kdelta(a,b)*kdelta(g,h)*pcdef;

            const double term1 = pabcd*pefgh + pabef*pcdgh +
                pabgh*pcdef + 3.0*deltaprod - tmp;

            const double term2 = pabcdefgh - 6.0*deltaprod -
                kdelta(a,b)*pcdefgh - kdelta(c,d)*pabefgh -
                kdelta(e,f)*pabcdgh - kdelta(g,h)*pabcdef -
                pabcd*pefgh - pabef*pcdgh - pabgh*pcdef + 2.0*tmp;

            const double nPoints = effectiveSampleSize();
            const double prod8 = (term1 + term2/nPoints)/nPoints/nPoints;
            cov = prod8 - cov4(a, b, c, d)*cov4(e, f, g, h);
        }
        return cov;
    }

    double ContOrthoPoly1D::covCov4(const unsigned a, const unsigned b,
                                    const unsigned c, const unsigned d,
                                    const unsigned e, const unsigned f,
                                    const unsigned g, const unsigned h) const
    {
        const bool has0 = (a == 0 && b == 0) || (c == 0 && d == 0) ||
                          (e == 0 && f == 0) || (g == 0 && h == 0);
        double cov = 0.0;
        if (!has0)
        {
            const double pabcdefgh = cachedJointAverage(a, b, c, d, e, f, g, h);
            const double pabcd = cachedJointAverage(a, b, c, d);
            const double pefgh = cachedJointAverage(e, f, g, h);
            const double N = effectiveSampleSize();
            cov = (pabcdefgh - pabcd*pefgh)/N/N/N;
        }
        return cov;
    }

    double ContOrthoPoly1D::cov8(const unsigned a, const unsigned b,
                                 const unsigned c, const unsigned d,
                                 const unsigned e, const unsigned f,
                                 const unsigned g, const unsigned h) const
    {
        const bool has0 = (a == 0 && b == 0) || (c == 0 && d == 0) ||
                          (e == 0 && f == 0) || (g == 0 && h == 0);
        double cov = 0.0;
        if (!has0)
        {
            const bool includeCubicPart = true;

            // First, calculate the O(N^-2) part
            const double pabef = cachedJointAverage(a, b, e, f);
            const double pcdgh = cachedJointAverage(c, d, g, h);
            const double pabgh = cachedJointAverage(a, b, g, h);
            const double pcdef = cachedJointAverage(c, d, e, f);

            const double deltaprod = kdelta(a, b)*kdelta(c, d)*kdelta(e, f)*kdelta(g, h);
            const double tmp2 = kdelta(c, d)*kdelta(g, h)*pabef +
                                kdelta(c, d)*kdelta(e, f)*pabgh +
                                kdelta(a, b)*kdelta(e, f)*pcdgh +
                                kdelta(a, b)*kdelta(g, h)*pcdef;

            const double term2 = pabef*pcdgh + pabgh*pcdef + 2.0*deltaprod - tmp2;
            double term3 = 0.0;

            if (includeCubicPart)
            {
                // Additional terms needed to calculate the O(N^-3) part
                const double pabcdefgh = cachedJointAverage(a, b, c, d, e, f, g, h);

                double sixsum = 0.0;
                if (kdelta(a, b))
                    sixsum += cachedJointAverage(c, d, e, f, g, h);
                if (kdelta(c, d))
                    sixsum += cachedJointAverage(a, b, e, f, g, h);
                if (kdelta(e, f))
                    sixsum += cachedJointAverage(a, b, c, d, g, h);
                if (kdelta(g, h))
                    sixsum += cachedJointAverage(a, b, c, d, e, f);

                const double pabcd = cachedJointAverage(a, b, c, d);
                const double pefgh = cachedJointAverage(e, f, g, h);
                const double tmp3 = tmp2 + kdelta(e, f)*kdelta(g, h)*pabcd +
                                           kdelta(a, b)*kdelta(c, d)*pefgh;

                term3 = pabcdefgh - 6.0*deltaprod - sixsum  -
                    (pabcd*pefgh + pabef*pcdgh + pabgh*pcdef) + 2.0*tmp3;
            }

            const double N = effectiveSampleSize();
            cov = (term2 + term3/N)/N/N;

            // const bool isVariance = ?;
            // if (isVariance)
            //     if (cov < 0.0)
            //         cov = 0.0;
        }
        return cov;
    }

    double ContOrthoPoly1D::epsExpectation(const unsigned m_in,
                                           const unsigned n_in,
                                           const bool highOrder) const
    {
        if (highOrder)
        {
            long double sum = 0.0L;
            if (m_in || n_in)
            {
                const unsigned m = std::min(m_in, n_in);
                const unsigned n = std::max(m_in, n_in);

                for (unsigned k=0; k<=n; ++k)
                {
                    const double f = k == n ? 1.0 : (k > m ? 1.0 : 2.0);
                    sum += f*cachedJointAverage(k, k, m, n);
                }
                if (m == n)
                    sum -= 1.0;
                else
                    sum -= (cachedJointAverage(m, m, m, n) +
                            cachedJointAverage(m, n, n, n))/2.0;
            }
            return sum/effectiveSampleSize();
        }
        else
            return 0.0;
    }

    double ContOrthoPoly1D::epsCovariance(const unsigned m1_in,
                                          const unsigned n1_in,
                                          const unsigned m2_in,
                                          const unsigned n2_in,
                                          const bool highOrder) const
    {
        const bool has0 = (m1_in == 0 && n1_in == 0) ||
                          (m2_in == 0 && n2_in == 0);
        if (has0)
            return 0.0;

        if (highOrder)
        {
            const unsigned m1 = std::min(m1_in, n1_in);
            const unsigned n1 = std::max(m1_in, n1_in);
            const unsigned m2 = std::min(m2_in, n2_in);
            const unsigned n2 = std::max(m2_in, n2_in);

            long double sum = 0.0L;

            // Process the -v_{m1,n1} term (i.e., the linear one) of eps_{m1,n1}
            sum += cov4(m2, n2, m1, n1);
            sum += cov6(m2, n2, n2, n2, m1, n1)/2.0;
            sum += cov6(m2, n2, m2, m2, m1, n1)/2.0;
            for (unsigned k=0; k<=n2; ++k)
            {
                const double factor = k > m2 ? 1.0 : 2.0;
                sum -= factor*cov6(k, m2, k, n2, m1, n1);
            }

            // Process the term -v_{m1,n1}/2 (v_{n1,n1} + v_{m1,m1}) of eps_{m1,n1}
            unsigned idx[2];
            idx[0] = n1;
            idx[1] = m1;
            for (unsigned ii=0; ii<2; ++ii)
            {
                const unsigned mOrN = idx[ii];
                sum += cov6(m1, n1, mOrN, mOrN, m2, n2)/2.0;
                sum += cov8(m1, n1, mOrN, mOrN, m2, n2, n2, n2)/4.0;
                sum += cov8(m1, n1, mOrN, mOrN, m2, n2, m2, m2)/4.0;
                for (unsigned k=0; k<=n2; ++k)
                {
                    const double factor = k > m2 ? 1.0 : 2.0;
                    sum -= factor/2.0*cov8(m1, n1, mOrN, mOrN, k, m2, k, n2);
                }
            }

            // Process the sum in eps_{m1,n1}
            for (unsigned k1=0; k1<=n1; ++k1)
            {
                const double f1 = k1 > m1 ? 1.0 : 2.0;
                sum -= f1*cov6(k1, m1, k1, n1, m2, n2);
                sum -= f1*cov8(k1, m1, k1, n1, m2, n2, n2, n2)/2.0;
                sum -= f1*cov8(k1, m1, k1, n1, m2, n2, m2, m2)/2.0;
                for (unsigned k=0; k<=n2; ++k)
                {
                    const double factor = k > m2 ? 1.0 : 2.0;
                    sum += f1*factor*cov8(k1, m1, k1, n1, k, m2, k, n2);
                }
            }

            return sum;
        }
        else
            return cov4(m2_in, n2_in, m1_in, n1_in);
    }

    CPP11_auto_ptr<StorablePolySeries1D> ContOrthoPoly1D::makeStorablePolySeries(
        const double i_xmin, const double i_xmax,
        const double *coeffs, const unsigned maxdeg) const
    {
        const unsigned sz = rCoeffs_.size();
        std::vector<std::pair<long double,long double> > rc(sz);
        for (unsigned i=0; i<sz; ++i)
        {
            const Recurrence& r(rCoeffs_[i]);
            rc[i].first = r.alpha;
            rc[i].second = r.sqrbeta;
        }
        return CPP11_auto_ptr<StorablePolySeries1D>(
            new StorablePolySeries1D(rc, i_xmin, i_xmax, meanX_, coeffs, maxdeg));
    }
}
