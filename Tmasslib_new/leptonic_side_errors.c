#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "leptonic_side_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/* leptonic_mwsq_args collects quantities
 * which do not change during calculations of derivatives
 * (except at the very end when the derivarive over
 * b mass squared is calculated).
 */
typedef struct _leptonic_mwsq_args {
    double topPx;
    double topPy;
    double mt;
    double mwsq;
    double eb;
    double clx;
    double cly;
    double clz;
    double nuz_orig;
} leptonic_mwsq_args;

/* Increments for numerical calculations of derivatives */
static const double delta_eta_mag = 1.0e-4;
static const double delta_phi_mag = 1.0e-4;
static const double reldelta_lmom = 1.0e-5;
static const double delta_bmsq = 0.01;

/* The following code should be equivalent to
 * "solve_leptonic_byMWsq" but should work faster.
 */
static double leptonic_mtsq(
    const double bPx, const double bPy, const double bPz,
    const double ee, const leptonic_mwsq_args *hlp)
{
    const double lPx    = ee*hlp->clx;
    const double lPy    = ee*hlp->cly;
    const double lPz    = ee*hlp->clz;
    const double eesq   = ee*ee;
    const double pwx    = hlp->topPx - bPx;
    const double pwy    = hlp->topPy - bPy;
    const double pwtsq  = pwx*pwx + pwy*pwy;
    const double pnux   = pwx - lPx;
    const double pnuy   = pwy - lPy;
    const double pnutsq = pnux*pnux + pnuy*pnuy;
    const double tmp    = lPz*lPz + hlp->mwsq - pnutsq + pwtsq;
    const double a      = 4.0*(lPz*lPz - eesq);
    const double b      = 4.0*lPz*(tmp - eesq);
    const double c      = eesq*eesq + tmp*tmp - 2.0*eesq*(tmp + 2.0*pnutsq);

    int i, nsols=0, isol=0, nsolstmp;
    double pnuz[2], pnuztmp[2];

    /* Check various assumptions */
    assert(a != 0.0);

    /* Solve the quadratic equation */
    nsolstmp = solve_quadratic(b/a, c/a, pnuztmp+0, pnuztmp+1);

    /* Filter out fantom solutions */
    for (i=0; i<nsolstmp; ++i)
        if (pwtsq + lPz*lPz + 2.0*lPz*pnuztmp[i] + hlp->mwsq - eesq - pnutsq >= 0.0)
            pnuz[nsols++] = pnuztmp[i];
    
    if (nsols == 0)
        return -1.0;

    if (nsols > 1)
        /* Choose the solution with the closest nuz */
        if (fabs(pnuz[1] - hlp->nuz_orig) < fabs(pnuz[0] - hlp->nuz_orig))
            isol = 1;

    {
        const double enu   = sqrt(pnutsq + pnuz[isol]*pnuz[isol]);
        const double etop  = enu + hlp->eb + ee;
        const double toppz = pnuz[isol] + lPz + bPz;
        const double mtsq  = etop*etop - toppz*toppz - 
	    hlp->topPx*hlp->topPx - hlp->topPy*hlp->topPy;
	assert(mtsq >= 0.0);
	return mtsq;
    }
}

/* The following code should be equivalent to
 * "solve_leptonic_byPb" but should work faster.
 */
static double leptonic_mwsq(
    const double bPx, const double bPy, const double bPz,
    const double ee, const leptonic_mwsq_args *hlp)
{
    const double lPx    = ee*hlp->clx;
    const double lPy    = ee*hlp->cly;
    const double lPz    = ee*hlp->clz;
    const double pebz   = lPz + bPz;
    const double eeb    = ee + hlp->eb;
    const double pwx    = hlp->topPx - bPx;
    const double pwy    = hlp->topPy - bPy;
    const double pnux   = pwx - lPx;
    const double pnuy   = pwy - lPy;
    const double pnutsq = pnux*pnux + pnuy*pnuy;
    const double a0     = hlp->topPx*hlp->topPx + hlp->topPy*hlp->topPy +
                          pebz*pebz + hlp->mt*hlp->mt - eeb*eeb - pnutsq;
    const double b0     = 2.0*pebz;
    const double c0     = 2.0*eeb;
    const double a      = b0*b0 - c0*c0;
    const double b      = 2.0*a0*b0/a;
    const double c      = (a0*a0 - c0*c0*pnutsq)/a;

    int i, nsolstmp, nsols=0, isol=0;
    double ew, pwz, mwsq, pnuz[2], pnuztmp[2];

    /* Check various assumptions */
    assert(a != 0.0);

    /* Solve the quadratic equation */
    nsolstmp = solve_quadratic(b, c, pnuztmp+0, pnuztmp+1);

    /* Filter out fantom solutions */
    for (i=0; i<nsolstmp; ++i)
        if (a0 + b0*pnuztmp[i] >= 0.0)
            pnuz[nsols++] = pnuztmp[i];

    if (nsols == 0)
        return -1.0;

    if (nsols > 1)
        /* Choose the solution with the closest nuz */
        if (fabs(pnuz[1] - hlp->nuz_orig) < fabs(pnuz[0] - hlp->nuz_orig))
            isol = 1;

    ew   = sqrt(pnutsq + pnuz[isol]*pnuz[isol]) + ee;
    pwz  = pnuz[isol] + lPz;
    mwsq = ew*ew - pwx*pwx - pwy*pwy - pwz*pwz;

    assert(mwsq >= 0.0);
    return mwsq;
}

