#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "simple_kinematics.h"
#include "topmass_utils.h"
#include "pi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The following function is needed to generate two random numbers.
 * Change as necessary to include/avoid dependence on some random
 * number generator.
 */
static void uniform_random_2(double *r1, double *r2)
{
    *r1 = uniform_random();
    *r2 = uniform_random();
}

const char *string3(v3_obj v)
{
    static char buf[64];
    sprintf(buf, "{%g %g %g}", v.x, v.y, v.z);
    return buf;
}

const char *string4(particle_obj part)
{
    static char buf[80];
    sprintf(buf, "{{%g %g %g} %g}", part.p.x, part.p.y, part.p.z, part.m);
    return buf;
}

v3_obj v3(double px, double py, double pz)
{
    v3_obj v;
    v.x = px;
    v.y = py;
    v.z = pz;
    return v;
}

v3_obj pt_eta_phi(double pt, double eta, double phi)
{
    v3_obj v;
    v.x = pt*cos(phi);
    v.y = pt*sin(phi);
    v.z = pt*sinh(eta);
    return v;
}

particle_obj particle(v3_obj v, double m)
{
    particle_obj part;
    part.p = v;
    part.m = m;
    part.e = sqrt(v.x*v.x + v.y*v.y + v.z*v.z + m*m);
    return part;
}

spin_obj spin(v3_obj vs)
{
    spin_obj s;
    s.s = vs;
    s.s0 = 0.0;
    return s;
}

spin_obj spin_boost(spin_obj s, boost_obj b)
{
    const double par = sprod3(b.dir, s.s);
    const double ch = cosh(b.rapidity);
    const double sh = sinh(b.rapidity);
    const double parPrime = ch*par - s.s0*sh;
    spin_obj res;
    res.s = sum3(diff3(s.s, mult3(b.dir, par)), mult3(b.dir, parPrime));
    res.s0 = ch*s.s0 - sh*par;
    return res;
}

double sprod3(v3_obj p1, v3_obj p2)
{
    return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
}

double angle(v3_obj p1, v3_obj p2)
{
    double len1 = mom(p1);
    double len2 = mom(p2);
    if (len1 == 0.0 || len2 == 0.0)
	return PI/2;
    else
    {
	double cosa = sprod3(p1, p2)/len1/len2;
	if (fabs(cosa) < 0.99)
	    return acos(cosa);
	else
	{
	    /* acos would loose too much precision */
	    v3_obj v = div3(p1, len1);
	    v3_obj u = div3(p2, len2);
            if (cosa > 0.0)
                return 2.0*asin(mom(diff3(v, u))/2.0);
            else
                return PI - 2.0*asin(mom(sum3(v, u))/2.0);
	}
    }
}

double cos3(v3_obj p1, v3_obj p2)
{
    const double len1 = mom(p1);
    const double len2 = mom(p2);
    if (len1 == 0.0 || len2 == 0.0)
	return 0.0;
    else
    {
	const double c = sprod3(p1, p2)/len1/len2;
	if (c < -1.0)
	    return -1.0;
	else if (c > 1.0)
	    return 1.0;
	else
	    return c;
    }
}

v3_obj vprod3(v3_obj p1, v3_obj p2)
{
    v3_obj v;
    v.x = p1.y*p2.z - p1.z*p2.y;
    v.y = p1.z*p2.x - p1.x*p2.z;
    v.z = p1.x*p2.y - p1.y*p2.x;
    return v;
}

v3_obj sum3(v3_obj p1, v3_obj p2)
{
    v3_obj v;
    v.x = p1.x + p2.x;
    v.y = p1.y + p2.y;
    v.z = p1.z + p2.z;
    return v;
}

v3_obj diff3(v3_obj p1, v3_obj p2)
{
    v3_obj v;
    v.x = p1.x - p2.x;
    v.y = p1.y - p2.y;
    v.z = p1.z - p2.z;
    return v;
}

v3_obj mult3(v3_obj p1, double c)
{
    v3_obj v;
    v.x = c*p1.x;
    v.y = c*p1.y;
    v.z = c*p1.z;
    return v;
}

v3_obj div3(v3_obj p1, double c)
{
    v3_obj v;
    assert(c != 0.0);
    v.x = p1.x/c;
    v.y = p1.y/c;
    v.z = p1.z/c;
    return v;
}

