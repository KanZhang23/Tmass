#ifndef MC_INTEGRATOR_PRIORS_H_
#define MC_INTEGRATOR_PRIORS_H_

#include "jet_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The following function returns a weight factor */
double massive_parton_whad_point(
    unsigned mwsq_model_number,
    double nominal_w_mass, double nominal_w_width,
    double param, float mwsq_cdf_value,
    const jet_info *q, const jet_info *qbar,
    double *mwsq, double *density);

double hadronic_top_point(
    unsigned mthad_model_number, double mwsq, double param,
    double mtscan, double topwidth, unsigned itop, float mt_cdf_value,
    const jet_info *q, const jet_info *qbar, const jet_info *bhad,
    double *mt, double *density);

#ifdef __cplusplus
}
#endif

#endif /* MC_INTEGRATOR_PRIORS_H_ */
