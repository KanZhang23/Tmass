#include <cmath>
#include <cassert>
#include <climits>

#include "geners/CPP11_auto_ptr.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/nm/OrthoPoly1D.hh"
#include "npstat/nm/findRootInLogSpace.hh"
#include "npstat/nm/allocators.hh"

#include "npstat/stat/AbsFilter1DBuilder.hh"

static inline double relative_delta(const double a, const double b)
{
    return fabs(a - b)/(fabs(a) + fabs(b) + 0.1);
}

static inline int mirrorIndex(const int i, const int period)
{
    int j = i % period;
    if (j < 0)
        j += period;
    int rat = i/period;
    if (i < 0 && i % period)
        --rat;
    if (rat % 2)
        return period - j - 1;
    else
        return j;
}

namespace npstat {
    PolyFilter1D* OrthoPolyFilter1DBuilder::makeFilter(
        const double* taper, const unsigned maxDegree,
        const unsigned binnum, const unsigned datalen) const
    {
        assert(taper);

        unsigned filterCenter;
        CPP11_auto_ptr<OrthoPoly1D> poly(this->makeOrthoPoly(
            maxDegree, binnum, datalen, &filterCenter));
        CPP11_auto_ptr<PolyFilter1D> f(new PolyFilter1D(filterCenter));
        const unsigned filterLen = poly->length();
        f->resize(filterLen);
        poly->linearFilter(taper, maxDegree, filterCenter, &(*f)[0], filterLen);
        return f.release();
    }

    bool PolyFilter1D::write(std::ostream& os) const
    {
        gs::write_pod(os, peak_);
        gs::write_pod_vector(os, static_cast<const std::vector<long double>&>(*this));
        return !os.fail();
    }

    PolyFilter1D* PolyFilter1D::read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(gs::ClassId::makeId<PolyFilter1D>());
        current.ensureSameId(id);

