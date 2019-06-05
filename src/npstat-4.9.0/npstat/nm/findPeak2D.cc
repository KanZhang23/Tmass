#include <cassert>
#include <stdexcept>

#include "npstat/nm/findPeak2D.hh"

// Functions which make orthogonal polys on the 5x5 grid
static double p5x5_0(double, double)
{
    return 0.2;
}

static double p5x5_1(const double x, double)
{
    return x/7.07106781186547524;
}

static double p5x5_2(double, const double y)
{
    return y/7.07106781186547524;
}

static double p5x5_3(const double x, double)
{
    return (x*x - 2.0)/8.36660026534075548;
}

static double p5x5_4(const double x, const double y)
{
    return x*y/10.0;
}

static double p5x5_5(double, const double y)
{
    return (y*y - 2.0)/8.36660026534075548;
}

namespace npstat {
    void Peak2D::rescaleCoords(const double centerCellCoordinateInX,
                               const double gridCellWidthInX,
                               const double centerCellCoordinateInY,
                               const double gridCellWidthInY)
    {
        if (!(gridCellWidthInX > 0.0 && gridCellWidthInY > 0.0))
           throw std::invalid_argument(
              "In npstat::Peak2D::rescaleCoords: cell width must be positive");

        x *= gridCellWidthInX;
        x += centerCellCoordinateInX;
        y *= gridCellWidthInY;
        y += centerCellCoordinateInY;

        hessian[0] /= gridCellWidthInX*gridCellWidthInX;
        hessian[1] /= gridCellWidthInX*gridCellWidthInY;
        hessian[2] /= gridCellWidthInY*gridCellWidthInY;
    }

    bool findPeak3by3(const double g[3][3], Peak2D* peak)
    {
        assert(peak);

        const double xm1(g[0][0] + g[0][1] + g[0][2]);
        const double x0(g[1][0] + g[1][1] + g[1][2]);
        const double xp1(g[2][0] + g[2][1] + g[2][2]);
        const double ym1(g[0][0] + g[1][0] + g[2][0]);
        const double y0(g[0][1] + g[1][1] + g[2][1]);
        const double yp1(g[0][2] + g[1][2] + g[2][2]);

        const double cx((xp1 - xm1)/6);
        const double cy((yp1 - ym1)/6);
        const double cxy((g[0][0] - g[0][2] - g[2][0] + g[2][2])/4);
        const double cxsq((xm1 + xp1 - 2*x0)/3);
        const double cysq((ym1 + yp1 - 2*y0)/3);
        const double det(cxsq*cysq - cxy*cxy);

        if (det)
        {
            peak->x = (cxy*cy - cx*cysq)/det;
            peak->y = (cx*cxy - cxsq*cy)/det;
        }
        else
        {
            peak->x = 0.0;
            peak->y = 0.0;
        }
        peak->magnitude = 0.5*(cxsq*peak->x*peak->x + cysq*peak->y*peak->y) + 
            cxy*peak->x*peak->y + cx*peak->x + cy*peak->y + g[1][1];

        peak->hessian[0] = cxsq;
        peak->hessian[1] = cxy;
        peak->hessian[2] = cysq;

        peak->valid = cxsq < 0.0 && cysq < 0.0 && det > 0.0;
        peak->inside = -1.0 <= peak->x && peak->x < 1.0 &&
                       -1.0 <= peak->y && peak->y < 1.0;

        return peak->valid && peak->inside;
    }

