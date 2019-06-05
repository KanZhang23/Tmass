#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "topmass_norm.h"
#include "p_struct_function.h"
#include "matrel_kleiss_stirling.h"
#include "matrel_mahlon_parke.h"
#include "generate_ttbar_pt.h"
#include "range_lookup.h"
#include "range_lookup_flt.h"
#include "topmass_utils.h"
#include "pi.h"

#define swap_double(a, b) do {\
    double dtmp = a;\
    a = b;\
    b = dtmp;\
} while(0);

#ifdef __cplusplus
extern "C" {
#endif

static double cauchy_distribution(const double x, const double peak,
                                  const double hwhm)
{
    return atan((x - peak)/hwhm)/PI + 0.5;
}

static double generate_relativistic_bw(const double polemass,
                                       const double width,
                                       const double maxmass)
{
    const double peak = polemass*polemass;
    const double hwhm = polemass*width;
    const double msqmax = maxmass*maxmass;
    double msq = cauchy_random(peak, hwhm);
    if (msq <= 0.0 || msq >= msqmax)
    {
        const double cdf_0 = cauchy_distribution(0.0, peak, hwhm);
        const double cdf_1 = cauchy_distribution(msqmax, peak, hwhm);
        do {
            const double rnd = cdf_0 + uniform_random()*(cdf_1 - cdf_0);
            msq = hwhm*tan(PI*(rnd - 0.5)) + peak;
        } while (msq <= 0.0 || msq >= msqmax);
    }
    return sqrt(msq);
}

static double bw_integral(double peak, double hwhm,
                          double xmin, double xmax)
{
    const double pmin = cauchy_distribution(xmin, peak, hwhm);
    const double pmax = cauchy_distribution(xmax, peak, hwhm);
    return pmax - pmin;
}

int fill_norm_parameters(norm_parameters *np, double pole_t_mass,
                         double twidth, double mtmin, double mtmax,
                         double electron_pt_min, double electron_eta_max,
                         double muon_pt_min, double muon_eta_max,
                         double miss_et_cut, double cone_cut, double iso_cut,
                         double jet_pt_cut, double jet_eta_cut,
                         unsigned polemass_bin)
{
    memset(np, 0, sizeof(norm_parameters));

    if (pole_t_mass <= 0.0)
        return 1;
    else
        np->pole_t_mass = pole_t_mass;

    if (twidth < 0.0)
        return 1;
    else
        np->twidth = twidth;

    if (mtmin >= mtmax)
        return 1;
    else
    {
        np->mtmin = mtmin;
        np->mtmax = mtmax;
    }

    np->electron_pt_min = electron_pt_min;
    np->electron_eta_max = electron_eta_max;
    np->muon_pt_min = muon_pt_min;
    np->muon_eta_max = muon_eta_max;
    np->miss_et_cut = miss_et_cut;
    np->cone_cut = cone_cut;
    np->iso_cut = iso_cut;
    np->jet_pt_cut = jet_pt_cut;
    np->jet_eta_cut = jet_eta_cut;
    np->polemass_bin = polemass_bin;

    return 0;
}

void print_norm_parameters(const norm_parameters *np, FILE *fp)
{
    fprintf(fp, "Normfactor calculation parameters:\n");
    fprintf(fp, "pole_t_mass = %g\n", np->pole_t_mass);
    fprintf(fp, "twidth = %g\n", np->twidth);
    fprintf(fp, "mtmin = %g\n", np->mtmin);
    fprintf(fp, "mtmax = %g\n", np->mtmax);
    fprintf(fp, "electron_pt_min = %g\n", np->electron_pt_min);
    fprintf(fp, "electron_eta_max = %g\n", np->electron_eta_max);
    fprintf(fp, "muon_pt_min = %g\n", np->muon_pt_min);
    fprintf(fp, "muon_eta_max = %g\n", np->muon_eta_max);
    fprintf(fp, "miss_et_cut = %g\n", np->miss_et_cut);
    fprintf(fp, "cone_cut = %g\n", np->cone_cut);
    fprintf(fp, "iso_cut = %g\n", np->iso_cut);
    fprintf(fp, "jet_pt_cut = %g\n", np->jet_pt_cut);
    fprintf(fp, "jet_eta_cut = %g\n", np->jet_eta_cut);
    fprintf(fp, "polemass_bin = %u\n", np->polemass_bin);
}

#define validate_decay(mother, m1, m2, p1, p2) do {\
    const int decay_status = phsp_decay(mother, m1, m2, p1, p2);\
    assert(decay_status == 0);\
} while(0);

