#ifndef INTERPOLATECUBICWITHDERIVATIVES_H_
#define INTERPOLATECUBICWITHDERIVATIVES_H_

//
// Cubic interpolation between xleft and xright.
//
// The value of the cubic polynomial at xleft is yleft and its derivative
// is leftDeriv. The value of the cubic polynomial at xright is yright
// and its derivative there is rigthDeriv.
//
#ifdef __cplusplus
extern "C" {
#endif

double interpolateCubicWithDerivatives(
    double x, double xleft, double yleft, double leftDeriv,
    double xright, double yright, double rigthDeriv);

#ifdef __cplusplus
}
#endif

#endif // INTERPOLATECUBICWITHDERIVATIVES_H_