/* Rotations are implemented using normalized quaternions */
rotation_obj rotation(v3_obj axis, double angle)
{
    rotation_obj q;
    double halftheta = angle/2.0;
    assert(axis.x || axis.y || axis.z);
    q.v = mult3(direction3(axis), sin(halftheta));
    q.s = cos(halftheta);
    return q;
}

v3_obj rotaxis(rotation_obj rot)
{
    return direction3(rot.v);
}

double rotangle(rotation_obj rot)
{
    return 2.0*atan2(mom(rot.v), rot.s);
}

static rotation_obj mult_q(rotation_obj q1, rotation_obj q2)
{
    rotation_obj q;
    q.s = q1.s*q2.s - sprod3(q1.v,q2.v);
    q.v = sum3(sum3(mult3(q2.v,q1.s),mult3(q1.v,q2.s)),vprod3(q1.v,q2.v));
    return q;
}

v3_obj rotate(v3_obj v, rotation_obj q)
{
    rotation_obj vq, conjugated;
    vq.v = v;
    vq.s = 0.0;
    conjugated.v.x = -q.v.x;
    conjugated.v.y = -q.v.y;
    conjugated.v.z = -q.v.z;
    conjugated.s   = q.s;
    return mult_q(mult_q(q,vq),conjugated).v;
}

rotation_obj tworots(rotation_obj rot1, rotation_obj rot2)
{
    rotation_obj q = mult_q(rot1, rot2);
    const double qnorm = sqrt(q.v.x*q.v.x + q.v.y*q.v.y + q.v.z*q.v.z + q.s*q.s);
    q.v.x /= qnorm;
    q.v.y /= qnorm;
    q.v.z /= qnorm;
    q.s   /= qnorm;
    return q;
}

/*  double energy(particle_obj part) */
/*  { */
/*      return sqrt(sprod3(part.p,part.p) + part.m*part.m); */
/*  } */

double sprod4(particle_obj p1, particle_obj p2)
{
    return energy(p1)*energy(p2) - p1.p.x*p2.p.x -
	p1.p.y*p2.p.y - p1.p.z*p2.p.z;
}

double invmass2(particle_obj p1, particle_obj p2)
{
    double msq = p1.m*p1.m + p2.m*p2.m + 2.0*sprod4(p1,p2);
    assert(msq >= 0.0);
    return sqrt(msq);
}

double invmass3(particle_obj p1, particle_obj p2, particle_obj p3)
{
    double msq = p1.m*p1.m + p2.m*p2.m + p3.m*p3.m +
        2.0*sprod4(p1, p2) + 2.0*sprod4(p1, p3) + 2.0*sprod4(p2, p3);
    assert(msq >= 0.0);
    return sqrt(msq);
}

particle_obj mult4(particle_obj p1, double c)
{
    particle_obj result;
    result.p.x = p1.p.x*c;
    result.p.y = p1.p.y*c;
    result.p.z = p1.p.z*c;
    result.m   = p1.m*c;
    result.e   = p1.e*c;
    return result;
}

particle_obj div4(particle_obj p1, double c)
{
    particle_obj result;
    assert(c != 0.0);
    result.p.x = p1.p.x/c;
    result.p.y = p1.p.y/c;
    result.p.z = p1.p.z/c;
    result.m   = p1.m/c;
    result.e   = p1.e/c;
    return result;
}

particle_obj sum4(particle_obj p1, particle_obj p2)
{
    particle_obj result;
    double msquared;

    result.p.x = p1.p.x + p2.p.x;
    result.p.y = p1.p.y + p2.p.y;
    result.p.z = p1.p.z + p2.p.z;
    /* The following method to calculate the mass is
     * better numerically than calculating the energy
     * first and then using m^2 = E^2 - p^2
     */
    msquared = p1.m*p1.m + p2.m*p2.m + 2.0*(
        energy(p1)*energy(p2) - p1.p.x*p2.p.x -
        p1.p.y*p2.p.y - p1.p.z*p2.p.z);
    if (msquared > 0.0)
        result.m = sqrt(msquared);
    else
        result.m = 0.0;
    result.e = sqrt(result.p.x*result.p.x + result.p.y*result.p.y +
                    result.p.z*result.p.z + result.m*result.m);
    return result;
}

