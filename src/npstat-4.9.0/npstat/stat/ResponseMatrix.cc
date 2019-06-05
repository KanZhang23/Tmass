#include <climits>
#include <numeric>

#include "npstat/stat/ResponseMatrix.hh"

namespace npstat {
    typedef ArrayND<std::pair<std::vector<unsigned long>, std::vector<double> > > Base;

    ResponseMatrix::ResponseMatrix(const ArrayShape& fromShape,
                                   const ArrayShape& toShape)
        : Base(fromShape), toShape_(toShape)
    {
    }

    ResponseMatrix::ResponseMatrix(const ArrayShape& fromShape,
                                   const ArrayShape& toShape,
                                   const Matrix<double>& dense)
        : Base(fromShape), toShape_(toShape)
    {
        sparsify(dense);
    }

    ResponseMatrix::ResponseMatrix(const unsigned* fromShape, unsigned fromDim,
                                   const unsigned* toShape, unsigned toDim)
        : Base(fromShape, fromDim), toShape_(toShape, toShape+toDim)
    {
    }

    ResponseMatrix::ResponseMatrix(const unsigned* fromShape, unsigned fromDim,
                                   const unsigned* toShape, unsigned toDim,
                                   const Matrix<double>& dense)
        : Base(fromShape, fromDim), toShape_(toShape, toShape+toDim)
    {
        sparsify(dense);
    }

    unsigned long ResponseMatrix::observedLength() const
    {
        const unsigned toSz = toShape_.size();
        unsigned long len = 1UL;
        for (unsigned i=0; i<toSz; ++i)
            len *= toShape_[i];
        return len;
    }

    double ResponseMatrix::linearEfficiency(const unsigned long i) const
    {
        const Base::value_type* d = data();
        return std::accumulate(d[i].second.begin(), d[i].second.end(), 0.0L);
    }

    bool ResponseMatrix::isValid() const
    {
        if (!rank())
            return false;
        const unsigned long thisLen = length();
        if (!thisLen)
            return false;

        if (!toShape_.size())
            return false;
        const unsigned long toLen = observedLength();
        if (!toLen)
            return false;

        bool hasPositive = false;

        const Base::value_type* dat = data();
        for (unsigned long i=0; i<thisLen; ++i)
        {
            const unsigned long sz = dat[i].first.size();
            if (sz != dat[i].second.size())
                return false;
            if (sz)
            {
                const unsigned long* idx = &dat[i].first[0];
                const double* probs = &dat[i].second[0];
                for (unsigned long j=0; j<sz; ++j)
                {
                    if (idx[j] >= toLen)
                        return false;
                    if (j && idx[j] <= idx[j-1])
                        return false;
                    if (probs[j] < 0.0)
                        return false;
                    if (probs[j] > 0.0)
                        hasPositive = true;
                }
            }
        }

        return hasPositive;
    }

    void ResponseMatrix::timesVector(const ArrayND<double>& unfoldedArr,
                                     ArrayND<double>* yhatArr) const
    {
        if (!isShapeCompatible(unfoldedArr)) throw std::invalid_argument(
            "In npstat::ResponseMatrix::timesVector: "
            "incompatible shape of the argument array");
        assert(yhatArr);
        yhatArr->reshape(toShape_);
        yhatArr->clear();
        double* yhat = const_cast<double*>(yhatArr->data());
        const double* unfolded = unfoldedArr.data();
        const unsigned long thisLen = length();
        const Base::value_type* dat = data();
        for (unsigned long i=0; i<thisLen; ++i)
        {
            const unsigned long sz = dat[i].first.size();
            if (sz)
            {
                const double source = unfolded[i];
                const unsigned long* idx = &dat[i].first[0];
                const double* probs = &dat[i].second[0];
                for (unsigned long j=0; j<sz; ++j)
                    yhat[idx[j]] += probs[j]*source;
            }
        }
    }