static void lep_side_masses(const leptonic_mwsq_args *hlp,
			    const double bPx, const double bPy,
			    const double bPz, const double mb,
			    const double lepMom, double *mtsq, double *mwsq)
{
    const particle_obj b  = particle(v3(bPx, bPy, bPz), mb);
    const particle_obj l  = particle(v3(lepMom*hlp->clx, lepMom*hlp->cly,
					lepMom*hlp->clz), 0.0);
    const particle_obj nu = particle(v3(hlp->topPx - b.p.x - l.p.x,
					hlp->topPy - b.p.y - l.p.y,
					hlp->nuz_orig), 0.0);
    const particle_obj w = sum4(l, nu);
    *mtsq = w.m*w.m + b.m*b.m + 2.0*sprod4(w, b);
    *mwsq = w.m*w.m;
}

void fill_lepton_side_solution(particle_obj l, particle_obj nu,
                               particle_obj b, lepton_side_solution *lsol)
{
    lsol->pblep = mom(b.p);
    lsol->bPx = b.p.x;  
    lsol->bPy = b.p.y;
    lsol->bPz = b.p.z;
    lsol->nux = nu.p.x;  
    lsol->nuy = nu.p.y;
    lsol->nuz = nu.p.z;
    lsol->mwsq = 2.0*sprod4(l, nu);
    lsol->tlepz = l.p.z + nu.p.z + b.p.z;
    lsol->mt = invmass3(l, nu, b);   
    lsol->fail = 0;    
}

#define fill_helper do {\
    helper.topPx = lPx + lsol->nux + lsol->bPx;\
    helper.topPy = lPy + lsol->nuy + lsol->bPy;\
    helper.mt    = lsol->mt;\
    helper.mwsq  = lsol->mwsq;\
    helper.eb    = hypot(lsol->pblep, mb);\
    helper.clx   = lPx/lmag0;\
    helper.cly   = lPy/lmag0;\
    helper.clz   = lPz/lmag0;\
    helper.nuz_orig = lsol->nuz;\
} while(0);

static void rehadronize_leptonic_side(
    const double lPx, const double lPy, const double lPz,
    const double mb, const lepton_side_solution *lsol,
    const double new_b_mass,
    v3_obj *new_l_mom, v3_obj *new_b_mom, double *new_nu_Pz)
{
    const particle_obj l = particle(v3(lPx, lPy, lPz), 0.0);
    const particle_obj nu = particle(v3(lsol->nux, lsol->nuy, lsol->nuz), 0.0);
    const particle_obj b = particle(v3(lsol->bPx, lsol->bPy, lsol->bPz), mb);
    const particle_obj w = sum4(l, nu);
    const particle_obj t = sum4(w, b);

    /* Boost everything to the top rest frame */
    const boost_obj top_boost = rest_boost(t);
    const boost_obj lab_boost = inverse_boost(top_boost);
    const particle_obj w_in_top = boost(w, top_boost);
    const particle_obj l_in_top = boost(l, top_boost);
    const particle_obj b_in_top = boost(b, top_boost);

    /* Redecay the top using new b mass */
    const double new_e_b = (t.m*t.m + new_b_mass*new_b_mass - w.m*w.m)/(2.0*t.m);
    const double new_pcms = sqrt(new_e_b*new_e_b - new_b_mass*new_b_mass);
    const v3_obj b_in_top_dir = direction3(b_in_top.p);
    const particle_obj new_b_in_top = particle(mult3(b_in_top_dir, new_pcms), new_b_mass);
    const particle_obj new_w_in_top = particle(mult3(b_in_top_dir, -new_pcms), w.m);

    /* Redecay the new W */
    const boost_obj wboost = rest_boost(w_in_top);
    const boost_obj totop = inverse_boost(rest_boost(new_w_in_top));
    const particle_obj new_l_in_top = boost(boost(l_in_top, wboost), totop);

    /* Boost the new particles into the lab frame */
    const particle_obj new_b = boost(new_b_in_top, lab_boost);
    const particle_obj new_l = boost(new_l_in_top, lab_boost);

    /* Check some assumptions */
    assert(new_e_b > new_b_mass);

    *new_l_mom = new_l.p;
    if (new_nu_Pz)
	*new_nu_Pz = t.p.z - new_b.p.z - new_l.p.z;

    /* Make up the new b with the energy of
     * the old b and direction of the new one.
     * Keep the b mass as it was for the old jet.
     */
    *new_b_mom = mult3(direction3(new_b.p), lsol->pblep);
}

