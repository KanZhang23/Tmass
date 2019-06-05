#include <cmath>
#include <cassert>
#include <stdexcept>

#include "npstat/nm/MathUtils.hh"

#include "npstat/stat/GridRandomizer.hh"
#include "npstat/stat/ArrayProjectors.hh"
#include "npstat/stat/StatUtils.hh"

namespace npstat {
    void GridRandomizer::clearCdfs()
    {
        unsigned len = cdfs_.size();
        for (unsigned i=0; i<len; ++i)
            delete cdfs_[i];

        if (iDeg_)
            for (unsigned i=0; i<dim_; ++i)
                delete norms_[i];
    }

    void GridRandomizer::copyCdfs(const GridRandomizer& r)
    {
        unsigned len = cdfs_.size();
        for (unsigned i=0; i<len; ++i)
            cdfs_[i] = new ArrayND<double>(*r.cdfs_[i]);

        if (iDeg_)
        {
            for (unsigned i=0; i<dim_; ++i)
                norms_[i] = new ArrayND<double>(*r.norms_[i]);
            norms_[dim_] = &grid_;
        }
    }

    GridRandomizer::GridRandomizer(const GridRandomizer& r)
        : grid_(r.grid_),
          boundary_(r.boundary_),
          iDeg_(r.iDeg_),
          dim_(r.dim_),
          cdfs_(r.cdfs_),
          norms_(r.norms_),
          binwidth_(r.binwidth_),
          densityNorm_(r.densityNorm_),
          genBins_(r.genBins_),
          work_(r.work_),
          memoizeIn_(r.memoizeIn_),
          memoizeOut_(r.memoizeOut_)
    {
        copyCdfs(r);
    }

    bool GridRandomizer::operator==(const GridRandomizer& r) const
    {
        return iDeg_ == r.iDeg_ &&
               dim_ == r.dim_ &&
               boundary_ == r.boundary_ &&
               grid_ == r.grid_;
    }

    GridRandomizer& GridRandomizer::operator=(const GridRandomizer& r)
    {
        if (this != &r)
        {
            clearCdfs();

            grid_ = r.grid_;
            boundary_ = r.boundary_;
            iDeg_ = r.iDeg_;
            dim_ = r.dim_;
            cdfs_ = r.cdfs_;
            norms_ = r.norms_;
            binwidth_ = r.binwidth_;
            densityNorm_ = r.densityNorm_;
            genBins_ = r.genBins_;
            work_ = r.work_;
            memoizeIn_ = r.memoizeIn_;
            memoizeOut_ = r.memoizeOut_;

            copyCdfs(r);
        }
        return *this;
    }

    void GridRandomizer::initialize()
    {
        // Only interpolation degrees of 0 and 1 are supported
        if (iDeg_ > 1U) throw std::invalid_argument(
            "In npstat::GridRandomizer::initialize: "
            "unsupported interpolation degree");

        // Make sure that all other arguments are sound
        if (!grid_.isDensity()) throw std::invalid_argument(
            "In npstat::GridRandomizer::initialize: "
            "grid values can not be used as a probability density");
        if (!dim_) throw std::invalid_argument(
            "In npstat::GridRandomizer::initialize: "
            "can not use zero dimensionality grid");
        if (dim_ != boundary_.dim()) throw std::invalid_argument(
            "In npstat::GridRandomizer::initialize: "
            "incompatible bounding box dimensionality");
        if (boundary_.volume() <= 0.0) throw std::invalid_argument(
            "In npstat::GridRandomizer::initialize: "
            "bounding box must have positive volume");

        // Calculate the bin width
        binwidth_.reserve(dim_);
        for (unsigned i=0; i<dim_; ++i)
            binwidth_.push_back(boundary_[i].length()/grid_.span(i));

        // Allocate some workspace
        genBins_.reserve(dim_);
        for (unsigned i=0; i<dim_; ++i)
            genBins_.push_back(i);
        work_.resize(dim_);
        memoizeIn_.resize(dim_);
        memoizeOut_.resize(dim_);
        for (unsigned i=0; i<dim_; ++i)
            memoizeIn_[i] = -1.23456e231;

        // Get the shape. Subtract 1 from each index.
        ArrayShape sh(grid_.shape());
        for (unsigned i=0; i<dim_; ++i)
            sh[i] -= 1U;

        // Prepare storage for normalization factors
        if (iDeg_)
        {
            norms_.resize(dim_ + 1U);
            norms_[dim_] = &grid_;
        }

        // Calculate the conditional CDFs
        cdfs_.resize(dim_);
        ArrayND<double>* cdf = new ArrayND<double>(grid_);
        for (unsigned count = dim_; count > 0U; --count)
        {
            const unsigned i = count - 1;
            ArrayND<double>* slice = new ArrayND<double>(*cdf,&genBins_[i],1U);
            cdf->convertToLastDimCdf<long double>(slice, iDeg_);
            if (iDeg_)
                norms_[i] = new ArrayND<double>(*slice);
            cdfs_[i] = cdf;
            cdf = slice;
        }
        delete cdf;

        // Normalize the density
        const double sum = grid_.sum<long double>();
        densityNorm_ = grid_.length()/(sum*boundary_.volume());
    }

