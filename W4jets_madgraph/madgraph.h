#ifndef MADGRAPH_H_
#define MADGRAPH_H_

// Some MG common blocks

#ifdef __cplusplus
extern "C" {
#endif

struct dcomplex {double real, imag;};

extern struct strong{
  double g;
} strong_;

extern struct weak{
  struct dcomplex gal[2];
} weak_;

extern struct rscale{
  double mu_r;
} rscale_;

extern struct masses{
  double mdl_mh,mdl_mz,mdl_mt,mdl_mw,mdl_mta;
} masses_;

extern struct widths{
  double mdl_wz,mdl_wh,mdl_ww,mdl_wt;
} widths_;

extern struct couplings{
//  struct dcomplex gc_10, gc_11, gc_12, gc_100;
  std::complex<double> gc_10, gc_11, gc_12, gc_100;
} couplings_;

extern struct to_scale{
  double  scale,scalefact,alpsfact;
  bool fixed_ren_scale,fixed_fac_scale,fixed_couplings;
  int ickkw,nhmult;
  bool hmult;
  int asrwgtflavor;
} to_scale_;

extern struct to_collider{
  double ebeam[2], xbk[2],q2fact[2];
  int lpp[2];
} to_collider_;

extern struct to_pdf{
  int lhaid;
  char pdlabel[7];
  char epa_label[7];
} to_pdf_;


double p0_dsig_(double p[][4], double* WTMC);
double p1_dsig_(double p[][4], double* WTMC);

double pdg2pdf_(int &, int &, double&, double&);

void pdfwrap_();

#ifdef __cplusplus
}
#endif

#endif // MADGRAPH_H_
