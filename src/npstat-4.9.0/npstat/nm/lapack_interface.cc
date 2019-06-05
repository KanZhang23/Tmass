#include <cmath>
#include <cstring>
#include <cassert>

#include "npstat/nm/lapack_interface.hh"

static int lapack_nlvl_generic(char* name, const int dim)
{
    int N1 = 0, N2 = 0, N3 = 0, N4 = 0;
    int ISPEC = 9;
    char opts[] = "";
    const int SMLSIZ = ilaenv_(&ISPEC, name, opts, &N1, &N2, &N3, &N4,
                               strlen(name), 0);
    assert(SMLSIZ >= 0);
    if (dim <= SMLSIZ+1)
        return 1;
    else
        return int( log2( dim*1.0/(SMLSIZ+1) ) ) + 1;
}

namespace npstat {
    namespace Private {
        int lapack_nlvl_dgelsd(const int dim)
        {
            char name[] = "DGELSD";
            return lapack_nlvl_generic(name, dim);
        }

        int lapack_nlvl_sgelsd(const int dim)
        {
            char name[] = "SGELSD";
            return lapack_nlvl_generic(name, dim);
        }
    }
}
