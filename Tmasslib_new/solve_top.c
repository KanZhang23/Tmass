#include <math.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sorter.h"
#include "solve_top.h"
#include "simple_kinematics.h"
#include "quartic_lib.h"
#include "topmass_utils.h"
#include "rpoly.h"
#include "ellipse_intersection.h"

#define MAX_ITERATIONS 100
#define RELATIVE_EPS_LEPTONIC 1.0e-12
#define RELATIVE_EPS_MINWMASS 1.0e-12

/* #define PRINT_DOUBLE(x) do {\      */
/*     printf(#x " = %.16g\n", (x));\ */
/* } while(0);                        */

#ifdef __cplusplus
extern "C" {
#endif

/* Interface to LAPACK */
void dgetrf_(int *M, int *N, double *A, int *LDA, int *IPIV, int *INFO);
void dgetrs_(char *TRANS, int *N, int *NRHS, double *A, int *LDA, int *IPIV,
	     double *B, int *LDB, int *INFO, int lenTRANS);

typedef struct {
    double d1;
    double d2;
} d_d_pair;

typedef struct {
    double d1;
    double d2;
    double d3;
} d_d_d_triple;

static const double start_beta[] = {1.0, 0.99, 0.975, 0.95, 0.9, 0.85, 0.75, 0.55, 0.3};

sort_struct_by_member_incr(d_d_pair, d2)
sort_struct_by_member_incr(d_d_d_triple, d3)
sort_struct_by_member_incr(lepton_side_solution, mwsq)

double pb_distance(const double pbsol, const double pb_data)
{
    assert(pbsol > 0.0);
    assert(pb_data > 0.0);
    return fabs(log(pbsol/pb_data));
}

static int positive_quartic_roots(double a, double b, double c,
                                  double d, double e, double rts[4]);

particle_obj hadronic_top_particle(const hadron_side_solution *hsol)
{
    assert(hsol);
    assert(hsol->is_valid);
    {
        const particle_obj parq    = particle(v3(hsol->qPx, hsol->qPy,
                                                 hsol->qPz), hsol->mq);
        const particle_obj parqbar = particle(v3(hsol->qbarPx, hsol->qbarPy,
                                                 hsol->qbarPz), hsol->mqbar);
        const particle_obj parbhad = particle(v3(hsol->bPx, hsol->bPy,
                                                 hsol->bPz), hsol->mb);
        return sum4(parbhad, sum4(parq, parqbar));
    }
}

int nuz_local_peak(const double topPx, const double topPy,
                   const double lPx, const double lPy, const double lPz,
                   const double bPx, const double bPy, const double bPz,
                   const double mt, const double mb_in, const int debug_level,
                   const double init_pb, const double init_nuz,
                   lepton_side_solution *extremum)
{
    const double mtsqt = mt*mt + topPx*topPx + topPy*topPy;
    const double bmag  = sqrt(bPx*bPx + bPy*bPy + bPz*bPz);
    const double cbx   = bPx/bmag;
    const double cby   = bPy/bmag;
    const double cbz   = bPz/bmag;
    const double ee    = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double cbtsq = cbx*cbx + cby*cby;

    double dpb = 0.0, dnuz = 0.0;
    double pb = init_pb, nuz = init_nuz, olddpb = 0.0;
    int niter, nchangedir = 0, converged = 0;

    /* Check various assumptions */
    assert(bmag > 0.0);

    /* This function only works for massless b */
    if (mb_in != 0.0 && debug_level > 0)
    {
        printf("WARNING in nuz_local_peak: mass of the b quark is ignored!\n");
        fflush(stdout);
    }

    /* Try to find an extremum from the given starting point */
    for (niter=0; niter<MAX_ITERATIONS; ++niter)
    {
        const double bx      = pb*cbx;
        const double by      = pb*cby;
        const double bz      = pb*cbz;
        const double nux     = topPx - bx - lPx;
        const double nuy     = topPy - by - lPy;
        const double enu     = sqrt(nux*nux + nuy*nuy + nuz*nuz);
        const double nubproj = cbx*nux + cby*nuy;
        const double tz      = lPz + bz + nuz;
        const double et      = ee + pb + enu;
        const double tmp     = (et - cbz*tz - nubproj)/enu;

        const double eq1     = et*et - tz*tz - mtsqt;
        const double eq2     = enu*(et - cbz*tz) - et*nubproj;

        const double deq1dpb  = 2.0*et*(1.0 - nubproj/enu) - 2.0*cbz*tz;
        const double deq1dnuz = 2.0*nuz*et/enu - 2.0*tz;
        const double deq2dpb  = cbtsq*(et + enu) - nubproj*(tmp + 2.0);
        const double deq2dnuz = nuz*(tmp + 1.0) - enu*cbz;

        const double det      = deq1dpb*deq2dnuz - deq1dnuz*deq2dpb;

        assert(enu != 0.0);
        if (det == 0.0)
            break;

        dpb  = (deq1dnuz*eq2 - deq2dnuz*eq1)/det;
        dnuz = (deq2dpb*eq1  - deq1dpb*eq2)/det;

        pb  += dpb;
        nuz += dnuz;

        if (debug_level >= 30)
        {
            printf("nuz_local_peak: iteration %d, pb = %g, nuPz = %g\n",
                   niter, pb, nuz);
            printf("Corresponding increments are %g and %g\n", dpb, dnuz);
        }

        /* Break if pb increments seem to be fluctuating */
        if (olddpb*dpb < 0.0)
            if (fabs(dpb) > fabs(olddpb))
                if (++nchangedir > 2)
                    break;
        olddpb = dpb;

        /* Break if the increments are small enough */
        if (fabs(dpb)/(fabs(pb) + 1.0) < RELATIVE_EPS_MINWMASS &&
            fabs(dnuz)/(fabs(nuz) + 1.0) < RELATIVE_EPS_MINWMASS)
        {
            const double denudpb = -nubproj/enu;
            const double d2eq1dpb = (deq2dpb - denudpb*deq1dpb)/enu;
            assert(deq1dnuz != 0.0);
            if (-d2eq1dpb/deq1dnuz > 0.0)
                /* This is a minimum */
                converged = -1;
            else
                /* This is a maximum */
                converged = 1;
            break;
        }
    }

    /* Check the convergence */
    if (!converged && debug_level >= 10)
    {
        static int max_warnings_iter = 1000;
        if (max_warnings_iter > 0)
        {
            printf("WARNING in nuz_local_peak: iterations failed to converge.\n");
            printf("Relative solution precisions are %g and %g, requested %g.\n",
                   fabs(dpb)/(fabs(pb) + 1.0), fabs(dnuz)/(fabs(nuz) + 1.0),
                   RELATIVE_EPS_MINWMASS);
            if (--max_warnings_iter == 0)
                printf("Further occurrences of this message "
                       "will be suppressed\n");
            fflush(stdout);
        }
    }

    if (converged && pb > 0.0)
    {
        /* Fill out the solution */
        const double wx   = topPx - pb*cbx;
        const double wy   = topPy - pb*cby;
        const double wz   = nuz + lPz;
        const double ew   = ee + pb;
        const double mwsq = ew*ew - wx*wx - wy*wy - wz*wz;

        assert(mwsq > 0.0);
        extremum->mwsq  = mwsq;
        extremum->pblep = pb;
        extremum->bPx   = pb*cbx;
        extremum->bPy   = pb*cby;
        extremum->bPz   = pb*cbz;
        extremum->mb    = 0.0;
        extremum->nux   = wx - lPx;
        extremum->nuy   = wy - lPy;
        extremum->nuz   = nuz;
        extremum->tlepz = pb*cbz + wz;
        extremum->mt    = mt;
        extremum->fail  = 0;

        return converged;
    }
    else
    {
        memset(extremum, 0, sizeof(lepton_side_solution));
        return 0;
    }
}

int w_mass_range(const double i_topPx, const double i_topPy,
                 const double i_lPx, const double i_lPy, const double ePz,
                 const double i_bPx, const double i_bPy, const double i_bPz,
                 const double mt, const double mb, const int debug_level,
                 const double *initial_mw_points, const size_t npoints,
                 lepton_side_solution *mwmin, lepton_side_solution *mwmax)
{
    /* At the mW extrema the phase space Jacobian will go to 0.
     * If b is massive, finding this point analytically seems
     * to be hopeless. We'll just try the gradient descent instead.
     * Unfortunately, the W mass minimum is not the only reason
     * why the Jacobian can become 0.
     *
     * It is convenient to rotate the coordinate system so that
     * the b direction lies in the yz plane. Then the x component
     * of the neutrino momentum can be found immediately, and all
     * subsequent calculations are simplified.
     */
    int nlsols = 0, solcount = 0, isol;
    size_t startpoint;
    double b[3], equation_matrix[3][3];
    double mwsq_start = 0.0, minWmassSq = DBL_MAX;
    lepton_side_solution pbinit[2];

    /* Rotate the coordinates */
    const v3_obj zAxis      = v3(0.0, 0.0, 1.0);
    const v3_obj yPrimeAxis = direction3(v3(i_bPx, i_bPy, 0.0));
    const v3_obj xPrimeAxis = vprod3(yPrimeAxis, zAxis);

    /* Calculate various things in the new coordinate system */
    const v3_obj i_topPt   = v3(i_topPx, i_topPy, 0.0);
    const v3_obj i_l       = v3(i_lPx, i_lPy, ePz);
    const double tPx       = sprod3(xPrimeAxis, i_topPt);
    const double tPyGoal   = sprod3(yPrimeAxis, i_topPt);
    const double ePx       = sprod3(xPrimeAxis, i_l);
    const double ePy       = sprod3(yPrimeAxis, i_l);
    const double ee        = mom(i_l);
    const double i_bPt     = hypot(i_bPx, i_bPy);
    const double i_Pb      = hypot(i_bPt, i_bPz);
    const double cby       = i_bPt/i_Pb;
    const double cbz       = i_bPz/i_Pb;
    const double nuPx      = tPx - ePx;
    const double wPx       = tPx;
    const double mbsquared = mb*mb;
    const double mtsqGoal  = mt*mt;

    /* Reset the solutions */
    memset(mwmin, 0, sizeof(lepton_side_solution));
    memset(mwmax, 0, sizeof(lepton_side_solution));
    mwmax->mwsq = (mt - mb)*(mt - mb);
    mwmax->mt   = mt;

    /* Find a starting point */
    assert(initial_mw_points);
    assert(npoints > 0);
    for (startpoint=0; startpoint<npoints; ++startpoint)
    {
        if (startpoint)
            assert(initial_mw_points[startpoint] > initial_mw_points[startpoint-1]);
        else
            assert(initial_mw_points[startpoint] >= 0.0);
        mwsq_start = initial_mw_points[startpoint]*initial_mw_points[startpoint];
	if (mwsq_start*1.01 >= mwmax->mwsq)
	    continue;
        nlsols = solve_leptonic_side(
            tPx, tPyGoal, ePx, ePy, ePz, 0.0, i_bPt, i_bPz,
            mt, mb, mwsq_start, debug_level, 1, pbinit);
        if (nlsols > 0)
            ++solcount;
        else
            solcount = 0;
        if (solcount > 1)
            break;
        if (solcount > 0 && startpoint == npoints-1)
        {
            /* Check numerical stability of nlsols using slightly higher mwsq */
            mwsq_start *= 1.01;
            nlsols = solve_leptonic_side(
                tPx, tPyGoal, ePx, ePy, ePz, 0.0, i_bPt, i_bPz,
                mt, mb, mwsq_start, debug_level, 1, pbinit);
        }
    }
    if (nlsols == 0)
        return 0;

    /* Go down the slope */
    for (isol=0; isol<nlsols; ++isol)
    {
        double pb   = pbinit[isol].pblep;
        double nuPy = pbinit[isol].nuy;
        double nuPz = pbinit[isol].nuz;
        double mwsq = mwsq_start;

        int niter;
        for (niter=0; niter<MAX_ITERATIONS; ++niter)
        {
            int ifail = 0;
            int nrows1 = 3;
            int IPIV[3];
            int NRHS = 1;

            const double wPy = nuPy + ePy;
            const double wPz = nuPz + ePz;
            const double tPy = wPy + cby*pb;
            const double tPz = wPz + cbz*pb;
            const double enu = sqrt(nuPx*nuPx + nuPy*nuPy + nuPz*nuPz);
            const double eb = sqrt(pb*pb + mbsquared);
            const double et = ee + enu + eb;
            const double ebcubed = eb*eb*eb;
            const double beta = pb/eb;
            const double c0 = enu*et*(ee*nuPz - ePz*enu);
            const double c1 = enu*(cby*(ePz*nuPy - ePy*nuPz)*beta + 
                                   beta*beta*(ee*nuPz - ePz*enu) + 
                                   cbz*(-(ee*(cby*nuPy + cbz*nuPz)) + 
                                        (cby*ePy + cbz*ePz)*enu));
            const double dpb = (c1 + c0*mbsquared/ebcubed);
            const double d0 = -(nuPz*(ee*(cbz*nuPz*tPz + 
                                          cby*(ePy*nuPz + nuPy*nuPz + cbz*nuPy*pb)) + 
                                      cby*(ePy*nuPz - ePz*nuPy)*eb -
                                      ee*nuPz*beta*(ee + eb)));
            const double d1 = 2*nuPz*(cbz*ePz*tPz + 
                                      cby*(ePz*nuPy + ePy*(tPz - nuPz))) - 
                (ee*(cby*(ePy + nuPy) + cbz*(tPz + nuPz)) + cby*ePy*eb)*
                enu + cbz*ePz*enu*enu + 
                beta*(2*ee*nuPz*(-ePz + nuPz) + (ee*ee - 3*ePz*nuPz)*enu + 
                      ee*enu*enu + eb*(-2*ePz*nuPz + ee*enu));
            const double dnuPz = (d1 + d0/enu);
            const double e0 = -(nuPy*(ee*(cbz*nuPz*tPz + 
                                          cby*(ePy*nuPz + nuPy*nuPz + cbz*nuPy*pb)) + 
                                      cby*(ePy*nuPz - ePz*nuPy)*eb -
                                      ee*nuPz*beta*(ee + eb)));
            const double e1 = 2*nuPy*(cbz*ePz*tPz + 
                                      cby*(ePy*ePz + ePz*nuPy + cbz*ePy*pb)) - 
                cby*(ee*(tPz - ePz) - ePz*eb)*enu + 
                cby*ePz*enu*enu - nuPy*beta*(2*ee*ePz - 2*ee*nuPz + 
                                             2*ePz*eb + 3*ePz*enu);
            const double dnuPy = (e1 + e0/enu);

            const double jaco = enu*(beta*(-(enu*ePz*et) + ee*et*nuPz) + 
                                     cbz*(enu*ePz - ee*nuPz)*tPz + 
                                     cby*(ePz*et*nuPy - ePy*et*nuPz + 
                                          enu*ePy*tPz - ee*nuPy*tPz));
            const double mtsq = et*et - tPx*tPx - tPy*tPy - tPz*tPz;

            const double dmtsqdpb   = -2.0*(cby*tPy + cbz*tPz - beta*et);
            const double dmtsqdnuPz = 2.0*(nuPz*(ee + eb)/enu - ePz - cbz*pb);
            const double dmtsqdnuPy = 2.0*(nuPy*(ee + eb)/enu - ePy - cby*pb);

            const double dtPydpb   = cby;
            const double dtPydnuPz = 0.0;
            const double dtPydnuPy = 1.0;

            assert(enu > 0.0);
            mwsq = (ee+enu)*(ee+enu) - wPx*wPx - wPy*wPy - wPz*wPz;

            /* Fortran storage convention */
            equation_matrix[0][0] = dpb;
            equation_matrix[1][0] = dnuPz;
            equation_matrix[2][0] = dnuPy;
            equation_matrix[0][1] = dmtsqdpb;
            equation_matrix[1][1] = dmtsqdnuPz;
            equation_matrix[2][1] = dmtsqdnuPy;
            equation_matrix[0][2] = dtPydpb;
            equation_matrix[1][2] = dtPydnuPz;
            equation_matrix[2][2] = dtPydnuPy;

            b[0] = -jaco;
            b[1] = mtsqGoal - mtsq;
            b[2] = tPyGoal - tPy;

            /* Note that dgetrf_ destroys the original equation
             * matrix and stores its factors in its place
             */
            dgetrf_(&nrows1, &nrows1, &equation_matrix[0][0],
                    &nrows1, IPIV, &ifail);
            assert(ifail >= 0);
            if (ifail > 0)
            {
                /* Matrix is singular. Will deal with this
                 * case later, if it actually occurs
                 */
                assert(0);
            }
            nrows1 = 3;
            dgetrs_("N", &nrows1, &NRHS, &equation_matrix[0][0],
                    &nrows1, IPIV, b, &nrows1, &ifail, 1);
            assert(ifail == 0);

            /* Update the current values */
            pb   += b[0];
            nuPz += b[1];    
            nuPy += b[2];

            if (debug_level >= 30)
            {
                printf("w_mass_range: after iteration %d, pb = %g, nuPz = %g, nuPy = %g\n",
                       niter, pb, nuPz, nuPy);
                printf("Corresponding increments are %g, %g, and %g\n",
                       b[0], b[1], b[2]);
            }
 
            /* Break if the increments are small enough */
            if (fabs(b[0])/(fabs(pb) + 1.0) < RELATIVE_EPS_MINWMASS &&
                fabs(b[1])/(fabs(nuPz) + 1.0) < RELATIVE_EPS_MINWMASS &&
                fabs(b[2])/(fabs(nuPy) + 1.0) < RELATIVE_EPS_MINWMASS)
                break;
       }

        /* Check the number of iterations */
        if (niter >= MAX_ITERATIONS && debug_level >= 10)
        {
            static int max_warnings_iter = 1000;
            if (max_warnings_iter > 0)
            {
                printf("WARNING in w_mass_range: iteration limit exceeded.\n");
                printf("Relative solution precisions are %g %g %g, requested %g.\n",
                       fabs(b[0])/(fabs(pb) + 1.0),
                       fabs(b[1])/(fabs(nuPz) + 1.0),
                       fabs(b[2])/(fabs(nuPy) + 1.0),
                       RELATIVE_EPS_MINWMASS);
                if (--max_warnings_iter == 0)
                    printf("Further occurrences of this message "
                           "will be suppressed\n");
                fflush(stdout);
            }
        }

        if (niter < MAX_ITERATIONS && pb > 0.0 && mwsq < minWmassSq)
        {
            const v3_obj out_nuT    = sum3(mult3(xPrimeAxis,nuPx),mult3(yPrimeAxis,nuPy));

            mwmin->mwsq    = mwsq;
            mwmin->pblep   = pb;
            mwmin->bPx     = pb*i_bPx/i_Pb;
            mwmin->bPy     = pb*i_bPy/i_Pb;
            mwmin->bPz     = pb*i_bPz/i_Pb;
            mwmin->mb      = mb;
            mwmin->nux     = out_nuT.x;
            mwmin->nuy     = out_nuT.y;
            mwmin->nuz     = nuPz;
            mwmin->tlepz   = mwmin->bPz + mwmin->nuz + ePz;
            mwmin->mt      = mt;
            mwmin->fail    = 0;

            minWmassSq = mwsq;
        }
    }

    if (minWmassSq < mwsq_start)
        /* This is probably the real minimum */
        return 1;

    /* At this point we are in trouble. Will attempt to
     * find the minimum W mass point by trial and error.
     */
    if (debug_level >= 10)
    {
        static int max_warnings_noconv = 1000;
        if (max_warnings_noconv > 0)
        {
            printf("WARNING in w_mass_range: linear iterations "
                   "did not converge to a true minimum.\n");
            printf("Switching to bisections.\n");
            if (--max_warnings_noconv == 0)
                printf("Further occurrences of this message "
                       "will be suppressed\n");
            fflush(stdout);
        }
    }
    {
        double lower_mwsq = 0.0;
        double upper_mwsq = mwsq_start;
        int nsolsupper = nlsols;
        lepton_side_solution pbupper[2];

        memcpy(pbupper, pbinit, 2*sizeof(lepton_side_solution));

        if (startpoint > 0)
            lower_mwsq = initial_mw_points[startpoint-1]*
                         initial_mw_points[startpoint-1];

        while (upper_mwsq - lower_mwsq > 0.01)
        {
            double halfmwsq = 0.5*(lower_mwsq + upper_mwsq);
            nlsols = solve_leptonic_side(
                tPx, tPyGoal, ePx, ePy, ePz, 0.0, i_bPt, i_bPz,
                mt, mb, halfmwsq, debug_level, 0, pbinit);
            if (nlsols > 0)
            {
                nsolsupper = nlsols;
                upper_mwsq = halfmwsq;
                memcpy(pbupper, pbinit, 2*sizeof(lepton_side_solution));
            }
            else
                lower_mwsq = halfmwsq;
        }

        mwmin->mwsq = upper_mwsq;
        mwmin->mt   = mt;
        mwmin->mb   = mb;
        mwmin->fail = 0;
        if (nsolsupper == 1)
        {
            mwmin->pblep   = pbupper->pblep;
            mwmin->nux     = pbupper->nux;
            mwmin->nuy     = pbupper->nuy;
            mwmin->nuz     = pbupper->nuz;
            mwmin->tlepz   = pbupper->tlepz;
        }
        else if (nsolsupper == 2)
        {
            /* The solutions should be mighty close ... */
            if (2.0*fabs(pbupper[0].pblep - pbupper[1].pblep)/
                   (pbupper[0].pblep + pbupper[1].pblep) < 0.01)
            {
                mwmin->pblep   = 0.5*(pbupper[0].pblep + pbupper[1].pblep);
                mwmin->nux     = 0.5*(pbupper[0].nux   + pbupper[1].nux);
                mwmin->nuy     = 0.5*(pbupper[0].nuy   + pbupper[1].nuy);
                mwmin->nuz     = 0.5*(pbupper[0].nuz   + pbupper[1].nuz);
                mwmin->tlepz   = 0.5*(pbupper[0].tlepz + pbupper[1].tlepz);
            }
            else
            {
                /* Duh. This happens. Just choose the solution
                 * which has closer b momentum to the original.
                 */
                double cl0 = pb_distance(pbupper[0].pblep, i_Pb);
                double cl1 = pb_distance(pbupper[1].pblep, i_Pb);
                int index = cl0 < cl1 ? 0 : 1;
                mwmin->pblep   = pbupper[index].pblep;
                mwmin->nux     = pbupper[index].nux;
                mwmin->nuy     = pbupper[index].nuy;
                mwmin->nuz     = pbupper[index].nuz;
                mwmin->tlepz   = pbupper[index].tlepz;
            }
        }
        else
            assert(0);

        /* Transform the solution back into
         * the original coordinate system
         */
        {
            const v3_obj out_nuT = sum3(mult3(xPrimeAxis,mwmin->nux),
                                        mult3(yPrimeAxis,mwmin->nuy));

            mwmin->bPx = mwmin->pblep*i_bPx/i_Pb;
            mwmin->bPy = mwmin->pblep*i_bPy/i_Pb;
            mwmin->bPz = mwmin->pblep*i_bPz/i_Pb;
            mwmin->nux = out_nuT.x;
            mwmin->nuy = out_nuT.y;
        }
        return 1;
    }
}

int solve_leptonic_byMW_approx(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double beta, const int debug_level,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4])
{
    const double pl         = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double clx        = lPx/pl;
    const double cly        = lPy/pl;
    const double clz        = lPz/pl;

    const double bmag       = sqrt(bPx*bPx + bPy*bPy + bPz*bPz);
    const double cbx        = bPx/bmag;
    const double cby        = bPy/bmag;
    const double cbz        = bPz/bmag;

    const double betasq     = beta*beta;
    const double cblt       = cbx*clx + cby*cly;
    const double sbzsq      = cbx*cbx + cby*cby;
    const double topPbt     = cbx*topPx + cby*topPy;
    const double topPlt     = clx*topPx + cly*topPy;
    const double slzsq      = 1.0 - clz*clz;
    const double t1         = mwsq + topPx*topPx + topPy*topPy;
    const double mwsqoverpl = mwsq/pl;
    const double t2         = mwsqoverpl/2.0 + topPlt;
    const double clzsq      = clz*clz;
    const double cbltsq     = cblt*cblt;
    const double cbzsq      = cbz*cbz;
    const double sbzquad    = sbzsq*sbzsq;
    const double mwsqoverplsq = mwsqoverpl*mwsqoverpl;
    const double topPbtsq   = topPbt*topPbt;
    const double mdiffsq    = mt*mt - mb*mb - mwsq;
    const double mdiffsqsq  = mdiffsq*mdiffsq;
    const double t3         = cbz*mwsqoverpl - 2.0*clz*topPbt;
    const double t3sq       = t3*t3;

    const double c4b0 = -4*(cbltsq + clzsq*sbzsq);
    const double c4b1 = 8*(cblt + cbz*clz)*sbzsq;
    const double c4b2 = 4*(cbltsq*cbzsq - cbz*(cbz + 2*cblt*clz)*
                           sbzsq - sbzquad*slzsq);
    const double c3b0 = 8*(cblt*t2 + clzsq*topPbt);
    const double c3b1 = -8*(sbzsq*t2 + (cblt + 2*cbz*clz)*topPbt);
    const double c3b2 = 8*(-((-(cbz*clz) + cbzsq*(cblt + cbz*clz))*t2) + 
                        (1 + cblt*cbz*clz - clzsq*sbzsq)*topPbt);
    const double c2b0 = -mwsqoverplsq - 4*(clzsq*t1 + topPlt*
                                           (mwsqoverpl + topPlt));
    const double c2b1 = 4*(-(cblt*mdiffsq) + mwsqoverpl*topPbt) + 
                        8*(cbz*clz*t1 + topPbt*topPlt);
    const double c2b2 = 4*mdiffsq*(cblt*cbz*clz + sbzsq*slzsq) + 
                        t3sq - 4*topPbtsq + 
                        4*cbzsq*topPlt*(mwsqoverpl + topPlt) - 
                        4*cbz*(cbz*t1 + 2*clz*topPbt*topPlt);
    const double c1b1 = 2*mdiffsq*(mwsqoverpl + 2*topPlt);
    const double c1b2 = 2*mdiffsq*(-(cbz*clz*mwsqoverpl) + 
                        2*(-(slzsq*topPbt) - cbz*clz*topPlt));
    const double c0b2 = -(mdiffsqsq*slzsq);

    const double d0 = betasq*c0b2;
    const double d1 = betasq*c1b2 + beta*c1b1;
    const double d2 = betasq*c2b2 + beta*c2b1 + c2b0;
    const double d3 = betasq*c3b2 + beta*c3b1 + c3b0;
    const double d4 = betasq*c4b2 + beta*c4b1 + c4b0;

    int i, npb_try, npb = 0, failflag[4] = {0};
    double pbapprox_try[4], pbapprox[4] = {0}, nuzlist[4] = {0};

    /* Check various assumptions */
    assert(pl > 0.0);
    assert(bmag > 0.0);
    assert(mt > 0.0);
    assert(mb >= 0.0);
    assert(mwsq > 0.0);
    assert((mt - mb)*(mt - mb) > mwsq);
    assert(beta > 0.0 && beta <= 1.0);

    /* Reset the output */
    memset(solutions, 0, 4*sizeof(lepton_side_solution));

    /* Solve the quartic equation */
    npb_try = positive_quartic_roots(d4, d3, d2, d1, d0, pbapprox_try);

    /* Check that the top Pz is within limits */
    for (i=0; i<npb_try; ++i)
    {
        const double pbthis  = pbapprox_try[i];
        const double newbeta = pbthis/sqrt(pbthis*pbthis + mb*mb);
        const double denom   = newbeta*cbz - clz;
        const double pwx     = topPx - pbthis*cbx;
        const double pwy     = topPy - pbthis*cby;
        const double pwz     = ((lPx*pwx + lPy*pwy + mwsq/2.0)/pl - 
                                newbeta*(mdiffsq/pbthis/2.0 + cbx*pwx + cby*pwy))/denom;
        const double topPz   = pwz + pbthis*cbz;
        assert(denom != 0.0);
        if (minTopPz < topPz && topPz < maxTopPz)
        {
            pbapprox[npb] = pbthis;
            nuzlist[npb]  = pwz - lPz;
            ++npb;
        }
    }

    return fill_leptonic_solutions(
        cbx, cby, cbz, lPx, lPy, lPz, topPx, topPy,
        mt, mb, mwsq, nuzlist, pbapprox, failflag,
        npb, solutions);
}