particle_obj diff4(particle_obj p1, particle_obj p2)
{
    particle_obj result;
    double mass_squared;

    result.p.x = p1.p.x - p2.p.x;
    result.p.y = p1.p.y - p2.p.y;
    result.p.z = p1.p.z - p2.p.z;    
    result.e = energy(p1) - energy(p2);
    mass_squared = result.e*result.e - result.p.x*result.p.x - 
	result.p.y*result.p.y - result.p.z*result.p.z;
    assert(mass_squared >= 0.0);
    result.m = sqrt(mass_squared);
    return result;
}

double mom(v3_obj p)
{
    return sqrt(sprod3(p,p));
}

v3_obj direction3(v3_obj p)
{
    v3_obj v;
    double len = mom(p);
    if (len == 0.0) {
        v.x = 1.0;
        v.y = 0.0;
        v.z = 0.0;
    } else {
        v = div3(p,len);
    }
    return v;
}

v3_obj proj3(v3_obj p1, v3_obj p2)
{
    const double p2lensq = p2.x*p2.x + p2.y*p2.y + p2.z*p2.z;
    assert(p2lensq > 0.0);
    return mult3(p2, (p1.x*p2.x + p1.y*p2.y + p1.z*p2.z)/p2lensq);
}

v3_obj orthogonalize3(v3_obj p1, v3_obj p2)
{
    return diff3(p1, proj3(p1, p2));
}

double acoplanarity(v3_obj p1, v3_obj p2, v3_obj p3)
{
    double m1 = mom(p1);
    double m2 = mom(p2);
    double m3 = mom(p3);
    if (m1 == 0.0 || m2 == 0.0 || m3 == 0.0)
        return 0.0;
    else
        return sprod3(p1,vprod3(p2,p3))/(m1*m2*m3);
}

double Pt(v3_obj p)
{
    return hypot(p.x,p.y);
}

double Et(particle_obj part)
{
    const double totP = mom(part.p);
    if (totP == 0.0)
        return 0.0;
    return Pt(part.p)/totP*energy(part);
}

double Phi(v3_obj p)
{
    return atan2(p.y, p.x);
}

double Theta(v3_obj p)
{
    double len = mom(p);
    if (len == 0.0) {
        return PI/2;
    } else {
        return acos(p.z/len);
    }
}

double Eta(v3_obj p)
{
    double pt = hypot(p.x,p.y);
    if (pt == 0.0)
    {
        assert(p.z == 0.0);
        return 0.0;
    }
    return asinh(p.z/pt);
}

double phidiff(v3_obj p1, v3_obj p2)
{
    double phi1 = atan2(p1.y, p1.x);
    double phi2 = atan2(p2.y, p2.x);
    if (fabs(phi1 - phi2) > PI)
    {
        if (phi1 > phi2)
            phi1 -= 2.0*PI;
        else
            phi1 += 2.0*PI;
    }
    return (phi1 - phi2);
}

double conediff(v3_obj p1, v3_obj p2)
{
    return hypot(phidiff(p1,p2), Eta(p1)-Eta(p2));
}

double maxconediff(v3_obj e1, v3_obj h1, v3_obj e2, v3_obj h2,
                   v3_obj e3, v3_obj h3, v3_obj e4, v3_obj h4)
{
    double c, maxdiff = 0.0;
    c = conediff(e1, h1);
    maxdiff = (maxdiff > c ? maxdiff : c);
    c = conediff(e2, h2);
    maxdiff = (maxdiff > c ? maxdiff : c);
    c = conediff(e3, h3);
    maxdiff = (maxdiff > c ? maxdiff : c);
    c = conediff(e4, h4);
    maxdiff = (maxdiff > c ? maxdiff : c);
    return maxdiff;
}

double det3by3(double m[3][3])
{
    const double *data1 = &m[0][0];
    return data1[1]*data1[5]*data1[6] - data1[2]*data1[4]*data1[6] +
        data1[2]*data1[3]*data1[7] - data1[0]*data1[5]*data1[7] - 
        data1[1]*data1[3]*data1[8] + data1[0]*data1[4]*data1[8];
}

