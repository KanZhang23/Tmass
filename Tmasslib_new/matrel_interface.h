#ifndef MATREL_INTERFACE_H_
#define MATREL_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This is an interface to fortran functions collected in the
 * file matrel.f. Read the header of this file for more information.
 *
 * Oct-21-2004   Pedro Movilla Fernandez
 */


  double QQBN( double *pq, double *pqb, 
	       double *pb, double *peb, double *pne, 
	       double *pbb, double *pm, double *pnm,
	       double *mW, double *GW, double *mt, int *opt);
  double GGN( double *pg1, double *pg2, 
	      double *pb, double *peb, double *pne, 
	      double *pbb, double *pm, double *pnm,
	      double *mW, double *GW, double *mt, int *opt);
  double KSQQB( double *pq, double *pqb, 
		double *pb, double *peb, double *pne, 
		double *pbb, double *pm, double *pnm,
		double *mW, double *GW, double *mt, int *opt);
  double KSGG( double *pg1, double *pg2, 
	       double *pb, double *peb, double *pne, 
	       double *pbb, double *pm, double *pnm,
	       double *mW, double *GW, double *mt, int *opt);
  double MPQQB( double *pg1, double *pg2, 
		double *pb, double *peb, double *pne, 
		double *pbb, double *pm, double *pnm,
		double *mW, double *GW, double *mt, int *opt);

#ifdef __cplusplus
}
#endif

#endif /* MATREL_INTERFACE_H_ */
