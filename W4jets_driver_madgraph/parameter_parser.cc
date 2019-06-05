#include <cstring>
#include <cassert>
#include <cstdlib>

#include "parameter_parser.hh"

#ifndef CONST
#define CONST
#endif

static char* get_c_string_parameter(Tcl_Interp *interp,
                                    const char *param_list,
                                    const char *pname)
{
    char *pvalue = NULL;
    int i, listlen, pairlen;
    char **listelem, **ppair;
    char *plist_local;

    if (param_list == NULL) {
	Tcl_AppendResult(interp, "null parameter list", NULL);
	return NULL;
    } else if (param_list[0] == '\0') {
	Tcl_AppendResult(interp, "empty parameter list", NULL);
	return NULL;
    } else if (pname == NULL) {
	Tcl_AppendResult(interp, "null parameter name", NULL);
	return NULL;
    } else if (pname[0] == '\0') {
	Tcl_AppendResult(interp, "empty parameter name", NULL);
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
    assert(pvalue);\
    int status = TCL_ERROR;\
    char *str = get_c_string_parameter(interp, param_list, pname);\
    if (str) {\
	status = parser (interp, str, pvalue);\
	free(str);\
    }\
    if (status != TCL_OK)\
        return status;\
} while(0);

int get_double_parameter(Tcl_Interp *interp, const char *param_list,
                         const char *pname, double *pvalue)
{
    get_nonstring_parameter(Tcl_GetDouble);
    return TCL_OK;
}

int get_int_parameter(Tcl_Interp *interp, const char *param_list,
                      const char *pname, int *pvalue)
{
    get_nonstring_parameter(Tcl_GetInt);
    return TCL_OK;
}

int get_bool_parameter(Tcl_Interp *interp, const char *param_list,
                       const char *pname, bool *pbool)
{
    int tmp;
    int *pvalue = &tmp;
    get_nonstring_parameter(Tcl_GetBoolean);
    assert(pbool);
    *pbool = tmp;
    return TCL_OK;
}

int get_unsigned_parameter(Tcl_Interp *interp, const char *param_list,
                           const char *pname, unsigned *pu)
{
    int tmp;
    int *pvalue = &tmp;
    get_nonstring_parameter(Tcl_GetInt);
    if (tmp < 0)
    {
	Tcl_AppendResult(interp, "parameter named \"",
			 pname, "\" can not be negative", NULL);
        return TCL_ERROR;
    }
    assert(pu);
    *pu = tmp;
    return TCL_OK;
}

int get_string_parameter(Tcl_Interp *interp, const char *param_list,
                         const char *pname, std::string *pvalue)
{
    assert(pvalue);
    int status = TCL_ERROR;
    char *str = get_c_string_parameter(interp, param_list, pname);
    if (str) {
	*pvalue = str;
	free(str);
        status = TCL_OK;
    }
    return status;
}
