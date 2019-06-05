#include <stdexcept>

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/nm/ArrayRange.hh"

namespace npstat {
    ArrayRange::ArrayRange(const unsigned* ishape, const unsigned imax)
    {
        if (imax)
        {
            assert(ishape);
            this->reserve(imax);
            for (unsigned i=0; i<imax; ++i)
                this->push_back(Interval<unsigned>(ishape[i]));
        }
    }

    bool ArrayRange::isCompatible(const ArrayShape& ishape) const
    {
        const unsigned imax = ishape.size();
        return isCompatible(imax ? &ishape[0] : (unsigned*)0, imax);
    }

    bool ArrayRange::isCompatible(const unsigned* ishape,
                                  const unsigned imax) const
    {
        if (this->size() != imax)
            return false;
        if (imax)
        {
            assert(ishape);
            for (unsigned i=0; i<imax; ++i)
                if ((*this)[i].length() == 0U)
                    return true;
            for (unsigned i=0; i<imax; ++i)
                if ((*this)[i].max() > ishape[i])
                    return false;
        }
        return true;
    }

    bool ArrayRange::operator<(const ArrayRange& r) const
    {
        const unsigned mysize = this->size();
        const unsigned othersize = r.size();
        if (mysize < othersize)
            return true;
        if (mysize > othersize)
            return false;
        for (unsigned i=0; i<mysize; ++i)
        {
            const Interval<unsigned>& left((*this)[i]);
            const Interval<unsigned>& right(r[i]);
            if (left.min() < right.min())
                return true;
            if (left.min() > right.min())
                return false;
            if (left.max() < right.max())
                return true;
            if (left.max() > right.max())
                return false;
        }
        return false;
    }

    ArrayRange& ArrayRange::stripOuterLayer()
    {
        const unsigned mysize = this->size();
        for (unsigned i=0; i<mysize; ++i)
        {
            (*this)[i].setMin((*this)[i].min() + 1U);
            const unsigned uplim = (*this)[i].max();
            if (uplim)
                (*this)[i].setMax(uplim - 1U);
        }
        return *this;
    }

    unsigned long ArrayRange::rangeSize() const
    {
        unsigned long result = 0UL;
        const unsigned imax = this->size();
        if (imax)
        {
            result = 1UL;
            for (unsigned i=0; i<imax; ++i)
                result *= (*this)[i].length();
        }
        return result;
    }

    ArrayShape ArrayRange::shape() const
    {
        const unsigned imax = this->size();
        ArrayShape oshape(imax);
        for (unsigned i=0; i<imax; ++i)
            oshape[i] = (*this)[i].length();
        return oshape;
    }

    void ArrayRange::lowerLimits(unsigned* limits,
                                 const unsigned limitsLen) const
    {
        const unsigned imax = this->size();
        if (limitsLen < imax) throw std::invalid_argument(
            "In npstat::ArrayRange::lowerLimits: "
            "insufficient size of the output buffer");
        if (imax)
        {
            assert(limits);
            const Interval<unsigned>* data = &(*this)[0];
            for (unsigned i=0; i<imax; ++i)
                limits[i] = data[i].min();
        }
    }

    void ArrayRange::upperLimits(unsigned* limits,
                                 const unsigned limitsLen) const
    {
        const unsigned imax = this->size();
        if (limitsLen < imax) throw std::invalid_argument(
            "In npstat::ArrayRange::upperLimits: "
            "insufficient size of the output buffer");
        if (imax)
        {
            assert(limits);
            const Interval<unsigned>* data = &(*this)[0];
            for (unsigned i=0; i<imax; ++i)
                limits[i] = data[i].max();
        }
    }

    void ArrayRange::rangeLength(unsigned* limits,
                                 const unsigned limitsLen) const
    {
        const unsigned imax = this->size();
        if (limitsLen < imax) throw std::invalid_argument(
            "In npstat::ArrayRange::rangeLength: "
            "insufficient size of the output buffer");
        if (imax)
        {
            assert(limits);
            const Interval<unsigned>* data = &(*this)[0];
            for (unsigned i=0; i<imax; ++i)
                limits[i] = data[i].length();
        }
    }

    bool ArrayRange::write(std::ostream& of) const
    {
        const unsigned long mydim = this->size();
        std::vector<unsigned> limits;
        limits.reserve(2UL*mydim);
        for (unsigned long i=0; i<mydim; ++i)
        {
            limits.push_back((*this)[i].min());
            limits.push_back((*this)[i].max());
        }
        gs::write_pod_vector(of, limits);
        return !of.fail();
    }

    void ArrayRange::restore(const gs::ClassId& id, std::istream& in,
                             ArrayRange* b)
    {
        static const gs::ClassId current(gs::ClassId::makeId<ArrayRange>());
        current.ensureSameId(id);

        std::vector<unsigned> limits;
        gs::read_pod_vector(in, &limits);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::ArrayRange::restore: "
                                    "input stream failure");
        const unsigned long nlimits = limits.size();
        if (nlimits % 2UL)
            throw gs::IOInvalidData("In npstat::ArrayRange::restore: "
                                    "bad limits");
        assert(b);
        b->clear();
        b->reserve(nlimits/2UL);
        for (unsigned long i=0; i<nlimits/2UL; ++i)
            b->push_back(npstat::Interval<unsigned>(limits[2U*i],
                                                    limits[2U*i+1U]));
    }
}
