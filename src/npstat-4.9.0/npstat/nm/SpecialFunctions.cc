// Most of the code below is cherry-picked from the Cephes library
// written by Stephen L. Moshier

#include <cassert>
#include <stdexcept>
#include <cmath>
#include <cfloat>
#include <vector>
#include <algorithm>

#include "npstat/nm/SpecialFunctions.hh"
#include "npstat/nm/MathUtils.hh"
#include "npstat/nm/Matrix.hh"

#include "npstat/rng/permutation.hh"

#define SQRTTWOPIL 2.506628274631000502415765L
#define SQTPI 2.50662827463100050242
#define LOGPI 1.14472988584940017414

#define MAXGAM 171.624376956302725
#define MAXSTIR 143.01608
#define MAXLOG 709.782712893383996732224
#define MINLOG (-708.396418532264106224)
#define MACHEP 1.2e-16
#define MAXLGM 2.556348e305
#define MAXNUM 1.79769313486231570815e308
#define LS2PI 0.91893853320467274178

#define GAUSS_MAX_SIGMA 40.0

static double igam(double a, double x);
static double invgauss(double x);

static double polevl(const double x, const double *p, int i)
{
    double ans = *p++;
    do {
	ans = ans * x + *p++;
    } while (--i);
    return ans;
}

static double p1evl(double x, const double *p, const int N)
{
    double ans = x + *p++;
    int i = N - 1;
    do {
	ans = ans * x  + *p++;
    } while (--i);
    return ans;
}

/* Gamma function computed by Stirling's formula.
 * The polynomial STIR is valid for 33 <= x <= 172.
 */
static double stirf(const double x)
{
    static const double STIR[5] = {
        7.87311395793093628397E-4,
        -2.29549961613378126380E-4,
        -2.68132617805781232825E-3,
        3.47222221605458667310E-3,
        8.33333333333482257126E-2,
    };

    double y, w;
    w = 1.0/x;
    w = 1.0 + w * polevl( w, STIR, 4 );
    y = exp(x);
    if( x > MAXSTIR )
    {
        // Avoid overflow in pow()
	const double v = pow( x, 0.5 * x - 0.25 );
	y = v * (v / y);
    }
    else
    {
	y = pow( x, x - 0.5 ) / y;
    }
    return SQTPI * y * w;
}

// Logarithm of the gamma function
static double lgam(double x, int* sgngam_out=0)
{
    int sgngam = 1;
    if (sgngam_out)
        *sgngam_out = sgngam;

    double p, q, u, w, z;
    int i;

    static const double A[] = {
        8.11614167470508450300E-4,
        -5.95061904284301438324E-4,
        7.93650340457716943945E-4,
        -2.77777777730099687205E-3,
        8.33333333333331927722E-2
    };
    static const double B[] = {
        -1.37825152569120859100E3,
        -3.88016315134637840924E4,
        -3.31612992738871184744E5,
        -1.16237097492762307383E6,
        -1.72173700820839662146E6,
        -8.53555664245765465627E5
    };
    static const double C[] = {
        -3.51815701436523470549E2,
        -1.70642106651881159223E4,
        -2.20528590553854454839E5,
        -1.13933444367982507207E6,
        -2.53252307177582951285E6,
        -2.01889141433532773231E6
    };

    if( x < -34.0 )
    {
	q = -x;
	w = lgam(q, &sgngam);
        if (sgngam_out)
            *sgngam_out = sgngam;
	p = floor(q);
	if( p == q )
            goto loverf;
	i = static_cast<int>(p);
	if( (i & 1) == 0 )
            sgngam = -1;
	else
            sgngam = 1;
        if (sgngam_out)
            *sgngam_out = sgngam;
	z = q - p;
	if( z > 0.5 )
        {
            p += 1.0;
            z = p - q;
        }
	z = q * sin( M_PI * z );
	if( z == 0.0 )
            goto loverf;
	// z = log(PI) - log( z ) - w;
	z = LOGPI - log( z ) - w;
	return( z );
    }

    if( x < 13.0 )
    {
	z = 1.0;
	p = 0.0;
	u = x;
	while( u >= 3.0 )
        {
            p -= 1.0;
            u = x + p;
            z *= u;
        }
	while( u < 2.0 )
        {
            if( u == 0.0 )
                goto loverf;
            z /= u;
            p += 1.0;
            u = x + p;
        }
	if( z < 0.0 )
        {
            sgngam = -1;
            z = -z;
        }
	else
            sgngam = 1;
        if (sgngam_out)
            *sgngam_out = sgngam;
	if( u == 2.0 )
            return( log(z) );
	p -= 2.0;
	x = x + p;
	p = x * polevl( x, B, 5 ) / p1evl( x, C, 6);
	return( log(z) + p );
    }

    if( x > MAXLGM )
    {
    loverf:
	assert(!"Overflow in lgam");
	return( sgngam * MAXNUM );
    }

    q = ( x - 0.5 ) * log(x) - x + LS2PI;
    if( x > 1.0e8 )
	return( q );

    p = 1.0/(x*x);
    if( x >= 1000.0 )
	q += ((   7.9365079365079365079365e-4 * p
                  - 2.7777777777777777777778e-3) *p
              + 0.0833333333333333333333) / x;
    else
	q += polevl( p, A, 4 ) / x;
    return( q );
}

// Complementary incomplete gamma
static double igamc(double a, double x)
{
    const double big = 4.503599627370496e15;
    const double biginv = 2.22044604925031308085e-16;

    double ans, ax, c, yc, r, t, y, z;
    double pk, pkm1, pkm2, qk, qkm1, qkm2;

    if( (x <= 0) || ( a <= 0) )
	return( 1.0 );

    if( (x < 1.0) || (x < a) )
	return( 1.0 - igam(a,x) );

    // Compute  x**a * exp(-x) / gamma(a)
    ax = a * log(x) - x - lgam(a);
    if( ax < -MAXLOG )
    {
	// mtherr( "igamc", UNDERFLOW );
        // assert(!"Underflow in igamc");
	return 0.0;
    }
    ax = exp(ax);

    // continued fraction
    y = 1.0 - a;
    z = x + y + 1.0;
    c = 0.0;
    pkm2 = 1.0;
    qkm2 = x;
    pkm1 = x + 1.0;
    qkm1 = z * x;
    ans = pkm1/qkm1;

    do
    {
	c += 1.0;
	y += 1.0;
	z += 2.0;
	yc = y * c;
	pk = pkm1 * z  -  pkm2 * yc;
	qk = qkm1 * z  -  qkm2 * yc;
	if( qk != 0 )
        {
            r = pk/qk;
            t = fabs( (ans - r)/r );
            ans = r;
        }
	else
            t = 1.0;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;
	if( fabs(pk) > big )
        {
            pkm2 *= biginv;
            pkm1 *= biginv;
            qkm2 *= biginv;
            qkm1 *= biginv;
        }
    }
    while( t > MACHEP );

    return( ans * ax );
}

// Incomplete gamma
static double igam(double a, double x)
{
    double ans, ax, c, r;

    if( (x <= 0) || ( a <= 0) )
	return( 0.0 );

    if( (x > 1.0) && (x > a ) )
	return( 1.0 - igamc(a,x) );

    // Compute  x**a * exp(-x) / gamma(a)
    ax = a * log(x) - x - lgam(a);
    if( ax < -MAXLOG )
    {
	// mtherr( "igam", UNDERFLOW );
        assert(!"Underflow in igam");
	return( 0.0 );
    }
    ax = exp(ax);

    // power series
    r = a;
    c = 1.0;
    ans = 1.0;

    do
    {
	r += 1.0;
	c *= x/r;
	ans += c;
    }
    while( c/ans > MACHEP );

    return( ans * ax/a );
}

