#ifndef CDF_3D_API_H_
#define CDF_3D_API_H_

#include "tcl.h"

#ifdef __cplusplus
extern "C" {
#endif

int init_cdf_3d_api(Tcl_Interp *interp);
void cleanup_cdf_3d_api(void);

#ifdef __cplusplus
}
#endif

#endif /* CDF_3D_API_H_ */
