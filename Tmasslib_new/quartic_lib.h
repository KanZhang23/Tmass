#ifndef QUARTIC_LIB_H_
#define QUARTIC_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

int cubic(double p,double q,double r,double v3[4]);
int quartic(double a,double b,double c,double d,double rts[4]);
void set_quartic_debug(int level);

#ifdef __cplusplus
}
#endif

#endif /* QUARTIC_LIB_H_ */
