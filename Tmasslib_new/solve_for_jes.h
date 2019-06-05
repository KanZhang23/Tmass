#ifndef SOLVE_FOR_JES_H_
#define SOLVE_FOR_JES_H_

#ifdef __cplusplus
extern "C" {
#endif

double solve_for_jes(double ptMC, double etaJet, double emf, double deltaJES);

double get_jet_sys_uncert(double pt, double detectorEta);

#ifdef __cplusplus
}
#endif

#endif /* SOLVE_FOR_JES_H_ */
