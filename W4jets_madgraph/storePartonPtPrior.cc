#include <string>
#include <sstream>
#include <cassert>
#include <cmath>

#include "storePartonPtPrior.hh"

#include "histoscope.h"

void storePartonPtPrior(const npstat::AbsDistribution1D& prior,
                        const unsigned nSteps,
                        const double logPtMin, const double logPtMax,
                        const unsigned evcount, const double jetPt,
                        const unsigned ijet)
{
    static const unsigned maxNJets = 4;

    assert(ijet < maxNJets);
    const double logStep = (logPtMax - logPtMin)/nSteps;

    std::ostringstream os;
    os << "Event " << evcount << ", jet " << ijet
       << ", log(Jet Pt) = " << log(jetPt);
    const std::string& title = os.str();

    const int id = hs_create_1d_hist(evcount*maxNJets+ijet, title.c_str(),
                                     "Importance Samplers", "log(Pt)",
                                     "Density", nSteps+1U,
                                     logPtMin-logStep/2.0,
                                     logPtMax+logStep/2.0);
    for (unsigned i=0; i<=nSteps; ++i)
    {
        const double x = i < nSteps ? logPtMin + i*logStep : logPtMax;
        hs_1d_hist_set_bin(id, i, prior.density(x));
    }
}
