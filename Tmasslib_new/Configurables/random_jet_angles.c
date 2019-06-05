#include <math.h>
#include <assert.h>

#include "random_jet_angles.h"
#include "topmass_utils.h"
#include "johnson_random.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEVIATION 0.7
#define MAX_PT 200.0
#define MIN_VALID_PT_LIGHT 10.0
#define MIN_VALID_PT_B 10.0

#define ONE5SQR5 3.3541019662496845446
#define SQR3     1.7320508075688772935

double johnsys_(double *X,double *Y,double *Z,
                int *MODE,double *DPAR,int *ERRSTA);

typedef struct {
    double a_sigma;
    double d_sigma;
    double f_sigma;
    double g_sigma;
    double a_kurt;
    double d_kurt;
    double f_kurt;
    double width_factor;
} Fit_parameters;

static Fit_parameters eta_parameters[2] = {
    {
        0.469163614724,
        -0.682184358899,
        0.0294410061506,
        0.007247351925,
        34.4939625032,
        -33.3057589444,
        50.0755048282,
        1.0
    },
    {
        0.335622613088,
        -0.996624485887,
        0.0203256585523,
        0.00969644507288,
        8.37891453925,
        -19.9014263675,
        24.5404410546,
        1.0
    }
};

static Fit_parameters phi_parameters[2] = {
    {
        0.452004254441,
        -1.13445537538,
        0.0168452969986,
        0.0170634998094,
        27.1900118964,
        -5.9928230161,
        42.3185092734,
        1.0
    },
    {
        0.272440287004,
        -1.29307426654,
        0.0145244918879,
        0.0143358237513,
        6.54308795587,
        -8.56548528351,
        17.566165594,
        1.0
    }
};

void set_eta_width_factor(double factor, int isB)
{
    const int index = isB ? 1 : 0;
    Fit_parameters *p = eta_parameters + index;
    assert(factor > 0.0);
    p->width_factor = factor;
}

void set_phi_width_factor(double factor, int isB)
{
    const int index = isB ? 1 : 0;
    Fit_parameters *p = phi_parameters + index;
    assert(factor > 0.0);
    p->width_factor = factor;
}

static void angular_parameters(const Fit_parameters *p_in, const int isB,
                               const double pt_in_0,
                               double *stdev, double *kurtosis)
{
    static const double minvalidpt[2] = {MIN_VALID_PT_LIGHT, MIN_VALID_PT_B};

    const int index = isB ? 1 : 0;
    const double pt_in = pt_in_0 < MAX_PT ? pt_in_0 : MAX_PT;
    const double pt = pt_in > minvalidpt[index] ? pt_in : minvalidpt[index];
    const Fit_parameters *p = p_in + index;
    const double twoxm1 = (2.0*pt/MAX_PT - 1.0);
    const double p1 = SQR3*twoxm1;
    const double p4 = ONE5SQR5*(twoxm1*twoxm1 - 1.0/3.0);

    *stdev = p->width_factor*(
        p->f_sigma*exp(p->a_sigma*p4 + p->d_sigma*p1) + p->g_sigma);
    if (kurtosis)
        *kurtosis = p->a_kurt*p4 + p->d_kurt*p1 + p->f_kurt;
}

static double angular_random(const Fit_parameters *p, const int isB,
                             const double pt)
{
    double x, stdev, kurt;
    angular_parameters(p, isB, pt, &stdev, &kurt);
    do {
        int status = johnson_random(0.0, stdev, 0.0, kurt,
                                    -MAX_DEVIATION, 0.0, &x, 1);
        assert(status == 0);
    } while (x > MAX_DEVIATION);
    return x;
}

static double bounded_johnson_random_fromrand(
    const double mean, const double sigma,
    const double skew, const double kurt,
    const double uniform_rnd,
    const double lolim, const double hilim)
{
    const double cdflo = johnson_cdf(lolim, mean, sigma, skew, kurt);
    const double cdfhi = johnson_cdf(hilim, mean, sigma, skew, kurt);
    assert(cdflo >= 0.0);
    assert(cdfhi >= 0.0);
    {
        const double mapped_cdf = cdflo + uniform_rnd*(cdfhi - cdflo);
        int status = 0;
        const double x = johnson_invcdf(mapped_cdf, mean, sigma,
                                        skew, kurt, &status);
        assert(status == 0);
        return x;
    }
}

