#ifndef SOLVE_TOP_H_
#define SOLVE_TOP_H_

#include <stdlib.h>
#include "simple_kinematics.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The following struct cannot contain any pointers */
typedef struct {
    double pblep;       /* magnitude of the leptonic side b momentum    */
    double bPx;         /* leptonic b momentum components               */
    double bPy;
    double bPz;
    double mb;

    double nux;         /* components of the neutrino momentum          */
    double nuy;
    double nuz;

    double mwsq;        /* squared invariant mass of the leptonic W     */
    double tlepz;       /* z component of the leptonic top momentum     */
    double mt;          /* leptonic top invariant mass                  */

    int fail;           /* Will be set to 1 if the maximum number of
                           iterations is reached without convergence    */
} lepton_side_solution;

/* The following struct cannot contain any pointers */
typedef struct {
    double qP;          /* magnitude of the q momentum                  */
    double qPx;         /* q momentum components                        */
    double qPy;
    double qPz;

    double qbarP;       /* magnitude of the qbar momentum               */
    double qbarPx;      /* qbar momentum components                     */
    double qbarPy;
    double qbarPz;

    double bP;          /* magnitude of the hadronic side b momentum    */
    double bPx;         /* b momentum components                        */
    double bPy;
    double bPz;

    double mq;          /* Quark masses used to find this solution      */
    double mqbar;
    double mb;

    int is_valid;       /* Will be set to 1 if the solution is found    */
                        /*   successfully, to 0 otherwise               */
} hadron_side_solution;

/* Function which constructs hadronic top out of hadron_side_solution */
particle_obj hadronic_top_particle(const hadron_side_solution *hadsol);

/* Function for quick estimation of possible leptonic top
 * z momentum range
 */
void leptonic_top_z_range(double Ecms, double ttbarPx, double ttbarPy,
                          double thadPx, double thadPy, double thadPz,
                          double mthad, double mtlep,
                          double *pzmin, double *pzmax);

/* Function to find the minimum and maximum possible W masses
 * on the leptonic side. Returns the number of extrema found.
 * Note that in the current implementation only the minimum
 * is searched for (the function returns 0 or 1), and the maximum
 * is just set to mt - mb.
 *
 * If the function does not find a solution, it means that it went through
 * all trial points in the "initial_mw_points" array without success.
 * Values in the "initial_mw_points" array must be arranged in the
 * increasing order.
 */
int w_mass_range(const double topPx, const double topPy,
                 const double lPx, const double lPy, const double lPz,
                 const double bPx, const double bPy, const double bPz,
                 const double mt, const double mb, const int debug_level,
                 const double *initial_mw_points, const size_t npoints,
                 lepton_side_solution *mwmin, lepton_side_solution *mwmax);

/* Function to find local nuz minimum or maximum. Return values are:
 *
 *  0  -- no local extremum found
 * -1  -- found local minimum
 *  1  -- found local maximum
 */
int nuz_local_peak(const double topPx, const double topPy,
                   const double lPx, const double lPy, const double lPz,
                   const double bPx, const double bPy, const double bPz,
                   const double mt, const double mb_in, const int debug_level,
                   const double initial_pb, const double initial_nuz,
                   lepton_side_solution *extremum);

/* A slow but very reliable function to search for global nuz miniumum
 * or maximum. A solution should originally exist at the "initial_nuz"
 * point. If not then the function returns 0, and the minmax solution
 * is not filled. "initial_step" argument gives the direction
 * of the search and the initial step size. 1 is returned on success.
 */
int variable_step_nuz_minmax(
              const double topPx, const double topPy,
              const double lPx, const double lPy, const double lPz,
              const double bPx, const double bPy, const double bPz,
              const double mt, const double mb, const int debug_level,
              const double initial_nuz, const double initial_step,
              const double precision, lepton_side_solution *sol);

/* Function which defines matching "distance" for magnitudes of momenta */
double pb_distance(double pb_solution, double pb_data);

/* In the two functions below, magnitudes of all quark momenta
 * are assumed to be unknown. Argument "param" is log(pq/pqbar).
 * The function returns the number of solutions found (0 or 1).
 */
int solve_hadronic_side(const double qPx,    const double qPy,
                        const double qPz,    const double mq,
                        const double qbarPx, const double qbarPy,
                        const double qbarPz, const double mqbar,
                        const double bPx,    const double bPy,
                        const double bPz,    const double mb,
                        const double mt,     const double mwsq,
                        const double param,  const int debug_level,
                        hadron_side_solution *solution);

int solve_hadronic_w(const double qPx,    const double qPy,
                     const double qPz,    const double mq,
                     const double qbarPx, const double qbarPy,
                     const double qbarPz, const double mqbar,
                     const double mwsq,   const double param,
                     double *pq, double *pqbar);

/* In all functions below which solve the leptonic side kinematics
 * arguments "mb" and "max_iterations" are ignored in the current
 * implementation. The kinematics is solved approximately in one step
 * with the assumption that the b jet is massless. This is likely
 * to change in the future.
 */

