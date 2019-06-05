#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat>

#include "geners/GenericIO.hh"
#include "geners/IOException.hh"

#include "npstat/nm/ArrayND.hh"

#include "npstat/stat/LocalPolyFilter1DReader.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/continuousDegreeTaper.hh"
#include "npstat/stat/WeightTableFilter1DBuilder.hh"
#include "npstat/stat/BoundaryHandling.hh"


static unsigned closestDoublyStochastic(npstat::ArrayND<long double>& a)
{
    typedef npstat::Matrix<long double> Mat;

    assert(a.rank() == 2U);
    const unsigned n = a.span(0);
    assert(n == a.span(1));
    const Mat A(n, n, a.data());
    Mat Jn(n, n);
    Jn.constFill(1.0L/n);
    Mat W(n, n, 1);
    W -= Jn;
    const Mat& Bstar(W*A*W + Jn);
    a.setData(Bstar.data(), Bstar.length());
    return 1U;
}


// static double makeRowStochastic(npstat::Matrix<double>& m)
// {
//     const unsigned nrows = m.nRows();
//     const unsigned ncols = m.nColumns();
//     double maxDelta = 0.0;

//     for (unsigned irow=0; irow<nrows; ++irow)
//     {
//         double* data = &m[irow][0];
//         long double ldsum = 0.0L;
//         for (unsigned icol=0; icol<ncols; ++icol)
//             ldsum += data[icol];
//         const double dsum = ldsum;
//         if (dsum == 0.0)
//             throw std::runtime_error("In makeRowStochastic: row sum is zero");
//         for (unsigned icol=0; icol<ncols; ++icol)
//             data[icol] /= dsum;
//         if (fabs(dsum - 1.0) > maxDelta)
//             maxDelta = fabs(dsum - 1.0);
//     }
//     return maxDelta;
// }


static long double scalarProduct(
    const double *x, const double *y, const unsigned len)
{
    long double sum = 0.0L;
    for (unsigned i=0; i<len; ++i)
        sum += *x++ * *y++;
    return sum;
}


static void groomEigenvalues(npstat::Matrix<double>& fm,
                             const double* eigenvalues,
                             const unsigned lenEigenvalues,
                             npstat::Matrix<double>& eigenvec)
{
    assert(eigenvalues);
    assert(fm.nRows() == lenEigenvalues);
    assert(fm.isSquare());
    assert(eigenvec.nRows() == lenEigenvalues);
    assert(eigenvec.isSquare());

    // Figure out the first eigenvalue that is larger than 1
    unsigned iFirst = lenEigenvalues;
    for (unsigned ieigen = lenEigenvalues; ieigen > 0U; --ieigen)
    {
        if (eigenvalues[ieigen - 1U] > 1.0)
            iFirst = ieigen - 1U;
        else
            break;
    }

    // Renormalize the eigenvectors whose eigenvalues are larger than 1.
    // We will assume that they are already orthogonal.
    for (unsigned ieigen = iFirst; ieigen < lenEigenvalues; ++ieigen)
    {
        double* data = &eigenvec[ieigen][0];
        const long double ldnorm = scalarProduct(data, data, lenEigenvalues);
        const double norm = sqrtl(ldnorm);
        assert(norm);
        for (unsigned i=0; i<lenEigenvalues; ++i)
            data[i] /= norm;
    }

    std::vector<double> bufVec(lenEigenvalues*2UL);
    double* buf = &bufVec[0];
    double* amp = buf + lenEigenvalues;

    // Go over all eigenvectors whose eigenvalues are larger than 1
    // and figure out by how much they are "amplified"
    for (unsigned ieigen = iFirst; ieigen < lenEigenvalues; ++ieigen)
    {
        const double* edata = &eigenvec[ieigen][0];
        for (unsigned row=0; row<lenEigenvalues; ++row)
            buf[row] = scalarProduct(edata, &fm[row][0], lenEigenvalues);
        const long double ldnorm = scalarProduct(buf, buf, lenEigenvalues);
        amp[ieigen] = sqrtl(ldnorm);
    }

    // Go over the filter matrix columns and decompose them into
    // eigenvectors whose eigenvalues are larger than 1 and the rest
    for (unsigned col=0; col<lenEigenvalues; ++col)
    {
        double* matdata = &fm[0][col];
        for (unsigned ieigen = iFirst; ieigen < lenEigenvalues; ++ieigen)
        {
            const double* edata = &eigenvec[ieigen][0];
            long double sprod = 0.0L;
            for (unsigned row=0; row<lenEigenvalues; ++row)
                sprod += edata[row]*matdata[row*lenEigenvalues];

            // Decrease vector content by its amplification factor
            const long double newSprod = sprod/amp[ieigen];
            const double fix = newSprod - sprod;
            for (unsigned row=0; row<lenEigenvalues; ++row)
                matdata[row*lenEigenvalues] += edata[row]*fix;
        }
    }
}


