#include <assert.h>
#include "range_lookup_flt.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned range_lookup_flt(const float *bounds, const unsigned nbounds,
			  const float x)
{
    assert(bounds);
    assert(nbounds > 0);

    if (x < bounds[0])
        return 0;
    else if (x >= bounds[nbounds-1])
        return nbounds;
    else
    {
        unsigned imin = 0;
        unsigned imax = nbounds-2;
        do {
            unsigned imed = (imin + imax)/2;
            if (x < bounds[imed])
                imax = imed - 1;
            else if (x >= bounds[imed+1])
                imin = imed + 1;
            else
                return imed + 1;
        } while (imax - imin > 1);
        if (x >= bounds[imin] && x < bounds[imin+1])
            return imin+1;
        else
            return imax+1;
    }
}

int is_strictly_increasing_flt(const float *bounds, const unsigned nbounds)
{
    unsigned i;

    assert(bounds);
    assert(nbounds > 0);

    for (i=1; i<nbounds; ++i)
        if (bounds[i-1] >= bounds[i])
            return 0;
    return 1;
}

#ifdef __cplusplus
}
#endif
