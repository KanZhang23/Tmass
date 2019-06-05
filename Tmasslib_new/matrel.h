#ifndef MATREL_H_
#define MATREL_H_

#ifdef __cplusplus
extern "C" {
#endif

double alpsks_();

double twidks_(double* M_T, double* M_W, double* M_B);

double qqbn_(double *pq, double *pqb, 
      	     double *pb, double *peb, double *pne, 
      	     double *pbb, double *pm, double *pnm,
      	     double *mW, double *GW, double *mt, int *opt);

double ggn_(double *pg1, double *pg2, 
            double *pb, double *peb, double *pne, 
            double *pbb, double *pm, double *pnm,
            double *mW, double *GW, double *mt, int *opt);

double ksqqb_(double *pq, double *pqb, 
      	      double *pb, double *peb, double *pne, 
      	      double *pbb, double *pm, double *pnm,
      	      double *mW, double *GW, double *mt, int *opt);

double ksgg_(double *pg1, double *pg2, 
             double *pb, double *peb, double *pne, 
      	     double *pbb, double *pm, double *pnm,
      	     double *mW, double *GW, double *mt, int *opt);

double mpqqb_(double *pg1, double *pg2, 
      	      double *pb, double *peb, double *pne, 
      	      double *pbb, double *pm, double *pnm,
	      double *mW, double *GW, double *mt, int *opt);

#ifdef __cplusplus
}
#endif

#endif /* MATREL_H_ */
