#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "parameter_parser.h"

#ifndef CONST
#define CONST
#endif

#ifdef __cplusplus
extern "C" {
#endif

char * get_string_parameter(Tcl_Interp *interp,
                            const char *param_list,
                            const char *pname)
{
    char *pvalue = NULL;
    int i, listlen, pairlen;
    char **listelem, **ppair;
    char *plist_local;

    if (param_list == NULL) {
	Tcl_SetResult(interp, "null parameter list", TCL_VOLATILE);
	return NULL;
    } else if (param_list[0] == '\0') {
	Tcl_SetResult(interp, "empty parameter list", TCL_VOLATILE);
	return NULL;
    } else if (pname == NULL) {
	Tcl_SetResult(interp, "null parameter name", TCL_VOLATILE);
	return NULL;
    } else if (pname[0] == '\0') {
	Tcl_SetResult(interp, "empty parameter name", TCL_VOLATILE);
	return NULL;
    }
    plist_local = strdup(param_list);
    assert(plist_local);
    if (Tcl_SplitList(interp, plist_local, &listlen, (CONST char ***)&listelem) != TCL_OK)
    {
	free(plist_local);
	return NULL;
    }
    for (i=0; i<listlen; ++i)
	if (Tcl_SplitList(interp, listelem[i], &pairlen, (CONST char ***)&ppair) == TCL_OK)
	{
	    if (pairlen == 2)
		if (strcmp(ppair[0], pname) == 0)
		{
		    pvalue = strdup(ppair[1]);
		    assert(pvalue);
		}
	    Tcl_Free((char *)ppair);
	    if (pvalue)
		break;
	}
    Tcl_Free((char *)listelem);
    free(plist_local);
    if (pvalue == NULL)
	Tcl_AppendResult(interp, "parameter named \"",
			 pname, "\" is not defined", NULL);
    return pvalue;
}

#define get_nonstring_parameter(parser) do {\
    int status = TCL_ERROR;\
    char *str = get_string_parameter(interp, param_list, pname);\
    if (str) {\
	status = parser (interp, str, pvalue);\
	free(str);\
    }\
    return status;\
} while(0);

int get_double_parameter(Tcl_Interp *interp, const char *param_list,
                         const char *pname, double *pvalue)
{
    get_nonstring_parameter(Tcl_GetDouble);
}

int get_int_parameter(Tcl_Interp *interp, const char *param_list,
                      const char *pname, int *pvalue)
{
    get_nonstring_parameter(Tcl_GetInt);
}

int get_boolean_parameter(Tcl_Interp *interp, const char *param_list,
                          const char *pname, int *pvalue)
{
    get_nonstring_parameter(Tcl_GetBoolean);
}

#ifdef __cplusplus
}
#endif
