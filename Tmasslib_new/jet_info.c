#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "jet_info.h"

#ifdef __cplusplus
extern "C" {
#endif

void fill_jet_info(jet_info *jet, double px, double py, double pz, double m,
                   double bProb, double bFake, double etaDet, double syserr,
                   double derr_dpt, double cuterr, int isLoose,
                   int ntracks, int is_extra)
{
    assert(jet);

    jet->px      = px;
    jet->py      = py;
    jet->pz      = pz;
    jet->pt      = hypot(px, py);
    jet->p       = hypot(jet->pt, pz);
    jet->m       = m;
    if (jet->p > 0.0)
	jet->et  = hypot(jet->p, m)/jet->p*jet->pt;
    else
	jet->et  = 0.0;
    jet->btagEt  = jet->et;
    jet->bProb   = bProb;
    jet->bFake   = bFake;
    jet->etaDet  = etaDet;
    jet->syserr  = syserr;
    jet->derr_dpt = derr_dpt;
    jet->cuterr = cuterr;
    jet->perm_info = 0;
    jet->isLoose = isLoose;
    jet->ntracks = ntracks;
    jet->is_extra = is_extra;
}

void make_dummy_jet(jet_info *jet)
{
    assert(jet);
    memset(jet, 0, sizeof(jet_info));
}

int is_dummy_jet(const jet_info *jet)
{
    unsigned i=0;
    assert(jet);
    const char* cjet = (const char*)jet;
    for (; i<sizeof(jet_info); ++i)
        if (cjet[i] != '\0')
            return 0;
    return 1;
}

int jet_has_btag(const jet_info *j)
{
    assert(j);
    return (j->bProb > 0.5);
}

void print_jet_info(const jet_info *jet, FILE *stream)
{
    assert(jet);
    if (stream)
        fprintf(stream, "px = %g, py = %g, pz = %g, pt = %g,\n"
                "m = %g, btagEt = %g, bProb = %g, bFake = %g, "
                "etaDet = %g, syserr = %g, derr_dpt = %g, "
                "ntracks = %d, is_extra = %d",
                jet->px, jet->py, jet->pz, jet->pt,
                jet->m, jet->btagEt, jet->bProb, jet->bFake,
                jet->etaDet, jet->syserr, jet->derr_dpt,
                jet->ntracks, jet->is_extra);
}

void scale_jet(const jet_info *jet, double scale, jet_info *result)
{
    assert(jet);
    assert(result);
    assert(scale > 0.0);
    if (jet != result)
        *result = *jet;
    result->px *= scale;
    result->py *= scale;
    result->pz *= scale;
    result->pt *= scale;
    result->et *= scale;
    result->p  *= scale;
    result->m  *= scale;
}

double svx_tag_prob(const jet_info *jet, char q_type)
{
    assert(jet);

    /* Limits whithin which the function is defined */
    static const double etMin       = 15.0;
    static const double etMax       = 180.0;
    static const double etaMaxLight = 1.74;
    static const double etaMaxB     = 1.81;

    const double etj   = jet->btagEt < etMin ? etMin :
                         jet->btagEt > etMax ? etMax :
                         jet->btagEt;
    const double etj_2 = etj*etj;
    const double etj_3 = etj_2*etj;
    const double etj_4 = etj_3*etj;
    const double etj_5 = etj_4*etj;

    const char qtype = tolower(q_type);
    const double etaMax = qtype == 'q' ? etaMaxLight : etaMaxB;
    const double eta_in = fabs(asinh(jet->pz/jet->pt));
    const double etaj   = eta_in > etaMax ? etaMax : eta_in;
    const double etaj_2 = etaj*etaj;
    const double etaj_3 = etaj_2*etaj;
    const double etaj_4 = etaj_3*etaj;
    const double etaj_5 = etaj_4*etaj;

    double efftag = 1.0;

    switch (qtype)
    {
    case 'q':
        efftag *= (0.00355 - 0.000263*etj + 0.0000118*etj_2 - 
                   0.000000141*etj_3 + 0.000000000753*etj_4 - 
                   0.00000000000155*etj_5);
        efftag *= (0.821 + 0.452*etaj + 0.437*etaj_2 - 0.555*etaj_3);
        break;
    case 'c':
        efftag *= 0.22;
    case 'b':
        efftag *= (0.108 + 0.0175*etj - 0.000347*etj_2 + 
                   0.00000332*etj_3 - 0.0000000158*etj_4 + 
                   0.0000000000293*etj_5);
        efftag *= (1.05 - 0.517*etaj + 1.457*etaj_2 - 
                   1.20*etaj_3 + 0.0466*etaj_4 + 0.0895*etaj_5);
        break;
    default:
        assert(0);
    }

    return efftag;
}

#ifdef __cplusplus
}
#endif