static double angular_random_fromrand(const Fit_parameters *p, const int isB,
                                      const double pt, float rnd)
{
    double stdev, kurt;
    angular_parameters(p, isB, pt, &stdev, &kurt);
    return bounded_johnson_random_fromrand(0.0, stdev, 0.0, kurt, rnd,
                                           -MAX_DEVIATION, MAX_DEVIATION);
}

static double angular_density(const Fit_parameters *p, const int isB,
                              const double pt, const double delta)
{
    if (fabs(delta) > MAX_DEVIATION)
        return 0.0;
    else
    {
        double density, stdev, kurt, dpar[5];
        double x, y = 0.0, z = 0.0;
        int status = 0, mode = 0;

        angular_parameters(p, isB, pt, &stdev, &kurt);
        dpar[0] = 1.0;
        dpar[1] = 0.0;
        dpar[2] = stdev;
        dpar[3] = 0.0;
        dpar[4] = kurt;
        x = delta;

        density = johnsys_(&x, &y, &z, &mode, dpar, &status);
        assert(density >= 0.0);
        assert(status == 0);

        return density;
    }
}

double b_eta_error(double pt)
{
    double err;
    angular_parameters(eta_parameters, 1, pt, &err, 0);
    return err;
}
    
double b_phi_error(double pt)
{
    double err;
    angular_parameters(phi_parameters, 1, pt, &err, 0);
    return err;
}
    
double q_eta_error(double pt)
{
    double err;
    angular_parameters(eta_parameters, 0, pt, &err, 0);
    return err;
}

double q_phi_error(double pt)
{
    double err;
    angular_parameters(phi_parameters, 0, pt, &err, 0);
    return err;
}

double random_b_deta(double pt)
{
    return angular_random(eta_parameters, 1, pt);
}

double random_b_dphi(double pt)
{
    return angular_random(phi_parameters, 1, pt);
}

double random_q_deta(double pt)
{
    return angular_random(eta_parameters, 0, pt);
}

double random_q_dphi(double pt)
{
    return angular_random(phi_parameters, 0, pt);
}

/* v3_obj randomize_angles(const v3_obj orig, const int isB) */
/* { */
/*      typedef double (*randomizer)(double); */
/*      static const randomizer random_deta[2] = {random_q_deta, random_b_deta}; */
/*      static const randomizer random_dphi[2] = {random_q_dphi, random_b_dphi}; */

/*      const int index = isB ? 1 : 0; */
/*      const double pt = Pt(orig); */
/*      const double neweta = Eta(orig) + random_deta[index](pt); */
/*      const double newphi = Phi(orig) + random_dphi[index](pt); */
/*      const double newpt  = mom(orig) / cosh(neweta); */

/*      return pt_eta_phi(newpt, neweta, newphi); */
/* } */

v3_obj new_eta_phi_fromrand(v3_obj orig, int isB,
                            float rnd_eta, float rnd_phi)
{
    const double pt = Pt(orig);
    const double deta = angular_random_fromrand(eta_parameters, isB, pt, rnd_eta);
    const double dphi = angular_random_fromrand(phi_parameters, isB, pt, rnd_phi);
    const double neweta = Eta(orig) + deta;
    const double newpt  = mom(orig) / cosh(neweta);
    const double newphi = Phi(orig) + dphi;
    return pt_eta_phi(newpt, neweta, newphi);    
}

double deta_density(double pt, int isB, double deta)
{
    return angular_density(eta_parameters, isB, pt, deta);
}

double dphi_density(double pt, int isB, double dphi)
{
    return angular_density(phi_parameters, isB, pt, dphi);
}

#ifdef __cplusplus
}
#endif
