/* Auto-generated from integ_template_w4jets.cc. Do not edit by hand. */
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

#ifndef DO_NOT_DECLARE_NTUPLE_PACKING_STRUCTURE
    struct _1_aux {
        struct _2_aux_bhad {
            float emf;
            float etaDet;
            float isTight;
            float Lxy;
            float nConeTracks;
            float negBin;
            float negRate;
            float posBin;
            float posRate;
            float sigmaLxy;
            float SVXtag;
        } bhad;
        struct _3_aux_blep {
            float emf;
            float etaDet;
            float isTight;
            float Lxy;
            float nConeTracks;
            float negBin;
            float negRate;
            float posBin;
            float posRate;
            float sigmaLxy;
            float SVXtag;
        } blep;
        struct _4_aux_q {
            float emf;
            float etaDet;
            float isTight;
            float Lxy;
            float nConeTracks;
            float negBin;
            float negRate;
            float posBin;
            float posRate;
            float sigmaLxy;
            float SVXtag;
        } q;
        struct _5_aux_qbar {
            float emf;
            float etaDet;
            float isTight;
            float Lxy;
            float nConeTracks;
            float negBin;
            float negRate;
            float posBin;
            float posRate;
            float sigmaLxy;
            float SVXtag;
        } qbar;
    } aux;
    struct _6_ev {
        float apl;
        float aplTopAna;
        float code;
        float DR;
        float hadSumEt;
        float Htz;
        float instLumi;
        float isCosmic;
        float isZ;
        float jetZVertex;
        float jetZVertexError;
        struct _7_ev_l {
            float x;
            float y;
            float z;
        } l;
        float lepjetsHT;
        float leptonAxSeg;
        float leptonCharge;
        float leptonConvCode;
        float leptonCutCode;
        float leptonFiducial;
        float leptonIso;
        float leptonIsPhoenix;
        float leptonStSeg;
        float leptonType;
        float lpt;
        float missingEt;
        float missingEtPhi;
        float nAllJet;
        float nBtags;
        float nLooseEle;
        float nLooseJet;
        float nLooseJetTopSum;
        float nLooseMu;
        float nTightJet;
        float nTightJetTopSum;
        float numVtxQual12;
        float otherBtags;
        float sphTopAna;
        float tightLepZ0;
        struct _8_ev_xunscal {
            float x;
            float y;
        } xunscal;
    } ev;
    float eventNum0;
    float eventNum1;
    struct _9_hepg {
        struct _10_hepg_bhad {
            float x;
            float y;
            float z;
        } bhad;
        struct _11_hepg_bhad0 {
            float x;
            float y;
            float z;
        } bhad0;
        struct _12_hepg_bhad2 {
            float x;
            float y;
            float z;
        } bhad2;
        struct _13_hepg_bhad5 {
            float x;
            float y;
            float z;
        } bhad5;
        struct _14_hepg_bhad9 {
            float x;
            float y;
            float z;
        } bhad9;
        struct _15_hepg_blep {
            float x;
            float y;
            float z;
        } blep;
        struct _16_hepg_blep0 {
            float x;
            float y;
            float z;
        } blep0;
        struct _17_hepg_blep2 {
            float x;
            float y;
            float z;
        } blep2;
        struct _18_hepg_blep5 {
            float x;
            float y;
            float z;
        } blep5;
        struct _19_hepg_blep9 {
            float x;
            float y;
            float z;
        } blep9;
        float coned0;
        float coned1;
        float coned2;
        float coned5;
        float cosa0;
        float cosa1;
        float cosa2;
        float cosa5;
        struct _20_hepg_dinu {
            float x;
            float y;
            float z;
        } dinu;
        float generator;
        float hadstatus;
        float idhep_q;
        float idhep_q0;
        float idhep_qbar;
        float idhep_qbar0;
        struct _21_hepg_isr {
            float e;
            float mult;
            struct _22_hepg_isr_p {
                float x;
                float y;
                float z;
            } p;
        } isr[3];
        struct _23_hepg_l {
            float x;
            float y;
            float z;
        } l;
        float lcharge;
        float lepstatus;
        float ltype;
        float matchbhad;
        float matchblep;
        float matchq;
        float matchqbar;
        float mbhad;
        float mbhad0;
        float mbhad2;
        float mbhad5;
        float mbhad9;
        float mblep;
        float mblep0;
        float mblep2;
        float mblep5;
        float mblep9;
        float mdinu;
        float mq;
        float mq0;
        float mq1;
        float mq2;
        float mq5;
        float mq9;
        float mqbar;
        float mqbar0;
        float mqbar1;
        float mqbar2;
        float mqbar5;
        float mqbar9;
        float mthad;
        float mthad2;
        float mthad5;
        float mtlep;
        float mtlep2;
        float mtlep5;
        float mwhad;
        float mwhad0;
        float mwhad2;
        float mwhad5;
        float mwlep;
        float mwlep0;
        float mwlep2;
        float mwlep5;
        float nCentralIsrPartons;
        float nIsrClusters;
        float nIsrPartons;
        struct _24_hepg_nu {
            float x;
            float y;
            float z;
        } nu;
        float param2;
        float param5;
        float parmod2;
        float parmod5;
        float passbmatch;
        float passmatch;
        float passwmatch;
        float pq2;
        float pq5;
        float pqbar2;
        float pqbar5;
        float ptq2;
        float ptq5;
        float ptqbar2;
        float ptqbar5;
        struct _25_hepg_q {
            float x;
            float y;
            float z;
        } q;
        struct _26_hepg_q0 {
            float x;
            float y;
            float z;
        } q0;
        struct _27_hepg_q1 {
            float x;
            float y;
            float z;
        } q1;
        struct _28_hepg_q2 {
            float x;
            float y;
            float z;
        } q2;
        struct _29_hepg_q5 {
            float x;
            float y;
            float z;
        } q5;
        struct _30_hepg_q9 {
            float x;
            float y;
            float z;
        } q9;
        struct _31_hepg_qbar {
            float x;
            float y;
            float z;
        } qbar;
        struct _32_hepg_qbar0 {
            float x;
            float y;
            float z;
        } qbar0;
        struct _33_hepg_qbar1 {
            float x;
            float y;
            float z;
        } qbar1;
        struct _34_hepg_qbar2 {
            float x;
            float y;
            float z;
        } qbar2;
        struct _35_hepg_qbar5 {
            float x;
            float y;
            float z;
        } qbar5;
        struct _36_hepg_qbar9 {
            float x;
            float y;
            float z;
        } qbar9;
        float status;
        struct _37_hepg_thad {
            float x;
            float y;
            float z;
        } thad;
        struct _38_hepg_tlep {
            float x;
            float y;
            float z;
        } tlep;
        struct _39_hepg_whad {
            float x;
            float y;
            float z;
        } whad;
        struct _40_hepg_whad0 {
            float x;
            float y;
            float z;
        } whad0;
        struct _41_hepg_wlep {
            float x;
            float y;
            float z;
        } wlep;
        struct _42_hepg_wlep0 {
            float x;
            float y;
            float z;
        } wlep0;
        struct _43_hepg_x {
            float x;
            float y;
            float z;
        } x;
    } hepg;
    struct _44_kin5 {
        struct _45_kin5_bhad {
            float x;
            float y;
            float z;
        } bhad;
        struct _46_kin5_blep {
            float x;
            float y;
            float z;
        } blep;
        float mbhad;
        float mblep;
        float mq;
        float mqbar;
        struct _47_kin5_q {
            float x;
            float y;
            float z;
        } q;
        struct _48_kin5_qbar {
            float x;
            float y;
            float z;
        } qbar;
    } kin5;
    struct _49_kin7 {
        struct _50_kin7_bhad {
            float x;
            float y;
            float z;
        } bhad;
        struct _51_kin7_blep {
            float x;
            float y;
            float z;
        } blep;
        float mbhad;
        float mblep;
        float mq;
        float mqbar;
        struct _52_kin7_q {
            float x;
            float y;
            float z;
        } q;
        struct _53_kin7_qbar {
            float x;
            float y;
            float z;
        } qbar;
    } kin7;
    struct _54_match {
        struct _55_match_best {
            float chisq;
            float iperm;
        } best;
        float ibhad;
        float iblep;
        float iq;
        float iqbar;
        float isValid;
        struct _56_match_second {
            float chisq;
            float iperm;
        } second;
        struct _57_match_worst {
            float chisq;
            float iperm;
        } worst;
    } match;
    float runNumber;
    float thisRow;
    struct _58_jet {
        float e;
        float emf;
        float etaDet;
        float etaMoment;
        float isTight;
        float Lxy;
        float mass;
        float nConeTracks;
        float negBin;
        float negRate;
        struct _59_jet_p {
            float x;
            float y;
            float z;
        } p;
        float phiMoment;
        float posBin;
        float posRate;
        float scale4;
        float scale5;
        float scale7;
        float sigmaLxy;
        float SVXtag;
    } jet[20];
    struct _60_pmatch {
        float bestJet;
        float bestJetDEta;
        float bestJetDPhi;
        float bestJetDr;
        float bestReverseDr;
        float drToClosestParton;
        float etaDet;
        float isAmbiguous;
        float isValid;
        float lepDr;
        float partonEta;
        float partonMass;
        float partonPt;
        float ptRatio5;
        float secondBestJetDr;
    } pmatch[4];
    int ntuple_column;