void weighted_ttbar_phsp_point(const integ_parameters *ipars,
                               const norm_parameters *normpars,
                               const particle_obj ttbar, const int use_bw,
                               particle_obj *tlep, particle_obj *thad,
                               particle_obj *wlep, particle_obj *whad,
                               particle_obj *q, particle_obj *qbar,
                               particle_obj *bhad, particle_obj *l,
                               particle_obj *nu, particle_obj *blep,
                               double *phsp_weight, double *norm_integral)
{
    double mtlep, mthad, mwlep, mwhad;
    const double mbhad = ipars->had_b_jet_mass;
    const double mblep = ipars->lep_b_jet_mass;

    /* Reset the weights */
    *phsp_weight = 0.0;
    *norm_integral = 0.0;

    if (ttbar.m <= 0.0)
        return;
    
    if (use_bw)
    {
        /* Generate masses of the intermediate compounds
         * according to their respective Breit-Wigners
         */
        mtlep = generate_relativistic_bw(normpars->pole_t_mass, normpars->twidth, ttbar.m);
        mthad = generate_relativistic_bw(normpars->pole_t_mass, normpars->twidth, ttbar.m);
        mwlep = generate_relativistic_bw(ipars->nominal_w_mass, ipars->nominal_w_width, ttbar.m);
        mwhad = generate_relativistic_bw(ipars->nominal_w_mass, ipars->nominal_w_width, ttbar.m);
    }
    else
    {
        /* "Dumb" mass assignments -- very inefficient
         * but good enough for testing purposes
         */
        mtlep = ttbar.m*sqrt(uniform_random());
        mthad = ttbar.m*sqrt(uniform_random());
        mwlep = ttbar.m*sqrt(uniform_random());
        mwhad = ttbar.m*sqrt(uniform_random());
        if (mthad == 0.0 || mtlep == 0.0 || mwlep == 0.0 || mwhad == 0.0)
            return;
        if (mtlep < mwlep)
            swap_double(mtlep, mwlep);
        if (mthad < mwhad)
            swap_double(mthad, mwhad);
    }
    
    /* Check phase space limits */
    if (mthad + mtlep >= ttbar.m)
        return;

    assert(mbhad >= 0.0);
    if (mbhad + mwhad >= mthad)
        return;

    assert(mblep >= 0.0);
    if (mblep + mwlep >= mtlep)
        return;
    
    /* Trim top mass Breit-Wigner tails a-la Herwig */
    if (mtlep < normpars->mtmin || mtlep > normpars->mtmax || 
        mthad < normpars->mtmin || mthad > normpars->mtmax)
        return;
    
    /* Produce the phase space decay chain */
    validate_decay(ttbar, mtlep, mthad, tlep, thad);
    validate_decay(*tlep, mwlep, mblep, wlep, blep);
    validate_decay(*thad, mwhad, mbhad, whad, bhad);
    validate_decay(*wlep, 0.0, 0.0, l, nu);
    validate_decay(*whad, 0.0, 0.0, q, qbar);

    /* Integrals over m^2 of the intermediate compounds */
    if (use_bw)
    {
        const double it = bw_integral(normpars->pole_t_mass*normpars->pole_t_mass,
                                      normpars->pole_t_mass*normpars->twidth,
                                      0.0, ttbar.m*ttbar.m);
        const double iw = bw_integral(ipars->nominal_w_mass*ipars->nominal_w_mass,
                                      ipars->nominal_w_mass*ipars->nominal_w_width,
                                      0.0, ttbar.m*ttbar.m);
        *norm_integral = it*it*iw*iw;
    }
    else
        /* The division by 4 is due to swaps between mwlep,mtlep and mwhad,mthad */
        *norm_integral = pow(ttbar.m, 8)/4.0;
    
    /* Phase space factors are set here to |p|/M_parent where
     * |p| is the daughter momentum in the parent rest frame
     */
    *phsp_weight = sqr_lambda(ttbar.m*ttbar.m, mthad*mthad, mtlep*mtlep)/(2.0*ttbar.m*ttbar.m) *
        sqr_lambda(mthad*mthad, mwhad*mwhad, mbhad*mbhad)/(2.0*mthad*mthad) *
        sqr_lambda(mtlep*mtlep, mwlep*mwlep, mblep*mblep)/(2.0*mtlep*mtlep);

    return;
}

