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
NTUPLE_UNPACKING_CODE

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
