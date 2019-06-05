#include <stdexcept>

#include "npstat/stat/NMCombinationSequencer.hh"

namespace npstat {
    NMCombinationSequencer::NMCombinationSequencer(const unsigned Min,
                                                   const unsigned Nin)
        : coeffs_(Min),
          m_(Min),
          n_(Nin)
    {
        if (!(m_ && n_ >= m_)) throw std::invalid_argument(
            "In npstat::NMCombinationSequencer constructor: invalid arguments");
        reset();
    }

    void NMCombinationSequencer::reset()
    {
        count_ = 0UL;
        valid_ = true;
        for (unsigned i=0; i<m_; ++i)
            coeffs_[i] = i;        
    }

    void NMCombinationSequencer::increment()
    {
        ++count_;
        unsigned *cnt = &coeffs_[0];
        int last = m_ - 1U;
        for (unsigned del = 0; last >= 0; --last, ++del)
            if (++cnt[last] < n_ - del)
                break;
        if (last < 0)
        {
            valid_ = false;
            for (unsigned i=0; i<m_; ++i)
                cnt[i] = 0U;
        }
        else
        {
            const int nmax = m_;
            for (++last; last < nmax; ++last)
                cnt[last] = cnt[last - 1] + 1U;
        }
    }

    void NMCombinationSequencer::operator++(int)
    {
        if (valid_)
            increment();        
    }

    NMCombinationSequencer& NMCombinationSequencer::operator++()
    {
        if (valid_)
            increment();
        return *this;
    }
}
