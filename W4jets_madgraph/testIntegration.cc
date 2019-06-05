#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <cassert>
#include <climits>

#include "CmdLine.hh"

#include "integrateW4Jets.hh"
#include "WJetsTFSet.hh"
#include "RandomNumberSequence.hh"
#include "misc_utils.hh"
#include "getJESSigma.hh"

#include "JetCorrectionsInterface.hh"

#include "geners/stringArchiveIO.hh"
#include "geners/Record.hh"

#include "npstat/rng/HOSobolGenerator.hh"
#include "npstat/rng/MersenneTwister.hh"

#include "npstat/stat/Distributions1D.hh"

#define L5_CORRECTOR 0U

#include "Pythia8/Pythia.h"
#include "Parameters_sm_no_b_mass.h"

using namespace Pythia8;

static Pythia pythia;
static Parameters_sm_no_b_mass * pars;


using namespace npstat;
using namespace geom3;
using namespace rk;
using namespace gs;
using namespace std;

static void generate_random_jet(AbsRandomGenerator& rng,
                                JetInfo* jet, const double jetPtCut)
{
    assert(jet);

    const double jetEta = (rng() - 0.5)*5.0;
    const double jetPhi = 2.0*M_PI*rng();
    const double jetPt = jetPtCut + 60.0*rng();
    const double m = 1.0 + 10.0*rng();
    const rk::P4 p4(jetPt*Vector3(cos(jetPhi), sin(jetPhi), sinh(jetEta)), m);

    const double syserr = getJESSigma(L5_CORRECTOR, jetPt, jetEta);

    const double delta = jetPt*0.001;
    const double sysplus = getJESSigma(L5_CORRECTOR, jetPt+delta, jetEta);
    const double sysminus = getJESSigma(L5_CORRECTOR, jetPt-delta, jetEta);
    const double sysderi = (sysplus - sysminus)/2.0/delta;

    *jet = JetInfo(p4, jetEta, syserr, sysderi);
}

static void generate_random_lepton(AbsRandomGenerator& rng, rk::P4* p4)
{
    assert(p4);

    const double leptonEta = (rng() - 0.5)*5.0;
    const double leptonPhi = 2.0*M_PI*rng();
    const double leptonPt = 20.0 + 100.0*rng();
    *p4 = rk::P4(leptonPt*Vector3(cos(leptonPhi), sin(leptonPhi),
                                  sinh(leptonEta)), 0.0);
}

static void print_usage(const char* progname)
{
    cout << "\nUsage: " << progname << " [options] tfFile outfile\n\n"
         << "Possible options are:\n\n"
         << "  -n, --nEvents           Specify the number of events to process.\n\n"
         << "  -s, --seed              Specify the random number generator seed.\n\n"
         << "  --nSkip                 Number of pseudo-random points to skip (in\n"
         << "                          multiples of integration point dimensionality,\n"
         << "                          must be a power of two).\n\n"
         << "  --nInteg                Specify the number of Quasi-MC points to use for\n"
         << "                          integration. Must be a power of two.\n\n"
         << "  --interlacingFactor     Interlacing factor for the QMC (1, 2, or 3).\n\n" 
         << "  --minDeltaJES           These three options specify the grid in the delta\n"
         << "  --maxDeltaJES           JES variable (as if specifying a histogram axis).\n"
         << "  --deltaJesSteps         \n\n"
         << "  --beamEnergy            Beam energy in GeV.\n\n"
         << "  --maxWmassForScan       Upper limit for the W mass integration.\n\n"
         << "  --nWmassScanPoints      Number of points to use in the W mass integration.\n\n"
         << "  --minPartonPtFactor     Minimum parton pt/jet pt ratio to consider for\n"
         << "                          building transfer function importance samplers.\n\n"
         << "  --jetPtCut              Jet Pt cut al level 5.\n\n"
         << "  --firstPowerOfTwo       Integration results are saved for 2^m, 2^(m+1), ...\n"
         << "                          QMC points. This option can be used to specify the\n"
         << "                          first power of 2 (i.e., the value of m) for which\n"
         << "                          the integration result is saved.\n\n"
         << "  -r, --reportInterval    Reporting interval in the cycle over QMC points.\n\n"
         << "  --maxSeconds            Approximate limit for the wall clock run time.\n\n"
         << "The following options are boolean switches (i.e., take no subsequent values):\n\n"
         << "  --usePseudoMC           Use pseudo-random points for integration\n"
         << "                          (instead of quasi-random).\n\n"
         << "  --useLogScaleForTFpt    Interpolate transfer functions in log(pt) instead\n"
         << "                          of pt (here, pt means parton pt).\n\n"
         << "  --allowTau              Integrate matrix element for W -> tau nu (with\n"
         << "                          subsequent tau -> l nu nu).\n\n"
         << "  --noDirectWDecay        Do not calculate matrix element integral for\n"
         << "                          W -> l nu with l = e or mu (i.e., integrate\n"
         << "                          W -> tau nu only).\n\n"
         << "  --binInEtaUsingJetEta   Use jet eta instead of detector eta to define\n"
         << "                          eta bins for the transfer functions.\n"
         << endl;
}