int solve_leptonic_side(const double topPx, const double topPy,
                        const double lPx, const double lPy, const double lPz,
                        const double bPx, const double bPy, const double bPz,
                        const double mt, const double mb_in, const double mwsq,
                        const int debug_level, const size_t i_max_iterations,
                        lepton_side_solution solutions[2])
{
    /* This function assumes massless b quark */
    const double pl    = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double clx   = lPx/pl;
    const double cly   = lPy/pl;
    const double clz   = lPz/pl;
    const double clxsq = clx*clx;
    const double clysq = cly*cly;
    const double cltsq = clxsq + clysq;

    const double bmag  = sqrt(bPx*bPx + bPy*bPy + bPz*bPz);
    const double cbx   = bPx/bmag;
    const double cby   = bPy/bmag;
    const double cbz   = bPz/bmag;
    const double cbxsq = cbx*cbx;
    const double cbysq = cby*cby;
    const double cbtsq = cbxsq + cbysq;

    const double mwsqoverpl   = mwsq/pl;
    const double mwsqoverplsq = mwsqoverpl*mwsqoverpl;
    const double mdiffsq      = mt*mt - mwsq;
    const double tmp0         = (cbz - clz)*(cbz - clz);
    const double tmp1         = cbz*clz - 1.0;
    const double topPxsq      = topPx*topPx;
    const double topPysq      = topPy*topPy;

    const double d0    = -cltsq*mdiffsq*mdiffsq;
    const double d1    = -2.0*mdiffsq*(tmp1*mwsqoverpl + 
                         2.0*(cbx*cltsq*topPx + clx*tmp1*topPx + 
                         (cby*cltsq + cly*tmp1)*topPy));
    const double d2    = 
        4.0*cbysq*clxsq*mdiffsq - 4.0*cby*cly*mdiffsq + 4.0*cbysq*clysq*mdiffsq + 
        4.0*cby*cbz*cly*clz*mdiffsq - 8.0*mwsq + 4.0*cbysq*mwsq + 4.0*cltsq*mwsq + 
        8.0*cbz*clz*mwsq - cbysq*mwsqoverplsq - 4.0*cbysq*clx*mwsqoverpl*topPx - 
        8.0*topPxsq + 4.0*cbysq*topPxsq + 4.0*cltsq*topPxsq - 4.0*cbysq*clxsq*topPxsq + 
        8.0*cbz*clz*topPxsq + 4.0*cby*mwsqoverpl*topPy - 4.0*cbysq*cly*mwsqoverpl*topPy - 
        4.0*cby*cbz*clz*mwsqoverpl*topPy + 8.0*cby*clx*topPx*topPy - 
        8.0*cbysq*clx*cly*topPx*topPy - 8.0*cby*cbz*clx*clz*topPx*topPy + 
        4.0*cbx*(clx*tmp1*(mdiffsq - 2.0*topPxsq) + 
               topPx*(mwsqoverpl - cbz*clz*mwsqoverpl - 
                      2.0*(cby*cltsq + cly*tmp1)*topPy)) - 8.0*topPysq + 
        4.0*cbysq*topPysq + 4.0*cltsq*topPysq - 4.0*cbysq*clxsq*topPysq + 
        8.0*cby*cly*topPysq - 8.0*cbysq*clysq*topPysq + 8.0*cbz*clz*topPysq - 
        8.0*cby*cbz*cly*clz*topPysq + cbxsq*(4.0*mwsq - mwsqoverplsq - 
        4.0*clx*mwsqoverpl*topPx + 4.0*clxsq*(mdiffsq - 2.0*topPxsq) + 4.0*topPxsq - 
        4.0*cly*mwsqoverpl*topPy - 8.0*clx*cly*topPx*topPy + 
        4.0*clysq*(mdiffsq - topPxsq - topPysq) + 4.0*topPysq);
    const double d3    = 4.0*(cbysq*tmp1*(mwsqoverpl + 
                         2.0*clx*topPx + 4.0*cly*topPy) + 
                         cbxsq*((-1.0 + cby*cly + cbz*clz)*mwsqoverpl + 
                         2.0*(clx*(-2.0 + cby*cly + 2.0*cbz*clz)*topPx + 
                         (cby*clxsq - cly + 2.0*cby*clysq + cbz*cly*clz)*topPy)) + 
                         cby*(cbysq*(cly*(mwsqoverpl + 2.0*clx*topPx) + 
                         2.0*(clxsq + 2.0*clysq)*topPy) + 
                         2.0*(tmp0*topPy + cbx*tmp1*
                         (cly*topPx + clx*topPy))) + 
                         cbx*(2.0*tmp0*topPx + cbysq*(clx*mwsqoverpl + 
                         4.0*clxsq*topPx + 2.0*clysq*topPx + 
                         2.0*clx*cly*topPy) + cbxsq*(2.0*(2.0*clxsq + clysq)*topPx + 
                         clx*(mwsqoverpl + 2.0*cly*topPy))));
    const double d4    = -4.0*cbtsq*(2.0 - 2.0*cbx*clx - clxsq - 
                         2.0*cby*cly + 2.0*cbx*cby*clx*cly - clysq + 
                         cbxsq*(-1.0 + 2.0*clxsq + clysq) + 
                         cbysq*(-1.0 + clxsq + 2.0*clysq) - 2.0*cbz*clz + 
                         2.0*cbx*cbz*clx*clz + 2.0*cby*cbz*cly*clz);

    int i, npb;
    double pbapprox[4];
    d_d_pair pb_and_nuz[2];

    /* Check various assumptions */
    assert(pl > 0.0);
    assert(bmag > 0.0);
    assert(mt > 0.0);
    assert(mwsq > 0.0);
    assert(mt*mt > mwsq);

    /* Reset the output */
    memset(solutions, 0, 2*sizeof(lepton_side_solution));

    /* This function only works for massless b */
    if (mb_in != 0.0 && debug_level > 0)
    {
        printf("WARNING in solve_leptonic_side: mass of the b quark is ignored!\n");
        fflush(stdout);
    }

    /* Solve the quartic equation */
    npb = positive_quartic_roots(d4, d3, d2, d1, d0, pbapprox);

    /* Return now if there are no solutions */
    if (npb == 0)
        return 0;

    /* Use just the two solutions which are closer to the original b */
    if (npb > 2)
    {
        d_d_pair pb_and_distance[4];
        for (i=0; i<npb; ++i)
        {
            pb_and_distance[i].d1 = pbapprox[i];
            pb_and_distance[i].d2 = pb_distance(pbapprox[i], bmag);
        }
        qsort(pb_and_distance, npb, sizeof(d_d_pair),
              sort_d_d_pair_by_d2_incr);
        for (i=0; i<npb; ++i)
            pbapprox[i] = pb_and_distance[i].d1;

        if (debug_level >= 10)
        {
            static int max_warnings_manysol = 1000;
            if (max_warnings_manysol > 0)
            {
                printf("WARNING in solve_leptonic_side: "
                       "%d solutions found, using just two\n", npb);
                if (--max_warnings_manysol == 0)
                    printf("Further occurrences of this message "
                           "will be suppressed\n");
                fflush(stdout);
            }
        }
    }

    /* Calculate the nuz values */
    for (i=0; i<npb && i<2; ++i)
    {
        const double pbthis = pbapprox[i];
        const double denom  = cbz - clz;
        const double pwx    = topPx - pbthis*cbx;
        const double pwy    = topPy - pbthis*cby;
        const double pwz    = (clx*pwx + cly*pwy + mwsq/pl/2.0 -
                               (mdiffsq/pbthis/2.0 + cbx*pwx + cby*pwy))/denom;
        assert(denom != 0.0);
        pb_and_nuz[i].d1 = pbthis;
        pb_and_nuz[i].d2 = pwz - lPz;
    }

    /* Arrange solutions in the order of increasing nuz */
    if (npb > 1)
        qsort(pb_and_nuz, 2, sizeof(d_d_pair),
              sort_d_d_pair_by_d2_incr);

    /* Fill out the solution structures */
    {
        double pbvalues[2] = {0}, pnuzvalues[2] = {0};
        int isol, failflag[2] = {0};
        for (isol=0; isol<2 && isol<npb; ++isol)
        {
            pbvalues[isol] = pb_and_nuz[isol].d1;
            pnuzvalues[isol] = pb_and_nuz[isol].d2;
        }
        return fill_leptonic_solutions(
            cbx, cby, cbz, lPx, lPy, lPz, topPx, topPy,
            mt, 0.0, mwsq, pnuzvalues, pbvalues, failflag,
            npb < 2 ? npb : 2, solutions);
    }
}

