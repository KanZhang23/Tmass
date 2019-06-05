#include <cmath>
#include <cfloat>
#include <stdexcept>

#include "npstat/nm/MathUtils.hh"
#include "npstat/nm/SpecialFunctions.hh"

static double curoot(double x)
/* 
     find cube root of x.

     30 Jan 1989   Don Herbison-Evans 

     called by cubic . 
*/
{
   double value;
   double absx;
   int neg;

   neg = 0;
   absx = x;
   if (x < 0.0)
   {
      absx = -x;
      neg = 1;
   }
   if (absx != 0.0) value = exp( log(absx)/3.0 );
      else value = 0.0;
   if (neg == 1) value = -value;
   return(value);
} /* curoot */

namespace npstat {
    unsigned solveQuadratic(const double b, const double c,
                            double *x1, double *x2)
    {
        const double d = b*b - 4.0*c;

        if (d < 0.0)
            return 0U;

        if (d == 0.0)
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
        return 2U;
    }

    double ndUnitSphereVolume(const unsigned dim)
    {
        switch (dim)
        {
        case 0U:
            return 1.0;

        case 1U:
            return 2.0;

        case 2U:
            return M_PI;

        case 3U:
            return 4.0/3.0*M_PI;

        default:
            return pow(M_PI, dim/2.0)/Gamma(dim/2.0 + 1.0);
        }
    }

    double ndUnitSphereArea(const unsigned dim)
    {
        return dim*ndUnitSphereVolume(dim);
    }