namespace npstat {
    LocalPolyFilter1D::LocalPolyFilter1D()
        : taper_(0), bins_(0), nbins_(0), maxDegree_(0)
    {
    }

    LocalPolyFilter1D::LocalPolyFilter1D(
        const double* itaper, const unsigned maxDegree,
        const AbsFilter1DBuilder& filterBuilder,
        const unsigned dataLen)
        : taper_(0), bins_(0), nbins_(dataLen), maxDegree_(maxDegree)
    {
        if (!dataLen) throw std::invalid_argument(
            "In npstat::LocalPolyFilter1D constructor: "
            "invalid expected data length (must be positive)");

        // Figure out the length of the weight array in the center
        const unsigned n_kernel = filterBuilder.centralWeightLength();
        if (n_kernel != dataLen)
            if (!(n_kernel % 2)) throw std::invalid_argument(
                "In npstat::LocalPolyFilter1D constructor: "
                "invalid length of the weight array (must be odd integer)");
        const unsigned n_half = n_kernel/2;

        // Copy the taper coefficients
        taper_ = new double[maxDegree + 1];
        if (itaper)
            copyBuffer(taper_, itaper, maxDegree+1);
        else
            for (unsigned i=0; i<=maxDegree; ++i)
                taper_[i] = 1.0;

        // Go over all bins and figure out the poly set
        const bool keepAll = filterBuilder.keepAllFilters();
        PolyFilter1D *central = 0, *fitter = 0;
        bins_ = new PolyFilter1D*[dataLen];
        bandwidthFactors_.reserve(dataLen);
        double lastbw = 0.0;

        for (unsigned i=0; i<dataLen; ++i)
        {
            if (keepAll || n_kernel == dataLen ||
                i < n_half || i + n_half >= dataLen)
            {
                // Create boundary weight and polynomials
                fitter = filterBuilder.makeFilter(taper_, maxDegree_,
                                                  i, dataLen);
                unique_.push_back(fitter);
                lastbw = filterBuilder.lastBandwidthFactor();
            }
            else
            {
                // (Re)use the symmetric weight
                if (central == 0)
                {
                    // We should never get here if dataLen <= n_kernel
                    assert(dataLen > n_kernel);
                    central = filterBuilder.makeFilter(taper_, maxDegree_,
                                                       i, dataLen);
                    unique_.push_back(central);
                    lastbw = filterBuilder.lastBandwidthFactor();
                }
                fitter = central;
            }
            bins_[i] = fitter;
            bandwidthFactors_.push_back(lastbw);
        }
    }

    LocalPolyFilter1D::LocalPolyFilter1D(const LocalPolyFilter1D& r)
        : taper_(0), bins_(0)
    {
        copyOtherData(r);
    }

    LocalPolyFilter1D& LocalPolyFilter1D::operator=(const LocalPolyFilter1D& r)
    {
        if (&r != this)
        {
            releaseMem();
            copyOtherData(r);
        }
        return *this;
    }

    void LocalPolyFilter1D::copyOtherData(const LocalPolyFilter1D& r)
    {
        nbins_ = r.nbins_;
        maxDegree_ = r.maxDegree_;
        bandwidthFactors_ = r.bandwidthFactors_;

        assert(!taper_);
        taper_ = new double[maxDegree_ + 1];
        copyBuffer(taper_, r.taper_, maxDegree_ + 1);

        assert(unique_.empty());
        const unsigned nUnique = r.unique_.size();
        unique_.resize(nUnique);
        for (unsigned i=0; i<nUnique; ++i)
            unique_[i] = new PolyFilter1D(*r.unique_[i]);

        assert(!bins_);
        bins_ = new PolyFilter1D*[nbins_];
        for (unsigned ibin=0; ibin<nbins_; ++ibin)
        {
            const PolyFilter1D* searchFor = r.bins_[ibin];
            unsigned idx = nUnique;
            for (unsigned i=0; i<nUnique; ++i)
                if (r.unique_[i] == searchFor)
                {
                    idx = i;
                    break;
                }
            bins_[ibin] = unique_.at(idx);
        }
    }

