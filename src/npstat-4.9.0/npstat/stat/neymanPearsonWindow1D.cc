#include <cmath>
#include <cfloat>
#include <cassert>
#include <algorithm>

#include "npstat/stat/neymanPearsonWindow1D.hh"


namespace npstat {
static bool adjustStep1(const AbsDistribution1D& s,
                        const AbsDistribution1D& bg,
                        const double x0, double* step)
{
    assert(step);
    double factor = 1.0;
    bool status = false;

    for (unsigned i=0; i<1000; ++i, factor /= 2.0)
    {
        const double x = x0 + *step*factor;
        if (x == x0)
            break;
        if (bg.density(x) > 0.0)
        {
            status = true;
            break;
        }
        if (s.density(x) > 0.0)
        {
            status = true;
            break;
        }
    }
    *step *= factor;
    return status;
}


// Return "false" if density ratio is undefined
static bool limitedDensityRatio(const AbsDistribution1D& s,
                                const AbsDistribution1D& bg,
                                const double x, double* ratio)
{
    assert(ratio);

    const double bgDens = bg.density(x);
    const double sDens = s.density(x);
    if (bgDens <= 0.0 && sDens <= 0.0)
    {
        *ratio = 0.0;
        return false;
    }
    if (bgDens <= 0.0)
        *ratio = DBL_MAX;
    else
        *ratio = sDens/bgDens;
    return true;
}


static void searchRatioInRange(const AbsDistribution1D& s,
                               const AbsDistribution1D& bg,
                               const double threshold,
                               const double searchStartCoordinate,
                               const double limit,
                               const double initialStep,
                               double* bound, NeymanPearson::Status* status)
{
    assert(initialStep);

    // Make steps; increase step size until one of the following
    // things happen:
    //
    // 1) Coordinate gets beyond "limit"
    // 2) S/B ratio becomes indeterminate
    // 3) S/B ratio becomes less than threshold
    //
    double x1 = 0.0, v1 = 0.0;
    bool haveValue1 = true, atTheLimit = false;

    for (double stepFactor = 1.0; haveValue1 && !atTheLimit; stepFactor *= 2.0)
    {
        x1 = searchStartCoordinate + stepFactor*initialStep;
        if ((initialStep > 0.0 && x1 >= limit) ||
            (initialStep < 0.0 && x1 <= limit))
        {
            atTheLimit = true;
            x1 = limit;
        }
        const double sd = s.density(x1);
        const double bgd = bg.density(x1);
        if (bgd > 0.0)
        {
            haveValue1 = true;
            v1 = sd/bgd;
        }
        else if (sd > 0.0)
        {
            haveValue1 = true;
            v1 = DBL_MAX;
        }
        else
        {
            haveValue1 = false;
            v1 = 0.0;
        }
        if (haveValue1 && v1 < threshold)
            break;
    }

    // Return if we are at the limit and still have high value of S/B
    if (haveValue1 && v1 >= threshold)
    {
        assert(atTheLimit);
        *status = NeymanPearson::SUPPORT_BOUNDARY;
        *bound = limit;
        return;
    }

    // Proceed with interval divisions
    double x0 = searchStartCoordinate;
    const double eps = 2.0*DBL_EPSILON;
    while (fabs(x1 - x0)/(fabs(x1) + fabs(x0) + DBL_EPSILON) > eps)
    {
        const double xtry = (x1 + x0)/2.0;
        const double sd = s.density(xtry);
        const double bgd = bg.density(xtry);
        if (bgd <= 0.0)
        {
            if (sd > 0.0)
                x0 = xtry;
            else
            {
                x1 = xtry;
                v1 = 0.0;
                haveValue1 = false;
                atTheLimit = false;
            }
        }
        else
        {
            const double rtry = sd/bgd;
            if (rtry > threshold)
                x0 = xtry;
            else
            {
                x1 = xtry;
                v1 = rtry;
                haveValue1 = true;
                atTheLimit = false;
            }
        }
    }

    if (atTheLimit)
    {
        *status = NeymanPearson::SUPPORT_BOUNDARY;
        *bound = limit;
    }
    else if (haveValue1)
    {
        assert(v1 <= threshold);
        *status = NeymanPearson::OK;
        *bound = (x1 + x0)/2.0;
    }
    else
    {
        *status = NeymanPearson::INDETERMINATE;
        *bound = x1;
    }
}
}