    void ResponseMatrix::rowMultiply(const ArrayND<double>& a,
                                     ArrayND<double>* result) const
    {
        if (!a.isCompatible(toShape_)) throw std::invalid_argument(
            "In npstat::ResponseMatrix::rowMultiply: "
            "incompatible shape of the argument array");
        assert(result);
        result->reshape(shapeData(), rank());
        double* rdata = const_cast<double*>(result->data());
        const double* adata = a.data();
        const unsigned long thisLen = length();
        const Base::value_type* dat = data();
        for (unsigned long i=0; i<thisLen; ++i)
        {
            long double sum = 0.0L;
            const unsigned long sz = dat[i].first.size();
            if (sz)
            {
                const unsigned long* idx = &dat[i].first[0];
                const double* probs = &dat[i].second[0];
                for (unsigned long j=0; j<sz; ++j)
                    sum += probs[j]*adata[idx[j]];
            }
            rdata[i] = sum;
        }
    }

    ResponseMatrix ResponseMatrix::T() const
    {
        ResponseMatrix result(&toShape_[0], toShape_.size(),
                              shapeData(), rank());
        Base::value_type* rdata = const_cast<Base::value_type*>(result.data());
        const unsigned long thisLen = length();
        const Base::value_type* dat = data();
        for (unsigned long i=0; i<thisLen; ++i)
        {
            const unsigned long sz = dat[i].first.size();
            if (sz)
            {
                const unsigned long* idx = &dat[i].first[0];
                const double* probs = &dat[i].second[0];
                for (unsigned long j=0; j<sz; ++j)
                {
                    rdata[idx[j]].first.push_back(i);
                    rdata[idx[j]].second.push_back(probs[j]);
                }
            }
        }
        return result;
    }

    void ResponseMatrix::shrinkToFit() const
    {
        const unsigned long thisLen = length();
        Base::value_type* dat = const_cast<Base::value_type*>(data());
        for (unsigned long i=0; i<thisLen; ++i)
        {
            std::vector<unsigned long>& idx(dat[i].first);
            const unsigned long sz = idx.size();
            const unsigned long cap = idx.capacity();
            if (cap > sz)
            {
                if (sz)
                {
                    std::vector<unsigned long>(idx).swap(idx);
                    std::vector<double>(dat[i].second).swap(dat[i].second);
                }
                else
                {
                    std::vector<unsigned long>().swap(idx);
                    std::vector<double>().swap(dat[i].second);
                }
            }
        }
    }

    bool ResponseMatrix::operator==(const ResponseMatrix& r) const
    {
        return toShape_ == r.toShape_ &&
            static_cast<const Base&>(*this) == static_cast<const Base&>(r);
    }

    void ResponseMatrix::sparsify(const Matrix<double>& dense)
    {
        const unsigned nrows = dense.nRows();
        const unsigned ncols = dense.nColumns();
        if (nrows != observedLength() || ncols != length())
            throw std::invalid_argument("In npstat::ResponseMatrix::sparsify: "
                                        "incompatible dense matrix dimensions");
        Base::value_type* dat = const_cast<Base::value_type*>(data());
        for (unsigned icol=0; icol<ncols; ++icol)
        {
            std::vector<unsigned long>& idx = dat[icol].first;
            std::vector<double>& probs = dat[icol].second;
            unsigned count = 0;
            for (unsigned row=0; row<nrows; ++row)
                if (dense[row][icol])
                    ++count;
            idx.reserve(count);
            probs.reserve(count);
            for (unsigned row=0; row<nrows; ++row)
            {
                const double v = dense[row][icol];
                if (v)
                {
                    idx.push_back(row);
                    probs.push_back(v);
                }
            }
        }
    }

    Matrix<double> ResponseMatrix::denseMatrix() const
    {
        const unsigned long nrows = observedLength();
        const unsigned long ncols = length();
        if (nrows*ncols > UINT_MAX) throw std::runtime_error(
            "In npstat::ResponseMatrix::denseMatrix: this sparse "
            "matrix can not be represented as dense (it is too large)");
        Matrix<double> result(nrows, ncols, 0);
        const Base::value_type* dat = data();
        for (unsigned icol=0; icol<ncols; ++icol)
        {
            const unsigned long sz = dat[icol].first.size();
            if (sz)
            {
                const unsigned long* idx = &dat[icol].first[0];
                const double* probs = &dat[icol].second[0];
                for (unsigned long j=0; j<sz; ++j)
                    result[idx[j]][icol] = probs[j];
            }
        }
        return result;
    }
}
