#include <cmath>
#include <cfloat>
#include <vector>

#include "npstat/nm/goldenSectionSearch.hh"
#include "npstat/nm/DualAxis.hh"

#define calculate_memoized(x, fcn) do {\
     if (fvalues[x] == dbmax)          \
     {                                 \
         fcn = f(axis.coordinate(x));  \
         fvalues[x] = fcn;             \
     }                                 \
     else                              \
         fcn = fvalues[x];             \
} while(0);

namespace npstat {
    MinSearchStatus1D goldenSectionSearchOnAGrid(
        const Functor1<double, double>& f, const DualAxis& axis,
        const unsigned i0, const unsigned initialStep,
        unsigned* imin, double* fMinusOne, double* fmin, double* fPlusOne)
    {
        static const double sqrsqr2 = pow(2.0, 0.25);
        static const double onemg = 1.0 - (sqrt(5.0) - 1.0)/2.0;
        static const double dbmax = DBL_MAX;

        assert(imin);
        assert(fMinusOne);
        assert(fmin);
        assert(fPlusOne);

        const long nCoords = axis.nCoords();
        if (nCoords < 3) throw std::invalid_argument(
            "In npstat::goldenSectionSearchOnAGrid: "
            "less than three points in the input grid");

        *imin = nCoords;
        *fMinusOne = 0.0;
        *fmin = 0.0;
        *fPlusOne = 0.0;

        long xcenter = i0;
        if (xcenter <= 0)
            xcenter = 1;
        if (xcenter >= nCoords - 1)
            xcenter = nCoords - 2;

        const long lStep = initialStep ? initialStep : 1;

        long xleft = xcenter - lStep;
        if (xleft < 0)
            xleft = 0;

        long xright = xcenter + lStep;
        if (xright >= nCoords)
            xright = nCoords - 1;

        double fleft = f(axis.coordinate(xleft));
        double fcenter = f(axis.coordinate(xcenter));
        double fright = f(axis.coordinate(xright));

        // This code can be used with functions that are expensive
        // to evaluate. Because of that we will memoize all results.
        std::vector<double> fvaluesBuf(axis.nCoords(), dbmax);
        double* fvalues = &fvaluesBuf[0];
        fvalues[xleft] = fleft;
        fvalues[xcenter] = fcenter;
        fvalues[xright] = fright;

        // First, bracket the minimum
        {
            double rightStepFactor = 1.0;
            double leftStepFactor = 1.0;
            double fcheck = 0.0;

            while (fright < fcenter || fleft < fcenter)
            {
                if (fright <= fcenter && fleft <= fcenter)
                {
                    // No minimum (there is instead a maximum or
                    // the function is a constant)
                    return MIN_SEARCH_FAILED;
                }

                if (fright < fcenter)
                {
                    // Move to the right
                    xleft = xcenter;
                    fleft = fcenter;
                    xcenter = xright;
                    fcenter = fright;
                    const long delta = round((xcenter - xleft)*rightStepFactor);
                    rightStepFactor *= sqrsqr2;
                    xright = xcenter + delta;
                    if (xright >= nCoords)
                        xright = nCoords - 1;
                    if (xright <= xcenter)
                    {
                        // xcenter is at the right edge.
                        // check if we have a minimum there.
                        const long xcheck = nCoords - 2;
                        calculate_memoized(xcheck, fcheck);
                        if (fcheck > fcenter)
                        {
                            *imin = xcenter;
                            *fMinusOne = fcheck;
                            *fmin = fcenter;
                            *fPlusOne = fcenter;
                            return MIN_ON_RIGHT_EDGE;
                        }
                        else
                        {
                            xright = xcenter;
                            fright = fcenter;
                            xcenter = xcheck;
                            fcenter = fcheck;
                            if (xleft >= xcenter)
                            {
                                xleft = xcenter - 1;
                                calculate_memoized(xleft, fleft);
                                rightStepFactor = 1.0;
                                leftStepFactor = 1.0;
                            }
                        }
                    }
                    else
                        calculate_memoized(xright, fright);
                }
                else
                {
                    // Move to the left
                    xright = xcenter;
                    fright = fcenter;
                    xcenter = xleft;
                    fcenter = fleft;
                    const long delta = round((xright - xcenter)*leftStepFactor);
                    leftStepFactor *= sqrsqr2;
                    xleft = xcenter - delta;
                    if (xleft < 0)
                        xleft = 0;
                    if (xleft >= xcenter)
                    {
                        // xcenter is at the left edge.
                        // check if we have a minimum there.
                        const long xcheck = 1;
                        calculate_memoized(xcheck, fcheck);
                        if (fcheck > fcenter)
                        {
                            *imin = xcenter;
                            *fMinusOne = fcenter;
                            *fmin = fcenter;
                            *fPlusOne = fcheck;
                            return MIN_ON_LEFT_EDGE;
                        }
                        else
                        {
                            xleft = xcenter;
                            fleft = fcenter;
                            xcenter = xcheck;
                            fcenter = fcheck;
                            if (xright <= xcenter)
                            {
                                xright = xcenter + 1;
                                calculate_memoized(xright, fright);
                                rightStepFactor = 1.0;
                                leftStepFactor = 1.0;
                            }
                        }
                    }
                    else
                        calculate_memoized(xleft, fleft);
                }
            }
        }

        // Now, converge towards the minimum
        double ftry = 0.0;
        while (xright - xleft > 2)
        {
            if (xcenter - xleft < xright - xcenter)
            {
                // Right interval is larger. Split it.
                const long delta = round(onemg*(xright - xcenter));
                const long xtry = xcenter + delta;
                calculate_memoized(xtry, ftry);
                if (fcenter < ftry)
                {
                    xright = xtry;
                    fright = ftry;
                }
                else
                {
                    xleft = xcenter;
                    fleft = fcenter;
                    xcenter = xtry;
                    fcenter = ftry;
                }
            }
            else
            {
                // Split the left interval
                const long delta = round(onemg*(xcenter - xleft));
                const long xtry = xcenter - delta;
                calculate_memoized(xtry, ftry);
                if (fcenter < ftry)
                {
                    xleft = xtry;
                    fleft = ftry;
                }
                else
                {
                    xright = xcenter;
                    fright = fcenter;
                    xcenter = xtry;
                    fcenter = ftry;
                }
            }
        }

        *imin = xcenter;
        *fMinusOne = fleft;
        *fmin = fcenter;
        *fPlusOne = fright;

        return MIN_SEARCH_OK;
    }
}
