#include <cstdio>
#include <sstream>
#include <fenv.h>
// #include <signal.h>

#include "tcl.h"
#include "histoscope.h"

#include "packStringToHSItem.hh"

#include "JetCorrectionsInterface.hh"

#include "integrateW4Jets.hh"
#include "QMCConvChecker.hh"
#include "getJESSigma.hh"

#include "geners/GenericIO.hh"
#include "npstat/stat/Distributions1D.hh"

/* Code which sets up the integration parameters */
#include "integration_parameters.cc"

#define L5_CORRECTOR 0U

using namespace std;

/* Various local variables which will be preserved between rows */
static int evcount, last_result_id, passcount, uid;
static Tcl_Interp *runInterp = NULL;

#include "Pythia8/Pythia.h"
#include "Parameters_sm_no_b_mass.h"
#include "madgraph.h"

using namespace Pythia8;

static Pythia pythia;
static Parameters_sm_no_b_mass * pars;

#ifdef __cplusplus
extern "C" {
#endif

int _hs_init(Tcl_Interp *interp)
{
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
    
    masses_.mdl_mh  = pars->mdl_MH;
    masses_.mdl_mz  = pars->mdl_MZ;
    masses_.mdl_mt  = pars->mdl_MT;
    masses_.mdl_mw  = pars->mdl_MW;
    masses_.mdl_mta = pars->mdl_MTA;

    //    cout << masses_.mdl_mh << " " << masses_.mdl_mta << endl;

    widths_.mdl_wz  = pars->mdl_WZ;
    widths_.mdl_wh  = pars->mdl_WH;
    widths_.mdl_ww  = pars->mdl_WW;
    widths_.mdl_wt  = pars->mdl_WT;

    couplings_.gc_100 = pars->GC_100;

    to_pdf_.pdlabel[0]='c';
    to_pdf_.pdlabel[1]='t';
    to_pdf_.pdlabel[2]='e';
    to_pdf_.pdlabel[3]='q';
    to_pdf_.pdlabel[4]='5';
    to_pdf_.pdlabel[5]='l';
    to_pdf_.pdlabel[6]='1';
    pdfwrap_();

    //    cout << couplings_.gc_100 << endl;
    int pdg1 = 2212;
    int pdg2 = 2;
    double x = .05;
    double Q = masses_.mdl_mw;
    cout << pdg2pdf_(pdg1,pdg2,x,Q) << endl;

    feenableexcept(FE_DIVBYZERO);
    // signal(SIGFPE, handler);

    return TCL_OK;
}

/* The following function will be called first */
int hs_ntuple_scan_init(Tcl_Interp *interp, int /* ntuple_id */,
                        const char *some_string)
{
    static bool perform_parameter_setup = true;

    /* Setup the integration parameters */
    if (perform_parameter_setup)
    {
        if (setup_integration_parameters(interp, some_string) != TCL_OK)
            return TCL_ERROR;
        uid = 0;
        perform_parameter_setup = false;
    }

    /* Initialize internal structures */
    evcount = 0;
    last_result_id = 0;
    passcount = 0;
    runInterp = interp;

    return TCL_OK;
}

/* The function below will be called at the end of a successful scan */
int hs_ntuple_scan_conclude(Tcl_Interp *interp, int /* ntuple_id */,
                            const char* /* some_string */)
{
    // assert(last_result_id > 0);
    Tcl_SetObjResult(interp, Tcl_NewIntObj(last_result_id));
    return TCL_OK;
}

/* The following function will be called for every ntuple row.
 * Scanning can be terminated at any time by returning TCL_ERROR.
 */
int hs_ntuple_scan_function(Tcl_Interp *interp, const float *row_data)
{
    typedef CPP11_shared_ptr<BinnedJetPtFcn<JetPtTF> > TFPtr;
    typedef CPP11_shared_ptr<BinnedJetPtFcn<JetPtEff> > EffPtr;

    assert(qmc);
    assert(tfset);

NTUPLE_UNPACKING_CODE

    if (evcount < maxevents)
    {
        if (evcount >= skipevents)
        {
	    ++passcount;

	    /* Print basic event id. Useful in case we die
             * inside the integration code.
             */
            cout << time_stamp() << " processing row "
                 << (int)thisRow << " run " << (int)runNumber
                 << " event " << 65536*(int)eventNum0 + (int)eventNum1 << '\n';
            cout.flush();

            /* Initialize the jet corrections. For details, see */
            /* http://www-cdf.fnal.gov/internal/physics/top/jets/example.html#example_ac++ */
            {
                const int nvtx = (int)ev.numVtxQual12;
                const int conesize = 0;  /* 0=0.4, 1=0.7 and 2=1.0             */
                const int version = 5;   /*  5   = Gen 5 (data or Monte Carlo);
                                          *  3,4 = 4.11.2 (data);
                                          *  2   = 4.9.1 (data);
                                          *  1 = do not use;
                                          *  0=Monte Carlo
                                          */
                const int syscode = 0;   /* Default values                     */
                const int nrun = (int)runNumber;
                const int mode = (hepg.status == 1.f ? 1 : 0); /* Use 1 for data, 0 for MC */

                init_generic_corrections(L5_CORRECTOR, 5, nvtx, conesize,
                                         version, syscode, nrun, mode);
            }

            // Fill out the jet information
            const unsigned nJetsExpected = 4;
            JetInfo jetinfos[nJetsExpected];

            // Assumption: we are using tight jets only
            const unsigned nTightJets = (unsigned)ev.nTightJet;
            if (nTightJets < nJetsExpected)
            {
                char temp[128];
                sprintf(temp, "Can't process events with %d tight jets", nTightJets);
                Tcl_SetResult(interp, temp, TCL_VOLATILE);
                return TCL_ERROR;
            }

            const unsigned maxJetsSaved = sizeof(jet)/sizeof(jet[0]);
            const unsigned njets = (unsigned)ev.nAllJet < maxJetsSaved ? 
                                   (unsigned)ev.nAllJet : maxJetsSaved;
            unsigned ifill = 0;
            for (unsigned i=0; i<njets && ifill<nJetsExpected; ++i)
                if (jet[i].isTight > 0.5f)
                {
                    const double pt0 = sqrt(jet[i].p.x*jet[i].p.x + jet[i].p.y*jet[i].p.y);
                    const double eta = jet[i].etaDet;
                    // const float syserr = generic_correction_uncertainty(
                    //    L5_CORRECTOR, pt0, eta, 1U);

                    /* Make sure that the jet Pt is consistent with the cut */
                    const double jetPt = pt0*jet[i].scale5;
                    assert(jetPt >= jetPtCut*0.999999);

                    const double syserr = getJESSigma(L5_CORRECTOR, jetPt, eta);
                    const double delta = jetPt*0.001;
                    const double sysplus = getJESSigma(L5_CORRECTOR, jetPt+delta, eta);
                    const double sysminus = getJESSigma(L5_CORRECTOR, jetPt-delta, eta);
                    const double sysderi = (sysplus - sysminus)/2.0/delta;

                    rk::P4 p4(geom3::Vector3(jet[i].p.x, jet[i].p.y, jet[i].p.z), jet[i].mass);
                    jetinfos[ifill++] = JetInfo(jet[i].scale5*p4, eta, syserr, sysderi);
                }
            assert(ifill == nJetsExpected);

            // Fill out the lepton information
            const rk::P4 lepton(geom3::Vector3(ev.l.x, ev.l.y, ev.l.z), 0.0);
            const int leptonCharge = (int)ev.leptonCharge;
            const bool isMuon = (int)ev.leptonType;

            // Assume that the system Pt is centered at (0, 0)
            const geom3::Vector3 centralValueOfTotalPt(0., 0., 0.);

            // Build the transfer functions. It has to be done here
            // rather than outside of the event cycle because JES
            // uncertainty is eta-dependent (even for the detector
            // jet pt cut) and, therefore, cannot be defined until
            // the jet eta values are known.
            vector<TFPtr> tfVec;
            vector<EffPtr> effVec;
            tfVec.reserve(nJetsExpected);
            effVec.reserve(nJetsExpected);
            for (unsigned i=0; i<nJetsExpected; ++i)
            {
                const double eta = jetinfos[i].detectorEta();
                const double jeserr = getJESSigma(L5_CORRECTOR, jetPtCut, eta);
                tfVec.push_back(tfset->getJetTf(jetPtCut, jeserr));
                effVec.push_back(tfset->getJetEff(jetPtCut, jeserr));
            }

            // Form proper tf and eff arguments for the subsequent
            // integration function call
            BinnedJetPtFcn<JetPtTF>* tfs[nJetsExpected];
            BinnedJetPtFcn<JetPtEff>* effs[nJetsExpected];
            for (unsigned i=0; i<nJetsExpected; ++i)
            {
                tfs[i] = &*tfVec[i];
                effs[i] = &*effVec[i];
            }

            // Object to store the integration result
            JesIntegResult result;

            // Integration convergence checker
            // WallClockTimeLimit convCheck(maxSeconds);

            QMCConvChecker convCheck(maxSeconds, deltaJesSteps, reltol,
                                     per_requirement, qmcuncert_m_min,
                                     qmcuncert_l_star, qmcuncert_r_lag);

            // Pareto distribution which can be used for
            // parton Pt prior
            const npstat::Pareto1D paretoPrior(0.0, 1.0, priorPowerParam > 0.0 ?
                                               priorPowerParam : 1.0);

            // Axis to use for the parton pt priors
            const double logPtMin = log(jetPtCut*minPartonPtFactor);
            const double logPtMax = log(maxPartonPt);
            const npstat::HistoAxis ax((logPtMax-logPtMin)*1000, logPtMin, logPtMax);

            // Filter for the parton pt priors
            if (!priorFilter)
            {
                CPP11_auto_ptr<npstat::LocalPolyFilter1D> filter =
                    npstat::symbetaLOrPEFilter1D(
                        -1, priorBandwidth, 0.0, ax.nBins(), ax.min(), ax.max(),
                        npstat::BoundaryHandling("BM_TRUNCATE"));
                priorFilter = filter.release();
            }

            // Call the main integration function
            integrateW4Jets_m(&pythia,pars,integrationMode, reportInterval,
                            allowTau, allowDirectDecay, eBeam,
                            nominalWmass, nominalWwidth,
                            maxWmassForScan, nWmassScanPoints,
                            jetinfos, lepton, leptonCharge, isMuon,
                            centralValueOfTotalPt, minPartonPtFactor,
                            tfs, effs,
                            minDeltaJES, maxDeltaJES, deltaJesSteps,
                            priorPowerParam > 0.0 ? paretoPrior : tfset->getPartonPtPrior(),
                            *priorFilter, ax, tfset->getTauTF(isMuon),
                            tfset->getSystemPtPrior(), *qmc, convCheck,
                            firstPowerOfTwoToStore, &result,
                            rawFcn, saveJetPriors);
            result.uid = (int)thisRow;

            // Convert the integration result into an HS item
            {
                ostringstream os;
                gs::write_item(os, result);

                char title[100];
                sprintf(title, "W4jets scan ntuple %d", evcount);
                const int id = packStringToHSItem(
                    uid++, title, category.c_str(), os.str());
                if (id <= 0)
                {
                    Tcl_AppendResult(interp, "failed to create scan ntuple", NULL);
                    return TCL_ERROR;
                }
                last_result_id = id;
            }

            // If necessary, write out the raw function values
            if (rawFcn)
            {
                assert(rawFcnStream);
                *rawFcnStream << "#\n"
                              << "# count " << passcount
                              << " row " << (int)thisRow
                              << " run " << (int)runNumber
                              << " event " << 65536*(int)eventNum0 + (int)eventNum1 << '\n'
                              << "#\n";
                npstat::dumpNtupleAsText(*rawFcn, *rawFcnStream);
                rawFcnStream->flush();
            }

            // Execute the periodic script
	    if (!periodic_script.empty() && script_period > 0 &&
                passcount % script_period == 0)
		if (Tcl_GlobalEval(runInterp, periodic_script.c_str()) != TCL_OK)
		    return TCL_ERROR;
        }
	++evcount;
    }

    return TCL_OK;
}

void _hs_fini(void)
{
    cleanup_integration_parameters();
}

#ifdef __cplusplus
}
#endif
