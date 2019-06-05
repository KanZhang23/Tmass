#include "topmass_utils.h"
#include "single_jet_probs.h"
#include "random_extra_jet.h"
#include "random_lepton_momentum.h"
#include "ratio_tf_shift.h"

/* Code in this file calls some functions implemented inside "Configurables"
 * directory so that they are picked up by the linker when the shared library
 * is created.
 */

static double dummy(double a, double b)
{
    return 0.0;
}

void dont_call_this_function_explicitly(void);

double p0_interpolator_hs_fcn(double x, double y, double z, int mode,
                              const double *pars, int *ierr);

void dont_call_this_function_explicitly(void)
{
    double a[1] = {0};
    jet_info rjet;

    pick_closest_element(a, 1, 0.0, dummy);
    random_extra_jet(0, &rjet);
    randomize_lepton_momentum(v3(20.0, 0.0, 0.0), 1);
    ratio_tf_shift(1, 2, 3, 4, 5);
    p0_interpolator_hs_fcn(1, 2, 3, 4, 0, 0);
}
