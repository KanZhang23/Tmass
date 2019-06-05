#ifndef CDF_2D_API_H_
#define CDF_2D_API_H_

#include "tcl.h"

#ifdef __cplusplus
extern "C" {
#endif

int init_cdf_2d_api(Tcl_Interp *interp);
void cleanup_cdf_2d_api(void);

#ifdef __cplusplus
}
#endif

#endif /* CDF_2D_API_H_ */