void weighted_ttbar_event(const integ_parameters *ipars,
                          const norm_parameters *normpars,
                          const int use_bw, ttbar_event *ev,
                          double *weight_qq, double *weight_gg)
{
    particle_obj ttbar0pt;
    double pdf_qq, pdf_gg, matrel_qq, matrel_gg;
    double phsp_weight, norm_integral, propagator_weight;
    v3_obj ttbar_pt;
    
    /* Reset the weights so that we can just return in case
     * some random variable brings us outside the phase space boundary
     */
    *weight_qq = 0.0;
    *weight_gg = 0.0;
    
    /* For weighted events production channel is undefined */
    ev->prod_channel = PROD_CHANNEL_UNDEFINED;
    
    /* Generate parton momenta */
    ev->Ecms  = ipars->cms_energy;
    ev->x1    = uniform_random();
    ev->x2    = uniform_random();
    ev->q1    = particle(v3(0.0, 0.0,  ev->x1*ev->Ecms/2.0), 0.0);
    ev->q2    = particle(v3(0.0, 0.0, -ev->x2*ev->Ecms/2.0), 0.0);
    
    /* Approximate ttbar compound (without any transverse momentum) */
    ttbar0pt = sum4(ev->q1, ev->q2);
    
    /* Assign some transverse momentum to the ttbar system */
    ttbar_pt = generate_ttbar_pt(normpars->pole_t_mass);
    ev->ttbar = particle(v3(ttbar_pt.x, ttbar_pt.y, ttbar0pt.p.z), ttbar0pt.m);
    
    /* Decay t and tbar */
    weighted_ttbar_phsp_point(ipars, normpars, ev->ttbar, use_bw,
                              &ev->t, &ev->tbar, &ev->wplus, &ev->wminus,
                              &ev->mu, &ev->nubar, &ev->bbar, &ev->ebar,
                              &ev->nu, &ev->b, &phsp_weight, &norm_integral);
    if (phsp_weight == 0.0)
        return;
    
    /* Figure out the remaining event weights.
     * First, calculate the structure functions.
     */
    if (ipars->matrel_code & MASK_PROTON_STRUCT)
        pdfterm_qqbar(ev->t, ev->tbar, ev->Ecms, &pdf_qq, &pdf_gg);
    else
    {
        pdf_qq = 1.0;
        pdf_gg = 0.0;
    }
    if (pdf_qq == 0.0 && pdf_gg == 0.0)
        return;
    
    /* Now, calculate the matrix element */
    if (ipars->matrel_code & MASK_KLEISS_STIRLING)
    {
        matrel_kleiss_stirling(
            ev->ebar, ev->nu, ev->b, ev->mu, ev->nubar, ev->bbar,
            ipars->nominal_w_mass, ipars->nominal_w_width,
            normpars->pole_t_mass, &matrel_qq, &matrel_gg);
    }
    else if (ipars->matrel_code & MASK_MAHLON_PARKE)
    {
        matrel_qq = matrel_mahlon_parke(
            ev->ebar, ev->nu, ev->b, ev->mu, ev->nubar, ev->bbar,
            ipars->nominal_w_mass, ipars->nominal_w_width,
            normpars->pole_t_mass, NULL, NULL, NULL);
        matrel_gg = 0.0;
    }
    else
    {
        matrel_qq = 1.0;
        matrel_gg = 0.0;
    }
    
    /* Now, the weight due to propagators */
    if (use_bw)
        propagator_weight = 1.0;
    else
    {
        propagator_weight = 
            relativistic_bw_density(ev->t.m, normpars->pole_t_mass, normpars->twidth) *
            relativistic_bw_density(ev->tbar.m, normpars->pole_t_mass, normpars->twidth) *
            relativistic_bw_density(ev->wplus.m, ipars->nominal_w_mass, ipars->nominal_w_width) *
            relativistic_bw_density(ev->wminus.m, ipars->nominal_w_mass, ipars->nominal_w_width);
    }
    
    /* Combine the weights */
    *weight_qq = norm_integral*phsp_weight*propagator_weight*pdf_qq*matrel_qq;
    assert(*weight_qq >= 0.0);
    *weight_gg = norm_integral*phsp_weight*propagator_weight*pdf_gg*matrel_gg;
    assert(*weight_gg >= 0.0);
}

