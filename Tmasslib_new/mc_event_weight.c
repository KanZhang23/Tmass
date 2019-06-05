#include "mc_event_weight.h"
#include "ttbar_phase_space.h"
#include "matrel_mahlon_parke.h"
#include "matrel_kleiss_stirling.h"
#include "p_struct_function.h"

#ifdef __cplusplus
extern "C" {
#endif

double mc_event_weight(
    const integ_parameters *ipars,
    const particle_obj q, const particle_obj qbar, const particle_obj bhad,
    const particle_obj blep, const particle_obj l, const particle_obj nu,
    const int leptonCharge, const double mtscan)
{
    double jaco_mw;
    double jaco_pnuz;
    double gridratio;
    double matrel = 1.0;
    double matrel_gg = 0.0;
    double structfun = 1.0;
    double structfun_gg = 0.0;

    /* Calculate the phase space factors */
    ttbar_phase_space(l, nu, blep, q, qbar, bhad,
                      &jaco_mw, &jaco_pnuz);

    /* Calculate the matrix element factor */
    if (ipars->matrel_code & MASK_KLEISS_STIRLING)
        kleiss_stirling_weights(q, qbar, bhad, blep, l, nu, leptonCharge,
				ipars->nominal_w_mass, ipars->nominal_w_width,
				mtscan, ipars->permute_w_daughters,
				&matrel, &matrel_gg);
    else if (ipars->matrel_code & MASK_MAHLON_PARKE)
        matrel = mahlon_parke_weight(q, qbar, bhad, blep, l, nu, leptonCharge,
				     ipars->nominal_w_mass, ipars->nominal_w_width,
				     mtscan, ipars->permute_w_daughters);

    /* Calculate the parton distribution function factor */
    if (ipars->matrel_code & MASK_PROTON_STRUCT)
        pdfterm_qqbar(sum4(bhad, sum4(q, qbar)), sum4(blep, sum4(l, nu)),
                      ipars->cms_energy, &structfun, &structfun_gg);

    /* For now, we will set the grid ratio factor to 1 */
    gridratio = 1.0;

    return (matrel*structfun + matrel_gg*structfun_gg) * jaco_mw * gridratio;
}

#ifdef __cplusplus
}
#endif
