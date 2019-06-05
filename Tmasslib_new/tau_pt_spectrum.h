#ifndef TAU_PT_SPECTRUM_H
#define TAU_PT_SPECTRUM_H

#define TAU_LEPTON_MASS 1.777

#ifdef __cplusplus
extern "C" {
#endif

double tau_daughter_spectrum(double x, double mtop, int ltype);

double tau_daughter_efficiency(double xcut, double mtop, int ltype);

#ifdef __cplusplus
}
#endif

#endif /* TAU_PT_SPECTRUM_H */