double det4by4(double m[4][4])
{
    const double *data1 = &m[0][0];
    return data1[3]*data1[6]*data1[9]*data1[12] - 
        data1[2]*data1[7]*data1[9]*data1[12] - 
        data1[3]*data1[5]*data1[10]*data1[12] + 
        data1[1]*data1[7]*data1[10]*data1[12] + 
        data1[2]*data1[5]*data1[11]*data1[12] - 
        data1[1]*data1[6]*data1[11]*data1[12] - 
        data1[3]*data1[6]*data1[8]*data1[13] + 
        data1[2]*data1[7]*data1[8]*data1[13] + 
        data1[3]*data1[4]*data1[10]*data1[13] - 
        data1[0]*data1[7]*data1[10]*data1[13] - 
        data1[2]*data1[4]*data1[11]*data1[13] + 
        data1[0]*data1[6]*data1[11]*data1[13] + 
        data1[3]*data1[5]*data1[8]*data1[14] - 
        data1[1]*data1[7]*data1[8]*data1[14] - 
        data1[3]*data1[4]*data1[9]*data1[14] + 
        data1[0]*data1[7]*data1[9]*data1[14] + 
        data1[1]*data1[4]*data1[11]*data1[14] - 
        data1[0]*data1[5]*data1[11]*data1[14] - 
        data1[2]*data1[5]*data1[8]*data1[15] + 
        data1[1]*data1[6]*data1[8]*data1[15] + 
        data1[2]*data1[4]*data1[9]*data1[15] - 
        data1[0]*data1[6]*data1[9]*data1[15] - 
        data1[1]*data1[4]*data1[10]*data1[15] + 
        data1[0]*data1[5]*data1[10]*data1[15];
}

particle_obj reverse_direction(particle_obj part)
{
    return particle(mult3(part.p,-1.0), part.m);
}

double f_beta(particle_obj part)
{
    double e = energy(part);
    assert(e > 0.0);
    return mom(part.p)/e;
}

double f_gamma(particle_obj part)
{
    assert(part.m > 0.0);
    return energy(part)/part.m;
}

double f_betagamma(particle_obj part)
{
    assert(part.m > 0.0);
    return mom(part.p)/part.m;
}

/* Boost into the rest system of the given particle */
boost_obj rest_boost(particle_obj part)
{
    boost_obj result;
    assert(part.m > 0.0);
    result.dir = direction3(part.p);
    result.rapidity = atanh(mom(part.p)/energy(part));
    return result;
}

boost_obj inverse_boost(boost_obj b)
{
    boost_obj result = b;
    result.rapidity *= -1.0;
    return result;
}

particle_obj boost(particle_obj part, boost_obj b)
{
    const double par = sprod3(b.dir, part.p);
    const double parPrime = cosh(b.rapidity)*par - 
	                    energy(part)*sinh(b.rapidity);
    particle_obj result;
    result.m = part.m;
    result.p = sum3(diff3(part.p, mult3(b.dir, par)), mult3(b.dir, parPrime));
    result.e = sqrt(result.p.x*result.p.x + result.p.y*result.p.y +
		    result.p.z*result.p.z + result.m*result.m);
    return result;
}

/* Function which returns cos of the decay angle
 * in the rest system of the parent particle
 */
double cosdecay(particle_obj p1, particle_obj p2)
{
    particle_obj parent  = sum4(p1, p2);
    particle_obj boosted = boost(p1, rest_boost(parent));
    return cos3(boosted.p, parent.p);
}

/* Function which returns cos of the angle
 * between the flight directions of two particles
 * in the rest system of the second particle
 */
double cosboosted2(particle_obj p1, particle_obj p2)
{
    particle_obj b1 = boost(p1, rest_boost(p2));
    return cos3(b1.p, p2.p);
}

/* Function which returns cos of the angle between two
 * particles in the rest system of the third particle
 */
double cosboosted3(particle_obj p1,
                          particle_obj p2,
                          particle_obj p3)
{
    boost_obj rest  = rest_boost(p3);
    particle_obj b1 = boost(p1, rest);
    particle_obj b2 = boost(p2, rest);
    return cos3(b1.p, b2.p);
}