    bool LocalPolyFilter1D::operator==(const LocalPolyFilter1D& r) const
    {
        if (!(nbins_ == r.nbins_ && maxDegree_ == r.maxDegree_))
            return false;
        for (unsigned i=0; i<=maxDegree_; ++i)
            if (taper_[i] != r.taper_[i])
                return false;
        for (unsigned i=0; i<nbins_; ++i)
            if (*bins_[i] != *r.bins_[i])
                return false;
        return bandwidthFactors_ == r.bandwidthFactors_;
    }

    void LocalPolyFilter1D::releaseMem()
    {
        for (unsigned i=unique_.size(); i>0; --i)
            delete unique_[i-1];
        unique_.clear();
        delete [] bins_; bins_ = 0;
        delete [] taper_; taper_ = 0;
    }

    LocalPolyFilter1D::~LocalPolyFilter1D()
    {
        releaseMem();
    }

    // CPP11_auto_ptr<LocalPolyFilter1D> LocalPolyFilter1D::eigenGroomedFilter(
    //     const double tolerance, const unsigned maxIterations) const
    // {
    //     Matrix<double> fm(nbins_, nbins_, 0);
    //     fillFilterMatrix(&fm);

    //     std::vector<double> eigenvaluesVec(nbins_);
    //     double* eigenvalues = &eigenvaluesVec[0];
    //     Matrix<double> eigenvectors(nbins_, nbins_);

    //     bool toleranceReached = false;
    //     double largestEigenvalue = 0.0;
    //     for (unsigned iter=0; iter<maxIterations; ++iter)
    //     {
    //         const Matrix<double>& ata = fm*fm.T();
    //         ata.symEigen(eigenvalues, nbins_, &eigenvectors, EIGEN_D_AND_C);


    //         std::cout << "eigenGrooming: " << iter
    //                   << ' ' << eigenvalues[nbins_ - 1U] << std::endl;
    //         if (iter && eigenvalues[nbins_ - 1U] > largestEigenvalue)
    //         {
    //             toleranceReached = true;
    //             break;
    //         }


    //         largestEigenvalue = eigenvalues[nbins_ - 1U];
    //         if (largestEigenvalue - 1.0 < tolerance)
    //         {
    //             toleranceReached = true;
    //             break;
    //         }

    //         groomEigenvalues(fm, eigenvalues, nbins_, eigenvectors);
    //         makeRowStochastic(fm);
    //     }

    //     if (tolerance > 0.0 && !toleranceReached)
    //         return CPP11_auto_ptr<LocalPolyFilter1D>();

    //     // Build a new filter out of the adjusted matrix
    //     CPP11_auto_ptr<LocalPolyFilter1D> res(new LocalPolyFilter1D());
    //     res->nbins_ = nbins_;
    //     res->maxDegree_ = maxDegree_;
    //     res->taper_ = new double[maxDegree_ + 1];
    //     copyBuffer(res->taper_, taper_, maxDegree_ + 1);
    //     res->unique_.resize(nbins_);
    //     res->bins_ = new PolyFilter1D*[nbins_];
    //     for (unsigned ibin=0; ibin<nbins_; ++ibin)
    //     {
    //         PolyFilter1D* nf = new PolyFilter1D(ibin);
    //         nf->resize(nbins_);
    //         copyBuffer(&(*nf)[0], &fm[ibin][0], nbins_);
    //         res->unique_[ibin] = nf;
    //         res->bins_[ibin] = nf;
    //     }

    //     return res;
    // }