// Inverse of complemented incomplete gamma integral.
// Work only for y0 <= 0.5.
static double igami(double a, double y0)
{
    double x0, x1, x, yl, yh, y, d, lgm, dithresh;
    int i, dir;

    assert(y0 <= 0.5);

    /* bound the solution */
    x0 = MAXNUM;
    yl = 0;
    x1 = 0;
    yh = 1.0;
    dithresh = 5.0 * MACHEP;

    /* approximation to inverse function */
    d = 1.0/(9.0*a);
    y = ( 1.0 - d - invgauss(y0) * sqrt(d) );
    x = a * y * y * y;

    lgm = lgam(a);

    for( i=0; i<10; i++ )
    {
	if( x > x0 || x < x1 )
            goto ihalve;
	y = igamc(a,x);
	if( y < yl || y > yh )
            goto ihalve;
	if( y < y0 )
        {
            x0 = x;
            yl = y;
        }
	else
        {
            x1 = x;
            yh = y;
        }
        /* compute the derivative of the function at this point */
	d = (a - 1.0) * log(x) - x - lgm;
	if( d < -MAXLOG )
            goto ihalve;
	d = -exp(d);
        /* compute the step to the next approximation of x */
	d = (y - y0)/d;
	if( fabs(d/x) < MACHEP )
            goto done;
	x = x - d;
    }

    /* Resort to interval halving if Newton iteration did not converge. */
ihalve:

    d = 0.0625;
    if( x0 == MAXNUM )
    {
	if( x <= 0.0 )
            x = 1.0;
	while( x0 == MAXNUM )
        {
            x = (1.0 + d) * x;
            y = igamc( a, x );
            if( y < y0 )
            {
                x0 = x;
                yl = y;
                break;
            }
            d = d + d;
        }
    }
    d = 0.5;
    dir = 0;

    for( i=0; i<400; i++ )
    {
	x = x1  +  d * (x0 - x1);
	y = igamc( a, x );
	lgm = (x0 - x1)/(x1 + x0);
	if( fabs(lgm) < dithresh )
            break;
	lgm = (y - y0)/y0;
	if( fabs(lgm) < dithresh )
            break;
	if( x <= 0.0 )
            break;
	if( y >= y0 )
        {
            x1 = x;
            yh = y;
            if( dir < 0 )
            {
                dir = 0;
                d = 0.5;
            }
            else if( dir > 1 )
                d = 0.5 * d + 0.5; 
            else
                d = (y0 - yl)/(yh - yl);
            dir += 1;
        }
	else
        {
            x0 = x;
            yl = y;
            if( dir > 0 )
            {
                dir = 0;
                d = 0.5;
            }
            else if( dir < -1 )
                d = 0.5 * d;
            else
                d = (y0 - yl)/(yh - yl);
            dir -= 1;
        }
    }
    if( x == 0.0 )
	// mtherr( "igami", UNDERFLOW );
        assert(!"Underflow in igami");

done:
    return( x );
}

static double invgauss(const double P)
{
    assert(P > 0.0);
    assert(P < 1.0);

    // Translated from PPND16 algorithm of Wichura (originally in Fortran).
    // This was not taken from Cephes.
    static const double ZERO = 0., ONE = 1., HALF = 0.5, 
        SPLIT1 = 0.425, SPLIT2 = 5., 
        CONST1 = 0.180625, CONST2 = 1.6;

    static const double A0 = 3.3871328727963666080, 
        A1 = 1.3314166789178437745E+2, 
        A2 = 1.9715909503065514427E+3, 
        A3 = 1.3731693765509461125E+4, 
        A4 = 4.5921953931549871457E+4, 
        A5 = 6.7265770927008700853E+4, 
        A6 = 3.3430575583588128105E+4, 
        A7 = 2.5090809287301226727E+3, 
        B1 = 4.2313330701600911252E+1, 
        B2 = 6.8718700749205790830E+2, 
        B3 = 5.3941960214247511077E+3, 
        B4 = 2.1213794301586595867E+4, 
        B5 = 3.9307895800092710610E+4, 
        B6 = 2.8729085735721942674E+4, 
        B7 = 5.2264952788528545610E+3;

    static const double C0 = 1.42343711074968357734, 
        C1 = 4.63033784615654529590, 
        C2 = 5.76949722146069140550, 
        C3 = 3.64784832476320460504, 
        C4 = 1.27045825245236838258, 
        C5 = 2.41780725177450611770E-1, 
        C6 = 2.27238449892691845833E-2, 
        C7 = 7.74545014278341407640E-4, 
        D1 = 2.05319162663775882187, 
        D2 = 1.67638483018380384940, 
        D3 = 6.89767334985100004550E-1, 
        D4 = 1.48103976427480074590E-1, 
        D5 = 1.51986665636164571966E-2, 
        D6 = 5.47593808499534494600E-4, 
        D7 = 1.05075007164441684324E-9;

    static const double E0 = 6.65790464350110377720, 
        E1 = 5.46378491116411436990, 
        E2 = 1.78482653991729133580, 
        E3 = 2.96560571828504891230E-1, 
        E4 = 2.65321895265761230930E-2, 
        E5 = 1.24266094738807843860E-3, 
        E6 = 2.71155556874348757815E-5, 
        E7 = 2.01033439929228813265E-7, 
        F1 = 5.99832206555887937690E-1, 
        F2 = 1.36929880922735805310E-1, 
        F3 = 1.48753612908506148525E-2, 
        F4 = 7.86869131145613259100E-4, 
        F5 = 1.84631831751005468180E-5, 
        F6 = 1.42151175831644588870E-7, 
        F7 = 2.04426310338993978564E-15;

    const double Q = P - HALF;

    double R, PPND16;
    if (fabs(Q) <= SPLIT1)
    {
        R = CONST1 - Q * Q;
        PPND16 = Q * (((((((A7 * R + A6) * R + A5) * R + A4) * R + A3) 
                        * R + A2) * R + A1) * R + A0) / 
            (((((((B7 * R + B6) * R + B5) * R + B4) * R + B3) 
               * R + B2) * R + B1) * R + ONE);
    }
    else
    {
        if (Q < ZERO)
            R = P;
        else
            R = ONE - P;
        R = sqrt(-log(R));
        if (R <= SPLIT2)
        {
            R = R - CONST2;
            PPND16 = (((((((C7 * R + C6) * R + C5) * R + C4) * R + C3) 
                        * R + C2) * R + C1) * R + C0) / 
                (((((((D7 * R + D6) * R + D5) * R + D4) * R + D3) 
                   * R + D2) * R + D1) * R + ONE);
        }
        else
        {
            R = R - SPLIT2 ;
            PPND16 = (((((((E7 * R + E6) * R + E5) * R + E4) * R + E3) 
                        * R + E2) * R + E1) * R + E0) / 
                (((((((F7 * R + F6) * R + F5) * R + F4) * R + F3) 
                   * R + F2) * R + F1) * R + ONE);
        }
        if (Q < ZERO)
            PPND16 = -PPND16;
    }
    return PPND16;
}

/* Continued fraction expansion #1
 * for incomplete beta integral
 */
static double incbcf( double a, double b, double x )
{
    static const double big = 4.503599627370496e15;
    static const double biginv = 2.22044604925031308085e-16;

    double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
    double k1, k2, k3, k4, k5, k6, k7, k8;
    double r, t, ans, thresh;
    int n;

    k1 = a;
    k2 = a + b;
    k3 = a;
    k4 = a + 1.0;
    k5 = 1.0;
    k6 = b - 1.0;
    k7 = k4;
    k8 = a + 2.0;

    pkm2 = 0.0;
    qkm2 = 1.0;
    pkm1 = 1.0;
    qkm1 = 1.0;
    ans = 1.0;
    r = 1.0;
    n = 0;
    thresh = 3.0 * MACHEP;
    do
    {
	
	xk = -( x * k1 * k2 )/( k3 * k4 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	xk = ( x * k5 * k6 )/( k7 * k8 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	if( qk != 0 )
            r = pk/qk;
	if( r != 0 )
        {
            t = fabs( (ans - r)/r );
            ans = r;
        }
	else
            t = 1.0;

	if( t < thresh )
            goto cdone;

	k1 += 1.0;
	k2 += 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 -= 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabs(qk) + fabs(pk)) > big )
        {
            pkm2 *= biginv;
            pkm1 *= biginv;
            qkm2 *= biginv;
            qkm1 *= biginv;
        }
	if( (fabs(qk) < biginv) || (fabs(pk) < biginv) )
        {
            pkm2 *= big;
            pkm1 *= big;
            qkm2 *= big;
            qkm1 *= big;
        }
    }
    while( ++n < 300 );

cdone:
    return(ans);
}

