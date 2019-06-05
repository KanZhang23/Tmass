#include <assert.h>
#include <math.h>

#include "tau_pt_spectrum.h"

#define SQRPI 1.772453850905516

#ifdef __cplusplus
extern "C" {
#endif

/* Calculate the integral over x from 0 to a of the following expression:
//
// exp(-x*x/s/s)*(5.0/3.0-3.0*x*x+4.0/3.0*x*x*x-
//                p*(1.0/3.0-3.0*x*x+8.0/3.0*x*x*x));
*/
static double muonInteg(const double p, const double s, const double a)
{
    const double c0 = (5.0 - p)/3.0;
    const double c2 = 3.0*(p - 1.0);
    const double c3 = 4.0/3.0*(1.0 - 2.0*p);

    const double r = a/s;
    const double ssq = s*s;
    const double erfr = erf(r);
    const double exprsq = exp(-r*r);
    const double i0 = SQRPI*s/2.0*erfr;
    const double i2 = ssq*(i0 - a*exprsq)/2.0;
    const double i3 = ssq*(ssq - (ssq + a*a)*exprsq)/2.0;

    return c0*i0 + c2*i2 + c3*i3;
}

/* Pythia does not seem to pass polarization info to TAUOLA correctly */
static const double average_e_polarization = 0.64875857;
static const double average_mu_polarization = 0.71484854;
static const double s = 0.0086468871;

/* The source of the formula used below is the paper "Measurement of the
 * polarization of a high energy muon beam" by the Spin Muon Collaboration
 * (SMC), B. Adeva et. al., Nuclear Instruments and Methods in Physics
 * Research A 343 (1994) 363-373. They cite two other references, and
 * one of those is unpublished. I did not have a chance to dig them up.
 *
 * The mass of the daughter lepton is ignored in this formula. This
 * approximation seems to work very well for tau decays into electrons,
 * but for muons there is a noticeable difference at low x.
 */
inline static double electron_dau_spectrum(const double x, const double Pmu)
{
    if (x >= 0.0 && x < 1.0)
        return 5.0/3.0-3.0*x*x+4.0/3.0*x*x*x-Pmu*(1.0/3.0-3.0*x*x+8.0/3.0*x*x*x);
    else
        return 0.0;
}

inline static double muon_dau_spectrum(const double normfactor,
                                       const double x, const double Pmu)
{
    const double r = x/s;
    return normfactor*(1.0 - exp(-r*r))*electron_dau_spectrum(x, Pmu);
}

/* The integral of electron_dau_spectrum from x to 1 */
inline static double electron_dau_efficiency(const double xcut, const double Pmu)
{
    if (xcut <= 0.0)
        return 1.0;
    else if (xcut >= 1.0)
        return 0.0;
    else
    {
        const double t = xcut - 1.0;
        return (t*t*(3.0 + (1.0 + Pmu)*xcut + (2.0*Pmu - 1.0)*xcut*xcut))/3.0;
    }
}

inline static double muon_dau_cdf(const double normfactor,
                                  const double x, const double Pmu)
{
    if (x <= 0.0)
        return 0.0;
    else if (x >= 1.0)
        return 1.0;
    else
    {
        const double xm1 = x - 1.0;
        const double cdf1 = x*(5.0 - 3.0*x*x + x*x*x - Pmu*xm1*xm1*(2.0*x + 1.0))/3.0;
        double cdf = normfactor*(cdf1 - muonInteg(Pmu, s, x));
        if (cdf < 0.0)
            cdf = 0.0;
        if (cdf > 1.0)
            cdf = 1.0;
        return cdf;
    }
}

/* The integral of muon_dau_spectrum from x to 1 */
inline static double muon_dau_efficiency(const double normfactor,
                                         const double xcut, const double Pmu)
{
    return 1.0 - muon_dau_cdf(normfactor, xcut, Pmu);
}

double tau_daughter_spectrum(const double x, const double mtop,
                             const int ltype)   
{
    static double normfactor = 0.0;
    if (normfactor <= 0.0)
        normfactor = 1.0/(1.0 - muonInteg(average_mu_polarization, s, 1.0));

    switch (ltype)
    {
    case 0:
        /* Electron */
        return electron_dau_spectrum(x, average_e_polarization);

    case 1:
        /* Muon */
        return muon_dau_spectrum(normfactor, x, average_mu_polarization);

    default:
        assert(0);
        return 0.0;
    }
}

double tau_daughter_efficiency(const double xcut, const double mtop,
                               const int ltype)
{
    static double normfactor = 0.0;
    if (normfactor <= 0.0)
        normfactor = 1.0/(1.0 - muonInteg(average_mu_polarization, s, 1.0));

    switch (ltype)
    {
    case 0:
        /* Electron */
        return electron_dau_efficiency(xcut, average_e_polarization);

    case 1:
        /* Muon */
        return muon_dau_efficiency(normfactor, xcut, average_mu_polarization);

    default:
        assert(0);
        return 0.0;
    }
}

#ifdef __cplusplus
}
#endif
