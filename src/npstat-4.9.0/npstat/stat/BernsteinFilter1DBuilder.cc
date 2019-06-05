#include <stdexcept>
#include <cassert>

#include "npstat/nm/Interval.hh"
#include "npstat/nm/OrthoPoly1D.hh"

#include "npstat/stat/BernsteinFilter1DBuilder.hh"

static inline double risingTriangleOverlap(const double xmin,
                                           const double xmax,
                                           const npstat::Interval<double>& bin)
{
    const npstat::Interval<double> triangleBase(xmin, xmax);
    const npstat::Interval<double>& overlap = triangleBase.overlap(bin);
    return overlap.length()*(overlap.midpoint() - xmin);
}

static inline double fallingTriangleOverlap(const double xmin,
                                            const double xmax,
                                            const npstat::Interval<double>& bin)
{
    const npstat::Interval<double> triangleBase(xmin, xmax);
    const npstat::Interval<double>& overlap = triangleBase.overlap(bin);
    return overlap.length()*(xmax - overlap.midpoint());
}

namespace npstat {
    BernsteinFilter1DBuilder::BernsteinFilter1DBuilder(
        const unsigned polyDegree, const unsigned dataLen,
        const bool useClosestPoly)
        : polySet_(polyDegree, dataLen),
          polyDegree_(polyDegree),
          dataLen_(dataLen),
          useClosestPoly_(useClosestPoly)
    {
        if (polyDegree >= dataLen_) throw std::invalid_argument(
            "In npstat::BernsteinFilter1DBuilder constructor: "
            "polynomial degree is too large, either decrease it or "
            "increase the number of bins");
    }

    void BernsteinFilter1DBuilder::fillClosestPoly(const unsigned binnum,
                                                   long double* f) const
    {
        const double w = (polyDegree_ + 1.0)/dataLen_;
        const unsigned nPolys = polyDegree_ + 1U;
        const double polyStep = dataLen_*1.0/nPolys;
        const unsigned ileft = static_cast<unsigned>(binnum*1.0/polyStep);
        unsigned iright = static_cast<unsigned>((binnum+1.0)/polyStep);
        if (iright > polyDegree_)
            iright = polyDegree_;
        if (iright > ileft + 1U)
            iright = ileft + 1U;

        if (ileft == iright)
        {
            for (unsigned i=0; i<dataLen_; ++i)
                f[i] = w*polySet_.poly(ileft, i);
        }
        else
        {
            // The bin spans polynomial number ileft and iright
            const double degEdge = iright*polyStep;
            const double rightFrac = (binnum + 1.0 - degEdge)/polyStep;
            const double leftFrac = (degEdge - binnum)/polyStep;
            assert(rightFrac >= 0.0);
            assert(leftFrac >= 0.0);
            const double leftWeight = leftFrac/(leftFrac + rightFrac)*w;
            const double rightWeight = rightFrac/(leftFrac + rightFrac)*w;
            if (leftWeight > 0.0)
                for (unsigned i=0; i<dataLen_; ++i)
                    f[i] += leftWeight*polySet_.poly(ileft, i);
            if (rightWeight > 0.0)
                for (unsigned i=0; i<dataLen_; ++i)
                    f[i] += rightWeight*polySet_.poly(iright, i);
        }
    }

    void BernsteinFilter1DBuilder::fillWeighted(const unsigned binnum,
                                                long double* f) const
    {
        const int udeg = polyDegree_;
        const int nPolys = udeg + 1;
        const double polyStep = dataLen_*1.0/nPolys;
        const int centerPoly = static_cast<int>((binnum+0.5)/polyStep);
        for (int ipoly=centerPoly-1; ipoly<=centerPoly+1; ++ipoly)
        {
            if (ipoly<0 || ipoly>udeg)
                continue;

            // Determine the "overlap" of the weight triangle corresponding
            // to this poly with the given bin. First, translate bin edges
            // into the poly number units.
            const Interval<double> bin(binnum/polyStep, (binnum+1U)/polyStep);

            // Integrate the weight shape over the bin
            double integ = 0.0;
            if (ipoly == 0)
            {
                integ += Interval<double>(0.0, 0.5).overlapLength(bin);
                integ += fallingTriangleOverlap(0.5, 1.5, bin);
            }
            else if (ipoly == udeg)
            {
                integ += risingTriangleOverlap(nPolys-1.5, nPolys-0.5, bin);
                integ += Interval<double>(nPolys-0.5, nPolys).overlapLength(bin);
            }
            else
            {
                integ += risingTriangleOverlap(ipoly-0.5, ipoly+0.5, bin);
                integ += fallingTriangleOverlap(ipoly+0.5, ipoly+1.5, bin);
            }
            if (integ > 0.0)
            {
                const double w = (udeg+1.0)/dataLen_*integ*polyStep;
                for (unsigned i=0; i<dataLen_; ++i)
                    f[i] += w*polySet_.poly(ipoly, i);
            }
        }
    }

    PolyFilter1D* BernsteinFilter1DBuilder::makeFilter(
        const double* /* taper */, unsigned /* lenTaper */,
        const unsigned binnum, const unsigned datalen) const
    {
        if (datalen != dataLen_) throw std::invalid_argument(
            "In npstat::BernsteinFilter1DBuilder::makeFilter: "
            "inconsistent data length");
        if (binnum >= dataLen_) throw std::out_of_range(
            "In npstat::BernsteinFilter1DBuilder::makeFilter: "
            "bin number out of range");

        PolyFilter1D* filter = new PolyFilter1D(binnum);
        filter->resize(datalen);
        long double* f = &(*filter)[0];
        for (unsigned i=0; i<datalen; ++i)
            f[i] = 0.0L;

        if (polyDegree_)
        {
            if (useClosestPoly_)
                fillClosestPoly(binnum, f);
            else
                fillWeighted(binnum, f);
        }
        else
        {
            const long double w = 1.0L/datalen;
            for (unsigned i=0; i<datalen; ++i)
                f[i] = w;
        }

        return filter;
    }
}
