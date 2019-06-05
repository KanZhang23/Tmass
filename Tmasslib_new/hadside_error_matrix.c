#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "hadside_error_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Increment of the b mass squared for numerical derivative calculation */
#define DELTA_BMSQ 0.1

enum {
    HEM_B_X = 0,
    HEM_B_Y,
    HEM_B_Z,
    HEM_Q_X,
    HEM_Q_Y,
    HEM_Q_Z,
    HEM_QBAR_X,
    HEM_QBAR_Y,
    HEM_QBAR_Z,
    N_HEM_ROWS
};

enum {
    HEM_W_X = HEM_Q_X,
    HEM_W_Y,
    HEM_W_Z,
    WMAT_SIZE
};

enum {
    B = 0,
    Q,
    QBAR
};

enum {
    X = 0,
    Y,
    Z
};

enum {
    ETA = HADERR_B_ETA,
    PHI = HADERR_B_PHI,
    MSQ = HADERR_B_MSQ
};

void haderr_jacobian(const hadron_side_solution *hsol,
		     double jaco_top[N_HADERR_ROWS][N_HADERR_COLUMNS])
{
    int i;
    v3_obj jet_p[3];
    double jetmom[3];
    double jaco[N_HEM_ROWS][N_HADERR_COLUMNS] = {{0}};
    double deriv[N_HADERR_ROWS][WMAT_SIZE] = {{0}};

    assert(hsol->is_valid);
    assert(hsol->qP > 0.0);
    assert(hsol->qbarP > 0.0);
    assert(hsol->bP > 0.0);

    jet_p[B] = v3(hsol->bPx, hsol->bPy, hsol->bPz);
    jet_p[Q] = v3(hsol->qPx, hsol->qPy, hsol->qPz);
    jet_p[QBAR] = v3(hsol->qbarPx, hsol->qbarPy, hsol->qbarPz);
    jetmom[B] = hsol->bP;
    jetmom[Q] = hsol->qP;
    jetmom[QBAR] = hsol->qbarP;

    /* Figure out the angle terms */
    for (i=0; i<3; ++i)
    {
        const int base = i*3;
        const v3_obj * const p = jet_p + i;
        const double costheta = p->z/jetmom[i];
        jaco[base + X][base + PHI] = -p->y;
        jaco[base + Y][base + PHI] = p->x;
        jaco[base + X][base + ETA] = -p->x*costheta;
        jaco[base + Y][base + ETA] = -p->y*costheta;
        jaco[base + Z][base + ETA] = (p->x*p->x + p->y*p->y)/jetmom[i];
    }

    /* Figure out the mass terms */
    {
        const particle_obj q    = particle(jet_p[Q], hsol->mq);
        const particle_obj qbar = particle(jet_p[QBAR], hsol->mqbar);
        const particle_obj b    = particle(jet_p[B], hsol->mb);
        const particle_obj w    = sum4(q, qbar);
        const particle_obj t    = sum4(w, b);
        const double mwsq       = w.m*w.m;
        const double wmag       = mom(w.p);
        const v3_obj wdir       = v3(w.p.x/wmag, w.p.y/wmag, w.p.z/wmag);

        /* Direction in the q qbar plane orthogonal to the W direction */
        const v3_obj wperp = direction3(diff3(q.p, proj3(q.p, w.p)));

	/* Angles of the W daughters with the W direction */
        const double a1   = angle(q.p, w.p);
        const double a2   = -angle(qbar.p, w.p);
        const double sin1 = sin(a1);
        const double cos1 = cos(a1);
        const double sin2 = sin(a2);
        const double cos2 = cos(a2);

        /* Light quark mass derivatives along the W direction */
        const double dpqdmqsqpar = wmag*sin1*sin1/mwsq;
        const double dpqbardmqbarsqpar = wmag*sin2*sin2/mwsq;

        /* Light quark mass derivatives orthogonal to the W direction */
        const double dpqdmqsqperp = -wmag*cos1*sin1/mwsq;
        const double dpqbardmqbarsqperp = -wmag*cos2*sin2/mwsq;

        /* Boost top decay products into the top rest frame */
        const boost_obj top_boost = rest_boost(t);
        const boost_obj lab_boost = inverse_boost(top_boost);
        const particle_obj w_in_top = boost(w, top_boost);
        const particle_obj q_in_top = boost(q, top_boost);
        const particle_obj qbar_in_top = boost(qbar, top_boost);
        const particle_obj b_in_top = boost(b, top_boost);

        /* Increase the b mass squared by some small amount */
        const double new_b_mass = sqrt(b.m*b.m + DELTA_BMSQ);

        /* Redecay the top into the new b */
        const double new_e_b = (t.m*t.m + new_b_mass*new_b_mass - w.m*w.m)/(2.0*t.m);
        const double new_pcms = sqrt(new_e_b*new_e_b - new_b_mass*new_b_mass);
        const v3_obj b_in_top_dir = direction3(b_in_top.p);
        const particle_obj new_b_in_top = particle(mult3(b_in_top_dir, new_pcms), new_b_mass);
        const particle_obj new_w_in_top = particle(mult3(b_in_top_dir, -new_pcms), w.m);

        /* Redecay the new W */
        const boost_obj wboost = rest_boost(w_in_top);
        const boost_obj totop = inverse_boost(rest_boost(new_w_in_top));
        const particle_obj new_q_in_top = boost(boost(q_in_top, wboost), totop);
        const particle_obj new_qbar_in_top = boost(boost(qbar_in_top, wboost), totop);

        /* Boost the new particles into the lab frame */
        const particle_obj new_b = boost(new_b_in_top, lab_boost);
        const particle_obj new_q = boost(new_q_in_top, lab_boost);
        const particle_obj new_qbar = boost(new_qbar_in_top, lab_boost);

        /* Make up the particles with the energies of
         * the old particles and directions of the new ones.
         * Keep the b mass as it was for the old jet.
         */
        v3_obj eff_jet_p[3];
        eff_jet_p[Q]    = mult3(direction3(new_q.p), mom(q.p));
        eff_jet_p[QBAR] = mult3(direction3(new_qbar.p), mom(qbar.p));
        eff_jet_p[B]    = mult3(direction3(new_b.p), mom(b.p));

        /* Check various assumptions */
	assert(new_e_b > new_b_mass);
        assert(wmag > 0.0);
        assert(a1 > 0.0);  /* If not then "wperp" is screwed up */

        /* Fill out the result */
        jaco[HEM_Q_X][HADERR_Q_MSQ]       = dpqdmqsqpar*wdir.x + dpqdmqsqperp*wperp.x;
        jaco[HEM_Q_Y][HADERR_Q_MSQ]       = dpqdmqsqpar*wdir.y + dpqdmqsqperp*wperp.y;
        jaco[HEM_Q_Z][HADERR_Q_MSQ]       = dpqdmqsqpar*wdir.z + dpqdmqsqperp*wperp.z;
        jaco[HEM_QBAR_X][HADERR_QBAR_MSQ] = dpqbardmqbarsqpar*wdir.x + dpqbardmqbarsqperp*wperp.x;
        jaco[HEM_QBAR_Y][HADERR_QBAR_MSQ] = dpqbardmqbarsqpar*wdir.y + dpqbardmqbarsqperp*wperp.y;
        jaco[HEM_QBAR_Z][HADERR_QBAR_MSQ] = dpqbardmqbarsqpar*wdir.z + dpqbardmqbarsqperp*wperp.z;
        for (i=0; i<3; ++i)
        {
            const int base = i*3;
            jaco[base + X][HADERR_B_MSQ] = (eff_jet_p[i].x - jet_p[i].x)/DELTA_BMSQ;
            jaco[base + Y][HADERR_B_MSQ] = (eff_jet_p[i].y - jet_p[i].y)/DELTA_BMSQ;
            jaco[base + Z][HADERR_B_MSQ] = (eff_jet_p[i].z - jet_p[i].z)/DELTA_BMSQ;
        }

	/* Matrix of derivatives which takes us from W and b momenta
	 * to W inv. mass squared, t inv mass squared, t.x, and t.y.
	 */
	deriv[HADERR_W_MSQ][HEM_W_X] = -2.0*w.p.x;
	deriv[HADERR_W_MSQ][HEM_W_Y] = -2.0*w.p.y;
	deriv[HADERR_W_MSQ][HEM_W_Z] = -2.0*w.p.z;
	deriv[HADERR_T_MSQ][HEM_B_X] = -2.0*t.p.x;
	deriv[HADERR_T_MSQ][HEM_B_Y] = -2.0*t.p.y;
	deriv[HADERR_T_MSQ][HEM_B_Z] = -2.0*t.p.z;
	deriv[HADERR_T_MSQ][HEM_W_X] = -2.0*t.p.x;
	deriv[HADERR_T_MSQ][HEM_W_Y] = -2.0*t.p.y;
	deriv[HADERR_T_MSQ][HEM_W_Z] = -2.0*t.p.z;
	deriv[HADERR_T_X][HEM_B_X]   = 1.0;
	deriv[HADERR_T_Y][HEM_B_Y]   = 1.0;
	deriv[HADERR_T_X][HEM_W_X]   = 1.0;
	deriv[HADERR_T_Y][HEM_W_Y]   = 1.0;
    }

    /* Since W momentum is just the sum of q and qbar momenta,
     * all W derivatives are just sums of q and qbar derivatives.
     * We will perform the addition in-place.
     */
    for (i=0; i<N_HADERR_COLUMNS; ++i)
    {
	jaco[HEM_W_X][i] = jaco[HEM_Q_X][i] + jaco[HEM_QBAR_X][i];
	jaco[HEM_W_Y][i] = jaco[HEM_Q_Y][i] + jaco[HEM_QBAR_Y][i];
	jaco[HEM_W_Z][i] = jaco[HEM_Q_Z][i] + jaco[HEM_QBAR_Z][i];
    }

    /* Get the final answer */
    m_mult(&deriv[0][0], &jaco[0][0], N_HADERR_ROWS,
	   WMAT_SIZE, N_HADERR_COLUMNS, &jaco_top[0][0]);
}

