#ifndef RANDOM_JET_MASSES_H_
#define RANDOM_JET_MASSES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The function below should generate jet mass (not parton!).
 * It is currently not implemented (always returns 0).
 */
double random_jet_mass(int isB);

#ifdef __cplusplus
}
#endif

#endif /* RANDOM_JET_MASSES_H_ */
