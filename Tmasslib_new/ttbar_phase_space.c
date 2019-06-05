#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include "ttbar_phase_space.h"

/* #define PRINT_PHASE_SPACE_FACTORS */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MTHADSQ = 0,
    MWHADSQ,
    PARAM,
    MTLEPSQ,
    SUMPTX,
    SUMPTY,
    MWLEPSQ
};

enum {
    Q = 0,
    QBAR,
    BHAD,
    BLEP,
    NUX,
    NUY,
    NUZ
};

static double nu_momentum_cutoff = 1.0;
static double q_momentum_cutoff = 1.0e-3;
static double had_phase_space_cutoff = 1.0;
static double detlepNu_cutoff = 1.0;
static double detlepW_cutoff = 1.0;

void set_phase_space_limits(const double nu_momentum_cutoff_in,
                            const double q_momentum_cutoff_in,
                            const double had_phase_space_cutoff_in,
                            const double detlepNu_cutoff_in,
                            const double detlepW_cutoff_in)
{
    nu_momentum_cutoff = nu_momentum_cutoff_in;
    q_momentum_cutoff = q_momentum_cutoff_in;
    had_phase_space_cutoff = had_phase_space_cutoff_in;
    detlepNu_cutoff = detlepNu_cutoff_in;
    detlepW_cutoff = detlepW_cutoff_in;
}