/* Continued fraction expansion #2
 * for incomplete beta integral
 */
static double incbd( double a, double b, double x )
{
    static const double big = 4.503599627370496e15;
    static const double biginv = 2.22044604925031308085e-16;

    double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
    double k1, k2, k3, k4, k5, k6, k7, k8;
    double r, t, ans, z, thresh;
    int n;

    k1 = a;
    k2 = b - 1.0;
    k3 = a;
    k4 = a + 1.0;
    k5 = 1.0;
    k6 = a + b;
    k7 = a + 1.0;;
    k8 = a + 2.0;

    pkm2 = 0.0;
    qkm2 = 1.0;
    pkm1 = 1.0;
    qkm1 = 1.0;
    z = x / (1.0-x);
    ans = 1.0;
    r = 1.0;
    n = 0;
    thresh = 3.0 * MACHEP;
    do
    {
	
	xk = -( z * k1 * k2 )/( k3 * k4 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	xk = ( z * k5 * k6 )/( k7 * k8 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	if( qk != 0 )
            r = pk/qk;
	if( r != 0 )
        {
            t = fabs( (ans - r)/r );
            ans = r;
        }
	else
            t = 1.0;

	if( t < thresh )
            goto cdone;

	k1 += 1.0;
	k2 -= 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 += 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabs(qk) + fabs(pk)) > big )
        {
            pkm2 *= biginv;
            pkm1 *= biginv;
            qkm2 *= biginv;
            qkm1 *= biginv;
        }
	if( (fabs(qk) < biginv) || (fabs(pk) < biginv) )
        {
            pkm2 *= big;
            pkm1 *= big;
            qkm2 *= big;
            qkm1 *= big;
        }
    }
    while( ++n < 300 );
cdone:
    return(ans);
}

/* Power series for incomplete beta integral.
   Use when b*x is small and x not too close to 1.  */
static double pseries( double a, double b, double x )
{
    double s, t, u, v, n, t1, z, ai;

    ai = 1.0 / a;
    u = (1.0 - b) * x;
    v = u / (a + 1.0);
    t1 = v;
    t = u;
    n = 2.0;
    s = 0.0;
    z = MACHEP * ai;
    while( fabs(v) > z )
    {
	u = (n - b) * x / n;
	t *= u;
	v = t / (a + n);
	s += v; 
	n += 1.0;
    }
    s += t1;
    s += ai;

    u = a * log(x);
    if( (a+b) < MAXGAM && fabs(u) < MAXLOG )
    {
	t = npstat::Gamma(a+b)/(npstat::Gamma(a)*npstat::Gamma(b));
	s = s * t * pow(x,a);
    }
    else
    {
	t = lgam(a+b) - lgam(a) - lgam(b) + u + log(s);
	if( t < MINLOG )
            s = 0.0;
	else
            s = exp(t);
    }
    return(s);
}

/* Incomplete beta function */
static double incbet( double aa, double bb, double xx )
{
    double a, b, t, x, xc, w, y;
    int flag;

    if( aa <= 0.0 || bb <= 0.0 )
	goto domerr;

    if( (xx <= 0.0) || ( xx >= 1.0) )
    {
	if( xx == 0.0 )
            return(0.0);
	if( xx == 1.0 )
            return( 1.0 );
    domerr:
	// mtherr( "incbet", DOMAIN );
        assert(!"Domain error in incbet");
	return( 0.0 );
    }

    flag = 0;
    if( (bb * xx) <= 1.0 && xx <= 0.95)
    {
	t = pseries(aa, bb, xx);
        goto done;
    }

    w = 1.0 - xx;

    /* Reverse a and b if x is greater than the mean. */
    if( xx > (aa/(aa+bb)) )
    {
	flag = 1;
	a = bb;
	b = aa;
	xc = xx;
	x = w;
    }
    else
    {
	a = aa;
	b = bb;
	xc = w;
	x = xx;
    }

    if( flag == 1 && (b * x) <= 1.0 && x <= 0.95)
    {
	t = pseries(a, b, x);
	goto done;
    }

    /* Choose expansion for better convergence. */
    y = x * (a+b-2.0) - (a-1.0);
    if( y < 0.0 )
	w = incbcf( a, b, x );
    else
	w = incbd( a, b, x ) / xc;

   /* Multiply w by the factor
   a      b   _             _     _
   x  (1-x)   | (a+b) / ( a | (a) | (b) ) .   */

    y = a * log(x);
    t = b * log(xc);
    if( (a+b) < MAXGAM && fabs(y) < MAXLOG && fabs(t) < MAXLOG )
    {
	t = pow(xc,b);
	t *= pow(x,a);
	t /= a;
	t *= w;
	t *= npstat::Gamma(a+b) / (npstat::Gamma(a) * npstat::Gamma(b));
	goto done;
    }

    /* Resort to logarithms.  */
    y += t + lgam(a+b) - lgam(a) - lgam(b);
    y += log(w/a);
    if( y < MINLOG )
	t = 0.0;
    else
	t = exp(y);

done:

    if( flag == 1 )
    {
	if( t <= MACHEP )
            t = 1.0 - MACHEP;
	else
            t = 1.0 - t;
    }
    return( t );
}