    unsigned solveCubic(double p, double q, double r, double v3[3])
/* 
   find the real roots of the cubic - 
       x**3 + p*x**2 + q*x + r = 0 

     12 Dec 2003 initialising n,m,po3
     12 Dec 2003 allow return of 3 zero roots if p=q=r=0
      2 Dec 2003 negating j if p>0
      1 Dec 2003 changing v from (sinsqk > doub0) to (sinsqk >= doub0)
      1 Dec 2003 test changing v from po3sq+po3sq to doub2*po3sq
     16 Jul 1981 Don Herbison-Evans

   input parameters - 
     p,q,r - coeffs of cubic equation. 

   output- 
     the number of real roots
     v3 - the roots. 

   global constants -
     rt3 - sqrt(3) 
     inv3 - 1/3 
     doubmax - square root of largest number held by machine 

     method - 
     see D.E. Littlewood, "A University Algebra" pp.173 - 6 

     15 Nov 2003 output 3 real roots: Don Herbison-Evans
        Apr 1981 initial version: Charles Prineas

     called by  cubictest,quartic,chris,yacfraid,neumark,descartes,ferrari.
     calls      quadratic,acos3,curoot,cubnewton. 
*/
    {
        const double doub0 = 0.0;
        const double doubmax = sqrt(0.5*DBL_MAX);
        const double rt3 = sqrt(3.0);

        int    n3 = 0;
        double po3 = 0,po3sq = 0,qo3 = 0;
        double uo3 = 0,u2o3 = 0,uo3sq4 = 0,uo3cu4 = 0;
        double v = 0,vsq = 0,wsq = 0;
        double m1 = 0,m2 = 0,mcube = 0;
        double muo3 = 0,s = 0,scube = 0,t = 0,cosk = 0,rt3sink = 0,sinsqk = 0;

        if (r == doub0)
        {
            n3 = solveQuadratic(p, q, &v3[0], &v3[1]);
            v3[n3++] = doub0;
            goto done;
        }
        if ((p == doub0) && (q == doub0))
        {
            v3[0] = curoot(-r);
            v3[1] = v3[0];
            v3[2] = v3[0];
            n3 = 3;
            goto done;
        }
        n3 = 1;
        if ((p > doubmax) || (p <  -doubmax))
        {
            v3[0] = -p;
            goto done;
        }
        if ((q > doubmax) || (q <  -doubmax))
        {
            if (q > doub0)
            {
                v3[0] =  -r/q;
                goto done;
            }
            else
                if (q < doub0)
                {
                    v3[0] = -sqrt(-q);
                    goto done; 
                }
                else
                {
                    v3[0] = doub0;
                    goto done;
                }
        }
        else
            if ((r > doubmax)|| (r < -doubmax))
            {
                v3[0] =  -curoot(r);
                goto done;
            }
            else
            {
                po3 = p/3.0;
                po3sq = po3*po3;
                if (po3sq > doubmax)
                {
                    v3[0] = -p;
                    goto done;
                }
                else
                {
                    v = r + po3*(po3sq+po3sq - q);
                    if ((v > doubmax) || (v < -doubmax))
                    {
                        v3[0] = -p;
                        goto done;
                    }
                    else
                    {
                        vsq = v*v;
                        qo3 = q/3.0;
                        uo3 = qo3 - po3sq;
                        u2o3 = uo3 + uo3;
                        if ((u2o3 > doubmax) || (u2o3 < -doubmax))
                        {
                            if (p == doub0)
                            {
                                if (q > doub0)
                                {
                                    v3[0] =  -r/q;
                                    goto done;
                                }
                                else
                                    if (q < doub0)
                                    {
                                        v3[0] =  -sqrt(-q);
                                        goto done;
                                    }
                                    else
                                    {
                                        v3[0] = doub0;
                                        goto done;
                                    }
                            }
                            else
                            {
                                v3[0] = -q/p;
                                goto done;
                            }
                        }
                        uo3sq4 = u2o3*u2o3;
                        if (uo3sq4 > doubmax)
                        {
                            if (p == doub0)
                            {
                                if (q > doub0)
                                {
                                    v3[0] = -r/q;
                                    goto done;
                                }
                                else
                                    if (q < doub0)
                                    {
                                        v3[0] = -sqrt(-q);
                                        goto done;
                                    }
                                    else
                                    {
                                        v3[0] = doub0;
                                        goto done;
                                    }
                            }
                            else
                            {
                                v3[0] = -q/p;
                                goto done;
                            }
                        }
                        uo3cu4 = uo3sq4*uo3;
                        wsq = uo3cu4 + vsq;
                        if (wsq >= doub0)
                        {
/* 
   cubic has one real root -
*/
                            if (v <= doub0)
                            {
                                mcube = ( -v + sqrt(wsq))/2.0;
                            }
                            else
                            {
                                mcube = ( -v - sqrt(wsq))/2.0;
                            }
                            m1 = curoot(mcube);
                            if (m1 != doub0)
                            {
                                m2 = -uo3/m1;
                            }
                            else
                            {
                                m2 = doub0;
                            }
                            v3[0] = m1 + m2 - po3;

                            if (wsq == doub0)
                            {
                                v3[1] = v3[0];
                                v3[2] = v3[0];
                                n3 = 3;
                            }
                        }
                        else
                        {
/* 
   cubic has three real roots -
*/
                            if (uo3 < doub0)
                            {
                                muo3 = -uo3;
                                if (muo3 > doub0)
                                {
                                    s = sqrt(muo3);
                                    if (p > doub0)
                                    {
                                        s = -s;
                                    }
                                }
                                else
                                {
                                    s = doub0;
                                }
                                scube = s*muo3;
                                if (scube == doub0)
                                {
                                    v3[0] = m1 + m2 - po3;
                                    n3 = 1;
                                }
                                else
                                {
                                    t =  -v/(scube+scube);
                                    cosk = cos(acos(t)/3.0);
                                    v3[0] = (s+s)*cosk - po3;
                                    n3 = 1 ;
                                    sinsqk = 1.0 - cosk*cosk;
                                    if (sinsqk >= doub0)
                                    {
                                        rt3sink = rt3*sqrt(sinsqk);
                                        v3[1] = s*(-cosk + rt3sink) - po3;
                                        v3[2] = s*(-cosk - rt3sink) - po3;
                                        n3 = 3;
                                    }
                                }
                            }
                            else
/* 
   cubic has multiple root -  
*/
                            {
                                v3[0] = curoot(v) - po3;
                                v3[1] = v3[0];
                                v3[2] = v3[0];
                                n3 = 3;
                            }
                        }
                    }
                }
            }
    done:
        return(n3) ;
    }

    bool parabolicExtremum(const double x1_in, const double y1_in,
                           const double x2_in, const double y2_in,
                           const double x3_in, const double y3_in,
                           double* extremumCoordinate, double* extremumValue)
    {
        assert(extremumCoordinate);
        assert(extremumValue);

        const double x1 = x1_in - x2_in;
        const double x3 = x3_in - x2_in;
        const double y1 = y1_in - y2_in;
        const double y3 = y3_in - y2_in;

        const double denom = x1*(x3 - x1)*x3;
        if (denom == 0.0) throw std::invalid_argument(
            "In npstat::parabolicExtremum: some of the x values coincide");

        const double a = (x1*y3 - x3*y1)/denom;
        if (a == 0.0) throw std::invalid_argument(
            "In npstat::parabolicExtremum: points fall on a straight line");
        const double b = (x3*x3*y1 - x1*x1*y3)/denom;
        const double x = -b/2.0/a;

        *extremumCoordinate = x + x2_in;
        *extremumValue = x*b/2.0 + y2_in;

        return a > 0.0;
    }
}
