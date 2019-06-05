#ifndef JET_INFO_H
#define JET_INFO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The "jet_info" structure below must not contain any pointers */
typedef struct
{
    double px;       /* X component of the jet momentum             */
    double py;       /* Y component of the jet momentum             */
    double pz;       /* Z component of the jet momentum             */
    double pt;       /* Transverse component of the jet momentum    */
    double et;       /* Transverse energy                           */
    double btagEt;   /* Et that does not scale (we assume that jet
                      * is filled with raw values to begin with)
                      */
    double p;        /* Magnitude of the jet momentum               */
    double m;        /* Jet mass                                    */
    double bProb;    /* Probability of being a b set by some
                      * b tagger. Not meaningful if 0.
                      */
    double bFake;    /* B tagger fake rate for this jet.
                      * Not meaningful if bProb is 0.
                      */
    double etaDet;   /* Jet detector eta                            */
    double syserr;   /* JES systematic error for this jet           */
    double derr_dpt; /* Derivative of the JES systematic error
                      * over the observed jet Pt. We assume that
                      * the derivative over Pz is 0.
                      */
    double cuterr;   /* JES systematic error at cutoff              */
    void *perm_info; /* Pointer to some additional per-jet info
                      * which must be permuted together with the jet
                      */
    int isLoose;     /* Is this a loose jet?                        */
    int ntracks;     /* Number of tracks associated with this jet   */
    int is_extra;    /* Set to 1 if this jet is not
                      * a ttbar decay product
                      */
} jet_info;

/* Do not ever set members of "jet_info" structure by hand,
 * use the "fill_jet_info" function instead. This way any
 * change in jet_info structure and "fill_jet_info" prototype
 * will be noticed by the compiler.
 */
void fill_jet_info(jet_info *jet, double px, double py, double pz, double m,
                   double bProb, double bFake, double etaDet, double syserr,
                   double derr_dpt, double cuterr, int isLoose,
                   int ntracks, int is_extra);

void make_dummy_jet(jet_info *jet);
int is_dummy_jet(const jet_info *jet);

/* A pretty dumb selector of b jets */
int jet_has_btag(const jet_info *jet);

/* Print the jet info (useful for debugging) */
void print_jet_info(const jet_info *jet, FILE *stream);

/* A function to multiply jet momentum by a scale. Note that
 * both "input_jet" and "result" can point to the same jet.
 */
void scale_jet(const jet_info *input_jet, double scale, jet_info *result);

/* SVX tag probability. "q_type" should be one of 'b' (for b quarks),
 * 'c' (charm), or 'q' (gluon or light quark, mistag probability is returned).
 */
double svx_tag_prob(const jet_info *jet, char q_type);

#ifdef __cplusplus
}
#endif

#endif /* JET_INFO_H */