void ttbar_event_weights(const integ_parameters *ipars,
                         const norm_parameters *normpars,
                         const int propagator_weight_type,
                         const particle_obj mu, const particle_obj nubar,
                         const particle_obj bbar, const particle_obj ebar,
                         const particle_obj nu, const particle_obj b,
                         double *weight_qq, double *weight_gg)
{
    const particle_obj wminus = sum4(mu, nubar);
    const particle_obj tbar = sum4(wminus, bbar);
    const particle_obj wplus = sum4(nu, ebar);
    const particle_obj t = sum4(wplus, b);
    const particle_obj ttbar = sum4(t, tbar);
    const double phsp_weight = 
        sqr_lambda(ttbar.m*ttbar.m, tbar.m*tbar.m, t.m*t.m)/(2.0*ttbar.m*ttbar.m) *
        sqr_lambda(tbar.m*tbar.m, wminus.m*wminus.m, bbar.m*bbar.m)/(2.0*tbar.m*tbar.m) *
        sqr_lambda(t.m*t.m, wplus.m*wplus.m, b.m*b.m)/(2.0*t.m*t.m);
    double norm_integral = 1.0, pdf_gg, pdf_qq, matrel_qq, matrel_gg, propagator_weight = 1.0;

    *weight_qq = 0.0;
    *weight_gg = 0.0;

    switch (propagator_weight_type)
    {    
    case PROPAG_WEIGHT_GENERATOR:
        {
            const double it = bw_integral(normpars->pole_t_mass*normpars->pole_t_mass,
                                          normpars->pole_t_mass*normpars->twidth,
                                          0.0, ttbar.m*ttbar.m);
            const double iw = bw_integral(ipars->nominal_w_mass*ipars->nominal_w_mass,
                                          ipars->nominal_w_mass*ipars->nominal_w_width,
                                          0.0, ttbar.m*ttbar.m);
            norm_integral = it*it*iw*iw;
        }
        break;
    case PROPAG_WEIGHT_BW:
        /* The division by 4 is due to swaps between mwlep,mtlep and mwhad,mthad */
        norm_integral = pow(ttbar.m, 8)/4.0;
        break;
    case PROPAG_WEIGHT_EFFECTIVE:
        assert(!"Effective propagators are not supported");
        break;
    default:
        assert(0);
    }

    /* Figure out the remaining event weights.
     * First, calculate the structure functions.
     */
    if (ipars->matrel_code & MASK_PROTON_STRUCT)
        pdfterm_qqbar(t, tbar, ipars->cms_energy, &pdf_qq, &pdf_gg);
    else
    {
        pdf_qq = 1.0;
        pdf_gg = 0.0;
    }
    if (pdf_qq == 0.0 && pdf_gg == 0.0)
    {
        printf("WARNING in ttbar_event_weights: zero PDFs\n");
        fflush(stdout);
        return;
    }

    /* Now, calculate the matrix element */
    if (ipars->matrel_code & MASK_KLEISS_STIRLING)
    {
        matrel_kleiss_stirling(
            ebar, nu, b, mu, nubar, bbar,
            ipars->nominal_w_mass, ipars->nominal_w_width,
            normpars->pole_t_mass, &matrel_qq, &matrel_gg);
    }
    else if (ipars->matrel_code & MASK_MAHLON_PARKE)
    {
        matrel_qq = matrel_mahlon_parke(
            ebar, nu, b, mu, nubar, bbar,
            ipars->nominal_w_mass, ipars->nominal_w_width,
            normpars->pole_t_mass, NULL, NULL, NULL);
        matrel_gg = 0.0;
    }
    else
    {
        matrel_qq = 1.0;
        matrel_gg = 0.0;
    }
    
    /* Now, the weight due to propagators */
    switch (propagator_weight_type)
    {
    case PROPAG_WEIGHT_GENERATOR:
        propagator_weight = 1.0;
        break;
    case PROPAG_WEIGHT_BW:
        propagator_weight = 
            relativistic_bw_density(t.m, normpars->pole_t_mass, normpars->twidth) *
            relativistic_bw_density(tbar.m, normpars->pole_t_mass, normpars->twidth) *
            relativistic_bw_density(wplus.m, ipars->nominal_w_mass, ipars->nominal_w_width) *
            relativistic_bw_density(wminus.m, ipars->nominal_w_mass, ipars->nominal_w_width);
        break;
    case PROPAG_WEIGHT_EFFECTIVE:
        assert(!"Effective propagators are not supported");
        break;
    default:
        assert(0);
    }

    /* Combine the weights */
    *weight_qq = norm_integral*phsp_weight*propagator_weight*pdf_qq*matrel_qq;
    assert(*weight_qq >= 0.0);
    *weight_gg = norm_integral*phsp_weight*propagator_weight*pdf_gg*matrel_gg;
    assert(*weight_gg >= 0.0);

    if (*weight_qq == 0.0 && *weight_gg == 0.0)
    {
        printf("WARNING in ttbar_event_weights: total weight is zero\n");
        fflush(stdout);
    }
}

