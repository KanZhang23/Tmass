#include <assert.h>
#include <math.h>
#include <string.h>

#include "johnson_random.h"
#include "sbshape.h"
#include "topmass_utils.h"
#include "cernlib_replacement.h"

#define PIOVERSQR12 0.9068996821171089

#ifdef __cplusplus
extern "C" {
#endif

int johnsel_(const double *SKEW, const double *KURT);
void jonpar_(const double *MEAN, const double *SIGMA, const double *SKEW,
             const double *KURT, double *XI, double *LAMBDA, double *GAMMA,
             double *DELTA, int *IERR);

static int lognormal_parameters(const double mean, const double sigma, 
                                const double skewness, double *x0,
                                double *mu, double *s)
{
    double b1, tmp, w, logw, emgamovd, xi;

    if (sigma <= 0.0)
        return 1;
    if (skewness == 0.0)
        return 1;
    b1 = skewness*skewness;
    tmp = pow((2.0+b1+sqrt(b1*(4.0+b1)))/2.0, 1.0/3.0);
    w = tmp+1.0/tmp-1.0;
    logw = log(w);
    if (logw <= 0.0)
        return 1;
    *s = sqrt(logw);
    emgamovd = sigma/sqrt(w*(w-1.0));
    xi = mean - emgamovd*sqrt(w);
    if (skewness > 0.0)
        *x0 = xi;
    else
        *x0 = xi - 2.0*mean;
    *mu = log(emgamovd);
    return 0;
}

double tanh_rise(double x, double locat, double w)
{
    const double width = fabs(w);
    if (width == 0.0)
    {
	if (x < locat)
	    return 0.0;
	else if (x > locat)
	    return 1.0;
	else
	    return 0.5;
    }
    return 0.5*(tanh((x-locat)/width*PIOVERSQR12) + 1.0);
}

int johnson_random(const double mean, const double sigma, const double skew,
                   const double kurt, const double locat, const double width,
		   double *rand, const int nrand)
{
    int i;

    if (nrand < 0)
        return 1;
    if (nrand == 0)
        return 0;
    assert(rand);
    if (sigma <= 0.0)
        return 1;
    switch (johnsel_(&skew, &kurt))
    {
    case 0:
        /* Impossible skewness and kurtosis */
        memset(rand, 0, nrand*sizeof(double));
        return 2;
    case 1:
    {
        /* Gaussian */
	double xra;
        for (i=0; i<nrand; ++i)
	{
	    do {
		xra = sigma*normal_random() + mean;
	    } while (uniform_random() > tanh_rise(xra, locat, width));
	    rand[i] = xra;
	}
    }
    break;
    case 2:
    {
        /* Lognormal */
        double x0, mu, s, xra;
        int stat = lognormal_parameters(mean, sigma, skew, &x0, &mu, &s);
        double sign = skew/fabs(skew);
        assert(stat == 0);
        for (i=0; i<nrand; ++i)
	{
	    do {
		xra = sign*(exp(s*normal_random() + mu) + x0);
	    } while (uniform_random() > tanh_rise(xra, locat, width));
            rand[i] = xra;
	}
    }
    break;
    case 3:
    {
        /* Johnson's S_b */
        int ierr = 0;
        double xi, lambda, gamma, delta, tmp, xra;
        sbfitmod_(0, &mean, &sigma, &skew, &kurt,
                  &gamma, &delta, &lambda, &xi, &ierr);
        assert(ierr == 0);
        for (i=0; i<nrand; ++i)
        {
	    do {
		tmp = exp((normal_random() - gamma)/delta);
		xra = lambda*tmp/(tmp + 1.0) + xi;
	    } while (uniform_random() > tanh_rise(xra, locat, width));
            rand[i] = xra;
        }
    }
    break;
    case 4:
    {
        /* Johnson's S_u */
        int ierr = 0;
        double xi, lambda, gamma, delta, xra;
        jonpar_(&mean, &sigma, &skew, &kurt,
                &xi, &lambda, &gamma, &delta, &ierr);
        assert(ierr == 0);
        for (i=0; i<nrand; ++i)
	{
	    do {
		xra = lambda*sinh((normal_random() - gamma)/delta) + xi;
	    } while (uniform_random() > tanh_rise(xra, locat, width));
	    rand[i] = xra;
	}
    }
    break;
    default:
        assert(0);
    }
    return 0;
}

double johnson_invcdf(double cdf, const double mean,
                      const double sigma, const double skew,
                      const double kurt, int *status)
{
    assert(status);
    *status = 0;

    if (cdf <= 0.0 || cdf >= 1.0)
    {
        *status = 1;
        return 0.0;
    }

    if (sigma <= 0.0)
    {
        *status = 2;
        return 0.0;
    }

    switch (johnsel_(&skew, &kurt))
    {
    case 0:
        /* Impossible skewness and kurtosis */
        *status = 3;
        return 0.0;
    case 1:
        /* Gaussian */
        return sigma*cr_dgausn(&cdf) + mean;
    case 2:
    {
        /* Lognormal */
        double x0, mu, s;
        int stat = lognormal_parameters(mean, sigma, skew, &x0, &mu, &s);
        double sign = skew/fabs(skew);
        assert(stat == 0);
        return sign*(exp(s*cr_dgausn(&cdf) + mu) + x0);
    }
    case 3:
    {
        /* Johnson's S_b */
        int ierr = 0;
        double xi, lambda, gamma, delta, tmp;
        sbfitmod_(0, &mean, &sigma, &skew, &kurt,
                  &gamma, &delta, &lambda, &xi, &ierr);
        assert(ierr == 0);
        tmp = exp((cr_dgausn(&cdf) - gamma)/delta);
        return lambda*tmp/(tmp + 1.0) + xi;
    }
    case 4:
    {
        /* Johnson's S_u */
        int ierr = 0;
        double xi, lambda, gamma, delta;
        jonpar_(&mean, &sigma, &skew, &kurt,
                &xi, &lambda, &gamma, &delta, &ierr);
        assert(ierr == 0);
        return lambda*sinh((cr_dgausn(&cdf) - gamma)/delta) + xi;
    }
    default:
        assert(0);
    }
}

double johnson_cdf(const double x, const double mean,
                   const double sigma, const double skew, const double kurt)
{
    double cdf = -1.0, diff;

    if (sigma <= 0.0)
        return cdf;

    switch (johnsel_(&skew, &kurt))
    {
    case 0:
        /* Impossible combination of skewness and kurtosis */
        break;
    case 1:
    {
        /* Gaussian */
        diff = (x - mean)/sigma;
        cdf = cr_dfreq(&diff);
    }
    break;
    case 2:
    {
        /* Lognormal */
        double x0, mu, s;
        int stat = lognormal_parameters(mean, sigma, skew, &x0, &mu, &s);
        double sign = skew/fabs(skew);
        assert(skew != 0.0);
        assert(stat == 0);
        diff = x*sign - x0;
        if (diff <= 0.0)
            cdf = (1.0 - sign)*0.5;
        else
        {
            diff = sign*(log(diff) - mu)/s;
            cdf = cr_dfreq(&diff);
        }
    }
    break;
    case 3:
    {
        /* Johnson's S_b */
        int ierr = 0;
        double xi, lambda, gamma, delta, tmp;
        sbfitmod_(0, &mean, &sigma, &skew, &kurt,
                  &gamma, &delta, &lambda, &xi, &ierr);
        assert(ierr == 0);
        if (x <= xi)
            cdf = 0.0;
        else if (x >= xi+lambda)
            cdf = 1.0;
        else
        {
            diff = (x - xi)/lambda;
            tmp = diff/(1.0 - diff);
            diff = delta*log(tmp) + gamma;
            cdf = cr_dfreq(&diff);
        }
    }
    break;
    case 4:
    {
        /* Johnson's S_u */
        int ierr = 0;
        double xi, lambda, gamma, delta;
        jonpar_(&mean, &sigma, &skew, &kurt,
                &xi, &lambda, &gamma, &delta, &ierr);
        assert(ierr == 0);
        diff = delta*asinh((x - xi)/lambda) + gamma;
        cdf = cr_dfreq(&diff);
    }
    break;
    default:
        assert(0);
    }
    return cdf;
}

#ifdef __cplusplus
}
#endif
