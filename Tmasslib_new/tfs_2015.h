#ifndef TFS_2015_H_
#define TFS_2015_H_

#include "simple_kinematics.h"

/* C interface to the new transfer functions */
#ifdef __cplusplus
extern "C" {
#endif

/* The following function returns 0 on success, something else on failure.
 * Arguments are as follows:
 *
 * gssaFile          -- file containing the transfer functions
 *
 * minExceedance     -- minimal Pt exceedance for the transfer function
 *                      normalization
 *
 * etaBinBoundaries  -- specification for bin boundaries in fabs(eta).
 * nEtaBins             The number of boundary values should be less
 *                      by one than the number of bins.
 *
 * partonMassSplit   -- split point for special low mass quark jet TFs.
 *
 * drStretchFactor   -- factor by which the "delta R" distribution should
 *                      be stretched when it is used as the importance
 *                      sampling density.
 *
 * jetPtCut          -- detector value for the Pt cut to use for jets
 *                      (the "loose" cut)
 *
 * interpolateParam  -- if not 0, interpolate TFs over parameters (slow)
 */
int load_tfs_2015(const char* gssaFile, double minExceedance,
                  const double* etaBinBoundaries, unsigned nEtaBins,
                  double partonMassSplit, double drStretchFactor,
                  double jetPtCut, int interpolateParam);

/* xcut = tf_pt_cutoff*jes_at_cut/partonPt */
double bare_transfer_function(double jetEta, int isB,
                              const double* predictors, unsigned nPredictors,
                              const double* x, unsigned nX, double xCut);

double bare_ptratio_exceedance(double jetEta, int isB,
                               const double* predictors, unsigned nPredictors,
                               double ptRatio, double xCut);

/* The following function changes the direction by -deltaPhi, -deltaEta
 * compared to the "randomize_direction_fromrand" function. The "density"
 * argument can be NULL in which case the density will not be calculated.
 */
v3_obj randomize_sampling_fromrand(v3_obj orig, int isB, double mparton,
                                   double rnd_eta, double rnd_phi,
                                   double *density);

/* This randomizes the "bare" TF, without taking JES into account.
 *
 * "npstatRandomGenerator" can either point to npstat::AbsRandomGenerator
 * object or be (void*)0 in which case Tmasslib "uniform_random" function
 * will be used instead.
 *
 * If the function returns 0, generated "barePtRatio" will be larger than
 * or equal to "xcut". If the function returns 1, the maximum number of
 * tries (given by "maxtries" argument) to generate "barePtRatio" above
 * "xcut" were exceeded.
 */
int bare_tf_random(double jetEta, int isB, double xCut,
                   const double* predictors, unsigned nPredictors,
                   void* npstatRandomGenerator, unsigned maxtries,
                   double* barePtRatio, double* deltaEta, double* deltaPhi);

double single_parton_eff(particle_obj parton, int isB,
                         double deltaJES, double sigmaAtCut);

/* Get the jet pt cut provided in the call to "load_tfs_2015" */
double get_jet_pt_cutoff(void);

/* Get the min Pt exceedance provided in the call to "load_tfs_2015" */
double get_min_pt_exceedance(void);

#ifdef __cplusplus
}
#endif

#endif /* TFS_2015_H_ */
