/* Auto-generated from data_integ_template_mtm3.c. Do not edit by hand. */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_integ_mtm3.h"
#include "histoscope.h"
#include "parton_syserr.h"
#include "transfer_function.h"
#include "tfs_2015.h"

#define LOG_MIN -1000.0
#define MAX_JETS_SAVED 20
#define L5_CORRECTOR 0
#define L7_CORRECTOR 1
#define PT_DONT_BOTHER 5.f
#define PT_SYSERR_MAX 500.f

/* Code which sets up the top mass scan parameters */
#include "scan_parameters.c"

/* Various local variables which will be preserved between rows */
static int evcount, last_result_id, passcount, uid = 0;
static Tcl_Interp *runInterp = NULL;

/* Function which simulates "level 0" jet */
#include "level0_pt.c"

static void fill_flt_buf(float* buf, const double value)
{
    buf[0] = *((float*)(&value));
    buf[1] = *((float*)(&value) + 1);
}

static double parton_syserr_calc(const particle_obj* parton, int isB)
{
    const float emf = 0.3f;
    float px = parton->p.x;
    float py = parton->p.y;
    float pt = sqrtf(px*px + py*py);
    if (pt < parton_syserr_pt_cutoff)
    {
        px *= (parton_syserr_pt_cutoff/pt);
        py *= (parton_syserr_pt_cutoff/pt);
    }
    const double eta = Eta(parton->p);
    const double pt0 = level0_pt(L5_CORRECTOR, pt, emf, eta);
    return generic_correction_uncertainty(L5_CORRECTOR, pt0, eta, 1U);
}

/* The following function will be called first */
int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id,
                        const char *some_string)
{
    /* Setup the parameters and propagators */
    if (setup_scan_parameters(interp, some_string) != TCL_OK)
        return TCL_ERROR;

    /* Set up the calculator for parton-based JES systematic error */
    set_parton_syserr_calculator(parton_syserr_calc);

    /* Initialize internal structures */
    evcount = 0;
    last_result_id = 0;
    passcount = 0;
    runInterp = interp;

    return TCL_OK;
}

/* The function below will be called at the end of a successful scan */
int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id,
                            const char *some_string)
{
    cleanup_scan_parameters();
    Tcl_SetObjResult(interp, Tcl_NewIntObj(last_result_id));
    return TCL_OK;
}

/* The following function will be called for every ntuple row.
 * Scanning can be terminated at any time by returning TCL_ERROR.
 */
