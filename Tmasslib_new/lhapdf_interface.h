#ifndef LHAPDF_INTERFACE_H_
#define LHAPDF_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

void init_lhapdf(const char* pdfname, double q2factor);
void cleanup_lhapdf(void);
void calculate_lhapdf(double x, double Q2, double* gPdf, double qPdf[8]);
double alphas_lhapdf(double Q2);

#ifdef __cplusplus
}
#endif

#endif // LHAPDF_INTERFACE_H_