namespace npstat {
    int neymanPearsonWindow1D(const AbsDistribution1D& s,
                              const AbsDistribution1D& bg,
                              const double searchStartCoordinate,
                              const double initialStepSize,
                              const double threshold,
                              double* leftBound,
                              NeymanPearson::Status* leftBoundStatus,
                              double* rightBound,
                              NeymanPearson::Status* rightBoundStatus)
    {
        assert(leftBound);
        assert(leftBoundStatus);
        assert(rightBound);
        assert(rightBoundStatus);

        *leftBound = 0.0;
        *leftBoundStatus = NeymanPearson::INVALID;
        *rightBound = 0.0;
        *rightBoundStatus = NeymanPearson::INVALID;

        if (threshold <= 0.0 || threshold >= DBL_MAX)
            return 1;     // Bad threshold

        const double xmin = std::max(s.quantile(0.0), bg.quantile(0.0));
        const double xmax = std::min(s.quantile(1.0), bg.quantile(1.0));
        if (xmin >= xmax)
            return 2;     // Incompatible supports of the densities

        if (searchStartCoordinate <= xmin || searchStartCoordinate >= xmax)
            return 3;     // Bad starting coordinate for the search

        const double sDens = s.density(searchStartCoordinate);
        if (sDens <= 0.0)
            return 4;     // Signal density at the starting point is 0

        const double bgDens = bg.density(searchStartCoordinate);
        if (bgDens > 0.0)
            if (sDens/bgDens <= threshold)
                return 5; // Starting point is not inside high S/B region

        double rstep = std::abs(initialStepSize);
        if (searchStartCoordinate + rstep >= xmax || rstep == 0.0)
            rstep = (xmax - searchStartCoordinate)/4.0;

        double lstep = -std::abs(initialStepSize);
        if (searchStartCoordinate + lstep <= xmin || lstep == 0.0)
            lstep = (xmin - searchStartCoordinate)/4.0;

        searchRatioInRange(s, bg, threshold, searchStartCoordinate,
                           xmin, lstep, leftBound, leftBoundStatus);
        searchRatioInRange(s, bg, threshold, searchStartCoordinate,
                           xmax, rstep, rightBound, rightBoundStatus);
        return 0;
    }


