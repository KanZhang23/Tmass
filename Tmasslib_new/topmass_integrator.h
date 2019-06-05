#ifndef TOPMASS_INTEGRATOR_H
#define TOPMASS_INTEGRATOR_H

#include <stdio.h>

#include "jet_info.h"
#include "solve_top.h"
#include "single_jet_probs.h"

#define MASK_PROTON_STRUCT   0x1
#define MASK_MAHLON_PARKE    0x2
#define MASK_KLEISS_STIRLING 0x4

#ifdef __cplusplus
extern "C" {
#endif

/* The following struct contains a bunch of parameters 
 * which define various aspects of the integration behavior.
 * It should be filled only once per job, not event-by-event.
 */
typedef struct {
    double nominal_w_mass;   /* Basic constants */
    double nominal_w_width;
    double cms_energy;

    double light_jet_mass;   /* Masses to use for jets. Make these negative */
    double had_b_jet_mass;   /* in order to work with original masses.      */
    double lep_b_jet_mass;

    double whad_coverage;    /* Coverage of the hadronic mW^2 grid          */
    double wlep_coverage;    /* Coverage of the leptonic mW^2 grid          */
    double thad_coverage;    /* Coverage of the hadronic Mt grid            */
    double tlep_coverage;    /* Coverage of the leptonic Mt grid            */
    /* double top_width;        Width of the Mt Breit-Wigner (must be > 0)  */
    unsigned wlep_npoints;   /* Number of points in the leptonic mW^2 grid  */
    int debug_level;         /* Determines how verbose the code is going
                              * to be. Set it to 0 to suppress all debug
                              * printouts.
                              */
    int matrel_code;         /* Determines whether to use the matrix element
                              * and the proton structure function. Should
                              * be a bitwise "OR" of the MASK_PROTON_STRUCT
			      * and one of the matrix element masks.
			      */
    int process_tcl_events;  /* Should be set to 0 (unless you really know
                              * what tcl events are and when it is safe
                              * to process them).
                              */
    int permute_jets;        /* Can be used to turn permuting the jets
			      * on and off.
			      */
    int param_grid_absolute; /* If 0, the grid in log(pq/pqbar) will be
                              * shifted so that the observed value of
                              * log(pq/pqbar) is in the grid center.
                              */
} integ_parameters;

/* Don't fill the above structure by hand, use the function below instead.
 * This way any change in the inteface will be noticed by the compiler.
 * The function returns 0 if all parameter values make sense, otherwise
 * it returns something else.
 */
int fill_integ_parameters(integ_parameters *ip, double nominal_w_mass,
			  double nominal_w_width, double cms_energy,
			  double light_jet_mass, double had_b_jet_mass,
                          double lep_b_jet_mass, double whad_coverage,
                          double wlep_coverage,
                          double thad_coverage, double tlep_coverage,
                          double top_width,
			  int wlep_npoints,
                          int debug_level, int matrel_code,
			  int process_tcl_events, int permute_jets,
			  int param_grid_absolute);

/* Call the following function to release integ_parameters memory (if any) */
void cleanup_integ_parameters(integ_parameters *ip);

/* Print the integration parameters (useful for debugging) */
void print_integ_parameters(const integ_parameters *ip, FILE *stream);

void hadronic_side_tf_product(
    const double *jes_points, const unsigned n_jes_points,
    const hadron_side_solution *hsol, const jet_info *pq,
    const jet_info *pqbar, const jet_info *pb,
    Prob_to_loose_parton *prob_to_loose_parton,
    double *tfprod);

#ifdef __cplusplus
}
#endif

#endif /* TOPMASS_INTEGRATOR_H */
