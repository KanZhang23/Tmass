#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>

#include "leptonic_side_mask.h"
#include "solve_top.h"
#include "random_parton_mass.h"
#include "transfer_function.h"
#include "single_parton_efficiency.h"

#define MIN_MT_MW_DIFFERENCE_LEP 0.5

#ifdef __cplusplus
extern "C" {
#endif

void init_leptonic_side_mask(Leptonic_side_mask *mask, double mt_min,
                             double mt_max, double pt_max, unsigned nMt,
                             unsigned nPt, unsigned nPhi)
{
    const unsigned len = nMt*nPt*nPhi*sizeof(char);
    assert(mask);
    assert(len);
    mask->data = (char *)malloc(len);
    assert(mask->data);
    memset(mask->data, 0, len);

    mask->mt_min = mt_min;
    mask->mt_max = mt_max;
    mask->pt_max = pt_max;
    mask->nMt = nMt;
    mask->nPt = nPt;
    mask->nPhi = nPhi;
}

void cleanup_leptonic_side_mask(Leptonic_side_mask *mask)
{
    assert(mask);
    if (mask->data)
    {
        free(mask->data);
        mask->data = 0;
    }
}

char leptonic_side_mask_value(const Leptonic_side_mask *mask, double mt,
                              double pt, double phi)
{
    assert(mask);
    {
        int iphi;
        const int ipt = pt/mask->pt_max*mask->nPt;
        const int imt = (mt - mask->mt_min)/(mask->mt_max - mask->mt_min)*mask->nMt;
        if (ipt < 0 || ipt >= (int)mask->nPt)
            return 0;
        if (imt < 0 || imt >= (int)mask->nMt)
            return 0;
        if (phi < 0.0)
            phi += 2.0*M_PI;
        if (phi >= 2.0*M_PI)
            phi -= 2.0*M_PI;
        iphi = phi/(2.0*M_PI)*mask->nPhi;
        assert(iphi >= 0 && iphi < (int)mask->nPhi);
        return mask->data[(imt*mask->nPt + ipt)*mask->nPhi + iphi];
    }
}

char leptonic_side_mask_inbin(const Leptonic_side_mask *mask, unsigned imt,
                              unsigned ipt, unsigned iphi)
{
    assert(mask);
    assert(imt < mask->nMt);
    assert(ipt < mask->nPt);
    assert(iphi < mask->nPhi);
    return mask->data[(imt*mask->nPt + ipt)*mask->nPhi + iphi];
}

void set_leptonic_side_mask(Leptonic_side_mask *mask, double mt,
                            double pt, double phi, char value)
{
    assert(mask);
    {
        int iphi;
        const int ipt = pt/mask->pt_max*mask->nPt;
        const int imt = (mt - mask->mt_min)/(mask->mt_max - mask->mt_min)*mask->nMt;
        if (ipt < 0 || ipt >= (int)mask->nPt)
            return;
        if (imt < 0 || imt >= (int)mask->nMt)
            return;
        if (phi < 0.0)
            phi += 2.0*M_PI;
        if (phi >= 2.0*M_PI)
            phi -= 2.0*M_PI;
        iphi = phi/(2.0*M_PI)*mask->nPhi;
        assert(iphi >= 0 && iphi < (int)mask->nPhi);
        mask->data[(imt*mask->nPt + ipt)*mask->nPhi + iphi] = value;
    }
}

double leptonic_side_masked_fraction(const Leptonic_side_mask *mask)
{
    assert(mask);
    {
        unsigned i, counter=0, nbins=mask->nMt*mask->nPt*mask->nPhi;
        for (i=0; i<nbins; ++i)
            if (mask->data[i])
                ++counter;
        return (double)counter/nbins;
    }
}

void clear_leptonic_side_mask(Leptonic_side_mask *mask)
{
    assert(mask);
    assert(mask->data);
    memset(mask->data, 0, mask->nMt*mask->nPt*mask->nPhi*sizeof(char));
}

static int compare_double_decr(const double *p1, const double *p2)
{
    if (*p1 < *p2)
        return 1;
    else if (*p1 > *p2)
        return -1;
    else
        return 0;
}

/*
 * This function returns maximum possible value of partonPT/jetPT.
 * This is the inverse of such u = jetPT/partonPT that the cumulative
 * transfer function of u equals "size_of_neglected_tf_low_tail".
 */
static double max_tf_scale(
    const double bPx, const double bPy, const double bPz, const double mb_in,
    const double size_of_neglected_tf_low_tail,
    const unsigned randomize_b_mass, const double cuterr)
{
    const double deltaJESToUse = 0.0;
    const double max_scale_cutoff = 10000.0;

    double masslist[5], maxscale[5];
    unsigned imass, mlist_len;

    const double bPt = hypot(bPx, bPy);
    const double target_cdf = 1.0 - size_of_neglected_tf_low_tail;
    const double precision = 0.01*size_of_neglected_tf_low_tail;

    if (size_of_neglected_tf_low_tail == 0.0)
        return max_scale_cutoff;

    assert(size_of_neglected_tf_low_tail > 0.0 &&
           size_of_neglected_tf_low_tail < 0.5);

    if (randomize_b_mass)
    {
        masslist[0] = random_b_mass_fromrand(0.01);
        masslist[1] = random_b_mass_fromrand(0.25);
        masslist[2] = random_b_mass_fromrand(0.5);
        masslist[3] = random_b_mass_fromrand(0.75);
        masslist[4] = random_b_mass_fromrand(0.99);
        mlist_len = 5;
    }
    else
    {
        masslist[0] = mb_in;
        mlist_len = 1;
    }

    for (imass=0; imass<mlist_len; ++imass)
    {
        const particle_obj b_in = particle(v3(bPx, bPy, bPz), masslist[imass]);
        const double b_in_eta = Eta(b_in.p);

        /* Bound the root using cdf_lo and cdf_hi */
        double cdf_lo = transfer_function_efficiency_2(b_in, b_in_eta, 2, bPt) *
                        single_parton_eff(b_in, 2, deltaJESToUse, cuterr);

        particle_obj b;
        double pfactor_lo = 1.0, pfactor_hi = 1.0, cdf_hi = cdf_lo;
        int below_max_scale = 1;

        assert(target_cdf >= cdf_hi);
        while (target_cdf >= cdf_hi && below_max_scale)
        {
            pfactor_hi *= 2.0;
            if (pfactor_hi > max_scale_cutoff)
            {
                pfactor_hi = max_scale_cutoff;
                below_max_scale = 0;
            }
            b = particle(mult3(b_in.p, pfactor_hi), b_in.m);
            cdf_hi = transfer_function_efficiency_2(b, b_in_eta, 2, bPt) *
                     single_parton_eff(b, 2, deltaJESToUse, cuterr);
        }

        if (target_cdf < cdf_hi)
        {
            unsigned cnt = 0;
            for (; cnt < 200U && cdf_hi - cdf_lo > precision; ++cnt)
            {
                double cdf;
                const double tryfactor = (pfactor_lo + pfactor_hi)/2.0;
                b = particle(mult3(b_in.p, tryfactor), b_in.m);
                cdf = transfer_function_efficiency_2(b, b_in_eta, 2, bPt) *
                      single_parton_eff(b, 2, deltaJESToUse, cuterr);
                if (cdf > target_cdf)
                {
                    cdf_hi = cdf;
                    pfactor_hi = tryfactor;
                }
                else
                {
                    cdf_lo = cdf;
                    pfactor_lo = tryfactor;
                }
            }
            maxscale[imass] = (pfactor_lo + pfactor_hi)/2.0;
        }
        else
            return max_scale_cutoff;
    }

    if (mlist_len > 1)
        qsort(maxscale, mlist_len, sizeof(double),
              (int (*)(const void *, const void *))compare_double_decr);
    return maxscale[0];
}

/* The following function provides roughly 99.9% blep Pt coverage
 * in the top mass range from 110 to 270 GeV.
 */
static double max_blep_pt(const double mt)
{
    return -161.5 + 3.1313803196*mt - 0.00467437086627*mt*mt;
}

/* The following function _does not_ generate a flat cone */
static v3_obj randomize_direction_in_cone(
    const v3_obj p, const double radius,
    const float r_eta, const float r_phi)
{
    const double eta0 = Eta(p);
    const double phi0 = Phi(p);
    const double r = radius*r_eta;
    const double phi = 2*M_PI*r_phi;
    const double neweta = eta0 + r*cos(phi);
    const double newphi = phi0 + r*sin(phi);
    const double newpt = mom(p)/cosh(neweta);
    return pt_eta_phi(newpt, neweta, newphi);
}

/* The following quantities may be randomized:
 *   mw, nuPz, p_b, eta_b, phi_b, m_b
 */
#define MAX_RAND_DIM 6

unsigned populate_leptonic_side_mask(
    Leptonic_side_mask *mask,
    const N_d_random_method rand_method, const int random_gen_param,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mb_in, const double cuterr,
    const double nuPz_min, const double nuPz_max,
    const double w_mass, const double w_width,
    const double size_of_neglected_tf_low_tail,
    const double b_random_cone_radius, const unsigned randomize_b_mass,
    unsigned max_points, const unsigned max_seconds)
{
    const double min_scale = 0.5;
    double nuz, mb, mwsq;
    const double nuz_range = nuPz_max - nuPz_min;
    const double peak = w_mass*w_mass;
    const double hwhm = w_mass*w_width;
    const unsigned randomize_b_angle = (b_random_cone_radius > 0.0);
    time_t t_check = time(NULL);
    const time_t t_end = t_check + (time_t)max_seconds;
    unsigned i, imt, points_used, time_check_period = 1000;
    float r[MAX_RAND_DIM];
    float *r_mw = &r[0], *r_nuPz = &r[1], *r_p_b = &r[2];
    float *r_eta = NULL, *r_phi = NULL, *r_m = NULL;

    if (max_points == 0)
        max_points = UINT_MAX;
    clear_leptonic_side_mask(mask);
    {
        const double mt_bw = (mask->mt_max - mask->mt_min)/mask->nMt;
        const v3_obj pb = v3(bPx, bPy, bPz);
        const double bPt = Pt(pb);

        /* The following assumes use of p/E transfer functions */
        const double global_max_scale = max_tf_scale(
            bPx, bPy, bPz, mb_in,
            size_of_neglected_tf_low_tail,
            randomize_b_mass, cuterr);

        /* Will always randomize mw, nuPz, and p_b */
        unsigned n_rand_dim = 3;
        if (randomize_b_angle)
        {
            r_eta = &r[n_rand_dim++];
            r_phi = &r[n_rand_dim++];
        }
        if (randomize_b_mass)
            r_m = &r[n_rand_dim++];
        assert(n_rand_dim <= MAX_RAND_DIM);

        init_n_d_random(rand_method, n_rand_dim, random_gen_param);

        for (i=0, points_used=0; points_used<max_points; ++points_used, ++i)
        {
            next_n_d_random(r);

            if (*r_mw > 0.f && *r_mw < 1.f)
                mwsq = hwhm*tan(M_PI*(*r_mw - 0.5)) + peak;
            else
                mwsq = 0.0;
            if (mwsq <= 0.0)
                continue;

            nuz = nuPz_min + *r_nuPz*nuz_range;

            if (r_m)
                mb = random_b_mass_fromrand(*r_m);
            else
                mb = mb_in;

            for (imt=0; imt<mask->nMt; ++imt)
            {
                const double mt = mask->mt_min + mt_bw*(imt + 0.5);
                const double mtlow = mt - mb - MIN_MT_MW_DIFFERENCE_LEP;
                if (mwsq < mtlow*mtlow)
                {
                    const double maxbPt = max_blep_pt(mt);
                    const double max_scale_from_pt = maxbPt/bPt > 1.5 ? maxbPt/bPt : 1.5;
                    const double max_scale = global_max_scale < max_scale_from_pt ?
                                             global_max_scale : max_scale_from_pt;
                    if (max_scale >= min_scale)
                    {
                        const double scale = min_scale + *r_p_b*(max_scale - min_scale);
                        v3_obj new_pb = mult3(pb, scale);

                        double topPx[4], topPy[4];
                        int j, nsols;

                        if (randomize_b_angle)
                            new_pb = randomize_direction_in_cone(
                                new_pb, b_random_cone_radius, *r_eta, *r_phi);

                        nsols = solve_leptonic_for_top_pt(
                            lPx, lPy, lPz, new_pb.x, new_pb.y, new_pb.z,
                            mt, mb, mwsq, nuz, topPx, topPy);
                        for (j=0; j<nsols; ++j)
                            set_leptonic_side_mask(mask, mt, hypot(topPx[j],topPy[j]),
                                                   atan2(topPy[j],topPx[j]), 1);
                    }
                }
            }

            if (i >= time_check_period)
            {
                const time_t t_now = time(NULL);
                if (t_check == t_now)
                    time_check_period *= 2;
                t_check = t_now;
                i = 0;                
                if (t_now >= t_end)
                {
                    ++points_used;
                    break;
                }
            }
        }
    }

    return points_used;
}

#ifdef __cplusplus
}
#endif