/* Inverse incomplete beta function */
static double incbi( double aa, double bb, double yy0 )
{
    double a, b, y0, d, y, x, x0, x1, lgm, yp, di, dithresh, yl, yh, xt;
    int i, rflg, dir, nflg;

    i = 0;
    if( yy0 <= 0.0 )
	return(0.0);
    if( yy0 >= 1.0 )
	return(1.0);
    x0 = 0.0;
    yl = 0.0;
    x1 = 1.0;
    yh = 1.0;
    nflg = 0;

    if( aa <= 1.0 || bb <= 1.0 )
    {
	dithresh = 1.0e-6;
	rflg = 0;
	a = aa;
	b = bb;
	y0 = yy0;
	x = a/(a+b);
	y = incbet( a, b, x );
	goto ihalve;
    }
    else
    {
	dithresh = 1.0e-4;
    }
    /* approximation to inverse function */

    yp = -npstat::inverseGaussCdf(yy0);

    if( yy0 > 0.5 )
    {
	rflg = 1;
	a = bb;
	b = aa;
	y0 = 1.0 - yy0;
	yp = -yp;
    }
    else
    {
	rflg = 0;
	a = aa;
	b = bb;
	y0 = yy0;
    }

    lgm = (yp * yp - 3.0)/6.0;
    x = 2.0/( 1.0/(2.0*a-1.0)  +  1.0/(2.0*b-1.0) );
    d = yp * sqrt( x + lgm ) / x
	- ( 1.0/(2.0*b-1.0) - 1.0/(2.0*a-1.0) )
	* (lgm + 5.0/6.0 - 2.0/(3.0*x));
    d = 2.0 * d;
    if( d < MINLOG )
    {
	x = 1.0;
	goto under;
    }
    x = a/( a + b * exp(d) );
    y = incbet( a, b, x );
    yp = (y - y0)/y0;
    if( fabs(yp) < 0.2 )
	goto newt;

    /* Resort to interval halving if not close enough. */
ihalve:

    dir = 0;
    di = 0.5;
    for( i=0; i<100; i++ )
    {
	if( i != 0 )
        {
            x = x0  +  di * (x1 - x0);
            if( x == 1.0 )
                x = 1.0 - MACHEP;
            if( x == 0.0 )
            {
                di = 0.5;
                x = x0  +  di * (x1 - x0);
                if( x == 0.0 )
                    goto under;
            }
            y = incbet( a, b, x );
            yp = (x1 - x0)/(x1 + x0);
            if( fabs(yp) < dithresh )
                goto newt;
            yp = (y-y0)/y0;
            if( fabs(yp) < dithresh )
                goto newt;
        }
	if( y < y0 )
        {
            x0 = x;
            yl = y;
            if( dir < 0 )
            {
                dir = 0;
                di = 0.5;
            }
            else if( dir > 3 )
                di = 1.0 - (1.0 - di) * (1.0 - di);
            else if( dir > 1 )
                di = 0.5 * di + 0.5; 
            else
                di = (y0 - y)/(yh - yl);
            dir += 1;
            if( x0 > 0.75 )
            {
                if( rflg == 1 )
                {
                    rflg = 0;
                    a = aa;
                    b = bb;
                    y0 = yy0;
                }
                else
                {
                    rflg = 1;
                    a = bb;
                    b = aa;
                    y0 = 1.0 - yy0;
                }
                x = 1.0 - x;
                y = incbet( a, b, x );
                x0 = 0.0;
                yl = 0.0;
                x1 = 1.0;
                yh = 1.0;
                goto ihalve;
            }
        }
	else
        {
            x1 = x;
            if( rflg == 1 && x1 < MACHEP )
            {
                x = 0.0;
                goto done;
            }
            yh = y;
            if( dir > 0 )
            {
                dir = 0;
                di = 0.5;
            }
            else if( dir < -3 )
                di = di * di;
            else if( dir < -1 )
                di = 0.5 * di;
            else
                di = (y - y0)/(yh - yl);
            dir -= 1;
        }
    }

    // Partial loss of precision. Hm... Will ignore for now - igv
    // mtherr( "incbi", PLOSS );
    if( x0 >= 1.0 )
    {
	x = 1.0 - MACHEP;
	goto done;
    }
    if( x <= 0.0 )
    {
    under:
	// mtherr( "incbi", UNDERFLOW );
        assert(!"Underflow in incbi");
	x = 0.0;
	goto done;
    }

newt:

    if( nflg )
	goto done;
    nflg = 1;
    lgm = lgam(a+b) - lgam(a) - lgam(b);

    for( i=0; i<8; i++ )
    {
	/* Compute the function at this point. */
	if( i != 0 )
            y = incbet(a,b,x);
	if( y < yl )
        {
            x = x0;
            y = yl;
        }
	else if( y > yh )
        {
            x = x1;
            y = yh;
        }
	else if( y < y0 )
        {
            x0 = x;
            yl = y;
        }
	else
        {
            x1 = x;
            yh = y;
        }
	if( x == 1.0 || x == 0.0 )
            break;
	/* Compute the derivative of the function at this point. */
	d = (a - 1.0) * log(x) + (b - 1.0) * log(1.0-x) + lgm;
	if( d < MINLOG )
            goto done;
	if( d > MAXLOG )
            break;
	d = exp(d);
	/* Compute the step to the next approximation of x. */
	d = (y - y0)/d;
	xt = x - d;
	if( xt <= x0 )
        {
            y = (x - x0) / (x1 - x0);
            xt = x0 + 0.5 * y * (x - x0);
            if( xt <= 0.0 )
                break;
        }
	if( xt >= x1 )
        {
            y = (x1 - x) / (x1 - x0);
            xt = x1 - 0.5 * y * (x1 - x);
            if( xt >= 1.0 )
                break;
        }
	x = xt;
	if( fabs(d/x) < 128.0 * MACHEP )
            goto done;
    }

    /* Did not converge.  */
    dithresh = 256.0 * MACHEP;
    goto ihalve;

done:

    if( rflg )
    {
	if( x <= MACHEP )
            x = 1.0 - MACHEP;
	else
            x = 1.0 - x;
    }
    return( x );
}

static void buildGaussDeriCoeffs(const unsigned order,
                                 std::vector<long long>* coeffs)
{
    assert(coeffs);
    const unsigned dim = order + 1U;
    coeffs->resize(dim);
    if (order == 0U)
    {
        (*coeffs)[0] = 1LL;
        return;
    }
    npstat::Matrix<long long> deriOp(dim, dim, 0);
    for (unsigned row=0; row<dim-1U; ++row)
        deriOp[row][row+1U] = static_cast<long long>(row+1U);
    for (unsigned row=1; row<dim; ++row)
        deriOp[row][row-1U] = -1LL;
    const npstat::Matrix<long long>& result = deriOp.pow(order);
    for (unsigned row=0; row<dim; ++row)
        (*coeffs)[row] = result[row][0];
}

static double Phi(const double x)
{
    const double sqrt2v = 1.41421356237309505;
    if (x < 0.0)
        return erfc(-x/sqrt2v)/2.0;
    else
        return (1.0 + erf(x/sqrt2v))/2.0;
}

// Evaluation of the cumulative bivariate normal distribution
// on the diagonal (x, x) for x <= 0 and correlation rho == 1 - a >= 0
// px = Phi( x ) (cumulative std. normal)
// pxs = Phi( x * sqrt( ( 1 - rho ) / ( 1 + rho ) ) )
// Phi2diag() should not be used directly but Phi2() instead.
static double Phi2diag( const double x,
                        const double a,
                        const double px,
                        const double pxs )
{
    if( a <= 0.0 ) return px;        // rho == 1
    if( a >= 1.0 ) return px * px;   // rho == 0

    double b = 2.0 - a, sqrt_ab = sqrt( a * b );   // b == 1 + rho

    const double c1 = 6.36619772367581343e-1;      // == 2 / PI
    const double c2 = 1.25331413731550025;         // == sqrt( PI / 2 )
    const double c3 = 1.57079632679489662;         // == PI / 2
    const double c4 = 1.591549430918953358e-1;     // == 1 / ( 2 * PI )

    // avoid asin() for values close to 1 (vertical tangent)
    double asr = ( a > 0.1 ? asin( 1.0 - a ) : acos( sqrt_ab ) );
    
    // return upper bound if absolute error within double precision
    double comp = px * pxs;
    if( comp * ( 1.0 - a - c1 * asr ) < 5e-17 )
        return b * comp;

    // initialize odd and even terms for recursion
    double tmp = c2 * x;
    double alpha = a * x * x / b;
    double a_even = -tmp * a;
    double a_odd = -sqrt_ab * alpha;
    double beta = x * x;
    double b_even = tmp * sqrt_ab;
    double b_odd = sqrt_ab * beta;
    double delta = 2.0 * x * x / b;
    double d_even = ( 1.0 - a ) * c3 - asr;
    double d_odd = tmp * ( sqrt_ab - a );

    // recursion; update odd and even terms in each step
    double res = 0.0, res_new = d_even + d_odd;
    int k = 2;
    while( res != res_new )
    {
        d_even = ( a_odd + b_odd + delta * d_even ) / k;
        a_even *= alpha / k;
        b_even *= beta / k;
        k++;
        a_odd *= alpha / k;
        b_odd *= beta / k;
        d_odd = ( a_even + b_even + delta * d_odd ) / k;
        k++;
        res = res_new;
        res_new += d_even + d_odd;
    }

    // transform; check against upper and lower bounds
    res *= exp( -x * x / b ) * c4;
    return std::max( ( 1.0 + c1 * asr ) * comp,
                     b * comp - std::max( 0.0, res ) );
}

static double squarethis(const double v)
{
    return v * v;
}