void leperr_jacobian(const double lPx, const double lPy, const double lPz,
		     const double mb, const lepton_side_solution *lsol,
		     double jaco[N_LEPERR_ROWS][N_LEPERR_COLUMNS])
{
    int i;
    leptonic_mwsq_args helper;
    const double lmag0 = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double lpt = sqrt(lPx*lPx + lPy*lPy);
    double mtsq[2], mwsq[2];

    assert(lsol->pblep > 0.0);
    assert(!lsol->fail);
    assert(lpt > 0.0);

    fill_helper;

    /* Calculate derivatives over b eta */
    for (i=0; i<2; ++i)
    {
        const double delta_eta = i ? delta_eta_mag : -delta_eta_mag;
        const double dbpx = -lsol->bPz*lsol->bPx/lsol->pblep*delta_eta;
        const double dbpy = -lsol->bPz*lsol->bPy/lsol->pblep*delta_eta;
        const double dbpz = (lsol->bPx*lsol->bPx + lsol->bPy*lsol->bPy)/
                            lsol->pblep*delta_eta;
	lep_side_masses(&helper, lsol->bPx+dbpx, lsol->bPy+dbpy,
			lsol->bPz+dbpz, mb, lmag0, mtsq+i, mwsq+i);
    }
    jaco[LEPERR_MWSQ][LEPERR_B_ETA] = (mwsq[1] - mwsq[0])/2.0/delta_eta_mag;
    jaco[LEPERR_MTSQ][LEPERR_B_ETA] = (mtsq[1] - mtsq[0])/2.0/delta_eta_mag;

    /* Calculate derivatives over b phi */
    for (i=0; i<2; ++i)
    {
        const double delta_phi = i ? delta_phi_mag : -delta_phi_mag;
        const double dbpx = -lsol->bPy * delta_phi;
        const double dbpy = lsol->bPx * delta_phi;
	lep_side_masses(&helper, lsol->bPx+dbpx, lsol->bPy+dbpy,
			lsol->bPz, mb, lmag0, mtsq+i, mwsq+i);
    }
    jaco[LEPERR_MWSQ][LEPERR_B_PHI] = (mwsq[1] - mwsq[0])/2.0/delta_phi_mag;
    jaco[LEPERR_MTSQ][LEPERR_B_PHI] = (mtsq[1] - mtsq[0])/2.0/delta_phi_mag;

    /* Calculate derivatives over lepton transverse momentum */
    for (i=0; i<2; ++i)
    {
        const double delta_lp = i ? reldelta_lmom*lmag0 : -reldelta_lmom*lmag0;
	lep_side_masses(&helper, lsol->bPx, lsol->bPy,
			lsol->bPz, mb, lmag0+delta_lp, mtsq+i, mwsq+i);
    }
    jaco[LEPERR_MWSQ][LEPERR_L_PT] = (mwsq[1] - mwsq[0])/2.0/(reldelta_lmom*lpt);
    jaco[LEPERR_MTSQ][LEPERR_L_PT] = (mtsq[1] - mtsq[0])/2.0/(reldelta_lmom*lpt);

    /* Calculate the derivative over b mass squared. It is likely that
     * the original solution is for the mb = 0 point, so we will only
     * try to increase the mass. This calculation has to happen last
     * because it changes the value of "helper".
     */
    lep_side_masses(&helper, lsol->bPx, lsol->bPy,
		    lsol->bPz, mb, lmag0, mtsq, mwsq);
    {
        const double new_b_mass = sqrt(mb*mb + delta_bmsq);
	v3_obj new_l_p, pb;
        double new_l_mom;

	rehadronize_leptonic_side(lPx, lPy, lPz, mb, lsol,
				  new_b_mass, &new_l_p, &pb, NULL);

        new_l_mom = mom(new_l_p);
        if (new_l_mom > 0.0)
        {
            helper.clx = new_l_p.x/new_l_mom;
            helper.cly = new_l_p.y/new_l_mom;
            helper.clz = new_l_p.z/new_l_mom;
        }

	lep_side_masses(&helper, pb.x, pb.y,
			pb.z, mb, new_l_mom, mtsq+1, mwsq+1);
    }
    jaco[LEPERR_MWSQ][LEPERR_B_MSQ] = (mwsq[1] - mwsq[0])/delta_bmsq;
    jaco[LEPERR_MTSQ][LEPERR_B_MSQ] = (mtsq[1] - mtsq[0])/delta_bmsq;
}

