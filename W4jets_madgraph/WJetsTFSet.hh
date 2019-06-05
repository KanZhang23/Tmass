#ifndef WJETSTFSET_HH_
#define WJETSTFSET_HH_

#include <vector>

#include "geners/CPP11_shared_ptr.hh"

#include "npstat/nm/GridAxis.hh"
#include "npstat/nm/LinInterpolatedTableND.hh"

#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/stat/AbsDistributionND.hh"

#include "BinnedJetPtFcn.hh"
#include "JetPtTF.hh"
#include "JetPtEff.hh"

//
// Class to read the jet TFs and other priors from disk
//
class WJetsTFSet
{
public:
    typedef CPP11_shared_ptr<const npstat::GridAxis> PtrAxis;
    typedef CPP11_shared_ptr<const npstat::AbsDistribution1D> PtrD1D;
    typedef CPP11_shared_ptr<const npstat::LinInterpolatedTableND<float> > PtrTab;

    WJetsTFSet(const char* gssaArchive, bool useLogScaleForTFpt,
               bool useDetectorEtaBins);

    ~WJetsTFSet();

    CPP11_shared_ptr<BinnedJetPtFcn<JetPtTF> > getJetTf(
        double detectorPtCut, double sigmaAtPtCut) const;

    CPP11_shared_ptr<BinnedJetPtFcn<JetPtEff> > getJetEff(
        double detectorPtCut, double sigmaAtPtCut) const;

    const npstat::AbsDistribution1D& getTauTF(bool isMuon) const;

    const npstat::AbsDistribution1D& getPartonPtPrior() const;

    const npstat::AbsDistributionND& getSystemPtPrior() const;

private:
    WJetsTFSet();

    // Definition of eta bins
    std::vector<double> etaBinLimits;

    // Eta-dependent transfer functions and priors
    std::vector<PtrAxis> partonPtAxes;
    std::vector<std::vector<PtrD1D> > jetPTTfs;
    std::vector<PtrTab> jetEffs;

    // Transfer functions and priors independent of eta
    CPP11_shared_ptr<const npstat::AbsDistribution1D> eTauTF;
    CPP11_shared_ptr<const npstat::AbsDistribution1D> muTauTF;
    CPP11_shared_ptr<const npstat::AbsDistribution1D> partonPtPrior;
    CPP11_shared_ptr<const npstat::AbsDistributionND> systemPtPrior;

    // Use detector eta to determine eta bins or jet eta?
    bool useDetectorEtaBins_;
};

#endif // WJETSTFSET_HH_