int solve_leptonic_byNuPz(const double i_topPx, const double i_topPy,
                          const double i_lPx, const double i_lPy, const double ePz,
                          const double i_bPx, const double i_bPy, const double i_bPz,
                          const double mt, const double mb_in, const double nuPz,
                          const int debug_level, const size_t i_max_iterations,
                          lepton_side_solution solutions[4])
{
    /* This function no longer assumes massless b quark.
     * It is convenient to rotate the coordinate system so that
     * the b direction lies in the yz plane. Then the x component
     * of the neutrino momentum can be found immediately, and all
     * subsequent calculations are simplified.
     */
    const v3_obj zAxis      = v3(0.0, 0.0, 1.0);
    const v3_obj yPrimeAxis = direction3(v3(i_bPx, i_bPy, 0.0));
    const v3_obj xPrimeAxis = vprod3(yPrimeAxis, zAxis);

    /* Calculate various things in the new coordinate system */
    const v3_obj i_topPt = v3(i_topPx, i_topPy, 0.0);
    const v3_obj i_l     = v3(i_lPx, i_lPy, ePz);
    const double tPx     = sprod3(xPrimeAxis, i_topPt);
    const double tPy     = sprod3(yPrimeAxis, i_topPt);
    const double ePx     = sprod3(xPrimeAxis, i_l);
    const double ePy     = sprod3(yPrimeAxis, i_l);
    const double ee      = mom(i_l);
    const double i_bPt   = hypot(i_bPx, i_bPy);
    const double i_Pb    = hypot(i_bPt, i_bPz);
    const double cby     = i_bPt/i_Pb;
    const double cbz     = i_bPz/i_Pb;
    const double nuPx    = tPx - ePx;
    const double mtsq    = mt*mt;
    const double nuPxsq  = nuPx*nuPx;
    const double nuPzsq  = nuPz*nuPz;
    const double tPxsq   = tPx*tPx;
    const double tPysq   = tPy*tPy;
    const double ePxsq   = ePx*ePx;
    const double ePysq   = ePy*ePy;
    const double ePzsq   = ePz*ePz;
    const double cbysq   = cby*cby;
    const double cbzsq   = cbz*cbz;
    double beta, coeffs[5][5], d[5];
    double pbapprox[8] = {0.0};
    int deg, npb = 0;

    /* Check various assumptions */
    assert(ee > 0.0);
    assert(i_bPt > 0.0);
    assert(mt > 0.0);

    /* Reset the output */
    memset(solutions, 0, 4*sizeof(lepton_side_solution));

    /* Process the special case of zero bPz */
    if (cbz == 0.0)
    {
        const double tPz   = ePz + nuPz;
        const double topE  = sqrt(tPxsq + tPysq + tPz*tPz + mtsq);
        const double nubE  = topE - ee;
        const double nubPy = tPy - ePy;

        if (mb_in == 0.0)
        {
            const double p = (nubE*nubE-nubPy*nubPy-nuPxsq-nuPzsq)/(nubE-nubPy)/2.0;
            assert(p > 0.0);
            assert(p < nubE);
            pbapprox[npb++] = p;
        }
        else
        {
            double roots[2];
            int i, nroots;

            const double mbsq = mb_in*mb_in;
            const double tmp = nubPy*nubPy + nuPxsq + nuPzsq;
            const double tmp2 = tmp - nubE*nubE;
            const double a = 4.0*(nubE*nubE - nubPy*nubPy);
            const double b = 4.0*nubPy*(tmp2 - mbsq);
            const double c = 2.0*mbsq*(tmp + nubE*nubE) - mbsq*mbsq - tmp2*tmp2;

            assert(mb_in > 0.0);
            assert(a != 0.0);

            nroots = solve_quadratic(b/a, c/a, roots, roots+1);
            for (i=0; i<nroots; ++i)
            {
                const double p = roots[i];
                if (p > 0.0)
                    if (tmp2 - mbsq - 2.0*nubPy*p < 0.0)
                        pbapprox[npb++] = p;
            }
        }
    }
    else
    {
        /* Fill out the equation coefficients.
         * coeffs[i][j] is the coefficient for pb^i*beta^j.
         */
        coeffs[0][0] = 0.0;
        coeffs[0][1] = 0.0;
        coeffs[0][2] = 0.0;
        coeffs[0][3] = 0.0;
        coeffs[0][4] = -ePxsq*ePxsq + 2*ePxsq*mtsq - mtsq*mtsq - 
            nuPxsq*nuPxsq + 2*ePxsq*nuPxsq + 
            4*ePzsq*nuPxsq + 2*mtsq*nuPxsq + 4*ePxsq*ePz*nuPz - 4*ePz*mtsq*nuPz + 
            4*ePz*nuPxsq*nuPz + 4*ePxsq*nuPzsq - tPxsq*tPxsq + 2*ePxsq*tPxsq - 
            2*mtsq*tPxsq + 2*nuPxsq*tPxsq - 4*ePz*nuPz*tPxsq + 
            4*ePysq*(ePzsq + mtsq + 2*ePz*nuPz + nuPzsq + tPxsq) - 4*ePxsq*ePy*tPy - 
            8*ePy*ePzsq*tPy - 4*ePy*mtsq*tPy + 4*ePy*nuPxsq*tPy - 
            8*ePy*ePz*nuPz*tPy - 4*ePy*tPxsq*tPy + 4*ePxsq*tPysq + 4*ePzsq*tPysq;
        coeffs[1][0] = 0.0;
        coeffs[1][1] = 0.0;
        coeffs[1][2] = 0.0;
        coeffs[1][3] = 4*ee*(-ePxsq + mtsq + nuPxsq + 2*ePz*nuPz + 2*nuPzsq + 
                            tPxsq - 2*ePy*tPy + 2*tPysq);
        coeffs[1][4] = 4*(cbz*(ePz + nuPz)*
                         (ePxsq + 2*ePysq - mtsq + nuPxsq - 2*ePz*nuPz - tPxsq - 2*ePy*tPy) + 
                         cby*(ePy - tPy)*(ePxsq + 2*ePzsq + mtsq - nuPxsq + 2*ePz*nuPz + 
                                          tPxsq + 2*ePy*tPy));
        coeffs[2][0] = 0.0;
        coeffs[2][1] = 0.0;
        coeffs[2][2] = 2*(-3*ePxsq - 2*ePysq - 2*ePzsq + mtsq + nuPxsq + 
                         2*ePz*nuPz + 2*nuPzsq + tPxsq - 2*ePy*tPy + 2*tPysq);
        coeffs[2][3] = 8*ee*(cbz*(ePz + nuPz) + cby*(ePy - tPy));
        coeffs[2][4] = 2*(4*cby*cbz*(ePz + nuPz)*(ePy - tPy) + 
                         cbzsq*(ePxsq + 2*ePysq - 2*ePzsq - mtsq + nuPxsq - 6*ePz*nuPz - 
                                2*nuPzsq - tPxsq - 2*ePy*tPy) + 
                         cbysq*(ePxsq - 2*ePysq + 2*ePzsq + mtsq - nuPxsq + 2*ePz*nuPz + 
                                tPxsq + 6*ePy*tPy - 2*tPysq));
        coeffs[3][0] = 0.0;
        coeffs[3][1] = -4*ee;
        coeffs[3][2] = 4*(cbz*(ePz + nuPz) + cby*(ePy - tPy));
        coeffs[3][3] = 4*ee;
        coeffs[3][4] = 4*(cbysq - cbzsq)*(cbz*(ePz + nuPz) + cby*(-ePy + tPy));
        coeffs[4][0] = -1.0;
        coeffs[4][1] = 0.0;
        coeffs[4][2] = 2.0;
        coeffs[4][3] = 0.0;
        coeffs[4][4] = -(cbysq - cbzsq)*(cbysq - cbzsq);
    
        if (mb_in == 0.0)
        {
            /* Solve the quartic equation for beta = 1 (massless b quark) */
            beta = 1.0;
            for (deg=0; deg<5; ++deg)
                d[deg] = beta*(beta*(beta*(beta*coeffs[deg][4]+coeffs[deg][3])+
                                     coeffs[deg][2])+coeffs[deg][1])+coeffs[deg][0];
            {
                double quartic_roots[4];
                int iroot, nroots;
    
                nroots = positive_quartic_roots(d[4], d[3], d[2], d[1], d[0], quartic_roots);
                for (iroot = 0; iroot < nroots; ++iroot)
                {
                    /* Check that this solution is not fake */
                    const double pbthis = quartic_roots[iroot];
                    const double tPz    = cbz*pbthis + nuPz + ePz;
                    const double eb     = pbthis/beta;
                    const double nuPy   = tPy - cby*pbthis - ePy;
                    const double enusq  = nuPxsq + nuPy*nuPy + nuPzsq;
                    if (mtsq + tPxsq + tPysq + tPz*tPz - eb*eb - 
                        ee*ee - enusq - 2.0*eb*ee > 0.0)
                        pbapprox[npb++] = pbthis;
                }
            }
        }
        else
        {
            const double mbsq = mb_in*mb_in;
            const double mbquad = mbsq*mbsq;
            const double c40 = coeffs[4][0];
            const double c42 = coeffs[4][2];
            const double c44 = coeffs[4][4];
            const double c32 = coeffs[3][2];
            const double c34 = coeffs[3][4];
            const double c22 = coeffs[2][2];
            const double c24 = coeffs[2][4];
            const double c13 = coeffs[1][3];
            const double c31 = coeffs[3][1];
            const double c14 = coeffs[1][4];
            const double c23 = coeffs[2][3];
            const double c04 = coeffs[0][4];
            const double sqrp8 = c40 + c42 + c44;
            const double sum3234 = c32 + c34;
            const double tmp1 = c22 + c24 + (2*c40 + c42)*mbsq;
            double poly_coeffs[9], poly_solutions[8];
            unsigned isol, n_poly_solutions = 0;
    
            assert(mb_in > 0.0);
    
            poly_coeffs[0] = sqrp8*sqrp8;
            poly_coeffs[1] = 2*sum3234*sqrp8;
            poly_coeffs[2] = sum3234*sum3234 + 2*sqrp8*tmp1;
            poly_coeffs[3] = 2*((c22 + c24)*sum3234 + c14*sqrp8 +
                             (3*c32*c40 + 2*c34*c40 + 2*c32*c42 + c34*c42 + c32*c44)*mbsq);
            poly_coeffs[4] = c22*c22 - c23*c23 + c24*c24 + 2*c14*sum3234 + 2*c04*sqrp8 +
                             (2*c32*c32 + 2*c32*c34 + 4*c24*c40 + 2*c24*c42)*mbsq + 
                             (6*c40*c40 + 6*c40*c42 + c42*c42 + 2*c40*c44)*mbquad + 
                             2*c22*(c24 + (3*c40 + 2*c42 + c44)*mbsq);
            poly_coeffs[5] = 2*(c04*sum3234 - c13*c23 + 
                             (2*c22*c32 - c23*c31 + c24*c32 + c22*c34)*mbsq + 
                             (3*c32*c40 + c34*c40 + c32*c42)*mbquad + c14*tmp1);
            poly_coeffs[6] = c14*c14 - c13*c13 + 2*c04*(c22 + c24) +
                             mbsq*(2*c22*c22 - c23*c23 + 2*c22*c24 - 2*c13*c31 + 
                                   2*c14*c32 + 4*c04*c40 + 2*c04*c42) +
                             mbquad*(c32*c32 - c31*c31 + 6*c22*c40 + 2*c24*c40 + 2*c22*c42) + 
                             mbsq*mbquad*2*c40*(2*c40 + c42);
            poly_coeffs[7] = 2*(c04*(c14 + c32*mbsq) + 
                                mbsq*(c14*(c22 + c40*mbsq) - c13*c23 + 
                                      mbsq*(c22*c32 - c23*c31 + c32*c40*mbsq)));
            poly_coeffs[8] = c04*(c04 + 2*mbsq*(c22 + c40*mbsq)) + 
                             mbsq*(-c13*c13 - 2*c13*c31*mbsq + 
                             mbsq*(c22*c22 + (2*c22*c40 - c31*c31)*mbsq + c40*c40*mbquad));

            /* Solve the polynomial equation here */
            {
                int i, deg, nroots;
                double zeror[8], zeroi[8];

                for (i=0, deg=8; i<8; ++i, --deg)
                    if (poly_coeffs[i])
                        break;
                assert(deg > 0);

                nroots = rpoly(poly_coeffs+i, deg, zeror, zeroi);
                assert(nroots >= 0);
                assert(nroots <= 8);

                for (i=0; i<nroots; ++i)
                    if (zeroi[i] == 0.0)
                        poly_solutions[n_poly_solutions++] = zeror[i];
            }

            /* Filter out incorrect solutions */
            for (isol=0; isol<n_poly_solutions; ++isol)
            {
                const double p = poly_solutions[isol];
                if (p > 0.0)
                {
                    const double ebsq = p*p + mbsq;
                    const double poly1 = c04 + p*(c14 + p*(c24 + p*(c34 + p*c44))) +
                                         ((c22 + p*(c32 + c42*p)) + c40*ebsq)*ebsq;
                    const double poly2 = c13 + c23*p + c31*mbsq;
                    if (poly1*poly2 < 0.0)
                        pbapprox[npb++] = p;
                }
            }
    
            if (npb > 4)
            {
                d_d_pair sol_set[8];
                int i;
                for (i=0; i<npb; ++i)
                {
                    sol_set[i].d1 = pbapprox[i];
                    sol_set[i].d2 = pb_distance(pbapprox[i], i_Pb);
                }
                qsort(sol_set, npb, sizeof(d_d_pair),
                      sort_d_d_pair_by_d2_incr);
                for (i=0; i<4; ++i)
                    pbapprox[i] = sol_set[i].d1;
                if (debug_level > 0)
                {
                    printf("WARNING in solve_leptonic_byNuPz: %d solutions\n", npb);
                    fflush(stdout);
                }
                npb = 4;
            }
        }
    }

    if (npb > 0)
    {
        /* Fill out the solution structures. They will be
         * arranged in the increasing mwsq order.
         */
        int failflag[4] = {0, 0, 0, 0};
        const v3_obj bdir  = direction3(v3(i_bPx, i_bPy, i_bPz));
        return fill_leptonic_solutions(
            bdir.x, bdir.y, bdir.z, i_lPx, i_lPy, ePz, i_topPx,
            i_topPy, mt, mb_in, -1.0, &nuPz, pbapprox, failflag,
            npb, solutions);
    }
    else
        return 0;
}