void leptonic_mtsq_gradient(const double lPx, const double lPy, const double lPz,
                            const double mb, const lepton_side_solution *lsol,
                            double mtsq_gradient[N_LEPERR_COLUMNS])
{
    int i;
    leptonic_mwsq_args helper;
    const double lmag0 = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double lpt = sqrt(lPx*lPx + lPy*lPy);
    double mtsq_ref;

    assert(lsol->pblep > 0.0);
    assert(lsol->mt > 0.0);
    assert(lsol->mwsq > 0.0);
    assert(!lsol->fail);
    assert(lpt > 0.0);

    memset(mtsq_gradient, 0, N_LEPERR_COLUMNS*sizeof(double));

    fill_helper;

    /* Calculate the reference point */
    mtsq_ref = leptonic_mtsq(lsol->bPx, lsol->bPy, lsol->bPz, lmag0, &helper);
    assert(mtsq_ref >= 0.0);

    /* Calculate the derivative over b eta. The fancy 2-try approach
     * is an attempt to avoid hitting the kinematic boundary.
     */
    for (i=0; i<2; ++i)
    {
        const double delta_eta = i ? delta_eta_mag : -delta_eta_mag;
        const double dbpx = -lsol->bPz*lsol->bPx/lsol->pblep*delta_eta;
        const double dbpy = -lsol->bPz*lsol->bPy/lsol->pblep*delta_eta;
        const double dbpz = (lsol->bPx*lsol->bPx + lsol->bPy*lsol->bPy)/
                            lsol->pblep*delta_eta;
        const double mtsq = leptonic_mtsq(lsol->bPx + dbpx, lsol->bPy + dbpy,
                                          lsol->bPz + dbpz, lmag0, &helper);
        if (mtsq >= 0.0)
        {
            mtsq_gradient[LEPERR_B_ETA] = (mtsq - mtsq_ref)/delta_eta;
            break;
        }
    }

    /* Calculate the derivative over b phi */
    for (i=0; i<2; ++i)
    {
        const double delta_phi = i ? delta_phi_mag : -delta_phi_mag;
        const double dbpx = -lsol->bPy * delta_phi;
        const double dbpy = lsol->bPx * delta_phi;
        const double mtsq = leptonic_mtsq(lsol->bPx + dbpx, lsol->bPy + dbpy,
                                          lsol->bPz, lmag0, &helper);
        if (mtsq >= 0.0)
        {
            mtsq_gradient[LEPERR_B_PHI] = (mtsq - mtsq_ref)/delta_phi;
            break;
        }
    }

    /* Calculate the derivative over lepton transverse momentum */
    for (i=0; i<2; ++i)
    {
        const double delta_lp = i ? reldelta_lmom*lmag0 : -reldelta_lmom*lmag0;
        const double mtsq = leptonic_mtsq(lsol->bPx, lsol->bPy, lsol->bPz,
                                          lmag0 + delta_lp, &helper);
        if (mtsq >= 0.0)
        {
            mtsq_gradient[LEPERR_L_PT] = (mtsq - mtsq_ref)/delta_lp*lmag0/lpt;
            break;
        }
    }

    /* Calculate the derivative over b mass squared. It is likely that
     * the original solution is for the mb = 0 point, so we will only
     * try to increase the mass. This calculation has to happen last
     * because it changes the value of "helper".
     */
    {
        const double new_b_mass = sqrt(mb*mb + delta_bmsq);
	v3_obj new_l_p, pb;
        double mtsq, new_l_mom;

	rehadronize_leptonic_side(lPx, lPy, lPz, mb, lsol,
				  new_b_mass, &new_l_p, &pb, NULL);

        new_l_mom = mom(new_l_p);
        if (new_l_mom > 0.0)
        {
            helper.clx = new_l_p.x/new_l_mom;
            helper.cly = new_l_p.y/new_l_mom;
            helper.clz = new_l_p.z/new_l_mom;
        }

        mtsq = leptonic_mtsq(pb.x, pb.y, pb.z, new_l_mom, &helper);
        if (mtsq >= 0.0)
            mtsq_gradient[LEPERR_B_MSQ] = (mtsq - mtsq_ref)/delta_bmsq;
    }
}

