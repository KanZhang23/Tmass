#include <assert.h>

#include "twidth.h"

#ifdef __cplusplus
extern "C" {
#endif

static double bMassValue = -1.0;   

double twidks_(double* M_T, double* M_W, double* M_B);
double twidth_(double* M_T, double* M_W, double* M_B);
double bqmass_(void);

double getTopWidth(double mt, double mw, double mb)
{
    return twidth_(&mt, &mw, &mb);
}

void setBQuarkMass(const double mb)
{
    assert(mb >= 0.0);
    bMassValue = mb;
}

double getBQuarkMass(void)
{
    return bqmass_();
}

double twidth_(double* M_T, double* M_W, double* M_B)
{
    static double mt = -127622.405, mw = -2276567.9, mb = -32323.5, twid = 0;
    if (!(*M_T == mt && *M_W == mw && *M_B == mb))
    {
        mt = *M_T;
        mw = *M_W;
        mb = *M_B;
        twid = twidks_(M_T, M_W, M_B);
    }
    return twid;
}

double bqmass_(void)
{
    assert(bMassValue >= 0.0);
    return bMassValue;
}

#ifdef __cplusplus
}
#endif
