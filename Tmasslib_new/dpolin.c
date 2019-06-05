#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

void dpolin_(const double *XA, const double *YA, const int *N,
	     const double *X, double *Y, double *DY);

void dpolin_(const double *XA, const double *YA, const int *N,
	     const double *X, double *Y, double *DY)
{
    register const double x = *X;
    const double xm0  = x - XA[0];
    const double xm1  = x - XA[1];
    const double xm2  = x - XA[2];
    const double dx01 = xm1 - xm0;
    const double dx02 = xm2 - xm0;
    const double dx12 = xm2 - xm1;
    assert(*N == 3);
    *Y = xm1/dx01*xm2/dx02*YA[0] - 
	 xm0/dx01*xm2/dx12*YA[1] +
	 xm0/dx02*xm1/dx12*YA[2];
    *DY = 0.0;
}

#ifdef __cplusplus
}
#endif
