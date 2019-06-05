#include <cmath>
#include <cassert>
#include <algorithm>
#include <iostream>

#include "integrateW4Jets.hh"

#include "w4jetsKinematics.hh"
#include "matrixElementWeight.hh"
#include "importanceSamplingDistro.hh"
#include "JetPtTFDensity.hh"
#include "RandomNumberSequence.hh"
#include "misc_utils.hh"
#include "storePartonPtPrior.hh"

#include "npstat/stat/StatAccumulator.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/TruncatedDistribution1D.hh"
#include "npstat/stat/DistributionMix1D.hh"
#include "npstat/stat/HistoAxis.hh"
#include "npstat/nm/allocators.hh"

using namespace npstat;
using namespace rk;
using namespace geom3;
using namespace Pythia8;

void integrateW4Jets_m(Pythia* pythiaIn,
                     Parameters_sm_no_b_mass* parsIn,
                     const unsigned mode,
                     const unsigned reportInterval,
                     const bool allowTau,
                     const bool allowDirectDecay,
                     const double eBeam,
                     const double nominalWmass,
                     const double nominalWwidth,
                     const double maxWmassForScan,
                     const unsigned nWmassScanPoints,
                     const JetInfo jets[4],
                     const P4 lepton,
                     const int leptonCharge,
                     const bool isMuon,
                     const Vector3 centralPtV,
                     const double minPartonPtFactor,
                     BinnedJetPtFcn<JetPtTF>* tfs[4],
                     BinnedJetPtFcn<JetPtEff>* effs[4],
                     const double minDeltaJes,
                     const double maxDeltaJes,
                     const unsigned nDeltaJes,
                     const AbsDistribution1D& partonPtPrior,
                     const npstat::LocalPolyFilter1D& jetPriorFilter,
                     const npstat::HistoAxis& ax,
                     const AbsDistribution1D& tauTransferFunction,
                     const AbsDistributionND& systemPtPrior,
                     const InMemoryNtuple<double>& randomNumbersNt,
                     AbsConvergenceChecker& convergenceChecker,
                     unsigned powerOfTwoToStore,
                     JesIntegResult* result,
                     npstat::InMemoryNtuple<double>* functionValues,
                     const bool saveJetPriors)
{
    static unsigned evcount = 0;

    // JetUser is supposed to be fixed now
    const bool usingBuggyJetUser = false;

    // Some hardwired parameters
    const double minWmassForScan = 1.0;
    const double tauToMuBranching = 0.1741;
    const double tauToEBranching = 0.1783;
    const double wToEBranching = 0.1071;
    const double wToMuBranching = 0.1063;
    const double wToTauBranching = 0.1138;

    // Check the validity of certain assumptions made in the code
    assert(RSEQ_TOTAL_PT_Y == RSEQ_TOTAL_PT_X + 1);

    // Check the basic sanity of the input arguments
    assert(allowTau || allowDirectDecay);
    assert(nominalWwidth > 0.0 &&
           nominalWmass > nominalWwidth &&
           maxWmassForScan > nominalWmass &&
           eBeam > maxWmassForScan);
    assert(nWmassScanPoints > 1U);
    assert(lepton.m() == 0.0);
    assert(std::abs(leptonCharge) == 1);
    assert(minPartonPtFactor > 0.0 && minPartonPtFactor < 1.0);
    for (unsigned i=0; i<4; ++i)
    {
        assert(tfs[i]);
        assert(effs[i]);
        for (unsigned j=i+1; j<4; ++j)
        {
            assert(tfs[i] != tfs[j]);
            assert(effs[i] != effs[j]);
        }
    }
    assert(minDeltaJes < maxDeltaJes);
    assert(nDeltaJes);
    assert(systemPtPrior.dim() == 2U);

    // Reset the output values
    assert(result);
    result->clear();
    result->minDeltaJes = minDeltaJes;
    result->maxDeltaJes = maxDeltaJes;
    result->nDeltaJes = nDeltaJes;
    if (functionValues)
    {
        assert(functionValues->nColumns() == nDeltaJes);
        functionValues->clear();
    }

    // Note the starting wall clock time
    const double startingTime = time_in_seconds();

    // The axis for the JES scan
    const HistoAxis jesAxis(nDeltaJes, minDeltaJes, maxDeltaJes);

    // Initialize certain VECBOS common blocks
    //    initvb_(&nominalWmass, &nominalWwidth, &eBeam);

    // Collect jet Pt values and check that they are reasonable
    double jetPtValues[4];
    for (unsigned ijet=0; ijet<4; ++ijet)
    {
        jetPtValues[ijet] = jets[ijet].pt();
        assert(jetPtValues[ijet] > 0.0);
        assert(jetPtValues[ijet] < eBeam);
    }

    // Set the jet info for the TFs and the efficiencies
    for (unsigned ijet=0; ijet<4; ++ijet)
    {
        tfs[ijet]->setJetInfo(jets[ijet]);
        effs[ijet]->setJetInfo(jets[ijet]);
    }

    // Generate parton Pt importance samplers
    CPP11_auto_ptr<const Tabulated1D> ptSamplers[4];
    for (unsigned ijet=0; ijet<4; ++ijet)
    {
        DistributionMix1D mix;

        for (int ijes=-1; ijes<=(int)nDeltaJes; ++ijes)
        {
            const double deltaJes = jesAxis.binCenter(ijes);
            tfs[ijet]->setDeltaJES(deltaJes);
            effs[ijet]->setDeltaJES(deltaJes);

            CPP11_auto_ptr<const Tabulated1D> s = importanceSamplingDistro(
                JetPtEffDensity(tfs[ijet]->fcn(), effs[ijet]->fcn()),
                partonPtPrior, minPartonPtFactor*jetPtValues[ijet], eBeam);
            if (s.get())
                mix.add(*s, 1.0);
            else if (!usingBuggyJetUser)
                assert(!"We should not be here. This is a bug. Please report.");
        }
        assert(mix.nComponents());

        const unsigned nBins = ax.nBins();
        std::vector<double> dens(2*nBins);
        double* bins = &dens[nBins];
        for (unsigned i=0; i<nBins; ++i)
            bins[i] = mix.density(ax.binCenter(i));

        jetPriorFilter.filter(bins, nBins, &dens[0]);

        int minPtBin = ax.binNumber(mix.quantile(0.0));
        if (minPtBin < 0)
            minPtBin = 0;
        int maxPtBin = ax.binNumber(mix.quantile(1.0));
        if (maxPtBin >= (int)nBins)
            maxPtBin = nBins - 1;
        const double logPtMin = ax.binCenter(minPtBin);
        const double logPtMax = ax.binCenter(maxPtBin);
        const unsigned nSteps = maxPtBin + 1 - minPtBin;
        npstat::Tabulated1D* p = new npstat::Tabulated1D(
            logPtMin, logPtMax - logPtMin, &dens[minPtBin], nSteps, 1U);
        ptSamplers[ijet] = CPP11_auto_ptr<const Tabulated1D>(p);

        if (saveJetPriors)
            storePartonPtPrior(*p, nSteps, logPtMin, logPtMax,
                               evcount, jetPtValues[ijet], ijet);
    }

    // Number of variables in the ntuple and the buffer for them
    const unsigned nCols = randomNumbersNt.nColumns();
    assert(nCols >= N_RSEQ_VARS);
    std::vector<double> randomBufVec(nCols);
    double* randomNumbers = &randomBufVec[0];

    // W mass relativistic Breit-Wigner
    const Cauchy1D wPropag(nominalWmass*nominalWmass,
                           nominalWmass*nominalWwidth);

    // We will scan W mass squared from 1 GeV on the left
    // to the provided limit on the right
    const double wCoverage = wPropag.cdf(maxWmassForScan*maxWmassForScan) -
                             wPropag.cdf(minWmassForScan*minWmassForScan);

    // Prepare the standard set of W mass squared and corresponding
    // propagator values
    const TruncatedDistribution1D wMassScanner(wPropag,
                                               minWmassForScan*minWmassForScan,
                                               maxWmassForScan*maxWmassForScan);
    std::vector<double> wMassSqGrid(nWmassScanPoints);
    std::vector<double> wMassSqDensities(nWmassScanPoints);
    for (unsigned iwscan=0; iwscan<nWmassScanPoints; ++iwscan)
    {
        const double x = (iwscan + 0.5)/nWmassScanPoints;
        const double mwsq = wMassScanner.quantile(x);
        const double density = wPropag.density(mwsq);
        wMassSqGrid[iwscan] = mwsq;
        wMassSqDensities[iwscan] = density;
    }

    // Accumulator for the integration weights
    std::vector<StatAccumulator> accVec(nDeltaJes);

    // How many samples to sum before storing the next
    // entry in the history
    unsigned long nextSamplesToStore = floor(pow(2.0, powerOfTwoToStore) + 0.5);

    // Buffer for the parton 4-momenta
    P4 partons[4];

    // Couple other useful buffers
    std::vector<long double> pointWeightVec(nDeltaJes);
    std::vector<double> bufVec(nDeltaJes);

    // Integration cycle
    const unsigned long maxSamples = randomNumbersNt.nRows();
    for (unsigned long isample=0; isample<maxSamples; ++isample)
    {
        if (reportInterval && isample % reportInterval == 0)
        {
            std::cout << time_stamp() << " QMC point " << isample+1
                      << " of " << maxSamples << '\n';
            std::cout.flush();
        }

        // Overall weight for this phase space point
        {
            long double* ldbuf = &pointWeightVec[0];
            for (unsigned ijes=0; ijes<nDeltaJes; ++ijes)
                *ldbuf++ = 1.0L;
        }

        // Fetch the (pseudo- or quasi-) random numbers
        randomNumbersNt.rowContents(isample, randomNumbers, nCols);

        // Generate parton Pts and calculate parton 4-momenta (massless).
        // Do not multiply by weights due to jet transfer functions and
        // efficiencies here -- the weight could be 0 due to absence of
        // kinematic solutions, and then we would simply waste our time.
        double partonPt[4];
        P4 partonSum;
        for (unsigned ijet=0; ijet<4; ++ijet)
        {
            const double lnPt = ptSamplers[ijet]->quantile(randomNumbers[ijet]);
            const double pt = exp(lnPt);
            const double density = ptSamplers[ijet]->density(lnPt)/pt;
            assert(density > 0.0);
            if (mode & I_PRIOR_DENSITY_MASK)
                for (unsigned ijes=0; ijes<nDeltaJes; ++ijes)
                    pointWeightVec[ijes] /= density;
            const double scale = pt/jetPtValues[ijet];
            partonPt[ijet] = pt;
            partons[ijet] = P4(Vector3(scale*jets[ijet].p4().momentum()), 0.0);
            partonSum += partons[ijet];
        }

        if (mode & I_ME_MASK)
        {
            // Total Pt of the W+4 jets system
            double syspt[2];
            systemPtPrior.unitMap(&randomNumbers[RSEQ_TOTAL_PT_X], 2, syspt);
            const Vector3 totalPt(centralPtV.x() + syspt[0],
                                  centralPtV.y() + syspt[1], 0.0);

            // Figure out transverse momentum of the W. We need to subtract
            // the summed parton Pt from the total system Pt.
            const Vector3 wPt(totalPt - partonSum.transverse());

            // Figure out Pt of the neutrino as well
            const Vector3 neutrinoPt(wPt - lepton.transverse());

            // What is the minimum possible W mass squared assuming
            // that W directly decayed into l nu (rather than tau nu)?
            const double minMWSquared = minimumWMassSquared(neutrinoPt,
                                                            lepton.momentum());
            // Scan the W mass squared
            long double wScanWeight = 0.0L;
            bool firstPointAboveCutoff = true;
            for (unsigned iwscan=0; iwscan<nWmassScanPoints; ++iwscan)
            {
                if (allowDirectDecay)
                {
                    // Direct decay of W into l nu
                    double mwsq = wMassSqGrid[iwscan];
                    double branchWeight = isMuon ? wToMuBranching : wToEBranching;

                    if (mwsq > minMWSquared)
                    {
                        bool integrateOverNuPz = false;
                        if (firstPointAboveCutoff)
                        {
                            // This is the first time we see mass squared
                            // above the kinematic cutoff. We need to switch
                            // to integration over nu Pz (the jacobian in the
                            // denominator turns to 0 at the cutoff). Since
                            // we simply use rectangular integration in the
                            // W mass, we need to switch W mass squared to
                            // the value of mw^2 at the rectangle border.
                            //
                            // Remaining question: do we still want to do
                            // this in case iwscan == 0, i.e., the very
                            // first point in the scan is already above
                            // the cutoff? We probably do, but this should
                            // be thought out better.
                            //
                            mwsq = wMassScanner.quantile(
                                (iwscan + 1.0)/nWmassScanPoints);
                            firstPointAboveCutoff = false;
                            integrateOverNuPz = true;
                        }

                        //                        parsIn->printIndependentCouplings();

                        branchWeight *= weightForKnownWMass(pythiaIn,parsIn,
                            mode, eBeam, nominalWmass, integrateOverNuPz,
                            partons, neutrinoPt, lepton, leptonCharge,
                            mwsq, wMassSqDensities[iwscan]);

                        // Finally, weight from the lepton transfer
                        // function normalization
                        branchWeight *= lepton.e();
                    }
                    else
                        // Not in the kinematically allowed region
                        branchWeight = 0.0;

                    wScanWeight += branchWeight;
                }

                if (allowTau)
                {
                    // W decays into tau nu with subsequent tau -> l nu nu
                    double mwsq = wMassSqGrid[iwscan];
                    double branchWeight = wToTauBranching;
                    branchWeight *= (isMuon ? tauToMuBranching : tauToEBranching);

                    // We do not know the tau momentum, so we need to
                    // integrate over it. Assume that the direction of
                    // the observed lepton and the direction of the tau
                    // coincide. Then the minimum possible tau momentum
                    // will be equal to the observed momentum of the charged
                    // lepton. What is the max possible tau momentum?
                    const double pTauMax = maxLeptonMomentum(
                        wPt.x(), wPt.y(), mwsq, lepton.momentum());

                    // Generate the new lepton
                    const double pTauMin = lepton.e();
                    const double pTauSpan = pTauMax - pTauMin;
                    if (pTauSpan > 0.0)
                    {
                        const double pTau = pTauMin +
                            randomNumbers[RSEQ_TAU]*pTauSpan;
                        const double pRatio = lepton.e()/pTau;
                        const P4 tau(lepton/pRatio);

                        // Check if we are in danger of singular jacobian
                        const double minMWSq = minimumWMassSquared(
                            neutrinoPt, tau.momentum());
                        assert(mwsq > minMWSq);
                        
                        bool integrateOverNuPz = !iwscan;
                        if (iwscan)
                            if (wMassSqGrid[iwscan-1U] <= minMWSq)
                                integrateOverNuPz = true;

                        if (integrateOverNuPz)
                            mwsq = wMassScanner.quantile(
                                (iwscan + 1.0)/nWmassScanPoints);

                        //                       parsIn->printIndependentCouplings();

                        branchWeight *= weightForKnownWMass(pythiaIn,parsIn,
                            mode, eBeam, nominalWmass, integrateOverNuPz,
                            partons, neutrinoPt, tau, leptonCharge,
                            mwsq, wMassSqDensities[iwscan]);

                        // Weight from the tau transfer function
                        branchWeight *= tauTransferFunction.density(pRatio);
                        branchWeight *= pTauSpan;
                    }
                    else
                        // Not in the kinematically allowed region
                        branchWeight = 0.0;

                    wScanWeight += branchWeight;
                }
            }

            // Multiply the point weight by the weight from the W mass scan
            double factor = wCoverage*wScanWeight/nWmassScanPoints;

            // Parton weight from the phase space integration
            for (unsigned ijet=0; ijet<4; ++ijet)
                factor *= partonPt[ijet];

            for (unsigned ijes=0; ijes<nDeltaJes; ++ijes)
                pointWeightVec[ijes] *= factor;
        }

        // Multiply by weights due to jet transfer functions and efficiencies
        if (mode & (I_TF_MASK | I_EFF_MASK))
        {
            for (unsigned ijes=0; ijes<nDeltaJes; ++ijes)
                if (pointWeightVec[ijes] > 0.0L)
                {
                    const double deltaJes = jesAxis.binCenter(ijes);
                    for (unsigned ijet=0; ijet<4; ++ijet)
                    {
                        tfs[ijet]->setDeltaJES(deltaJes);
                        effs[ijet]->setDeltaJES(deltaJes);
                        if (mode & I_TF_MASK)
                            pointWeightVec[ijes] *= (tfs[ijet]->fcn()).density(partonPt[ijet]);
                        if (mode & I_EFF_MASK)
                            pointWeightVec[ijes] *= (effs[ijet]->fcn())(partonPt[ijet]);
                    }
                }
        }

        // Accumulate the weight for this phase space point
        for (unsigned ijes=0; ijes<nDeltaJes; ++ijes)
        {
            accVec[ijes].accumulate(pointWeightVec[ijes]);
            convergenceChecker.accumulate(ijes, pointWeightVec[ijes]);
        }

        // Check if we want to store the function values
        if (functionValues)
        {
            npstat::copyBuffer(&bufVec[0], &pointWeightVec[0], nDeltaJes);
            functionValues->fill(&bufVec[0], nDeltaJes);
        }

        // Check if we need to store this accumulator
        if (isample + 1UL == std::min(nextSamplesToStore, maxSamples))
        {
            result->history.push_back(accVec);
            nextSamplesToStore = floor(pow(2.0, ++powerOfTwoToStore) + 0.5);

            // Check the convergence
            result->timeElapsed = time_in_seconds() - startingTime;
            result->status = convergenceChecker.check(
                result->timeElapsed, result->history);

            // Fill out the QMC uncertainty estimates
            std::vector<double> uvec(nDeltaJes);
            for (unsigned ijes=0; ijes<nDeltaJes; ++ijes)
                uvec[ijes] = convergenceChecker.getRelUncertainty(ijes);
            result->qmcRelUncertainties.push_back(uvec);

            // Stop integrating of the convergence checker
            // says we should not continue
            if (result->status != CS_CONTINUE)
                break;
        }
    }

    if (result->status == CS_CONTINUE)
        // We have simply run to the end of the random sequence.
        // Don't need to update the time because it was updated
        // during the last integration cycle.
        result->status = CS_MAX_CYCLES;

    ++evcount;
}
