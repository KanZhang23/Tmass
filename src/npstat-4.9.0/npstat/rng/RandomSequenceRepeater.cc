#include <cassert>
#include <stdexcept>

#include "npstat/rng/RandomSequenceRepeater.hh"

namespace npstat {
    double RandomSequenceRepeater::operator()()
    {
        if (dim_ != 1U) throw std::invalid_argument(
            "In npstat::RandomSequenceRepeater::operator(): "
            "repetition of multivariate sequences is not implemented");
        if (pointer_ == sequence_.size())
            sequence_.push_back(orig_());
        return sequence_[pointer_++];
    }

    void RandomSequenceRepeater::run(double* buf, const unsigned bufLen,
                                     const unsigned nPt)
    {
        if (nPt)
        {
            if (bufLen < nPt*dim_) throw std::invalid_argument(
                "In npstat::RandomSequenceRepeater::run: "
                "insufficient length of the output buffer");
            assert(buf);

            // Do we need to add points?
            const unsigned long ptUsed = sequence_.size()/dim_;
            if (pointer_ + nPt > ptUsed)
            {
                const unsigned nAdd = pointer_ + nPt - ptUsed;
                assert(nAdd <= nPt);
                orig_.run(buf, bufLen, nAdd);
                const unsigned npush = nAdd*dim_;
                for (unsigned i=0; i<npush; ++i)
                    sequence_.push_back(buf[i]);
                if (pointer_ == ptUsed)
                {
                    pointer_ += nPt;
                    return;
                }
            }

            const double* from = &sequence_[pointer_*dim_];
            const unsigned ncopy = nPt*dim_;
            for (unsigned i=0; i<ncopy; ++i)
                *buf++ = *from++;
            pointer_ += nPt;
        }
    }

    void RandomSequenceRepeater::skip(const unsigned long nPt)
    {
        if (nPt)
        {
            // Do we need to add points?
            const unsigned long ptUsed = sequence_.size()/dim_;
            if (pointer_ + nPt > ptUsed)
            {
                const unsigned nAdd = pointer_ + nPt - ptUsed;
                if (dim_ > 1U)
                {
                    std::vector<double> bufVec(dim_);
                    double* buf = &bufVec[0];
                    for (unsigned i=0; i<nAdd; ++i)
                    {
                        orig_.run(buf, dim_, 1U);
                        for (unsigned j=0; j<dim_; ++j)
                            sequence_.push_back(buf[j]);
                    }
                }
                else
                    for (unsigned i=0; i<nAdd; ++i)
                        sequence_.push_back(orig_());
            }
            pointer_ += nPt;
        }
    }
}
