#ifndef HS_SHAREDLIB_H_
#define HS_SHAREDLIB_H_

#include "simple_tcl_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void *handle;
    char *filename;
    int refcount;
} DLLibrary;

/* Headers for the Tcl API functions */
tcl_routine(sharedlib);

#ifdef __cplusplus
}
#endif

#endif /* not HS_SHAREDLIB_H_ */