// Auxiliary function Phi2help() will be called (twice, with x and y interchanged) by Phi2()
// In particular, certain pathological cases will have been dealt with in Phi2() before
// Axis is transformed into diagonal, and Phi2diag() is called
static double Phi2help( const double x,
                        const double y,
                        const double rho )
{
    // note: case x == y == 0.0 is dealt with in Phi2()
    if( fabs(x) <= DBL_MIN ) return ( y >= 0.0 ? 0.0 : 0.5 );

    double s = sqrt( ( 1.0 - rho ) * ( 1.0 + rho ) );
    double a = 0.0, b1 = -fabs( x ), b2 = 0.0;

    // avoid cancellation by treating the cases rho -> +-1
    // separately (cutoff at |0.99| might be optimized);
    if( rho > 0.99 )
    {
        double tmp = sqrt( ( 1.0 - rho ) / ( 1.0 + rho ) );
        b2 = -fabs( ( x - y ) / s - x * tmp );
        a = squarethis( ( x - y ) / x / s - tmp );
    }
    else if( rho < -0.99 )
    {
        double tmp = sqrt( ( 1.0 + rho ) / ( 1.0 - rho ) );
        b2 = -fabs( ( x + y ) / s - x * tmp );
        a = squarethis( ( x + y ) / x / s - tmp );
    }
    else
    {
        b2 = -fabs( rho * x - y ) / s;
        a = squarethis( b2 / x );
    }

    // Phi(): cumulative std. normal (has to be defined elsewhere)
    double p1 = Phi( b1 ), p2 = Phi( b2 );

    // reduction to the diagonal
    double q = 0.0;
    if( a <= 1.0 )
        q = 0.5 * Phi2diag( b1, 2.0 * a / ( 1.0 + a ), p1, p2 );
    else
        q = p1 * p2 - 0.5 * Phi2diag( b2, 2.0 / ( 1.0 + a ), p2, p1 );

    // finally, transformation according to conditions on x, y, rho
    int c1 = ( y / x >= rho ), c2 = ( x < 0.0 ), c3 = c2 && ( y >= 0.0 );
    return ( c1 && c3 ? q - 0.5
                      : c1 && c2 ? q
                      : c1 ? 0.5 - p1 + q
                      : c3 ? p1 - q - 0.5
                      : c2 ? p1 - q
                      : 0.5 - q );
}

// Evaluation of the cumulative bivariate normal distribution
// for arbitrary (x,y) and correlation rho. This function is
// described in the article "Recursive Numerical Evaluation of
// the Cumulative Bivariate Normal Distribution" by Christian Meyer,
// Journal of Statistical Software, Vol. 52, Issue 10, Mar 2013.
//
// It appears not to work very well for large rho (for example,
// Phi2(-0, 0.5, -0.98865243049242746) is wrong), so for large
// values of rho implementation by Genz is used (which has its
// own problems).
//
static double Phi2( const double x,
                    const double y,
                    const double rho )
{
    // special case |rho| == 1 => lower or upper Frechet bound
    // Phi(): cumulative std. normal (has to be defined elsewhere)
    if( ( 1.0 - rho ) * ( 1.0 + rho ) <= 0.0 )
    {
        if( rho > 0.0 )
            return Phi( std::min( x, y ) );
        else
            return std::max( 0.0, std::min( 1.0, Phi( x ) + Phi( y ) - 1.0 ) );
    }

    // special case x == y == 0, avoids complications in Phi2help()
    if( x == 0.0 && y == 0.0 )
    {
        if( rho > 0.0 )
            return Phi2diag( 0.0, 1.0 - rho, 0.5, 0.5 );
        else
            return 0.5 - Phi2diag( 0.0, 1.0 + rho, 0.5, 0.5 );
    }

    // standard case by reduction formula
    const double result = std::max( 0.0,
                          std::min( 1.0,
                          Phi2help( x, y, rho ) + Phi2help( y, x, rho ) ) );

    // std::cout.precision(17);
    // std::cout << "Phi2(" << x << ", " << y << ", " << rho << ") = " << result << std::endl;

    return result;
}

/*
 *     A function for computing bivariate normal probabilities.
 *
 *       Alan Genz
 *       Department of Mathematics
 *       Washington State University
 *       Pullman, WA 99164-3113
 *       Email : alangenz@wsu.edu
 *
 *    This function is based on the method described by 
 *        Drezner, Z and G.O. Wesolowsky, (1989),
 *        On the computation of the bivariate normal integral,
 *        Journal of Statist. Comput. Simul. 35, pp. 101-107,
 *    with major modifications for double precision, and for |R| close to 1.
 *
 * BVND calculates the probability that X > DH and Y > DK.
 *      Note: Prob( X < DH, Y < DK ) = BVND( -DH, -DK, R ).
 *
 * Parameters
 *
 *   DH  DOUBLE PRECISION, integration limit
 *   DK  DOUBLE PRECISION, integration limit
 *   R   DOUBLE PRECISION, correlation coefficient
 *
 * Translated from Fortran by igv (July 2014).
 */
static double bvnd(const double DH, const double DK, const double R)
{
    const double TWOPI = 2.0*M_PI;

    // Gauss-Legendre points
    static const double X[11][4] = {
        {0., 0.                 , 0.                 , 0.                 },
        {0., -0.9324695142031522, -0.9815606342467191, -0.9931285991850949},
        {0., -0.6612093864662647, -0.9041172563704750, -0.9639719272779138},
        {0., -0.2386191860831970, -0.7699026741943050, -0.9122344282513259},
        {0., 0.                 , -0.5873179542866171, -0.8391169718222188},
        {0., 0.                 , -0.3678314989981802, -0.7463319064601508},
        {0., 0.                 , -0.1252334085114692, -0.6360536807265150},
        {0., 0.                 , 0.                 , -0.5108670019508271},
        {0., 0.                 , 0.                 , -0.3737060887154196},
        {0., 0.                 , 0.                 , -0.2277858511416451},
        {0., 0.                 , 0.                 , -0.7652652113349733}
    };

    // Gauss-Legendre weights
    static const double W[11][4] = {
        {0., 0.                , 0.                 , 0.                 },
        {0., 0.1713244923791705, 0.04717533638651177, 0.01761400713915212},
        {0., 0.3607615730481384, 0.1069393259953183 , 0.04060142980038694},
        {0., 0.4679139345726904, 0.1600783285433464 , 0.06267204833410906},
        {0., 0.                , 0.2031674267230659 , 0.08327674157670475},
        {0., 0.                , 0.2334925365383547 , 0.1019301198172404 },
        {0., 0.                , 0.2491470458134029 , 0.1181945319615184 },
        {0., 0.                , 0.                 , 0.1316886384491766 },
        {0., 0.                , 0.                 , 0.1420961093183821 },
        {0., 0.                , 0.                 , 0.1491729864726037 },
        {0., 0.                , 0.                 , 0.1527533871307259 }
    };

    int I, IS, LG, NG;
    double AS, A, B, C, D, RS, XS, BVN;
    double SN, ASR, H, K, BS, HS, HK;

    if ( fabs(R) < 0.3 )
    {
        NG = 1;
        LG = 3;
    }
    else if ( fabs(R) < 0.75 )
    {
        NG = 2;
        LG = 6;
    }
    else
    {
        NG = 3;
        LG = 10;
    }
    H = DH;
    K = DK;
    HK = H*K;
    BVN = 0.0;

    if ( fabs(R) < 0.925 )
    {
        if ( fabs(R) > 0.0 )
        {
            HS = ( H*H + K*K )/2.0;
            ASR = asin(R);
            for (I = 1; I <= LG; ++I)
            {
                for (IS = -1; IS <= 1; IS += 2)
                {
                    SN = sin( ASR*(  IS*X[I][NG] + 1.0 )/2.0 );
                    BVN += W[I][NG]*exp( ( SN*HK-HS )/( 1.0-SN*SN ) );
                }
            }
            BVN = BVN*ASR/( 2.0*TWOPI );
        }
        BVN += Phi(-H)*Phi(-K);
    }
    else
    {
        if ( R < 0.0 )
        {
            K = -K;
            HK = -HK;
        }
        if ( fabs(R) < 1.0 )
        {
            AS = ( 1 - R )*( 1 + R );
            A = sqrt(AS);
            BS = pow(H - K, 2);
            C = ( 4 - HK )/8.0;
            D = ( 12 - HK )/16.0;
            ASR = -( BS/AS + HK )/2.0;
            if ( ASR > -100.0 )
                BVN = A*exp(ASR)*( 1 - C*( BS - AS )*( 1 - D*BS/5 )/3 +
                                   C*D*AS*AS/5 );
            if ( -HK < 100.0 )
            {
                B = sqrt(BS);
                BVN -= exp( -HK/2 )*SQTPI*Phi(-B/A)*B
                    *( 1 - C*BS*( 1 - D*BS/5 )/3 );
            }
            A /= 2.0;
            for (I = 1; I <= LG; ++I)
            {
                for (IS = -1; IS <= 1; IS += 2)
                {
                    XS = pow( A*(  IS*X[I][NG] + 1 ), 2);
                    RS = sqrt( 1 - XS );
                    ASR = -( BS/XS + HK )/2;
                    if ( ASR > -100 )
                        BVN += A*W[I][NG]*exp( ASR )
                            *( exp( -HK*XS/( 2*pow(1 + RS, 2) ) )/RS        
                               - ( 1 + C*XS*( 1 + D*XS ) ) );
                }
            }
            BVN = -BVN/TWOPI;
        }
        if ( R > 0.0 )
            BVN += Phi( -std::max( H, K ) );
        else
        {
            BVN = -BVN;
            if ( K > H )
            {
                if ( H < 0.0 )
                    BVN += Phi(K)  - Phi(H);
                else
                    BVN += Phi(-H) - Phi(-K);
            }
        }
    }
    return BVN;
}

