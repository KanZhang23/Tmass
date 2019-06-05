#ifndef LEPTONIC_SIDE_MASK_H_
#define LEPTONIC_SIDE_MASK_H_

#include "n_d_random.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double mt_min;
    double mt_max;
    double pt_max;  /* max Pt of the ttbar system         */
    unsigned nMt;
    unsigned nPt;
    unsigned nPhi;
    char *data;     /* array dimensioned [nMt][nPt][nPhi] */
} Leptonic_side_mask;

void init_leptonic_side_mask(Leptonic_side_mask *mask, double mt_min,
                             double mt_max, double pt_max, unsigned nMt,
                             unsigned nPt, unsigned nPhi);
void cleanup_leptonic_side_mask(Leptonic_side_mask *mask);

void clear_leptonic_side_mask(Leptonic_side_mask *mask);
void set_leptonic_side_mask(Leptonic_side_mask *mask, double mt,
                            double pt, double phi, char value);
char leptonic_side_mask_value(const Leptonic_side_mask *mask, double mt,
                              double pt, double phi);
char leptonic_side_mask_inbin(const Leptonic_side_mask *mask, unsigned imt,
                              unsigned ipt, unsigned iphi);

double leptonic_side_masked_fraction(const Leptonic_side_mask *mask);

/* In the following function, bPx, bPy, and bPz
 * arguments are jet momentum components, not
 * parton momentum.
 */
unsigned populate_leptonic_side_mask(
    Leptonic_side_mask *mask, 
    N_d_random_method rand_method, int random_gen_param,
    double lPx, double lPy, double lPz,
    double bPx, double bPy, double bPz, double mb, double cuterr,
    double nuPz_min, double nuPz_max,
    double w_mass, double w_width,
    double size_of_neglected_tf_low_tail,
    double b_random_cone_radius, unsigned randomize_b_mass,
    unsigned max_points, unsigned max_seconds);

#ifdef __cplusplus
}
#endif

#endif /* LEPTONIC_SIDE_MASK_H_ */
