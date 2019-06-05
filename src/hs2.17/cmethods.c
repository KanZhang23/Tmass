#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "fit_function.h"

#define SQR2PI 2.506628274631
#define SQR2 1.414213562373095
#define PIOVERSQR12 0.9068996821171089
#define PI 3.14159265358979323846
#define SQRPIOVER2 1.2533141373155

Minuit_c_fit_function expthresh;
Minuit_c_fit_function exp_pdf;
Minuit_c_fit_function gausian_pdf;
Minuit_c_fit_function bifurcated_gaussian;
Minuit_c_fit_function threshold_erf;
Minuit_c_fit_function cauchy_pdf;
Minuit_c_fit_function bifurcated_cauchy;
Minuit_c_fit_function threshold_atan;
Minuit_c_fit_function sechsquared_pdf;
Minuit_c_fit_function threshold_tanh;
Minuit_c_fit_function poly_curve;
Minuit_c_fit_function poly_chebyshev;
Minuit_c_fit_function poly_legendre;
Minuit_c_fit_function linear_pdf;
Minuit_c_fit_function bivariate_gaussian;
Minuit_c_fit_function trivariate_gaussian;
Minuit_c_fit_function null_fit_fun;
Minuit_c_fit_function unity_fit_fun;
Minuit_c_fit_function const_fit_fun;
Minuit_c_fit_function x_fit_fun;
Minuit_c_fit_function weibull_pdf;
Minuit_c_fit_function linear_1d_fit_fun;
Minuit_c_fit_function linear_2d_fit_fun;
Minuit_c_fit_function linear_3d_fit_fun;
Minuit_c_fit_function quadratic_2d_fit_fun;
Minuit_c_fit_function huber_pdf;
Minuit_c_fit_function gumbel_pos_pdf;
Minuit_c_fit_function gumbel_neg_pdf;

Minuit_futil(expthresh)
{
    /* Needs 4 parameters: H_l, H_r, start, and width */
    double diff, width;

    diff = x-pars[2];
    width = fabs(pars[3]);
    if (width == 0.0) {
	if (diff < 0.0) {
            return pars[0];
	} else if (diff > 0.0) {
	    return pars[1];
	} else {
	    return (pars[0]+pars[1])/2.0;
	}
    } else if (diff > 0.0) {
	return pars[0]+(pars[1]-pars[0])*(1.0-exp(-diff/width));
    } else {
	return pars[0];
    }
}

Minuit_futil(exp_pdf)
{
    /* Needs 3 parameters: area, x0, width */
    double x0, width;

    x0 = pars[1];
    width = fabs(pars[2]);
    if (x < x0 || width <= 0.0)
	return 0.0;
    return pars[0]/width*exp(-(x-x0)/width);
}

Minuit_futil(trivariate_gaussian)
{
    /* Needs 10 parameters: volume, mean_x, mean_y, mean_z, 
     * sigma_x, sigma_y, sigma_z, rho_xy, rho_xz, rho_yz
     */
    double sx, sy, sz;
    double rho_xy, rho_xz, rho_yz;
    double denom, dx, dy, dz, tmp;

    sx     = fabs(pars[4]);
    sy     = fabs(pars[5]);
    sz     = fabs(pars[6]);
    if (sx <= 0.0 || sy <= 0.0 || sz <= 0.0)
	return 0.0;
    dx     = (x - pars[1])/sx;
    dy     = (y - pars[2])/sy;
    dz     = (z - pars[3])/sz;
    rho_xy = pars[7];
    rho_xz = pars[8];
    rho_yz = pars[9];
    if (rho_xy <= -1.0 || rho_xy >= 1.0 ||
	rho_xz <= -1.0 || rho_xz >= 1.0 ||
	rho_yz <= -1.0 || rho_yz >= 1.0)
	return 0.0;
    denom  = (1.0 + rho_xy*rho_yz*rho_xz - rho_xy*rho_xy - rho_yz*rho_yz - rho_xz*rho_xz);
    if (denom <= 0.0)
	return 0.0;
    tmp    = exp(-0.5/denom*(dx*dx*(1.0 - rho_yz*rho_yz) +
			     dy*dy*(1.0 - rho_xz*rho_xz) +
			     dz*dz*(1.0 - rho_xy*rho_xy) +
			     2.0*dx*dy*(rho_yz*rho_xz - rho_xy) +
			     2.0*dy*dz*(rho_xy*rho_xz - rho_yz) +
			     2.0*dx*dz*(rho_xy*rho_yz - rho_xz)));
    return pars[0]*tmp/((2.0*PI)*SQR2PI*sx*sy*sz*sqrt(denom));
}