int unweighted_ttbar_event(const integ_parameters *ipars,
                           const norm_parameters *normpars,
                           int prod_channel, double maxweight_qq,
                           double maxweight_gg, double maxweight_sum,
                           ttbar_event *event, double *pweight_qq,
                           double *pweight_gg)
{
    double maxweight, weight_qq, weight_gg, weight_sum;
    double *pweight;
    int count;

    switch (prod_channel)
    {
    case PROD_CHANNEL_QQ:
        maxweight = maxweight_qq;
        pweight = &weight_qq;
        break;

    case PROD_CHANNEL_GG:
        maxweight = maxweight_gg;
        pweight = &weight_gg;
        break;

    case PROD_CHANNEL_UNDEFINED:
        maxweight = maxweight_sum;
        pweight = &weight_sum;
        break;

    default:
        assert(0);
    }

    for (count=1;;++count)
    {
        /* Make sure we did not overfill the counter */
        assert(count > 0);

        weighted_ttbar_event(ipars, normpars, 1, event, &weight_qq, &weight_gg);
        weight_sum = weight_qq + weight_gg;
        if (*pweight > maxweight)
        {
            printf("WARNING in unweighted_ttbar_event (channel %d): w is %g, maxw is %g\n",
                   prod_channel, *pweight, maxweight);
            fflush(stdout);
        }
        if (uniform_random() < *pweight/maxweight)
            /* Accept this event */
            break;
    }

    if (prod_channel == PROD_CHANNEL_UNDEFINED)
    {
        if (uniform_random() < weight_qq/weight_sum)
            event->prod_channel = PROD_CHANNEL_QQ;
        else
            event->prod_channel = PROD_CHANNEL_GG;
    }
    else
        event->prod_channel = prod_channel;

    if (pweight_qq)
        *pweight_qq = weight_qq;
    if (pweight_gg)
        *pweight_gg = weight_gg;

    return count;
}

void topmass_norm(const integ_parameters *ipars,
                  const norm_parameters *normpars,
                  const int use_bw,
                  const unsigned nevents,
                  norm_accumulator *qq_norm,
                  norm_accumulator *gg_norm,
		  norm_accumulator *sum_norm)
{
    unsigned i;
    ttbar_event ev;
    double w_gg, w_qq;

    norm_reset(qq_norm);
    norm_reset(gg_norm);
    norm_reset(sum_norm);
    for (i=0; i<nevents; ++i)
    {
        weighted_ttbar_event(ipars, normpars, use_bw, &ev, &w_qq, &w_gg);
        norm_accumulate(qq_norm, w_qq);
        norm_accumulate(gg_norm, w_gg);
        norm_accumulate(sum_norm, w_gg + w_qq);
    }
}

