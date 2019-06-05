#include <cmath>
#include <cfloat>
#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "npstat/nm/OrthoPoly1D.hh"
#include "npstat/nm/findRootInLogSpace.hh"

#include "npstat/rng/permutation.hh"

#include "npstat/stat/BoundaryMethod.hh"
#include "npstat/stat/Filter1DBuilders.hh"
#include "npstat/stat/continuousDegreeTaper.hh"

namespace npstat {
    namespace Private {
        class BandwidthFunc : public Functor1<double, double>
        {
        public:
            inline BandwidthFunc(const unsigned m, const double a,
                                 const double b)
                : a_(a), b_(b), m_(m) {}

            inline double operator()(const double& h) const
                {return pow(h, 2U*m_ + 3U)*(a_ + b_*h*h);}

        private:
            double a_;
            double b_;
            unsigned m_;
        };
    }
}

//
// Solve the equation a*x^{2 m + 3} + b*x^{2 m + 5} == c
//
static double solveBandwidthEquation(const unsigned m, const double a,
                                     const double b, const double c)
{
    assert(a >= 0.0);
    assert(b >= 0.0);
    assert(c > 0.0);

    const unsigned twomp5 = 2U*m + 5U;
    if (a == 0.0)
    {
        assert(b > 0.0);
        return pow(c/b, 1.0/twomp5);
    }

    const unsigned twomp3 = 2U*m + 3U;
    if (b == 0.0)
    {
        assert(a > 0.0);
        return pow(c/a, 1.0/twomp3);
    }

    npstat::Private::BandwidthFunc func(m, a, b);
    const double h0 = std::min(pow(c/b, 1.0/twomp5), pow(c/a, 1.0/twomp3));
    if (func(h0) <= c)
        return h0;
    else
    {
        const double tol = 10.0*DBL_EPSILON;
        double h;
        const bool status = findRootInLogSpace(func, c, h0/1.1, tol, &h, 0.1);
        assert(status);
        return h;
    }
}