void ttbar_phase_space(
    const particle_obj lep, const particle_obj nu, const particle_obj blep,
    const particle_obj q, const particle_obj qbar, const particle_obj bhad,
    double *factor_mWsq, double *factor_nuPz)
{
    /* We need to build a Jacobian for converting variables
     *
     * |q.p|, |qbar.p|, |bhad.p|, |blep.p|, nu.p.x, nu.p.y, nu.p.z
     *
     * into
     *
     * mthadsq, mwhadsq, param, mtlepsq, sumPtx, sumPty, mwlepsq,
     *
     * where param = log(|q.p|/|qbar.p|).
     *
     * The second factor corresponds to the case when instead of
     * mwlepsq we continue using nu.p.z. Factors of 2 are omitted
     * in various rows.
     */
    if (factor_mWsq || factor_nuPz)
    {
        int i, j;
        double beta[4], rho[4], D[7][7] = {{0}};
        double dtmp, dethad, dethadW, detlepW = 0.0, detlepNu = 0.0;
        double rhoprod = 1.0, betaprod = 1.0, eta_factor = 1.0;
        const particle_obj *quarks[4];
        const particle_obj whad   = sum4(q, qbar);
        const particle_obj tophad = sum4(whad, bhad);
        const particle_obj wlep   = sum4(lep, nu);
        const particle_obj toplep = sum4(wlep, blep);
        const double Ewhad        = energy(whad);
        const double Ethad        = energy(tophad);
        const double Elep         = energy(lep);
        const double Etlep        = energy(toplep);
        double enu                = energy(nu);
        const v3_obj nudir        = div3(nu.p, enu);

        if (factor_mWsq)
            *factor_mWsq = 0.0;
        if (factor_nuPz)
            *factor_nuPz = 0.0;

        quarks[Q]    = &q;
        quarks[QBAR] = &qbar;
        quarks[BHAD] = &bhad;
        quarks[BLEP] = &blep;

        for (i=0; i<4; ++i)
        {
            rho[i]  = mom(quarks[i]->p);
            if (rho[i] < q_momentum_cutoff)
                return;
            beta[i] = rho[i]/energy(*quarks[i]);
            rhoprod  *= rho[i];
            betaprod *= beta[i];
	    eta_factor *= ((quarks[i]->p.x*quarks[i]->p.x + 
			    quarks[i]->p.y*quarks[i]->p.y)/rho[i]/rho[i]);
        }

/*         for (i=0; i<3; ++i) */
/*             D[MTHADSQ][i] = Ethad*beta[i] - sprod3(quarks[i]->p,tophad.p)/rho[i]; */
        i = BHAD;
        D[MTHADSQ][i] = Ethad*beta[i] - sprod3(quarks[i]->p,tophad.p)/rho[i];

        for (i=0; i<2; ++i)
            D[MWHADSQ][i] = Ewhad*beta[i] - sprod3(quarks[i]->p,whad.p)/rho[i];

	/* In the following two lines we assume param = log(rho[Q]/rho[QBAR]) */
        D[PARAM][Q]    = 1.0/rho[Q];
        D[PARAM][QBAR] = -1.0/rho[QBAR];

        D[MTLEPSQ][BLEP] = beta[BLEP]*Etlep - sprod3(blep.p, toplep.p)/rho[BLEP];
        D[MTLEPSQ][NUX]  = nudir.x*Etlep - toplep.p.x;
        D[MTLEPSQ][NUY]  = nudir.y*Etlep - toplep.p.y;
        D[MTLEPSQ][NUZ]  = nudir.z*Etlep - toplep.p.z;

        for (i=0; i<4; ++i)
            D[SUMPTX][i] = quarks[i]->p.x/rho[i];
        D[SUMPTX][NUX] = 1.0;

        for (i=0; i<4; ++i)
            D[SUMPTY][i] = quarks[i]->p.y/rho[i];
        D[SUMPTY][NUY] = 1.0;

        D[MWLEPSQ][NUX]  = 2.0*(nudir.x*Elep - lep.p.x);
        D[MWLEPSQ][NUY]  = 2.0*(nudir.y*Elep - lep.p.y);
        D[MWLEPSQ][NUZ]  = 2.0*(nudir.z*Elep - lep.p.z);

        /* Turns out that this Jacobian has a very sparse structure. 
         * In fact, its determinant can be factorized into the product
         * of determinants of three matrices 2x2, 1x1, and 4x4.
         *
         * The 4x4 matrix can be associated with the leptonic side,
         * while 2x2 and 1x1 matrices are relevant to the hadronic side.
         */
        dethadW = D[MWHADSQ][Q]*D[PARAM][QBAR] - D[PARAM][Q]*D[MWHADSQ][QBAR];
        dethad = D[MTHADSQ][BHAD] * dethadW;

        /* The approximate Jacobian below assumes zero q and qbar masses.
         *
         * dethad = (Ethad*beta[BHAD] - sprod3(quarks[BHAD]->p,tophad.p)/rho[BHAD])*
	 *   (1.0 - cos3(q.p, qbar.p));
         */

        /* Limit the effect of the phase space on the likelihood */
        if (fabs(dethad) < had_phase_space_cutoff)
            dethad = had_phase_space_cutoff;
        if (enu < nu_momentum_cutoff)
            enu = nu_momentum_cutoff;

        dtmp = mom(lep.p)*eta_factor*betaprod*rhoprod/enu/dethad;

        if (factor_mWsq)
        {
            double m[4][4];
            for (i=0; i<4; ++i)
                for (j=0; j<4; j++)
                    m[i][j] = D[MTLEPSQ+i][MTLEPSQ+j];
            detlepW = det4by4(m);
            if (fabs(detlepW) < detlepW_cutoff)
                detlepW = detlepW_cutoff;
            *factor_mWsq = fabs(dtmp/detlepW);
        }

        if (factor_nuPz)
        {
            double u[3][3];
            for (i=0; i<3; ++i)
                for (j=0; j<3; j++)
                    u[i][j] = D[MTLEPSQ+i][MTLEPSQ+j];
            detlepNu = det3by3(u);
            if (fabs(detlepNu) < detlepNu_cutoff)
                detlepNu = detlepNu_cutoff;
            *factor_nuPz = fabs(dtmp/detlepNu);
        }

#ifdef PRINT_PHASE_SPACE_FACTORS
        printf("\nphsp_factors %g %g %g %g %g %g %g\n",
               dethad, D[MTHADSQ][BHAD], dethadW,
               D[MWHADSQ][Q], D[MWHADSQ][QBAR],
               detlepW, detlepNu);
#endif
    }
}

#ifdef __cplusplus
}
#endif