int solve_leptonic_byMWsq(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx_in, const double bPy_in, const double bPz_in,
    const double mwsq, const double mb, const double pb,
    const int debug_level, const size_t max_iterations,
    lepton_side_solution solutions[2])
{
    const double bmag   = sqrt(bPx_in*bPx_in + bPy_in*bPy_in + bPz_in*bPz_in);
    const double cbx    = bPx_in/bmag;
    const double cby    = bPy_in/bmag;
    const double cbz    = bPz_in/bmag;
    const double bPx    = pb*cbx;
    const double bPy    = pb*cby;
    const double bPz    = pb*cbz;
    const double eesq   = lPx*lPx + lPy*lPy + lPz*lPz;
    const double pwx    = topPx - bPx;
    const double pwy    = topPy - bPy;
    const double pwtsq  = pwx*pwx + pwy*pwy;
    const double pnux   = pwx - lPx;
    const double pnuy   = pwy - lPy;
    const double pnutsq = pnux*pnux + pnuy*pnuy;
    const double tmp    = lPz*lPz + mwsq - pnutsq + pwtsq;
    const double a      = 4.0*(lPz*lPz - eesq);
    const double b      = 4.0*lPz*(tmp - eesq);
    const double c      = eesq*eesq + tmp*tmp - 2.0*eesq*(tmp + 2.0*pnutsq);

    int i, nsols, nsolstmp;
    double pnuz[2], pnuztmp[2], ee;

    /* Check various assumptions */
    assert(mwsq >= 0.0);
    assert(bmag > 0.0);
    assert(a != 0.0);

    /* Reset the output */
    memset(solutions, 0, 2*sizeof(lepton_side_solution));

    /* Solve the quadratic equation */
    nsolstmp = solve_quadratic(b/a, c/a, pnuztmp+0, pnuztmp+1);

    /* Filter out fantom solutions */
    nsols = 0;
    for (i=0; i<nsolstmp; ++i)
        if (pwtsq + lPz*lPz + 2.0*lPz*pnuztmp[i] + mwsq - eesq - pnutsq >= 0.0)
            pnuz[nsols++] = pnuztmp[i];

    if (nsols == 0)
        return 0;

    /* Fill out the result structure */
    ee = sqrt(eesq);
    for (i=0; i<nsols; ++i)
    {
        const double enu   = sqrt(pnutsq + pnuz[i]*pnuz[i]);
        const double etop  = enu + pb + ee;
        const double toppz = pnuz[i] + lPz + bPz;
        const double mtsq  = etop*etop - toppz*toppz - 
                             topPx*topPx - topPy*topPy;
        lepton_side_solution *sol = solutions+i;

	assert(mtsq >= 0.0);

        sol->pblep = pb;
        sol->bPx   = bPx;
        sol->bPy   = bPy;
        sol->bPz   = bPz;
        sol->mb    = 0.0;
        sol->nux   = pnux;
        sol->nuy   = pnuy;
        sol->nuz   = pnuz[i];
        sol->mwsq  = mwsq;
        sol->tlepz = toppz;
        sol->mt    = sqrt(mtsq);
    }

    return nsols;
}

double min_leptonic_mwsq_byPb(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx_in, const double bPy_in, const double bPz_in,
    const double mb, const double pb, lepton_side_solution *sol)
{
    const double bmag   = sqrt(bPx_in*bPx_in + bPy_in*bPy_in + bPz_in*bPz_in);
    const double cbx    = bPx_in/bmag;
    const double cby    = bPy_in/bmag;
    const double cbz    = bPz_in/bmag;
    const double bPx    = pb*cbx;
    const double bPy    = pb*cby;
    const double bPz    = pb*cbz;
    const double eb     = hypot(pb, mb);

    /* W mass is at minimum when cos(theta_nu) is the same as cos(theta_l) */
    const double ee     = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double costhetanu = lPz/ee;
    const double sinthetanu = sqrt(1.0 - costhetanu*costhetanu);
    const double pwx    = topPx - bPx;
    const double pwy    = topPy - bPy;
    const double pnux   = pwx - lPx;
    const double pnuy   = pwy - lPy;
    const double pnut   = hypot(pnux, pnuy);
    const double enu    = pnut/sinthetanu;
    const double pnuz   = enu*costhetanu;
    const double pwz    = lPz + pnuz;
    const double ew     = ee + enu;
    const double et     = ew + eb;
    const double topPz  = pwz + bPz;
    const double mtsq   = et*et - topPx*topPx - topPy*topPy - topPz*topPz;
    
    assert(sinthetanu > 0.0);
    assert(mtsq > 0.0);

    sol->pblep = pb;
    sol->bPx   = bPx;
    sol->bPy   = bPy;
    sol->bPz   = bPz;
    sol->mb    = mb;
    sol->nux   = pnux;
    sol->nuy   = pnuy;
    sol->nuz   = pnuz;
    sol->mwsq  = ew*ew - pwx*pwx - pwy*pwy - pwz*pwz;
    sol->tlepz = topPz;
    sol->mt    = sqrt(mtsq);
    sol->fail  = 0;

    assert(sol->mwsq > 0.0);
    return sol->mwsq;    
}

double min_leptonic_mt_byPb(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx_in, const double bPy_in, const double bPz_in,
    const double mb, const double pb, lepton_side_solution *sol)
{
    const double bmag   = sqrt(bPx_in*bPx_in + bPy_in*bPy_in + bPz_in*bPz_in);
    const double cbx    = bPx_in/bmag;
    const double cby    = bPy_in/bmag;
    const double cbz    = bPz_in/bmag;
    const double bPx    = pb*cbx;
    const double bPy    = pb*cby;
    const double bPz    = pb*cbz;
    const double pebz   = lPz + bPz;
    const double ee     = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double eb     = hypot(pb, mb);
    const double eeb    = ee + eb;
    const double costhetanu = pebz/eeb;
    const double sinthetanu = sqrt(1.0 - costhetanu*costhetanu);
    const double pwx    = topPx - bPx;
    const double pwy    = topPy - bPy;
    const double pnux   = pwx - lPx;
    const double pnuy   = pwy - lPy;
    const double pnut   = hypot(pnux, pnuy);
    const double enu    = pnut/sinthetanu;
    const double pnuz   = enu*costhetanu;
    const double pwz    = lPz + pnuz;
    const double ew     = ee + enu;
    const double et     = ew + eb;
    const double topPz  = pwz + bPz;
    const double mtsq   = et*et - topPx*topPx - topPy*topPy - topPz*topPz;
    
    assert(sinthetanu > 0.0);
    assert(mtsq > 0.0);

    sol->pblep = pb;
    sol->bPx   = bPx;
    sol->bPy   = bPy;
    sol->bPz   = bPz;
    sol->mb    = mb;
    sol->nux   = pnux;
    sol->nuy   = pnuy;
    sol->nuz   = pnuz;
    sol->mwsq  = ew*ew - pwx*pwx - pwy*pwy - pwz*pwz;
    sol->tlepz = topPz;
    sol->mt    = sqrt(mtsq);
    sol->fail  = 0;

    return sol->mt;
}

