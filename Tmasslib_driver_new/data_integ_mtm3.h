#ifndef DATA_INTEG_MTM3_H_
#define DATA_INTEG_MTM3_H_

#include "tcl.h"

int hs_ntuple_scan_init(Tcl_Interp *interp, int ntuple_id,
                        const char *some_string);

int hs_ntuple_scan_conclude(Tcl_Interp *interp, int ntuple_id,
                            const char *some_string);

int hs_ntuple_scan_function(Tcl_Interp *interp, const float *row_data);

#endif /* DATA_INTEG_MTM3_H_ */