Minuit_futil(bivariate_gaussian)
{
    /* Needs 6 parameters: volume, mean_x, mean_y, sigma_x, sigma_y, rho */
    double mean_x, mean_y, sigma_x, sigma_y, rho, denom;
    double dx, dy, tmp;

    mean_x = pars[1];
    mean_y = pars[2];
    sigma_x = fabs(pars[3]);
    sigma_y = fabs(pars[4]);
    rho = pars[5];

    denom = 1.0 - rho*rho;
    if (sigma_x <= 0.0 || sigma_y <= 0.0 || denom <= 0.0)
	return 0.0;
    dx = (x - mean_x)/sigma_x;
    dy = (y - mean_y)/sigma_y;
    tmp = dx*dx + dy*dy - 2.0*rho*dx*dy;
    return (pars[0]/2.0/PI/sigma_x/sigma_y/sqrt(denom)*exp(-tmp/2.0/denom));
}

Minuit_futil(linear_pdf)
{
    /* Linear pdf needs 4 parameters: norm, center, base, and log(H_l/H_r) */
    double diff, base, logratio, meanh, heightratio;
    const double biglog = log(DBL_MAX);

    base = fabs(pars[2]);
    if (base == 0.0)
	return 0.0;
    diff = (x - pars[1])/base;
    if (fabs(diff) > 0.5)
	return 0.0;
    meanh = pars[0]/base;
    logratio = pars[3];

    if (logratio == 0.0)
    {
	/* This is a uniform distribution */
	return meanh;
    }
    else if (logratio <= -biglog)
    {
	/* This is a triangle distribution   /|
         * which looks like this:           /_|
         */
	return 2.0*meanh*(0.5 + diff);
    }
    else if (logratio >= biglog)
    {
	/* This is a triangle distribution  |\
         * which looks like this:           |_\
         */
	return 2.0*meanh*(0.5 - diff);
    }
    /* This distribution     /|
     * looks like this:     |_|
     */
    heightratio = exp(logratio);
    return 2.0*meanh*(0.5 + diff*(1.0-heightratio)/(1.0+heightratio));
}

Minuit_futil(huber_pdf)
{
    static double lasta = -1.0;
    static double norm = 1.0;
    double sigma, a, absdiff;

/*      norm  =  pars[0];  */
/*      mean  =  pars[1];  */
/*      sigma = |pars[2]|; */
/*      a     = |pars[3]|; */
    sigma = fabs(pars[2]);
    a = fabs(pars[3]);
    if (sigma == 0.0 || a == 0.0)
	return 0.0;
    if (a != lasta)
    {
	lasta = a;
	norm = 0.5/(exp(-a*a/2.0)/a + SQRPIOVER2*erf(a/SQR2));
    }
    absdiff = fabs((x - pars[1])/sigma);
    if (absdiff < a)
	return pars[0]*norm*exp(-absdiff*absdiff/2.0)/sigma;
    else
	return pars[0]*norm*exp(-a*absdiff+a*a/2.0)/sigma;
}

Minuit_futil(gaussian_pdf)
{
    double sigma, diff;

/*      norm = pars[0]; */
/*      mean = pars[1]; */
    sigma = fabs(pars[2]);
    if (sigma == 0.0)
	return 0.0;
    diff = (x - pars[1])/sigma;
    return (pars[0]/SQR2PI/sigma*exp(-diff*diff/2.0));
}

