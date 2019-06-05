#ifndef IMPORTANCESAMPLINGDISTRO_HH_
#define IMPORTANCESAMPLINGDISTRO_HH_

#include <cmath>
#include <cassert>

#include "geners/CPP11_auto_ptr.hh"
#include "npstat/stat/Distributions1D.hh"

// Build the importance sampling density for subsequent efficient
// sampling of parton Pt. It is assumed that the argument functor
// calculates the density in jet Pt (observed space) while its argument
// is parton Pt (phase space) -- that is, the functor knows the jet Pt
// internally.
//
// It is expected that this distro will be used along the following lines:
//
// CPP11_auto_ptr<Tabulated1D> distro = importanceSamplingDistro(...);
// double logPt = distro->quantile(rnd);
// double partonPt = exp(logPt);
// double samplingDensityInPt = distro->density(logPt)/partonPt;
//
// If "rnd" are numbers randomly distributed on [0, 1) then the
// resulting parton Pt values will be distributed according to
// the sampling density calculated in this manner.
//
// Function arguments are as follows:
//
// densityCalculator -- Density functor (e.g., product of transfer
//                      function and efficiency).
//
// partonPtPrior     -- Prior for the parton Pt distribution
//
// minPartonPt       -- Minimum parton Pt to scan (something like
//                      1/3 of jet Pt)
//
// maxPartonPt       -- Maximum kinematically allowed parton Pt.
//                      Can be safely set to the beam energy, but lower
//                      values might result in more efficient code.
//
// samplingFactor    -- Multiplicative step with which the space between
//                      minPartonPt and maxPartonPt will be sampled.
//                      Should be slightly larger than 1.
//
// discardFraction   -- Fraction of scanned distribution to discard at
//                      the beginning and at the end of the scanned
//                      parton Pt interval when the importance sampling
//                      density is constructed. Larger fraction leads
//                      to reduced coverage but more efficient code.
template<class Funct>
CPP11_auto_ptr<const npstat::Tabulated1D> importanceSamplingDistro(
    const Funct densityCalculator,
    const npstat::AbsDistribution1D& partonPtPrior,
    const double minPartonPt, const double maxPartonPt,
    const double samplingFactor=1.001, const double discardFraction=1.0e-6)
{
    assert(minPartonPt > 0.0);
    assert(maxPartonPt > minPartonPt);
    assert(samplingFactor > 1.0 && samplingFactor <= 2.0);
    assert(discardFraction >= 0.0 && discardFraction < 1.0);

    const double logmin = log(minPartonPt);
    const double logmax = log(maxPartonPt);
    const unsigned nIntervals = floor((logmax - logmin)/
                                      log(samplingFactor) + 1.0);
    const double logstep = (logmax - logmin)/nIntervals;
    const unsigned tableLen = nIntervals + 1U;

    std::vector<double> scan;
    scan.reserve(tableLen);
    long double sum = 0.0L;
    for (unsigned i=0; i<tableLen; ++i)
    {
        const double logpt = logmin + i*logstep;
        const double partonPt = exp(logpt);
        const double dens = partonPt*densityCalculator(partonPt)*
                            partonPtPrior.density(partonPt);
        scan.push_back(dens);
        sum += dens;
    }
    assert(sum >= 0.0L);

    if (sum == 0.0L)
    {
        // This is potentially possible due to buggy JetUser
        return CPP11_auto_ptr<const npstat::Tabulated1D>();
    }

    const long double fraclim = discardFraction/2.0;

    unsigned ifirst = 0;
    long double isum = 0.0L;
    for (; ifirst<tableLen; ++ifirst)
    {
        isum += scan[ifirst];
        if (isum/sum > fraclim)
            break;
    }

    unsigned ilast = nIntervals;
    isum = 0.0L;
    for (; ilast; --ilast)
    {
        isum += scan[ilast];
        if (isum/sum > fraclim)
            break;
    }

    assert(ifirst < ilast);

    npstat::Tabulated1D* p = new npstat::Tabulated1D(
        logmin + ifirst*logstep, (ilast - ifirst)*logstep,
        &scan[ifirst], ilast - ifirst + 1U, 1U);
    return CPP11_auto_ptr<const npstat::Tabulated1D>(p);
}

#endif // IMPORTANCESAMPLINGDISTRO_HH_