/*
 * Note that the following is true (according to
 * how weighted_ttbar_phsp_point is called inside
 * weighted_ttbar_event function):
 *
 * tlep = event->t;
 * wlep = event->wplus;
 * blep = event->b;
 * l    = event->ebar;
 * nu   = event->nu;
 *
 * thad = event->tbar;
 * whad = event->wminus;
 * bhad = event->bbar;
 * q    = event->mu;
 * qbar = event->nubar;
 *
 * For events generated by Herwig, Herwig q maps into
 * event->nubar and Herwig qbar maps into event->mu.
 */

static double min_iso(v3_obj l, v3_obj q0, v3_obj q1, v3_obj q2, v3_obj q3)
{
    double c, mindiff = DBL_MAX;
    c = conediff(l, q0);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(l, q1);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(l, q2);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(l, q3);
    mindiff = (mindiff < c ? mindiff : c);
    return mindiff;
}

static double min_iso_3(v3_obj l, v3_obj q0, v3_obj q1, v3_obj q2)
{
    double c, mindiff = DBL_MAX;
    c = conediff(l, q0);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(l, q1);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(l, q2);
    mindiff = (mindiff < c ? mindiff : c);
    return mindiff;
}

static double min_cone(v3_obj q0, v3_obj q1, v3_obj q2, v3_obj q3)
{
    double c, mindiff = DBL_MAX;
    c = conediff(q0, q1);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q0, q2);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q0, q3);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q1, q2);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q1, q3);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q2, q3);
    mindiff = (mindiff < c ? mindiff : c);
    return mindiff;
}

static double min_cone_3(v3_obj q0, v3_obj q1, v3_obj q2)
{
    double c, mindiff = DBL_MAX;
    c = conediff(q0, q1);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q0, q2);
    mindiff = (mindiff < c ? mindiff : c);
    c = conediff(q1, q2);
    mindiff = (mindiff < c ? mindiff : c);
    return mindiff;
}

static int is_electron_accepted(const integ_parameters *ipars,
                                const norm_parameters *normpars,
                                const particle_obj e)
{
    return Pt(e.p) > normpars->electron_pt_min && 
           fabs(Eta(e.p)) < normpars->electron_eta_max;
}

static int is_muon_accepted(const integ_parameters *ipars,
                            const norm_parameters *normpars,
                            const particle_obj mu)
{
    return Pt(mu.p) > normpars->muon_pt_min && 
           fabs(Eta(mu.p)) < normpars->muon_eta_max;
}

static int is_lepton_accepted(const integ_parameters *ipars,
                              const norm_parameters *normpars,
                              const particle_obj l, const int isMu)
{
    if (isMu)
        return is_muon_accepted(ipars, normpars, l);
    else
        return is_electron_accepted(ipars, normpars, l);
}

static int is_jet_accepted(const integ_parameters *ipars,
                           const norm_parameters *normpars,
                           const particle_obj j, const int isB)
{
    return Pt(j.p) > normpars->jet_pt_cut &&
           fabs(Eta(j.p)) < normpars->jet_eta_cut;
}

int is_event_accepted(const integ_parameters *ipars,
                      const norm_parameters *normpars,
                      const particle_obj q, const particle_obj qbar,
                      const particle_obj bhad, const particle_obj blep,
                      const particle_obj l, const int isMu,
                      const particle_obj recoil)
{
    return is_3jet_event_accepted(ipars, normpars, q, qbar, bhad,
                                  blep, l, isMu, recoil, -1);
}

