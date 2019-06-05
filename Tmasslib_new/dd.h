/* Quadruple precision arithmetic. This code is a partial 
 * port to C of the LBNL multiprecision package "qd".
 * See http://www.nersc.gov/~dhb/mpdist/mpdist.html.
 */

#ifndef DD_H_
#define DD_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __sgi
#define dd_inline static
#else
#define dd_inline static inline
#endif

#define DD_FLTPREC       0x000
#define DD_INVALIDPREC   0x100
#define DD_DBLPREC       0x200
#define DD_LDBLPREC      0x300

typedef struct _dd_t {
    double hi;
    double lo;
} dd_t;

/* Computes (a+b) and err(a+b).  Assumes |a| >= |b|. */
dd_inline double quick_two_sum(double a, double b, double *err)
{
    double s = a + b;
    *err = b - (s - a);
    return s;
}

/* Computes (a-b) and err(a-b).  Assumes |a| >= |b| */
dd_inline double quick_two_diff(double a, double b, double *err)
{
    double s = a - b;
    *err = (a - s) - b;
    return s;
}

dd_inline double two_sum(double a, double b, double *err)
{
    double s = a + b;
    double bb = s - a;
    *err = (a - (s - bb)) + (b - bb);
    return s;
}

dd_inline double two_diff(double a, double b, double *err)
{
    double s = a - b;
    double bb = s - a;
    *err = (a - (s - bb)) - (b + bb);
    return s;
}

dd_inline void split(double a, double *hi, double *lo)
{
    double temp;
    temp = 134217729.0 * a;
    *hi = temp - (temp - a);
    *lo = a - *hi;
}

dd_inline double two_prod(double a, double b, double *err)
{
    double a_hi, a_lo, b_hi, b_lo;
    double p = a * b;
    split(a, &a_hi, &a_lo);
    split(b, &b_hi, &b_lo);
    *err = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
    return p;
}

dd_inline double two_square(double a, double *err)
{
    double hi, lo;
    double q = a * a;
    split(a, &hi, &lo);
    *err = ((hi * hi - q) + 2.0 * hi * lo) + lo * lo;
    return q;
}

dd_inline dd_t dd_real(double hi, double lo) 
{
    dd_t a;
    a.hi = hi;
    a.lo = lo;
    return a;
}

dd_inline dd_t dd_sum(dd_t a, dd_t b)
{
    double s1, s2, t1, t2;
    s1 = two_sum(a.hi, b.hi, &s2);
    t1 = two_sum(a.lo, b.lo, &t2);
    s2 += t1;
    s1 = quick_two_sum(s1, s2, &s2);
    s2 += t2;
    s1 = quick_two_sum(s1, s2, &s2);
    return dd_real(s1, s2);
}

dd_inline dd_t dd_plus_d(dd_t a, double b)
{
    double s1, s2;
    s1 = two_sum(a.hi, b, &s2);
    s2 += a.lo;
    s1 = quick_two_sum(s1, s2, &s2);
    return dd_real(s1, s2);
}

dd_inline dd_t dd_minus_d(dd_t a, double b)
{
    double s1, s2;
    s1 = two_diff(a.hi, b, &s2);
    s2 += a.lo;
    s1 = quick_two_sum(s1, s2, &s2);
    return dd_real(s1, s2);
}

dd_inline dd_t dd_diff(dd_t a, dd_t b)
{
    double s1, s2, t1, t2;
    s1 = two_diff(a.hi, b.hi, &s2);
    t1 = two_diff(a.lo, b.lo, &t2);
    s2 += t1;
    s1 = quick_two_sum(s1, s2, &s2);
    s2 += t2;
    s1 = quick_two_sum(s1, s2, &s2);
    return dd_real(s1, s2);
}

dd_inline dd_t dd_prod(dd_t a, dd_t b)
{
    double p1, p2;
    p1 = two_prod(a.hi, b.hi, &p2);
    p2 += a.hi * b.lo;
    p2 += a.lo * b.hi;
    p1 = quick_two_sum(p1, p2, &p2);
    return dd_real(p1, p2);
}

dd_inline dd_t dd_times_d(dd_t a, double b)
{
    double p1, p2;
    p1 = two_prod(a.hi, b, &p2);
    p2 += (a.lo * b);
    p1 = quick_two_sum(p1, p2, &p2);
    return dd_real(p1, p2);
}

dd_inline dd_t dd_div(dd_t a, dd_t b)
{
    double q1, q2, q3;
    dd_t r;
    q1 = a.hi / b.hi;
    r = dd_diff(a, dd_times_d(b, q1));
    q2 = r.hi / b.hi;
    r = dd_diff(r, dd_times_d(b, q2));
    q3 = r.hi / b.hi;
    q1 = quick_two_sum(q1, q2, &q2);
    return dd_plus_d(dd_real(q1, q2), q3);
}

dd_inline dd_t dd_square(dd_t a)
{
    double p1, p2;
    double s1, s2;
    p1 = two_square(a.hi, &p2);
    p2 += 2.0 * a.hi * a.lo;
    p2 += a.lo * a.lo;
    s1 = quick_two_sum(p1, p2, &s2);
    return dd_real(s1, s2);
}

#ifdef __i386
/*
 * Return the precision macro representing the current
 * floating point precision.
 */
static int dd_getprecision(void)
{
    unsigned short cw;
    __asm__ __volatile__("fnstcw %0":"=m" (cw));
    return cw & DD_LDBLPREC;
}
/*
 * Set the precision to prec if it is a valid
 * floating point precision macro. Returns 1
 * if precision set, 0 otherwise.
 */
static int dd_setprecision(int prec)
{
    unsigned short cw;
    __asm__ __volatile__("fnstcw %0":"=m" (cw));
    if (!(prec & ~DD_LDBLPREC) && (prec != DD_INVALIDPREC)) {
	cw = (cw & ~DD_LDBLPREC) | (prec & DD_LDBLPREC);
	__asm__ __volatile__("fldcw %0"::"m" (cw));
	return 1;
    }
    return 0;
}
#else
static int dd_getprecision(void) {return DD_INVALIDPREC;}
static int dd_setprecision(int prec) {return 0;}
#endif /* __i386 */

#ifdef __cplusplus
}
#endif

#endif /* DD_H_ */