particle_obj off_diagonal_axis(particle_obj t, particle_obj tbar)
{
    static const particle_obj beam_direction = {{0.0, 0.0, 1.0}, 0.0, 1.0};
    particle_obj off_diagonal_dir, result;
    const particle_obj ttbar = sum4(t, tbar);
    const boost_obj zmf_boost = rest_boost(ttbar);
    const particle_obj tboosted = boost(t, zmf_boost);
    const particle_obj beam_boosted = boost(beam_direction, zmf_boost);
    const double cqt = cos3(tboosted.p, beam_boosted.p);
    const double sqt = sqrt(1.0 - cqt*cqt);
    const double beta = f_beta(tboosted);
    const double tanpsi = beta*beta*sqt*cqt/(1.0 - beta*beta*sqt*sqt);
    const v3_obj basisx = direction3(beam_boosted.p);
    const double par = sprod3(basisx, tboosted.p);
    const v3_obj perp = diff3(tboosted.p, mult3(basisx, par));
    const v3_obj basisy = direction3(perp);
    double psi = atan(tanpsi);
    if (psi < 0.0) psi += PI;
    off_diagonal_dir.p = sum3(mult3(basisx,cos(psi)),mult3(basisy,sin(psi)));
    off_diagonal_dir.m = 0.0;
    result = boost(off_diagonal_dir, inverse_boost(zmf_boost));
    result.p = direction3(result.p);
    result.e = 1.0;
    return result;
}

double sqr_lambda(double x, double y, double z)
{
    if (x < 0.0 || y < 0.0 || z < 0.0)
        return 0.0;
    else if (x == 0.0)
        return fabs(y - z);
    else if (y == 0.0)
        return fabs(x - z);
    else if (z == 0.0)
        return fabs(x - y);
    else
    {
        double dtmp = x*x + y*y + z*z - 2.0*x*y - 2.0*x*z - 2.0*y*z;
        return dtmp > 0.0 ? sqrt(dtmp) : 0.0;
    }
}

int phsp_decay(const particle_obj parent, const double m1, const double m2,
               particle_obj *dau1, particle_obj *dau2)
{
    static const v3_obj null3d = {0.0, 0.0, 0.0};

    const boost_obj lab_boost = inverse_boost(rest_boost(parent));
    const double msum = m1 + m2;
    particle_obj dau1_cms, dau2_cms;

    /* Check whether the decay is possible */
    if (m1 < 0.0 || m2 < 0.0 || parent.m < msum)
    {
        *dau1 = particle(null3d, 0.0);
        *dau2 = *dau1;
        return 1;
    }

    if (parent.m == msum)
    {
        dau1_cms = particle(null3d, m1);
        dau1_cms = particle(null3d, m2);
    }
    else
    {
        double r1, r2;
        uniform_random_2(&r1, &r2);
        {
            const double costheta = r1*2.0 - 1.0;
            const double sinthetasq = 1.0 - costheta*costheta;
            const double sintheta = sinthetasq > 0.0 ? sqrt(sinthetasq) : 0.0;
            const double phi      = 2.0*PI*r2;
            double pcms;
            v3_obj p1_cms;

            if (m1 && m2)
            {
                double e1_cms = (parent.m*parent.m + m1*m1 - m2*m2)/(2.0*parent.m);
                pcms = sqrt(e1_cms*e1_cms - m1*m1);
            }
            else if (m1)
                pcms = (parent.m*parent.m - m1*m1)/(2.0*parent.m);
            else
                pcms = (parent.m*parent.m - m2*m2)/(2.0*parent.m);
            p1_cms.x = pcms*sintheta*cos(phi);
            p1_cms.y = pcms*sintheta*sin(phi);
            p1_cms.z = pcms*costheta;
            dau1_cms = particle(p1_cms, m1);
            dau2_cms = particle(mult3(p1_cms, -1.0), m2);
        }
    }

    /* Boost to the lab frame */
    *dau1 = boost(dau1_cms, lab_boost);
    *dau2 = boost(dau2_cms, lab_boost);

    return 0;
}