int is_3jet_event_accepted(const integ_parameters *ipars,
                           const norm_parameters *normpars,
                           const particle_obj q, const particle_obj qbar,
                           const particle_obj bhad, const particle_obj blep,
                           const particle_obj l, const int isMu,
                           const particle_obj recoil,
                           const int extra_jet)
{
    /* Lepton acceptance */
    const int l_acc = is_lepton_accepted(ipars, normpars, l, isMu);

    v3_obj pjet;
    double mincone, miniso;
    int opa_jet, opa_l, missEt_acc, jet_Pt_acc = 1;

    switch (extra_jet)
    {
    case Q:
        pjet = sum3(sum3(qbar.p,bhad.p),blep.p);
        mincone = min_cone_3(qbar.p, blep.p, bhad.p);
        miniso = min_iso_3(l.p, qbar.p, blep.p, bhad.p);
        break;
    case QBAR:
        pjet = sum3(sum3(q.p,bhad.p),blep.p);
        mincone = min_cone_3(q.p, blep.p, bhad.p);
        miniso = min_iso_3(l.p, q.p, blep.p, bhad.p);
        break;
    case BHAD:
        pjet = sum3(sum3(q.p,qbar.p),blep.p);
        mincone = min_cone_3(q.p, qbar.p, blep.p);
        miniso = min_iso_3(l.p, q.p, qbar.p, blep.p);
        break;
    case BLEP:
        pjet = sum3(sum3(q.p,qbar.p),bhad.p);
        mincone = min_cone_3(q.p, qbar.p, bhad.p);
        miniso = min_iso_3(l.p, q.p, qbar.p, bhad.p);
        break;
    default:
        pjet = sum3(sum3(sum3(q.p,qbar.p),bhad.p),blep.p);
        mincone = min_cone(q.p, qbar.p, blep.p, bhad.p);
        miniso = min_iso(l.p, q.p, qbar.p, blep.p, bhad.p);
        break;
    }

    opa_jet = mincone > normpars->cone_cut ? 1 : 0;
    opa_l = miniso > normpars->iso_cut ? 1 : 0;
    missEt_acc = Pt(sum3(sum3(pjet,l.p),recoil.p)) > 
        normpars->miss_et_cut ? 1 : 0;

    if (jet_Pt_acc && extra_jet != Q)
        jet_Pt_acc = is_jet_accepted(ipars, normpars, q, 0);
    if (jet_Pt_acc && extra_jet != QBAR)
        jet_Pt_acc = is_jet_accepted(ipars, normpars, qbar, 0);
    if (jet_Pt_acc && extra_jet != BHAD)
        jet_Pt_acc = is_jet_accepted(ipars, normpars, bhad, 1);
    if (jet_Pt_acc && extra_jet != BLEP)
        jet_Pt_acc = is_jet_accepted(ipars, normpars, blep, 2);

    return jet_Pt_acc * l_acc * missEt_acc * opa_jet * opa_l;
}

double parton_level_acceptance(const integ_parameters *ipars,
                               const norm_parameters *normpars,
                               const ttbar_event *ev)
{
/*     const double jet_Pt_acc = ipars->include_acceptance ?  */
/*         henris_acc_value(Pt(ev->mu.p), 0) * */
/*         henris_acc_value(Pt(ev->nubar.p), 0) * */
/*         henris_acc_value(Pt(ev->bbar.p), 1) * */
/*         henris_acc_value(Pt(ev->b.p), 2) : 1.0; */

    const double jet_Pt_acc = 1.0;
    const double e_acc = is_electron_accepted(ipars, normpars, ev->ebar);
    const double missEt_acc = Pt(ev->nu.p) > normpars->miss_et_cut ? 1.0 : 0.0;
    const double opa_jet = min_cone(ev->mu.p, ev->nubar.p, ev->bbar.p, ev->b.p) > 
        normpars->cone_cut ? 1.0 : 0.0;
    const double opa_l = min_iso(ev->ebar.p, ev->mu.p, ev->nubar.p, ev->bbar.p, ev->b.p) > 
        normpars->iso_cut ? 1.0 : 0.0;

    return jet_Pt_acc * e_acc * missEt_acc * opa_jet * opa_l;
}