int hs_ntuple_scan_function(Tcl_Interp *interp, const float *row_data)
{
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
        float posBin;
        float posRate;
        float scale4;
        float scale5;
        float scale7;
        float sigmaLxy;
        float SVXtag;
    } jet[20];
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



    if (evcount < maxevents)
    {
        if (evcount >= skipevents)
        {
            static norm_accumulator *mcresults = NULL;
            static unsigned mc_scan_result_size = 0;
            static char *vars[] = {
                "mt",
                "prob",
                "logli",
                "npoints",
                "jes",
		"errprob",
		"errfrac",
                "eff",
                "prob_0",
                "prob_1"
            };
            static const unsigned nvars = sizeof(vars)/sizeof(vars[0]);
            static char title[256];

            const int nTightJets = (int)ev.nTightJet;
            const int njets = (int)ev.nAllJet < MAX_JETS_SAVED ? 
                              (int)ev.nAllJet : MAX_JETS_SAVED;
            jet_info jets[5];
            time_t timestamp = time(NULL);
            int i, id = 0, imass, ifill = 0, delta_t = 0;
            unsigned ijes;
            Scan_status scan_status;

	    ++passcount;

	    /* Print basic event id. Useful in case we die
             * inside the top mass library.
             */
	    printf("Processing row %d run %d event %d\n",
		   (int)thisRow, (int)runNumber,
		   65536*(int)eventNum0 + (int)eventNum1);
	    fflush(stdout);

            /* Check the number of jets */
            if (nTightJets < 4 || nTightJets > 5)
            {
                sprintf(title, "Can't process events with %d tight jets", nTightJets);
                Tcl_SetResult(interp, title, TCL_VOLATILE);
                return TCL_ERROR;
            }

            /* Jet permutations must be turned on
             * in order to get a meaningful scan
             */
            if (!ipars.permute_jets)
            {
                Tcl_SetResult(interp, "Please turn on jet permutations",
                              TCL_VOLATILE);
                return TCL_ERROR;
            }

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
                /*
                 * init_generic_corrections(L7_CORRECTOR, 7, nvtx, conesize,
                 *                          version, syscode, nrun, mode);
                 */
            }

            const double l5cut = get_jet_pt_cutoff();

	    /* Fill out the jet information */
            for (i=0; i<njets; ++i)
                if (jet[i].isTight > 0.5f)
                {
                    /* These are level 0 jets. Make sure that the L5
                     * jet Pt is consistent with the cut.
                     */
                    const unsigned corrMode = 1U;
                    const double pt0 = sqrt(jet[i].p.x*jet[i].p.x + jet[i].p.y*jet[i].p.y);
                    const double ptJet = pt0*jet[i].scale5;
                    if (ptJet >= l5cut*0.999999)
                    {
                        const double eta = jet[i].etaDet;
                        const double tightJetPtCut = level0_pt(L5_CORRECTOR, l5cut,
                                                               jet[i].emf, eta);
                        const float cuterr = generic_correction_uncertainty(
                            L5_CORRECTOR, tightJetPtCut, eta, corrMode);

                        const float syserr = generic_correction_uncertainty(
                            L5_CORRECTOR, pt0, eta, corrMode);

                        /* Calculate uncertainty derivative w.r.t. L5 (not L0!) jet pt */
                        const double delta = ptJet*0.001;
                        const double ptPlus0 = level0_pt(L5_CORRECTOR, ptJet+delta, 0.3f, eta);
                        const double ptMinus0 = level0_pt(L5_CORRECTOR, ptJet-delta, 0.3f, eta);
                        const double sysplus = generic_correction_uncertainty(
                            L5_CORRECTOR, ptPlus0, eta, corrMode);
                        const double sysminus = generic_correction_uncertainty(
                            L5_CORRECTOR, ptMinus0, eta, corrMode);
                        const double sysderi = (sysplus - sysminus)/2.0/delta;

                        assert((unsigned)ifill < sizeof(jets)/sizeof(jets[0]));
                        fill_jet_info(jets+ifill, jet[i].p.x, jet[i].p.y, jet[i].p.z,
                                      jet[i].mass, jet[i].SVXtag, jet[i].negRate, eta,
                                      syserr, sysderi, cuterr, 0, (int)jet[i].nConeTracks, 0);
                        scale_jet(jets+ifill, jet[i].scale5, jets+ifill);
                        ++ifill;
                    }
                }
            assert(ifill >= 4);

            /* Allocate memory for scan results */
            get_static_memory((void **)&mcresults, sizeof(norm_accumulator),
                              &mc_scan_result_size, topscan_points*n_jes_values);

	    /* Perform the scan */
            scan_status = mc_top_jes_scan(
                &ipars, &mpars, jes_values, n_jes_values,
                top_mass_values, topscan_points,
                jets, ifill, ev.l.x, ev.l.y, ev.l.z,
                (int)ev.leptonCharge, (int)ev.leptonType,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                prob_to_acquire_jet, prob_to_loose_parton,
                mcresults);
            assert(scan_status < N_SCAN_STATUS_VALUES);
            delta_t = (int)(time(NULL)-timestamp);

	    /* If requested, fill the ntuple of results */
            if (fill_result_ntuple)
            {
                const int ntuple_uid = uid++;
                sprintf(title, "Mass scan ntuple %d, status is %s, t = %d sec",
                        evcount, scan_status_names[scan_status], delta_t);
                id = hs_create_ntuple(ntuple_uid, title, category, nvars, vars);
                if (id <= 0)
                {
                    hs_ntuple_scan_conclude(interp, -1, NULL);
                    Tcl_SetResult(interp, "failed to create mass scan ntuple",
                                  TCL_VOLATILE);
                    return TCL_ERROR;
                }

                for (ijes=0; ijes<n_jes_values; ++ijes)
                    for (imass=0; imass<topscan_points; ++imass)
                    {
                        float data[nvars];
                        unsigned ivar = 0;
                        int fillstat;

                        const norm_accumulator *r = mcresults + 
                            imass*n_jes_values + ijes;
                        const double prob = norm_value(r);
                        const double errprob = norm_error(r);

                        assert(prob >= 0.0);
                        assert(errprob >= 0.0);
                        data[ivar++] = top_mass_values[imass];
                        data[ivar++] = prob_factor*prob;
                        if (prob > 0.0)
                            data[ivar++] = log(prob_factor*prob);
                        else
                            data[ivar++] = LOG_MIN;
                        data[ivar++] = r->ntries;
                        data[ivar++] = jes_values[ijes];
                        data[ivar++] = prob_factor*errprob;
                        if (prob > 0.0)
                            data[ivar++] = errprob/prob;
                        else
                            data[ivar++] = 0.0;
                        if (r->wmax > 0.0)
                            data[ivar++] = prob/r->wmax;
                        else
                            data[ivar++] = 0.0;

                        fill_flt_buf(data+ivar, prob); ivar+=2;

                        assert(ivar == nvars);
                        fillstat = hs_fill_ntuple(id, data);
                        assert(fillstat == id);
                    }
            }

            last_result_id = id;

	    if (periodic_script && script_period && passcount % script_period == 0)
		if (Tcl_GlobalEval(runInterp, periodic_script) != TCL_OK)
		    return TCL_ERROR;

            printf("Run %d event %d\n", (int)runNumber,
                   65536*(int)eventNum0 + (int)eventNum1);
            printf("Event %d processing time is %d sec\n", evcount, delta_t);
            printf("Scan status: %s\n", scan_status_names[scan_status]);
	    fflush(stdout);
        }
	++evcount;
    }

    return TCL_OK;
}