    CPP11_auto_ptr<LocalPolyFilter1D>
    LocalPolyFilter1D::eigenGroomedFilter() const
    {
        Matrix<double> fm(nbins_, nbins_, 0);
        fillFilterMatrix(&fm);

        std::vector<double> eigenvaluesVec(nbins_);
        double* eigenvalues = &eigenvaluesVec[0];
        Matrix<double> eigenvectors(nbins_, nbins_);

        const Matrix<double>& ata = fm*fm.T();
        ata.symEigen(eigenvalues, nbins_, &eigenvectors, EIGEN_D_AND_C);
        groomEigenvalues(fm, eigenvalues, nbins_, eigenvectors);

        // Build a new filter out of the adjusted matrix
        CPP11_auto_ptr<LocalPolyFilter1D> res(new LocalPolyFilter1D());
        res->nbins_ = nbins_;
        res->maxDegree_ = maxDegree_;
        res->bandwidthFactors_ = bandwidthFactors_;
        res->taper_ = new double[maxDegree_ + 1];
        copyBuffer(res->taper_, taper_, maxDegree_ + 1);
        res->unique_.resize(nbins_);
        res->bins_ = new PolyFilter1D*[nbins_];
        for (unsigned ibin=0; ibin<nbins_; ++ibin)
        {
            PolyFilter1D* nf = new PolyFilter1D(ibin);
            nf->resize(nbins_);
            copyBuffer(&(*nf)[0], &fm[ibin][0], nbins_);
            res->unique_[ibin] = nf;
            res->bins_[ibin] = nf;
        }

        return res;
    }

    CPP11_auto_ptr<LocalPolyFilter1D> LocalPolyFilter1D::doublyStochasticFilter(
        const double tolerance, const unsigned maxIterations) const
    {
        ArrayND<long double> filterScan(nbins_, nbins_);
        filterScan.clear();
        long double* filterData = const_cast<long double*>(filterScan.data());
        for (unsigned ibin=0; ibin<nbins_; ++ibin)
        {
            const PolyFilter1D& f(*bins_[ibin]);
            const unsigned ipeak = f.peakPosition();
            const unsigned fLen = f.size();

            assert(ibin >= ipeak);
            assert(ibin + fLen <= nbins_ + ipeak);

            long double* dest = 0;
            if (ibin + fLen - ipeak == nbins_)
                dest = filterData + (ibin+1U)*nbins_ - fLen;
            else
                dest = filterData + ibin*nbins_ + (ibin - ipeak);
            copyBuffer(dest, &f[0], fLen);
        }

        unsigned nCyclesMade = 0;
        bool changingFilterLength = false;
        if (maxIterations)
        {
            const long double startSum = filterScan.sum<long double>();
            assert(startSum > 0.0L);
            if (filterScan.isDensity())
            {
                // Standard copula-making code in arbitrary dimensions
                // will work just fine. It will preserve positivity
                // of the filter and will not increase the number of
                // non-zero entries.
                nCyclesMade = filterScan.makeCopulaSteps(tolerance,
                                                         maxIterations);
            }
            else
            {
                // We have negative elements in the filter.
                // What are we going to do about it? Apparently,
                // it no longer makes sense to adjust individual
                // array elements as in the "makeCopulaSteps" method
                // because the procedure is not going to converge.
                //
                // The method employed here follows the article by
                // R. Khoury, "Closest Matrices in the Space of
                // Generalized Doubly Stochastic Matrices", Journal
                // of Mathematical Analysis and Applications 222,
                // 562-568 (1998). The "closeness" of the obtained
                // doubly stochastic filter should be understood
                // in the Frobenius norm.
                //
                nCyclesMade = closestDoublyStochastic(filterScan);
                changingFilterLength = true;
            }
            if (nCyclesMade)
            {
                const long double endSum = filterScan.sum<long double>();
                assert(endSum > 0.0L);
                filterScan *= (startSum/endSum);
            }
        }
        if (tolerance > 0.0 && nCyclesMade >= maxIterations)
            return CPP11_auto_ptr<LocalPolyFilter1D>();

        CPP11_auto_ptr<LocalPolyFilter1D> res(new LocalPolyFilter1D());
        res->nbins_ = nbins_;
        res->maxDegree_ = maxDegree_;
        res->bandwidthFactors_ = bandwidthFactors_;
        res->taper_ = new double[maxDegree_ + 1];
        copyBuffer(res->taper_, taper_, maxDegree_ + 1);
        res->unique_.resize(nbins_);
        res->bins_ = new PolyFilter1D*[nbins_];
        for (unsigned ibin=0; ibin<nbins_; ++ibin)
        {
            const PolyFilter1D& f(*bins_[ibin]);
            const unsigned ipeak = f.peakPosition();

            PolyFilter1D* nf = 0;
            unsigned nCopy = 0;
            const long double* src = 0;
            if (changingFilterLength)
            {
                nf = new PolyFilter1D(ibin);
                nCopy = nbins_;
                src = filterScan.data() + ibin*nbins_;
            }
            else
            {
                nf = new PolyFilter1D(ipeak);
                nCopy = f.size();
                if (ibin + nCopy - ipeak == nbins_)
                    src = filterScan.data() + (ibin+1U)*nbins_ - nCopy;
                else
                    src = filterScan.data() + ibin*nbins_ + (ibin - ipeak);
            }
            nf->resize(nCopy);
            copyBuffer(&(*nf)[0], src, nCopy);
            res->unique_[ibin] = nf;
            res->bins_[ibin] = nf;
        }

        return res;
    }