Minuit_futil(bifurcated_cauchy)
{
    /* Bifurcated Cauchy needs 4 parameters:
     * norm, peak, hwhm_l, and hwhm_r
     */
    double diff, peak, sigma_l, sigma_r;

    peak = pars[1];
    sigma_l = pars[2];
    sigma_r = pars[3];
    if ((sigma_l == 0.0 && sigma_r == 0.0) || 
	sigma_l < 0.0 || sigma_r < 0.0)
	return 0.0;

    if (x == peak)
    {	
	diff = 0.0;
    }
    else if (x > peak)
    {
	if (sigma_r)
	    diff = (x - peak)/sigma_r;
	else
	    return 0.0;
    }
    else
    {
	if (sigma_l)
	    diff = (x - peak)/sigma_l;
	else
	    return 0.0;
    }
    return pars[0]/PI*2.0/(sigma_l+sigma_r)/(1.0+diff*diff);
}

Minuit_futil(bifurcated_gaussian)
{
    /* Bifurcated Gaussian needs 4 parameters:
     * norm, peak, sigma_l, and sigma_r
     */
    double diff, peak, sigma_l, sigma_r;

    peak = pars[1];
    sigma_l = pars[2];
    sigma_r = pars[3];
    if ((sigma_l == 0.0 && sigma_r == 0.0) || 
	sigma_l < 0.0 || sigma_r < 0.0)
	return 0.0;
    
    if (x == peak)
    {	
	diff = 0.0;
    }
    else if (x > peak)
    {
	if (sigma_r)
	    diff = (x - peak)/sigma_r;
	else
	    return 0.0;
    }
    else
    {
	if (sigma_l)
	    diff = (x - peak)/sigma_l;
	else
	    return 0.0;
    }
    return pars[0]*exp(-diff*diff/2.0)/(sigma_r+sigma_l)/SQR2PI*2.0;
}

Minuit_futil(threshold_tanh)
{
    /* Tanh threshold needs 4 parameters: H_l, H_r, locat, and width */
    double locat, width;
    
    locat = pars[2];
    width = fabs(pars[3]);
    if (width == 0.0)
    {
	if (x < locat)
	    return pars[0];
	else if (x > locat)
	    return pars[1];
	else
	    return (pars[0]+pars[1])/2.0;
    }
    return pars[0] + (pars[1]-pars[0])/2.0*
	(tanh((x-locat)/width*PIOVERSQR12) + 1.0);
}

Minuit_futil(threshold_atan)
{
    /* Atan threshold needs 4 parameters: H_l, H_r, locat, and width */
    double locat, width;

    locat = pars[2];
    width = fabs(pars[3]);
    if (width == 0.0)
    {
	if (x < locat)
	    return pars[0];
	else if (x > locat)
	    return pars[1];
	else
	    return (pars[0]+pars[1])/2.0;
    }
    return pars[0] + (pars[1]-pars[0])*(0.5+atan((x-locat)/width)/PI);
}

Minuit_futil(threshold_erf)
{
    /* Erf threshold needs 4 parameters: H_l, H_r, locat, and width */
    double locat, width;
    
    locat = pars[2];
    width = fabs(pars[3]);
    if (width == 0.0)
    {
	if (x < locat)
	    return pars[0];
	else if (x > locat)
	    return pars[1];
	else
	    return (pars[0]+pars[1])/2.0;
    }
    return pars[0] + (pars[1]-pars[0])*(1.0+erf((x-locat)/width/SQR2))/2.0;
}

Minuit_futil(poly_legendre)
{
    /* Legendre polynomial needs at least 3 parameters: min, max, and a0 */
    double di, diff, dmin, dmax, range, result, pminus2, pminus1, p;
    int i, degree;

    dmin = pars[0];
    dmax = pars[1];
    range = (dmax - dmin);
    if (range <= 0.0)
	return 0.0;
    degree = mode - 3;
    if (degree < 0)
	return 0.0;
    diff = 2.0*(x-dmin)/range - 1.0;
    if (diff < -1.0 || diff > 1.0)
	return 0.0;
    result = pars[2];
    if (degree == 0)
	return result;
    result += pars[3]*diff;
    if (degree == 1)
	return result;
    pminus2 = 1.0;
    pminus1 = diff;
    for (i=2; i<=degree; ++i)
    {
	di = (double)i;
	p = ((2.0*di-1.0)*diff*pminus1 - (di-1.0)*pminus2)/di;
	result += p*pars[i+2];
	pminus2 = pminus1;
	pminus1 = p;
    }
    return result;
}

