#include <cassert>
#include <cmath>
#include <functional>
#include <stdexcept>

#include "npstat/nm/bilinearSection.hh"
#include "npstat/nm/Interval.hh"
#include "npstat/nm/MathUtils.hh"

static double findUseableR(const double a, const double b,
                           const double c, const bool linear)
{
    double r;
    if (linear || a == 0.0)
    {
        if (b == 0.0)
        {
            assert(c == 0.0);
            r = 1.0;
        }
        else
            r = -c/b;
    }
    else
    {
        double r1, r2;
        const unsigned nr = npstat::solveQuadratic(b/a, c/a, &r1, &r2);
        if (nr != 2U)
            throw std::runtime_error("In findUseableR (npstat static function):"
                                     " unexpected number of equation solutions");
        assert(r1 >= 0.0 || r2 >= 0.0);
        if (r1 < 0.0)
            r = r2;
        else if (r2 < 0.0)
            r = r1;
        else
            r = std::min(r1, r2);
    }
    return r;
}

namespace npstat {
    unsigned bilinearSection(const double z00, const double z10,
                             const double z11, const double z01,
                             const double level,
                             const unsigned nPointsToSample,
                             std::vector<std::pair<double,double> >* section1,
                             std::vector<std::pair<double,double> >* section2)
    {
        assert(section1);
        assert(section2);
        section1->clear();
        section2->clear();

        if (z00 > level && z10 > level && z11 > level && z01 > level)
            return 0;
        if (z00 < level && z10 < level && z11 < level && z01 < level)
            return 0;
        if (!nPointsToSample)
            return 0;

        // Find which sides cross the level
        const bool bottom_cross = Interval<double>(
            z00,z10,true).isInsideLower(level);
        const bool left_cross   = Interval<double>(
            z00,z01,true).isInsideLower(level);
        const bool top_cross    = Interval<double>(
            z01,z11,true).isInsideLower(level);
        const bool right_cross  = Interval<double>(
            z10,z11,true).isInsideLower(level);

        unsigned nCross = 0;
        if (bottom_cross) ++nCross;
        if (left_cross) ++nCross;
        if (top_cross) ++nCross;
        if (right_cross) ++nCross;

        if (nCross < 2)
        {
            // nCross == 1 is possible in case the level coincides
            // with some vertex. It is not useful because there is
            // no curve inside.
            return 0;
        }

        section1->reserve(nPointsToSample);
        if (nPointsToSample == 1U)
        {
            // Can only produce one point. Put it at the center,
            // assuming that this is some kind of a rough estimate
            // done for each pixel.
            section1->push_back(std::pair<double,double>(0.5, 0.5));
            return 1;
        }

        // Vertex codes for scanning the angles
        int vertexCodes[2];
        unsigned nVertexCodes = 0;

        const double s1 = z00 - z01 - z10 + z11;
        switch (nCross)
        {
        case 2U:
        {
            const double s2 = z10 - z00;
            const double s3 = z01 - z00;
            const double step = 1.0/(nPointsToSample - 1U);

            if (left_cross && right_cross)
            {
                // Can do y dependence on x
                for (unsigned i=0; i<nPointsToSample; ++i)
                {
                    const double x = step*i;
                    const double y = (level - x*s2 - z00)/(x*s1 + s3);
                    section1->push_back(std::pair<double,double>(x, y));
                }
                return 1;
            }

            if (top_cross && bottom_cross)
            {
                // Can do x dependence on y
                for (unsigned i=0; i<nPointsToSample; ++i)
                {
                    const double y = step*i;
                    const double x = (level - y*s3 - z00)/(y*s1 + s2);
                    section1->push_back(std::pair<double,double>(x, y));
                }
                return 1;
            }

            // Find a vertex used as a center for angle scan
            if (bottom_cross && left_cross)
                vertexCodes[nVertexCodes++] = 0;
            else if (left_cross && top_cross)
                vertexCodes[nVertexCodes++] = 1;
            else if (top_cross && right_cross)
                vertexCodes[nVertexCodes++] = 11;
            else if (right_cross && bottom_cross)
                vertexCodes[nVertexCodes++] = 10;
            else
                // This should never happen
                assert(0);
        }
        break;

        case 3U:
        {
            // Is this possible at all? Could not
            // trigger this branch in any of the
            // Monte Carlo tests of this code.
            assert(!"This case is not handled yet");
        }
        break;

        case 4U:
        {
            // Saddle-like surface. We need to understand
            // which sides are going to get connected to
            // each other.
            section2->reserve(nPointsToSample);

            const double valueAtTheSaddle = s1 ? (z00*z11 - z01*z10)/s1 :
                (z00 + z11 + z01 + z10)/4.0;
            std::pair<double,int> pts[4];
            pts[0] = std::pair<double,int>(z00,0);
            pts[1] = std::pair<double,int>(z10,10);
            pts[2] = std::pair<double,int>(z11,11);
            pts[3] = std::pair<double,int>(z01,1);
            if (level < valueAtTheSaddle)
            {
                // The sides adjacent to the two lowest points are connected.
                // Figure out which points are the lowest.
                std::sort(pts, pts+4);
            }
            else
            {
                // The sides adjacent to the two highest points are connected.
                std::sort(pts, pts+4,
                          std::greater<std::pair<double,unsigned> >());
            }
            vertexCodes[nVertexCodes++] = pts[0].second;
            vertexCodes[nVertexCodes++] = pts[1].second;
        }
        break;

        default:
            assert(0);
        }

        // Do the angular scanning
        assert(nVertexCodes < 3U);
        for (unsigned ipt=0; ipt<nVertexCodes; ++ipt)
        {
            std::vector<std::pair<double,double> >* sect = 
                ipt ? section2 : section1;

            double s2, s3, phistart, phiend, xc, yc, c;

            switch (vertexCodes[ipt])
            {
            case 0:
                // Lower left
                xc = 0.0;
                yc = 0.0;
                phistart = 0.0;
                phiend = M_PI/2.0;
                c = z00 - level;
                s2 = z10 - z00;
                s3 = z01 - z00;
                break;

            case 10:
                // Lower right
                xc = 1.0;
                yc = 0.0;
                phistart = M_PI/2.0;
                phiend = M_PI;
                c = z10 - level;
                s2 = z10 - z00;
                s3 = z11 - z10;
                break;

            case 11:
                // Upper right
                xc = 1.0;
                yc = 1.0;
                phistart = -M_PI/2.0;
                phiend = -M_PI;
                c = z11 - level;
                s2 = z11 - z01;
                s3 = z11 - z10;
                break;

            case 1:
                // Upper left
                xc = 0.0;
                yc = 1.0;
                phistart = 0.0;
                phiend = -M_PI/2.0;
                c = z01 - level;
                s2 = z11 - z01;
                s3 = z01 - z00;
                break;

            default:
                assert(0);
            }

            const double step = (phiend-phistart)/(nPointsToSample-1U);
            for (unsigned i=0; i<nPointsToSample; ++i)
            {
                const double phi = phistart + i*step;
                const double cosphi = cos(phi);
                const double sinphi = sin(phi);
                const double a = s1*cosphi*sinphi;
                const double b = s2*cosphi + s3*sinphi;
                const double r = findUseableR(
                    a, b, c, i == 0 || i == nPointsToSample - 1U);
                sect->push_back(std::pair<double,double>(
                                    xc + r*cosphi, yc + r*sinphi));
            }
        }

        return nVertexCodes;
    }
}