namespace npstat {
    double inverseGaussCdf(const double x)
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::inverseGaussCdf: argument outside of [0, 1] interval");
        if (x == 1.0)
            return GAUSS_MAX_SIGMA;
        else if (x == 0.0)
            return -GAUSS_MAX_SIGMA;
        else
            return invgauss(x);
    }

    double incompleteBeta(const double a, const double b,
                          const double x)
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::incompleteBeta: argument outside of [0, 1] interval");
        if (!(a > 0.0 && b > 0.0)) throw std::invalid_argument(
            "In npstat::incompleteBeta: parameters must be positive");
        return incbet(a, b, x);
    }

    double inverseIncompleteBeta(const double a, const double b,
                                 const double x)
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::inverseIncompleteBeta: "
            "argument outside of [0, 1] interval");
        if (!(a > 0.0 && b > 0.0)) throw std::invalid_argument(
            "In npstat::inverseIncompleteBeta: parameters must be positive");
        return incbi(a, b, x);
    }

    double Gamma(double x)
    {
        if (x <= 0.0) throw std::domain_error(
            "In npstat::Gamma: argument must be positive");

        // The Stirling formula overflows for x >= MAXGAM
        if (x >= MAXGAM) throw std::overflow_error(
            "In npstat::Gamma: argument is too large");

        const unsigned ix = x;
        if (ix && x == static_cast<double>(ix))
            return ldfactorial(ix - 1U);

        static const double P[] = {
            1.60119522476751861407E-4,
            1.19135147006586384913E-3,
            1.04213797561761569935E-2,
            4.76367800457137231464E-2,
            2.07448227648435975150E-1,
            4.94214826801497100753E-1,
            9.99999999999999996796E-1
        };
        static const double Q[] = {
            -2.31581873324120129819E-5,
            5.39605580493303397842E-4,
            -4.45641913851797240494E-3,
            1.18139785222060435552E-2,
            3.58236398605498653373E-2,
            -2.34591795718243348568E-1,
            7.14304917030273074085E-2,
            1.00000000000000000320E0
        };

        double p, z = 1.0, q = x;

        if (q > 33.0)
            return stirf(q);

        while( x >= 3.0 )
	{
            x -= 1.0;
            z *= x;
	}

        while( x < 2.0 )
	{
            if( x < 1.e-9 )
		return( z/((1.0 + 0.5772156649015329 * x) * x) );
            z /= x;
            x += 1.0;
	}

        if( x == 2.0 )
            return(z);

        x -= 2.0;
        p = polevl( x, P, 6 );
        q = polevl( x, Q, 7 );
        return( z * p / q );
    }

    double incompleteGamma(const double a, const double x)
    {
        return igam(a, x);
    }

    double incompleteGammaC(const double a, const double x)
    {
        return igamc(a, x);
    }

    double inverseIncompleteGammaC(const double a, const double x)
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::inverseIncompleteGammaC: "
            "argument outside of [0, 1] interval");
        if (x <= 0.5)
            return igami(a, x);
        else
            return inverseIncompleteGamma(a, 1.0 - x);
    }

    double inverseIncompleteGamma(const double a, const double x)
    {
        if (!(x >= 0.0 && x <= 1.0)) throw std::domain_error(
            "In npstat::inverseIncompleteGamma: "
            "argument outside of [0, 1] interval");
        if (x >= 0.5)
            return igami(a, 1.0 - x);
        else
        {
            const double targetEps = 8.0*MACHEP;
            double xmin = 0.0;
            double xmax = igami(a, 0.5);
            while ((xmax - xmin)/(xmax + xmin + 1.0) > targetEps)
            {
                const double xtry = (xmax + xmin)/2.0;
                if (igam(a, xtry) > x)
                    xmax = xtry;
                else
                    xmin = xtry;
            }
            return (xmax + xmin)/2.0;
        }
    }

    long double dawsonIntegral(const long double x_in)
    {
        const unsigned deg = 20U;
        const long double x = fabsl(x_in);
        long double result = 0.0L;

        if (x == 0.0L)
            ;
        else if (x <= 1.0L)
        {
            const long double epsilon = LDBL_EPSILON/2.0L; 
            const long double twoxsq = -2.0L*x*x;
            long double term = 1.0L;
            long double dfact = 1.0L;
            long double xn = 1.0L;
            unsigned y = 0;
    
            do {
                result += term;
                y += 1U;
                dfact *= (2U*y + 1U);
                xn *= twoxsq;
                term = xn/dfact;
            } while (fabsl(term) > epsilon * fabsl(result));

            result *= x;
        }
        else if (x <= 1.75L)
        {
            static const long double coeffs[deg + 1U] = {
                +4.563960711239483142081e-1L,  -9.268566100670767619861e-2L,
                -7.334392170021420220239e-3L,  +3.379523740404396755124e-3L,
                -3.085898448678595090813e-4L,  -1.519846724619319512311e-5L,
                +4.903955822454009397182e-6L,  -2.106910538629224721838e-7L,
                -2.930676220603996192089e-8L,  +3.326790071774057337673e-9L,
                +3.335593457695769191326e-11L, -2.279104036721012221982e-11L,
                +7.877561633156348806091e-13L, +9.173158167107974472228e-14L,
                -7.341175636102869400671e-15L, -1.763370444125849029511e-16L,
                +3.792946298506435014290e-17L, -4.251969162435936250171e-19L,
                -1.358295820818448686821e-19L, +5.268740962820224108235e-21L,
                +3.414939674304748094484e-22L
            };
            const long double midpoint = (1.75L + 1.0L)/2.0L;
            const long double halflen = (1.75L - 1.0L)/2.0L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        else if (x <= 2.5L)
        {
            static const long double coeffs[deg + 1U] = {
                +2.843711194548592808550e-1L, -6.791774139166808940530e-2L,
                +6.955211059379384327814e-3L,  -2.726366582146839486784e-4L,
                -6.516682485087925163874e-5L,  +1.404387911504935155228e-5L,
                -1.103288540946056915318e-6L,  -1.422154597293404846081e-8L,
                +1.102714664312839585330e-8L,  -8.659211557383544255053e-10L,
                -8.048589443963965285748e-12L, +6.092061709996351761426e-12L,
                -3.580977611213519234324e-13L, -1.085173558590137965737e-14L,
                +2.411707924175380740802e-15L, -7.760751294610276598631e-17L,
                -6.701490147030045891595e-18L, +6.350145841254563572100e-19L,
                -2.034625734538917052251e-21L, -2.260543651146274653910e-21L,
                +9.782419961387425633151e-23L
            };
            const long double midpoint = (2.5L + 1.75L)/2.0L;
            const long double halflen = (2.5L - 1.75L)/2.0L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        else if (x <= 3.25L)
        {
            static const long double coeffs[deg + 1U] = {
                +1.901351274204578126827e-1L,  -3.000575522193632460118e-2L,
                +2.672138524890489432579e-3L,  -2.498237548675235150519e-4L,
                +2.013483163459701593271e-5L,  -8.454663603108548182962e-7L,
                -8.036589636334016432368e-8L,  +2.055498509671357933537e-8L,
                -2.052151324060186596995e-9L,  +8.584315967075483822464e-11L,
                +5.062689357469596748991e-12L, -1.038671167196342609090e-12L,
                +6.367962851860231236238e-14L, +3.084688422647419767229e-16L,
                -3.417946142546575188490e-16L, +2.311567730100119302160e-17L,
                -6.170132546983726244716e-20L, -9.133176920944950460847e-20L,
                +5.712092431423316128728e-21L, +1.269641078369737220790e-23L,
                -2.072659711527711312699e-23L
            };
            const long double midpoint = (3.25L + 2.5L)/2.0L;
            const long double halflen = (3.25L - 2.5L)/2.0L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        else if (x <= 4.25L)
        {
            static const long double coeffs[deg + 1U] = {
                +1.402884974484995678749e-1L,  -2.053975371995937033959e-2L,
                +1.595388628922920119352e-3L,  -1.336894584910985998203e-4L,
                +1.224903774178156286300e-5L,  -1.206856028658387948773e-6L,
                +1.187997233269528945503e-7L,  -1.012936061496824448259e-8L,
                +5.244408240062370605664e-10L, +2.901444759022254846562e-11L,
                -1.168987502493903926906e-11L, +1.640096995420504465839e-12L,
                -1.339190668554209618318e-13L, +3.643815972666851044790e-15L,
                +6.922486581126169160232e-16L, -1.158761251467106749752e-16L,
                +8.164320395639210093180e-18L, -5.397918405779863087588e-20L,
                -5.052069908100339242896e-20L, +5.322512674746973445361e-21L,
                -1.869294542789169825747e-22L
            };
            const long double midpoint = (4.25L + 3.25L)/2.0L;
            const long double halflen = (4.25L - 3.25L)/2.0L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        else if (x <= 5.5L)
        {
            static const long double coeffs[deg + 1U] = {
                +1.058610209741581514157e-1L,  -1.429297757627935191694e-2L,
                +9.911301703835545472874e-4L,  -7.079903107876049846509e-5L,
                +5.229587914675267516134e-6L,  -4.016071345964089296212e-7L,
                +3.231734714422926453741e-8L,  -2.752870944370338482109e-9L,
                +2.503059741885009530630e-10L, -2.418699000594890423278e-11L,
                +2.410158905786160001792e-12L, -2.327254341132174000949e-13L,
                +1.958284411563056492727e-14L, -1.099893145048991004460e-15L,
                -2.959085292526991317697e-17L, +1.966366179276295203082e-17L,
                -3.314408783993662492621e-18L, +3.635520318133814622089e-19L,
                -2.550826919215104648800e-20L, +3.830090587178262542288e-22L,
                +1.836693763159216122739e-22L
            };
            const long double midpoint = (5.5L + 4.25L)/2.0L;
            const long double halflen = (5.5L - 4.25L)/2.0L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        else if (x <= 7.25L)
        {
            static const long double coeffs[deg + 1U] = {
                +8.024637207807814739314e-2L,  -1.136614891549306029413e-2L,
                +8.164249750628661856014e-4L,  -5.951964778701328943018e-5L,
                +4.407349502747483429390e-6L,  -3.317746826184531133862e-7L,
                +2.541483569880571680365e-8L,  -1.983391157250772649001e-9L,
                +1.579050614491277335581e-10L, -1.284592098551537518322e-11L,
                +1.070070857004674207604e-12L, -9.151832297362522251950e-14L,
                +8.065447314948125338081e-15L, -7.360105847607056315915e-16L,
                +6.995966000187407197283e-17L, -6.964349343411584120055e-18L,
                +7.268789359189778223225e-19L, -7.885125241947769024019e-20L,
                +8.689022564130615225208e-21L, -9.353211304381231554634e-22L
                +9.218280404899298404756e-23L
            };
            const long double midpoint = (7.25L + 5.5L)/2.0L;
            const long double halflen = (7.25L - 5.5L)/2.0L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        else
        {
            const int nterms = 50;
            long double term[nterms + 1];

            const long double epsilon = LDBL_EPSILON/2.0L; 
            const long double xsq = x*x;
            const long double twox = 2.0L*x;
            const long double twoxsq = 2.0L*xsq;

            long double xn = twoxsq;
            long double factorial = 1.0L;
            int n = 2;

            term[0] = 1.0L;
            term[1] = 1.0L/xn;
            for (; n <= nterms; ++n)
            {
                xn *= twoxsq;
                factorial *= (2*n - 1);
                term[n] = factorial/xn;
                if (term[n] < epsilon)
                    break;
            }   
            if (n > nterms)
                n = nterms;

            for (; n >= 0; --n)
                result += term[n];
            result /= twox;
        }

        if (x_in < 0.0L)
            result *= -1.0L;
        return result;
    }

    long double inverseExpsqIntegral(const long double x_in)
    {
        const long double smallx = sqrtl(LDBL_EPSILON);
        const long double x = fabsl(x_in);   
        long double result = 0.0L;

        // At the very beginning, the forward function behaves as x + x^3/3
        if (x < smallx)
            result = x;
        // Are we below the integral upper limit of 1/2?
        else if (x < 0.5449871041836222236624201L)
        {
            static const long double coeffs[] = {
                0.2579094020165510878448047L,      0.2509616007789004064162848L, 
                -0.008037441097819308338583395L,  -0.0009645198158737501684728918L, 
                0.0001298963476188684667851777L,   2.83303393956705648285913e-6L, 
                -1.879031091504004970753414e-6L,   8.89033414496187588540693e-8L, 
                2.190598580039940838615595e-8L,   -2.958785324082878640818902e-9L, 
                -1.393055660359914508583854e-10L,  5.934164278634234871603216e-11L, 
                -2.038318546964313720058418e-12L, -8.718325681035840557959458e-13L, 
                1.013108738230624177815487e-13L,   7.80188419892238096141015e-15L, 
                -2.410959710954351931245264e-15L,  4.28768887009677018630627e-17L, 
                4.075320326600366577580808e-17L,  -3.925657400094226595458262e-18L, 
                -4.52032825617882087470387e-19L,   1.08172986581534678569357e-19L, 
                7.619258918621063777867205e-23L,  -2.043372106982501109979208e-21L, 
                1.577535083736277024466253e-22L,   2.659861973971217145144534e-23L
            };
            static const unsigned deg = sizeof(coeffs)/sizeof(coeffs[0]) - 1U;
            const long double midpoint = 0.2724935520918111118312101L;
            const long double halflen = midpoint;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        // Are we below the integral upper limit of 1?
        else if (x < 1.462651745907181608804049L)
        {
            static const long double coeffs[] = {
                0.7736025700148903044157852L,      0.2483094370727645101600738L, 
                -0.0236047661681907541554642L,     0.001718250905865626423590199L, 
                -3.383723505479159579142638e-6L,  -0.00002706856684823251983606182L, 
                5.564585822987557167324481e-6L,   -6.297400054716145999859568e-7L, 
                1.793804726508794465230363e-8L,    9.974525492565834910508807e-9L, 
                -2.630092849889704961303573e-9L,   3.584362156719671871930982e-10L, 
                -1.848479157680947543606944e-11L, -4.505786185834443101923114e-12L, 
                1.497497786049082262992397e-12L,  -2.345976552921671025035597e-13L, 
                1.675613965321671764579391e-14L,   2.080526674977816958132577e-15L, 
                -9.222206647061674196609463e-16L,  1.635927767802667303380293e-16L, 
                -1.458481834781130100146786e-17L, -8.477734754986502993701378e-19L, 
                5.883853143077372799032853e-19L,  -1.178516243865358739330946e-19L, 
                1.244597734746846080964198e-20L,   1.834734630829857873717171e-22L, 
                -3.79697806729613468087694e-22L,   8.640525246432963515541941e-23L, 
                -1.072975125060893395507772e-23L
            };
            static const unsigned deg = sizeof(coeffs)/sizeof(coeffs[0]) - 1U;
            const long double midpoint = 1.003819425045401916233234L;
            const long double halflen = 0.4588323208617796925708142L;

            result = chebyshevSeriesSum(coeffs, deg, (x-midpoint)/halflen);
        }
        // Are we below the integral upper limit of 3/2?
        else if (x < 4.063114058624186262070998L)
        {
            static const long double coeffs[] = {
                1.237474264255216149826922L,       0.2515376732927722040966062L, 
                0.01255809348578646846677752L,    -0.001555984193633101089113648L, 
                -0.00003266175488449684464535135L, 0.00001861132693718087179337154L, 
                3.038788024730725430182981e-7L,   -3.055587383693358717339231e-7L, 
                2.195404333387651282823557e-10L,   5.223891523975639259623118e-9L, 
                -8.74172529522376286448331e-11L,  -9.289565568740836123320397e-11L, 
                3.037929091913979095346417e-12L,   1.697081673433425594636218e-12L, 
                -8.376226831184981721677363e-14L, -3.144033465184008496066638e-14L, 
                2.107731553034119316083898e-15L,   5.865893869124205701237769e-16L, 
                -5.061884931584969465772662e-17L, -1.096017981587409945712935e-17L, 
                1.182058217010476282761648e-18L,   2.040660311333339268713157e-19L, 
                -2.708723954709022960299832e-20L, -3.767175977793052434129249e-21L, 
                6.118697880934383791686931e-22L,   6.976999259292224769625502e-23L
            };
            static const unsigned deg = sizeof(coeffs)/sizeof(coeffs[0]) - 1U;
            const long double midpoint = 0.9003422806239746400989134L;
            const long double halflen = 0.2836972833794905219117775L;

            result = chebyshevSeriesSum(coeffs, deg, (sqrtl(logl(x))-midpoint)/halflen);
        }
        // Are we below the integral upper limit of 2?
        else if (x < 16.4526277655072302247364L)
        {
            static const long double coeffs[] = {
                1.750318675095801084701066L,       0.2503081949080952607456335L, 
                -0.0003683988977021577213047602L, -0.0003053217430231520892093692L, 
                0.00004984978059867808464521542L, -2.910067173452788393366537e-6L, 
                -1.234138038866370318487004e-7L,   3.698515524218906567339791e-8L, 
                -2.598716127513287147666407e-9L,  -8.028249755820916232073154e-11L, 
                3.38719730193993051252875e-11L,   -2.805688393204576054583932e-12L, 
                -4.636644559366711367183408e-14L,  3.430651608591744447051587e-14L, 
                -3.234144260654980846019301e-15L, -1.479571272220063709849795e-17L, 
                3.663824718035758023592578e-17L,  -3.866514289799821609911763e-18L, 
                2.280975526810673800594954e-20L,   4.039168081650367776987e-20L, 
                -4.73516370177886271900388e-21L,   7.25430770939280219423293e-23L, 
                4.526173229162084391018431e-23L
            };
            static const unsigned deg = sizeof(coeffs)/sizeof(coeffs[0]) - 1U;
            const long double midpoint = 1.428752297062238501405865L;
            const long double halflen = 0.2447127330587733393951737L;

            result = chebyshevSeriesSum(coeffs, deg, (sqrtl(logl(x))-midpoint)/halflen);
        }
        // Are we below the integral upper limit of 3?
        else if (x < 1444.545122892714154713760L)
        {
            static const long double coeffs[] = {
                2.50294099696623892772079L,         0.4994357077985400736034567L, 
                -0.00291622179390211582491955L,     0.0005756988194441929049790467L, 
                -0.00002810720142885454120222505L, -0.00001091749276776510487675747L, 
                3.29093808250468757948933e-6L,     -4.888188874032568897843031e-7L, 
                4.172102909667302951211426e-8L,    -4.599575541801073701575834e-10L, 
                -6.053857271985015243693571e-10L,   1.509946372242848844836431e-10L, 
                -2.455367299247951048634247e-11L,   2.672752200212154657233337e-12L, 
                -9.103858678297231810743428e-14L,  -3.718269817594061105825935e-14L, 
                1.069438018054794309965648e-14L,   -1.744407969349348513403047e-15L, 
                1.883808777693390076608131e-16L,   -7.095219918870126016632479e-18L, 
                -2.561647683564104671111204e-18L,   7.953272251856515822155603e-19L, 
                -1.363875890657281736925731e-19L,   1.519215969460881098898382e-20L, 
                -6.014662801208487581697383e-22L,  -2.017882982607174277058989e-22L, 
                6.264772567011851344575421e-23L
            };
            static const unsigned deg = sizeof(coeffs)/sizeof(coeffs[0]) - 1U;
            const long double midpoint = 2.185393865913887927395372L;
            const long double halflen = 0.5119288357928760865943334L;

            result = chebyshevSeriesSum(coeffs, deg, (sqrtl(logl(x))-midpoint)/halflen);
        }
        else
        {
            // At this point, sqrtl(logl(x)) looks more or less like
            // a straight line. We can do Newton-Raphson efficiently
            // in this transformed variable.
            const long double target = sqrtl(logl(x));
            long double xtry = target, xold = 0.0L;
            while (fabsl((xtry - xold)/xtry) > 2.0*LDBL_EPSILON)
            {
                xold = xtry;
                const long double daws = dawsonIntegral(xtry);
                const long double sqrval = sqrtl(xtry*xtry + logl(daws));
                const long double deriv = 0.5L/sqrval/daws;
                xtry += (target - sqrval)/deriv;
            }
            result = xtry;
        }

        if (x_in < 0.0L)
            result *= -1.0L;
        return result;
    }

    long double normalDensityDerivative(const unsigned order,
                                        const long double x)
    {
        static std::vector<std::vector<long long> > coeffs;
        const unsigned ncoeffs = coeffs.size();
        if (order >= ncoeffs)
            coeffs.resize(order + 1);
        unsigned nc = coeffs[order].size();
        if (nc == 0U)
        {
            buildGaussDeriCoeffs(order, &coeffs[order]);
            nc = coeffs[order].size();
        }
        assert(nc == order + 1U);
        return polySeriesSum(&coeffs[order][0], order, x)*
            expl(-x*x/2.0L)/SQRTTWOPIL;
    }

    double bivariateNormalIntegral(const double rho,
                                   const double x, const double y)
    {
        const double absrho = fabs(rho);
        if (absrho > 1.0) throw std::domain_error(
            "In npstat::bivariateNormalIntegral: "
            "correlation coefficient is outside of the [-1, 1] interval");
        if (absrho < 0.925)
            return Phi2(-x, -y, rho);
        else
            return bvnd(x, y, rho);
    }
}
