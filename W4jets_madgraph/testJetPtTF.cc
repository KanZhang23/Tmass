#include <sstream>
#include <iostream>

#include "BinnedJetPtFcn.hh"
#include "JetPtTFDensity.hh"
#include "convertMCJetPtToData.hh"
#include "importanceSamplingDistro.hh"
#include "misc_utils.hh"
#include "HighEnergyTauTF.hh"
#include "MuonTauTF.hh"
#include "JesIntegResult.hh"

#include "geners/StringArchive.hh"
#include "geners/BinaryFileArchive.hh"
#include "geners/Record.hh"
#include "geners/Reference.hh"

#include "npstat/nm/LinearMapper1d.hh"

#include "npstat/rng/MersenneTwister.hh"

#include "npstat/stat/TruncatedDistribution1D.hh"
#include "npstat/stat/HistoND.hh"
#include "npstat/stat/InMemoryNtuple.hh"
#include "npstat/stat/Distribution1DReader.hh"

#define SYS_UNCERTAINTY_A   0.5813572637827301
#define SYS_UNCERTAINTY_B (-0.8613531161467861)

using namespace npstat;
using namespace geom3;
using namespace rk;
using namespace gs;

// The systematic uncertainty drops from 10% at 10 GeV to 4% at 50 GeV
// to 2% at infinity
namespace {
    struct ModelUncert
    {
        inline double operator()(const double pt) const
        {
            const double a = SYS_UNCERTAINTY_A;
            const double b = SYS_UNCERTAINTY_B;

            return 0.02 + a*pow(pt, b);
        }
    };

    struct ModelUncertDeriv
    {
        inline double operator()(const double pt) const
        {
            const double a = SYS_UNCERTAINTY_A;
            const double b = SYS_UNCERTAINTY_B;

            return a*b*pow(pt, b-1.0);
        }
    };
}

typedef double (JetPtTF::*JetPtTFFcn)(double) const;

static void scanTfIntoHisto(JetPtTF& tf, JetPtTFFcn fcn, HistoND<double>* h)
{
    assert(h);
    assert(h->dim() == 2U);

    const HistoAxis& partonAxis = h->axis(0);
    const HistoAxis& jetAxis = h->axis(1);
    const unsigned nPart = partonAxis.nBins();
    const unsigned nJet = jetAxis.nBins();

    const JetInfo origJet = tf.getJetInfo();

    ModelUncert syserr;
    ModelUncertDeriv derrdpt;

    for (unsigned ijet=0; ijet<nJet; ++ijet)
    {
        const double jetPt = jetAxis.binCenter(ijet);
        P4 p4(jetPt*UnitVector3::xAxis(), 0.0);
        JetInfo jet(p4, 0.0, syserr(jetPt), derrdpt(jetPt));
        tf.setJetInfo(jet);

        for (unsigned ipart=0; ipart<nPart; ++ipart)
        {
            const double partonPt = partonAxis.binCenter(ipart);
            const double value = (tf.*fcn)(partonPt);
            h->setBin(ipart, ijet, value);
        }
    }

    tf.setJetInfo(origJet);
}


static void scanEffTfIntoHisto(const JetPtEff& eff, JetPtTF& tf,
                               JetPtTFFcn fcn, HistoND<double>* h)
{
    assert(h);
    assert(h->dim() == 2U);

    const HistoAxis& partonAxis = h->axis(0);
    const HistoAxis& jetAxis = h->axis(1);
    const unsigned nPart = partonAxis.nBins();
    const unsigned nJet = jetAxis.nBins();

    const JetInfo origJet = tf.getJetInfo();

    ModelUncert syserr;
    ModelUncertDeriv derrdpt;

    for (unsigned ijet=0; ijet<nJet; ++ijet)
    {
        const double jetPt = jetAxis.binCenter(ijet);
        P4 p4(jetPt*UnitVector3::xAxis(), 0.0);
        JetInfo jet(p4, 0.0, syserr(jetPt), derrdpt(jetPt));
        tf.setJetInfo(jet);

        for (unsigned ipart=0; ipart<nPart; ++ipart)
        {
            const double partonPt = partonAxis.binCenter(ipart);
            const double value = (tf.*fcn)(partonPt)*eff(partonPt);
            h->setBin(ipart, ijet, value);
        }
    }

    tf.setJetInfo(origJet);
}


