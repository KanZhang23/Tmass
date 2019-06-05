#include <cassert>

#include "geners/GenericIO.hh"
#include "npstat/stat/SequentialPolyFilterND.hh"

namespace npstat {
    SequentialPolyFilterND::SequentialPolyFilterND(
        const LocalPolyFilter1D** filters,
        const unsigned nFilters, const bool takeOwnership)
        : filters_(0), nFilters_(nFilters), takeOwnership_(takeOwnership)
    {
        if (!nFilters_) throw std::invalid_argument(
            "In npstat::SequentialPolyFilterND constructor: "
            "no input filters provided");
        assert(filters);
        filters_ = new const LocalPolyFilter1D*[nFilters_];
        for (unsigned i=0; i<nFilters_; ++i)
            filters_[i] = filters[i];

        strides_.resize(nFilters_);
        strides_[nFilters_ - 1] = 1UL;
        for (unsigned j=nFilters_ - 1; j>0; --j)
            strides_[j - 1] = strides_[j]*filters_[j]->dataLen();

        indexBuf_.resize(nFilters_);
    }

    SequentialPolyFilterND::~SequentialPolyFilterND()
    {
        if (takeOwnership_)
            for (unsigned i=nFilters_; i>0; --i)
                delete filters_[i - 1];
        delete [] filters_;
    }

    bool SequentialPolyFilterND::operator==(
        const SequentialPolyFilterND& r) const
    {
        if (nFilters_ != r.nFilters_)
            return false;
        if (strides_ != r.strides_)
            return false;
        for (unsigned i=0; i<nFilters_; ++i)
            if (*(filters_[i]) != *(r.filters_[i]))
                return false;
        return true;
    }

    const LocalPolyFilter1D* SequentialPolyFilterND::filter(
        const unsigned dimNumber) const
    {
        if (dimNumber >= nFilters_) throw std::out_of_range(
            "In npstat::SequentialPolyFilterND::filter: "
            "dimension number is out of range");
        return filters_[dimNumber];
    }

    ArrayShape SequentialPolyFilterND::dataShape() const
    {
        ArrayShape shape;
        shape.reserve(nFilters_);
        for (unsigned i=0; i<nFilters_; ++i)
            shape.push_back(filters_[i]->dataLen());
        return shape;
    }

    ArrayND<double> SequentialPolyFilterND::getFilter(
        const unsigned* index, const unsigned lenIndex) const
    {
        if (lenIndex != nFilters_) throw std::invalid_argument(
            "In npstat::SequentialPolyFilterND::getFilter: "
            "incompatible index dimensionality");
        assert(index);

        const PolyFilter1D& firstFilter(filters_[0]->getFilter(index[0]));
        ArrayND<double> arr(makeShape(firstFilter.size()));
        arr.setData(&firstFilter[0], firstFilter.size());

        for (unsigned d=1; d<lenIndex; ++d)
        {
            const PolyFilter1D& filter(filters_[d]->getFilter(index[d]));
            ArrayND<double> a(makeShape(filter.size()));
            a.setData(&filter[0], filter.size());
            arr.uninitialize().operator=(arr.outer(a));
        }

        return arr;
    }

    Matrix<double> SequentialPolyFilterND::getFilterMatrix() const
    {
        Matrix<double> m(filters_[0]->getFilterMatrix());
        for (unsigned d=1; d<nFilters_; ++d)
        {
            const Matrix<double>& mat = m.outer(filters_[d]->getFilterMatrix());
            m.uninitialize();
            m = mat;
        }
        return m;
    }

    double SequentialPolyFilterND::selfContribution(const unsigned* index,
                                                    const unsigned dim) const
    {
        if (dim != nFilters_) throw std::invalid_argument(
            "In npstat::SequentialPolyFilterND::getFilter: "
            "incompatible index dimensionality");
        assert(index);

        double prod = 1.0;
        for (unsigned idim=0; idim<dim; ++idim)
        {
            const PolyFilter1D& filt = filters_[idim]->getFilter(index[idim]);
            prod *= filt.at(filt.peakPosition());
        }
        return prod;
    }

    double SequentialPolyFilterND::linearSelfContribution(
        unsigned long l) const
    {
        unsigned* idx = &indexBuf_[0];
        for (unsigned i=0; i<nFilters_; ++i)
        {
            idx[i] = l / strides_[i];
            l -= (idx[i] * strides_[i]);
        }
        return selfContribution(idx, nFilters_);
    }

    bool SequentialPolyFilterND::write(std::ostream& os) const
    {
        std::vector<const LocalPolyFilter1D*> ptrs(nFilters_);
        for (unsigned i=0; i<nFilters_; ++i)
            ptrs[i] = filters_[i];
        return gs::write_item(os, ptrs);
    }

    SequentialPolyFilterND* SequentialPolyFilterND::read(const gs::ClassId& id,
                                                         std::istream& in)
    {
        static const gs::ClassId myId(
            gs::ClassId::makeId<SequentialPolyFilterND>());
        myId.ensureSameId(id);

        std::vector<const LocalPolyFilter1D*> ptrs;
        gs::restore_item(in, &ptrs);
        return new SequentialPolyFilterND(&ptrs[0], ptrs.size(), true);
    }
}
