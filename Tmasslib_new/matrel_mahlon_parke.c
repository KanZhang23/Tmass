#include <math.h>
#include <assert.h>
#include "matrel_mahlon_parke.h"

#ifdef __cplusplus
extern "C" {
#endif

double matrel_mahlon_parke(
    particle_obj ebar_lab, particle_obj nu_lab, particle_obj b_lab,
    particle_obj mu_lab, particle_obj nubar_lab, particle_obj bbar_lab,
    double mW, double GW, double mtop,
    double *ptterm, double *ptbarterm, double *pbigbrace)
{
    /* Set the following to 1 to include W propagators
     * into the matrix element
     */
    const int include_w_propagators = 0;

    static const particle_obj beam_direction = {{0.0, 0.0, 1.0}, 0.0, 1.0};
    static const particle_obj opposite_beam_direction = {{0.0, 0.0, -1.0}, 0.0, 1.0};
    const double mWsq = mW*mW;
    const double GWsq = GW*GW;
    const double mtsq = mtop*mtop;
    const particle_obj wplus_lab = sum4(ebar_lab, nu_lab);
    const particle_obj t_lab = sum4(wplus_lab, b_lab);
    const particle_obj wminus_lab = sum4(mu_lab, nubar_lab);
    const particle_obj tbar_lab = sum4(wminus_lab, bbar_lab);
    const particle_obj ttbar_lab = sum4(t_lab, tbar_lab);

    /* Boost all particles into the zero-momentum frame */
    const boost_obj zmf_boost = rest_boost(ttbar_lab);
    const particle_obj q      = boost(beam_direction, zmf_boost);
    const particle_obj qbar   = boost(opposite_beam_direction, zmf_boost);
    const particle_obj t      = boost(t_lab, zmf_boost);
    const particle_obj tbar   = boost(tbar_lab, zmf_boost);
    const particle_obj ebar   = boost(ebar_lab, zmf_boost);
    const particle_obj mu     = boost(mu_lab, zmf_boost);
    /* const particle_obj nu     = boost(nu_lab, zmf_boost);    */
    /* const particle_obj b      = boost(b_lab, zmf_boost);     */
    /* const particle_obj nubar  = boost(nubar_lab, zmf_boost); */
    /* const particle_obj bbar   = boost(bbar_lab, zmf_boost);  */

    /* Calculate various components for the final formula */
    const double twoebarnu    = 2.0*sprod4(ebar_lab, nu_lab);
    const double c_hat_ebarb  = cosboosted3(ebar_lab, b_lab, wplus_lab);
    const double twomunubar   = 2.0*sprod4(mu_lab, nubar_lab);
    const double c_hat_mubbar = cosboosted3(mu_lab, bbar_lab, wminus_lab);
    const double beta    = f_beta(t);
    const double gamma   = f_gamma(t);
    const double cqt     = cos3(q.p, t.p);
    const double sqt     = sqrt(1.0 - cqt*cqt);
    const double cebarq  = cos3(ebar.p, q.p);
    const double cmuqbar = cos3(mu.p, qbar.p);
    const double cmutbar = cos3(mu.p, tbar.p);
    const double cebart  = cos3(ebar.p, t.p);
    const double cebarmu = cos3(ebar.p, mu.p);

    /* Calculate the factors and return the final result */
    double numer, denom, bigbrace, result;
    double tterm = 0.0, tbarterm = 0.0;

    if (mtsq > twoebarnu)
    {
        numer = (1.0-c_hat_ebarb*c_hat_ebarb) +
            twoebarnu/mtsq*(1.0+c_hat_ebarb)*(1.0+c_hat_ebarb);
        if (include_w_propagators)
            denom = (twoebarnu-mWsq)*(twoebarnu-mWsq) + mWsq*GWsq;
        else
            denom = 1.0;
        tterm = (mtsq - twoebarnu)*numer/denom;
    }
    if (ptterm)
        *ptterm = tterm;

    if (mtsq > twomunubar)
    {
        numer = (1.0-c_hat_mubbar*c_hat_mubbar) +
            twomunubar/mtsq*(1.0+c_hat_mubbar)*(1.0+c_hat_mubbar);
        if (include_w_propagators)
            denom = (twomunubar-mWsq)*(twomunubar-mWsq) + mWsq*GWsq;
        else
            denom = 1.0;
        tbarterm = (mtsq - twomunubar)*numer/denom;
    }
    if (ptbarterm)
        *ptbarterm = tbarterm;

    numer = (1.0-cebarq*cmuqbar) - beta*(cmutbar+cebart) + 
        beta*cqt*(cebarq+cmuqbar) + 0.5*beta*beta*sqt*sqt*(1.0-cebarmu);
    denom = gamma*gamma*(1.0-beta*cebart)*(1.0-beta*cmutbar);
    bigbrace = (2.0 - beta*beta*sqt*sqt) - numer/denom;
    if (bigbrace < 0.0)
    {
        /* Should not normally happen, but might for wrong
         * permutations/far away top mass assignments
         */
        bigbrace = 0.0;
    }
    if (pbigbrace)
        *pbigbrace = bigbrace;

    result = tterm*tbarterm*bigbrace;
    assert(result >= 0.0);
    return result;
}

double mahlon_parke_weight(
    particle_obj q, particle_obj qbar, particle_obj bhad,
    particle_obj blep, particle_obj lep, particle_obj nu,
    int leptonCharge, double mW, double widthW, double mt)
{
    /* Note that the following might not be correct for Pythia.
     * In Herwig we will have q set to ubar or cbar, qbar set to d or s.
     * That is, q and qbar meaning is swapped due to specific ordering
     * of the decay daughters.
     */
    if (leptonCharge > 0)
        return matrel_mahlon_parke(
            lep, nu, blep, qbar, q, bhad,
            mW, widthW, mt, 0, 0, 0);
    else
        return matrel_mahlon_parke(
            qbar, q, bhad, lep, nu, blep,
            mW, widthW, mt, 0, 0, 0);
}

#ifdef __cplusplus
}
#endif
