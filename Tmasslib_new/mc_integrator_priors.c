#include <math.h>

#include "mc_integrator_priors.h"
#include "topmass_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

double massive_parton_whad_point(
    const unsigned mwsq_model_number,
    const double nominal_w_mass, const double nominal_w_width,
    const double param, const float mwsq_cdf_value,
    const jet_info *q, const jet_info *qbar,
    double *mwsq, double *density)
{
    /* For now, generate simple relativistic Breit-Wigner */
    const double peak = nominal_w_mass*nominal_w_mass;
    const double hwhm = nominal_w_mass*nominal_w_width;

    *mwsq = hwhm*tan(M_PI*(mwsq_cdf_value - 0.5)) + peak;
    *density = cauchy_density(*mwsq, peak, hwhm);

    return 1.0;
}

double hadronic_top_point(
    unsigned mthad_model_number, double mwsq, double param,
    double mtscan, double topwidth, unsigned itop, float mt_cdf_value,
    const jet_info *q, const jet_info *qbar, const jet_info *bhad,
    double *mt, double *density)
{
    /* For now, generate simple Breit-Wigner */
    const double peak = mtscan;
    const double hwhm = topwidth/2.0;

    *mt = hwhm*tan(M_PI*(mt_cdf_value - 0.5)) + peak;
    *density = cauchy_density(*mt, peak, hwhm);

    return 1.0;
}

#ifdef __cplusplus
}
#endif
