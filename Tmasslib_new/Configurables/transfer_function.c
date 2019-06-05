#include <assert.h>
#include <math.h>

#include "transfer_function.h"
#include "tfs_2015.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline double deltaPhiShort(const double phi1,
                                   const double phi2)
{
    double delta = phi1 - phi2;
    if (delta > M_PI)
        delta -= 2.0*M_PI;
    else if (delta < -M_PI)
        delta += 2.0*M_PI;
    return delta;
}

double transfer_function(const particle_obj solved_jet,
                         const jet_info* jseen,
                         const double jes, const int isB)
{
    const double partonPt = sqrt(solved_jet.p.x*solved_jet.p.x +
                                 solved_jet.p.y*solved_jet.p.y);
    const double partonEta = asinh(solved_jet.p.z/partonPt);
    const double partonPhi = atan2(solved_jet.p.y, solved_jet.p.x);

    const double jetEta = asinh(jseen->pz/jseen->pt);
    const double jetPhi = atan2(jseen->py, jseen->px);

    const double deta = jetEta - partonEta;
    const double dphi = deltaPhiShort(jetPhi, partonPhi);
    const double dR = sqrt(deta*deta + dphi*dphi);

    const double deltaJES = (jes - 1.0)/jseen->syserr;
    const double jes_at_cut = 1.0 + deltaJES*jseen->cuterr;
    const double xcut = get_jet_pt_cutoff()*jes_at_cut/partonPt;

    /* 
     * Predictor and argument order must be consistent with
     * the code which builds the transfer functions
     */
    double predictors[2], x[2];
    predictors[0] = solved_jet.m;
    predictors[1] = partonPt;
    x[0] = dR;
    x[1] = jseen->pt*jes/partonPt;

    assert(partonPt > 0.0);
    assert(jseen->syserr > 0.0);
    assert(jseen->cuterr > 0.0);

    return bare_transfer_function(jseen->etaDet, isB,
                                  predictors, sizeof(predictors)/sizeof(predictors[0]),
                                  x, sizeof(x)/sizeof(x[0]), xcut)/partonPt;
}

double transfer_function_efficiency_2(const particle_obj solved_jet,
                                      const double etaJet,
                                      const int isB, const double ptCut)
{
    const double partonPt = sqrt(solved_jet.p.x*solved_jet.p.x +
                                 solved_jet.p.y*solved_jet.p.y);
    double eff = 0.0;
    if (partonPt > 1.0)
    {
        const double ptRat = ptCut/partonPt;
        const double xCut = get_jet_pt_cutoff()/partonPt;

        double predictors[2];
        predictors[0] = solved_jet.m;
        predictors[1] = partonPt;

        eff = bare_ptratio_exceedance(etaJet, isB, 
                                      predictors,
                                      sizeof(predictors)/sizeof(predictors[0]),
                                      ptRat, xCut);
    }
    return eff;
}

#ifdef __cplusplus
}
#endif
