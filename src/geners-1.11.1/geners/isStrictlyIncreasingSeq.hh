#ifndef GENERS_ISSTRICTLYINCREASINGSEQ_HH_
#define GENERS_ISSTRICTLYINCREASINGSEQ_HH_

namespace gs {
    /** Check if the sequence of values is strictly increasing */
    template<class Iter>
    inline bool isStrictlyIncreasingSeq(Iter begin, Iter const end)
    {
        if (begin == end)
            return false;
        Iter first(begin);
        bool status = ++begin != end;
        for (; begin != end && status; ++begin, ++first)
            if (!(*first < *begin))
                status = false;
        return status;
    }
}

#endif // GENERS_ISSTRICTLYINCREASINGSEQ_HH_
