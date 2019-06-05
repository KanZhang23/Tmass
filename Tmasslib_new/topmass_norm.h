#ifndef TOPMASS_NORM_H
#define TOPMASS_NORM_H

#include <stdio.h>

#include "topmass_integrator.h"
#include "simple_kinematics.h"
#include "norm_accumulator.h"
#include "permutations.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PROPAG_WEIGHT_GENERATOR = 0,
    PROPAG_WEIGHT_BW,
    PROPAG_WEIGHT_EFFECTIVE
};

enum {
    PROD_CHANNEL_QQ = 0,
    PROD_CHANNEL_GG,
    PROD_CHANNEL_UNDEFINED
};

typedef struct {
    double pole_t_mass;     /* Pole mass of the top quark
                             */
    double twidth;          /* Width of the top quark
                             */
    double mtmin;           /* Lower cutoff for the top quark mass.
                             * Can be used to simulate Herwig.
                             */
    double mtmax;           /* Upper cutoff for the top quark mass.
                             * Can be used to simulate Herwig.
                             */
    double electron_pt_min; /* Various cuts for efficiency estimation */
    double electron_eta_max;
    double muon_pt_min;
    double muon_eta_max;
    double miss_et_cut;     /* Minimum apparent missing Pt */
    double cone_cut;        /* Minimun cone angle between two jets */
    double iso_cut;         /* Minimun cone angle between a jet and a lepton */
    double jet_pt_cut;      /* This value is only used in the space of
                             * reconstructed jet Pt. In the parton space
                             * one should use "henris_acc_value" function.
                             */
    double jet_eta_cut;
    unsigned polemass_bin;  /* Top mass bin number for finding the
                             * Mt-dependent average effective propagator
                             * (if such a propagator is used)
                             */
} norm_parameters;

/* The following function returns 0 if the input parameters are
 * within acceptable limits, 1 if there is a problem.
 */
int fill_norm_parameters(norm_parameters *np, double pole_t_mass,
                         double twidth, double mtmin, double mtmax,
                         double electron_pt_min, double electron_eta_max,
                         double muon_pt_min, double muon_eta_max,
                         double miss_et_cut, double cone_cut, double iso_cut,
                         double jet_pt_cut, double jet_eta_cut,
                         unsigned polemass_bin);

void print_norm_parameters(const norm_parameters *np, FILE *fp);

/* The following function generates a weighted point in a six-body
 * phase space. If parameter "use_bw" is not 0 then the intermediate
 * compounds will be generated according to top/W Breit-Wigners (normal
 * case). "use_bw" can be set to 0 for calculating the "standard"
 * six-body phase space and testing this function. Masses of q, qbar,
 * l, nu, and blep are assumed to be 0. Mass of bhad is taken from
 * ipars->b_jet_mass.
 *
 * Note that 4-momenta of the daughters will be set correctly only
 * if the returned weight is not 0. If the weight is 0 then 4-momenta
 * of the returned daughters are undefined.
 */
void weighted_ttbar_phsp_point(const integ_parameters *ipars,
                               const norm_parameters *normpars,
                               particle_obj ttbar, int use_bw,
                               particle_obj *tlep, particle_obj *thad,
                               particle_obj *wlep, particle_obj *whad,
                               particle_obj *q, particle_obj *qbar,
                               particle_obj *bhad, particle_obj *l,
                               particle_obj *nu, particle_obj *blep,
                               double *phsp_weight, double *norm_integral);

typedef struct {
    particle_obj q1;      /* Either q or g going in positive Z direction */
    particle_obj q2;      /* Either q or g going in negative Z direction */
    particle_obj ttbar;
    particle_obj t;
    particle_obj tbar;
    particle_obj wplus;
    particle_obj b;
    particle_obj ebar;
    particle_obj nu;
    particle_obj wminus;
    particle_obj bbar;
    particle_obj mu;
    particle_obj nubar;
    double Ecms;
    double x1;
    double x2;
    int prod_channel;
} ttbar_event;

/* Note that the contents of *event will make sense 
 * only if the returned weight is not 0.
 */
void weighted_ttbar_event(const integ_parameters *ipars,
                          const norm_parameters *normpars,
                          int use_bw, ttbar_event *event,
                          double *weight_qq, double *weight_gg);

