#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "geners/GenericIO.hh"
#include "geners/IOException.hh"

#include "npstat/nm/OrthoPoly1D.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/LocalMultiFilter1D.hh"

namespace npstat {
    LocalMultiFilter1D::LocalMultiFilter1D()
        : nbins_(0), maxDegree_(0)
    {
    }

    LocalMultiFilter1D::LocalMultiFilter1D(
        const unsigned maxDegree,
        const OrthoPolyFilter1DBuilder& filterBuilder,
        const unsigned dataLen)
        : bins_(maxDegree+1U), nbins_(dataLen), maxDegree_(maxDegree)
    {
        if (!dataLen) throw std::invalid_argument(
            "In npstat::LocalMultiFilter1D constructor: "
            "invalid expected data length (must be positive)");

        // Figure out the length of the weight array in the center
        const unsigned n_kernel = filterBuilder.centralWeightLength();
        if (n_kernel != dataLen)
            if (!(n_kernel % 2)) throw std::invalid_argument(
                "In npstat::LocalMultiFilter1D constructor: "
                "invalid length of the weight array (must be odd integer)");
        const unsigned n_half = n_kernel/2;

        // Vector to hold the taper coefficients
        std::vector<double> taper(maxDegree+1U, 0.0);

        // Pointers to filter data
        for (unsigned i=0; i<=maxDegree; ++i)
            bins_[i] = new PolyFilter1D*[dataLen];

        // Go over all bins and figure out the poly set
        const bool keepAll = filterBuilder.keepAllFilters();
        std::vector<PolyFilter1D*> central(maxDegree+1U, 0);
        for (unsigned i=0; i<dataLen; ++i)
        {
            CPP11_auto_ptr<OrthoPoly1D> poly;
            unsigned filterCenter = 0;
            taper[maxDegree] = 0.0;

            for (unsigned ideg=0; ideg<=maxDegree; ++ideg)
            {
                if (ideg)
                    taper[ideg - 1] = 0.0;
                taper[ideg] = 1.0;
                PolyFilter1D* filter = 0;

                if (keepAll || n_kernel == dataLen ||
                    i < n_half || i + n_half >= dataLen)
                {
                    // Create boundary weight and polynomials
                    if (!poly.get())
                        poly = CPP11_auto_ptr<OrthoPoly1D>(
                            filterBuilder.makeOrthoPoly(maxDegree, i, dataLen,
                                                        &filterCenter));
                    filter = new PolyFilter1D(filterCenter);
                    const unsigned filterLen = poly->length();
                    filter->resize(filterLen);
                    poly->linearFilter(&taper[0], maxDegree,
                                       filterCenter, &(*filter)[0], filterLen);
                    unique_.push_back(filter);
                }
                else
                {
                    // (Re)use the symmetric weight
                    if (central[ideg] == 0)
                    {
                        // We should never get here if dataLen <= n_kernel
                        assert(dataLen > n_kernel);
                        if (!poly.get())
                            poly = CPP11_auto_ptr<OrthoPoly1D>(
                                filterBuilder.makeOrthoPoly(maxDegree,i,dataLen,
                                                            &filterCenter));
                        central[ideg] = new PolyFilter1D(filterCenter);
                        const unsigned filterLen = poly->length();
                        central[ideg]->resize(filterLen);
                        poly->linearFilter(&taper[0], maxDegree, filterCenter,
                                           &(*central[ideg])[0], filterLen);
                        unique_.push_back(central[ideg]);
                    }
                    filter = central[ideg];
                }

                assert(filter);
                bins_[ideg][i] = filter;
            }
        }
    }

    LocalMultiFilter1D::LocalMultiFilter1D(const LocalMultiFilter1D& r)
    {
        copyOtherData(r);
    }

    LocalMultiFilter1D& LocalMultiFilter1D::operator=(const LocalMultiFilter1D& r)
    {
        if (&r != this)
        {
            releaseMem();
            copyOtherData(r);
        }
        return *this;
    }