    int signalToBgMaximum1D(const AbsDistribution1D& s,
                            const AbsDistribution1D& bg,
                            const double searchStartCoordinate,
                            const double initialStepSize,
                            double* maximumPosition,
                            double* maximumSignalToBgRatio,
                            NeymanPearson::Status* searchStatus)
    {
        assert(maximumPosition);
        assert(maximumSignalToBgRatio);
        assert(searchStatus);

        *maximumPosition = 0.0;
        *maximumSignalToBgRatio = 0.0;
        *searchStatus = NeymanPearson::INVALID;

        const double xmin = std::max(s.quantile(0.0), bg.quantile(0.0));
        const double xmax = std::min(s.quantile(1.0), bg.quantile(1.0));
        if (xmin >= xmax)
            return 2;     // Incompatible supports of the densities

        if (searchStartCoordinate <= xmin || searchStartCoordinate >= xmax)
            return 3;     // Bad starting coordinate for the search

        if (bg.density(searchStartCoordinate) <= 0.0)
        {
            if (s.density(searchStartCoordinate) <= 0.0)
                return 4; // Indeterminate S/B ratio at the starting point
            else
            {
                *maximumPosition = searchStartCoordinate;
                *maximumSignalToBgRatio = DBL_MAX;
                *searchStatus = NeymanPearson::OK;
                return 0;
            }
        }

        double rstep = std::abs(initialStepSize);
        if (searchStartCoordinate + rstep >= xmax || rstep == 0.0)
            rstep = (xmax - searchStartCoordinate)/4.0;

        double lstep = -std::abs(initialStepSize);
        if (searchStartCoordinate + lstep <= xmin || lstep == 0.0)
            lstep = (xmin - searchStartCoordinate)/4.0;

        // Find such values of lstep and rstep that S/B is not
        // indeterminate
        if (!adjustStep1(s, bg, searchStartCoordinate, &lstep) ||
            !adjustStep1(s, bg, searchStartCoordinate, &rstep))
        {
            *searchStatus = NeymanPearson::INDETERMINATE;
            return 5;     // Can not determine useful step size
        }

        double xleft = searchStartCoordinate + lstep;
        double xcenter = searchStartCoordinate;
        double xright = searchStartCoordinate + rstep;
        double vleft = 0.0, vright = 0.0, vcenter = 0.0;
        if (!limitedDensityRatio(s, bg, xleft, &vleft) ||
            !limitedDensityRatio(s, bg, xcenter, &vcenter) ||
            !limitedDensityRatio(s, bg, xright, &vright))
        {
            *searchStatus = NeymanPearson::INDETERMINATE;
            return 0;
        }
        if (vleft == DBL_MAX)
            *maximumPosition = xleft;
        if (vright == DBL_MAX)
            *maximumPosition = xright;
        if (vleft == DBL_MAX || vright == DBL_MAX)
        {
            *maximumSignalToBgRatio = DBL_MAX;
            *searchStatus = NeymanPearson::OK;
            return 0;
        }

        // Move to the left or right until we bound the maximum
        bool bounded = false;
        for (unsigned i=0; i<2000; ++i)
        {
            if (vcenter > vleft && vcenter > vright)
            {
                bounded = true;
                break;
            }
            if (vcenter <= vleft && vcenter <= vright)
            {
                *maximumPosition = xcenter;
                *maximumSignalToBgRatio = vcenter;
                return 6; // We are either at the minimum or on a plateau
            }

            if (vcenter > vleft)
            {
                // Move to the right
                xleft = xcenter;
                vleft = vcenter;
                xcenter = xright;
                vcenter = vright;
                if (xcenter >= xmax)
                {
                    *maximumPosition = xcenter;
                    *maximumSignalToBgRatio = vcenter;
                    *searchStatus = NeymanPearson::SUPPORT_BOUNDARY;
                    return 0;
                }
                lstep = (xleft - xcenter)/2.0;
                rstep *= 2.0;
                if (xcenter + rstep >= xmax)
                {
                    rstep = (xmax - xcenter)/2.0;
                    const double tmp = xcenter + rstep;
                    if (tmp == xcenter)
                    {
                        *maximumPosition = xcenter;
                        *maximumSignalToBgRatio = vcenter;
                        *searchStatus = NeymanPearson::SUPPORT_BOUNDARY;
                        return 0;
                    }
                }
                if (!adjustStep1(s, bg, xcenter, &rstep))
                {
                    *maximumPosition = xcenter;
                    *maximumSignalToBgRatio = vcenter;
                    *searchStatus = NeymanPearson::INDETERMINATE;
                    return 5;     // Can not determine useful step size
                }
                xright = xcenter + rstep;
                if (!limitedDensityRatio(s, bg, xright, &vright))
                {
                    *maximumPosition = xcenter;
                    *maximumSignalToBgRatio = vcenter;
                    *searchStatus = NeymanPearson::INDETERMINATE;
                    return 0;
                }
                if (vright == DBL_MAX)
                    *maximumPosition = xright;
            }
            else
            {
                // Move to the left
                xright = xcenter;
                vright = vcenter;
                xcenter = xleft;
                vcenter = vleft;
                if (xcenter <= xmin)
                {
                    *maximumPosition = xcenter;
                    *maximumSignalToBgRatio = vcenter;
                    *searchStatus = NeymanPearson::SUPPORT_BOUNDARY;
                    return 0;
                }
                rstep = (xright - xcenter)/2.0;
                lstep *= 2.0;
                if (xcenter + lstep <= xmin)
                {
                    lstep = (xmin - xcenter)/2.0;
                    const double tmp = xcenter + lstep;
                    if (tmp == xcenter)
                    {
                        *maximumPosition = xcenter;
                        *maximumSignalToBgRatio = vcenter;
                        *searchStatus = NeymanPearson::SUPPORT_BOUNDARY;
                        return 0;
                    }
                }
                if (!adjustStep1(s, bg, xcenter, &lstep))
                {
                    *maximumPosition = xcenter;
                    *maximumSignalToBgRatio = vcenter;
                    *searchStatus = NeymanPearson::INDETERMINATE;
                    return 5;     // Can not determine useful step size
                }
                xleft = xcenter + lstep;
                if (!limitedDensityRatio(s, bg, xleft, &vleft))
                {
                    *maximumPosition = xcenter;
                    *maximumSignalToBgRatio = vcenter;
                    *searchStatus = NeymanPearson::INDETERMINATE;
                    return 0;
                }
                if (vleft == DBL_MAX)
                    *maximumPosition = xleft;
            }
            if (vleft == DBL_MAX || vright == DBL_MAX)
            {
                *maximumSignalToBgRatio = DBL_MAX;
                *searchStatus = NeymanPearson::OK;
                return 0;
            }
        }

        if (!bounded)
            return 7;    // Caught in an internal cycle?

        const double eps = sqrt(DBL_EPSILON);
        while (fabs(xright - xleft)/(fabs(xleft) + fabs(xright) + eps) > eps)
        {
            double xtry = 0.0, vtry = 0.0;
            const bool splitRight = fabs(xright - xcenter) > fabs(xleft - xcenter);
            if (splitRight)
                xtry = (xright + xcenter)/2.0;
            else
                xtry = (xleft + xcenter)/2.0;
            if (!limitedDensityRatio(s, bg, xtry, &vtry))
            {
                *maximumPosition = xcenter;
                *maximumSignalToBgRatio = vcenter;
                *searchStatus = NeymanPearson::INDETERMINATE;
                return 0;
            }
            if (vtry == DBL_MAX)
            {
                *maximumPosition = xtry;
                *maximumSignalToBgRatio = DBL_MAX;
                *searchStatus = NeymanPearson::OK;
                return 0;
            }

            const bool adjustCenter = vtry > vcenter;
            if (splitRight)
            {
                if (adjustCenter)
                {
                    xleft = xcenter;
                    vleft = vcenter;
                }
                else
                {
                    xright = xtry;
                    vright = vtry;
                }
            }
            else
            {
                if (adjustCenter)
                {
                    xright = xcenter;
                    vright = vcenter;
                }
                else
                {
                    xleft = xtry;
                    vleft = vtry;
                }
            }
            if (adjustCenter)
            {
                xcenter = xtry;
                vcenter = vtry;
            }
        }

        *maximumPosition = xcenter;
        *maximumSignalToBgRatio = vcenter;
        *searchStatus = NeymanPearson::OK;
        return 0;
    }
}