Minuit_futil(poly_chebyshev)
{
    /* Chebyshev polynomial needs at least 3 parameters: min, max, and a0 */
    double diff, dmin, dmax, range, result, pminus2, pminus1, p;
    int i, degree;

    dmin = pars[0];
    dmax = pars[1];
    range = (dmax - dmin);
    if (range <= 0.0)
	return 0.0;
    degree = mode - 3;
    if (degree < 0)
	return 0.0;
    diff = 2.0*(x-dmin)/range - 1.0;
    if (diff < -1.0 || diff > 1.0)
	return 0.0;
    result = pars[2];
    if (degree == 0)
	return result;
    result += pars[3]*diff;
    if (degree == 1)
	return result;
    pminus2 = 1.0;
    pminus1 = diff;
    for (i=2; i<=degree; ++i)
    {
	p = 2.0*diff*pminus1 - pminus2;
	result += p*pars[i+2];
	pminus2 = pminus1;
	pminus1 = p;
    }
    return result;
}

Minuit_futil(poly_curve)
{
    /* Polynomial needs at least 2 parameters: origin and a0 */
    double diff, result;
    int i;
    
    diff = x-pars[0];
    result = pars[mode-1];
    for (i=mode-2; i>0; --i)
	result = result*diff + pars[i];
    return result;
}

Minuit_futil(cauchy_pdf)
{
    /* Cauchy pdf needs 3 parameters: norm, mean, and hwhm */
    double sigma, diff;

    sigma = fabs(pars[2]);
    if (sigma == 0.0)
	return 0.0;
    diff = (x - pars[1])/sigma;
    return pars[0]/PI/sigma/(1.0+diff*diff);
}

Minuit_futil(sechsquared_pdf)
{
    /* Sech-squared pdf needs 3 parameters: norm, mean, and sigma */
    double sigma, diff, tmp;

/*      norm = pars[0]; */
/*      mean = pars[1]; */
    sigma = fabs(pars[2]);
    if (sigma == 0.0)
	return 0.0;
    diff = PIOVERSQR12*(x - pars[1])/sigma;
    tmp = cosh(diff);
    return PIOVERSQR12/2.0/sigma*pars[0]/tmp/tmp;
}

Minuit_futil(null_fit_fun)
{
    return 0.0;
}

Minuit_futil(unity_fit_fun)
{
    return 1.0;
}

Minuit_futil(const_fit_fun)
{
    return pars[0];
}

Minuit_futil(x_fit_fun)
{
    return x;
}

Minuit_futil(weibull_pdf)
{
    /* Need 4 parameters: area, x0, scale, and power */
    double scale, power, diff;

/*      x0 = pars[1]; */
    scale = fabs(pars[2]);
    if (scale == 0.0)
	return 0.0;
    power = pars[3];
    if (power == 0.0)
	return 0.0;
    diff = (x - pars[1])/scale;
    if (diff <= 0.0)
	return 0.0;
    return pars[0]*fabs(power)/scale*pow(diff,power-1.0)*exp(-pow(diff,power));
}

Minuit_futil(linear_1d_fit_fun)
{
    return pars[0]*x + pars[1];
}

Minuit_futil(linear_2d_fit_fun)
{
    return pars[0]*x + pars[1]*y + pars[2];
}

Minuit_futil(quadratic_2d_fit_fun)
{
    return pars[0]*x*x + pars[1]*x*y + pars[2]*y*y + 
	pars[3]*x + pars[4]*y + pars[5];
}

Minuit_futil(linear_3d_fit_fun)
{
    return pars[0]*x + pars[1]*y + pars[2]*z + pars[3];
}

Minuit_futil(gumbel_pos_pdf)
{
    double b, q;
    b = fabs(pars[2]);
    if (b == 0.0)
        return 0.0;
    q = (pars[1]-x)/b;
    return pars[0]*exp(q-exp(q))/b;
}

Minuit_futil(gumbel_neg_pdf)
{
    double b, q;
    b = fabs(pars[2]);
    if (b == 0.0)
        return 0.0;
    q = (x-pars[1])/b;
    return pars[0]*exp(q-exp(q))/b;
}