    void LocalMultiFilter1D::copyOtherData(const LocalMultiFilter1D& r)
    {
        nbins_ = r.nbins_;
        maxDegree_ = r.maxDegree_;

        assert(unique_.empty());
        assert(bins_.empty());
        const unsigned nUnique = r.unique_.size();
        unique_.resize(nUnique);
        for (unsigned i=0; i<nUnique; ++i)
            unique_[i] = new PolyFilter1D(*r.unique_[i]);

        bins_.resize(maxDegree_+1U);
        for (unsigned deg=0; deg<=maxDegree_; ++deg)
        {
            bins_[deg] = new PolyFilter1D*[nbins_];
            for (unsigned ibin=0; ibin<nbins_; ++ibin)
            {
                const PolyFilter1D* searchFor = r.bins_[deg][ibin];
                unsigned idx = nUnique;
                for (unsigned i=0; i<nUnique; ++i)
                    if (r.unique_[i] == searchFor)
                    {
                        idx = i;
                        break;
                    }
                bins_[deg][ibin] = unique_.at(idx);
            }
        }
    }

    bool LocalMultiFilter1D::operator==(const LocalMultiFilter1D& r) const
    {
        if (!(nbins_ == r.nbins_ && maxDegree_ == r.maxDegree_))
            return false;
        const unsigned sz = bins_.size();
        if (r.bins_.size() != sz)
            return false;
        for (unsigned ideg=0; ideg<sz; ++ideg)
            for (unsigned i=0; i<nbins_; ++i)
                if (*bins_[ideg][i] != *r.bins_[ideg][i])
                    return false;
        return true;
    }

    void LocalMultiFilter1D::releaseMem()
    {
        for (unsigned i=unique_.size(); i>0; --i)
            delete unique_[i-1];
        unique_.clear();

        for (unsigned i=bins_.size(); i>0; --i)
            delete [] bins_[i-1];
        bins_.clear();
    }

    LocalMultiFilter1D::~LocalMultiFilter1D()
    {
        releaseMem();
    }

    const PolyFilter1D& LocalMultiFilter1D::getFilter(
        const unsigned degree, const unsigned binNumber)  const
    {
        if (degree > maxDegree_) throw std::out_of_range(
            "In npstat::LocalMultiFilter1D::getFilter: filter degree out of range");
        if (binNumber >= nbins_) throw std::out_of_range(
            "In npstat::LocalMultiFilter1D::getFilter: bin number out of range");
        return *bins_[degree][binNumber];
    }

    void LocalMultiFilter1D::clearSumBuffer() const
    {
        if (sumBuffer_.size() != nbins_)
            sumBuffer_.resize(nbins_);
        long double* sum = &sumBuffer_[0];
        for (unsigned i=0; i<nbins_; ++i)
            sum[i] = 0.0L;
    }

    void LocalMultiFilter1D::addWeightedFilter(const long double w,
                                               const unsigned degree,
                                               const unsigned binNum) const
    {
        const PolyFilter1D* filter(bins_.at(degree)[binNum]);
        const unsigned filterLen = filter->size();
        assert(filterLen <= nbins_);

        long double* sum = &sumBuffer_[0];
        const PolyFilter1D::value_type *f = &(*filter)[0];

        if (filterLen != nbins_)
        {
            if (filterLen % 2)
            {
                // Odd filter size. Are we in the center?
                const unsigned peak = filter->peakPosition();
                const unsigned center = filterLen/2;
                if (peak == center)
                {
                    // We are in the center
                    assert(binNum >= center);
                    sum += (binNum - center);
                }
                else if (peak > center)
                    // We are on the right side
                    sum += (nbins_ - filterLen);
            }
            else
            {
                // Even size filter. This can't be in the center.
                if (!(filter->peakPosition() < filterLen/2))
                    // We are on the right side
                    sum += (nbins_ - filterLen);
            }
        }

        for (unsigned i=0; i<filterLen; ++i)
            sum[i] += w*f[i];
    }

