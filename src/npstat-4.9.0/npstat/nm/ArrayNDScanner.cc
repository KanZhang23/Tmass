#include <stdexcept>
#include <cassert>

#include "npstat/nm/ArrayNDScanner.hh"

namespace npstat {
    void ArrayNDScanner::initialize(
        const unsigned* shape, const unsigned lenShape)
    {
        // Check argument validity
        if (lenShape == 0U || lenShape >= CHAR_BIT*sizeof(unsigned long))
            throw std::invalid_argument(
                "In npstat::ArrayNDScanner::initialize: invalid scan shape");
        assert(shape);
        for (unsigned j=0; j<lenShape; ++j)
            if (!shape[j]) throw std::invalid_argument(
                "In npstat::ArrayNDScanner::initialize: "
                "number of scans must be positive in each dimension");

        // Initialize the scanner data
        state_ = 0UL;
        dim_ = lenShape;
        strides_[dim_ - 1] = 1UL;
        for (unsigned j=dim_ - 1; j>0; --j)
            strides_[j - 1] = strides_[j]*shape[j];
        maxState_ = strides_[0]*shape[0];
    }

    void ArrayNDScanner::getIndex(
        unsigned* ix, const unsigned indexBufferLen) const
    {
        if (indexBufferLen < dim_) throw std::invalid_argument(
            "In npstat::ArrayNDScanner::getIndex: "
            "insufficient length of the output buffer");
        if (state_ >= maxState_) throw std::runtime_error(
            "In npstat::ArrayNDScanner::getIndex: invalid scanner state");
        assert(ix);

        unsigned long l = state_;
        for (unsigned i=0; i<dim_; ++i)
        {
            unsigned long idx = l / strides_[i];
            ix[i] = static_cast<unsigned>(idx);
            l -= (idx * strides_[i]);
        }
    }
}
