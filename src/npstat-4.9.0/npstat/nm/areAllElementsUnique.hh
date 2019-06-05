#ifndef NPSTAT_AREALLELEMENTSUNIQUE_HH_
#define NPSTAT_AREALLELEMENTSUNIQUE_HH_

/*!
// \file areAllElementsUnique.hh
//
// \brief A simple O(n^2) template function for checking if all elements in
//        a container are unique. Works well with small containers.
//
// Author: I. Volobouev
//
// June 2015
*/

namespace npstat {
    template<class Iter>
    inline bool areAllElementsUnique(Iter begin, Iter const end)
    {
        for (; begin != end; ++begin)
        {
            Iter comp(begin);
            for (++comp; comp != end; ++comp)
                if (*comp == *begin)
                    return false;
        }
        return true;
    }
}

#endif // NPSTAT_AREALLELEMENTSUNIQUE_HH_