/* The following function returns the weights for
 * a given event configuration. The weights are
 * identical to those produced by "weighted_ttbar_event"
 * function when the event was generated.
 *
 * "propagator_weight_type" should be one of PROPAG_WEIGHT_GENERATOR,
 *     PROPAG_WEIGHT_BW, or PROPAG_WEIGHT_EFFECTIVE
 */
void ttbar_event_weights(const integ_parameters *ipars,
                         const norm_parameters *normpars,
                         int propagator_weight_type,
                         particle_obj mu, particle_obj nubar,
                         particle_obj bbar, particle_obj ebar,
                         particle_obj nu, particle_obj b,
                         double *weight_qq, double *weight_gg);

/* In the following function, the "prod_channel" argument
 * has the following meaning:
 *
 * prod_channel == PROD_CHANNEL_QQ : generate qq -> ttbar
 *   Only maxweight_qq is used.
 *
 * prod_channel == PROD_CHANNEL_GG : generate gg -> ttbar
 *   Only maxweight_gg is used.
 *
 * prod_channel == PROD_CHANNEL_UNDEFINED : generate natural mixture
 *                                          of qq -> ttbar and gg -> ttbar
 *   Only maxweight_sum is used.
 *
 * The function returns the number of tries made
 * in the acceptance/rejection procedure.
 *
 * The pointers weight_qq and weight_gg can be NULL.
 */
int unweighted_ttbar_event(const integ_parameters *ipars,
                           const norm_parameters *normpars,
                           int prod_channel, double maxweight_qq,
                           double maxweight_gg, double maxweight_sum,
                           ttbar_event *event, double *weight_qq,
                           double *weight_gg);

/* The following function calculates tree-level normalization
 * factors for qq->ttbar, gg->ttbar, and combined.
 */
void topmass_norm(const integ_parameters *ipars,
                  const norm_parameters *normpars,
                  int use_bw, unsigned nevents,
                  norm_accumulator *qq_norm,
                  norm_accumulator *gg_norm,
		  norm_accumulator *sum_norm);

/* The following function returns the overall event
 * acceptance, including the acceptance due to jet Pt,
 * modelled in the space of parton variables.
 */
double parton_level_acceptance(const integ_parameters *ipars,
                               const norm_parameters *normpars,
                               const ttbar_event *ev);

/* The following function models event acceptance
 * (just accepted or rejected) in the space of
 * reconstructed variables.
 */
int is_event_accepted(const integ_parameters *ipars,
                      const norm_parameters *normpars,
                      particle_obj q, particle_obj qbar,
                      particle_obj bhad, particle_obj blep,
                      particle_obj l, int isMu, particle_obj recoil);

/* The following function models the event acceptance for 3 jets.
 * The "extra_jet" parameter should be set to one of Q, QBAR, BLEP,
 * or BHAD. Any other setting will result in a 4-jet acceptance.
 */
int is_3jet_event_accepted(const integ_parameters *ipars,
                           const norm_parameters *normpars,
                           particle_obj q, particle_obj qbar,
                           particle_obj bhad, particle_obj blep,
                           particle_obj l, int isMu, particle_obj recoil,
                           int extra_jet);

/* The following function reconstructs tree-level top decay
 * from the event information provided by Herwig in HEPG.
 * This is what Herwig does (-> means decay, ~ means fudge):
 *
 * t        -> w0 b0        (b0 is the tree-level b)
 * w0 b0    ~  w b
 * w        -> q0 qbar0
 * q0 qbar0 ~  q qbar
 *
 * The last fudging stage is absent on the leptonic side,
 * so that the function can be used to defudge the leptonic
 * side with l, nu supplied in place of q0, qbar0.
 *
 * The function returns 0 on success.
 */
int defudge_herwig(const particle_obj t, const particle_obj w0,
                   const particle_obj w, const particle_obj q0,
                   const particle_obj qbar0, particle_obj *q_tree,
                   particle_obj *qbar_tree);

/* The following function returns 0 on success, 1 if the decays
 * can not be performed with the new masses.
 */
int redecay_top_quark(const particle_obj q, const particle_obj qbar,
                      const particle_obj b, const double new_mq,
                      const double new_mqbar, const double new_mb,
                      particle_obj *new_q, particle_obj *new_qbar,
                      particle_obj *new_b);

#ifdef __cplusplus
}
#endif

#endif /* TOPMASS_NORM_H */