namespace npstat {
    CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> getBoundaryFilter1DBuilder(
        const BoundaryHandling& bm,
        const AbsDistribution1D* distro, const double stepSize,
        const unsigned char* exclusionMask, const unsigned exclusionMaskLen,
        const bool excludeCentralPoint)
    {
        typedef CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> RPtr;

        const unsigned nParams = bm.nParameters();
        const double* methodParameters = bm.parameters();
        RPtr result;

        switch (bm.methodId())
        {
        case Private::BM_TRUNCATE:
            assert(!nParams);
            result = RPtr(new TruncatingFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_STRETCH:
            assert(!nParams);
            result = RPtr(new StretchingFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_FOLD:
            assert(!nParams);
            result = RPtr(new FoldingFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_CONSTSQ:
            assert(!nParams);
            result = RPtr(new ConstSqFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_FOLDSQ:
            assert(!nParams);
            result = RPtr(new FoldingSqFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_CONSTVAR:
            assert(!nParams);
            result = RPtr(new ConstVarFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_FOLDVAR:
            assert(!nParams);
            result = RPtr(new FoldingVarFilter1DBuilder(
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_CONSTBW:
            assert(nParams == 2U);
            result = RPtr(new ConstBwFilter1DBuilder(
                              methodParameters[0], methodParameters[1],
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        case Private::BM_FOLDBW:
            assert(nParams == 2U);
            result = RPtr(new FoldBwFilter1DBuilder(
                              methodParameters[0], methodParameters[1],
                              distro, stepSize, exclusionMask,
                              exclusionMaskLen, excludeCentralPoint));
        break;

        // Template for future cases
        // case BM_WHATEVER:
        //     assert(nParams == ?U);
        //     result = RPtr(new (
        //                       distro, stepSize, exclusionMask,
        //                       exclusionMaskLen, excludeCentralPoint));
        // break;

        default:
            assert(!"Missing switch case in npstat::getBoundaryFilter1DBuilder."
                   " This is a bug. Please report.");
        }
        return result;
    }

    double StretchingFilter1DBuilder::calculateCriterion(
        const AbsDistribution1D* distro, const double h,
        const int datalen, const int weightCenterPos,
        const double stepSize, double* /* workbuf */) const
    {
        long double sum = 0.0L;
        for (int i=weightCenterPos; i<datalen; ++i)
        {
            const double x = (i - weightCenterPos)*stepSize;
            const double dens = distro->density(x/h);
            if (dens <= 0.0)
                break;
            sum += dens;
        }
        for (int i=weightCenterPos-1; i>=0; --i)
        {
            const double x = (i - weightCenterPos)*stepSize;
            const double dens = distro->density(x/h);
            if (dens <= 0.0)
                break;
            sum += dens;
        }
        return sum*stepSize;
    }

    double ConstSqFilter1DBuilder::calculateCriterion(
        const AbsDistribution1D* distro, const double h,
        const int len, const int binnum,
        const double step, double* w) const
    {
        unsigned firstWeightUsed, sizeNeeded;
        scanTheDensity(distro, h, len, binnum, step, w,
                       &firstWeightUsed, &sizeNeeded);
        w += firstWeightUsed;
        long double fsum = 0.0L, sum = 0.0L;
        for (unsigned i=0; i<sizeNeeded; ++i)
        {
            fsum += w[i];
            sum += w[i]*w[i];
        }
        assert(fsum > 0.0L);
        const double fscale = 1.0/(fsum*step);
        return sum*step*fscale*fscale;
    }

    double ConstVarFilter1DBuilder::calculateCriterion(
        const AbsDistribution1D* distro, const double h,
        const int len, const int binnum,
        const double step, double* w) const
    {
        scanTheDensity(distro, h, len, binnum, step, w);
        long double fsum = 0.0L, sum = 0.0L;
        for (int i=0; i<len; ++i)
        {
            const double x = (i - binnum)*step;
            fsum += w[i];
            sum += x*x*w[i];
        }
        assert(fsum > 0.0L);
        return sum/fsum;
    }

    ConstBwFilter1DBuilder::ConstBwFilter1DBuilder(
        const double deg, const double sigma, const AbsDistribution1D* distro,
        const double centralStepSize, const unsigned char* exclusionMask,
        const unsigned exclusionMaskLen, const bool excludeCentralPoint)
        : AbsBoundaryFilter1DBuilder(distro, centralStepSize,
                                     exclusionMask, exclusionMaskLen,
                                     excludeCentralPoint),
          filterDeg_(deg),
          typicalWidth_(sigma)
    {
        continuousDegreeTaper(filterDeg_, &taperVec_);
        if (filterDeg_ != floor(filterDeg_)) throw std::invalid_argument(
            "In npstat::ConstBwFilter1DBuilder constructor: fractional "
            "filter degrees are not supported");
    }

    double ConstBwFilter1DBuilder::calculateCriterion(
        const AbsDistribution1D* distro, const double h,
        const int len, const int binnum,
        const double step, double* w) const
    {
        // To optimize this criterion, at this point we need to build
        // the complete filter with the given h
        const double* taper = &taperVec_[0];
        const unsigned maxDeg = taperVec_.size() - 1U;

        unsigned loind, sizeNeeded;
        scanTheDensity(distro, h, len, binnum, step, w,
                       &loind, &sizeNeeded);
        npstat::OrthoPoly1D poly(maxDeg, w+loind, sizeNeeded, step);

        assert(binnum >= 0);
        assert(loind <= static_cast<unsigned>(binnum));
        const int filterCenter = binnum - static_cast<int>(loind);
        PolyFilter1D f(filterCenter);
        const int filterLen = poly.length();
        assert(filterLen > 0 && static_cast<unsigned>(filterLen) == sizeNeeded);
        f.resize(sizeNeeded);
        poly.linearFilter(taper, maxDeg, filterCenter, &f[0], sizeNeeded);

        // Calculate the criterion. Note that the filter is normalized
        // in such a way that the sum of all f[i] values equals 1.0.
        const unsigned mp1 = maxDeg + 1U;
        long double sumsq = 0.0L, smp1p1 = 0.0L, smp2p2 = 0.0L;
        for (int i=0; i<filterLen; ++i)
        {
            const double x = (i - filterCenter)*step;
            const double xmp1 = pow(x, mp1);
            const double xmp2 = x*xmp1;
            const double weight = f[i];

            smp1p1 += xmp1*weight;
            smp2p2 += xmp2*weight;
            sumsq  += weight*weight;
        }
        smp1p1 *= typicalWidth_/ldfactorial(maxDeg);
        smp1p1 *= smp1p1;

        smp2p2 /= ldfactorial(mp1);
        smp2p2 *= smp2p2;

        return solveBandwidthEquation(maxDeg, smp1p1, smp2p2, sumsq);
    }
}