    double GridRandomizer::density(const double* x, const unsigned xLen) const
    {
        if (xLen != dim_) throw std::invalid_argument(
            "In npstat::GridRandomizer::density: "
            "incompatible input point dimensionality");
        assert(x);
        if (!boundary_.isInsideWithBounds(x, xLen))
            return 0.0;
        const double* bw = &binwidth_[0];
        double* y = &work_[0];
        for (unsigned i=0; i<dim_; ++i)
            y[i] = (x[i] - boundary_[i].min())/bw[i] - 0.5;
        double v = 0.0;
        switch (iDeg_)
        {
        case 0:
            v = grid_.closest(y, dim_);
            break;

        case 1:
            v = grid_.interpolate1(y, dim_);
            break;

        default:
            assert(0);
        }
        return v*densityNorm_;
    }

    void GridRandomizer::generate(const double* randNum,
                                  const unsigned bufLen,
                                  double* resultBuf) const
    {
        if (bufLen)
        {
            assert(randNum);
            assert(resultBuf);

            switch (iDeg_)
            {
            case 0U:
                generateFlat(randNum, bufLen, resultBuf);
                break;

            case 1U:
                generateInterpolated(randNum, bufLen, resultBuf);
                break;

            default:
                // This should never happen
                assert(0);
            }
        }
    }

    void GridRandomizer::generateFlat(const double* randNum,
                                      const unsigned bufLen,
                                      double* resultBuf) const
    {
        // Construct various useful pointers
        unsigned* bins = &genBins_[0];
        const double* bw = &binwidth_[0];
        const Interval<double>* box = &boundary_[0];

        // Go over dimensions and invert conditional CDFs numerically
        for (unsigned i=0; i<dim_ && i<bufLen; ++i)
        {
            bins[i] = 0U;
            const ArrayND<double>* cdf = cdfs_[i];
            const double* data = cdf->data() + 
                                 cdf->linearIndex(&bins[0], i + 1U);
            const unsigned span = cdf->span(i);
            const double rnd = randNum[i]*data[span - 1U];
            double rem;

            if (rnd <= 0.0)
                rem = 0.0;
            else if (rnd < data[0])
                rem = rnd/data[0];
            else if (rnd >= data[span - 1U])
            {
                bins[i] = span - 1;
                rem = 1.0;
            }
            else
            {
                bins[i] = quantileBinFromCdf(data, span, rnd, &rem) + 1U;
                assert(bins[i] < span);
            }

            // Smear the generated number uniformly over the bin
            const double maxval = box[i].max();
            double g = box[i].min() + bw[i]*(bins[i] + rem);
            if (g > maxval)
                g = maxval;
            resultBuf[i] = g;
        }
    }

