#ifndef TWIDTH_H_
#define TWIDTH_H_

#ifdef __cplusplus
extern "C" {
#endif

void setBQuarkMass(double mb);
double getBQuarkMass(void);

double getTopWidth(double mt, double mw, double mb);

#ifdef __cplusplus
}
#endif

#endif // TWIDTH_H_