int defudge_herwig(const particle_obj t, const particle_obj w0,
                   const particle_obj w, const particle_obj q0,
                   const particle_obj qbar0, particle_obj *q_tree,
                   particle_obj *qbar_tree)
{
    const boost_obj totop = rest_boost(t);
    const particle_obj w0_in_trest = boost(w0, totop);
    const particle_obj w_in_trest = boost(w, totop);
    const particle_obj q0_in_trest = boost(q0, totop);
    const particle_obj qbar0_in_trest = boost(qbar0, totop);
    
    /* w0 daughters in the w0 rest frame should look
     * like w daughters in the w rest frame
     */
    const boost_obj wboost = rest_boost(w_in_trest);
    const boost_obj backboost = inverse_boost(rest_boost(w0_in_trest));
    const particle_obj q1_in_trest = boost(boost(q0_in_trest, wboost), backboost);
    const particle_obj qbar1_in_trest = boost(boost(qbar0_in_trest, wboost), backboost);

    /* Boost back to the lab */
    const boost_obj fromtop = inverse_boost(totop);
    *q_tree = boost(q1_in_trest, fromtop);
    *qbar_tree = boost(qbar1_in_trest, fromtop);

    return 0;
}

int redecay_top_quark(const particle_obj q, const particle_obj qbar,
                      const particle_obj b, const double new_mq,
                      const double new_mqbar, const double new_mb,
                      particle_obj *new_q, particle_obj *new_qbar,
                      particle_obj *new_b)
{
    /* Boost all particles to the parent top rest frame */
    const particle_obj t = sum4(sum4(q, qbar), b);
    const boost_obj totop = rest_boost(t);
    const particle_obj q_in_trest = boost(q, totop);
    const particle_obj qbar_in_trest = boost(qbar, totop);
    const particle_obj b_in_trest = boost(b, totop);
    const particle_obj w_in_trest = sum4(q_in_trest, qbar_in_trest);
    const double mw = w_in_trest.m;

    if (new_mqbar + new_mq < mw && mw + new_mb < t.m)
    {
        const boost_obj to_w_in_trest = rest_boost(w_in_trest);

        /* Redecay the W into partons with new masses */
        const particle_obj q_in_wrest = boost(q_in_trest, to_w_in_trest);
        const double eq_cms = (mw*mw + new_mq*new_mq - new_mqbar*new_mqbar)/(2.0*mw);
        const double pq_cms = (new_mq == 0.0 ? eq_cms : sqrt(eq_cms*eq_cms - new_mq*new_mq));
        const v3_obj q_in_wrest_dir = direction3(q_in_wrest.p);
        const particle_obj newq_in_wrest = particle(mult3(q_in_wrest_dir, pq_cms), new_mq);
        const particle_obj newqbar_in_wrest = particle(mult3(q_in_wrest_dir, -pq_cms), new_mqbar);

        /* Redecay the top into b with new mass and W */
        const double eb_cms = (t.m*t.m + new_mb*new_mb - mw*mw)/(2.0*t.m);
        const double pb_cms = sqrt(eb_cms*eb_cms - new_mb*new_mb);
        const v3_obj b_in_trest_dir = direction3(b_in_trest.p);
        const particle_obj newb_in_trest = particle(mult3(b_in_trest_dir, pb_cms), new_mb);
        const particle_obj neww_in_trest = particle(mult3(b_in_trest_dir, -pb_cms), mw);

        /* neww daughters in the neww rest frame should look like
         * w_in_trest daughters in the w_in_trest rest frame
         */
        const boost_obj backboost = inverse_boost(rest_boost(neww_in_trest));
        const particle_obj newq_in_trest = boost(newq_in_wrest, backboost);
        const particle_obj newqbar_in_trest = boost(newqbar_in_wrest, backboost);

        /* Boost everything back to the lab */
        const boost_obj lab_boost = inverse_boost(totop);
        *new_q = boost(newq_in_trest, lab_boost);
        *new_qbar = boost(newqbar_in_trest, lab_boost);
        *new_b = boost(newb_in_trest, lab_boost);

        return 0;
    }
    else
    {
        memset(new_q, 0, sizeof(particle_obj));
        memset(new_qbar, 0, sizeof(particle_obj));
        memset(new_b, 0, sizeof(particle_obj));
        return 1;
    }
}

#ifdef __cplusplus
}
#endif