    void GridRandomizer::generateInterpolated(const double* randNum,
                                              const unsigned bufLen,
                                              double* resultBuf) const
    {
        // Check whether we have just seen these arguments
        double* memIn = &memoizeIn_[0];
        double* memOut = &memoizeOut_[0];
        unsigned nSame = 0;
        for (; nSame<dim_ && nSame<bufLen; ++nSame)
        {
            if (randNum[nSame] == memIn[nSame])
                resultBuf[nSame] = memOut[nSame];
            else
                break;
        }

        // Go over dimensions and invert the conditional CDFs numerically
        for (unsigned i=nSame; i<dim_ && i<bufLen; ++i)
        {
            const ArrayND<double>* cdf = cdfs_[i];
            const unsigned span = cdf->span(i);

            // Renormalize the random number
            const double norm = norms_[i]->interpolate1(resultBuf, i);
            const double rnd = randNum[i]*norm;
            
            // g is the location in bin width units using
            // histogram-like origin
            double g;
            if (rnd <= 0.0)
                g = 0.0;
            else if (rnd >= norm)
                g = span;
            else
            {
                // Interpolate the cdf linearly from the known
                // precise values in the grid points. Note that
                // the very first dimension does not have to be
                // interpolated for those points.
                const double* cdfdata = i ? 0 : cdf->data();
                double data0;
                if (i)
                {
                    resultBuf[i] = 0.0;
                    data0 = cdf->interpolate1(resultBuf, i+1);
                }
                else
                    data0 = cdfdata[0];
                if (rnd <= data0)
                    g = 0.5*rnd/data0;
                else
                {
                    double dataLast;
                    if (i)
                    {
                        resultBuf[i] = span - 1.0;
                        dataLast = cdf->interpolate1(resultBuf, i+1);
                    }
                    else
                        dataLast = cdfdata[span - 1U];
                    if (rnd >= dataLast)
                        g = span-0.5+0.5*(rnd - dataLast)/(norm - dataLast);
                    else
                    {
                        // Find the right bin number by bisection
                        unsigned imin = 0, imax = span - 1U;
                        double cmin = data0, cmax = dataLast;
                        while (imax - imin > 1U)
                        {
                            const unsigned itry = (imax + imin)/2U;
                            double c;
                            if (i)
                            {
                                resultBuf[i] = itry;
                                c = cdf->interpolate1(resultBuf, i+1);
                            }
                            else
                                c = cdfdata[itry];
                            if (c > rnd)
                            {
                                imax = itry;
                                cmax = c;
                            }
                            else
                            {
                                imin = itry;
                                cmin = c;
                            }
                        }

                        // We have the bin number. Now, we need
                        // to figure out what is the slope of the
                        // density function from this bin to the next.
                        resultBuf[i] = imin;
                        const double f0 = norms_[i+1]->interpolate1(
                            resultBuf, i+1);
                        const double k = 2.0*(cmax - cmin - f0);

                        // Solve the quadratic equation to invert
                        // the cumulative density function
                        double x;
                        const double y = rnd - cmin;
                        if (std::abs(k) < 1.e-10*f0)
                            x = y/(cmax - cmin);
                        else
                        {
                            double x1, x2;
                            if (!solveQuadratic(2.0*f0/k, -2.0*y/k, &x1, &x2))
                                throw std::runtime_error(
                                    "In npstat::GridRandomizer::generateInterpolated:"
                                    " no solutions");
                            if (std::abs(x1 - 0.5) < std::abs(x2 - 0.5))
                                x = x1;
                            else
                                x = x2;
                            if (x < 0.0)
                                x = 0.0;
                            else if (x > 1.0)
                                x = 1.0;
                        }
                        g = x + imin + 0.5;
                    }
                }
            }
            resultBuf[i] = g - 0.5;

            // Remember input and output for potential quick lookup
            memIn[i] = randNum[i];
            memOut[i] = resultBuf[i];
        }

        // Convert the index into the box coordinate system
        const double* bw = &binwidth_[0];
        const Interval<double>* box = &boundary_[0];
        for (unsigned i=0; i<dim_ && i<bufLen; ++i)
        {
            const double minval = box[i].min();
            const double maxval = box[i].max();
            resultBuf[i] = minval + (resultBuf[i] + 0.5)*bw[i];
            if (resultBuf[i] < minval)
                resultBuf[i] = minval;
            else if (resultBuf[i] > maxval)
                resultBuf[i] = maxval;
        }
    }
}
