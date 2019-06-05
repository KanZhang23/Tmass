#include <limits.h>
#include <math.h>

#include "event_efficiency.h"
#include "qqbar_deltaR_efficiency.h"

#ifdef __cplusplus
extern "C" {
#endif

double w_reco_efficiency(particle_obj q, particle_obj qbar)
{
    const double deltaR = conediff(q.p, qbar.p);
    const double qpt = sqrt(q.p.x*q.p.x + q.p.y*q.p.y);
    const double qbarpt = sqrt(qbar.p.x*qbar.p.x + qbar.p.y*qbar.p.y);

    /* Protection against null particles */
    if (qpt < 1.0 || qbarpt < 1.0)
        return 0.0;
    else
        return qqbar_deltaR_efficiency(deltaR, q.m/qpt, qbar.m/qbarpt);
}

#ifdef __cplusplus
}
#endif