        unsigned peak = 0U;
        gs::read_pod(in, &peak);
        PolyFilter1D* f = 0;
        if (!in.fail())
        {
            f = new PolyFilter1D(peak);
            gs::read_pod_vector(in, static_cast<std::vector<long double>*>(f));
        }
        if (in.fail())
        {
            delete f; f = 0;
            throw gs::IOReadFailure("In npstat::PolyFilter1D::read: "
                                    "input stream failure");
        }
        return f;
    }

    AbsBoundaryFilter1DBuilder::AbsBoundaryFilter1DBuilder(
        const AbsDistribution1D* distro, const double step,
        const unsigned char* exclusionMask, const unsigned exclusionMaskLen,
        const bool excludeCentralPoint)
        : distro_(distro),
          step_(step),
          lastBandwidthFactor_(0.0),
          centralIntegral_(0.0),
          centralIntegralLen_(0U),
          excludeCentralPoint_(excludeCentralPoint)
    {
        if (step_ <= 0.0) throw std::invalid_argument(
            "In npstat::AbsBoundaryFilter1DBuilder constructor: "
            "invalid step size");
        assert(distro_);

        const double maxratio = floor(fabs(distro_->quantile(1.0))/step_);
        const double minratio = floor(fabs(distro_->quantile(0.0))/step_);
        const double topratio = maxratio > minratio ? maxratio : minratio;
        if (topratio >= static_cast<double>(INT_MAX))
            maxlen_ = 2*(static_cast<unsigned>(INT_MAX) - 1) + 1;
        else
            maxlen_ = 2*static_cast<unsigned>(topratio) + 1;

        // Fill the exclusion mask
        if (exclusionMask && exclusionMaskLen)
        {
            exclusionMask_.resize(exclusionMaskLen);
            for (unsigned i=0; i<exclusionMaskLen; ++i)
                exclusionMask_[i] = exclusionMask[i];
        }
    }

    void AbsBoundaryFilter1DBuilder::scanTheDensity(
        const AbsDistribution1D* distro, const double h,
        const int datalen, const int j0,
        const double stepSize, double* wbuf,
        unsigned* firstWeightUsed, unsigned* sizeNeeded) const
    {
        assert(wbuf);
        clearBuffer(wbuf, datalen);

        if (isFolding())
        {
            int loind = j0, hiind = j0;

            for (int jmin = j0 - 1; jmin > INT_MIN; --jmin)
            {
                const double x = (jmin - j0)*stepSize;
                const double dens = distro->density(x/h)/h;
                if (dens <= 0.0)
                    break;
                const int ind = mirrorIndex(jmin, datalen);
                if (ind < loind)
                    loind = ind;
                wbuf[ind] += dens;
            }

            for (int jmax = j0; jmax < INT_MAX; ++jmax)
            {
                const double x = (jmax - j0)*stepSize;
                const double dens = distro->density(x/h)/h;
                if (dens <= 0.0)
                    break;
                const int ind = mirrorIndex(jmax, datalen);
                if (ind > hiind)
                    hiind = ind;
                wbuf[ind] += dens;
            }

            if (firstWeightUsed)
                *firstWeightUsed = loind;
            if (sizeNeeded)
                *sizeNeeded = hiind + 1 - loind;
        }
        else
        {
            int jmin = j0 - 1;
            for (; jmin >= 0; --jmin)
            {
                const double x = (jmin - j0)*stepSize;
                const double dens = distro->density(x/h)/h;
                if (dens <= 0.0)
                    break;
                wbuf[jmin] = dens;
            }
            ++jmin;

            int jmax = j0;
            for (; jmax < datalen; ++jmax)
            {
                const double x = (jmax - j0)*stepSize;
                const double dens = distro->density(x/h)/h;
                if (dens <= 0.0)
                    break;
                wbuf[jmax] = dens;
            }
            assert(jmax > jmin);

            if (firstWeightUsed)
                *firstWeightUsed = jmin;
            if (sizeNeeded)
                *sizeNeeded = jmax - jmin;
        }
    }

    OrthoPoly1D* AbsBoundaryFilter1DBuilder::makeOrthoPoly(
        const unsigned maxDeg, const unsigned i,
        const unsigned dataLen, unsigned* filterCenter) const
    {
        if (!(dataLen && dataLen <= static_cast<unsigned>(INT_MAX)))
            throw std::invalid_argument(
                "In npstat::AbsBoundaryFilter1DBuilder::makeOrthoPoly: "
                "invalid data length parameter");
        assert(filterCenter);
        assert(i < dataLen);

        const bool hasExcl = !exclusionMask_.empty();
        if (hasExcl)
            if (exclusionMask_.size() != dataLen)
                throw std::invalid_argument(
                    "In npstat::AbsBoundaryFilter1DBuilder::makeOrthoPoly: "
                    "data length is not compatible with the exclusion mask");

        // Make sure we have enough space
        if (w_.size() < dataLen)
            w_.resize(dataLen);
        double* wbuf = &w_[0];

        // Calculate the bandwidth criterion with the weight placed
        // in the center of the interval
        if (centralIntegralLen_ != dataLen)
        {
            centralIntegral_ = calculateCriterion(
                distro_, 1.0, dataLen, dataLen/2, step_, wbuf);
            centralIntegralLen_ = dataLen;
        }

        // Figure out if the boundary affects the support of the weight
        const unsigned n_half = maxlen_/2;
        const bool buildBoundaryKernel = i < n_half || i + n_half >= dataLen;

        int j0;
        unsigned sizeNeeded;

        if (buildBoundaryKernel)
        {
            const double tol = 2.0*DBL_EPSILON*sqrt(dataLen) + 1.0e-13;

            double h = 1.0;
            const double currentInteg = calculateCriterion(
                distro_, h, dataLen, i, step_, wbuf);
            if (relative_delta(centralIntegral_, currentInteg) > tol)
            {
                // h is often expected to be in the [1, 2] interval
                const double firstFactorToTry = 1.2;
                const bool status = findRootInLogSpace(
                    Fnc(*this, dataLen, i), centralIntegral_,
                    sqrt(firstFactorToTry), tol, &h,
                    0.5*log(1.01*firstFactorToTry/0.99));
                assert(status);
                assert(h > 0.0);
            }

            // Rescan the weight function with the given h
            unsigned loind;
            scanTheDensity(distro_, h, dataLen, i, step_,
                           wbuf, &loind, &sizeNeeded);
            wbuf += loind;
            j0 = i - loind;
            lastBandwidthFactor_ = h;
        }
        else
        {
            // Fast scan of the density, even in case it is folded
            assert(dataLen >= maxlen_);
            j0 = static_cast<int>(n_half);
            sizeNeeded = maxlen_;
            const int jmax = sizeNeeded;
            for (int j=0; j<jmax; ++j)
                wbuf[j] = distro_->density(step_*(j - j0));
            lastBandwidthFactor_ = 1.0;
        }

        // Process the exclusions
        if (hasExcl)
        {
            const int jmax = sizeNeeded;
            for (int j=0; j<jmax; ++j)
            {
                // bin j0 of the weight corresponds to the mask bin i
                const int imask = j + static_cast<int>(i) - j0;
                if (imask >= 0 && imask < static_cast<int>(dataLen))
                    if (exclusionMask_[imask])
                        wbuf[j] = 0.0;
            }
        }

        if (excludeCentralPoint_)
            wbuf[j0] = 0.0;

        *filterCenter = j0;
        return new npstat::OrthoPoly1D(maxDeg, wbuf, sizeNeeded, step_);
    }
}