static void scanTfDensityIntoHisto(JetPtTF& tf, const double partPt,
                                   HistoND<double>* h)
{
    assert(h);
    assert(h->dim() == 1U);

    const HistoAxis& jetAxis = h->axis(0);
    const unsigned nJet = jetAxis.nBins();

    const JetInfo origJet = tf.getJetInfo();

    ModelUncert syserr;
    ModelUncertDeriv derrdpt;

    for (unsigned ijet=0; ijet<nJet; ++ijet)
    {
        const double jetPt = jetAxis.binCenter(ijet);
        P4 p4(jetPt*UnitVector3::xAxis(), 0.0);
        JetInfo jet(p4, 0.0, syserr(jetPt), derrdpt(jetPt));
        tf.setJetInfo(jet);
        h->setBin(ijet, tf.density(partPt));
    }

    tf.setJetInfo(origJet);
}


static void scanEffIntoHisto(const JetPtEff& eff, HistoND<double>* h)
{
    assert(h);
    assert(h->dim() == 1U);

    const HistoAxis& axis = h->axis(0);
    const unsigned nBins = axis.nBins();
    for (unsigned i=0; i<nBins; ++i)
    {
        const double partPt = axis.binCenter(i);
        h->setBin(i, eff(partPt));
    }
}


static double integrateTFInPtJet(JetPtTF& tf, const double partPt,
                                 const double ptmin, const double ptmax,
                                 const unsigned nIntervals)
{
    const JetInfo origJet = tf.getJetInfo();
    const double step = (ptmax - ptmin)/nIntervals;

    ModelUncert syserr;
    ModelUncertDeriv derrdpt;

    long double sum = 0.0L;
    for (unsigned i=0; i<=nIntervals; ++i)
    {
        {
            const double pt = ptmin + step*i;
            P4 p4(pt*UnitVector3::xAxis(), 0.0);
            JetInfo jet(p4, 0.0, syserr(pt), derrdpt(pt));
            tf.setJetInfo(jet);
            const double fval = tf.density(partPt);
            if (i == 0 || i == nIntervals)
                sum += fval;
            else
                sum += 2.0*fval;
        }
        if (i < nIntervals)
        {
            const double pt = ptmin + step*(i + 0.5);
            P4 p4(pt*UnitVector3::xAxis(), 0.0);
            JetInfo jet(p4, 0.0, syserr(pt), derrdpt(pt));
            tf.setJetInfo(jet);
            const double fval = tf.density(partPt);
            sum += 4.0*fval;
        }
    }

    tf.setJetInfo(origJet);
    return sum*step/6.0;
}


