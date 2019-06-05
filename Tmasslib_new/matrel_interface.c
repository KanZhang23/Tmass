#include "matrel_interface.h"
#include "matrel.h"

#ifdef __cplusplus
extern "C" {
#endif

double QQBN(
       double *pq, double *pqb, 
       double *pb, double *peb, double *pne, 
       double *pbb, double *pm, double *pnm,
       double *mW, double *GW, double *mt, int *opt)    
{ return qqbn_( pq, pqb, pb, peb, pne, pbb, pm, pnm, mW, GW, mt, opt ); }

double GGN( 
        double *pg1, double *pg2, 
	double *pb, double *peb, double *pne, 
	double *pbb, double *pm, double *pnm,
	double *mW, double *GW, double *mt, int *opt )    
{ return ggn_( pg1, pg2, pb, peb, pne, pbb, pm, pnm, mW, GW, mt, opt ); }

double KSQQB( 
        double *pq, double *pqb, 
	double *pb, double *peb, double *pne, 
	double *pbb, double *pm, double *pnm,
	double *mW, double *GW, double *mt, int *opt)  
{ return ksqqb_( pq, pqb, pb, peb, pne, pbb, pm, pnm, mW, GW, mt, opt); }

double KSGG( 
        double *pg1, double *pg2, 
        double *pb, double *peb, double *pne, 
        double *pbb, double *pm, double *pnm,
        double *mW, double *GW, double *mt, int *opt)  
{ return ksgg_( pg1, pg2, pb, peb, pne, pbb, pm, pnm, mW, GW, mt, opt); }

double MPQQB( double *pg1, double *pg2, 
	      double *pb, double *peb, double *pne, 
	      double *pbb, double *pm, double *pnm,
	      double *mW, double *GW, double *mt, int *opt)  
{ return mpqqb_( pg1, pg2, pb, peb, pne, pbb, pm, pnm, mW, GW, mt, opt); }

#ifdef __cplusplus
}
#endif

