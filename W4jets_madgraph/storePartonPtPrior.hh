#ifndef STOREPARTONPTPRIOR_HH_
#define STOREPARTONPTPRIOR_HH_

#include "npstat/stat/AbsDistribution1D.hh"

void storePartonPtPrior(const npstat::AbsDistribution1D& prior,
                        unsigned nsteps, double logPtMin, double logPtMax,
                        unsigned eventNumber, double jetPt, unsigned jetNumber);

#endif // STOREPARTONPTPRIOR_HH_
