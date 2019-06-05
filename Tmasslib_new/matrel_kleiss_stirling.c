#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "matrel_kleiss_stirling.h"
#include "matrel_interface.h"

#define LIGHT_Q_MASS 0.0
#define G_MASS 0.0

#ifdef __cplusplus
extern "C" {
#endif

void matrel_kleiss_stirling(
     particle_obj ebar_lab, particle_obj nu_lab, particle_obj b_lab,
     particle_obj mu_lab, particle_obj nubar_lab, particle_obj bbar_lab,
     double mW, double GW, double mt, 
     double *matrel_qq, double *matrel_gg)
{
    /* Flag for including/excluding spin correlations */
    const int include_spin_correlation = 1;

    /* Propagator options:
     * include_propagator = 1 ... include W propagator but not top propagator
     * include_propagator = 2 ... include top propagator but not W propagator
     * include_propagator = 3 ... include W propagator and top propagator
     * include_propagator = 4 ... include modified top propagator but no W propagator
     *                   else ... include neither W nor top propagator
     */
    int include_propagator = 3;

    /* Four-momenta of the top quarks */
    const particle_obj t_lab = sum4(sum4(ebar_lab, nu_lab), b_lab);
    const particle_obj tbar_lab = sum4(sum4(mu_lab, nubar_lab), bbar_lab);
    const particle_obj ttbar_lab = sum4(t_lab, tbar_lab);

    /* Derive initial state parton four-momenta */
    /* const double Tz = t_lab.p.z + tbar_lab.p.z; */
    /* const double sqrtMT = sqrt( ttbar_lab.m*ttbar_lab.m + Tz*Tz ); */
    /* const double  qz = .5*( Tz+sqrtMT ); */
    /* const double qbz = .5*( Tz-sqrtMT ); */
    /* const particle_obj    q_lab = { {0., 0., qz   } , 0.}; */
    /* const particle_obj qbar_lab = { {0., 0., qbz  } , 0.}; */
    /* const particle_obj    q_lab = { {            0.,            0., qz   }, 0.}; */
    /* const particle_obj qbar_lab = { { ttbar_lab.p.x, ttbar_lab.p.y, qbz  }, 0.}; */
    /* const particle_obj    q_lab = { { .5*ttbar_lab.p.x, .5*ttbar_lab.p.y, qz   }, 0.}; */
    /* const particle_obj qbar_lab = { { .5*ttbar_lab.p.x, .5*ttbar_lab.p.y, qbz  }, 0.}; */
    /* Assume three-momenta parallel to z axis in ttbar rest frame 
     * and boost back to lab frame */
    const particle_obj qbar = particle(v3(0., 0., -.5*ttbar_lab.m), LIGHT_Q_MASS);
    const particle_obj q    = particle(v3(0., 0.,  .5*ttbar_lab.m), LIGHT_Q_MASS);
    const particle_obj g1   = particle(v3(0., 0., -.5*ttbar_lab.m), G_MASS);
    const particle_obj g2   = particle(v3(0., 0.,  .5*ttbar_lab.m), G_MASS);

    const boost_obj lab_boost = inverse_boost(rest_boost(ttbar_lab));

    const particle_obj qbar_lab = boost(qbar, lab_boost);
    const particle_obj    q_lab = boost(   q, lab_boost);
    const particle_obj   g1_lab = boost(  g1, lab_boost);
    const particle_obj   g2_lab = boost(  g2, lab_boost);

    /* Prepare arrays for matrix element calculation */
    double  pq[4] ={ -energy(     q_lab ),    -q_lab.p.x,    -q_lab.p.y,    -q_lab.p.z };
    double pqb[4] ={ -energy(  qbar_lab ), -qbar_lab.p.x, -qbar_lab.p.y, -qbar_lab.p.z };
    double pg1[4] ={ -energy(    g1_lab ),   -g1_lab.p.x,   -g1_lab.p.y,   -g1_lab.p.z };
    double pg2[4] ={ -energy(    g2_lab ),   -g2_lab.p.x,   -g2_lab.p.y,   -g2_lab.p.z };
    double  pb[4]= {  energy(     b_lab ),     b_lab.p.x,     b_lab.p.y,     b_lab.p.z };
    double peb[4]= {  energy(  ebar_lab ),  ebar_lab.p.x,  ebar_lab.p.y,  ebar_lab.p.z };
    double pne[4]= {  energy(    nu_lab ),    nu_lab.p.x,    nu_lab.p.y,    nu_lab.p.z };
    double pbb[4]= {  energy(  bbar_lab ),  bbar_lab.p.x,  bbar_lab.p.y,  bbar_lab.p.z };
    double  pm[4]= {  energy(    mu_lab ),    mu_lab.p.x,    mu_lab.p.y,    mu_lab.p.z };
    double pnm[4]= {  energy( nubar_lab ), nubar_lab.p.x, nubar_lab.p.y, nubar_lab.p.z };

    /* Calculate the matrix elements */
    if( include_spin_correlation )
    {
	/*                 qqbar/gg |      t      |     tbar  
	 *   (PLAB index:     1    2   6    8    7    3   4    5) */
	*matrel_qq = KSQQB(  pq, pqb, pb, peb, pne, pbb, pm, pnm,
			     &mW, &GW, &mt, &include_propagator);
	*matrel_gg = KSGG ( pg1, pg2, pb, peb, pne, pbb, pm, pnm,
			    &mW, &GW, &mt, &include_propagator);
    }
    else
    {
	*matrel_qq = QQBN(  pq, pqb, pb, peb, pne, pbb, pm, pnm,
			    &mW, &GW, &mt, &include_propagator);
	*matrel_gg =  GGN( pg1, pg2, pb, peb, pne, pbb, pm, pnm,
			   &mW, &GW, &mt, &include_propagator); 
    }
    if (*matrel_qq < 0.0)
    {
        /* fprintf(stdout,"matrel_kleiss_stirling: matrel_qq=%g",*matrel_qq); */
	*matrel_qq = 0.0; 
    }
    if (*matrel_gg < 0.0)
    {
	/* fprintf(stdout,"matrel_kleiss_stirling: matrel_gg=%g",*matrel_gg); */
	*matrel_gg = 0.0; 
    }
    /* assert( *matrel_qq>=0.0 );  */
    /* assert( *matrel_gg>=0.0 );  */

    if( include_propagator==4 )
    {
	const double mt4 = mt*mt*mt*mt;
	*matrel_qq /= mt4;
	*matrel_gg /= mt4;
    }
}

void kleiss_stirling_weights(
    particle_obj q, particle_obj qbar, particle_obj bhad,
    particle_obj blep, particle_obj lep, particle_obj nu,
    int leptonCharge, double mW, double widthW, double mt,
    double *weight_qq, double *weight_gg)
{
    /* Note that the following might not be correct for Pythia.
     * In Herwig we will have q set to ubar or cbar, qbar set to d or s.
     * That is, q and qbar meaning is swapped due to specific ordering
     * of the decay daughters.
     */
    if (leptonCharge > 0)
        matrel_kleiss_stirling(
            lep, nu, blep, qbar, q, bhad,
            mW, widthW, mt, weight_qq, weight_gg);
    else if (leptonCharge < 0)
        matrel_kleiss_stirling(
            qbar, q, bhad, lep, nu, blep,
            mW, widthW, mt, weight_qq, weight_gg);
    else
        assert(!"Lepton charge must not be 0");
}

#ifdef __cplusplus
}
#endif