int solve_leptonic_byPb(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx_in, const double bPy_in, const double bPz_in,
    const double mt, const double mb, const double pb,
    const int debug_level, const size_t max_iterations,
    lepton_side_solution solutions[2])
{
    const double bmag   = sqrt(bPx_in*bPx_in + bPy_in*bPy_in + bPz_in*bPz_in);
    const double cbx    = bPx_in/bmag;
    const double cby    = bPy_in/bmag;
    const double cbz    = bPz_in/bmag;
    const double bPx    = pb*cbx;
    const double bPy    = pb*cby;
    const double bPz    = pb*cbz;
    const double pebz   = lPz + bPz;
    const double ee     = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double eb     = hypot(pb, mb);
    const double eeb    = ee + eb;
    const double pwx    = topPx - bPx;
    const double pwy    = topPy - bPy;
    const double pnux   = pwx - lPx;
    const double pnuy   = pwy - lPy;
    const double pnutsq = pnux*pnux + pnuy*pnuy;
    const double a0     = topPx*topPx + topPy*topPy + pebz*pebz + 
                          mt*mt - eeb*eeb - pnutsq;
    const double b0     = 2.0*pebz;
    const double c0     = 2.0*eeb;
    const double a      = b0*b0 - c0*c0;
    const double b      = 2.0*a0*b0/a;
    const double c      = (a0*a0 - c0*c0*pnutsq)/a;

    int i, nsols, nsolstmp;
    double pnuz[2], pnuztmp[2];

    /* Check various assumptions */
    assert(a != 0.0);
    assert(bmag > 0.0);

    /* Reset the output */
    memset(solutions, 0, 2*sizeof(lepton_side_solution));

    /* Solve the quadratic equation */
    nsolstmp = solve_quadratic(b, c, pnuztmp+0, pnuztmp+1);

    /* Filter out fantom solutions */
    nsols = 0;
    for (i=0; i<nsolstmp; ++i)
        if (a0 + b0*pnuztmp[i] >= 0.0)
            pnuz[nsols++] = pnuztmp[i];

    if (nsols == 0)
        return 0;

    /* Fill out the result structure */
    for (i=0; i<nsols; ++i)
    {
        const double ew  = sqrt(pnutsq + pnuz[i]*pnuz[i]) + ee;
        const double pwz = pnuz[i] + lPz;
        lepton_side_solution *sol = solutions+i;

        sol->pblep = pb;
        sol->bPx   = bPx;
        sol->bPy   = bPy;
        sol->bPz   = bPz;
        sol->mb    = mb;
        sol->nux   = pnux;
        sol->nuy   = pnuy;
        sol->nuz   = pnuz[i];
        sol->mwsq  = ew*ew - pwx*pwx - pwy*pwy - pwz*pwz;
        sol->tlepz = pwz + bPz;
        sol->mt    = mt;

	assert(sol->mwsq >= 0.0);
    }

    return nsols;
}

int solve_hadronic_w(const double qPx,    const double qPy,
                     const double qPz,    const double mq,
                     const double qbarPx, const double qbarPy,
                     const double qbarPz, const double mqbar,
                     const double mwsq,   const double param,
                     double *pq, double *pqbar)
{
    const double qmag    = sqrt(qPx*qPx + qPy*qPy + qPz*qPz);
    const double cqx     = qPx/qmag;
    const double cqy     = qPy/qmag;
    const double cqz     = qPz/qmag;
    
    const double qbarmag = sqrt(qbarPx*qbarPx + qbarPy*qbarPy + qbarPz*qbarPz);
    const double cqbarx  = qbarPx/qbarmag;
    const double cqbary  = qbarPy/qbarmag;
    const double cqbarz  = qbarPz/qbarmag;

    const double cqqbar  = cqx*cqbarx + cqy*cqbary + cqz*cqbarz;
    const double ratio   = exp(param);

    const int q_is_extra = fabs(qmag - 1.0) < 1.0e-3;
    const int qbar_is_extra = fabs(qbarmag - 1.0) < 1.0e-3;

    /* Check various assumptions */
    assert(qmag > 0.0);
    assert(qbarmag > 0.0);
    assert(mwsq > (mq + mqbar)*(mq + mqbar));
    assert(1.0 - cqqbar > 0.0);
    assert(pq);
    assert(pqbar);
    assert(!(q_is_extra && qbar_is_extra));

    if (mq == 0.0 && mqbar == 0.0)
    {
        /* We have it easy */
        const double product = mwsq/2.0/(1.0 - cqqbar);
        *pq    = sqrt(product * ratio);
        *pqbar = sqrt(product / ratio);
    }
    else
    {
        /* Have to solve the full equation */
        const double msqov2 = (mwsq - mq*mq - mqbar*mqbar)/2.0;
        const double a = ratio*ratio*(1.0 - cqqbar*cqqbar);
        const double b = mq*mq - 2.0*cqqbar*msqov2*ratio + mqbar*mqbar*ratio*ratio;
        const double c = mq*mq*mqbar*mqbar - msqov2*msqov2;

        double pqbarsq[2], goodsols[2];
        int i, ngood = 0;
        const int nsols = solve_quadratic(b/a, c/a, pqbarsq, pqbarsq+1);
        for (i=0; i<nsols; ++i)
            if (pqbarsq[i] > 0.0)
                goodsols[ngood++] = sqrt(pqbarsq[i]);
        switch (ngood)
        {
        case 0:
            *pq = 0.0;
            *pqbar = 0.0;
            return 0;
        case 1:
            *pqbar = goodsols[0];
            break;
        case 2:
            {
                /* Choose the "closest" solution */
                double distance[2];
                for (i=0; i<2; ++i)
                {
                    const double qdistance = q_is_extra ? 0.0 :
                        pb_distance(goodsols[i]*ratio, qmag);
                    const double qbardistance = qbar_is_extra ? 0.0 :
                        pb_distance(goodsols[i], qbarmag);
                    distance[i] = qdistance + qbardistance;
                }
                if (distance[0] < distance[1])
                    *pqbar = goodsols[0];
                else
                    *pqbar = goodsols[1];
            }
            break;
        default:
            /* This should never happen */
            assert(0);
        }
        *pq = *pqbar*ratio;
    }
    return 1;
}

int solve_hadronic_side(const double qPx,    const double qPy,
                        const double qPz,    const double mq,
                        const double qbarPx, const double qbarPy,
                        const double qbarPz, const double mqbar,
                        const double bPx,    const double bPy,
                        const double bPz,    const double mb,
                        const double mt,     const double mwsq,
                        const double param,  const int debug_level,
                        hadron_side_solution *solution)
{
    double pq, pqbar;

    assert((mt - mb)*(mt - mb) > mwsq);

    if (solve_hadronic_w(qPx, qPy, qPz, mq,
                         qbarPx, qbarPy, qbarPz, mqbar,
                         mwsq, param, &pq, &pqbar))
    {
        /* Solve for pb */
        const double qmag    = sqrt(qPx*qPx + qPy*qPy + qPz*qPz);
        const double cqx     = qPx/qmag;
        const double cqy     = qPy/qmag;
        const double cqz     = qPz/qmag;

        const double qbarmag = sqrt(qbarPx*qbarPx + qbarPy*qbarPy + qbarPz*qbarPz);
        const double cqbarx  = qbarPx/qbarmag;
        const double cqbary  = qbarPy/qbarmag;
        const double cqbarz  = qbarPz/qbarmag;

        const double bmag    = sqrt(bPx*bPx + bPy*bPy + bPz*bPz);
        const double cbx     = bPx/bmag;
        const double cby     = bPy/bmag;
        const double cbz     = bPz/bmag;

        const double wPx   = pq*cqx + pqbar*cqbarx;
        const double wPy   = pq*cqy + pqbar*cqbary;
        const double wPz   = pq*cqz + pqbar*cqbarz;

        const double pwsq  = wPx*wPx + wPy*wPy + wPz*wPz;
        const double cbwpw = cbx*wPx + cby*wPy + cbz*wPz;
        const double massd = (mt*mt - mwsq - mb*mb)/2.0;
        const double a     = pwsq + mwsq - cbwpw*cbwpw;
        const double b     = -2.0*massd*cbwpw;
        const double c     = (pwsq + mwsq)*mb*mb - massd*massd;

        const int bhad_is_extra = fabs(bmag - 1.0) < 1.0e-3;

        double pbsols[2], pgood[2];
        int i, ngood = 0, nfill;
        const int nsols = solve_quadratic(b/a, c/a, pbsols, pbsols+1);

        assert(bmag > 0.0);
        assert(a > 0.0);

        for (i=0; i<nsols; ++i)
            if (pbsols[i] > 0.0 && massd + cbwpw*pbsols[i] > 0.0)
                pgood[ngood++] = pbsols[i];
        nfill = ngood;
        switch (ngood)
        {
        case 0:
            memset(solution, 0, sizeof(hadron_side_solution));
            return 0;
        case 1:
            break;
        case 2:
            if (!bhad_is_extra)
            {
                nfill = 1;
                if (pb_distance(pgood[0],bmag) > pb_distance(pgood[1],bmag))
                    pgood[0] = pgood[1];
            }
            break;
        default:
            assert(0);
        }

        /* Fill out the solution struct */
        for (i=0; i<nfill; ++i)
        {
            const double pb = pgood[i];
            solution[i].qP     = pq;
            solution[i].qPx    = pq*cqx;
            solution[i].qPy    = pq*cqy;
            solution[i].qPz    = pq*cqz;
            solution[i].qbarP  = pqbar;
            solution[i].qbarPx = pqbar*cqbarx;
            solution[i].qbarPy = pqbar*cqbary;
            solution[i].qbarPz = pqbar*cqbarz;
            solution[i].bP     = pb;
            solution[i].bPx    = pb*cbx;
            solution[i].bPy    = pb*cby;
            solution[i].bPz    = pb*cbz;
            solution[i].mq     = mq;
            solution[i].mqbar  = mqbar;
            solution[i].mb     = mb;
            solution[i].is_valid = 1;
        }

        return nfill;
    }
    else
    {
        memset(solution, 0, sizeof(hadron_side_solution));
        return 0;
    }
}

int fill_leptonic_solutions(
    const double cbx, const double cby, const double cbz,
    const double lPx, const double lPy, const double lPz,
    const double topPx, const double topPy,
    const double mt, const double mb,
    const double mwsq_in, const double *pnuPz,
    const double *pbvalues, const int *failflag,
    const size_t nsols, lepton_side_solution *solutions)
{
    size_t isol, nsolutions = 0;

    for (isol = 0; isol < nsols; ++isol)
    {
        assert(pbvalues[isol] > 0.0);
        {
            double pwz, mwsq;
            lepton_side_solution *sptr = &solutions[nsolutions++];

            const double pb  = pbvalues[isol];
            const double pbx = pb * cbx;
            const double pby = pb * cby;
            const double pbz = pb * cbz;
            const double pwx = topPx - pbx;
            const double pwy = topPy - pby;

            if (mwsq_in < 0.0)
            {
                const double eb    = sqrt(pb*pb + mb*mb);
                const double beta  = pb/eb;
                const double ee    = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
                const double cbxsq = cbx*cbx;
                const double cbysq = cby*cby;
                const double cbtsq = cbxsq + cbysq;
                const double pwxsq = pwx*pwx;
                const double pwysq = pwy*pwy;
                const double nuPz  = *pnuPz;
                const double nux   = pwx - lPx;
                const double nuy   = pwy - lPy;
                const double enu   = sqrt(nux*nux + nuy*nuy + nuPz*nuPz);
                const double ew    = ee + enu;
                const double tmp0  = cbx*(lPx - topPx) + cby*(lPy - topPy) + cbtsq*pb;
                const double denom = beta*enu*enu + ee*tmp0 + eb*(tmp0 + beta*enu) +
                    enu*(beta*ee + tmp0 - cbz*lPz - cbz*nuPz - cbz*cbz*pb);
                assert(denom != 0.0);

                pwz   = lPz + nuPz;
                mwsq  = ew*ew - (pwxsq + pwysq + pwz*pwz);
            }
            else
            {
                pwz   = lPz + pnuPz[isol];
                mwsq  = mwsq_in;
            }

            sptr->pblep = pb;
            sptr->bPx   = pbx;
            sptr->bPy   = pby;
            sptr->bPz   = pbz;
            sptr->mb    = mb;
            sptr->nux   = pwx - lPx;
            sptr->nuy   = pwy - lPy;
            sptr->nuz   = pwz - lPz;
            sptr->mwsq  = mwsq;
            sptr->tlepz = pbz + pwz;
            sptr->mt    = mt;
            sptr->fail  = failflag[isol];
        }
    }

    if (nsolutions > 1 && mwsq_in < 0.0)
        /* Sort the solutions in the order of increasing mW */
        qsort(solutions, nsolutions, sizeof(lepton_side_solution),
              sort_lepton_side_solution_by_mwsq_incr);

    return nsolutions;
}

