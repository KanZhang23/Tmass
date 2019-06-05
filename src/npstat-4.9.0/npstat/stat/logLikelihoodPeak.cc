#include <cassert>
#include <stdexcept>

#include "npstat/nm/MathUtils.hh"
#include "npstat/stat/logLikelihoodPeak.hh"

static double find_crossing(const double x1, const double y1,
                            const double x2, const double y2,
                            const double level)
{
    const double dy = y2 - y1;
    assert(dy != 0.0);
    return x1 + (level - y1)/dy*(x2 - x1);
}

namespace npstat {
    LikelihoodSummary logLikelihoodPeak(const double* array,
                                        const unsigned npoints,
                                        const double down,
                                        const double leftCoordinate,
                                        const double rightCoordinate)
    {
        if (npoints < 3U) throw std::invalid_argument(
            "In npstat::logLikelihoodPeak: insufficient data length");
        if (down <= 0.0) throw std::invalid_argument(
            "In npstat::logLikelihoodPeak: \"down\" argument must be positive");
        assert(array);

        const double step = (rightCoordinate-leftCoordinate)/(npoints-1);
        const unsigned last = npoints - 1;

        // Find the position of the maximum
        double maxval = array[0];
        unsigned maxind = 0;

        for (unsigned i=1; i<npoints; ++i)
            if (array[i] > maxval)
            {
                maxval = array[i];
                maxind = i;
            }

        LikelihoodPoint peak;
        double peakVal = maxval;
        if (maxind == 0)
            peak = LikelihoodPoint(leftCoordinate,
                                   LikelihoodPoint::ON_THE_EDGE);
        else if (maxind == last)
            peak = LikelihoodPoint(rightCoordinate,
                                   LikelihoodPoint::ON_THE_EDGE);
        else
        {
            // Fit a parabola to the peak and the points around it
            const double f0 = array[maxind-1U];
            const double f1 = array[maxind];
            const double f2 = array[maxind+1U];

            const double deri1 = (f2 - f0)/2.0;
            const double deri2 = (f2 - f1) + (f0 - f1);
            double dn = -deri1/deri2;
            if (dn < -0.5)
                dn = -0.5;
            else if (dn > 0.5)
                dn = 0.5;

            peakVal = f1 + (deri1 + deri2*(dn/2.0))*dn;
            peak = LikelihoodPoint(leftCoordinate + (maxind + dn)*step,
                                   LikelihoodPoint::OK);
        }

        // Find the point on the left
        const double errorLevel = peakVal - down;
        LikelihoodPoint left;
        if (maxind == 0)
            left = LikelihoodPoint(leftCoordinate,
                                   LikelihoodPoint::NOT_FOUND);
        else if (array[0] == errorLevel)
        {
            left = LikelihoodPoint(leftCoordinate,
                                   LikelihoodPoint::ON_THE_EDGE);
        }
        else if (array[0] > errorLevel)
        {
            left = LikelihoodPoint(leftCoordinate,
                                   LikelihoodPoint::NOT_FOUND);
        }
        else 
        {
            unsigned leftCrossing;
            for (leftCrossing = 1; array[leftCrossing] < errorLevel &&
                                   leftCrossing < maxind;
                 ++leftCrossing);

            if (leftCrossing == maxind && maxind != last)
            {
                // Use parabolic model
                const double f0 = array[maxind-1U];
                const double f1 = array[maxind];
                const double f2 = array[maxind+1U];

                const double a = ((f2 - f1) + (f0 - f1))/2.0;
                const double b = (f2 - f0)/2.0;
                const double c = f1 - errorLevel;

                assert(a < 0.0);
                double x1, x2;
                if (solveQuadratic(b/a, c/a, &x1, &x2) != 2U)
                    throw std::runtime_error("In npstat::logLikelihoodPeak: "
                                "unexpected number of solutions (location 1)");
                const double xsm = x1 < x2 ? x1 : x2;
                left = LikelihoodPoint(leftCoordinate + (maxind + xsm)*step,
                                       LikelihoodPoint::OK);
            }
            else
            {
                // Use linear model
                const double x = leftCoordinate + leftCrossing*step;
                const double xm1 = x - step;
                const double coord = find_crossing(
                    xm1, array[leftCrossing-1],
                    x, array[leftCrossing], errorLevel);
                left = LikelihoodPoint(coord, LikelihoodPoint::OK);
            }
        }

        // Find the point on the right
        LikelihoodPoint right;
        if (maxind == last)
            right = LikelihoodPoint(rightCoordinate,
                                    LikelihoodPoint::NOT_FOUND);
        else if (array[last] == errorLevel)
            right = LikelihoodPoint(rightCoordinate,
                                    LikelihoodPoint::ON_THE_EDGE);
        else if (array[last] > errorLevel)
            right = LikelihoodPoint(rightCoordinate,
                                    LikelihoodPoint::NOT_FOUND);
        else
        {
            unsigned rightCrossing;
            for (rightCrossing=last-1; array[rightCrossing] < errorLevel &&
                                       rightCrossing > maxind;
                 --rightCrossing);

            if (rightCrossing == maxind && maxind != 0)
            {
                // Use parabolic model
                const double f0 = array[maxind-1U];
                const double f1 = array[maxind];
                const double f2 = array[maxind+1U];

                const double a = ((f2 - f1) + (f0 - f1))/2.0;
                const double b = (f2 - f0)/2.0;
                const double c = f1 - errorLevel;

                assert(a < 0.0);
                double x1, x2;
                if (solveQuadratic(b/a, c/a, &x1, &x2) != 2U)
                    throw std::runtime_error("In npstat::logLikelihoodPeak: "
                                "unexpected number of solutions (location 2)");
                const double xsm = x1 > x2 ? x1 : x2;
                right = LikelihoodPoint(leftCoordinate + (maxind + xsm)*step,
                                        LikelihoodPoint::OK);
            }
            else
            {
                // Use linear model
                const double x = leftCoordinate + (rightCrossing+1U)*step;
                const double xm1 = x - step;
                const double coord = find_crossing(
                    xm1, array[rightCrossing],
                    x, array[rightCrossing+1], errorLevel);
                right = LikelihoodPoint(coord, LikelihoodPoint::OK);
            }
        }

        return LikelihoodSummary(peak, left, right, down);
    }
}