void leptonic_mwsq_gradient(const double lPx, const double lPy, const double lPz,
                            const double mb, const lepton_side_solution *lsol,
                            double mwsq_gradient[N_LEPERR_COLUMNS])
{
    int i;
    leptonic_mwsq_args helper;
    const double lmag0 = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double lpt = sqrt(lPx*lPx + lPy*lPy);
    double mwsq_ref;

    assert(lsol->pblep > 0.0);
    assert(lsol->mt > 0.0);
    assert(lsol->mwsq > 0.0);
    assert(!lsol->fail);
    assert(lpt > 0.0);

    memset(mwsq_gradient, 0, N_LEPERR_COLUMNS*sizeof(double));

    fill_helper;

    /* Calculate the reference point */
    mwsq_ref = leptonic_mwsq(lsol->bPx, lsol->bPy, lsol->bPz, lmag0, &helper);
    assert(mwsq_ref >= 0.0);
    /* assert(fabs(mwsq_ref - lsol->mwsq)/lsol->mwsq < 0.04); */

    /* Calculate the derivative over b eta. The fancy 2-try approach
     * is an attempt to avoid hitting the kinematic boundary.
     */
    for (i=0; i<2; ++i)
    {
        const double delta_eta = i ? delta_eta_mag : -delta_eta_mag;
        const double dbpx = -lsol->bPz*lsol->bPx/lsol->pblep*delta_eta;
        const double dbpy = -lsol->bPz*lsol->bPy/lsol->pblep*delta_eta;
        const double dbpz = (lsol->bPx*lsol->bPx + lsol->bPy*lsol->bPy)/
                            lsol->pblep*delta_eta;
        const double mwsq = leptonic_mwsq(lsol->bPx + dbpx, lsol->bPy + dbpy,
                                          lsol->bPz + dbpz, lmag0, &helper);
        if (mwsq >= 0.0)
        {
            mwsq_gradient[LEPERR_B_ETA] = (mwsq - mwsq_ref)/delta_eta;
            break;
        }
    }

    /* Calculate the derivative over b phi */
    for (i=0; i<2; ++i)
    {
        const double delta_phi = i ? delta_phi_mag : -delta_phi_mag;
        const double dbpx = -lsol->bPy * delta_phi;
        const double dbpy = lsol->bPx * delta_phi;
        const double mwsq = leptonic_mwsq(lsol->bPx + dbpx, lsol->bPy + dbpy,
                                          lsol->bPz, lmag0, &helper);
        if (mwsq >= 0.0)
        {
            mwsq_gradient[LEPERR_B_PHI] = (mwsq - mwsq_ref)/delta_phi;
            break;
        }
    }

    /* Calculate the derivative over lepton transverse momentum */
    for (i=0; i<2; ++i)
    {
        const double delta_lp = i ? reldelta_lmom*lmag0 : -reldelta_lmom*lmag0;
        const double mwsq = leptonic_mwsq(lsol->bPx, lsol->bPy, lsol->bPz,
                                          lmag0 + delta_lp, &helper);
        if (mwsq >= 0.0)
        {
            mwsq_gradient[LEPERR_L_PT] = (mwsq - mwsq_ref)/delta_lp*lmag0/lpt;
            break;
        }
    }

    /* Calculate the derivative over b mass squared. It is likely that
     * the original solution is for the mb = 0 point, so we will only
     * try to increase the mass. This calculation has to happen last
     * because it changes the value of "helper".
     */
    {
        const double new_b_mass = sqrt(mb*mb + delta_bmsq);
	v3_obj new_l_p, pb;
        double mwsq, new_l_mom;

	rehadronize_leptonic_side(lPx, lPy, lPz, mb, lsol,
				  new_b_mass, &new_l_p, &pb, NULL);

        new_l_mom = mom(new_l_p);
        if (new_l_mom > 0.0)
        {
            helper.clx = new_l_p.x/new_l_mom;
            helper.cly = new_l_p.y/new_l_mom;
            helper.clz = new_l_p.z/new_l_mom;
        }

        mwsq = leptonic_mwsq(pb.x, pb.y, pb.z, new_l_mom, &helper);
        if (mwsq >= 0.0)
            mwsq_gradient[LEPERR_B_MSQ] = (mwsq - mwsq_ref)/delta_bmsq;
    }
}

#ifdef __cplusplus
}
#endif
