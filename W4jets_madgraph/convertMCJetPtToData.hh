#ifndef CONVERTMCJETPTTODATA_HH_
#define CONVERTMCJETPTTODATA_HH_

#include <cfloat>
#include <cassert>

//
// Solve the equation Pt,MC = JES(Pt,data)*Pt,data for Pt,data
//
template<class ErrCalc>
double convertMCJetPtToData(const double mcPt, const double deltaJes,
                            const ErrCalc sigmaCalc)
{
    if (deltaJes == 0.0)
        return mcPt;

    double ptmin = mcPt;
    double ptmax = mcPt;

    if (deltaJes > 0.0)
    {
        // JES is > 1 so that Pt,data < Pt,MC
        ptmin *= 0.9;
        double jes = 1.0 + deltaJes*sigmaCalc(ptmin);
        while (ptmin*jes >= mcPt)
        {
            ptmin *= 0.9;
            jes = 1.0 + deltaJes*sigmaCalc(ptmin);
        }
    }
    else
    {
        // Pt,data > Pt,MC
        ptmax *= 1.1;
        double jes = 1.0 + deltaJes*sigmaCalc(ptmax);
        while (ptmax*jes <= mcPt)
        {
            ptmax *= 1.1;
            jes = 1.0 + deltaJes*sigmaCalc(ptmax);
        }
    }

    double fmin = ptmin*(1.0 + deltaJes*sigmaCalc(ptmin));
    assert(fmin <= mcPt);

    double fmax = ptmax*(1.0 + deltaJes*sigmaCalc(ptmax));
    assert(fmax >= mcPt);

    for (unsigned i=0; i<1000U; ++i)
    {
        const double ptry = (ptmax + ptmin)/2.0;
        if ((ptmax - ptmin)/ptmax < 8.0*DBL_EPSILON)
            return ptry;
        const double ftry = ptry*(1.0 + deltaJes*sigmaCalc(ptry));
        if (ftry == mcPt)
            return ptry;
        if (ftry < mcPt)
        {
            ptmin = ptry;
            fmin = ftry;
        }
        else
        {
            ptmax = ptry;
            fmax = ftry;
        }
    }
    return (ptmax + ptmin)/2.0;
}

#endif // CONVERTMCJETPTTODATA_HH_
