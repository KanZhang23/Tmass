#ifndef NPSTAT_MIRRORWEIGHT_HH_
#define NPSTAT_MIRRORWEIGHT_HH_

//======================================================================
// mirrorWeight.hh
//
// This is an internal header which is subject to change without notice.
// Application code should never call the functions declared/defined in
// this header directly.
//
// Author: I. Volobouev
//
// December 2011
//======================================================================

#include <cassert>
#include <climits>

#include "npstat/nm/ArrayND.hh"

namespace npstat {
    namespace Private {
        template <typename T, unsigned StackLen, unsigned StackDim>
        void mirrorWeightLoop(unsigned level, unsigned long idxFrom,
                              unsigned long idxTo, unsigned mirrorDirection,
                              const ArrayND<T,StackLen,StackDim>& from,
                              ArrayND<double,StackLen,StackDim>& to)
        {
            const bool positive(mirrorDirection & (1UL << level));
            const unsigned imax = from.span(level);
            if (level == from.rank() - 1)
            {
                const T* dfrom = from.data() + idxFrom;
                double* dto = const_cast<double *>(to.data() + (idxTo+imax-1));
                if (positive)
                    for (unsigned i=0; i<imax; ++i)
                        *dto++ = *dfrom++;
                else
                    for (unsigned i=0; i<imax; ++i)
                        *dto-- = *dfrom++;
            }
            else
            {
                const unsigned long fromstride = from.strides()[level];
                const unsigned long tostride = to.strides()[level];
                idxTo += (imax - 1)*tostride;
                for (unsigned i=0; i<imax; ++i)
                {
                    mirrorWeightLoop(level+1, idxFrom, idxTo,
                                     mirrorDirection, from, to);
                    idxFrom += fromstride;
                    if (positive)
                        idxTo += tostride;
                    else
                        idxTo -= tostride;
                }
            }
        }

        //
        // The "mirrorWeight" utility function turns the weight function scan in
        // one hyperoctant into the complete scan over the whole support region.
        // This can not be done by the ArrayND "multiMirror" method because the
        // input hyperoctant includes the central grid points so that the result
        // has an odd number of scanned points in each dimension.
        //
        template <typename T, unsigned StackLen, unsigned StackDim>
        void mirrorWeight(const ArrayND<T,StackLen,StackDim>& from,
                          ArrayND<double,StackLen,StackDim>* to)
        {
            const unsigned dim = from.rank();
            assert(dim);
#ifndef NDEBUG
            const unsigned maxdim = CHAR_BIT*sizeof(unsigned long);
            assert(dim < maxdim);
#endif
            assert(to);
            assert(dim == to->rank());
            for (unsigned i=0; i<dim; ++i)
                assert(to->span(i) == 2U*from.span(i) - 1U);
            const unsigned long maxcycle = 1UL << dim;
            for (unsigned long icycle=0UL; icycle<maxcycle; ++icycle)
                mirrorWeightLoop(0U, 0UL, 0UL, icycle, from, *to);
        }
    }
}

#endif // NPSTAT_MIRRORWEIGHT_HH_