int variable_step_nuz_minmax(
              const double topPx, const double topPy,
              const double lPx, const double lPy, const double lPz,
              const double bPx, const double bPy, const double bPz,
              const double mt, const double mb, const int debug_level,
              const double initial_nuz, const double initial_step,
              const double precision, lepton_side_solution *sol)
{
    int nsols, nlastsols;
    double newpoint, step = initial_step;
    double oldpoint, minpoint, maxpoint;
    lepton_side_solution nusols[4];
    lepton_side_solution lastsols[4];

    assert(sol);
    assert(fabs(initial_step) > 0.0);
    assert(precision > 0.0);

    newpoint = initial_nuz;
    nlastsols = solve_leptonic_byNuPz(
        topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
        mt, mb, newpoint, debug_level, 0, lastsols);
    assert(nlastsols >= 0);
    if (nlastsols == 0)
    {
        memset(sol, 0, sizeof(lepton_side_solution));
        return 0;
    }

    do {
        oldpoint = newpoint;
        newpoint = oldpoint + step;
        step *= 2.0;
        nsols = solve_leptonic_byNuPz(
            topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
            mt, mb, newpoint, debug_level, 0, nusols);
        if (nsols > 0)
        {
            nlastsols = nsols;
            memcpy(lastsols, nusols, nsols*sizeof(lepton_side_solution));
        }
    } while (nsols > 0);

    if (initial_step > 0.0)
    {
        maxpoint = newpoint;
        minpoint = oldpoint;
    }
    else
    {
        maxpoint = oldpoint;
        minpoint = newpoint;
    }

    while (fabs(maxpoint - minpoint)/
           (1.0 + 0.5*fabs(maxpoint + minpoint)) > precision)
    {
        newpoint = 0.5*(maxpoint + minpoint);
        nsols = solve_leptonic_byNuPz(
            topPx, topPy, lPx, lPy, lPz, bPx, bPy, bPz,
            mt, mb, newpoint, debug_level, 0, nusols);
        if (nsols)
        {
            nlastsols = nsols;
            memcpy(lastsols, nusols, nsols*sizeof(lepton_side_solution));
        }
        if (initial_step > 0.0)
        {
            if (nsols)
                minpoint = newpoint;
            else
                maxpoint = newpoint;
        }
        else
        {
            if (nsols)
                maxpoint = newpoint;
            else
                minpoint = newpoint;
        }
    }

    /* Fill out the extremum info */
    sol->fail  = 0;
    if (nlastsols == 1)
    {
        assert(lastsols->mwsq > 0.0);
        memcpy(sol, lastsols, sizeof(lepton_side_solution));
    }
    else if (nlastsols == 2)
    {
        /* The solutions must be close */
        const double ave_pb  = 0.5*(lastsols[0].pblep + lastsols[1].pblep);
        const double diff_pb = lastsols[0].pblep - lastsols[1].pblep;
        const double ave_mwsq = 0.5*(lastsols[0].mwsq + lastsols[1].mwsq);

        if (debug_level > 0 && fabs(diff_pb)/(1.0 + fabs(ave_pb)) >= 100.0*precision)
        {
            printf("WARNING in variable_step_nuz_minmax: pb solutions "
                   "at the extremum are very different\n");
            fflush(stdout);
        }
        assert(ave_mwsq  > 0.0);
        sol->mwsq  = ave_mwsq;
        sol->mt    = mt;
        sol->pblep = ave_pb;
        sol->bPx   = 0.5*(lastsols[0].bPx + lastsols[1].bPx);
        sol->bPy   = 0.5*(lastsols[0].bPy + lastsols[1].bPy);
        sol->bPz   = 0.5*(lastsols[0].bPz + lastsols[1].bPz);
        sol->mb    = 0.5*(lastsols[0].mb  + lastsols[1].mb);
        sol->nux   = 0.5*(lastsols[0].nux + lastsols[1].nux);
        sol->nuy   = 0.5*(lastsols[0].nuy + lastsols[1].nuy);
        sol->nuz   = 0.5*(lastsols[0].nuz + lastsols[1].nuz);
        sol->tlepz = 0.5*(lastsols[0].tlepz + lastsols[1].tlepz);
    }
    else
        /* Don't know what to to when there are more than 2 solutions */
        assert(0);

    return 1;
}

static int positive_quartic_roots(double a, double b, double c,
                                  double d, double e, double rts[4])
{
    double A, B, C, D;
    double quartic_roots[4];
    int iroot, nroots, npb = 0;

    assert(a != 0.0);
    A = b/a;
    B = c/a;
    C = d/a;
    D = e/a;

    nroots = quartic(A, B, C, D, quartic_roots);

    for (iroot = 0; iroot < nroots; ++iroot)
        if (quartic_roots[iroot] > 0.0)
        {
            /* Check the solution validity */
            const double x0     = quartic_roots[iroot];
            const double f0     = x0*(x0*(x0*(x0 + A) + B) + C) + D;
            const double deriv1 = x0*(x0*(4.0*x0 + 3.0*A) + 2.0*B) + C;
            const double deriv2 = x0*(12.0*x0 + 6.0*A) + 2.0*B;
            const double deriv3 = 24.0*x0 + 6.0*A;
            double newroot = -1.0;

            if (deriv3 != 0.0)
            {
                /* Approximate using cubic */
                double x[4];
                const int ncubroots = cubic(
                    deriv2/deriv3*3.0, deriv1/deriv3*6.0,
                    f0/deriv3*6.0, x);
                const int require_roots = deriv1*deriv1 > 2.0*deriv2*f0 ? 1 : 3;
                if (ncubroots >= require_roots)
                {
                    double mindiff = 1.0;
                    int i;
                    for (i=0; i<ncubroots; ++i)
                    {
                        const double absdiff = fabs(x[i]);
                        if (absdiff < mindiff)
                        {
                            mindiff = absdiff;
                            newroot = x0 + x[i];
                        }
                    }
                }
            }
            else if (deriv1*deriv1 > 2.0*deriv2*f0)
            {
                /* Looks like the solution is real.
                 * Improve the precision.
                 */
                if (deriv2 == 0.0)
                    newroot = x0 - f0/deriv1;
                else
                {
                    double x1, x2;
                    if (solve_quadratic(2.0*deriv1/deriv2,
                                        2.0*f0/deriv2, &x1, &x2))
                    {
                        const double dx = fabs(x1) < fabs(x2) ? x1 : x2;
                        if (fabs(dx) < 1.0)
                            newroot = x0 + dx;
                    }
                }
            }

            if (newroot > 0.0)
                rts[npb++] = newroot;
        }

    for (iroot = npb; iroot < 4; ++iroot)
        rts[iroot] = 0.0;

    return npb;
}

double max_lepton_pt(const double wx, const double wy, const double mwsq,
                     const double lPx, const double lPy, const double lPz)
{
    const double lPt = hypot(lPx, lPy);
    const double cex = lPx/lPt;
    const double cey = lPy/lPt;
    const double a = cey*cey*(mwsq + wx*wx) + cex*cex*(mwsq + wy*wy) -
                     2.0*cex*cey*wx*wy;
    const double b = -mwsq*(cex*wx + cey*wy);
    const double c = -mwsq*mwsq/4.0;

    double pt[2], pt_viable[2];
    int i, nsols, n_viable = 0;

    assert(lPt > 0.0);

    if (fabs(a) < 1.0e-20)
    {
        if (b == 0.0)
            return 0.0;
        else
        {
            nsols = 1;
            pt[0] = -c/b;
        }
    }
    else
        nsols = solve_quadratic(b/a, c/a, pt, pt + 1);

    for (i=0; i<nsols; ++i)
    {
        const double thispt = pt[i];
        if (thispt > 0.0)
        {
            const double nux = wx - thispt*cex;
            const double nuy = wy - thispt*cey;
            if (mwsq/2.0 + thispt*cex*nux + thispt*cey*nuy > 0.0)
                pt_viable[n_viable++] = thispt;
        }
    }

    switch (n_viable)
    {
    case 0:
        return 0.0;
    case 1:
        return pt_viable[0];
    case 2:
        return pt_viable[0] > pt_viable[1] ? pt_viable[0] : pt_viable[1];
    default:
        assert(0);
    }
}

void fill_hadron_side_solution(const particle_obj q,
                               const particle_obj qbar,
                               const particle_obj b,
                               hadron_side_solution *hsol)
{
    hsol->qP     = mom(q.p);
    hsol->qPx    = q.p.x;
    hsol->qPy    = q.p.y;
    hsol->qPz    = q.p.z;
    hsol->mq     = q.m;
    hsol->qbarP  = mom(qbar.p);
    hsol->qbarPx = qbar.p.x;
    hsol->qbarPy = qbar.p.y;
    hsol->qbarPz = qbar.p.z;
    hsol->mqbar  = qbar.m;
    hsol->bP     = mom(b.p);
    hsol->bPx    = b.p.x;
    hsol->bPy    = b.p.y;
    hsol->bPz    = b.p.z;
    hsol->mb     = b.m;
    hsol->is_valid = 1;	
}

void leptonic_top_z_range(const double Ecms,
                          const double ttbarPx, const double ttbarPy,
                          const double thadPx, const double thadPy,
                          const double thadPz, const double mthad,
                          const double mtlep, double *pzmin, double *pzmax)
{
    assert(mthad > 0.0);
    assert(mtlep > 0.0);
    assert(Ecms > mthad + mtlep);
    assert(pzmin);
    assert(pzmax);
    {
        const double ehad   = sqrt(thadPx*thadPx + thadPy*thadPy +
                                   thadPz*thadPz + mthad*mthad);
        const double tlepPx = ttbarPx - thadPx;
        const double tlepPy = ttbarPy - thadPy;
        const double mttsq  = tlepPx*tlepPx + tlepPy*tlepPy + mtlep*mtlep;
        int isign;
        double sols[2];
        for (isign=-1; isign<=1; isign+=2)
        {
            const double tmp = Ecms - ehad + isign*thadPz;
            const double b   = 2*isign*tmp;
            assert(b);
            sols[(isign + 1)/2] = (mttsq - tmp*tmp)/b;
        }
        if (sols[0] < sols[1])
        {
            *pzmin = sols[0];
            *pzmax = sols[1];
        }
        else
        {
            *pzmin = sols[1];
            *pzmax = sols[0];
        }
    }
}

int solve_leptonic_side_massiveb(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double *betalist, unsigned n_beta_points,
    const int debug_level, const size_t i_max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4])
{
    const size_t max_iterations = (i_max_iterations > 0 ? 
                                   i_max_iterations : MAX_ITERATIONS);

    const double pl         = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double clx        = lPx/pl;
    const double cly        = lPy/pl;
    const double clz        = lPz/pl;

    const double bmag       = sqrt(bPx*bPx + bPy*bPy + bPz*bPz);
    const double cbx        = bPx/bmag;
    const double cby        = bPy/bmag;
    const double cbz        = bPz/bmag;

    const double cblt       = cbx*clx + cby*cly;
    const double sbzsq      = cbx*cbx + cby*cby;
    const double topPbt     = cbx*topPx + cby*topPy;
    const double topPlt     = clx*topPx + cly*topPy;
    const double slzsq      = 1.0 - clz*clz;
    const double t1         = mwsq + topPx*topPx + topPy*topPy;
    const double mwsqoverpl = mwsq/pl;
    const double t2         = mwsqoverpl/2.0 + topPlt;
    const double clzsq      = clz*clz;
    const double cbltsq     = cblt*cblt;
    const double cbzsq      = cbz*cbz;
    const double sbzquad    = sbzsq*sbzsq;
    const double mwsqoverplsq = mwsqoverpl*mwsqoverpl;
    const double topPbtsq   = topPbt*topPbt;
    const double mdiffsq    = mt*mt - mb*mb - mwsq;
    const double mdiffsqsq  = mdiffsq*mdiffsq;
    const double t3         = cbz*mwsqoverpl - 2.0*clz*topPbt;
    const double t3sq       = t3*t3;

    const double c4b0 = -4*(cbltsq + clzsq*sbzsq);
    const double c4b1 = 8*(cblt + cbz*clz)*sbzsq;
    const double c4b2 = 4*(cbltsq*cbzsq - cbz*(cbz + 2*cblt*clz)*
                           sbzsq - sbzquad*slzsq);
    const double c3b0 = 8*(cblt*t2 + clzsq*topPbt);
    const double c3b1 = -8*(sbzsq*t2 + (cblt + 2*cbz*clz)*topPbt);
    const double c3b2 = 8*(-((-(cbz*clz) + cbzsq*(cblt + cbz*clz))*t2) + 
                        (1 + cblt*cbz*clz - clzsq*sbzsq)*topPbt);
    const double c2b0 = -mwsqoverplsq - 4*(clzsq*t1 + topPlt*
                                           (mwsqoverpl + topPlt));
    const double c2b1 = 4*(-(cblt*mdiffsq) + mwsqoverpl*topPbt) + 
                        8*(cbz*clz*t1 + topPbt*topPlt);
    const double c2b2 = 4*mdiffsq*(cblt*cbz*clz + sbzsq*slzsq) + 
                        t3sq - 4*topPbtsq + 
                        4*cbzsq*topPlt*(mwsqoverpl + topPlt) - 
                        4*cbz*(cbz*t1 + 2*clz*topPbt*topPlt);
    const double c1b1 = 2*mdiffsq*(mwsqoverpl + 2*topPlt);
    const double c1b2 = 2*mdiffsq*(-(cbz*clz*mwsqoverpl) + 
                        2*(-(slzsq*topPbt) - cbz*clz*topPlt));
    const double c0b2 = -(mdiffsqsq*slzsq);

    static unsigned callcount = 0;
    static int *fastbreak = 0;
    static unsigned n_fastbreak = 0;
    double pbvalues[4] = {0};
    int failflag[4] = {0}, isol, maxnpb = 1;
    unsigned istart;

    ++callcount;

    /* Check various assumptions */
    assert(pl > 0.0);
    assert(bmag > 0.0);
    assert(mt > 0.0);
    assert(mb > 0.0);
    assert(mwsq > 0.0);
    assert((mt - mb)*(mt - mb) > mwsq);

    /* Reset the output */
    memset(solutions, 0, 4*sizeof(lepton_side_solution));

    /* Figure out the starting points */
    if (betalist == 0 || n_beta_points == 0)
    {
        betalist = start_beta;
        n_beta_points = sizeof(start_beta)/sizeof(start_beta[0]);
    }

    /* Memory for flagging bad starting points */
    get_static_memory((void **)&fastbreak, sizeof(int),
                      &n_fastbreak, n_beta_points);
    for (istart = 0; istart < n_beta_points; ++istart)
        fastbreak[istart] = 0;

    /* Cycle over solutions. We will choose the lower pb
     * solution on the first pass and higher solution
     * on the second.
     */
    for (isol = 0; isol < 2 && isol < maxnpb; ++isol)
	for (istart = 0; istart < n_beta_points && pbvalues[isol] == 0.0; ++istart)
	{
	    double relative_shift = 1.0;
	    double beta = betalist[istart];
	    unsigned niter;

	    if (fastbreak[istart])
		continue;

	    for (niter = 0; niter < max_iterations; ++niter)
	    {
		const double betasq = beta*beta;
		const double d0 = betasq*c0b2;
		const double d1 = betasq*c1b2 + beta*c1b1;
		const double d2 = betasq*c2b2 + beta*c2b1 + c2b0;
		const double d3 = betasq*c3b2 + beta*c3b1 + c3b0;
		const double d4 = betasq*c4b2 + beta*c4b1 + c4b0;

		/* Solve the quartic equation */
		double pbthis, pb[4];
                int npb = positive_quartic_roots(d4, d3, d2, d1, d0, pb);
                const int npbsave = npb;

		/* Break if there are no solutions */
		if (npb == 0)
		{
		    if (niter == 0)
		    {
			/* We don't want to use this starting beta value
			 * on the second pass
			 */
			fastbreak[istart] = 1;
		    }
		    else if (debug_level >= 10)
		    {
                        static int max_warnings_break = 1000;
                        if (max_warnings_break > 0)
                        {
                            printf("WARNING in solve_leptonic_side_massiveb call %u: "
                                   "no solutions after %d iteration(s), start %d\n",
                                   callcount, niter, istart);
                            if (--max_warnings_break == 0)
                                printf("Further occurrences of this message "
                                       "will be suppressed\n");
                            fflush(stdout);
                        }
		    }
		    pbvalues[isol] = 0.0;
		    break;
		}

		/* We should not normally have more than two solutions */
		if (npb > 2)
                {
                    /* If this happens, choose the two solutions
                     * which provide better match to the pb
                     * calculated with the assumed beta
                     */
                    int ib;
                    d_d_pair tosort[4];

                    for (ib=0; ib<npb; ++ib)
                    {
                        tosort[ib].d1 = pb[ib];
                        if (beta < 1.0)
                        {
                            const double pb0 = mb*beta/sqrt(1.0 - beta*beta);
                            tosort[ib].d2 = pb_distance(pb[ib], pb0);
                        }
                        else
                        {
                            /* Just select the two highest solutions */
                            tosort[ib].d2 = -pb[ib];
                        }
                    }
                    qsort(tosort, npb, sizeof(d_d_pair),
                          sort_d_d_pair_by_d2_incr);
                    pb[0] = tosort[0].d1;
                    pb[1] = tosort[1].d1;
                    npb = 2;
                }
                if (npb > maxnpb)
                    maxnpb = npb;

		/* Arrange solutions in the increasing order */
		if (npb == 2)
		    if (pb[0] > pb[1])
		    {
			const double dtmp = pb[0];
			pb[0] = pb[1];
			pb[1] = dtmp;
		    }

		/* Store the relevant solution in the variable
		   which is defined outside the loop */
		pbthis = isol ? pb[npb-1] : pb[0];
		relative_shift = 2.0*(pbthis - pbvalues[isol])/
		    (pbthis + pbvalues[isol]);
		pbvalues[isol] = pbthis;

		/* Break if the precision is good enough */
		if (fabs(relative_shift) < RELATIVE_EPS_LEPTONIC)
                {
                    /* Warn if we are breaking out of
                     * the equation with more than two solutions
                     */
                    if (npbsave > 2 && debug_level >= 10)
                    {
                        static int max_warnings_4sols = 1000;
                        if (max_warnings_4sols > 0)
                        {
                            printf("WARNING in solve_leptonic_side_massiveb call %u: "
                                   "breaking out with %d solutions.\n",
                                   callcount, npbsave);
                            if (--max_warnings_4sols == 0)
                                printf("Further occurrences of this message "
                                       "will be suppressed\n");
                            fflush(stdout);
                        }
                    }
		    break;
                }

		/* Update the beta value */
		beta = pbthis/sqrt(pbthis*pbthis + mb*mb);
	    }

	    /* Check the number of iterations */
	    if (max_iterations > 1 && niter == max_iterations)
	    {
		failflag[isol] = 1;
		if (debug_level >= 10)
		{
                    static int max_warnings_iter = 1000;
                    if (max_warnings_iter > 0)
                    {
                        printf("WARNING in solve_leptonic_side_massiveb call %u: "
                               "iteration limit exceeded.\n", callcount);
                        printf("Relative solution precision is %g, requested %g.\n",
                               fabs(relative_shift), RELATIVE_EPS_LEPTONIC);
                        if (--max_warnings_iter == 0)
                            printf("Further occurrences of this message "
                                   "will be suppressed\n");
                        fflush(stdout);
                    }
		}
	    }
	}

    /* Check for top Pz boundaries */
    {
        int ngood = 0;
        double pbgood[4], pnuzgood[4];
        int failgood[4];

        for (isol=0; isol<4; ++isol)
        {
            const double pb = pbvalues[isol];
            if (pb > 0.0)
            {
                const double beta  = pb/sqrt(pb*pb + mb*mb);
                const double pwx   = topPx - pb*cbx;
                const double pwy   = topPy - pb*cby;
                const double denom = beta*cbz - clz;
                const double pwz   = ((lPx*pwx + lPy*pwy + mwsq/2.0)/pl - 
                                      beta*(mdiffsq/pb/2.0 + cbx*pwx + cby*pwy))/denom;
                const double topPz = pwz + pb*cbz;
                assert(denom != 0.0);
                if (minTopPz < topPz && topPz < maxTopPz)
                {
                    pbgood[ngood]   = pb;
                    failgood[ngood] = failflag[isol];
                    pnuzgood[ngood] = pwz - lPz;
                    ++ngood;
                }
            }
        }

        return fill_leptonic_solutions(
            cbx, cby, cbz, lPx, lPy, lPz, topPx, topPy,
            mt, mb, mwsq, pnuzgood, pbgood, failgood,
            ngood, solutions);
    }
}