void hadronic_mwsq_gradient(v3_obj pq, v3_obj pqbar,
                            double mwsq_gradient[N_HADERR_COLUMNS])
{
    const double mompq = mom(pq);
    const double mompqbar = mom(pqbar);
    const double costhetaq = pq.z/mompq;
    const double costhetaqbar = pqbar.z/mompqbar;
    const double cosa = sprod3(pq, pqbar)/mompq/mompqbar;
    const double dpqx_deta = -pq.x*costhetaq;
    const double dpqy_deta = -pq.y*costhetaq;
    const double dpqz_deta = (pq.x*pq.x + pq.y*pq.y)/mompq;
    const double dpqx_dphi = -pq.y;
    const double dpqy_dphi = pq.x;
    const double dpqz_dphi = 0.0;
    const double dpqbarx_deta = -pqbar.x*costhetaqbar;
    const double dpqbary_deta = -pqbar.y*costhetaqbar;
    const double dpqbarz_deta = (pqbar.x*pqbar.x + pqbar.y*pqbar.y)/mompqbar;
    const double dpqbarx_dphi = -pqbar.y;
    const double dpqbary_dphi = pqbar.x;
    const double dpqbarz_dphi = 0.0;
    const v3_obj pw = sum3(pq, pqbar);

    assert(mompq && mompqbar);

    mwsq_gradient[HADERR_B_ETA] = 0.0;
    mwsq_gradient[HADERR_B_PHI] = 0.0;
    mwsq_gradient[HADERR_B_MSQ] = 0.0;
    mwsq_gradient[HADERR_Q_ETA] = -2.0*(pw.x*dpqx_deta + pw.y*dpqy_deta + pw.z*dpqz_deta);
    mwsq_gradient[HADERR_Q_PHI] = -2.0*(pw.x*dpqx_dphi + pw.y*dpqy_dphi + pw.z*dpqz_dphi);
    mwsq_gradient[HADERR_Q_MSQ] = -mompqbar/mompq*(1.0 + cosa);
    mwsq_gradient[HADERR_QBAR_ETA] = -2.0*(pw.x*dpqbarx_deta + pw.y*dpqbary_deta + pw.z*dpqbarz_deta);
    mwsq_gradient[HADERR_QBAR_PHI] = -2.0*(pw.x*dpqbarx_dphi + pw.y*dpqbary_dphi + pw.z*dpqbarz_dphi);
    mwsq_gradient[HADERR_QBAR_MSQ] = -mompq/mompqbar*(1.0 + cosa);
}

#ifdef __cplusplus
}
#endif