int main(int argc, char* argv[])
{
    typedef CPP11_shared_ptr<BinnedJetPtFcn<JetPtTF> > TFPtr;
    typedef CPP11_shared_ptr<BinnedJetPtFcn<JetPtEff> > EffPtr;

    // Parse input arguments
    CmdLine cmdline(argc, argv);
    if (argc == 1)
    {
        print_usage(cmdline.progname());
        return 0;
    }

    // Nonmodifiable parameters
    const unsigned minIntegPoints = 32;
    const double nominalWmass = 80.385;
    const double nominalWwidth = 2.085;
    const Vector3 centralValueOfTotalPt(0., 0., 0.);

    // Modifiable parameters
    unsigned nEvents = 3;
    unsigned long seed = 0;
    unsigned nSkip = 0;
    unsigned nIntegrationPoints = 2048;
    unsigned interlacingFactor = 2;
    double minDeltaJES = -3.05;
    double maxDeltaJES = 3.05;
    unsigned deltaJesSteps = 61;
    double eBeam = 980.0;
    double maxWmassForScan = 2.0*nominalWmass;
    unsigned nWmassScanPoints = 40;
    double minPartonPtFactor = 0.35;
    double jetPtCut = 15.0;
    unsigned firstPowerOfTwoToStore = 6;
    unsigned reportInterval = 0;
    double maxSeconds = DBL_MAX;

    bool useQuasiMC;
    bool useLogScaleForTFpt;
    bool allowTau;
    bool allowDirectDecay;
    bool useDetectorEtaBins;

    string tfFile;
    string outFile;

    try {
        cmdline.option("-n", "--nEvents") >> nEvents;
        cmdline.option("-s", "--seed") >> seed;
        cmdline.option(0, "--nSkip") >> nSkip;
        cmdline.option(0, "--nInteg") >> nIntegrationPoints;
        cmdline.option(0, "--interlacingFactor") >> interlacingFactor;
        cmdline.option(0, "--minDeltaJES") >> minDeltaJES;
        cmdline.option(0, "--maxDeltaJES") >> maxDeltaJES;
        cmdline.option(0, "--deltaJesSteps") >> deltaJesSteps;
        cmdline.option(0, "--beamEnergy") >> eBeam;
        cmdline.option(0, "--maxWmassForScan") >> maxWmassForScan;
        cmdline.option(0, "--nWmassScanPoints") >> nWmassScanPoints;
        cmdline.option(0, "--minPartonPtFactor") >> minPartonPtFactor;
        cmdline.option(0, "--jetPtCut") >> jetPtCut;
        cmdline.option(0, "--firstPowerOfTwo") >> firstPowerOfTwoToStore;
        cmdline.option("-r", "--reportInterval") >> reportInterval;
        cmdline.option(0, "--maxSeconds") >> maxSeconds;

        useQuasiMC = !cmdline.has(0, "--usePseudoMC");
        useLogScaleForTFpt = cmdline.has(0, "--useLogScaleForTFpt");
        allowTau = cmdline.has(0, "--allowTau");
        allowDirectDecay = !cmdline.has(0, "--noDirectWDecay");
        useDetectorEtaBins = !cmdline.has(0, "--binInEtaUsingJetEta");

        if (interlacingFactor < 1 || interlacingFactor > 3)
            throw CmdLineError("QMC interlacing factor out of range");
        
        if (nIntegrationPoints < minIntegPoints)
            throw CmdLineError("not enough QMC integration points");

        if (useQuasiMC && nIntegrationPoints > 1048576)
            throw CmdLineError("too many QMC integration points");

        if (!is_power_of_two(nIntegrationPoints))
            throw CmdLineError("number of QMC integration points "
                               "must be a power of 2");

        if (nSkip)
            if (!is_power_of_two(nSkip))
                throw CmdLineError("number of random points to skip "
                                   "must be a power of 2");

        if (!allowTau && !allowDirectDecay)
            throw CmdLineError("at least one W decay mode must be allowed");
        
        cmdline.optend();

        if (cmdline.argc() != 2)
            throw CmdLineError("wrong number of command line arguments");
        cmdline >> tfFile >> outFile;
    }
    catch (const CmdLineError& e) {
        cerr << "Error in " << cmdline.progname() << ": " << e.str() << endl;
        return 1;
    }

    // Read the transfer function data
    WJetsTFSet tfset(tfFile.c_str(), useLogScaleForTFpt, useDetectorEtaBins);

    // Random number generator to use
    CPP11_auto_ptr<AbsRandomGenerator> rng;
    if (seed)
        rng = CPP11_auto_ptr<AbsRandomGenerator>(new MersenneTwister(seed));
    else
        // The seed itself will be random
        rng = CPP11_auto_ptr<AbsRandomGenerator>(new MersenneTwister());

    // Skip some number of points (if requested)
    if (nSkip)
    {
        double buf[N_RSEQ_VARS];
        for (unsigned i=0; i<nSkip; ++i)
            rng->run(buf, N_RSEQ_VARS, 1U);
    }

    // Quasi-MC (or pseudo-MC) vectors for integration
    InMemoryNtuple<double> qmc(simpleColumnNames(N_RSEQ_VARS));
    {
        HOSobolGenerator sobol(N_RSEQ_VARS, interlacingFactor, 20U);
        AbsRandomGenerator& gen = useQuasiMC ?
            dynamic_cast<AbsRandomGenerator&>(sobol) : *rng;
        double buf[minIntegPoints*N_RSEQ_VARS];
        for (unsigned i=0; i<nIntegrationPoints/minIntegPoints; ++i)
        {
            gen.run(buf, minIntegPoints*N_RSEQ_VARS, minIntegPoints);
            qmc.fill(buf, minIntegPoints*N_RSEQ_VARS);
        }
    }

    // Initialize jet corrections
    const int jc_nvtx = 1;
    const int jc_conesize = 0;
    const int jc_version = 5;
    const int jc_syscode = 0;
    const int jc_nrun = 198000;
    const int jc_mode = 0; // Use 1 for data, 0 for MC
    init_generic_corrections(L5_CORRECTOR, 5, jc_nvtx, jc_conesize,
                             jc_version, jc_syscode, jc_nrun, jc_mode);

    // Archive to store the results
    StringArchive ar;

    pythia.readString("ProcessLevel:all = off");
    pythia.init();
    pars = Parameters_sm_no_b_mass::getInstance(); 
    ParticleData *particleDataPtr = &pythia.particleData;
    SusyLesHouches *slhaPtr = &pythia.slhaInterface.slha;
    pars->setIndependentParameters(particleDataPtr,
                                   pythia.couplingsPtr, 
                                   slhaPtr);
    pars->setIndependentCouplings();
    pars->printIndependentParameters();
    pars->printIndependentCouplings();


    // Cycle over events
    for (unsigned iev=0; iev<nEvents; ++iev)
    {
        cout << time_stamp() << " processing event " << iev << endl;

        // Generate random event data
        //
        // Make a set of jets with some guaranteed minimum
        // separation in R
        //
        const unsigned nJets = 4;
        JetInfo jets[nJets];
        while (true)
        {
            for (unsigned ijet=0; ijet<nJets; ++ijet)
                generate_random_jet(*rng, jets + ijet, jetPtCut);

            double minDR = 1.0e100;
            for (unsigned i=1; i<nJets; ++i)
                for (unsigned j=0; j<i; ++j)
                {
                    const double dr = deltaR(jets[i].p4(), jets[j].p4());
                    if (dr < minDR)
                        minDR = dr;
                }
            if (minDR > 0.4)
                break;
        }

        // Generate a random lepton
        rk::P4 lepton;
        generate_random_lepton(*rng, &lepton);
        const int leptonCharge = (*rng)() > 0.5 ? 1 : -1;
        const bool isMuon = (*rng)() > 0.5;

        // Build the transfer functions. It has to be done here
        // rather than outside of the event cycle because JES
        // uncertainty is eta-dependent (even for the detector
        // jet pt cut) and, therefore, cannot be defined until
        // the jet eta values are known.
        vector<TFPtr> tfVec;
        vector<EffPtr> effVec;
        tfVec.reserve(nJets);
        effVec.reserve(nJets);
        for (unsigned i=0; i<nJets; ++i)
        {
            const double eta = jets[i].detectorEta();
            const double jeserr = getJESSigma(L5_CORRECTOR, jetPtCut, eta);
            tfVec.push_back(tfset.getJetTf(jetPtCut, jeserr));
            effVec.push_back(tfset.getJetEff(jetPtCut, jeserr));
        }

        // Form proper tf and eff arguments for the subsequent
        // integration function call
        BinnedJetPtFcn<JetPtTF>* tfs[nJets];
        BinnedJetPtFcn<JetPtEff>* effs[nJets];
        for (unsigned i=0; i<nJets; ++i)
        {
            tfs[i] = &*tfVec[i];
            effs[i] = &*effVec[i];
        }

        // Object to store the integration results
        JesIntegResult result;

        // Integration convergence checker
        WallClockTimeLimit convCheck(maxSeconds);

        // Filter for the parton Pt priors
        const double minPartonPt = 5.0;
        const double maxPartonPt = 0.7*eBeam;
        HistoAxis ax((log(maxPartonPt)-log(minPartonPt))*1000,
                     log(minPartonPt), log(maxPartonPt));
        CPP11_auto_ptr<LocalPolyFilter1D> filter = symbetaLOrPEFilter1D(
            -1, 0.1, 0.0, ax.nBins(), ax.min(), ax.max(),
            BoundaryHandling("BM_TRUNCATE"));

        // Call the main integration function
        integrateW4Jets_m(&pythia,pars,0xffffffff, reportInterval,
                        allowTau, allowDirectDecay, eBeam,
                        nominalWmass, nominalWwidth,
                        maxWmassForScan, nWmassScanPoints,
                        jets, lepton, leptonCharge, isMuon,
                        centralValueOfTotalPt, minPartonPtFactor,
                        tfs, effs,
                        minDeltaJES, maxDeltaJES, deltaJesSteps,
                        Uniform1D(0.0, eBeam), *filter, ax,
                        tfset.getTauTF(isMuon),
                        tfset.getSystemPtPrior(), qmc, convCheck,
                        firstPowerOfTwoToStore, &result);

        // Save the result in the archive
        ostringstream os;
        os << "Event " << iev;
        ar << Record(result, os.str(), "");
    }

    // Write output archive to disk
    if (writeCompressedStringArchiveExt(ar, outFile.c_str()))
    {
        cout << "Wrote output file " << outFile << endl;
        return 0;
    }
    else
    {
        cout << "Failed to write output file " << outFile << endl;
        return 1;
    }
}
