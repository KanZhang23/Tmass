#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "topmass_integrator.h"
#include "transfer_function.h"
#include "single_parton_efficiency.h"
#include "parton_syserr.h"

#ifdef __cplusplus
extern "C" {
#endif

int fill_integ_parameters(integ_parameters *ip, double nominal_w_mass,
			  double nominal_w_width, double cms_energy,
			  double light_jet_mass, double had_b_jet_mass,
                          double lep_b_jet_mass, double whad_coverage,
                          double wlep_coverage,
                          double thad_coverage, double tlep_coverage,
                          double top_width,
			  int wlep_npoints,
                          int debug_level, int matrel_code,
			  int process_tcl_events, int permute_jets,
			  int param_grid_absolute)
{
    /* Basic typo protection */
    if (nominal_w_mass <= 0.0)                         return 1;
    if (nominal_w_width <= 0.0)                        return 2;
    if (cms_energy <= 0.0)                             return 3;
    if (light_jet_mass >= nominal_w_mass)              return 4;
    if (had_b_jet_mass >= nominal_w_mass)              return 5;
    if (lep_b_jet_mass >= nominal_w_mass)              return 6;
    if (whad_coverage <= 0.0 || whad_coverage > 1.0)   return 7;
    if (wlep_coverage <= 0.0 || wlep_coverage > 1.0)   return 8;
    if (wlep_npoints <= 0)                             return 9;
    if (matrel_code < 0)                               return 10;
    if (thad_coverage <= 0.0 || thad_coverage > 1.0)   return 11;
    if (tlep_coverage <= 0.0 || tlep_coverage > 1.0)   return 12;
    /* if (top_width <= 0.0)                              return 13; */

    ip->nominal_w_mass = nominal_w_mass;
    ip->nominal_w_width = nominal_w_width;
    ip->cms_energy = cms_energy;
    ip->light_jet_mass = light_jet_mass;
    ip->had_b_jet_mass = had_b_jet_mass;
    ip->lep_b_jet_mass = lep_b_jet_mass;
    ip->whad_coverage = whad_coverage;
    ip->wlep_coverage = wlep_coverage;
    ip->thad_coverage = thad_coverage;
    ip->tlep_coverage = tlep_coverage;
    /* ip->top_width = top_width; */
    ip->wlep_npoints = wlep_npoints;
    ip->debug_level = debug_level;
    ip->matrel_code = matrel_code;
    ip->process_tcl_events = process_tcl_events;
    ip->permute_jets = permute_jets;
    ip->param_grid_absolute = param_grid_absolute;

    return 0;
}

void cleanup_integ_parameters(integ_parameters *ip)
{
}

void print_integ_parameters(const integ_parameters *ip, FILE *stream)
{
    fprintf(stream,
            "Top mass integrator parameters: "
	    "nominal_w_mass = %g, "
	    "nominal_w_width = %g, "
	    "cms_energy = %g, "
	    "light_jet_mass = %g, "
	    "had_b_jet_mass = %g, "
	    "lep_b_jet_mass = %g, "
	    "whad_coverage = %g, "
	    "wlep_coverage = %g, "
	    "thad_coverage = %g, "
	    "tlep_coverage = %g, "
/*            "top_width = %g, " */
	    "wlep_npoints = %u, "
	    "debug_level = %d, "
	    "matrel_code = %d, "
	    "process_tcl_events = %d, "
	    "permute_jets = %d, "
	    "param_grid_absolute = %d\n",
	    ip->nominal_w_mass,
	    ip->nominal_w_width,
	    ip->cms_energy,
	    ip->light_jet_mass,
	    ip->had_b_jet_mass,
	    ip->lep_b_jet_mass,
	    ip->whad_coverage,
	    ip->wlep_coverage,
            ip->thad_coverage,
            ip->tlep_coverage,
/*            ip->top_width,     */
	    ip->wlep_npoints,
	    ip->debug_level,
	    ip->matrel_code,
	    ip->process_tcl_events,
	    ip->permute_jets,
            ip->param_grid_absolute);
}

void hadronic_side_tf_product(
    const double *jes_points, const unsigned n_jes_points,
    const hadron_side_solution *hsol, const jet_info *pq,
    const jet_info *pqbar, const jet_info *pb,
    Prob_to_loose_parton *prob_to_loose_parton,
    double *tfprod)
{
    const particle_obj q = particle(v3(hsol->qPx, hsol->qPy, hsol->qPz), hsol->mq);
    const particle_obj qbar = particle(v3(hsol->qbarPx, hsol->qbarPy, hsol->qbarPz), hsol->mqbar);
    const particle_obj b = particle(v3(hsol->bPx, hsol->bPy, hsol->bPz), hsol->mb);
    unsigned ijes;

    if (pq->is_extra)
    {
        assert(0);

        /* printf("In hadronic_side_tf_product: extra q\n"); */
        const double syserr = parton_syserr(&q, 0);
        assert(prob_to_loose_parton);
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*syserr;
            tfprod[ijes] = prob_to_loose_parton(&q, scale, 0);
        }
    }
    else
    {
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*pq->syserr;
            const double corr = jes_points[ijes]*pq->derr_dpt*pq->pt;
            const double f = scale + corr;
            if (f > 0.0)
            {
                tfprod[ijes] = f*transfer_function(q, pq, scale, 0);
                if (tfprod[ijes] > 0.0)
                    tfprod[ijes] *= single_parton_eff(q, 0, jes_points[ijes], pq->cuterr);
                else
                    tfprod[ijes] = 0.0;
            }
            else
                tfprod[ijes] = 0.0;
        }
    }

    if (pqbar->is_extra)
    {
        assert(0);

        /* printf("In hadronic_side_tf_product: extra qbar\n"); */
        const double syserr = parton_syserr(&qbar, 0);
        assert(prob_to_loose_parton);
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*syserr;
            tfprod[ijes] *= prob_to_loose_parton(&qbar, scale, 0);
        }
    }
    else
    {
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*pqbar->syserr;
            const double corr = jes_points[ijes]*pqbar->derr_dpt*pqbar->pt;
            const double f = scale + corr;
            if (f > 0.0)
            {
                if (tfprod[ijes] > 0.0)
                    tfprod[ijes] *= (f*transfer_function(qbar, pqbar, scale, 0));
                else
                    tfprod[ijes] = 0.0;
                if (tfprod[ijes] > 0.0)
                    tfprod[ijes] *= single_parton_eff(qbar, 0, jes_points[ijes], pqbar->cuterr);
                else
                    tfprod[ijes] = 0.0;
            }
            else
                tfprod[ijes] = 0.0;
        }
    }

    if (pb->is_extra)
    {
        assert(0);

        /* printf("In hadronic_side_tf_product: extra bhad\n"); */
        const double syserr = parton_syserr(&b, 1);
        assert(prob_to_loose_parton);
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*syserr;
            tfprod[ijes] *= prob_to_loose_parton(&b, scale, 1);
        }
    }
    else
    {
        for (ijes=0; ijes<n_jes_points; ++ijes)
        {
            const double scale = 1.0 + jes_points[ijes]*pb->syserr;
            const double corr = jes_points[ijes]*pb->derr_dpt*pb->pt;
            const double f = scale + corr;
            if (f > 0.0)
            {
                if (tfprod[ijes] > 0.0)
                    tfprod[ijes] *= (f*transfer_function(b, pb, scale, 1));
                else
                    tfprod[ijes] = 0.0;
                if (tfprod[ijes] > 0.0)
                    tfprod[ijes] *= single_parton_eff(b, 1, jes_points[ijes], pb->cuterr);
                else
                    tfprod[ijes] = 0.0;
            }
            else
                tfprod[ijes] = 0.0;
        }
    }
}

#ifdef __cplusplus
}
#endif