#endif /* DO_NOT_DECLARE_NTUPLE_PACKING_STRUCTURE */

    ntuple_column = 0;
    runNumber = row_data[ntuple_column++];
    eventNum0 = row_data[ntuple_column++];
    eventNum1 = row_data[ntuple_column++];
    ev.code = row_data[ntuple_column++];
    ev.isZ = row_data[ntuple_column++];
    ev.isCosmic = row_data[ntuple_column++];
    ev.nTightJetTopSum = row_data[ntuple_column++];
    ev.nLooseJetTopSum = row_data[ntuple_column++];
    ev.nAllJet = row_data[ntuple_column++];
    ev.nTightJet = row_data[ntuple_column++];
    ev.nLooseJet = row_data[ntuple_column++];
    ev.nLooseEle = row_data[ntuple_column++];
    ev.nLooseMu = row_data[ntuple_column++];
    ev.jetZVertex = row_data[ntuple_column++];
    ev.jetZVertexError = row_data[ntuple_column++];
    ev.instLumi = row_data[ntuple_column++];
    ev.l.x = row_data[ntuple_column++];
    ev.l.y = row_data[ntuple_column++];
    ev.l.z = row_data[ntuple_column++];
    ev.lpt = row_data[ntuple_column++];
    ev.leptonType = row_data[ntuple_column++];
    ev.leptonIsPhoenix = row_data[ntuple_column++];
    ev.leptonCharge = row_data[ntuple_column++];
    ev.leptonIso = row_data[ntuple_column++];
    ev.tightLepZ0 = row_data[ntuple_column++];
    ev.leptonCutCode = row_data[ntuple_column++];
    ev.leptonConvCode = row_data[ntuple_column++];
    ev.leptonAxSeg = row_data[ntuple_column++];
    ev.leptonStSeg = row_data[ntuple_column++];
    ev.leptonFiducial = row_data[ntuple_column++];
    ev.xunscal.x = row_data[ntuple_column++];
    ev.xunscal.y = row_data[ntuple_column++];
    ev.nBtags = row_data[ntuple_column++];
    ev.otherBtags = row_data[ntuple_column++];
    ev.numVtxQual12 = row_data[ntuple_column++];
    ev.missingEt = row_data[ntuple_column++];
    ev.missingEtPhi = row_data[ntuple_column++];
    ev.lepjetsHT = row_data[ntuple_column++];
    ev.hadSumEt = row_data[ntuple_column++];
    ev.aplTopAna = row_data[ntuple_column++];
    ev.sphTopAna = row_data[ntuple_column++];
    ev.apl = row_data[ntuple_column++];
    ev.DR = row_data[ntuple_column++];
    ev.Htz = row_data[ntuple_column++];
    hepg.status = row_data[ntuple_column++];
    hepg.generator = row_data[ntuple_column++];
    hepg.ltype = row_data[ntuple_column++];
    hepg.lcharge = row_data[ntuple_column++];
    hepg.l.x = row_data[ntuple_column++];
    hepg.l.y = row_data[ntuple_column++];
    hepg.l.z = row_data[ntuple_column++];
    hepg.nu.x = row_data[ntuple_column++];
    hepg.nu.y = row_data[ntuple_column++];
    hepg.nu.z = row_data[ntuple_column++];
    hepg.q.x = row_data[ntuple_column++];
    hepg.q.y = row_data[ntuple_column++];
    hepg.q.z = row_data[ntuple_column++];
    hepg.mq = row_data[ntuple_column++];
    hepg.qbar.x = row_data[ntuple_column++];
    hepg.qbar.y = row_data[ntuple_column++];
    hepg.qbar.z = row_data[ntuple_column++];
    hepg.mqbar = row_data[ntuple_column++];
    hepg.blep.x = row_data[ntuple_column++];
    hepg.blep.y = row_data[ntuple_column++];
    hepg.blep.z = row_data[ntuple_column++];
    hepg.mblep = row_data[ntuple_column++];
    hepg.bhad.x = row_data[ntuple_column++];
    hepg.bhad.y = row_data[ntuple_column++];
    hepg.bhad.z = row_data[ntuple_column++];
    hepg.mbhad = row_data[ntuple_column++];
    hepg.wlep.x = row_data[ntuple_column++];
    hepg.wlep.y = row_data[ntuple_column++];
    hepg.wlep.z = row_data[ntuple_column++];
    hepg.mwlep = row_data[ntuple_column++];
    hepg.whad.x = row_data[ntuple_column++];
    hepg.whad.y = row_data[ntuple_column++];
    hepg.whad.z = row_data[ntuple_column++];
    hepg.mwhad = row_data[ntuple_column++];
    hepg.tlep.x = row_data[ntuple_column++];
    hepg.tlep.y = row_data[ntuple_column++];
    hepg.tlep.z = row_data[ntuple_column++];
    hepg.mtlep = row_data[ntuple_column++];
    hepg.thad.x = row_data[ntuple_column++];
    hepg.thad.y = row_data[ntuple_column++];
    hepg.thad.z = row_data[ntuple_column++];
    hepg.mthad = row_data[ntuple_column++];
    hepg.x.x = row_data[ntuple_column++];
    hepg.x.y = row_data[ntuple_column++];
    hepg.x.z = row_data[ntuple_column++];
    hepg.idhep_q = row_data[ntuple_column++];
    hepg.idhep_qbar = row_data[ntuple_column++];
    hepg.q0.x = row_data[ntuple_column++];
    hepg.q0.y = row_data[ntuple_column++];
    hepg.q0.z = row_data[ntuple_column++];
    hepg.mq0 = row_data[ntuple_column++];
    hepg.qbar0.x = row_data[ntuple_column++];
    hepg.qbar0.y = row_data[ntuple_column++];
    hepg.qbar0.z = row_data[ntuple_column++];
    hepg.mqbar0 = row_data[ntuple_column++];
    hepg.blep0.x = row_data[ntuple_column++];
    hepg.blep0.y = row_data[ntuple_column++];
    hepg.blep0.z = row_data[ntuple_column++];
    hepg.mblep0 = row_data[ntuple_column++];
    hepg.bhad0.x = row_data[ntuple_column++];
    hepg.bhad0.y = row_data[ntuple_column++];
    hepg.bhad0.z = row_data[ntuple_column++];
    hepg.mbhad0 = row_data[ntuple_column++];
    hepg.wlep0.x = row_data[ntuple_column++];
    hepg.wlep0.y = row_data[ntuple_column++];
    hepg.wlep0.z = row_data[ntuple_column++];
    hepg.mwlep0 = row_data[ntuple_column++];
    hepg.whad0.x = row_data[ntuple_column++];
    hepg.whad0.y = row_data[ntuple_column++];
    hepg.whad0.z = row_data[ntuple_column++];
    hepg.mwhad0 = row_data[ntuple_column++];
    hepg.idhep_q0 = row_data[ntuple_column++];
    hepg.idhep_qbar0 = row_data[ntuple_column++];
    hepg.passmatch = row_data[ntuple_column++];
    hepg.passbmatch = row_data[ntuple_column++];
    hepg.passwmatch = row_data[ntuple_column++];
    hepg.matchq = row_data[ntuple_column++];
    hepg.matchqbar = row_data[ntuple_column++];
    hepg.matchblep = row_data[ntuple_column++];
    hepg.matchbhad = row_data[ntuple_column++];
    jet[0].p.x = row_data[ntuple_column++];
    jet[1].p.x = row_data[ntuple_column++];
    jet[2].p.x = row_data[ntuple_column++];
    jet[3].p.x = row_data[ntuple_column++];
    jet[4].p.x = row_data[ntuple_column++];
    jet[5].p.x = row_data[ntuple_column++];
    jet[6].p.x = row_data[ntuple_column++];
    jet[7].p.x = row_data[ntuple_column++];
    jet[8].p.x = row_data[ntuple_column++];
    jet[9].p.x = row_data[ntuple_column++];
    jet[10].p.x = row_data[ntuple_column++];
    jet[11].p.x = row_data[ntuple_column++];
    jet[12].p.x = row_data[ntuple_column++];
    jet[13].p.x = row_data[ntuple_column++];
    jet[14].p.x = row_data[ntuple_column++];
    jet[15].p.x = row_data[ntuple_column++];
    jet[16].p.x = row_data[ntuple_column++];
    jet[17].p.x = row_data[ntuple_column++];
    jet[18].p.x = row_data[ntuple_column++];
    jet[19].p.x = row_data[ntuple_column++];
    jet[0].p.y = row_data[ntuple_column++];
    jet[1].p.y = row_data[ntuple_column++];
    jet[2].p.y = row_data[ntuple_column++];
    jet[3].p.y = row_data[ntuple_column++];
    jet[4].p.y = row_data[ntuple_column++];
    jet[5].p.y = row_data[ntuple_column++];
    jet[6].p.y = row_data[ntuple_column++];
    jet[7].p.y = row_data[ntuple_column++];
    jet[8].p.y = row_data[ntuple_column++];
    jet[9].p.y = row_data[ntuple_column++];
    jet[10].p.y = row_data[ntuple_column++];
    jet[11].p.y = row_data[ntuple_column++];
    jet[12].p.y = row_data[ntuple_column++];
    jet[13].p.y = row_data[ntuple_column++];
    jet[14].p.y = row_data[ntuple_column++];
    jet[15].p.y = row_data[ntuple_column++];
    jet[16].p.y = row_data[ntuple_column++];
    jet[17].p.y = row_data[ntuple_column++];
    jet[18].p.y = row_data[ntuple_column++];
    jet[19].p.y = row_data[ntuple_column++];
    jet[0].p.z = row_data[ntuple_column++];
    jet[1].p.z = row_data[ntuple_column++];
    jet[2].p.z = row_data[ntuple_column++];
    jet[3].p.z = row_data[ntuple_column++];
    jet[4].p.z = row_data[ntuple_column++];
    jet[5].p.z = row_data[ntuple_column++];
    jet[6].p.z = row_data[ntuple_column++];
    jet[7].p.z = row_data[ntuple_column++];
    jet[8].p.z = row_data[ntuple_column++];
    jet[9].p.z = row_data[ntuple_column++];
    jet[10].p.z = row_data[ntuple_column++];
    jet[11].p.z = row_data[ntuple_column++];
    jet[12].p.z = row_data[ntuple_column++];
    jet[13].p.z = row_data[ntuple_column++];
    jet[14].p.z = row_data[ntuple_column++];
    jet[15].p.z = row_data[ntuple_column++];
    jet[16].p.z = row_data[ntuple_column++];
    jet[17].p.z = row_data[ntuple_column++];
    jet[18].p.z = row_data[ntuple_column++];
    jet[19].p.z = row_data[ntuple_column++];
    jet[0].e = row_data[ntuple_column++];
    jet[1].e = row_data[ntuple_column++];
    jet[2].e = row_data[ntuple_column++];
    jet[3].e = row_data[ntuple_column++];
    jet[4].e = row_data[ntuple_column++];
    jet[5].e = row_data[ntuple_column++];
    jet[6].e = row_data[ntuple_column++];
    jet[7].e = row_data[ntuple_column++];
    jet[8].e = row_data[ntuple_column++];
    jet[9].e = row_data[ntuple_column++];
    jet[10].e = row_data[ntuple_column++];
    jet[11].e = row_data[ntuple_column++];
    jet[12].e = row_data[ntuple_column++];
    jet[13].e = row_data[ntuple_column++];
    jet[14].e = row_data[ntuple_column++];
    jet[15].e = row_data[ntuple_column++];
    jet[16].e = row_data[ntuple_column++];
    jet[17].e = row_data[ntuple_column++];
    jet[18].e = row_data[ntuple_column++];
    jet[19].e = row_data[ntuple_column++];
    jet[0].mass = row_data[ntuple_column++];
    jet[1].mass = row_data[ntuple_column++];
    jet[2].mass = row_data[ntuple_column++];
    jet[3].mass = row_data[ntuple_column++];
    jet[4].mass = row_data[ntuple_column++];
    jet[5].mass = row_data[ntuple_column++];
    jet[6].mass = row_data[ntuple_column++];
    jet[7].mass = row_data[ntuple_column++];
    jet[8].mass = row_data[ntuple_column++];
    jet[9].mass = row_data[ntuple_column++];
    jet[10].mass = row_data[ntuple_column++];
    jet[11].mass = row_data[ntuple_column++];
    jet[12].mass = row_data[ntuple_column++];
    jet[13].mass = row_data[ntuple_column++];
    jet[14].mass = row_data[ntuple_column++];
    jet[15].mass = row_data[ntuple_column++];
    jet[16].mass = row_data[ntuple_column++];
    jet[17].mass = row_data[ntuple_column++];
    jet[18].mass = row_data[ntuple_column++];
    jet[19].mass = row_data[ntuple_column++];
    jet[0].etaDet = row_data[ntuple_column++];
    jet[1].etaDet = row_data[ntuple_column++];
    jet[2].etaDet = row_data[ntuple_column++];
    jet[3].etaDet = row_data[ntuple_column++];
    jet[4].etaDet = row_data[ntuple_column++];
    jet[5].etaDet = row_data[ntuple_column++];
    jet[6].etaDet = row_data[ntuple_column++];
    jet[7].etaDet = row_data[ntuple_column++];
    jet[8].etaDet = row_data[ntuple_column++];
    jet[9].etaDet = row_data[ntuple_column++];
    jet[10].etaDet = row_data[ntuple_column++];
    jet[11].etaDet = row_data[ntuple_column++];
    jet[12].etaDet = row_data[ntuple_column++];
    jet[13].etaDet = row_data[ntuple_column++];
    jet[14].etaDet = row_data[ntuple_column++];
    jet[15].etaDet = row_data[ntuple_column++];
    jet[16].etaDet = row_data[ntuple_column++];
    jet[17].etaDet = row_data[ntuple_column++];
    jet[18].etaDet = row_data[ntuple_column++];
    jet[19].etaDet = row_data[ntuple_column++];
    jet[0].emf = row_data[ntuple_column++];
    jet[1].emf = row_data[ntuple_column++];
    jet[2].emf = row_data[ntuple_column++];
    jet[3].emf = row_data[ntuple_column++];
    jet[4].emf = row_data[ntuple_column++];
    jet[5].emf = row_data[ntuple_column++];
    jet[6].emf = row_data[ntuple_column++];
    jet[7].emf = row_data[ntuple_column++];
    jet[8].emf = row_data[ntuple_column++];
    jet[9].emf = row_data[ntuple_column++];
    jet[10].emf = row_data[ntuple_column++];
    jet[11].emf = row_data[ntuple_column++];
    jet[12].emf = row_data[ntuple_column++];
    jet[13].emf = row_data[ntuple_column++];
    jet[14].emf = row_data[ntuple_column++];
    jet[15].emf = row_data[ntuple_column++];
    jet[16].emf = row_data[ntuple_column++];
    jet[17].emf = row_data[ntuple_column++];
    jet[18].emf = row_data[ntuple_column++];
    jet[19].emf = row_data[ntuple_column++];
    jet[0].scale4 = row_data[ntuple_column++];
    jet[1].scale4 = row_data[ntuple_column++];
    jet[2].scale4 = row_data[ntuple_column++];
    jet[3].scale4 = row_data[ntuple_column++];
    jet[4].scale4 = row_data[ntuple_column++];
    jet[5].scale4 = row_data[ntuple_column++];
    jet[6].scale4 = row_data[ntuple_column++];
    jet[7].scale4 = row_data[ntuple_column++];
    jet[8].scale4 = row_data[ntuple_column++];
    jet[9].scale4 = row_data[ntuple_column++];
    jet[10].scale4 = row_data[ntuple_column++];
    jet[11].scale4 = row_data[ntuple_column++];
    jet[12].scale4 = row_data[ntuple_column++];
    jet[13].scale4 = row_data[ntuple_column++];
    jet[14].scale4 = row_data[ntuple_column++];
    jet[15].scale4 = row_data[ntuple_column++];
    jet[16].scale4 = row_data[ntuple_column++];
    jet[17].scale4 = row_data[ntuple_column++];
    jet[18].scale4 = row_data[ntuple_column++];
    jet[19].scale4 = row_data[ntuple_column++];
    jet[0].scale5 = row_data[ntuple_column++];
    jet[1].scale5 = row_data[ntuple_column++];
    jet[2].scale5 = row_data[ntuple_column++];
    jet[3].scale5 = row_data[ntuple_column++];
    jet[4].scale5 = row_data[ntuple_column++];
    jet[5].scale5 = row_data[ntuple_column++];
    jet[6].scale5 = row_data[ntuple_column++];
    jet[7].scale5 = row_data[ntuple_column++];
    jet[8].scale5 = row_data[ntuple_column++];
    jet[9].scale5 = row_data[ntuple_column++];
    jet[10].scale5 = row_data[ntuple_column++];
    jet[11].scale5 = row_data[ntuple_column++];
    jet[12].scale5 = row_data[ntuple_column++];
    jet[13].scale5 = row_data[ntuple_column++];
    jet[14].scale5 = row_data[ntuple_column++];
    jet[15].scale5 = row_data[ntuple_column++];
    jet[16].scale5 = row_data[ntuple_column++];
    jet[17].scale5 = row_data[ntuple_column++];
    jet[18].scale5 = row_data[ntuple_column++];
    jet[19].scale5 = row_data[ntuple_column++];
    jet[0].scale7 = row_data[ntuple_column++];
    jet[1].scale7 = row_data[ntuple_column++];
    jet[2].scale7 = row_data[ntuple_column++];
    jet[3].scale7 = row_data[ntuple_column++];
    jet[4].scale7 = row_data[ntuple_column++];
    jet[5].scale7 = row_data[ntuple_column++];
    jet[6].scale7 = row_data[ntuple_column++];
    jet[7].scale7 = row_data[ntuple_column++];
    jet[8].scale7 = row_data[ntuple_column++];
    jet[9].scale7 = row_data[ntuple_column++];
    jet[10].scale7 = row_data[ntuple_column++];
    jet[11].scale7 = row_data[ntuple_column++];
    jet[12].scale7 = row_data[ntuple_column++];
    jet[13].scale7 = row_data[ntuple_column++];
    jet[14].scale7 = row_data[ntuple_column++];
    jet[15].scale7 = row_data[ntuple_column++];
    jet[16].scale7 = row_data[ntuple_column++];
    jet[17].scale7 = row_data[ntuple_column++];
    jet[18].scale7 = row_data[ntuple_column++];
    jet[19].scale7 = row_data[ntuple_column++];
    jet[0].SVXtag = row_data[ntuple_column++];
    jet[1].SVXtag = row_data[ntuple_column++];
    jet[2].SVXtag = row_data[ntuple_column++];
    jet[3].SVXtag = row_data[ntuple_column++];
    jet[4].SVXtag = row_data[ntuple_column++];
    jet[5].SVXtag = row_data[ntuple_column++];
    jet[6].SVXtag = row_data[ntuple_column++];
    jet[7].SVXtag = row_data[ntuple_column++];
    jet[8].SVXtag = row_data[ntuple_column++];
    jet[9].SVXtag = row_data[ntuple_column++];
    jet[10].SVXtag = row_data[ntuple_column++];
    jet[11].SVXtag = row_data[ntuple_column++];
    jet[12].SVXtag = row_data[ntuple_column++];
    jet[13].SVXtag = row_data[ntuple_column++];
    jet[14].SVXtag = row_data[ntuple_column++];
    jet[15].SVXtag = row_data[ntuple_column++];
    jet[16].SVXtag = row_data[ntuple_column++];
    jet[17].SVXtag = row_data[ntuple_column++];
    jet[18].SVXtag = row_data[ntuple_column++];
    jet[19].SVXtag = row_data[ntuple_column++];
    jet[0].Lxy = row_data[ntuple_column++];
    jet[1].Lxy = row_data[ntuple_column++];
    jet[2].Lxy = row_data[ntuple_column++];
    jet[3].Lxy = row_data[ntuple_column++];
    jet[4].Lxy = row_data[ntuple_column++];
    jet[5].Lxy = row_data[ntuple_column++];
    jet[6].Lxy = row_data[ntuple_column++];
    jet[7].Lxy = row_data[ntuple_column++];
    jet[8].Lxy = row_data[ntuple_column++];
    jet[9].Lxy = row_data[ntuple_column++];
    jet[10].Lxy = row_data[ntuple_column++];
    jet[11].Lxy = row_data[ntuple_column++];
    jet[12].Lxy = row_data[ntuple_column++];
    jet[13].Lxy = row_data[ntuple_column++];
    jet[14].Lxy = row_data[ntuple_column++];
    jet[15].Lxy = row_data[ntuple_column++];
    jet[16].Lxy = row_data[ntuple_column++];
    jet[17].Lxy = row_data[ntuple_column++];
    jet[18].Lxy = row_data[ntuple_column++];
    jet[19].Lxy = row_data[ntuple_column++];
    jet[0].sigmaLxy = row_data[ntuple_column++];
    jet[1].sigmaLxy = row_data[ntuple_column++];
    jet[2].sigmaLxy = row_data[ntuple_column++];
    jet[3].sigmaLxy = row_data[ntuple_column++];
    jet[4].sigmaLxy = row_data[ntuple_column++];
    jet[5].sigmaLxy = row_data[ntuple_column++];
    jet[6].sigmaLxy = row_data[ntuple_column++];
    jet[7].sigmaLxy = row_data[ntuple_column++];
    jet[8].sigmaLxy = row_data[ntuple_column++];
    jet[9].sigmaLxy = row_data[ntuple_column++];
    jet[10].sigmaLxy = row_data[ntuple_column++];
    jet[11].sigmaLxy = row_data[ntuple_column++];
    jet[12].sigmaLxy = row_data[ntuple_column++];
    jet[13].sigmaLxy = row_data[ntuple_column++];
    jet[14].sigmaLxy = row_data[ntuple_column++];
    jet[15].sigmaLxy = row_data[ntuple_column++];
    jet[16].sigmaLxy = row_data[ntuple_column++];
    jet[17].sigmaLxy = row_data[ntuple_column++];
    jet[18].sigmaLxy = row_data[ntuple_column++];
    jet[19].sigmaLxy = row_data[ntuple_column++];
    jet[0].isTight = row_data[ntuple_column++];
    jet[1].isTight = row_data[ntuple_column++];
    jet[2].isTight = row_data[ntuple_column++];
    jet[3].isTight = row_data[ntuple_column++];
    jet[4].isTight = row_data[ntuple_column++];
    jet[5].isTight = row_data[ntuple_column++];
    jet[6].isTight = row_data[ntuple_column++];
    jet[7].isTight = row_data[ntuple_column++];
    jet[8].isTight = row_data[ntuple_column++];
    jet[9].isTight = row_data[ntuple_column++];
    jet[10].isTight = row_data[ntuple_column++];
    jet[11].isTight = row_data[ntuple_column++];
    jet[12].isTight = row_data[ntuple_column++];
    jet[13].isTight = row_data[ntuple_column++];
    jet[14].isTight = row_data[ntuple_column++];
    jet[15].isTight = row_data[ntuple_column++];
    jet[16].isTight = row_data[ntuple_column++];
    jet[17].isTight = row_data[ntuple_column++];
    jet[18].isTight = row_data[ntuple_column++];
    jet[19].isTight = row_data[ntuple_column++];
    jet[0].nConeTracks = row_data[ntuple_column++];
    jet[1].nConeTracks = row_data[ntuple_column++];
    jet[2].nConeTracks = row_data[ntuple_column++];
    jet[3].nConeTracks = row_data[ntuple_column++];
    jet[4].nConeTracks = row_data[ntuple_column++];
    jet[5].nConeTracks = row_data[ntuple_column++];
    jet[6].nConeTracks = row_data[ntuple_column++];
    jet[7].nConeTracks = row_data[ntuple_column++];
    jet[8].nConeTracks = row_data[ntuple_column++];
    jet[9].nConeTracks = row_data[ntuple_column++];
    jet[10].nConeTracks = row_data[ntuple_column++];
    jet[11].nConeTracks = row_data[ntuple_column++];
    jet[12].nConeTracks = row_data[ntuple_column++];
    jet[13].nConeTracks = row_data[ntuple_column++];
    jet[14].nConeTracks = row_data[ntuple_column++];
    jet[15].nConeTracks = row_data[ntuple_column++];
    jet[16].nConeTracks = row_data[ntuple_column++];
    jet[17].nConeTracks = row_data[ntuple_column++];
    jet[18].nConeTracks = row_data[ntuple_column++];
    jet[19].nConeTracks = row_data[ntuple_column++];
    jet[0].posRate = row_data[ntuple_column++];
    jet[1].posRate = row_data[ntuple_column++];
    jet[2].posRate = row_data[ntuple_column++];
    jet[3].posRate = row_data[ntuple_column++];
    jet[4].posRate = row_data[ntuple_column++];
    jet[5].posRate = row_data[ntuple_column++];
    jet[6].posRate = row_data[ntuple_column++];
    jet[7].posRate = row_data[ntuple_column++];
    jet[8].posRate = row_data[ntuple_column++];
    jet[9].posRate = row_data[ntuple_column++];
    jet[10].posRate = row_data[ntuple_column++];
    jet[11].posRate = row_data[ntuple_column++];
    jet[12].posRate = row_data[ntuple_column++];
    jet[13].posRate = row_data[ntuple_column++];
    jet[14].posRate = row_data[ntuple_column++];
    jet[15].posRate = row_data[ntuple_column++];
    jet[16].posRate = row_data[ntuple_column++];
    jet[17].posRate = row_data[ntuple_column++];
    jet[18].posRate = row_data[ntuple_column++];
    jet[19].posRate = row_data[ntuple_column++];
    jet[0].posBin = row_data[ntuple_column++];
    jet[1].posBin = row_data[ntuple_column++];
    jet[2].posBin = row_data[ntuple_column++];
    jet[3].posBin = row_data[ntuple_column++];
    jet[4].posBin = row_data[ntuple_column++];
    jet[5].posBin = row_data[ntuple_column++];
    jet[6].posBin = row_data[ntuple_column++];
    jet[7].posBin = row_data[ntuple_column++];
    jet[8].posBin = row_data[ntuple_column++];
    jet[9].posBin = row_data[ntuple_column++];
    jet[10].posBin = row_data[ntuple_column++];
    jet[11].posBin = row_data[ntuple_column++];
    jet[12].posBin = row_data[ntuple_column++];
    jet[13].posBin = row_data[ntuple_column++];
    jet[14].posBin = row_data[ntuple_column++];
    jet[15].posBin = row_data[ntuple_column++];
    jet[16].posBin = row_data[ntuple_column++];
    jet[17].posBin = row_data[ntuple_column++];
    jet[18].posBin = row_data[ntuple_column++];
    jet[19].posBin = row_data[ntuple_column++];
    jet[0].negRate = row_data[ntuple_column++];
    jet[1].negRate = row_data[ntuple_column++];
    jet[2].negRate = row_data[ntuple_column++];
    jet[3].negRate = row_data[ntuple_column++];
    jet[4].negRate = row_data[ntuple_column++];
    jet[5].negRate = row_data[ntuple_column++];
    jet[6].negRate = row_data[ntuple_column++];
    jet[7].negRate = row_data[ntuple_column++];
    jet[8].negRate = row_data[ntuple_column++];
    jet[9].negRate = row_data[ntuple_column++];
    jet[10].negRate = row_data[ntuple_column++];
    jet[11].negRate = row_data[ntuple_column++];
    jet[12].negRate = row_data[ntuple_column++];
    jet[13].negRate = row_data[ntuple_column++];
    jet[14].negRate = row_data[ntuple_column++];
    jet[15].negRate = row_data[ntuple_column++];
    jet[16].negRate = row_data[ntuple_column++];
    jet[17].negRate = row_data[ntuple_column++];
    jet[18].negRate = row_data[ntuple_column++];
    jet[19].negRate = row_data[ntuple_column++];
    jet[0].negBin = row_data[ntuple_column++];
    jet[1].negBin = row_data[ntuple_column++];
    jet[2].negBin = row_data[ntuple_column++];
    jet[3].negBin = row_data[ntuple_column++];
    jet[4].negBin = row_data[ntuple_column++];
    jet[5].negBin = row_data[ntuple_column++];
    jet[6].negBin = row_data[ntuple_column++];
    jet[7].negBin = row_data[ntuple_column++];
    jet[8].negBin = row_data[ntuple_column++];
    jet[9].negBin = row_data[ntuple_column++];
    jet[10].negBin = row_data[ntuple_column++];
    jet[11].negBin = row_data[ntuple_column++];
    jet[12].negBin = row_data[ntuple_column++];
    jet[13].negBin = row_data[ntuple_column++];
    jet[14].negBin = row_data[ntuple_column++];
    jet[15].negBin = row_data[ntuple_column++];
    jet[16].negBin = row_data[ntuple_column++];
    jet[17].negBin = row_data[ntuple_column++];
    jet[18].negBin = row_data[ntuple_column++];
    jet[19].negBin = row_data[ntuple_column++];
    jet[0].etaMoment = row_data[ntuple_column++];
    jet[1].etaMoment = row_data[ntuple_column++];
    jet[2].etaMoment = row_data[ntuple_column++];
    jet[3].etaMoment = row_data[ntuple_column++];
    jet[4].etaMoment = row_data[ntuple_column++];
    jet[5].etaMoment = row_data[ntuple_column++];
    jet[6].etaMoment = row_data[ntuple_column++];
    jet[7].etaMoment = row_data[ntuple_column++];
    jet[8].etaMoment = row_data[ntuple_column++];
    jet[9].etaMoment = row_data[ntuple_column++];
    jet[10].etaMoment = row_data[ntuple_column++];
    jet[11].etaMoment = row_data[ntuple_column++];
    jet[12].etaMoment = row_data[ntuple_column++];
    jet[13].etaMoment = row_data[ntuple_column++];
    jet[14].etaMoment = row_data[ntuple_column++];
    jet[15].etaMoment = row_data[ntuple_column++];
    jet[16].etaMoment = row_data[ntuple_column++];
    jet[17].etaMoment = row_data[ntuple_column++];
    jet[18].etaMoment = row_data[ntuple_column++];
    jet[19].etaMoment = row_data[ntuple_column++];
    jet[0].phiMoment = row_data[ntuple_column++];
    jet[1].phiMoment = row_data[ntuple_column++];
    jet[2].phiMoment = row_data[ntuple_column++];
    jet[3].phiMoment = row_data[ntuple_column++];
    jet[4].phiMoment = row_data[ntuple_column++];
    jet[5].phiMoment = row_data[ntuple_column++];
    jet[6].phiMoment = row_data[ntuple_column++];
    jet[7].phiMoment = row_data[ntuple_column++];
    jet[8].phiMoment = row_data[ntuple_column++];
    jet[9].phiMoment = row_data[ntuple_column++];
    jet[10].phiMoment = row_data[ntuple_column++];
    jet[11].phiMoment = row_data[ntuple_column++];
    jet[12].phiMoment = row_data[ntuple_column++];
    jet[13].phiMoment = row_data[ntuple_column++];
    jet[14].phiMoment = row_data[ntuple_column++];
    jet[15].phiMoment = row_data[ntuple_column++];
    jet[16].phiMoment = row_data[ntuple_column++];
    jet[17].phiMoment = row_data[ntuple_column++];
    jet[18].phiMoment = row_data[ntuple_column++];
    jet[19].phiMoment = row_data[ntuple_column++];
    hepg.hadstatus = row_data[ntuple_column++];
    hepg.lepstatus = row_data[ntuple_column++];
    hepg.dinu.x = row_data[ntuple_column++];
    hepg.dinu.y = row_data[ntuple_column++];
    hepg.dinu.z = row_data[ntuple_column++];
    hepg.mdinu = row_data[ntuple_column++];
    hepg.nIsrPartons = row_data[ntuple_column++];
    hepg.nCentralIsrPartons = row_data[ntuple_column++];
    hepg.nIsrClusters = row_data[ntuple_column++];
    hepg.isr[0].p.x = row_data[ntuple_column++];
    hepg.isr[1].p.x = row_data[ntuple_column++];
    hepg.isr[2].p.x = row_data[ntuple_column++];
    hepg.isr[0].p.y = row_data[ntuple_column++];
    hepg.isr[1].p.y = row_data[ntuple_column++];
    hepg.isr[2].p.y = row_data[ntuple_column++];
    hepg.isr[0].p.z = row_data[ntuple_column++];
    hepg.isr[1].p.z = row_data[ntuple_column++];
    hepg.isr[2].p.z = row_data[ntuple_column++];
    hepg.isr[0].e = row_data[ntuple_column++];
    hepg.isr[1].e = row_data[ntuple_column++];
    hepg.isr[2].e = row_data[ntuple_column++];
    hepg.isr[0].mult = row_data[ntuple_column++];
    hepg.isr[1].mult = row_data[ntuple_column++];
    hepg.isr[2].mult = row_data[ntuple_column++];
    thisRow = row_data[ntuple_column++];
    match.best.iperm = row_data[ntuple_column++];
    match.best.chisq = row_data[ntuple_column++];
    match.second.iperm = row_data[ntuple_column++];
    match.second.chisq = row_data[ntuple_column++];
    match.worst.iperm = row_data[ntuple_column++];
    match.worst.chisq = row_data[ntuple_column++];
    match.isValid = row_data[ntuple_column++];
    match.iq = row_data[ntuple_column++];
    match.iqbar = row_data[ntuple_column++];
    match.iblep = row_data[ntuple_column++];
    match.ibhad = row_data[ntuple_column++];
    hepg.q1.x = row_data[ntuple_column++];
    hepg.q1.y = row_data[ntuple_column++];
    hepg.q1.z = row_data[ntuple_column++];
    hepg.mq1 = row_data[ntuple_column++];
    hepg.qbar1.x = row_data[ntuple_column++];
    hepg.qbar1.y = row_data[ntuple_column++];
    hepg.qbar1.z = row_data[ntuple_column++];
    hepg.mqbar1 = row_data[ntuple_column++];
    hepg.q2.x = row_data[ntuple_column++];
    hepg.q2.y = row_data[ntuple_column++];
    hepg.q2.z = row_data[ntuple_column++];
    hepg.mq2 = row_data[ntuple_column++];
    hepg.qbar2.x = row_data[ntuple_column++];
    hepg.qbar2.y = row_data[ntuple_column++];
    hepg.qbar2.z = row_data[ntuple_column++];
    hepg.mqbar2 = row_data[ntuple_column++];
    hepg.blep2.x = row_data[ntuple_column++];
    hepg.blep2.y = row_data[ntuple_column++];
    hepg.blep2.z = row_data[ntuple_column++];
    hepg.mblep2 = row_data[ntuple_column++];
    hepg.bhad2.x = row_data[ntuple_column++];
    hepg.bhad2.y = row_data[ntuple_column++];
    hepg.bhad2.z = row_data[ntuple_column++];
    hepg.mbhad2 = row_data[ntuple_column++];
    hepg.mwlep2 = row_data[ntuple_column++];
    hepg.mwhad2 = row_data[ntuple_column++];
    hepg.mtlep2 = row_data[ntuple_column++];
    hepg.mthad2 = row_data[ntuple_column++];
    hepg.pq2 = row_data[ntuple_column++];
    hepg.pqbar2 = row_data[ntuple_column++];
    hepg.ptq2 = row_data[ntuple_column++];
    hepg.ptqbar2 = row_data[ntuple_column++];
    hepg.param2 = row_data[ntuple_column++];
    hepg.parmod2 = row_data[ntuple_column++];
    hepg.q5.x = row_data[ntuple_column++];
    hepg.q5.y = row_data[ntuple_column++];
    hepg.q5.z = row_data[ntuple_column++];
    hepg.mq5 = row_data[ntuple_column++];
    hepg.qbar5.x = row_data[ntuple_column++];
    hepg.qbar5.y = row_data[ntuple_column++];
    hepg.qbar5.z = row_data[ntuple_column++];
    hepg.mqbar5 = row_data[ntuple_column++];
    hepg.blep5.x = row_data[ntuple_column++];
    hepg.blep5.y = row_data[ntuple_column++];
    hepg.blep5.z = row_data[ntuple_column++];
    hepg.mblep5 = row_data[ntuple_column++];
    hepg.bhad5.x = row_data[ntuple_column++];
    hepg.bhad5.y = row_data[ntuple_column++];
    hepg.bhad5.z = row_data[ntuple_column++];
    hepg.mbhad5 = row_data[ntuple_column++];
    hepg.mwlep5 = row_data[ntuple_column++];
    hepg.mwhad5 = row_data[ntuple_column++];
    hepg.mtlep5 = row_data[ntuple_column++];
    hepg.mthad5 = row_data[ntuple_column++];
    hepg.pq5 = row_data[ntuple_column++];
    hepg.pqbar5 = row_data[ntuple_column++];
    hepg.ptq5 = row_data[ntuple_column++];
    hepg.ptqbar5 = row_data[ntuple_column++];
    hepg.param5 = row_data[ntuple_column++];
    hepg.parmod5 = row_data[ntuple_column++];
    hepg.coned0 = row_data[ntuple_column++];
    hepg.coned1 = row_data[ntuple_column++];
    hepg.coned2 = row_data[ntuple_column++];
    hepg.coned5 = row_data[ntuple_column++];
    hepg.cosa0 = row_data[ntuple_column++];
    hepg.cosa1 = row_data[ntuple_column++];
    hepg.cosa2 = row_data[ntuple_column++];
    hepg.cosa5 = row_data[ntuple_column++];
    hepg.q9.x = row_data[ntuple_column++];
    hepg.q9.y = row_data[ntuple_column++];
    hepg.q9.z = row_data[ntuple_column++];
    hepg.mq9 = row_data[ntuple_column++];
    hepg.qbar9.x = row_data[ntuple_column++];
    hepg.qbar9.y = row_data[ntuple_column++];
    hepg.qbar9.z = row_data[ntuple_column++];
    hepg.mqbar9 = row_data[ntuple_column++];
    hepg.blep9.x = row_data[ntuple_column++];
    hepg.blep9.y = row_data[ntuple_column++];
    hepg.blep9.z = row_data[ntuple_column++];
    hepg.mblep9 = row_data[ntuple_column++];
    hepg.bhad9.x = row_data[ntuple_column++];
    hepg.bhad9.y = row_data[ntuple_column++];
    hepg.bhad9.z = row_data[ntuple_column++];
    hepg.mbhad9 = row_data[ntuple_column++];
    kin5.q.x = row_data[ntuple_column++];
    kin5.q.y = row_data[ntuple_column++];
    kin5.q.z = row_data[ntuple_column++];
    kin5.mq = row_data[ntuple_column++];
    kin5.qbar.x = row_data[ntuple_column++];
    kin5.qbar.y = row_data[ntuple_column++];
    kin5.qbar.z = row_data[ntuple_column++];
    kin5.mqbar = row_data[ntuple_column++];
    kin5.blep.x = row_data[ntuple_column++];
    kin5.blep.y = row_data[ntuple_column++];
    kin5.blep.z = row_data[ntuple_column++];
    kin5.mblep = row_data[ntuple_column++];
    kin5.bhad.x = row_data[ntuple_column++];
    kin5.bhad.y = row_data[ntuple_column++];
    kin5.bhad.z = row_data[ntuple_column++];
    kin5.mbhad = row_data[ntuple_column++];
    kin7.q.x = row_data[ntuple_column++];
    kin7.q.y = row_data[ntuple_column++];
    kin7.q.z = row_data[ntuple_column++];
    kin7.mq = row_data[ntuple_column++];
    kin7.qbar.x = row_data[ntuple_column++];
    kin7.qbar.y = row_data[ntuple_column++];
    kin7.qbar.z = row_data[ntuple_column++];
    kin7.mqbar = row_data[ntuple_column++];
    kin7.blep.x = row_data[ntuple_column++];
    kin7.blep.y = row_data[ntuple_column++];
    kin7.blep.z = row_data[ntuple_column++];
    kin7.mblep = row_data[ntuple_column++];
    kin7.bhad.x = row_data[ntuple_column++];
    kin7.bhad.y = row_data[ntuple_column++];
    kin7.bhad.z = row_data[ntuple_column++];
    kin7.mbhad = row_data[ntuple_column++];
    aux.q.etaDet = row_data[ntuple_column++];
    aux.q.emf = row_data[ntuple_column++];
    aux.q.SVXtag = row_data[ntuple_column++];
    aux.q.Lxy = row_data[ntuple_column++];
    aux.q.sigmaLxy = row_data[ntuple_column++];
    aux.q.isTight = row_data[ntuple_column++];
    aux.q.nConeTracks = row_data[ntuple_column++];
    aux.q.posRate = row_data[ntuple_column++];
    aux.q.posBin = row_data[ntuple_column++];
    aux.q.negRate = row_data[ntuple_column++];
    aux.q.negBin = row_data[ntuple_column++];
    aux.qbar.etaDet = row_data[ntuple_column++];
    aux.qbar.emf = row_data[ntuple_column++];
    aux.qbar.SVXtag = row_data[ntuple_column++];
    aux.qbar.Lxy = row_data[ntuple_column++];
    aux.qbar.sigmaLxy = row_data[ntuple_column++];
    aux.qbar.isTight = row_data[ntuple_column++];
    aux.qbar.nConeTracks = row_data[ntuple_column++];
    aux.qbar.posRate = row_data[ntuple_column++];
    aux.qbar.posBin = row_data[ntuple_column++];
    aux.qbar.negRate = row_data[ntuple_column++];
    aux.qbar.negBin = row_data[ntuple_column++];
    aux.blep.etaDet = row_data[ntuple_column++];
    aux.blep.emf = row_data[ntuple_column++];
    aux.blep.SVXtag = row_data[ntuple_column++];
    aux.blep.Lxy = row_data[ntuple_column++];
    aux.blep.sigmaLxy = row_data[ntuple_column++];
    aux.blep.isTight = row_data[ntuple_column++];
    aux.blep.nConeTracks = row_data[ntuple_column++];
    aux.blep.posRate = row_data[ntuple_column++];
    aux.blep.posBin = row_data[ntuple_column++];
    aux.blep.negRate = row_data[ntuple_column++];
    aux.blep.negBin = row_data[ntuple_column++];
    aux.bhad.etaDet = row_data[ntuple_column++];
    aux.bhad.emf = row_data[ntuple_column++];
    aux.bhad.SVXtag = row_data[ntuple_column++];
    aux.bhad.Lxy = row_data[ntuple_column++];
    aux.bhad.sigmaLxy = row_data[ntuple_column++];
    aux.bhad.isTight = row_data[ntuple_column++];
    aux.bhad.nConeTracks = row_data[ntuple_column++];
    aux.bhad.posRate = row_data[ntuple_column++];
    aux.bhad.posBin = row_data[ntuple_column++];
    aux.bhad.negRate = row_data[ntuple_column++];
    aux.bhad.negBin = row_data[ntuple_column++];
    pmatch[0].isValid = row_data[ntuple_column++];
    pmatch[0].isAmbiguous = row_data[ntuple_column++];
    pmatch[0].bestJet = row_data[ntuple_column++];
    pmatch[0].bestJetDr = row_data[ntuple_column++];
    pmatch[0].secondBestJetDr = row_data[ntuple_column++];
    pmatch[0].bestReverseDr = row_data[ntuple_column++];
    pmatch[0].drToClosestParton = row_data[ntuple_column++];
    pmatch[0].lepDr = row_data[ntuple_column++];
    pmatch[0].partonPt = row_data[ntuple_column++];
    pmatch[0].partonEta = row_data[ntuple_column++];
    pmatch[0].partonMass = row_data[ntuple_column++];
    pmatch[0].ptRatio5 = row_data[ntuple_column++];
    pmatch[0].bestJetDEta = row_data[ntuple_column++];
    pmatch[0].bestJetDPhi = row_data[ntuple_column++];
    pmatch[0].etaDet = row_data[ntuple_column++];
    pmatch[1].isValid = row_data[ntuple_column++];
    pmatch[1].isAmbiguous = row_data[ntuple_column++];
    pmatch[1].bestJet = row_data[ntuple_column++];
    pmatch[1].bestJetDr = row_data[ntuple_column++];
    pmatch[1].secondBestJetDr = row_data[ntuple_column++];
    pmatch[1].bestReverseDr = row_data[ntuple_column++];
    pmatch[1].drToClosestParton = row_data[ntuple_column++];
    pmatch[1].lepDr = row_data[ntuple_column++];
    pmatch[1].partonPt = row_data[ntuple_column++];
    pmatch[1].partonEta = row_data[ntuple_column++];
    pmatch[1].partonMass = row_data[ntuple_column++];
    pmatch[1].ptRatio5 = row_data[ntuple_column++];
    pmatch[1].bestJetDEta = row_data[ntuple_column++];
    pmatch[1].bestJetDPhi = row_data[ntuple_column++];
    pmatch[1].etaDet = row_data[ntuple_column++];
    pmatch[2].isValid = row_data[ntuple_column++];
    pmatch[2].isAmbiguous = row_data[ntuple_column++];
    pmatch[2].bestJet = row_data[ntuple_column++];
    pmatch[2].bestJetDr = row_data[ntuple_column++];
    pmatch[2].secondBestJetDr = row_data[ntuple_column++];
    pmatch[2].bestReverseDr = row_data[ntuple_column++];
    pmatch[2].drToClosestParton = row_data[ntuple_column++];
    pmatch[2].lepDr = row_data[ntuple_column++];
    pmatch[2].partonPt = row_data[ntuple_column++];
    pmatch[2].partonEta = row_data[ntuple_column++];
    pmatch[2].partonMass = row_data[ntuple_column++];
    pmatch[2].ptRatio5 = row_data[ntuple_column++];
    pmatch[2].bestJetDEta = row_data[ntuple_column++];
    pmatch[2].bestJetDPhi = row_data[ntuple_column++];
    pmatch[2].etaDet = row_data[ntuple_column++];
    pmatch[3].isValid = row_data[ntuple_column++];
    pmatch[3].isAmbiguous = row_data[ntuple_column++];
    pmatch[3].bestJet = row_data[ntuple_column++];
    pmatch[3].bestJetDr = row_data[ntuple_column++];
    pmatch[3].secondBestJetDr = row_data[ntuple_column++];
    pmatch[3].bestReverseDr = row_data[ntuple_column++];
    pmatch[3].drToClosestParton = row_data[ntuple_column++];
    pmatch[3].lepDr = row_data[ntuple_column++];
    pmatch[3].partonPt = row_data[ntuple_column++];
    pmatch[3].partonEta = row_data[ntuple_column++];
    pmatch[3].partonMass = row_data[ntuple_column++];
    pmatch[3].ptRatio5 = row_data[ntuple_column++];
    pmatch[3].bestJetDEta = row_data[ntuple_column++];
    pmatch[3].bestJetDPhi = row_data[ntuple_column++];
    pmatch[3].etaDet = row_data[ntuple_column++];



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