int solve_leptonic_side_massiveb_verbose(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double *betalist, unsigned n_beta_points,
    const int debug_level, const size_t i_max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4])
{
    const size_t max_iterations = (i_max_iterations > 0 ? 
                                   i_max_iterations : MAX_ITERATIONS);

    const double pl         = sqrt(lPx*lPx + lPy*lPy + lPz*lPz);
    const double clx        = lPx/pl;
    const double cly        = lPy/pl;
    const double clz        = lPz/pl;

    const double bmag       = sqrt(bPx*bPx + bPy*bPy + bPz*bPz);
    const double cbx        = bPx/bmag;
    const double cby        = bPy/bmag;
    const double cbz        = bPz/bmag;

    const double cblt       = cbx*clx + cby*cly;
    const double sbzsq      = cbx*cbx + cby*cby;
    const double topPbt     = cbx*topPx + cby*topPy;
    const double topPlt     = clx*topPx + cly*topPy;
    const double slzsq      = 1.0 - clz*clz;
    const double t1         = mwsq + topPx*topPx + topPy*topPy;
    const double mwsqoverpl = mwsq/pl;
    const double t2         = mwsqoverpl/2.0 + topPlt;
    const double clzsq      = clz*clz;
    const double cbltsq     = cblt*cblt;
    const double cbzsq      = cbz*cbz;
    const double sbzquad    = sbzsq*sbzsq;
    const double mwsqoverplsq = mwsqoverpl*mwsqoverpl;
    const double topPbtsq   = topPbt*topPbt;
    const double mdiffsq    = mt*mt - mb*mb - mwsq;
    const double mdiffsqsq  = mdiffsq*mdiffsq;
    const double t3         = cbz*mwsqoverpl - 2.0*clz*topPbt;
    const double t3sq       = t3*t3;

    const double c4b0 = -4*(cbltsq + clzsq*sbzsq);
    const double c4b1 = 8*(cblt + cbz*clz)*sbzsq;
    const double c4b2 = 4*(cbltsq*cbzsq - cbz*(cbz + 2*cblt*clz)*
                           sbzsq - sbzquad*slzsq);
    const double c3b0 = 8*(cblt*t2 + clzsq*topPbt);
    const double c3b1 = -8*(sbzsq*t2 + (cblt + 2*cbz*clz)*topPbt);
    const double c3b2 = 8*(-((-(cbz*clz) + cbzsq*(cblt + cbz*clz))*t2) + 
                        (1 + cblt*cbz*clz - clzsq*sbzsq)*topPbt);
    const double c2b0 = -mwsqoverplsq - 4*(clzsq*t1 + topPlt*
                                           (mwsqoverpl + topPlt));
    const double c2b1 = 4*(-(cblt*mdiffsq) + mwsqoverpl*topPbt) + 
                        8*(cbz*clz*t1 + topPbt*topPlt);
    const double c2b2 = 4*mdiffsq*(cblt*cbz*clz + sbzsq*slzsq) + 
                        t3sq - 4*topPbtsq + 
                        4*cbzsq*topPlt*(mwsqoverpl + topPlt) - 
                        4*cbz*(cbz*t1 + 2*clz*topPbt*topPlt);
    const double c1b1 = 2*mdiffsq*(mwsqoverpl + 2*topPlt);
    const double c1b2 = 2*mdiffsq*(-(cbz*clz*mwsqoverpl) + 
                        2*(-(slzsq*topPbt) - cbz*clz*topPlt));
    const double c0b2 = -(mdiffsqsq*slzsq);

    static unsigned callcount = 0;
    static int *fastbreak = 0;
    static unsigned n_fastbreak = 0;
    double pbvalues[4] = {0};
    int failflag[4] = {0}, isol, maxnpb = 1, max_sols_ever = 0, nosol_broke = 0;
    unsigned istart;

    ++callcount;

    /* Check various assumptions */
    assert(pl > 0.0);
    assert(bmag > 0.0);
    assert(mt > 0.0);
    assert(mb > 0.0);
    assert(mwsq > 0.0);
    assert((mt - mb)*(mt - mb) > mwsq);

    /* Reset the output */
    memset(solutions, 0, 4*sizeof(lepton_side_solution));

    /* Figure out the starting points */
    if (betalist == 0 || n_beta_points == 0)
    {
        betalist = start_beta;
        n_beta_points = sizeof(start_beta)/sizeof(start_beta[0]);
    }

    /* Memory for flagging bad starting points */
    get_static_memory((void **)&fastbreak, sizeof(int),
                      &n_fastbreak, n_beta_points);
    for (istart = 0; istart < n_beta_points; ++istart)
        fastbreak[istart] = 0;

    if (debug_level >= 100)
    {
        printf("\nsolve_leptonic_side_massiveb_3: p %g m %g beta %g\n",
               bmag, mb, betalist[0]);
    }

    /* Cycle over solutions. We will choose the lower pb
     * solution on the first pass and higher solution
     * on the second.
     */
    for (isol = 0; isol < 2 && isol < maxnpb; ++isol)
	for (istart = 0; istart < n_beta_points && pbvalues[isol] == 0.0; ++istart)
	{
	    double relative_shift = 1.0, last_pb_chosen = 0.0;
	    double beta = betalist[istart];
	    unsigned niter;

	    if (fastbreak[istart])
		continue;

	    for (niter = 0; niter < max_iterations; ++niter)
	    {
		double betasq = beta*beta;
		double d0 = betasq*c0b2;
		double d1 = betasq*c1b2 + beta*c1b1;
		double d2 = betasq*c2b2 + beta*c2b1 + c2b0;
		double d3 = betasq*c3b2 + beta*c3b1 + c3b0;
		double d4 = betasq*c4b2 + beta*c4b1 + c4b0;

		/* Solve the quartic equation */
		double pbthis, pb[4];
                int npb = positive_quartic_roots(d4, d3, d2, d1, d0, pb);
                const int npbsave = npb;

                if (npb > max_sols_ever)
                    max_sols_ever = npb;

                if (debug_level >= 100)
                {
                    int i;
                    printf("Iter %s %d: beta %g, %d sols",
                           isol ? "up" : "down", niter, beta, npb);
                    for (i=0; i<npb; ++i)
                        printf(" %g", pb[i]);
                    if (npb == 0)
                        printf("\n");
                }

		/* Break if there are no solutions */
		if (npb == 0)
		{
		    if (niter == 0)
		    {
			/* We don't want to use this starting beta value
			 * on the second pass
			 */
			fastbreak[istart] = 1;
                        break;
		    }
                    else
                    {
                        pbvalues[isol] = 0.0;
                        nosol_broke = (isol+1)*10000 + niter;
                        break;
                    }
		}

		/* We should not normally have more than two solutions */
		if (npb > 2)
                {
                    /* If this happens, choose the two solutions
                     * which provide better match to the pb
                     * calculated with the assumed beta
                     */
                    int ib;
                    d_d_pair tosort[4];

                    for (ib=0; ib<npb; ++ib)
                    {
                        tosort[ib].d1 = pb[ib];
                        if (beta < 1.0)
                        {
                            const double pb0 = mb*beta/sqrt(1.0 - beta*beta);
                            tosort[ib].d2 = pb_distance(pb[ib], pb0);
                        }
                        else
                        {
                            /* Just select the two highest solutions */
                            tosort[ib].d2 = -pb[ib];
                        }
                    }
                    qsort(tosort, npb, sizeof(d_d_pair),
                          sort_d_d_pair_by_d2_incr);
                    pb[0] = tosort[0].d1;
                    pb[1] = tosort[1].d1;
                    npb = 2;
                }
                if (npb > maxnpb)
                    maxnpb = npb;

		/* Arrange solutions in the increasing order */
		if (npb == 2)
		    if (pb[0] > pb[1])
		    {
			const double dtmp = pb[0];
			pb[0] = pb[1];
			pb[1] = dtmp;
		    }

		/* Store the relevant solution in the variable
		   which is defined outside the loop */
                pbthis = isol ? pb[npb-1] : pb[0];
		relative_shift = 2.0*(pbthis - pbvalues[isol])/
		    (pbthis + pbvalues[isol]);
		pbvalues[isol] = pbthis;

                if (debug_level >= 100)
                    printf(", pick %g", pbthis);
                if (niter > 0 && debug_level >= 100)
                {
                    if ((isol == 0 && pbthis > last_pb_chosen) ||
                        (isol == 1 && pbthis < last_pb_chosen))
                        printf(" -OPP-");
                }
                if (debug_level >= 100)
                    printf("\n");

                last_pb_chosen = pbthis;

		/* Break if the precision is good enough */
		if (fabs(relative_shift) < RELATIVE_EPS_LEPTONIC)
                {
                    /* Warn if we are breaking out of
                     * the equation with more than two solutions
                     */
                    if (npbsave > 2 && debug_level >= 10)
                    {
                        static int max_warnings_4sols = 1000;
                        if (max_warnings_4sols > 0)
                        {
                            printf("WARNING in solve_leptonic_side_massiveb_3 call %u: "
                                   "breaking out with %d solutions.\n",
                                   callcount, npbsave);
                            if (--max_warnings_4sols == 0)
                                printf("Further occurrences of this message "
                                       "will be suppressed\n");
                            fflush(stdout);
                        }
                    }
                    if (debug_level >= 100)
                    {
                        printf("Iter %s %d converged to %g\n",
                               isol ? "up" : "down", niter, pbthis);
                    }
		    break;
                }

		/* Update the beta value */
		beta = pbthis/sqrt(pbthis*pbthis + mb*mb);
	    }

	    /* Check the number of iterations */
	    if (max_iterations > 1 && niter == max_iterations)
	    {
		failflag[isol] = 1;
		if (debug_level >= 10)
		{
                    static int max_warnings_iter = 1000;
                    if (max_warnings_iter > 0)
                    {
                        printf("WARNING in solve_leptonic_side_massiveb_3 call %u: "
                               "iteration limit exceeded.\n", callcount);
                        printf("Relative solution precision is %g, requested %g.\n",
                               fabs(relative_shift), RELATIVE_EPS_LEPTONIC);
                        if (--max_warnings_iter == 0)
                            printf("Further occurrences of this message "
                                   "will be suppressed\n");
                        fflush(stdout);
                    }
		}
	    }
	}

    /* Check for top Pz boundaries */
    {
        int ngood = 0;
        double pbgood[4], pnuzgood[4];
        int failgood[4] = {0}, matches = 0;

        for (isol=0; isol<4; ++isol)
        {
            const double pb = pbvalues[isol];
            if (pb > 0.0)
            {
                const double beta  = pb/sqrt(pb*pb + mb*mb);
                const double pwx   = topPx - pb*cbx;
                const double pwy   = topPy - pb*cby;
                const double denom = beta*cbz - clz;
                const double pwz   = ((lPx*pwx + lPy*pwy + mwsq/2.0)/pl - 
                                      beta*(mdiffsq/pb/2.0 + cbx*pwx + cby*pwy))/denom;
                const double topPz = pwz + pb*cbz;
                assert(denom != 0.0);

                if (minTopPz < topPz && topPz < maxTopPz)
                {
                    pbgood[ngood]   = pb;
                    failgood[ngood] = failflag[isol];
                    pnuzgood[ngood] = pwz - lPz;
                    ++ngood;

                    if (fabs(2.0*(pb - bmag)/(pb + bmag)) < 1.0e-3)
                        matches = 1;
                }
            }
        }

        if (!matches)
        {
            if (debug_level < 100)
                return solve_leptonic_side_massiveb_verbose(
                    topPx, topPy, lPx, lPy, lPz,
                    bPx, bPy, bPz, mt, mb, mwsq,
                    betalist, n_beta_points,
                    100, i_max_iterations,
                    minTopPz, maxTopPz, solutions);
            else
            {
                printf("nsols = %d, nbr = %d, max_sols = %d, failflags = %d %d, beta = %g\n\n",
                       ngood, nosol_broke, max_sols_ever, failgood[0], failgood[1], bmag/hypot(bmag, mb));
                fflush(stdout);
            }
        }
        return fill_leptonic_solutions(
            cbx, cby, cbz, lPx, lPy, lPz, topPx, topPy,
            mt, mb, mwsq, pnuzgood, pbgood, failgood,
            ngood, solutions);
    }
}