int phsp_decay_2d(const particle_obj parent, const double m1, const double m2,
		  particle_obj *dau1, particle_obj *dau2)
{
    static const v3_obj null3d = {0.0, 0.0, 0.0};

    const boost_obj lab_boost = inverse_boost(rest_boost(parent));
    const double msum = m1 + m2;
    particle_obj dau1_cms, dau2_cms;

    /* Check the z momentum component of the parent */
    assert(parent.p.z == 0.0);

    /* Check whether the decay is possible */
    if (m1 < 0.0 || m2 < 0.0 || parent.m < msum)
    {
        *dau1 = particle(null3d, 0.0);
        *dau2 = *dau1;
        return 1;
    }

    if (parent.m == msum)
    {
        dau1_cms = particle(null3d, m1);
        dau1_cms = particle(null3d, m2);
    }
    else
    {
	const double phi = 2.0*PI*uniform_random();
	double pcms;
	v3_obj p1_cms;

	if (m1 && m2)
	{
	    double e1_cms = (parent.m*parent.m + m1*m1 - m2*m2)/(2.0*parent.m);
	    pcms = sqrt(e1_cms*e1_cms - m1*m1);
	}
	else if (m1)
	    pcms = (parent.m*parent.m - m1*m1)/(2.0*parent.m);
	else
	    pcms = (parent.m*parent.m - m2*m2)/(2.0*parent.m);
	p1_cms.x = pcms*cos(phi);
	p1_cms.y = pcms*sin(phi);
	p1_cms.z = 0.0;
	dau1_cms = particle(p1_cms, m1);
	dau2_cms = particle(mult3(p1_cms, -1.0), m2);
    }

    /* Boost to the lab frame */
    *dau1 = boost(dau1_cms, lab_boost);
    *dau2 = boost(dau2_cms, lab_boost);

    return 0;
}

void m_mult(const double *m1, const double *m2, const int nrows1,
	    const int ncols1, const int ncols2, double *result)
{
    int i, row, col;

    assert(nrows1 > 0);
    assert(ncols1 > 0);
    assert(ncols2 > 0);

    memset(result, 0, nrows1*ncols2*sizeof(double));
    for (row=0; row<nrows1; ++row)
    {
	const double *data1 = m1 + row*ncols1;
	for (col=0; col<ncols2; ++col)
	{
	    const double *data2 = m2 + col;
	    double *r = result + col + row*ncols2;
	    for (i=0; i<ncols1; ++i)
		*r += data1[i]*data2[i*ncols2];
	}
    }
}

void m_mult_tr(const double *m1, const double *m2, const int nrows1,
	       const int ncols1, const int nrows2, double *result)
{
    int i, row, col;

    assert(nrows1 > 0);
    assert(ncols1 > 0);
    assert(nrows2 > 0);

    memset(result, 0, nrows1*nrows2*sizeof(double));
    for (row=0; row<nrows1; ++row)
    {
	const double *data1 = m1 + row*ncols1;
	for (col=0; col<nrows2; ++col)
	{
	    const double *data2 = m2 + col*ncols1;
	    double *r = result + col + row*nrows2;
	    for (i=0; i<ncols1; ++i)
		*r += data1[i]*data2[i];
	}
    }
}

int solve_quadratic(const double b, const double c,
                    double *x1, double *x2)
{
    const double d = b*b - 4.0*c;

    if (d < 0.0)
        return 0;

    if (d == 0)
    {
        *x1 = -b/2.0;
        *x2 = *x1;
    }
    else if (b == 0.0)
    {
        *x1 = sqrt(-c);
        *x2 = -(*x1);
    }
    else
    {
        *x1 = -(b + (b < 0.0 ? -1.0 : 1.0)*sqrt(d))/2.0;
        *x2 = c/(*x1);
    }
    return 2;
}

double phi_delta(const double phi1, const double phi2)
{
    int n = 0, maxiter = fabs(phi1 - phi2)/2/PI, niter;
    double phimod = phi1;
    ++maxiter;
    for (niter=0; niter<maxiter && fabs(phimod-phi2)>PI; ++niter)
    {
        if (phimod > phi2)
            phimod = phi1 + --n*2*PI;
        else
            phimod = phi1 + ++n*2*PI;
    }
    return phimod - phi2;
}

void decompose_angle_delta(double eta_origin, double phi_origin,
                           double eta_point, double phi_point,
                           double eta_delta_point, double phi_delta_point,
                           double *delta_in_line, double *delta_orthogonal)
{
    const double detap = eta_point - eta_origin;
    const double dphip = phi_delta(phi_point, phi_origin);
    const double delta_eta = eta_delta_point - eta_point;
    const double delta_phi = phi_delta(phi_delta_point, phi_point);
    const double baselen = sqrt(detap*detap + dphip*dphip);
    assert(baselen > 0.0);
    *delta_in_line = (delta_eta*detap + delta_phi*dphip)/baselen;
    *delta_orthogonal = (delta_eta*dphip - delta_phi*detap)/baselen;
}

#ifdef __cplusplus
}
#endif
