#include <sstream>

#include "WJetsTFSet.hh"

#include "MuonTauTF.hh"
#include "HighEnergyTauTF.hh"

#include "geners/CPP11_auto_ptr.hh"
#include "geners/stringArchiveIO.hh"
#include "geners/Reference.hh"

#include "npstat/stat/Distribution1DReader.hh"

using namespace gs;
using namespace npstat;

WJetsTFSet::WJetsTFSet(const char* fname,
                       const bool useLogScaleForTFpt,
                       const bool useDetectorEtaBins)
    : useDetectorEtaBins_(useDetectorEtaBins)
{
    // Allow (de)serialization of HighEnergyTauTF and MuonTauTF classes
    StaticDistribution1DReader::registerClass<HighEnergyTauTF>();
    StaticDistribution1DReader::registerClass<MuonTauTF>();

    // Load the archive
    CPP11_auto_ptr<StringArchive> par(readCompressedStringArchiveExt(fname));
    assert(par.get());

    // Extract eta bin limits
    Reference<std::vector<double> >(
        *par, "Eta bin limits", "").restore(0, &etaBinLimits);

    const unsigned nEtaBins = etaBinLimits.size();
    partonPtAxes.reserve(nEtaBins);
    jetPTTfs.reserve(nEtaBins);
    jetEffs.reserve(nEtaBins);

    // Extract tf data binned in eta
    for (unsigned ieta=0; ieta<nEtaBins; ++ieta)
    {
        std::vector<double> ptlist;
        std::ostringstream os;
        os << "Eta bin " << ieta << " pt values";
        Reference<std::vector<double> >(*par, os.str(), "").restore(0, &ptlist);
        PtrAxis paxis(new npstat::GridAxis(ptlist, useLogScaleForTFpt));
        partonPtAxes.push_back(paxis);

        std::vector<PtrD1D> dists;
        std::ostringstream ot;
        ot << "Eta bin " << ieta << " tf densities";
        Reference<std::vector<PtrD1D> >(*par, ot.str(), "").restore(0, &dists);
        jetPTTfs.push_back(dists);

        std::ostringstream ow;
        ow << "Eta bin " << ieta << " parton efficiency";
        PtrTab tptr = Reference<LinInterpolatedTableND<float> >(
            *par, ow.str(), "").getShared(0);
        jetEffs.push_back(tptr);
    }

    // TFs not binned in eta
    eTauTF = Reference<AbsDistribution1D>(*par, "tau to e tf", "").getShared(0);
    muTauTF = Reference<AbsDistribution1D>(*par, "tau to mu tf", "").getShared(0);
    systemPtPrior = Reference<AbsDistributionND>(
        *par, "W + 4 jets Pt profile", "").getShared(0);
    partonPtPrior = Reference<AbsDistribution1D>(
        *par, "Parton Pt prior", "").getShared(0);

    // Pop the last eta bin limit (the code will assume that it is infinity)
    etaBinLimits.pop_back();
}

WJetsTFSet::~WJetsTFSet()
{
}

const npstat::AbsDistribution1D& WJetsTFSet::getTauTF(const bool isMuon) const
{
    return isMuon ? *muTauTF : *eTauTF;
}

const npstat::AbsDistributionND& WJetsTFSet::getSystemPtPrior() const
{
    return *systemPtPrior;
}

const npstat::AbsDistribution1D& WJetsTFSet::getPartonPtPrior() const
{
    return *partonPtPrior;
}

CPP11_shared_ptr<BinnedJetPtFcn<JetPtEff> > WJetsTFSet::getJetEff(
        const double detectorPtCut, const double sigmaAtPtCut) const
{
    typedef BinnedJetPtFcn<JetPtEff> Ret;

    const unsigned n = etaBinLimits.size() + 1U;
    std::vector<JetPtEff> effs;
    effs.reserve(n);
    for (unsigned i=0; i<n; ++i)
        effs.push_back(JetPtEff(jetEffs.at(i), detectorPtCut, sigmaAtPtCut));
    Ret* p = new Ret(etaBinLimits, effs, useDetectorEtaBins_);
    return CPP11_shared_ptr<Ret>(p);
}

CPP11_shared_ptr<BinnedJetPtFcn<JetPtTF> > WJetsTFSet::getJetTf(
        const double detectorPtCut, const double sigmaAtPtCut) const
{
    typedef BinnedJetPtFcn<JetPtTF> Ret;

    const unsigned n = etaBinLimits.size() + 1U;
    std::vector<JetPtTF> tfs;
    tfs.reserve(n);
    for (unsigned i=0; i<n; ++i)
        tfs.push_back(JetPtTF(*partonPtAxes.at(i), jetPTTfs.at(i),
                              detectorPtCut, sigmaAtPtCut));
    Ret* p = new Ret(etaBinLimits, tfs, useDetectorEtaBins_);
    return CPP11_shared_ptr<Ret>(p);
}
