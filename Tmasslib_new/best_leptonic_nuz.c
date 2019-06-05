#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include "best_leptonic_nuz.h"
#include "quartic_lib.h"
#include "sorter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double nuz;
    double distance;
    double mw;
    double mt;
} result;

sort_struct_by_member_incr(result, distance)

static double calculate_distance(
    const double eE, const double lPx, const double lPy, const double lPz,
    const double nuPx, const double nuPy, const double nuPz,
    const double ebE, const double ebPx, const double ebPy, const double ebPz,
    const double mebsq, const double mW0, const double mT0,
    const double hwhmWprop, const double hwhmTprop, const double Ecms,
    double *mw, double *mt)
{
    const double enu = sqrt(nuPx*nuPx + nuPy*nuPy + nuPz*nuPz);
    const double mwsq = 2.0*(eE*enu - lPx*nuPx - lPy*nuPy - lPz*nuPz);
    const double mtsq = mebsq + 2.0*(ebE*enu - ebPx*nuPx - ebPy*nuPy - ebPz*nuPz);
    const double dmw = (mwsq - mW0*mW0)/hwhmWprop;
    const double dmt = (mtsq - mT0*mT0)/hwhmTprop;
    double distance = hypot(dmw, dmt);

    assert(mwsq >= 0.0);
    assert(mtsq >= 0.0);

    if (enu + ebE > Ecms/2.0)
	/* This solution is invalid. Assign a very high distance. */
	distance += 100000.0;

    if (mw)
        *mw = sqrt(mwsq);
    if (mt)
        *mt = sqrt(mtsq);

    return distance;
}

int best_leptonic_nuz(const double topPx, const double topPy, 
                      const double lPx, const double lPy, const double lPz,
                      const double bPx, const double bPy, const double bPz,
                      const double mb, const double mW0, const double mT0,
                      const double hwhmWprop, const double hwhmTprop,
                      const double nuPz_ref, const double Ecms,
		      double *nuPz_out, double *distance, double *mw,
		      double *mt)
{
    const double nuPx  = topPx - lPx - bPx;
    const double nuPy  = topPy - lPy - bPy;
    const double eE    = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double eb    = sqrt(bPx*bPx + bPy*bPy + bPz*bPz + mb*mb);
    const double ebE   = eE + eb;
    const double ebPx  = lPx + bPx;
    const double ebPy  = lPy + bPy;
    const double ebPz  = lPz + bPz;
    const double mebsq = mb*mb + 2.0*(eE*eb - lPx*bPx - lPy*bPy - lPz*bPz);
    const double mw0sq = mW0*mW0/2.0;
    const double mt0sq = (mT0*mT0 - mebsq)/2.0;
    const double hwhmWsq = hwhmWprop*hwhmWprop;
    const double hwhmTsq = hwhmTprop*hwhmTprop;

    const double tmpb = mt0sq + ebPx*nuPx + ebPy*nuPy;
    const double tmpc = mw0sq + lPx*nuPx + lPy*nuPy;
    const double tmpd = nuPx*nuPx + nuPy*nuPy;

    const double c2  = -ebE*ebPz*hwhmWsq - eE*hwhmTsq*lPz;
    const double c11 = (eE*eE + lPz*lPz)*hwhmTsq + 
                       (ebE*ebE + ebPz*ebPz)*hwhmWsq;
    const double c10 = ebPz*hwhmWsq*tmpb + hwhmTsq*lPz*tmpc;
    const double c01 = -ebE*hwhmWsq*tmpb - eE*hwhmTsq*tmpc;
    const double c2pow2  = c2*c2;
    const double c10pow2 = c10*c10;

    const double a4 = 4*c2pow2 - c11*c11;
    const double a3 = -2*c10*c11 + 4*c01*c2;
    const double a2 = c01*c01 - c10pow2 + a4*tmpd;
    const double a1 = -2*(c10*c11 - c01*c2)*tmpd;
    const double a0 = tmpd*(c2pow2*tmpd - c10pow2);

    result r[5];
    double rts[4], nuz[4];
    int i, nrts, nnuz = 0;

    assert(hwhmWsq > 0.0);
    assert(hwhmTsq > 0.0);
    assert(nuPz_out);
    assert(distance);
    assert(mw);
    assert(mt);

    assert(a4 != 0.0);
    nrts = quartic(a3/a4, a2/a4, a1/a4, a0/a4, rts);

    /* Remove fantom roots */
    for (i=0; i<nrts; ++i)
    {
        const double nuPz  = rts[i];
        const double enu   = sqrt(tmpd + nuPz*nuPz);
        const double lside = c2*(enu*enu + nuPz*nuPz) + c01*nuPz;
        const double rside = -(c11*nuPz + c10)*enu;
        if (fabs(lside - rside) <= fabs(lside + rside))
            nuz[nnuz++] = nuPz;
    }

    /* Must have either one minimum or two minima and one maximum */
    assert(nnuz > 0);

    /* Pick the solution with the smallest distance */
    for (i=0; i<nnuz; ++i)
    {
        r[i].nuz = nuz[i];
        r[i].distance = calculate_distance(
            eE, lPx, lPy, lPz, nuPx, nuPy, nuz[i],
            ebE, ebPx, ebPy, ebPz, mebsq, mW0, mT0,
            hwhmWprop, hwhmTprop, Ecms, &r[i].mw, &r[i].mt);
    }
    /* Include the solution which corresponds
     * to the "reference" nuz value. The reson for
     * this inclusion is that the minimum point might
     * correspond to a nuz value which can not happen
     * due to the limited energy available in the CMS.
     */
    r[nnuz].nuz = nuPz_ref;
    r[nnuz].distance = calculate_distance(
	eE, lPx, lPy, lPz, nuPx, nuPy, nuPz_ref,
	ebE, ebPx, ebPy, ebPz, mebsq, mW0, mT0,
	hwhmWprop, hwhmTprop, Ecms, &r[nnuz].mw, &r[nnuz].mt);

    qsort(r, nnuz+1, sizeof(result), sort_result_by_distance_incr);

    *nuPz_out = r[0].nuz;
    *distance = r[0].distance;
    *mw = r[0].mw;
    *mt = r[0].mt;

    return nnuz;
}

#ifdef __cplusplus
}
#endif
