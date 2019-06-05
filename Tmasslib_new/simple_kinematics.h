#ifndef SIMPLE_KINEMATICS_H_
#define SIMPLE_KINEMATICS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double x;
    double y;
    double z;
} v3_obj;

typedef struct {
    v3_obj v;
    double s;
} rotation_obj;

/* It is assumed that the particle object e is >= 0.0 and m >= 0.0 */
typedef struct {
    v3_obj p;
    double m;
    double e;
} particle_obj;

typedef struct {
    v3_obj dir;
    double rapidity;
} boost_obj;

typedef struct {
    v3_obj s;
    double s0;
} spin_obj;

/* Constructors for 3 and 4-vectors */
v3_obj v3(double px, double py, double pz);
v3_obj pt_eta_phi(double pt, double eta, double phi);
particle_obj particle(v3_obj v, double m);
spin_obj spin(v3_obj s);

/* String representation of 3 and 4-vectors. Static buffers are used. */
const char *string3(v3_obj v);
const char *string4(particle_obj part);

/* Functions which act on 3-vectors */
double sprod3(v3_obj p1, v3_obj p2);
double angle(v3_obj p1, v3_obj p2);
double cos3(v3_obj p1, v3_obj p2);
double mom(v3_obj p);
double Pt(v3_obj p);
double Phi(v3_obj p);
double Theta(v3_obj p);
double Eta(v3_obj p);
double phidiff(v3_obj p1, v3_obj p2);
double conediff(v3_obj p1, v3_obj p2);
double maxconediff(v3_obj e1, v3_obj h1, v3_obj e2, v3_obj h2,
                   v3_obj e3, v3_obj h3, v3_obj e4, v3_obj h4);
double acoplanarity(v3_obj p1, v3_obj p2, v3_obj p3);

v3_obj vprod3(v3_obj p1, v3_obj p2);
v3_obj sum3(v3_obj p1, v3_obj p2);
v3_obj diff3(v3_obj p1, v3_obj p2);
v3_obj mult3(v3_obj p1, double c);
v3_obj div3(v3_obj p1, double c);
v3_obj direction3(v3_obj p);

/* Project one vector (p1) onto the direction of the other (p2).
 * p2 must be non-zero.
 */
v3_obj proj3(v3_obj p1, v3_obj p2);

/* Project one vector (p1) onto the plane orthogonal to the other (p2).
 * p2 must be non-zero.
 */
v3_obj orthogonalize3(v3_obj p1, v3_obj p2);

/* Costructor for rotations. The axis vector does not
 * have to be normalized but must be non-zero.
 */
rotation_obj rotation(v3_obj axis, double angle);

/* Returned axis will be normalized */
v3_obj rotaxis(rotation_obj rot);
double rotangle(rotation_obj rot);

/* Function for rotating 3-vectors */
v3_obj rotate(v3_obj v, rotation_obj rot);

/* Rotating a 3-vector using "tworots" result is equivalent
 * to rotating with rot2 first, followed by rotation with rot1.
 */
rotation_obj tworots(rotation_obj rot1, rotation_obj rot2);

/* Functions related to particle kinematics */
#define energy(part) ((part).e)
double sprod4(particle_obj p1, particle_obj p2);
double invmass2(particle_obj p1, particle_obj p2);
double invmass3(particle_obj p1, particle_obj p2, particle_obj p3);
particle_obj sum4(particle_obj p1, particle_obj p2);
particle_obj diff4(particle_obj p1, particle_obj p2);
particle_obj mult4(particle_obj p1, double c);
particle_obj div4(particle_obj p1, double c);
double Et(particle_obj part);
particle_obj reverse_direction(particle_obj part);
/* 
 * Particle speed (in units of c)
 */
double f_beta(particle_obj part);
/* 
 * Particle gamma factor
 */
double f_gamma(particle_obj part);
/* 
 * Particle beta gamma factor (faster and more precise 
 * than the product of the two previous functions).
 */
double f_betagamma(particle_obj part);
/*
 * The following function calculates the boost to the rest system of "part".
 */
boost_obj rest_boost(particle_obj part);
/*
 * The following function calculates the boost inverse to the given one.
 */
boost_obj inverse_boost(boost_obj b);
/*
 * The following function boosts particle "part" using boost object "b".
 */
particle_obj boost(particle_obj part, boost_obj b);
/*
 * The following function boosts covariant spin "s" using boost object "b".
 */
spin_obj spin_boost(spin_obj s, boost_obj b);
/* 
 * Function which returns cos of the decay angle
 * in the rest system of the parent particle
 */
double cosdecay(particle_obj dau1, particle_obj dau2);
/* 
 * Function which returns cos of the angle
 * between the flight directions of two particles
 * in the rest system of the second particle
 */
double cosboosted2(particle_obj p1, particle_obj p2);
/* 
 * Function which returns cos of the angle between two
 * particles in the rest system of the third particle
 */
double cosboosted3(particle_obj p1, particle_obj p2, particle_obj p3);
/*
 * Off-diagonal axis for use in t tbar spin correlations studies
 */
particle_obj off_diagonal_axis(particle_obj t, particle_obj tbar);
/*
 * Random phase-space particle decay into two daughters.
 * Returns 0 if everything is fine, 1 if the decay is impossible.
 */
int phsp_decay(particle_obj parent, double m1, double m2,
               particle_obj *dau1, particle_obj *dau2);
/*
 * Random phase-space particle decay into two daughters in 2d.
 * Returns 0 if everything is fine, 1 if the decay is impossible.
 * The z component of the parent's momentum must be 0, otherwise
 * the function will generate run time error.
 */
int phsp_decay_2d(particle_obj parent, double m1, double m2,
		  particle_obj *dau1, particle_obj *dau2);

/* Square root of the kinematical lambda function.
 * This function makes certain assumptions about its usage.
 * Normally it has to be called with all positive arguments.
 * Also, the square root of one of the arguments must be
 * larger than the sum of the square roots of the two other.
 * If these conditions are not satisfied, the function
 * returns 0.
 *
 * The CMS momentum of the daughters in a two-body decay is
 * sqr_lambda(M_mother^2, M_dau1^2, M_dau2^2) / (2*M_mother)
 */
double sqr_lambda(double x, double y, double z);

/* Determinants for 3x3 and 4x4 matrices */
double det3by3(double m[3][3]);
double det4by4(double m[4][4]);

/* Multiply two matrices m1 and m2 */
void m_mult(const double *m1, const double *m2,
	    int nrows1, int ncols1, int ncols2, double *result);

/* Multiply matrix m1 by transpose of matrix m2 */
void m_mult_tr(const double *m1, const double *m2,
	       int nrows1, int ncols1, int nrows2, double *result);

/* Numerically stable code to solve quadratic equations.
 * Returns the number of solutions found.
 */
int solve_quadratic(double b, double c, double *x1, double *x2);

/* The following function finds the shortest delta in phi.
 * Phi is assumed to be periodic with period 2*PI.
 * Note that it is better to use the "phidiff" function
 * in case delta phi is needed for 3d vectors.
 */
double phi_delta(double phi1, double phi2);

/* The following function decomposes angular difference in the eta-phi
 * space into the angular difference along the shortest line in the
 * eta-phi space which connects the origin and the given point, and
 * in the direction orthogonal to this line.
 */
void decompose_angle_delta(double eta_origin, double phi_origin,
                           double eta_point, double phi_point,
                           double eta_delta_point, double phi_delta_point,
                           double *delta_in_line, double *delta_orthogonal);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLE_KINEMATICS_H_ */