    bool findPeak5by5(const double g[5][5], Peak2D* peak)
    {
        // Orthogonal polynomials on the 5x5 grid
        static const double poly[6][5][5] = {
            {
                {0.2,0.2,0.2,0.2,0.2},
                {0.2,0.2,0.2,0.2,0.2},
                {0.2,0.2,0.2,0.2,0.2},
                {0.2,0.2,0.2,0.2,0.2},
                {0.2,0.2,0.2,0.2,0.2}
            },
            {
                {-0.28284271247461901,-0.28284271247461901,-0.28284271247461901,
                 -0.28284271247461901,-0.28284271247461901},
                {-0.141421356237309505,-0.141421356237309505,-0.141421356237309505,
                 -0.141421356237309505,-0.141421356237309505},
                {0,0,0,0,0},
                {0.141421356237309505,0.141421356237309505,0.141421356237309505,
                 0.141421356237309505,0.141421356237309505},
                {0.28284271247461901,0.28284271247461901,0.28284271247461901,
                 0.28284271247461901,0.28284271247461901}
            },
            {
                {-0.28284271247461901,-0.141421356237309505,0,
                 0.141421356237309505,0.28284271247461901},
                {-0.28284271247461901,-0.141421356237309505,0,0.141421356237309505,
                 0.28284271247461901},{-0.28284271247461901,-0.141421356237309505,0,
                                       0.141421356237309505,0.28284271247461901},
                {-0.28284271247461901,-0.141421356237309505,0,0.141421356237309505,
                 0.28284271247461901},{-0.28284271247461901,-0.141421356237309505,0,
                                       0.141421356237309505,0.28284271247461901}
            },
            {
                {0.239045721866878728,0.239045721866878728,0.239045721866878728,
                 0.239045721866878728,0.239045721866878728},
                {-0.119522860933439364,-0.119522860933439364,-0.119522860933439364,
                 -0.119522860933439364,-0.119522860933439364},
                {-0.239045721866878728,-0.239045721866878728,-0.239045721866878728,
                 -0.239045721866878728,-0.239045721866878728},
                {-0.119522860933439364,-0.119522860933439364,-0.119522860933439364,
                 -0.119522860933439364,-0.119522860933439364},
                {0.239045721866878728,0.239045721866878728,0.239045721866878728,
                 0.239045721866878728,0.239045721866878728}
            },
            {
                {0.4,0.2,0,-0.2,-0.4},
                {0.2,0.1,0,-0.1,-0.2},
                {0,0,0,0,0},
                {-0.2,-0.1,0,0.1,0.2},
                {-0.4,-0.2,0,0.2,0.4}
            },
            {
                {0.239045721866878728,-0.119522860933439364,-0.239045721866878728,
                 -0.119522860933439364,0.239045721866878728},
                {0.239045721866878728,-0.119522860933439364,-0.239045721866878728,
                 -0.119522860933439364,0.239045721866878728},
                {0.239045721866878728,-0.119522860933439364,-0.239045721866878728,
                 -0.119522860933439364,0.239045721866878728},
                {0.239045721866878728,-0.119522860933439364,-0.239045721866878728,
                 -0.119522860933439364,0.239045721866878728},
                {0.239045721866878728,-0.119522860933439364,-0.239045721866878728,
                 -0.119522860933439364,0.239045721866878728}
            }
        };

        typedef double (*PolyFcn)(double, double);
        static const PolyFcn fcns[6] = {p5x5_0, p5x5_1, p5x5_2, p5x5_3, p5x5_4, p5x5_5};

        assert(peak);

        // Calculate the expansion coefficients
        double c[6] = {0, 0, 0, 0, 0, 0};
        for (unsigned ic=0; ic<6; ++ic)
            for (unsigned i=0; i<5; ++i)
                for (unsigned j=0; j<5; ++j)
                    c[ic] += poly[ic][i][j]*g[i][j];

        // The location of the peak
        const double det = 40.0*c[3]*c[5] - 7.0*c[4]*c[4];
        if (det)
        {
            peak->x = (9.89949493661166534*c[2]*c[4] - 
                       23.6643191323984642*c[1]*c[5])/det;
            peak->y = (9.89949493661166534*c[1]*c[4] -
                       23.6643191323984642*c[2]*c[3])/det;
        }
        else
        {
            peak->x = 0.0;
            peak->y = 0.0;
        }

        // The value at the peak
        peak->magnitude = 0.0;
        for (unsigned ic=0; ic<6; ++ic)
            peak->magnitude += c[ic]*fcns[ic](peak->x, peak->y);

        // The hessian
        peak->hessian[0] = c[3]/4.18330013267037774;
        peak->hessian[1] = c[4]/10.0;
        peak->hessian[2] = c[5]/4.18330013267037774;

        // Check that the Hessian is negative-definite
        const bool goodhess = peak->hessian[0]*peak->hessian[2] - 
                              peak->hessian[1]*peak->hessian[1] > 0.0;

        peak->valid = c[3] < 0.0 && c[5] < 0.0 && det && goodhess;
        peak->inside = -2.0 <= peak->x && peak->x < 2.0 &&
                       -2.0 <= peak->y && peak->y < 2.0;

        return peak->valid && peak->inside;
    }
}