int main(int argc, char* argv[])
{
    // Parse input arguments
    if (argc != 2)
    {
        std::cout << "\nUsage: testJetPtTF archiveName\n" << std::endl;
        return argc == 1 ? 0 : 1;
    }
    const char* archiveName = argv[1];

    // Hardwired parameters
    const double tol = 1.0e-10;
    const double detectorPtCut = 15.0;
    const double mcPtCut = 10.0;
    const double partonPtMin = 5.0;
    const double partonPtMax = 100.0;
    const double meanAtPtMin = 1.1;
    const double meanAtPtMax = 0.9;
    const unsigned partonPtIntervals = 95;
    const unsigned nGridPt = partonPtIntervals + 1;
    const unsigned nGenerate = 100000;

    // Get ourselves a generator of pseudo-random numbers
    MersenneTwister rng(1000003);

    // The parton Pt grid for the densities
    std::vector<double> gridCoords;
    gridCoords.reserve(nGridPt);

    std::vector<CPP11_shared_ptr<const AbsDistribution1D> > distros;
    distros.reserve(nGridPt);

    // Build the transfer function
    const double step = (partonPtMax - partonPtMin)/partonPtIntervals;
    const LinearMapper1d meanMap(partonPtMin, meanAtPtMin,
                                 partonPtMax, meanAtPtMax);
    for (unsigned ipt=0; ipt<nGridPt; ++ipt)
    {
        const double partPt = partonPtMin + ipt*step;
        gridCoords.push_back(partPt);

        const double resolution = 1.0/sqrt(partPt);
        const double ratioMean = meanMap(partPt);

        const Gauss1D g(ratioMean, resolution);
        const double umin = mcPtCut/partPt;
        const double umax = ratioMean + 10.0*resolution;
        TruncatedDistribution1D* t = new TruncatedDistribution1D(g, umin, umax);
        distros.push_back(CPP11_shared_ptr<const AbsDistribution1D>(t));
    }

    ModelUncert syserr;
    ModelUncertDeriv derrdpt;

    JetPtTF tf(GridAxis(gridCoords), distros,
               detectorPtCut, syserr(detectorPtCut));

    // Build the efficiency interpolator
    UniformAxis cutAxis(51, 10.0, 20.0, "MC cut");
    UniformAxis partonPtAxis(501, 1.0, 101.0, "Parton Pt");
    LinInterpolatedTableND<float>* tbl = new LinInterpolatedTableND<float>(
        cutAxis, false, false, partonPtAxis, false, false, "Efficiency");
    CPP11_shared_ptr<const LinInterpolatedTableND<float> > effInterpTable(tbl);
    ArrayND<float>& effData(tbl->table());
    const unsigned nCutScan = cutAxis.nCoords();
    const unsigned nPartScan = partonPtAxis.nCoords();
    for (unsigned i=0; i<nCutScan; ++i)
    {
        const double mcCut = cutAxis.coordinate(i);

        for (unsigned j=0; j<nPartScan; ++j)
        {
            const double partPt = partonPtAxis.coordinate(j);
            const double resolution = 1.0/sqrt(partPt);
            const double ratioMean = meanMap(partPt);
            const Gauss1D g(ratioMean, resolution);
            effData(i,j) = g.exceedance(mcCut/partPt);
        }
    }
    JetPtEff effInterp(effInterpTable, detectorPtCut, syserr(detectorPtCut));

    // Various scan definitions
    const double deltaJesValues[] = {-1.5, 0.0, 1.0};
    const unsigned nJesValues = sizeof(deltaJesValues)/sizeof(deltaJesValues[0]);

    const double samplePartonPtValues[] = {10.0, 10.3, 10.5, 10.7,
                                           50.0, 50.3, 50.5, 50.7,
                                           120.0};
    const unsigned nSampledPartonPt = sizeof(samplePartonPtValues)/
                                      sizeof(samplePartonPtValues[0]);

    const double sampleJetPtValues[] = {16.0, 50.0, 120.0};
    const unsigned nSampledJetPt = sizeof(sampleJetPtValues)/
                                   sizeof(sampleJetPtValues[0]);

    HistoAxis xaxis(400, partonPtMin, partonPtMin+200.0, "Parton Pt");
    HistoAxis yaxis(1200, partonPtMin, partonPtMin+300.0, "Jet Pt");

    // Archive for saving the results
    BinaryFileArchive ar(archiveName, "w");
    assert(ar.isOpen());

    for (unsigned ijes=0; ijes<nJesValues; ++ijes)
    {
        const double deltaJes = deltaJesValues[ijes];
        tf.setDeltaJES(deltaJes);
        effInterp.setDeltaJES(deltaJes);

        {
            std::ostringstream tstream;
            tstream << "Efficiency Scan for delta_JES = " << deltaJes;
            const std::string& title = tstream.str();
            HistoND<double> h(xaxis, title.c_str(), "Efficiency");
            scanEffIntoHisto(effInterp, &h);
            ar << Record(h, title, "");
        }

        {
            std::ostringstream tstream;
            tstream << "TF Density Scan for delta_JES = " << deltaJes;
            const std::string& title = tstream.str();
            HistoND<double> h(xaxis, yaxis, title.c_str(), "Density");
            scanTfIntoHisto(tf, &JetPtTF::density, &h);
            ar << Record(h, title, "");
        }

        {
            std::ostringstream tstream;
            tstream << "TF Product Scan for delta_JES = " << deltaJes;
            const std::string& title = tstream.str();
            HistoND<double> h(xaxis, yaxis, title.c_str(), "Density");
            scanEffTfIntoHisto(effInterp, tf, &JetPtTF::density, &h);
            ar << Record(h, title, "");
        }

        {
            std::ostringstream tstream;
            tstream << "TF CDF Scan for delta_JES = " << deltaJes;
            const std::string& title = tstream.str();
            HistoND<double> h(xaxis, yaxis, title.c_str(), "CDF");
            scanTfIntoHisto(tf, &JetPtTF::cdf, &h);
            ar << Record(h, title, "");
        }

        // Verify the relationship between "cdf" and "density"
        const unsigned ntries = 100;
        unsigned nchecks = 0;
        for (unsigned itry=0; itry<ntries; ++itry)
        {
            const double partonPt = xaxis.min() + rng()*xaxis.length();
            const double resolution = 1.0/sqrt(partonPt);
            const double ratioMean = meanMap(partonPt);
            const Gauss1D g(ratioMean, resolution);
            const double locut = ratioMean-10.0*resolution;

            double rnd;
            g.random(rng, &rnd);
            const double jetPt = partonPt*rnd;
            if (rnd <= locut || jetPt <= detectorPtCut)
                continue;

            double lolim = locut*partonPt;
            if (lolim < detectorPtCut)
                lolim = detectorPtCut;
            const double integ = integrateTFInPtJet(
                tf, partonPt, lolim, jetPt, 10000);

            P4 p4(jetPt*UnitVector3::xAxis(), 0.0);
            JetInfo jet(p4, 0.0, syserr(jetPt), derrdpt(jetPt));
            tf.setJetInfo(jet);
            const double cdf = tf.cdf(partonPt);

            assert(relative_delta(integ, cdf) < tol);
            ++nchecks;
        }
        assert(nchecks > ntries/2);

        // Sample the jet Pt at some values of parton Pt
        for (unsigned isample=0; isample<nSampledPartonPt; ++isample)
        {
            const double partonPt = samplePartonPtValues[isample];

            {
                std::ostringstream tstream;
                tstream << "1d TF Density for delta_JES = " << deltaJes
                        << ", parton Pt = " << partonPt;
                const std::string& title = tstream.str();
                HistoND<double> h(yaxis, title.c_str(), "Density");
                scanTfDensityIntoHisto(tf, partonPt, &h);
                ar << Record(h, title, "");
            }

            {
                std::ostringstream tstream;
                tstream << "Generated Jets for delta_JES = " << deltaJes
                        << ", parton Pt = " << partonPt;
                const std::string& title = tstream.str();
                HistoND<double> h(yaxis, title.c_str(), "N");

                for (unsigned i=0; i<nGenerate; ++i)
                {
                    const double mcPt = tf.sampleMCJetPt(partonPt, rng());
                    const double jetPt = convertMCJetPtToData(
                        mcPt, deltaJes, syserr);
                    h.fill(jetPt, 1.0);
                }

                ar << Record(h, title, "");
            }
        }

        // Test the importance sampler
        for (unsigned isample=0; isample<nSampledJetPt; ++isample)
        {
            const double jetPt = sampleJetPtValues[isample];
            P4 p4(jetPt*UnitVector3::xAxis(), 0.0);
            JetInfo jet(p4, 0.0, syserr(jetPt), derrdpt(jetPt));
            tf.setJetInfo(jet);

            {
            CPP11_auto_ptr<const Tabulated1D> sampler = importanceSamplingDistro(
                JetPtTFDensity(tf), Uniform1D(0.0, 501.0), jetPt/3.0, 500.0);

            {
                std::ostringstream tstream;
                tstream << "Importance Sampler for delta_JES = " << deltaJes
                        << ", Jet Pt = " << jetPt;
                const std::string& title = tstream.str();
                InMemoryNtuple<float> nt(ntupleColumns("pt", "density"),
                                         title.c_str());

                const unsigned nscan = 1000;
                for (unsigned i=0; i<nscan; ++i)
                {
                    const double r = (i + 0.5)/nscan;
                    const double logPt = sampler->quantile(r);
                    const double partonPt = exp(logPt);
                    const double density = sampler->density(logPt)/partonPt;
                    nt.fill(partonPt, density);
                }

                ar << Record(nt, title, "");
            }

            {
                std::ostringstream tstream;
                tstream << "Generated Partons for delta_JES = " << deltaJes
                        << ", Jet Pt = " << jetPt;
                const std::string& title = tstream.str();
                HistoND<double> h(xaxis, title.c_str(), "N");

                for (unsigned i=0; i<nGenerate; ++i)
                {
                    const double r = rng();
                    const double logPt = sampler->quantile(r);
                    const double partonPt = exp(logPt);
                    h.fill(partonPt, 1.0);
                }

                ar << Record(h, title, "");
            }
            }

            {
            CPP11_auto_ptr<const Tabulated1D> sampler = importanceSamplingDistro(
                JetPtEffDensity(tf, effInterp), Uniform1D(0.0, 501.0), jetPt/3.0, 500.0);

            {
                std::ostringstream tstream;
                tstream << "Efficiency Sampler for delta_JES = " << deltaJes
                        << ", Jet Pt = " << jetPt;
                const std::string& title = tstream.str();
                InMemoryNtuple<float> nt(ntupleColumns("pt", "density"),
                                         title.c_str());

                const unsigned nscan = 1000;
                for (unsigned i=0; i<nscan; ++i)
                {
                    const double r = (i + 0.5)/nscan;
                    const double logPt = sampler->quantile(r);
                    const double partonPt = exp(logPt);
                    const double density = sampler->density(logPt)/partonPt;
                    nt.fill(partonPt, density);
                }

                ar << Record(nt, title, "");
            }

            {
                std::ostringstream tstream;
                tstream << "Efficiency Partons for delta_JES = " << deltaJes
                        << ", Jet Pt = " << jetPt;
                const std::string& title = tstream.str();
                HistoND<double> h(xaxis, title.c_str(), "N");

                for (unsigned i=0; i<nGenerate; ++i)
                {
                    const double r = rng();
                    const double logPt = sampler->quantile(r);
                    const double partonPt = exp(logPt);
                    h.fill(partonPt, 1.0);
                }

                ar << Record(h, title, "");
            }
            }

        }
    }

    // Just make sure that we can instantiate BinnedJetPtFcn
    std::vector<double> etaBinLimits(1);
    etaBinLimits[0] = 1.0;

    std::vector<JetPtEff> v1;
    v1.push_back(effInterp);
    v1.push_back(effInterp);

    BinnedJetPtFcn<JetPtEff> beff(etaBinLimits, v1, true);
    const double jetPt = 100.0;
    P4 p4(jetPt*UnitVector3::xAxis(), 0.0);
    JetInfo jet(p4, 0.0, syserr(jetPt), derrdpt(jetPt));
    beff.setJetInfo(jet);
    beff.setDeltaJES(1.9);

    HighEnergyTauTF tautf(0.2);
    for (unsigned i=0; i<1000; ++i)
    {
        const double x = drand48();
        const double cdf = tautf.cdf(x);
        const double x2 = tautf.quantile(cdf);
        assert(relative_delta(x, x2) < 1.e-12);
    }

    MuonTauTF mutautf(0.2, 0.01);
    for (unsigned i=0; i<1000; ++i)
    {
        const double x = drand48();
        const double cdf = mutautf.cdf(x);
        const double x2 = mutautf.quantile(cdf);
        assert(relative_delta(x, x2) < 1.e-12);
    }

    // Check transfer function I/O
    StaticDistribution1DReader::registerClass<HighEnergyTauTF>();
    StaticDistribution1DReader::registerClass<MuonTauTF>();

    StringArchive tar;
    tar << Record(tf, "a", "")
        << Record(effInterp, "b", "")
        << Record(beff, "c", "")
        << Record(dynamic_cast<AbsDistribution1D&>(tautf), "d", "")
        << Record(dynamic_cast<AbsDistribution1D&>(mutautf), "e", "");

    Reference<JetPtTF> r1(tar, "a", "");
    assert(r1.unique());
    CPP11_auto_ptr<JetPtTF> pr1 = r1.get(0);
    assert(*pr1 == tf);

    Reference<JetPtEff> r2(tar, "b", "");
    assert(r2.unique());
    CPP11_auto_ptr<JetPtEff> pr2 = r2.get(0);
    assert(*pr2 == effInterp);

    Reference<BinnedJetPtFcn<JetPtEff> > r3(tar, "c", "");
    assert(r3.unique());
    CPP11_auto_ptr<BinnedJetPtFcn<JetPtEff> > pr3 = r3.get(0);
    assert(*pr3 == beff);

    Reference<AbsDistribution1D> r4(tar, "d", "");
    assert(r4.unique());
    CPP11_auto_ptr<AbsDistribution1D> pr4 = r4.get(0);
    assert(*pr4 == tautf);

    Reference<AbsDistribution1D> r5(tar, "e", "");
    assert(r5.unique());
    CPP11_auto_ptr<AbsDistribution1D> pr5 = r5.get(0);
    assert(*pr5 == mutautf);

    // Check I/O for JesIntegResult objects
    JesIntegResult jires;
    jires.uid = -15;
    jires.minDeltaJes = -3.0;
    jires.maxDeltaJes = 3.0;
    jires.nDeltaJes = 30;
    jires.timeElapsed = 23.2;
    jires.status = CS_MAX_CYCLES;
    for (unsigned ihist=0; ihist<4; ++ihist)
    {
        std::vector<npstat::StatAccumulator> avec;
        for (unsigned iacc=0; iacc<3; ++iacc)
        {
            npstat::StatAccumulator acc;
            for (unsigned i=0; i<100; ++i)
                acc.accumulate(drand48());
            avec.push_back(acc);
        }
        jires.history.push_back(avec);
    }

    tar << Record(jires, "History", "");
    Reference<JesIntegResult> hiref(tar, "History", "");
    assert(hiref.unique());
    JesIntegResult jires2;
    hiref.restore(0, &jires2);
    assert(jires == jires2);
    
    return 0;
}