int solve_leptonic_side_massiveb_brute(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const int debug_level, const size_t i_max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4])
{
    const long double pl         = sqrtl(lPx*lPx + lPy*lPy + lPz*lPz);
    const long double clx        = lPx/pl;
    const long double cly        = lPy/pl;
    const long double clz        = lPz/pl;

    const long double bmag       = sqrtl(bPx*bPx + bPy*bPy + bPz*bPz);
    const long double cbx        = bPx/bmag;
    const long double cby        = bPy/bmag;
    const long double cbz        = bPz/bmag;
    const long double mbsq       = mb*mb;

    const long double cblt       = cbx*clx + cby*cly;
    const long double sbzsq      = cbx*cbx + cby*cby;
    const long double topPbt     = cbx*topPx + cby*topPy;
    const long double topPlt     = clx*topPx + cly*topPy;
    const long double slzsq      = 1.0 - clz*clz;
    const long double mwsqoverpl = mwsq/pl;
    const long double clzsq      = clz*clz;
    const long double cbltsq     = cblt*cblt;
    const long double cbzsq      = cbz*cbz;
    const long double sbzquad    = sbzsq*sbzsq;
    const long double mwsqoverplsq = mwsqoverpl*mwsqoverpl;
    const long double topPbtsq   = topPbt*topPbt;
    const long double mdiffsq    = mt*mt - mbsq - mwsq;
    const long double mdiffsqsq  = mdiffsq*mdiffsq;
    const long double t1         = mwsq + topPx*topPx + topPy*topPy;
    const long double t2         = mwsqoverpl/2.0 + topPlt;
    const long double t3         = cbz*mwsqoverpl - 2.0*clz*topPbt;
    const long double t3sq       = t3*t3;

    const long double c4b0 = -4*(cbltsq + clzsq*sbzsq);
    const long double c4b1 = 8*(cblt + cbz*clz)*sbzsq;
    const long double c4b2 = 4*(cbltsq*cbzsq - cbz*(cbz + 2*cblt*clz)*
                           sbzsq - sbzquad*slzsq);
    const long double c3b0 = 8*(cblt*t2 + clzsq*topPbt);
    const long double c3b1 = -8*(sbzsq*t2 + (cblt + 2*cbz*clz)*topPbt);
    const long double c3b2 = 8*(-((-(cbz*clz) + cbzsq*(cblt + cbz*clz))*t2) + 
                           (1 + cblt*cbz*clz - clzsq*sbzsq)*topPbt);
    const long double c2b0 = -mwsqoverplsq - 4*(clzsq*t1 + topPlt*
                                           (mwsqoverpl + topPlt));
    const long double c2b1 = 4*(-(cblt*mdiffsq) + mwsqoverpl*topPbt) + 
                        8*(cbz*clz*t1 + topPbt*topPlt);
    const long double c2b2 = 4*mdiffsq*(cblt*cbz*clz + sbzsq*slzsq) + 
                        t3sq - 4*topPbtsq + 
                        4*cbzsq*topPlt*(mwsqoverpl + topPlt) - 
                        4*cbz*(cbz*t1 + 2*clz*topPbt*topPlt);
    const long double c1b1 = 2*mdiffsq*(mwsqoverpl + 2*topPlt);
    const long double c1b2 = 2*mdiffsq*(-(cbz*clz*mwsqoverpl) + 
                                   2*(-(slzsq*topPbt) - cbz*clz*topPlt));
    const long double c0b2 = -(mdiffsqsq*slzsq);

    const long double c0i0 = c0b2 + c2b0*mbsq;
    const long double c1i0 = c1b2 + c3b0*mbsq;
    const long double c2i0 = c2b2 + c4b0*mbsq + c2b0;
    const long double c3i0 = c3b2 + c3b0;
    const long double c4i0 = c4b2 + c4b0;

    const int blep_is_extra = fabs((double)bmag - 1.0) < 1.0e-3;

    static unsigned callcount = 0;
    double poly_coeffs[9], poly_solutions[8];
    unsigned isol, n_poly_solutions = 0;
    d_d_d_triple sol_set[8];
    unsigned n_filtered_solutions = 0;

    ++callcount;

    /* Check various assumptions */
    assert(pl > 0.0);
    assert(bmag > 0.0);
    assert(mt > 0.0);
    assert(mb > 0.0);
    assert(mwsq > 0.0);
    assert((mt - mb)*(mt - mb) > mwsq);

    /* Reset the output */
    memset(solutions, 0, 4*sizeof(lepton_side_solution));

    /* Polynomial coefficients in the order of decreasing powers */
    poly_coeffs[8] = c0i0*c0i0 - c1b1*c1b1*mbsq;
    poly_coeffs[7] = 2*c0i0*c1i0 - 2*c1b1*c2b1*mbsq;
    poly_coeffs[6] = -c1b1*c1b1 + c1i0*c1i0 + 2*c0i0*c2i0 - c2b1*c2b1*mbsq - 2*c1b1*c3b1*mbsq;
    poly_coeffs[5] = -2*(-(c1i0*c2i0) - c0i0*c3i0 + c2b1*c3b1*mbsq + c1b1*(c2b1 + c4b1*mbsq));
    poly_coeffs[4] = -c2b1*c2b1 + c2i0*c2i0 - 2*c1b1*c3b1 + 2*c1i0*c3i0 + 
                     2*c0i0*c4i0 - c3b1*c3b1*mbsq - 2*c2b1*c4b1*mbsq;  
    poly_coeffs[3] = -2*(c2b1*c3b1 - c2i0*c3i0 + c1b1*c4b1 - c1i0*c4i0 + c3b1*c4b1*mbsq);
    poly_coeffs[2] = -c3b1*c3b1 + c3i0*c3i0 - 2*c2b1*c4b1 + 2*c2i0*c4i0 - c4b1*c4b1*mbsq;
    poly_coeffs[1] = 2*(c3i0*c4i0 - c3b1*c4b1);
    poly_coeffs[0] = (c4i0 - c4b1)*(c4i0 + c4b1);

    /* Check that the leading coefficient is not 0 */
    assert(poly_coeffs[0]);

    /* Solve the polynomial equation here */
    {
        int i;
        double zeror[8], zeroi[8];

        const int nroots = rpoly(poly_coeffs, 8, zeror, zeroi);
        assert(nroots >= 0);
        assert(nroots <= 8);

        for (i=0; i<nroots; ++i)
            if (zeroi[i] == 0.0)
                poly_solutions[n_poly_solutions++] = zeror[i];
    }

    /* Filter out incorrect solutions:
     *   a) pb <= 0
     *   b) fakes
     *   c) pb that bring topPz out of range
     */
    for (isol=0; isol<n_poly_solutions; ++isol)
    {
        const double pb = poly_solutions[isol];
        if (pb > 0.0)
        {
            const double poly1 = c0i0 + pb*(c1i0 + pb*(c2i0 + pb*(c3i0 + pb*c4i0)));
            const double poly2 = c1b1 + pb*(c2b1 + pb*(c3b1 + pb*c4b1));
            if (poly1*poly2 < 0.0)
            {
                const double beta  = pb/sqrt(pb*pb + mbsq);
                const double pwx   = topPx - pb*cbx;
                const double pwy   = topPy - pb*cby;
                const double denom = beta*cbz - clz;
                const double pwz   = ((lPx*pwx + lPy*pwy + mwsq/2.0)/pl - 
                                      beta*(mdiffsq/pb/2.0 + cbx*pwx + cby*pwy))/denom;
                const double topPz = pwz + pb*cbz;
                assert(denom != 0.0);
                if (minTopPz < topPz && topPz < maxTopPz)
                {
                    sol_set[n_filtered_solutions].d1 = pb;
                    sol_set[n_filtered_solutions].d2 = pwz - lPz;
                    ++n_filtered_solutions;
                }
            }
        }
    }

    /* Check if we have too many solutions.
     * If so, select the four solutions which
     * are closest to the observed pb value.
     */
    if (n_filtered_solutions > 4)
    {
        for (isol=0; isol<n_filtered_solutions; ++isol)
        {
            if (blep_is_extra)
                sol_set[isol].d3 = pb_distance(sol_set[isol].d1, 70.0);
            else
                sol_set[isol].d3 = pb_distance(sol_set[isol].d1, bmag);
        }
        qsort(sol_set, n_filtered_solutions, sizeof(d_d_d_triple),
              sort_d_d_d_triple_by_d3_incr);
        if (debug_level > 0)
        {
            printf("WARNING in solve_leptonic_side_massiveb_brute: "
                   "%d solutions\n", n_filtered_solutions);
            fflush(stdout);
        }
    }

    /* Fill out the solution structures */
    if (n_filtered_solutions > 0)
    {
        double pbgood[4], pnuzgood[4];
        int failgood[4] = {0};
        const unsigned ngood = n_filtered_solutions < 4 ? n_filtered_solutions : 4;

        for (isol=0; isol<ngood; ++isol)
        {
            pbgood[isol] = sol_set[isol].d1;
            pnuzgood[isol] = sol_set[isol].d2;
        }

        return fill_leptonic_solutions(
            cbx, cby, cbz, lPx, lPy, lPz, topPx, topPy,
            mt, mb, mwsq, pnuzgood, pbgood, failgood,
            ngood, solutions);
    }
    else
        return 0;
}

int solve_leptonic_for_top_pt(
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double nuPz, double topPx[4], double topPy[4])
{
    int i, goodsols = 0;
    const double deltamsqov2 = (mt*mt - mb*mb - mwsq)/2.0;

    for (i=0; i<4; ++i)
    {
        topPx[i] = 0.0;
        topPy[i] = 0.0;
    }

    if (deltamsqov2 > 0.0)
    {
        const double ebsq = bPx*bPx + bPy*bPy + bPz*bPz + mb*mb;
        const double pwz = lPz + nuPz;
        Ellipse_coeffs eq[2];
        double pnux[4], pnuy[4];
        int nsols;

        eq[0].a00 = lPy*lPy + lPz*lPz;
        eq[0].a01 = -lPx*lPy;
        eq[0].a11 = lPx*lPx + lPz*lPz;
        eq[0].b0  = -(lPx*(mwsq + 2*lPz*nuPz));
        eq[0].b1  = -(lPy*(mwsq + 2*lPz*nuPz));
        eq[0].c   = (lPx*lPx + lPy*lPy)*nuPz*nuPz - lPz*mwsq*nuPz - mwsq*mwsq/4.0;

        eq[1].a00 = bPy*bPy + bPz*bPz + mb*mb;
        eq[1].a01 = -bPx*bPy;
        eq[1].a11 = bPx*bPx + bPz*bPz + mb*mb;
        eq[1].b0  = -2*(-lPx*eq[1].a00 + bPx*(deltamsqov2 + bPy*lPy + bPz*pwz));
        eq[1].b1  = -2*(-lPy*eq[1].a11 + bPy*(deltamsqov2 + bPx*lPx + bPz*pwz));
        eq[1].c   = -deltamsqov2*deltamsqov2 - 
            bPx*bPx*lPx*lPx + ebsq*lPx*lPx - bPy*bPy*lPy*lPy + 
            ebsq*lPy*lPy - 2*bPy*bPz*lPy*lPz - bPz*bPz*lPz*lPz + ebsq*lPz*lPz + 
            ebsq*mwsq - 2*bPy*bPz*lPy*nuPz - 2*bPz*bPz*lPz*nuPz + 
            2*ebsq*lPz*nuPz - bPz*bPz*nuPz*nuPz + ebsq*nuPz*nuPz - 
            2*bPx*lPx*(bPy*lPy + bPz*pwz) - 
            2*deltamsqov2*(bPx*lPx + bPy*lPy + bPz*pwz);

        nsols = ellipse_intersection(eq, pnux, pnuy);
        for (i=0; i<nsols; ++i)
        {
            const double pwx = lPx + pnux[i];
            const double pwy = lPy + pnuy[i];
            const double pbpw = bPx*pwx + bPy*pwy + bPz*pwz;
            if (pbpw + deltamsqov2 > 0.0)
            {
                const double plpnu = lPx*pnux[i] + lPy*pnuy[i] + lPz*nuPz;
                if (plpnu + mwsq/2.0 > 0.0)
                {
                    topPx[goodsols] = pwx + bPx;
                    topPy[goodsols] = pwy + bPy;
                    ++goodsols;
                }
            }
        }
    }

    return goodsols;
}

#ifdef __cplusplus
}
#endif