    CPP11_auto_ptr<LocalMultiFilter1D> symbetaMultiFilter1D(
        const int power, const double bandwidth, const unsigned mxdegree,
        const unsigned grid_points, double interval_min,
        double interval_max, const BoundaryHandling& bm,
        const unsigned char* exclusionMask, const bool excludeCentralPoint)
    {
        if (!grid_points) throw std::invalid_argument(
            "In npstat::symbetaMultiFilter: number of grid points "
            "must be positive");

        if (bandwidth <= 0.0) throw std::invalid_argument(
            "In npstat::symbetaMultiFilter: bandwidth must be positive");

        CPP11_auto_ptr<AbsDistribution1D> kernel;
        if (power < 0)
            kernel = CPP11_auto_ptr<AbsDistribution1D>(
                new TruncatedGauss1D(0.0, bandwidth, 12.0));
        else
            kernel = CPP11_auto_ptr<AbsDistribution1D>(
                new SymmetricBeta1D(0.0, bandwidth, power));
        if (interval_min > interval_max)
            std::swap(interval_min, interval_max);
        const double grid_step = (interval_max - interval_min)/grid_points;

        CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> filterBuilder =
            getBoundaryFilter1DBuilder(bm, kernel.get(), grid_step,
                                       exclusionMask, grid_points,
                                       excludeCentralPoint);
        return CPP11_auto_ptr<LocalMultiFilter1D>(
            new LocalMultiFilter1D(mxdegree, *filterBuilder, grid_points));
    }

    Matrix<double> LocalMultiFilter1D::getFilterMatrix(const unsigned degree) const
    {
        Matrix<double> fm(nbins_, nbins_, 0);
        fillFilterMatrix(degree, &fm);
        return fm;
    }

    void LocalMultiFilter1D::fillFilterMatrix(const unsigned degree,
                                              Matrix<double>* fm) const
    {
        assert(fm);
        assert(fm->nRows() == nbins_);
        assert(fm->isSquare());

        double* filterData = const_cast<double*>(fm->data());
        for (unsigned ibin=0; ibin<nbins_; ++ibin)
        {
            const PolyFilter1D& f(*bins_.at(degree)[ibin]);
            const unsigned ipeak = f.peakPosition();
            const unsigned fLen = f.size();

            assert(ibin >= ipeak);
            assert(ibin + fLen <= nbins_ + ipeak);

            double* dest = 0;
            if (ibin + fLen - ipeak == nbins_)
                dest = filterData + (ibin+1U)*nbins_ - fLen;
            else
                dest = filterData + ibin*nbins_ + (ibin - ipeak);
            copyBuffer(dest, &f[0], fLen);
        }        
    }

    bool LocalMultiFilter1D::write(std::ostream& os) const
    {
        std::vector<unsigned> binMap((maxDegree_+1U)*nbins_);
        const std::vector<PolyFilter1D*>::const_iterator b = unique_.begin();
        const std::vector<PolyFilter1D*>::const_iterator e = unique_.end();
        for (unsigned ideg=0; ideg<=maxDegree_; ++ideg)
            for (unsigned i=0; i<nbins_; ++i)
            {
                std::vector<PolyFilter1D*>::const_iterator it = 
                    std::find(b, e, bins_[ideg][i]);
                assert(it != e);
                binMap[ideg*nbins_ + i] = it - b;
            }

        gs::write_pod_vector(os, binMap);
        gs::write_pod(os, nbins_);
        gs::write_pod(os, maxDegree_);
        gs::write_item(os, unique_);

        return !os.fail();
    }

    LocalMultiFilter1D* LocalMultiFilter1D::read(const gs::ClassId& id,
                                                 std::istream& in)
    {
        static const gs::ClassId myId(gs::ClassId::makeId<LocalMultiFilter1D>());
        myId.ensureSameId(id);

        std::vector<unsigned> binMap;
        gs::read_pod_vector(in, &binMap);
        unsigned nbins, maxdegree;
        gs::read_pod(in, &nbins);
        gs::read_pod(in, &maxdegree);
        if (binMap.size() != (maxdegree + 1U)*nbins)
            throw gs::IOInvalidData("In npstat::LocalMultiFilter1D::read: "
                                    "corrupted object record");

        CPP11_auto_ptr<LocalMultiFilter1D> obj(new LocalMultiFilter1D());
        obj->nbins_ = nbins;
        obj->maxDegree_ = maxdegree;
        gs::restore_item(in, &obj->unique_);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::LocalMultiFilter1D::read: "
                                    "input stream failure");

        obj->bins_.resize(maxdegree+1U);
        for (unsigned deg=0; deg<=maxdegree; ++deg)
        {
            obj->bins_[deg] = new PolyFilter1D*[nbins];
            for (unsigned i=0; i<nbins; ++i)
                obj->bins_[deg][i] = obj->unique_[binMap[deg*nbins + i]];
        }
        return obj.release();
    }
}