/* In the function below, the b momentum is used to define only
 * the b quark direction, and the magnitude of the b momentum is treated
 * as an unknown. The function returns the number of solutions found:
 * 0, 1, or 2. If more than 2 solutions exist then those two solutions
 * are chosen which have their pb closer to the original one.
 */
int solve_leptonic_side(const double topPx, const double topPy,
                        const double lPx, const double lPy, const double lPz,
                        const double bPx, const double bPy, const double bPz,
                        const double mt, const double mb, const double mwsq,
                        const int debug_level, const size_t max_iterations,
                        lepton_side_solution solutions[2]);

/* In the function below, the b momentum is used to define only
 * the b quark direction, and the magnitude of the b momentum is treated
 * as an unknown. The function returns the number of solutions found
 * (up to 4).
 */
int solve_leptonic_byNuPz(const double topPx, const double topPy,
                          const double lPx, const double lPy, const double lPz,
                          const double bPx, const double bPy, const double bPz,
                          const double mt, const double mb, const double nuPz,
                          const int debug_level, const size_t max_iterations,
                          lepton_side_solution solutions[4]);

/* The following function uses b momentum to find mwsq and nuPz. Same
 * as in all other "solve_leptonic" functions, arguments bPx, bPy,
 * and bPz, define only the direction of the b. Magnitude of the b
 * momentum is defined by the "pb" argument.
 */
int solve_leptonic_byPb(const double topPx, const double topPy,
                        const double lPx, const double lPy, const double lPz,
                        const double bPx, const double bPy, const double bPz,
                        const double mt, const double mb, const double pb,
                        const int debug_level, const size_t max_iterations,
                        lepton_side_solution solutions[2]);

/* The following function finds minimum leptonic top mass for which
 * "solve_leptonic_byPb" may return a solution. Arguments bPx, bPy,
 * and bPz, define only the direction of the b. Magnitude of the b
 * momentum is defined by the "pb" argument.
 */
double min_leptonic_mt_byPb(const double topPx, const double topPy,
                        const double lPx, const double lPy, const double lPz,
                        const double bPx, const double bPy, const double bPz,
                        const double mb, const double pb,
                        lepton_side_solution *lsol);

/* The following function finds minimum leptonic W mass squared for
 * which "solve_leptonic_byPb" may return a solution. Arguments bPx,
 * bPy, and bPz, define only the direction of the b. Magnitude of
 * the b momentum is defined by the "pb" argument.
 */
double min_leptonic_mwsq_byPb(const double topPx, const double topPy,
                        const double lPx, const double lPy, const double lPz,
                        const double bPx, const double bPy, const double bPz,
                        const double mb, const double pb,
                        lepton_side_solution *lsol);

/* The following function uses b momentum and mW squared to find nuPz.
 * It works just like the similar function in the classic kinematic fit.
 * Massless b is assumed in this implementation.
 */
int solve_leptonic_byMWsq(const double topPx, const double topPy,
                          const double lPx, const double lPy, const double lPz,
                          const double bPx, const double bPy, const double bPz,
                          const double mwsq, const double mb, const double pb,
                          const int debug_level, const size_t max_iterations,
                          lepton_side_solution solutions[2]);

/* Helper function for filling out hadronic solution from particles */
void fill_hadron_side_solution(particle_obj q, particle_obj qbar,
                               particle_obj b, hadron_side_solution *hsol);

/* Helper function for filling out the lepton_side_solution data */
int fill_leptonic_solutions(
    const double cbx, const double cby, const double cbz,
    const double lPx, const double lPy, const double lPz,
    const double topPx, const double topPy,
    const double mt, const double mb,
    const double mwsq, const double *nuPz,
    const double *pbvalues, const int *failflag, size_t nsols,
    lepton_side_solution *solutions);

/* The following function finds maximum possible lepton
 * transverse momentum in case we know Pt of the W, W mass,
 * and lepton direction.
 */
double max_lepton_pt(const double wx, const double wy, const double mwsq,
                     const double lPx, const double lPy, const double lPz);

/* The following function approximates the leptonic side
 * solution for the given value of b quark beta (v/c).
 */
int solve_leptonic_byMW_approx(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double beta, const int debug_level,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4]);

/* The following function attempts to solve the equations with massive b */
int solve_leptonic_side_massiveb(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double *betalist, unsigned n_beta_points,
    const int debug_level, const size_t max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4]);

int solve_leptonic_side_massiveb_verbose(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double *betalist, unsigned n_beta_points,
    const int debug_level, const size_t max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4]);

int solve_leptonic_side_massiveb_brute(
    const double topPx, const double topPy,
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const int debug_level, const size_t max_iterations,
    const double minTopPz, const double maxTopPz,
    lepton_side_solution solutions[4]);

int solve_leptonic_for_top_pt(
    const double lPx, const double lPy, const double lPz,
    const double bPx, const double bPy, const double bPz,
    const double mt, const double mb, const double mwsq,
    const double nuPz, double topPx[4], double topPy[4]);

#ifdef __cplusplus
}
#endif

#endif /* SOLVE_TOP_H_ */