    double LocalPolyFilter1D::taper(const unsigned degree) const
    {
        if (degree <= maxDegree_)
            return taper_[degree];
        else
            return 0.0;
    }

    const PolyFilter1D& LocalPolyFilter1D::getFilter(
        const unsigned binNumber)  const
    {
        if (binNumber >= nbins_) throw std::out_of_range(
            "In npstat::LocalPolyFilter1D::getFilter: bin number out of range");
        return *bins_[binNumber];
    }

    double LocalPolyFilter1D::selfContribution(const unsigned binNumber) const
    {
        if (binNumber >= nbins_) throw std::out_of_range(
            "In npstat::LocalPolyFilter1D::selfContribution: "
            "bin number out of range");
        const unsigned filterPeak = bins_[binNumber]->peakPosition();
        return bins_[binNumber]->at(filterPeak);
    }

    void LocalPolyFilter1D::clearSumBuffer() const
    {
        if (sumBuffer_.size() != nbins_)
            sumBuffer_.resize(nbins_);
        long double* sum = &sumBuffer_[0];
        for (unsigned i=0; i<nbins_; ++i)
            sum[i] = 0.0L;
    }

    void LocalPolyFilter1D::addWeightedFilter(const long double w,
                                              const unsigned binNum) const
    {
        const PolyFilter1D* filter(bins_[binNum]);
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

    CPP11_auto_ptr<LocalPolyFilter1D> symbetaLOrPEFilter1D(
        const int power, const double bandwidth, const double mxdegree,
        const unsigned grid_points, double interval_min,
        double interval_max, const BoundaryHandling& bm,
        const unsigned char* exclusionMask, const bool excludeCentralPoint)
    {
        if (!grid_points) throw std::invalid_argument(
            "In npstat::symbetaLOrPEFilter: number of grid points "
            "must be positive");

        if (bandwidth < 0.0) throw std::invalid_argument(
            "In npstat::symbetaLOrPEFilter: bandwidth must not be negative");

        if (bandwidth == 0.0)
        {
            bool somethingIsMasked = excludeCentralPoint;
            if (exclusionMask && !excludeCentralPoint)
                somethingIsMasked = std::accumulate(
                    exclusionMask, exclusionMask+grid_points, 0UL);
            if (somethingIsMasked)
                throw std::invalid_argument(
                    "In npstat::symbetaLOrPEFilter: bandwidth must be positive");
            else
                return CPP11_auto_ptr<LocalPolyFilter1D>(
                    new DummyLocalPolyFilter1D(grid_points));
        }

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

        std::vector<double> taper;
        continuousDegreeTaper(mxdegree, &taper);

        CPP11_auto_ptr<AbsBoundaryFilter1DBuilder> filterBuilder =
            getBoundaryFilter1DBuilder(bm, kernel.get(),
                                       grid_step, exclusionMask, grid_points,
                                       excludeCentralPoint);
        return CPP11_auto_ptr<LocalPolyFilter1D>(
            new LocalPolyFilter1D(&taper[0], taper.size()-1U,
                                  *filterBuilder, grid_points));
    }

    Matrix<double> LocalPolyFilter1D::getFilterMatrix() const
    {
        Matrix<double> fm(nbins_, nbins_, 0);
        fillFilterMatrix(&fm);
        return fm;
    }

    void LocalPolyFilter1D::fillFilterMatrix(Matrix<double>* fm) const
    {
        assert(fm);
        assert(fm->nRows() == nbins_);
        assert(fm->isSquare());

        double* filterData = const_cast<double*>(fm->data());
        for (unsigned ibin=0; ibin<nbins_; ++ibin)
        {
            const PolyFilter1D& f(*bins_[ibin]);
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

    bool LocalPolyFilter1D::write(std::ostream& os) const
    {
        std::vector<unsigned> binMap(nbins_);
        const std::vector<PolyFilter1D*>::const_iterator b = unique_.begin();
        const std::vector<PolyFilter1D*>::const_iterator e = unique_.end();
        for (unsigned i=0; i<nbins_; ++i)
        {
            std::vector<PolyFilter1D*>::const_iterator it = 
                std::find(b, e, bins_[i]);
            assert(it != e);
            binMap[i] = it - b;
        }

        gs::write_pod_vector(os, binMap);
        gs::write_pod_vector(os, bandwidthFactors_);
        gs::write_pod(os, maxDegree_);
        gs::write_pod_array(os, taper_, maxDegree_+1U);
        gs::write_item(os, unique_);

        return !os.fail();
    }

    LocalPolyFilter1D* LocalPolyFilter1D::read(const gs::ClassId& id,
                                               std::istream& in)
    {
        static const gs::ClassId myId(gs::ClassId::makeId<LocalPolyFilter1D>());

        if (id.name() == myId.name())
        {
            myId.ensureSameVersion(id);

            std::vector<unsigned> binMap;
            gs::read_pod_vector(in, &binMap);

            CPP11_auto_ptr<LocalPolyFilter1D> obj(new LocalPolyFilter1D());
            gs::read_pod_vector(in, &obj->bandwidthFactors_);
            obj->nbins_ = binMap.size();
            gs::read_pod(in, &obj->maxDegree_);
            obj->taper_ = new double[obj->maxDegree_ + 1U];
            gs::read_pod_array(in, obj->taper_, obj->maxDegree_ + 1U);
            gs::restore_item(in, &obj->unique_);
            obj->bins_ = new PolyFilter1D*[obj->nbins_];
            for (unsigned i=0; i<obj->nbins_; ++i)
                obj->bins_[i] = obj->unique_[binMap[i]];

            if (in.fail())
                throw gs::IOReadFailure("In npstat::LocalPolyFilter1D::read: "
                                        "input stream failure");
            return obj.release();
        }
        else
            return StaticLocalPolyFilter1DReader::instance().read(id, in);
    }

    DummyLocalPolyFilter1D::DummyLocalPolyFilter1D(const unsigned dataLen)
        : LocalPolyFilter1D(0, 0U, NonmodifyingFilter1DBuilder(), dataLen)
    {
    }

    bool DummyLocalPolyFilter1D::write(std::ostream& os) const
    {
        const unsigned len = dataLen();
        gs::write_pod(os, len);
        return !os.fail();
    }

    DummyLocalPolyFilter1D* DummyLocalPolyFilter1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<DummyLocalPolyFilter1D>());
        current.ensureSameId(id);

        unsigned len = 0;
        gs::read_pod(in, &len);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::DummyLocalPolyFilter1D::read: "
                                    "input stream failure");
        return new DummyLocalPolyFilter1D(len);
    }

    double symbetaWeightAt0(const int m, const double bandwidth)
    {
        if (bandwidth <= 0.0) throw std::invalid_argument(
            "In npstat::symbetaWeightAt0: bandwidth must be positive");

        double y = 0.0;
        if (m < 0)
            y = 0.398942280401432678;
        else if (m <= 10)
        {
            static const double normcoeffs[11] = {
                0.5, 0.75, 0.9375, 1.09375, 1.23046875, 1.353515625,
                1.46630859375, 1.571044921875, 1.6692352294921875,
                1.76197052001953125, 1.85006904602050781};
            y = normcoeffs[m];
        }
        else
        {
            SymmetricBeta1D sb(0.0, 1.0, m);
            y = sb.density(0.0);
        }
        return y/bandwidth;
    }
}
