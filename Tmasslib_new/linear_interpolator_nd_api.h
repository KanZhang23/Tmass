#ifndef LINEAR_INTERPOLATOR_ND_API_H_
#define LINEAR_INTERPOLATOR_ND_API_H_

#include "tcl.h"

#ifdef __cplusplus
extern "C" {
#endif

int init_linear_interpolator_nd_api(Tcl_Interp *interp);
void cleanup_linear_interpolator_nd_api(void);

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_INTERPOLATOR_ND_API_H_ */
