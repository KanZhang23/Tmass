#include <cassert>
#include <vector>

#include "cernlib_replacement.h"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/nm/GaussLegendreQuadrature.hh"

#ifdef __cplusplus
extern "C" {
#endif

double cr_dfreq(const double *x)
{
    const npstat::Gauss1D g(0.0, 1.0);
    return g.cdf(*x);
}
    
double cr_dgausn(const double *x)
{
    const npstat::Gauss1D g(0.0, 1.0);
    return g.quantile(*x);
}

void cr_dgset(const double *a, const double *b, const int *n_in,
              double *x, double *w)
{
    assert(*a == -1.0);
    assert(*b == 1.0);
    const unsigned npoints = *n_in;
    const unsigned nhalf = npoints/2;
    assert(nhalf*2 == npoints);
    npstat::GaussLegendreQuadrature quad(npoints);
    std::vector<long double> buffer(npoints);
    long double* abscissae = &buffer[0];
    long double* weights = abscissae + nhalf;
    quad.getAbscissae(abscissae, nhalf);
    quad.getWeights(weights, nhalf);
    for (unsigned i=0; i<npoints; ++i)
    {
        unsigned j;
        if (i < nhalf)
            j = nhalf - i - 1;
        else
            j = i - nhalf;
        x[i] = abscissae[j];
        w[i] = weights[j];
    }
}

#ifdef __cplusplus
}
#endif
