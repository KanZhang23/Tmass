#include <cassert>

#include "interpolateCubicWithDerivatives.h"

#ifdef __cplusplus
extern "C" {
#endif

double interpolateCubicWithDerivatives(const double x,
    const double xleft, const double yleft, const double leftDeriv_in,
    const double xright, const double yright, const double rigthDeriv_in)
{
    const double unit = xright - xleft;
    assert(unit > 0.0);

    const double dx = (x - xleft)/unit;
    const double leftDeriv = leftDeriv_in*unit;
    const double rightDeriv = rigthDeriv_in*unit;
    const double a = leftDeriv + rightDeriv - 2.0*(yright - yleft);
    const double b = 3.0*(yright - yleft) - 2.0*leftDeriv - rightDeriv;

    return ((a*dx + b)*dx + leftDeriv)*dx + yleft;
}

#ifdef __cplusplus
}
#endif
